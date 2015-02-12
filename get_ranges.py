import serial
import argparse
import struct

parser = argparse.ArgumentParser(description='Get min and max values stored in arduino EEPROM')
parser.add_argument('--port', dest='port', default='/dev/ttyACM0', help='Serial port to use')
parser.add_argument('--baud', dest='baud', default=115200, type=int, help='Baud rate to use')
args = parser.parse_args()

ser = serial.Serial(args.port, args.baud)
ser.flushInput()
ser.write(b'\xfe')
vals = ser.read(24)
z = struct.unpack('<HHHHHHHHHHHH', vals)
for i in range(0, 12, 2):
  print(i//2, z[i], z[i+1])
