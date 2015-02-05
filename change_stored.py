import serial
import sys

n, which, val = int(sys.argv[1]), 1 if sys.argv[2] == 'max' else 0, int(sys.argv[3])
ser = serial.Serial('/dev/ttyACM0', 115200)
v1 = which << 7
v1 |= (n & 0b11111) << 2
v1 |= val >> 8
v2 = val & 0xff
print('{:08b}{:08b}'.format(v1, v2))
ser.write(bytes([v1, v2]))
