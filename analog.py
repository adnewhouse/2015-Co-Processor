import serial

bb = lambda x: int(bool(x))
ser = serial.Serial('/dev/ttyACM0', 115200)
while True:
    #ser.write(b'\xff')
    b = ser.read(1)
    n = b[0]
    a0, a1, a2, a3, a4, a5, checksum = bool(n & 0x01), bool(n & 0x02), bool(n & 0x04), bool(n & 0x08), bool(n & 0x10), bool(n & 0x20), bool(n & 0x40)
    if checksum != (a0 ^ a1 ^ a2 ^ a3 ^ a4 ^ a5):
        ser.write(b'\xff')
        continue
    
    print('A0 {} A1 {} A2 {} A3 {} A4 {} A5 {}'.format(int(a0), int(a1), int(a2), int(a3), int(a4), int(a5)))
