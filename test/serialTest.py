import argparse
import serial
import time
parser = argparse.ArgumentParser()
parser.add_argument("-f", "--file", help="Input file.")
parser.add_argument("-s", "--serial", help="Serial port.")
parser.add_argument("-b", "--baud", help="Baud rate.", default=4800)
args = parser.parse_args()
print(f"Reading file '{args.file}', sending to {args.serial} @ {args.baud}")

ser = serial.Serial(args.serial, args.baud)
with open(args.file,"r") as f:
	count = 1
	for line in f:
		linennl = line.rstrip()
		print(f"{count}>{linennl}")
		ser.write(linennl.encode("utf-8"))
		ser.write("\r\n".encode("utf-8"))
		count += 1
		time.sleep(0.25)

