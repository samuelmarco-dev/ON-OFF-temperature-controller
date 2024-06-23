import { SerialPort } from "serialport";
import { ReadlineParser } from "@serialport/parser-readline";

import Tempeture from "../interfaces/index.js";
import extractTemperature from "../services/extractSerial.js";

export async function readData(port: SerialPort, dataSerial: Tempeture[]): Promise<string> {
    return new Promise((resolve, reject) => {
        const parser = port.pipe(new ReadlineParser({ delimiter: "\r\n" }));

        parser.on("data", (data: Buffer) => {
            const dataString = data.toString();
            console.log(`Received data: ${dataString}`);

            const lastData = extractTemperature(dataString);
            dataSerial.push(lastData);
            resolve(dataString);
        });

        parser.on("error", (error: Error) => {
            console.error("Error in ReadlineParser:", error);
            reject(error);
        });

        port.on("error", (error: Error) => {
            console.error("SerialPort error:", error);
            reject(error);
        });
    });
}
