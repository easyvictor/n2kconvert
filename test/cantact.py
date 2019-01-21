from canard import can
from canard.hw import cantact
import sys
import argparse
import time
import re

parser = argparse.ArgumentParser()
parser.add_argument("-f", "--file", help="Input file.")
parser.add_argument("-s", "--serial", help="Serial port.")
parser.add_argument("-b", "--baud", help="Baud rate.", default=250000)
args = parser.parse_args()
print(f"Reading file '{args.file}', sending to {args.serial} @ {args.baud}")

dev = cantact.CantactDev(args.serial)
dev.set_bitrate(250000)
dev.start()
with open(args.file,"r") as f:
	count = 1
	for line in f:
		linennl = line.rstrip()
		lineparts = linennl.split(' ')
		lineid = int(re.search(r'<(.*)>', lineparts[0]).group(1), 0)
		linedata = [int(i,16) for i in lineparts[2:]]
		print(f"{count}>{linennl}")
		frame = can.Frame(id=lineid, dlc=len(linedata), data=linedata, is_extended_id=True)
		dev.send(frame)
		print("%d: %s" % (count, str(frame)))
		time.sleep(0.05)
		count += 1