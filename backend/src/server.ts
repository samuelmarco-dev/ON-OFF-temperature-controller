import express, { Request, Response } from "express";
import cors from "cors";
import { SerialPort } from "serialport";
import { AutoDetectTypes } from "@serialport/bindings-cpp";

import { serialPortMethod } from "./middlewares/index.js";
import Tempeture from "./interfaces/index.js";

const port = 3000;
const baudRate = 19200;

const app = express();
app.use(express.json());
app.use(cors());

const latestData: Array<Tempeture> = [];
let portSerial: SerialPort<AutoDetectTypes>;

async function initializeSerialCommunication() {
    console.log("Establishing serial communication");
    const availablePorts = await serialPortMethod.getAvailablePorts();

    if (!availablePorts.length) {
        console.log("No serial ports available");
        process.exit(1);
    } else {
        const portPath = availablePorts[0];
        portSerial = await serialPortMethod.connectToPort(portPath,baudRate);

        try {
            await serialPortMethod.readData(portSerial, latestData);
        } catch (error) {
            console.log("Error receiving data:", error);
        }
    }
}
initializeSerialCommunication().catch(console.error);

app.get("/data", (req: Request, res: Response) => {
    res.status(200).send(latestData[latestData.length - 1]);
});

app.listen(port, () => console.log(`API running in http://localhost:${port}`));
