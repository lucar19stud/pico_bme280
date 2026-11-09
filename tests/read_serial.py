import serial
import argparse

parser = argparse.ArgumentParser(description="Read BME280 measurements from the Pico")
parser.add_argument("port", help="serial port, e.g. /dev/tty.usbmodem144301")
args = parser.parse_args()
meas = []

def split_line(line):
    strp = line.split()
    result = {"type": strp[0]}
    for i in strp[1:]:
        k, sep, v = i.partition("=")
        if sep == "":
            continue
        result[k] = int(v, 0)
    return result

with serial.Serial(args.port, 115200, timeout=2) as ser:
    for _ in range(10):
        raw = ser.readline()
        line = raw.decode("ascii", errors="replace").strip()

        strp = line.split()
        if not strp:
            continue
        if strp[0] not in ("MEAS", "ERROR"):
            continue

        meas.append(split_line(line))


for m in meas:
    print(m)