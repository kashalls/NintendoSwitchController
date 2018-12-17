const SerialPort = require('serialport');

const SwitchInterface = '/dev/ttyS2';
const baudRate = 57600;

const Switch = new SerialPort(SwitchInterface, { baudRate });

Switch.write(Buffer.from([128, 128, 128, 128, '0b00010000']));
Switch.read();

process.exit(0)