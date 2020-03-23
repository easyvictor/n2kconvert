#!/bin/bash
OUTPUT_FIFO="/dev/n2kconvert"
FORWARD_FIFO="/dev/n2kforward"
TIME_FIFO="/dev/n2ktime"
doquit() {
   echo "Removing FIFOs $OUTPUT_FIFO, $FORWARD_FIFO, $TIME_FIFO; then quitting."
   rm "$OUTPUT_FIFO"
   rm "$FORWARD_FIFO"
   rm "$TIME_FIFO"
   exit 0
}

trap 'doquit' QUIT
trap 'doquit' INT
trap 'doquit' TERM

echo "Creating fifo: $OUTPUT_FIFO"
mkfifo "$OUTPUT_FIFO"
echo "Creating fifo: $FORWARD_FIFO"
mkfifo "$FORWARD_FIFO"
echo "Creating fifo: $TIME_FIFO"
mkfifo "$TIME_FIFO"
systemd-notify --ready
while true ; do
  sleep 30
done
