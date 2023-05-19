/*
  emonPiV2.0 Factory Test
*/
#define Serial Serial3
#define RFM69CW
#include <Arduino.h>
#include "RFM69_LPL.h"
#include <emonLibCM.h>

RFM69 rf;

typedef struct {
    unsigned long Msg;
    int Vrms,P1,P2,P3,P4,P5,P6;
    int T1,T2,T3;
    unsigned long pulse;
} PayloadTX;
PayloadTX emontx;
 
#define MAX_TEMPS 3                                       
 
#define DEFAULT_VCAL 807.86
#define DEFAULT_ICAL 60.06
#define DEFAULT_LEAD 3.2

struct {
  byte RF_freq = RF69_433MHZ;                              // Frequency of radio module can be RFM_433MHZ, RFM_868MHZ or RFM_915MHZ. 
  byte networkGroup = 210;                                 // wireless network group, must be the same as emonBase / emonPi and emonGLCD. OEM default is 210
  byte nodeID = 15;                                        // node ID for this emonTx.
  DeviceAddress allAddresses[MAX_TEMPS];                   // sensor address data
} EEProm;

int allTemps[MAX_TEMPS];                                   // Array to receive temperature measurements

const byte LEDpin      = PIN_PC2;  // emonTx V3 LED

void setup() 
{  
  //wdt_enable(WDTO_8S);
  
  pinMode(LEDpin, OUTPUT);
  digitalWrite(LEDpin,HIGH);
  
  Serial.begin(115200);

  Serial.println(F("emonPi V2 Factory Test"));
  Serial.println(F("Trying to start radio..."));

  rf.setPins(PIN_PA7,PIN_PA4,PIN_PA5,PIN_PA6);
  rf.initialize(RF69_433MHZ, EEProm.nodeID, EEProm.networkGroup);
  rf.encrypt("89txbe4p8aik5kt3");
   
  Serial.print(F("RFM69CW "));
  Serial.print(F(" Freq: "));
  Serial.print(F("433MHz"));
  Serial.print(F(" Group: ")); Serial.print(EEProm.networkGroup);
  Serial.print(F(" Node: ")); Serial.print(EEProm.nodeID);
  Serial.println();
  
  digitalWrite(LEDpin,LOW);
  
  EmonLibCM_setADC(12,39.333);

  EmonLibCM_ADCCal(1.024);
  
  EmonLibCM_SetADC_VChannel(0, DEFAULT_VCAL);                         // ADC Input channel, voltage calibration
  EmonLibCM_SetADC_IChannel(3, DEFAULT_ICAL, DEFAULT_LEAD);           // ADC Input channel, current calibration, phase calibration
  EmonLibCM_SetADC_IChannel(4, DEFAULT_ICAL, DEFAULT_LEAD);           // The current channels will be read in this order
  EmonLibCM_SetADC_IChannel(5, DEFAULT_ICAL, DEFAULT_LEAD);           
  EmonLibCM_SetADC_IChannel(6, DEFAULT_ICAL, DEFAULT_LEAD);           
  EmonLibCM_SetADC_IChannel(8, DEFAULT_ICAL, DEFAULT_LEAD);
  EmonLibCM_SetADC_IChannel(9, DEFAULT_ICAL, DEFAULT_LEAD);

  EmonLibCM_datalog_period(1.8);
  
  EmonLibCM_setAssumedVrms(240.0);

  EmonLibCM_setPulseEnable(true);
  EmonLibCM_setPulsePin(PIN_PB5);
  EmonLibCM_setPulseMinPeriod(100);

  EmonLibCM_setTemperatureDataPin(PIN_PB4);                            // OneWire data pin (emonTx V3.4)
  EmonLibCM_setTemperaturePowerPin(PIN_PC1);                           // Temperature sensor Power Pin - 19 for emonTx V3.4  (-1 = Not used. No sensors, or sensor are permanently powered.)
  EmonLibCM_setTemperatureResolution(11);                              // Resolution in bits, allowed values 9 - 12. 11-bit resolution, reads to 0.125 degC
  EmonLibCM_setTemperatureAddresses(EEProm.allAddresses);              // Name of array of temperature sensors
  EmonLibCM_setTemperatureArray(allTemps);                             // Name of array to receive temperature measurements
  EmonLibCM_setTemperatureMaxCount(MAX_TEMPS);                         // Max number of sensors, limited by wiring and array size.
  
  EmonLibCM_TemperatureEnable(true);  
  EmonLibCM_Init();                                                    // Start continuous monitoring.
  emontx.Msg = 0;
  printTemperatureSensorAddresses();
}

