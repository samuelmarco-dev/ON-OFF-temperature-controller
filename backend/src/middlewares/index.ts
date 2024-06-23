import { getAvailablePorts } from "./getAvaliablePorts.js";
import { connectToPort } from "./connectToPort.js";
import { readData } from "./readData.js";

export const serialPortMethod = {
    getAvailablePorts,
    connectToPort,
    readData,
};
