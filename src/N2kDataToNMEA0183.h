/*
N2kDataToNMEA0183.h

Copyright (c) 2015-2018 Timo Lappalainen, Kave Oy, www.kave.fi

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <NMEA0183.h>
#include <NMEA2000.h>

//------------------------------------------------------------------------------
class tN2kDataToNMEA0183 : public tNMEA2000::tMsgHandler, public tNMEA0183::tMsgHandler {
public:
  using tSendNMEA0183MessageCallback=void (*)(const tNMEA0183Msg &NMEA0183Msg);
    
protected:
  static const unsigned long RMCPeriod=1000;
  double Latitude;
  double Longitude;
  double Altitude;
  double Variation;
  double Deviation;
  double HeadingMagSensor;
  double HeadingMagnetic;
  double HeadingTrue;
  double HeadingTrueSensor;
  double COG;
  double SOG;
  double WindSpeedApp;
  double WindAngleApp;
  double WindSpeedTrue;
  double WindDirTrue;
  unsigned long LastHeadingMagSensorTime;
  unsigned long LastHeadingTrueSensorTime;
  unsigned long LastCOGSOGTime;
  unsigned long LastPositionTime;
  unsigned long LastPosSend;
  unsigned long LastWindTime;
  uint16_t DaysSince1970;
  double SecondsSinceMidnight;
  unsigned long NextRMCSend;

  tNMEA0183 *pNMEA0183Out;

  tSendNMEA0183MessageCallback SendNMEA0183MessageCallback;

protected:
  // NMEA2000 message handlers
  void HandleHeading(const tN2kMsg &N2kMsg); // 127250
  void HandleVariation(const tN2kMsg &N2kMsg); // 127258
  void HandleBoatSpeed(const tN2kMsg &N2kMsg); // 128259
  void HandleDepth(const tN2kMsg &N2kMsg); // 128267
  void HandlePosition(const tN2kMsg &N2kMsg); // 129025
  void HandleCOGSOG(const tN2kMsg &N2kMsg); // 129026
  void HandleGNSS(const tN2kMsg &N2kMsg); // 129029
  void HandleWind(const tN2kMsg &N2kMsg); // 130306
  void HandleEnvParams(const tN2kMsg &N2kMsg); // 130311
  // NMEA0183 message handlers (for aux input)
  void HandleHeadingNMEA0183(const tNMEA0183Msg &NMEA0183Msg); // HDG
  // Message senders
  void SetNextRMCSend() { NextRMCSend=millis()+RMCPeriod; }
  void SendRMC();
  void SendMessage(const tNMEA0183Msg &NMEA0183Msg);

  // Utilities
  void UpdateHeadingsNewMagnetic();
  void UpdateHeadingsNewTrue();
  float WrapAngle(float angle);
  void CalcTrueWind();
  
public:
  tN2kDataToNMEA0183(tNMEA2000 *_pNMEA2000, tNMEA0183 *_pNMEA0183AuxIn, tNMEA0183 *_pNMEA0183Out)
    : tNMEA2000::tMsgHandler(0,_pNMEA2000), 
      tNMEA0183::tMsgHandler(_pNMEA0183AuxIn) {
    SendNMEA0183MessageCallback=0;
    pNMEA0183Out=_pNMEA0183Out;
    Latitude=N2kDoubleNA; Longitude=N2kDoubleNA; Altitude=N2kDoubleNA;
    Variation=N2kDoubleNA; Deviation=N2kDoubleNA; 
    HeadingMagSensor=N2kDoubleNA; HeadingMagnetic=N2kDoubleNA;
    HeadingTrueSensor=N2kDoubleNA; HeadingTrue=N2kDoubleNA;
    WindAngleApp=N2kDoubleNA; WindSpeedApp=N2kDoubleNA;
    WindDirTrue=N2kDoubleNA; WindSpeedTrue=N2kDoubleNA;
    COG=N2kDoubleNA; SOG=N2kDoubleNA;
    SecondsSinceMidnight=N2kDoubleNA; DaysSince1970=N2kUInt16NA;
    LastPosSend=0;
    NextRMCSend=millis()+RMCPeriod;
    LastHeadingMagSensorTime=0;
    LastHeadingTrueSensorTime=0;
    LastCOGSOGTime=0;
    LastPositionTime=0;
    LastWindTime=0;
  }
  void HandleMsg(const tN2kMsg &N2kMsg);
  void HandleMsg(const tNMEA0183Msg &NMEA0183Msg);
  void SetSendNMEA0183MessageCallback(tSendNMEA0183MessageCallback _SendNMEA0183MessageCallback) {
    SendNMEA0183MessageCallback=_SendNMEA0183MessageCallback;
  }
  void Update();
};

