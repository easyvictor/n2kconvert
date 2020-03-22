/*
n2kconvert
Converts NMEA2000 data to NMEA0183 sentences
 
Reads some messages from NMEA2000 and converts them to NMEA0183
sentence format, which is then output either to stdout or a named pipe.

This code is being used on a Raspberry Pi 3, with a PiCAN2 for CAN.
It is generally combined with the program kplex to mux with 
other NMEA0183 streams.

To use this example you need install also:

- NMEA2000 library
- NMEA_socketCAN library
- NMEA0183 library


Copyright (c) 2018 Jonathan Lovegren

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#include <NMEA2000_SocketCAN.h>
#include <NMEA0183LinuxStream.h>
#include "N2kDataToNMEA0183.h"
#include "BoardSerialNumber.h"
#include "Options.h"
#include <iostream>
#include <csignal>
#include <chrono>
#include <thread>

// Reading serial number depends of used board. BoardSerialNumber module
// has methods for RPi, Arduino DUE and Teensy. For others function returns
// 0 and then DefaultSerialNumber will be used.
static const uint32_t DefaultSerialNumber = 999999;

// Set the information for other bus devices, which messages we support
static const unsigned long TransmitMessages[] = {0};
static const unsigned long ReceiveMessages[] = {
  127250, // Heading
  127258, // Magnetic Variation
  128259, // Boat Speed
  128267, // Depth
  129025, // Lat/Lon rapid
  129026, // COG SOG rapid
  129029, // GNSS Data
  130306, // Wind
  0
};

// Flag for stopping program
static bool run_program = true;

// For cout and cerr
using namespace std;

// ******** Setup ********
// Configures the input and output streams,
// and data conversion.
bool Setup( tNMEA2000& NMEA2000,
            tNMEA0183* pNMEA0183AuxIn,
            tNMEA0183& NMEA0183Out,
            tN2kDataToNMEA0183& N2kDataToNMEA0183,
            tSocketStream* pForwardStream) {
  bool status = false;
  // Setup NMEA2000 system
  char SnoStr[33];
  uint32_t SerialNumber = GetBoardSerialNumber();
  if (SerialNumber == 0) SerialNumber = DefaultSerialNumber;
  snprintf(SnoStr,32,"%lu",(long unsigned int)SerialNumber);
  NMEA2000.SetProductInformation(SnoStr, // Manufacturer's Model serial code
                                 120, // Manufacturer's product code
                                 "n2kconvert",  // Manufacturer's Model ID
                                 "1.0.0.1 (2018-12-30)",  // Manufacturer's Software version code
                                 "1.0.0.0 (2018-12-30)" // Manufacturer's Model version
                                 );
  // Set device information
  NMEA2000.SetDeviceInformation(SerialNumber, // Unique number. Use e.g. Serial number.
                                130, // Device function=PC Gateway. See codes on http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20%26%20function%20codes%20v%202.00.pdf
                                25, // Device class=Inter/Intranetwork Device. See codes on http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20%26%20function%20codes%20v%202.00.pdf
                                2046 // Just choosen free from code list on http://www.nmea.org/Assets/20121020%20nmea%202000%20registration%20list.pdf
                               );
  // Forward settings
  if (pForwardStream) {
    NMEA2000.SetForwardStream(pForwardStream);
    //NMEA2000.SetForwardType(tNMEA2000::fwdt_Text); // Show in clear text. Leave uncommented for default Actisense format.
    NMEA2000.EnableForward(true);
  } else {
    NMEA2000.EnableForward(false);
  }
  // Mode
  NMEA2000.SetMode(tNMEA2000::N2km_ListenAndNode,25);
  // Message settings
  NMEA2000.ExtendTransmitMessages(TransmitMessages);
  NMEA2000.ExtendReceiveMessages(ReceiveMessages);
  NMEA2000.AttachMsgHandler(&N2kDataToNMEA0183);
  // Open ports
  // Open NMEA2000 CAN
  status = NMEA2000.Open();
  if (!status) {
    cerr << "Problem opening NMEA2000 port.\n";
    return false;
  }
  // Open NMEA0183 aux input (optional)
  if (pNMEA0183AuxIn) {
    status = pNMEA0183AuxIn->Open();
    if (!status) {
      cerr << "Problem opening Auxiliary NMEA0183 input port.\n";
      return false;
    }
  }
  // Open NMEA013 output
  status = NMEA0183Out.Open();
  if (!status) {
    cerr << "Problem opening NMEA0183Out port.\n";
    return false;
  }
  // Setup was successful
  return true;
}

// ******** WaitForEvent ********
// This is preliminary definition. Polls periodically.
auto sched_time = chrono::steady_clock::now();
void WaitForEvent() {
  sched_time += chrono::milliseconds(50);
  this_thread::sleep_until(sched_time);
}

// ******** HandleSignal ********
// Signal called when kill signal received
void HandleSignal(int signal) {
  cout << "Received signal " << signal << ".\n";
  run_program = false;
}

// ******** Main Program ********
int main(int argc, char* argv[]) {
  // Setup signal handler
  signal(SIGINT, HandleSignal);
  signal(SIGTERM, HandleSignal);
  // Parse arguments from cmd line annd oad config file
  string config_file, can_port, aux_in_serial, aux_in_baud, out_stream, fwd_stream;
  bool debug_mode = false;
  bool status_ok = false;
  status_ok = SetOptions(argc, argv, // inputs
    &config_file, &can_port, &aux_in_serial, &aux_in_baud, &out_stream, &fwd_stream, &debug_mode); // outputs
  if (!status_ok) {
    cerr << "Problem loading options. Exiting.\n";
    return 3;
  }
  // Create parsing objects
  tNMEA2000_SocketCAN NMEA2000((char*)can_port.c_str());
  tNMEA0183LinuxStream NMEA0183OutStream(out_stream.c_str());
  tNMEA0183 NMEA0183Out(&NMEA0183OutStream);
  // Optional aux input stream
  tNMEA0183LinuxStream *pNMEA0183AuxInStream = NULL;
  tNMEA0183 *pNMEA0183AuxIn = NULL;
  if (!aux_in_serial.empty() && !aux_in_baud.empty()) {
    pNMEA0183AuxInStream = new tNMEA0183LinuxStream(aux_in_serial.c_str(), atoi(aux_in_baud.c_str()));
    pNMEA0183AuxIn = new tNMEA0183(pNMEA0183AuxInStream);
  }
  tN2kDataToNMEA0183 N2kDataToNMEA0183(&NMEA2000, pNMEA0183AuxIn, &NMEA0183Out);
  // Optional forward stream
  tSocketStream *pForwardStream = NULL;
  if (!fwd_stream.empty()) {
    pForwardStream = new tSocketStream(fwd_stream.c_str());
  }
  // Setup parsing objects
  status_ok = Setup(NMEA2000, pNMEA0183AuxIn, NMEA0183Out, N2kDataToNMEA0183, pForwardStream);
  if (!status_ok) {
    cerr << "Problem during Setup. Exiting.\n";
    return 3;
  }
  // Set current time for measurements
  sched_time = chrono::steady_clock::now();
  // Debug time vars
  auto debug_time = sched_time;
  auto start_parse_time = sched_time;
  
  // **** Main Program Loop ****
  cout << "Running!\n";
  while (run_program) {
    // Wait until trigger to parse/send
    WaitForEvent();
    // Debug timing
    if (debug_mode) {
      start_parse_time = chrono::steady_clock::now();
    }
    // Parse NMEA2000 and send NMEA0183Out
    NMEA2000.ParseMessages();
    if (pNMEA0183AuxIn) {
      pNMEA0183AuxIn->ParseMessages();
    }
    N2kDataToNMEA0183.Update();
    // Debug timing and prints
    if (debug_mode) {
      auto time_now = chrono::steady_clock::now();
      auto parse_time = chrono::duration_cast<chrono::milliseconds>(time_now - start_parse_time).count();
      auto loop_time = chrono::duration_cast<chrono::milliseconds>(time_now - debug_time).count();
      float usage = float(parse_time)/float(loop_time)*100.0f;
      cout << "Parsed messages. "
        << "Parse: " << parse_time << "ms; "
        << "Loop: " << loop_time << "ms; "
        << "Usage: " << usage << "%\n";
      debug_time = time_now;
    }
  }
  cout << "Exiting.\n";
  return 0;
}