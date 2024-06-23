import { SerialPort } from "serialport";

export async function connectToPort(portPath: string, baudRate: number): Promise<SerialPort> {
    console.log(`\nConnecting to port ${portPath}...`);
    
    const configSerialObj = {
        path: portPath,
        baudRate,
        autoOpen: false,
    };

    const port = new SerialPort(configSerialObj);
    port.open();
    console.log(`Port ${portPath} connected successfully`);
    
    return port;
}
