let data = []; // Array inicial vazio para armazenar os dados

const fetchData = async () => {
    const response = await fetch("http://localhost:3000/data");
    const data = await response.json();
    return data;
};

const margin = { top: 20, right: 30, bottom: 30, left: 40 };
const width = 800 - margin.left - margin.right;
const height = 400 - margin.top - margin.bottom;

const svg = d3
    .select("#chart")
    .append("svg")
    .attr("width", width + margin.left + margin.right)
    .attr("height", height + margin.top + margin.bottom)
    .append("g")
    .attr("transform", `translate(${margin.left},${margin.top})`);

const x = d3.scaleTime().range([0, width]);
const y = d3.scaleLinear().range([height, 0]);

const lineRoomTemperature = d3
    .line()
    .x((d) => x(d.timestamp))
    .y((d) => y(d.roomTemperature));

const lineDesiredTemperature = d3
    .line()
    .x((d) => x(d.timestamp))
    .y((d) => y(d.desiredTemperature));

svg.append("g")
    .attr("class", "x axis")
    .attr("transform", `translate(0,${height})`);

svg.append("g").attr("class", "y axis");

svg.append("path")
    .attr("class", "line roomTemperature")
    .style("stroke", "steelblue");

svg.append("path")
    .attr("class", "line desiredTemperature")
    .style("stroke", "red");

function updateChart() {
    x.domain(d3.extent(data, (d) => d.timestamp));
    y.domain([
        0,
        d3.max(data, (d) => Math.max(d.roomTemperature, d.desiredTemperature)),
    ]);

    svg.select(".x.axis").call(
        d3.axisBottom(x).tickFormat(d3.timeFormat("%H:%M:%S"))
    );

    svg.select(".y.axis").call(d3.axisLeft(y));

    svg.select(".line.roomTemperature")
        .datum(data)
        .attr("d", lineRoomTemperature);

    svg.select(".line.desiredTemperature")
        .datum(data)
        .attr("d", lineDesiredTemperature);
}

async function simulateSerialData() {
    // Simula o recebimento de dados da porta serial
    const newData = await fetchData();

    // Adicionamos os novos dados ao array data
    data.push({
        timestamp: new Date(),
        roomTemperature: newData.roomTemperature,
        desiredTemperature: newData.desiredTemperature,
    });

    // Limitamos o array data a 25 elementos
    if (data.length > 25) data.shift();

    updateChart();
}

setInterval(simulateSerialData, 1000);
