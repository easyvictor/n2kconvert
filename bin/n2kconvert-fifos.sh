#!/bin/bash
FIFO_NAME="/dev/n2kconvert"

doquit() {
   echo "Removing FIFO $FIFO_NAME and quitting."
   rm "$FIFO_NAME"
   exit 0
}

trap 'doquit' QUIT
trap 'doquit' INT
trap 'doquit' TERM

echo "Creating fifo: $FIFO_NAME"
mkfifo "$FIFO_NAME"
systemd-notify --ready
while true ; do
  sleep 30
done
