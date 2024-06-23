import { SerialPort } from "serialport";

export async function getAvailablePorts(): Promise<string[]> {
    console.log("Getting available serial ports...");
    const ports = await SerialPort.list();
    console.log("Available ports:", ports);

    return ports.map((port) => port.path);
}
