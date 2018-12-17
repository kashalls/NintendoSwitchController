import serial

ser = serial.Serial("COM6", 57600, timeout = 0)
ser.write(bytearray([128, 128, 128, 128, '0b00010000']))

ser.read()
ser.close()