void loop()             
{ 
  if (EmonLibCM_Ready())   
  {
    int acPresent_pass = 0;
    if (EmonLibCM_acPresent()) {
      acPresent_pass = 1;
    }

    emontx.Msg++;
    
    emontx.P1 = EmonLibCM_getMean(0);
    emontx.P2 = EmonLibCM_getMean(1);
    emontx.P3 = EmonLibCM_getMean(2);
    emontx.P4 = EmonLibCM_getMean(3);
    emontx.P5 = EmonLibCM_getMean(4); 
    emontx.P6 = EmonLibCM_getMean(5); 

    int ct_pass = 0;

    if (emontx.P1>1950 && emontx.P1<2100) {
      ct_pass++;
    }
    if (emontx.P2>1950 && emontx.P2<2100) {
      ct_pass++;
    }
    if (emontx.P3>1950 && emontx.P3<2100) {
      ct_pass++;
    }
    if (emontx.P4>1950 && emontx.P4<2100) {
      ct_pass++;
    }
    if (emontx.P5>1950 && emontx.P5<2100) {
      ct_pass++;
    }
    if (emontx.P6>1950 && emontx.P6<2100) {
      ct_pass++;
    }
    
    emontx.Vrms = EmonLibCM_getVrms() * 100;

    int voltage_pass = 0;
    if (emontx.Vrms>22000 && emontx.Vrms<26000) {
      voltage_pass = 1;
    }
    
    emontx.T1 = allTemps[0];
    emontx.T2 = allTemps[1];
    emontx.T3 = allTemps[2];

    emontx.pulse = EmonLibCM_getPulseCount();

    if (rf.sendWithRetry(5,(byte *)&emontx, sizeof(emontx))) {
      Serial.println("ack");
    } else {
      Serial.println("no ack");
    }

    delay(50);
    
    Serial.print(F("MSG:")); Serial.print(emontx.Msg);
    Serial.print(F(",Vrms:")); Serial.print(emontx.Vrms*0.01);
    
    Serial.print(F(",P1:")); Serial.print(emontx.P1);
    Serial.print(F(",P2:")); Serial.print(emontx.P2);
    Serial.print(F(",P3:")); Serial.print(emontx.P3);
    Serial.print(F(",P4:")); Serial.print(emontx.P4);
    Serial.print(F(",P5:")); Serial.print(emontx.P5);
    Serial.print(F(",P6:")); Serial.print(emontx.P6);
     
    if (emontx.T1!=30000) { Serial.print(F(",T1:")); Serial.print(emontx.T1*0.01); }
    if (emontx.T2!=30000) { Serial.print(F(",T2:")); Serial.print(emontx.T2*0.01); }
    if (emontx.T3!=30000) { Serial.print(F(",T3:")); Serial.print(emontx.T3*0.01); }

    Serial.print(F(",pulse:")); Serial.print(emontx.pulse);

    Serial.print(F(",acpass:")); Serial.print(acPresent_pass);
    Serial.print(F(",vpass:")); Serial.print(voltage_pass);
    Serial.print(F(",ct_pass:")); Serial.print(ct_pass);


    Serial.println();
    delay(40);

    if (acPresent_pass && voltage_pass && ct_pass==6) {
        digitalWrite(LEDpin,LOW); delay(50);digitalWrite(LEDpin,HIGH);
    } else {
        digitalWrite(LEDpin,HIGH); delay(50);digitalWrite(LEDpin,LOW);
    }
  }
  delay(20);
}
