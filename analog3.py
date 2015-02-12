import serial

ser = serial.Serial('/dev/ttyACM0', 115200)

while True:
   l = ser.readline().rstrip()
   i = int(l)
   if i == 0: i = 1
   v = (i / 1023) * 5
   d = 10.68 * v ** -0.94
   print(d)
   q = '{:.2f}'.format(d)
   print('\x1b[0G\x1b[7m' + q + '\x1b[0m' + '█' * min(int(d * 2) - len(q), 200) + '\x1b[0K', end='')
