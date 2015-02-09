import serial
import sys
import argparse

def adc_val(voltage): return int(voltage * 1024) // 5
default_cutoff = adc_val(0.837)
bin_min = adc_val(0.348)
bin_max = adc_val(0.629)
dry_run = False
def gen_cmd(which, n, val):
    v1 = which << 7
    v1 |= (n & 0b11111) << 2
    v1 |= val >> 8
    v2 = val & 0xff
    return bytes([v1, v2])
def print_bin(f):
    print('{:08b} {:08b}'.format(f[0], f[1]))

def send_cmd(which, n, val):
    f = print_bin if dry_run else ser.write
    f(gen_cmd(which, n, val))
#    print(ser.readline())
def set_min(n, val):
    send_cmd(0, n, val)
def set_max(n, val):
    send_cmd(1, n, val)
def set_both(n, min=0, max=1023):
    set_min(n, min)
    set_max(n, max)

parser = argparse.ArgumentParser(description='Store min and max sensor values in arduino eeprom')
parser.add_argument('--defaults|-d', dest='defaults', action='store_true', help='Restore defaults')
parser.add_argument('--sensor|-s', dest='sensor', type=int, help='Sensor number')
parser.add_argument('--min', dest='min', type=int, help='Minimum value')
parser.add_argument('--max', dest='max', type=int, help='Maximum value')
parser.add_argument('--port', dest='port', default='/dev/ttyACM0', help='Serial port to use')
parser.add_argument('--baud', dest='baud', default=115200, type=int, help='Baud rate to use')
parser.add_argument('--dry', dest='dry', action='store_true', help='Just print bytes that would have been sent')
args = parser.parse_args()
print(args.dry)
dry_run = args.dry

ser = serial.Serial(args.port, args.baud)
if args.defaults:
    if args.min is not None or args.max is not None:
        print('--defaults is exclusive with --min and --max', file=sys.stderr)
        sys.exit(1)
    for i in range(5):
#        ser.read()
        set_both(i, default_cutoff, 1023)
    set_both(5, bin_min, bin_max)
else:
    if args.min is not None:
        set_min(args.sensor, args.min)
    if args.max is not None:
        set_max(args.sensor, args.max)
