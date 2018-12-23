/*
 NMEA0183/NMEA2000 library. NMEA2000 -> NMEA0183
   Reads some messages from NMEA2000 and converts them to NMEA0183
   format to NMEA0183_out (Serial on Arduino or /dev/tnt0 on RPi).
   Also forwards all NMEA2000 bus messages in Actisense format.

   The example is designed for sending N2k data to OpenCPN on RPi with PiCAN2.
   It can be used also on PC with some Arduino/Teensy board to provide
   NMEA0183 (and also NMEA2000) data to PC. Example has been tested on
   RPi3B, ESP32, Arduino DUE, Arduino Mega and Teensy.

 To use this example you need install also:
   - NMEA2000 library
   - NMEA0183 library
   - Related CAN libraries.

 The example works with default settings on Arduino/Teensy boards and on
 Rasberry Pi 3 with PiCAN2 shield.

 On Rasberry Pi you need also Code Block environment and tty0tty (virtual
 null modem cable)
*/

#include <NMEA2000_SocketCAN.h>
#include "N2kDataToNMEA0183.h"
#include "BoardSerialNumber.h"
#include <iostream>
#include <unistd.h>
#include "NMEA0183LinuxStream.h"

// Reading serial number depends of used board. BoardSerialNumber module
// has methods for RPi, Arduino DUE and Teensy. For others function returns
// 0 and then DefaultSerialNumber will be used.
#define DefaultSerialNumber 999999
uint32_t GetSerialNumber() {
  uint32_t Sno=GetBoardSerialNumber();
  return ( Sno!=0?Sno:DefaultSerialNumber );
}

// Set the information for other bus devices, which messages we support
const unsigned long TransmitMessages[] PROGMEM={0};
const unsigned long ReceiveMessages[] PROGMEM={/*126992L,*/127250L,127258L,128259UL,128267UL,129025UL,129026L,129029L,0};

// *****************************************************************************
void setup( tNMEA2000& NMEA2000,
            tNMEA0183LinuxStream& NMEA0183OutStream,
            tNMEA0183& NMEA0183_Out,
            tN2kDataToNMEA0183& N2kDataToNMEA0183,
            tSocketStream& ForwardStream) {
  // Setup NMEA2000 system

  char SnoStr[33];
  uint32_t SerialNumber=GetSerialNumber();
  snprintf(SnoStr,32,"%lu",(long unsigned int)SerialNumber);

  NMEA2000.SetProductInformation(SnoStr, // Manufacturer's Model serial code
                                 120, // Manufacturer's product code
                                 "n2kconvert",  // Manufacturer's Model ID
                                 "1.0.0.1 (2018-04-03)",  // Manufacturer's Software version code
                                 "1.0.0.0 (2018-04-03)" // Manufacturer's Model version
                                 );
  // Det device information
  NMEA2000.SetDeviceInformation(SerialNumber, // Unique number. Use e.g. Serial number.
                                130, // Device function=PC Gateway. See codes on http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20%26%20function%20codes%20v%202.00.pdf
                                25, // Device class=Inter/Intranetwork Device. See codes on http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20%26%20function%20codes%20v%202.00.pdf
                                2046 // Just choosen free from code list on http://www.nmea.org/Assets/20121020%20nmea%202000%20registration%20list.pdf
                               );
  NMEA2000.SetForwardStream(&ForwardStream);
  NMEA2000.SetForwardType(tNMEA2000::fwdt_Text); // Show in clear text. Leave uncommented for default Actisense format.
  NMEA2000.SetMode(tNMEA2000::N2km_ListenAndNode,25);
  //NMEA2000.EnableForward(false);

  NMEA2000.ExtendTransmitMessages(TransmitMessages);
  NMEA2000.ExtendReceiveMessages(ReceiveMessages);
  NMEA2000.AttachMsgHandler(&N2kDataToNMEA0183);

  NMEA2000.Open();

  // Setup NMEA0183 ports and handlers
  NMEA0183_Out.SetMessageStream(&NMEA0183OutStream);
  NMEA0183_Out.Open();
}

// *****************************************************************************
// This is preliminary definition. For RPi we need to build some
// event system to minimize cpu usage.
void WaitForEvent() {
  usleep(100);
}

// *****************************************************************************
int main(int argc, char** argv) {
  // Parse arguments
  tNMEA2000_SocketCAN NMEA2000((char*)"vcan0");
  tNMEA0183LinuxStream NMEA0183OutStream("/dev/n2kout");
  tSocketStream ForwardStream;
  tNMEA0183 NMEA0183_Out;
  tN2kDataToNMEA0183 N2kDataToNMEA0183(&NMEA2000, &NMEA0183_Out);
  setup(NMEA2000, NMEA0183OutStream, NMEA0183_Out, N2kDataToNMEA0183, ForwardStream);
  std::cout << "Running!" << std::endl;
  while ( true ) {
    WaitForEvent();
    NMEA2000.ParseMessages();
    N2kDataToNMEA0183.Update();
  }

  return 0;
}
