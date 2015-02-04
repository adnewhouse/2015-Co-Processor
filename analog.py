import serial

bb = lambda x: int(bool(x))
ser = serial.Serial('/dev/ttyACM0', 115200)
while True:
    #ser.write(b'\xff')
    b = ser.read(1)
    n = b[0]
    checksum, a5, a4, a3, a2, a1, a0 = bool(n & 0x01), bool(n & 0x02), bool(n & 0x04), bool(n & 0x08), bool(n & 0x10), bool(n & 0x20), bool(n & 0x40)
    if checksum != (a0 ^ a1 ^ a2 ^ a3 ^ a4 ^ a5):
        print('Bad checksum')
        ser.write(b'\xff')
        continue
    
    print('tote1 {} tote2 {} tote3 {} tote4 {} tote5 {} bin {}'.format(int(a0), int(a1), int(a2), int(a3), int(a4), int(a5)))
