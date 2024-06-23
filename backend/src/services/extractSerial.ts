import Tempeture from "../interfaces";

export default function extractTemperature(serialOutput: string): Tempeture {
    const parts = serialOutput.split(";");

    const roomTemperaturePart = parts[0].substring(1); // Remove o primeiro caractere 'A'
    const roomTemperature = parseFloat(roomTemperaturePart.replace(",", ".")); // Converte para número float

    const desiredTemperaturePart = parts[1].substring(1); // Remove o primeiro caractere 'D'
    const desiredTemperature = parseFloat(desiredTemperaturePart); // Converte para número float

    return {
        roomTemperature,
        desiredTemperature,
    };
}
