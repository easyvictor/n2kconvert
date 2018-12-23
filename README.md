# n2kconvert
Converts NMEA2000 data to NMEA0183 sentences
 
Reads some messages from NMEA2000 and converts them to NMEA0183
sentence format, which is then output either to stdout or a named pipe.

This code is being used on a Raspberry Pi 3, with a PiCAN2 for CAN.
It is generally combined with the program kplex to mux with 
other NMEA0183 streams.

To use this example you need install also:

- NMEA2000 library
- NMEA_sfocketCAN library
- NMEA0183 library
