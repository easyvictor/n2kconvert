/*
N2kDataToNMEA0183.cpp

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

#include "N2kDataToNMEA0183.h"
#include <N2kMessages.h>
#include <NMEA0183Messages.h>
#include <math.h>

const double radToDeg=180.0/M_PI;
const double mToFeet=3.2808398950131;

//*****************************************************************************
// Handle incoming NMEA2000 messages
void tN2kDataToNMEA0183::HandleMsg(const tN2kMsg &N2kMsg) {
  switch (N2kMsg.PGN) {
    case 127250UL: HandleHeading(N2kMsg);
    case 127258UL: HandleVariation(N2kMsg);
    case 128259UL: HandleBoatSpeed(N2kMsg);
    case 128267UL: HandleDepth(N2kMsg);
    case 129025UL: HandlePosition(N2kMsg);
    case 129026UL: HandleCOGSOG(N2kMsg);
    case 129029UL: HandleGNSS(N2kMsg);
    case 130306UL: HandleWind(N2kMsg);
    case 130311UL: HandleEnvParams(N2kMsg);
  }
}

//*****************************************************************************
// Handle incoming NMEA0183 messages on the aux input port
void tN2kDataToNMEA0183::HandleMsg(const tNMEA0183Msg &NMEA0183Msg) {
  // Call all handlers here by checking message code
  if (NMEA0183Msg.IsMessageCode("HDG")) {
    HandleHeadingNMEA0183(NMEA0183Msg);
  }
}

//*****************************************************************************
void tN2kDataToNMEA0183::Update() {
  SendRMC();
  if (LastHeadingMagSensorTime+2000 < millis()) { 
    HeadingMagSensor = N2kDoubleNA;
    UpdateHeadingsNewMagnetic(); // Update dependent variables accordingly
  }
  if (LastHeadingTrueSensorTime+2000 < millis()) { 
    HeadingTrueSensor = N2kDoubleNA; 
    UpdateHeadingsNewTrue(); // Update dependent variables accordingly
  }
  if (LastCOGSOGTime+2000<millis()) { COG=N2kDoubleNA; SOG=N2kDoubleNA; }
  if (LastPositionTime+4000<millis()) { Latitude=N2kDoubleNA; Longitude=N2kDoubleNA; }
  if (LastWindTime+2000<millis()) {
    WindSpeedApp = N2kDoubleNA;
    WindAngleApp = N2kDoubleNA;
    WindSpeedTrue = N2kDoubleNA;
    WindDirTrue = N2kDoubleNA;
  }
}

//*****************************************************************************
void tN2kDataToNMEA0183::SendMessage(const tNMEA0183Msg &NMEA0183Msg) {
  if ( pNMEA0183Out!=0 ) pNMEA0183Out->SendMessage(NMEA0183Msg);
  if ( SendNMEA0183MessageCallback!=0 ) SendNMEA0183MessageCallback(NMEA0183Msg);
}

//*****************************************************************************
void tN2kDataToNMEA0183::HandleHeading(const tN2kMsg &N2kMsg) {
unsigned char SID;
tN2kHeadingReference ref;
double _Deviation, _Variation, _Heading;

  if (ParseN2kHeading(N2kMsg, SID, _Heading, _Deviation, _Variation, ref)) {
    if (ref == N2khr_magnetic) {
      if (!N2kIsNA(_Heading)) HeadingMagSensor = _Heading; // Update magnetic sensor heading
      if (!N2kIsNA(_Variation)) Variation = _Variation; // Update Variation
      if (!N2kIsNA(_Deviation)) Deviation = _Deviation; // Update Deviation
      UpdateHeadingsNewMagnetic();
      // Send HDG message
      tNMEA0183Msg NMEA0183MsgHDG;
      if (NMEA0183SetHDG(NMEA0183MsgHDG, HeadingMagSensor, Deviation, Variation)) {
        SendMessage(NMEA0183MsgHDG);
      }
      // Send HDT as well if we have the right data
      if (!N2kIsNA(HeadingTrue)) {
        tNMEA0183Msg NMEA0183MsgHDT;
        if (NMEA0183SetHDT(NMEA0183MsgHDT, HeadingTrue)) {
          SendMessage(NMEA0183MsgHDT);
        }
      }
    } else if (ref == N2khr_true) {
      if (!N2kIsNA(_Heading)) HeadingTrueSensor = _Heading; // Update true heading
      UpdateHeadingsNewTrue();
      // Send HDT message
      tNMEA0183Msg NMEA0183MsgHDT;
      if (NMEA0183SetHDT(NMEA0183MsgHDT, HeadingTrue)) {
        SendMessage(NMEA0183MsgHDT);
      }
    }
  }
}

//*****************************************************************************
// Handle incoming HDG
void tN2kDataToNMEA0183::HandleHeadingNMEA0183(const tNMEA0183Msg &NMEA0183Msg) {
  double _Heading, _Deviation, _Variation;
  if (NMEA0183ParseHDG_nc(NMEA0183Msg, _Heading, _Deviation, _Variation)) {
    if (!NMEA0183IsNA(_Heading)) HeadingMagSensor=_Heading; // Update magnetic sensor heading
    if (!NMEA0183IsNA(_Variation)) Variation=_Variation; // Update Variation
    if (!NMEA0183IsNA(_Deviation)) Deviation=_Deviation; // Update Deviation
    UpdateHeadingsNewMagnetic();
    // Send HDG message
    tNMEA0183Msg NMEA0183MsgHDG;
    if (NMEA0183SetHDG(NMEA0183MsgHDG, HeadingMagSensor, Deviation, Variation)) {
      SendMessage(NMEA0183MsgHDG);
    }
    // Send HDT as well if we have the right data
    if (!N2kIsNA(HeadingTrue)) {
      tNMEA0183Msg NMEA0183MsgHDT;
      if (NMEA0183SetHDT(NMEA0183MsgHDT, HeadingTrue)) {
        SendMessage(NMEA0183MsgHDT);
      }
    }
  }
}

void tN2kDataToNMEA0183::UpdateHeadingsNewMagnetic() {
  // This function is called with the magnetic sensor heading is updated and
  // it is the *controlling* source, so the others are calculated from it.
  // Updates include timeout expirations (see 2 sec timeout above)
  if (!N2kIsNA(HeadingMagSensor) && !NMEA0183IsNA(HeadingMagSensor)) {
    // We have a valid Mag reading. Good.
    // Start with our magnetic sensor reading, assuming no deviation
    HeadingMagnetic = HeadingMagSensor;
    // If we have deviation, add it to the sensor reading to correct it
    if (!N2kIsNA(Deviation) && !NMEA0183IsNA(Deviation)) {
      HeadingMagnetic = WrapAngle(HeadingMagnetic + Deviation);
    }
    // If we have variation, add it to the mag heading to get true
    if (!N2kIsNA(Variation) && !NMEA0183IsNA(Variation)) { 
      HeadingTrue = WrapAngle(HeadingMagnetic + Variation);
    }
    LastHeadingMagSensorTime = millis();
  } else {
    // Must have an expired mag sensor reading. This creates a few problems.
    // 1) HeadingMagnetic can be retrieved from the mag sensor, or true sensor minus variation.
    //    So if true or variation are also gone, we have no HeadingMagnetic
    // 2) HeadingTrue can be retrieved from the true sensor, or mag sensor plus variation.
    //    So if true sensor is also gone, we have no HeadingTrue
    if (N2kIsNA(HeadingTrueSensor) || NMEA0183IsNA(HeadingTrueSensor)) {
      // Case 2)
      HeadingTrue = N2kDoubleNA;
    } else if (N2kIsNA(Variation) || NMEA0183IsNA(Variation)) {
      // Case 1)
      HeadingMagnetic = N2kDoubleNA;
    }
  }
}

void tN2kDataToNMEA0183::UpdateHeadingsNewTrue() {
  // This function is called with the true heading is updated and
  // it is the *controlling* source, so the mag heading is calculated from it.
  // In reality this is probably never used, unless you have a sensor 
  // which is specifically a GPS based true heading sensor.
  // Note, Deviation is not touched here as there is no point to "back calculate"
  // A magnetic sensor value. We only care about magnetic heading.
  if (!N2kIsNA(HeadingTrueSensor) && !NMEA0183IsNA(HeadingTrueSensor)) {
    // True heading is good. We can begin to use this.
    HeadingTrue = HeadingTrueSensor;
    // If we have variation, subtract it from the true heading to get magnetic    
    if (!N2kIsNA(Variation) && !NMEA0183IsNA(Variation)) { 
      HeadingMagnetic = WrapAngle(HeadingTrue - Variation);
    }
    LastHeadingTrueSensorTime = millis();
  } else {
    // Must have an expired true sensor reading. This creates a few problems.
    // 1) HeadingTrue can be retrieved from the true sensor, or mag sensor plus variation.
    //    So if mag or variation are also gone, we have no HeadingTrue
    // 2) HeadingMagnetic can be retrieved from the mag sensor, or true sensor minus variation.
    //    So if mag sensor is also gone, we have no HeadingMagnetic
    if (N2kIsNA(HeadingMagSensor) || NMEA0183IsNA(HeadingMagSensor)) {
      // Case 2)
      HeadingMagnetic = N2kDoubleNA;
    } else if (N2kIsNA(Variation) || NMEA0183IsNA(Variation)) {
      // Case 1)
      HeadingTrue = N2kDoubleNA;
    }
  }
}

//*****************************************************************************
void tN2kDataToNMEA0183::HandleVariation(const tN2kMsg &N2kMsg) {
  unsigned char SID;
  tN2kMagneticVariation Source;
  double _Variation;
  if (ParseN2kMagneticVariation(N2kMsg,SID,Source,DaysSince1970,_Variation)) {
    if (!N2kIsNA(_Variation)) Variation = _Variation; // Update Variation
  }
}

//*****************************************************************************
void tN2kDataToNMEA0183::HandleBoatSpeed(const tN2kMsg &N2kMsg) {
unsigned char SID;
double WaterReferenced;
double GroundReferenced;
tN2kSpeedWaterReferenceType SWRT;

  if ( ParseN2kBoatSpeed(N2kMsg,SID,WaterReferenced,GroundReferenced,SWRT) ) {
    tNMEA0183Msg NMEA0183Msg;
    if ( NMEA0183SetVHW(NMEA0183Msg,HeadingTrue,HeadingMagnetic,WaterReferenced) ) {
      SendMessage(NMEA0183Msg);
    }
  }
}

//*****************************************************************************
void tN2kDataToNMEA0183::HandleDepth(const tN2kMsg &N2kMsg) {
unsigned char SID;
double DepthBelowTransducer;
double Offset;
double Range;

  if ( ParseN2kWaterDepth(N2kMsg,SID,DepthBelowTransducer,Offset,Range) ) {
      tNMEA0183Msg NMEA0183Msg;
      // If user here has set a depth offset, apply it as well
      if (DepthOffset_ft != N2kDoubleNA)
        Offset += DepthOffset_ft / mToFeet;
      if ( NMEA0183SetDPT(NMEA0183Msg,DepthBelowTransducer,Offset) ) {
        SendMessage(NMEA0183Msg);
      } 
      if ( NMEA0183SetDBT(NMEA0183Msg,DepthBelowTransducer) ) {
        SendMessage(NMEA0183Msg);
      }
  }
}

//*****************************************************************************
void tN2kDataToNMEA0183::HandlePosition(const tN2kMsg &N2kMsg) {
  tNMEA0183Msg NMEA0183Msg;
  if ( ParseN2kPGN129025(N2kMsg, Latitude, Longitude) ) {
    if ( NMEA0183SetGLL(NMEA0183Msg, SecondsSinceMidnight, Latitude, Longitude) ) {
      SendMessage(NMEA0183Msg);
    }
    LastPositionTime=millis();
  }
}

//*****************************************************************************
void tN2kDataToNMEA0183::HandleCOGSOG(const tN2kMsg &N2kMsg) {
unsigned char SID;
tN2kHeadingReference HeadingReference;
tNMEA0183Msg NMEA0183Msg;

  if ( ParseN2kCOGSOGRapid(N2kMsg,SID,HeadingReference,COG,SOG) ) {
    LastCOGSOGTime=millis();
    double MCOG = (!N2kIsNA(COG) && !N2kIsNA(Variation))
      ? WrapAngle(COG - Variation)
      : NMEA0183DoubleNA;
    if ( HeadingReference==N2khr_magnetic ) {
      MCOG=COG;
      if ( !N2kIsNA(Variation) ) COG = WrapAngle(MCOG + Variation);
    }
    if ( NMEA0183SetVTG(NMEA0183Msg,COG,MCOG,SOG) ) {
      SendMessage(NMEA0183Msg);
    }
  }
}

//*****************************************************************************
void tN2kDataToNMEA0183::HandleGNSS(const tN2kMsg &N2kMsg) {
unsigned char SID;
tN2kGNSStype GNSStype;
tN2kGNSSmethod GNSSmethod;
unsigned char nSatellites;
double HDOP;
double PDOP;
double GeoidalSeparation;
unsigned char nReferenceStations;
tN2kGNSStype ReferenceStationType;
uint16_t ReferenceSationID;
double AgeOfCorrection;

  if ( ParseN2kGNSS(N2kMsg,SID,DaysSince1970,SecondsSinceMidnight,Latitude,Longitude,Altitude,GNSStype,GNSSmethod,
                    nSatellites,HDOP,PDOP,GeoidalSeparation,
                    nReferenceStations,ReferenceStationType,ReferenceSationID,AgeOfCorrection) ) {
    LastPositionTime=millis(); 
    // RMC will be sent as part of later update, once more data has arrived.
    // But we should send time message immediately.
    tNMEA0183Msg NMEA0183MsgZDA;
    if (NMEA0183SetZDA(NMEA0183MsgZDA, SecondsSinceMidnight, DaysSince1970)) {
      SendMessage(NMEA0183MsgZDA);
    }
  }
}

//*****************************************************************************
void tN2kDataToNMEA0183::HandleWind(const tN2kMsg &N2kMsg) {
  unsigned char SID;
  tN2kWindReference WindReference = N2kWind_Apparent;
  double WindSpeed = N2kDoubleNA;
  double WindAngle = N2kDoubleNA;
  if ( ParseN2kWindSpeed(N2kMsg,SID,WindSpeed,WindAngle,WindReference) ) {
    tNMEA0183Msg NMEA0183MsgMWV, NMEA0183MsgMWD;
    LastWindTime=millis();
    if ( WindReference==N2kWind_Apparent ) {
      // Only handle apparent wind for now
      WindAngleApp = WindAngle;
      WindSpeedApp = WindSpeed;
      if (NMEA0183SetMWV(NMEA0183MsgMWV,  WindAngleApp*radToDeg, NMEA0183Wind_Apparent, WindSpeedApp)) {
        SendMessage(NMEA0183MsgMWV);
      }
      CalcTrueWind();
      if (WindDirTrue != N2kDoubleNA && WindSpeedTrue != N2kDoubleNA) {
        double WindDirMag_deg = (Variation != N2kDoubleNA) ? WrapAngle(WindDirTrue - Variation)*radToDeg : N2kDoubleNA;
        double WindDirTrue_deg = WindDirTrue * radToDeg;
        if (NMEA0183SetMWD(NMEA0183MsgMWD, WindDirTrue_deg, WindDirMag_deg, WindSpeedTrue)) {
          SendMessage(NMEA0183MsgMWD);
        }
      }
    }
  }
}

void tN2kDataToNMEA0183::HandleEnvParams(const tN2kMsg &N2kMsg) {
  unsigned char SID;
  tN2kTempSource TempSource;
  double Temperature;
  tN2kHumiditySource HumiditySource;
  double Humidity;
  double AtmosphericPressure;
  if ( ParseN2kEnvironmentalParameters(N2kMsg, SID, TempSource, Temperature,
                                      HumiditySource, Humidity, AtmosphericPressure) ) {
    // Check for sea temp data
    if ( TempSource == N2kts_SeaTemperature ) {
      tNMEA0183Msg NMEA0183Msg;
      // Send water temperature
      // From N2k, comes in K. Convert to C.
      if ( NMEA0183SetMTW(NMEA0183Msg, KelvinToC(Temperature)) ) {
        SendMessage(NMEA0183Msg);
      }
    }
  }
}

//*****************************************************************************
void tN2kDataToNMEA0183::SendRMC() {
    if ( NextRMCSend<=millis() && !N2kIsNA(Latitude) ) {
      tNMEA0183Msg NMEA0183Msg;
      if ( NMEA0183SetRMC(NMEA0183Msg,SecondsSinceMidnight,Latitude,Longitude,COG,SOG,DaysSince1970,Variation) ) {
        SendMessage(NMEA0183Msg);
      }
      SetNextRMCSend();
    }
}

//*****************************************************************************
float tN2kDataToNMEA0183::WrapAngle(float angle) {
  // Wraps any angles going outside [0, 2*pi)
  angle = fmod(angle, 2*M_PI);
  if (angle < 0) {
    angle += 2*M_PI;
  }
  return angle;
}

//*****************************************************************************
void tN2kDataToNMEA0183::CalcTrueWind() {
  double vWindApp_NE[2];
  double vWindTrue_NE[2];
  double vBoatVelocity_NE[2];
  double WindAngleApp_NE;
  // Need all this data to correctly calculate true wind speed and dir
  if (HeadingTrue == N2kDoubleNA
   || WindAngleApp == N2kDoubleNA
   || WindSpeedApp == N2kDoubleNA
   || COG == N2kDoubleNA
   || SOG == N2kDoubleNA ) {
    WindDirTrue = N2kDoubleNA;
    WindSpeedTrue = N2kDoubleNA;
  } else {
    WindAngleApp_NE = HeadingTrue + WindAngleApp;
    vWindApp_NE[0] = WindSpeedApp * cos(WindAngleApp_NE);
    vWindApp_NE[1] = WindSpeedApp * sin(WindAngleApp_NE);
    vBoatVelocity_NE[0] = SOG * cos(COG);
    vBoatVelocity_NE[1] = SOG * sin(COG);
    vWindTrue_NE[0] = vWindApp_NE[0] - vBoatVelocity_NE[0];
    vWindTrue_NE[1] = vWindApp_NE[1] - vBoatVelocity_NE[1];
    WindDirTrue = WrapAngle(atan2(vWindTrue_NE[1], vWindTrue_NE[0]));
    WindSpeedTrue = sqrt(pow(vWindTrue_NE[0],2) + pow(vWindTrue_NE[1],2));
  }
}
