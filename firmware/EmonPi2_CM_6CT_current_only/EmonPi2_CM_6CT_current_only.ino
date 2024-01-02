/*
  emonPi2 Continuous Sampling (RMS current only variant)
  using EmonLibCM https://github.com/openenergymonitor/EmonLibCM
  Authors: Robin Emley, Robert Wall, Trystan Lea
  
  -----------------------------------------
  Part of the openenergymonitor.org project
  Licence: GNU GPL V3

  v1.0.0 currently only variant based on EmonPi2_CM_3x_temperature_transmitter
*/

#define Serial Serial3

#define RFM69_JEELIB_CLASSIC 1
#define RFM69_JEELIB_NATIVE 2
#define RFM69_LOW_POWER_LABS 3

#define RadioFormat RFM69_LOW_POWER_LABS

const char *firmware_version = {"1.0.0\n\r"};
/*

emonhub.conf node decoder (nodeid is 17 when switch is off, 18 when switch is on)
See: https://github.com/openenergymonitor/emonhub/blob/emon-pi/configuration.md
copy the following into emonhub.conf:

[[17]]
  nodename = emonPi2_17
  [[[rx]]]
    names = MSG, I1, I2, I3, I4, I5, I6
    datacodes = L,h,h,h,h,h,h
    scales = 1,0.001,0.001,0.001,0.001,0.001,0.001
    units = n,W,W,W,W,W,W

*/
// Comment/Uncomment as applicable
#define DEBUG                                              // Debug level print out
#define EMONTX4

// #define EEWL_DEBUG

#define OFF HIGH
#define ON LOW

#define RFM69CW

#include <Arduino.h>
#include <avr/wdt.h>

#if RadioFormat == RFM69_LOW_POWER_LABS
  #include "RFM69_LPL.h"
#else
  #include "RFM69_JeeLib.h"                                        // Minimal radio library that supports both original JeeLib format and later native format
#endif

// EEWL_START = 102, Start EEPROM wear leveling section after config section which takes up first 99 bytes, a few bytes of padding here
// EEWL_BLOCKS = 14, 14 x 29 byte blocks = 406 bytes. EEWL_END = 102+406 = 508 a few bytes short of 512 byte limit
// EEWL is only updated if values change by more than 200Wh, this means for a typical house consuming ~4000kWh/year
// 20,000 writes per channel to EEPROM, there's a 100,000 write lifetime for any individual EEPROM byte.
// With a circular buffer of 14 blocks, this extends the lifetime from 5 years to 70 years.
// IMPORTANT! If adding to config section change EEWL_START and check EEWL_END Implications.

#include <emonEProm.h>                                     // OEM EEPROM library
#include <emonLibCM.h>                                     // OEM Continuous Monitoring library

// Include EmonTxV4_config.ino in the same directory for settings functions & data

RFM69 rf;

typedef struct {
    unsigned long Msg;
    int I1,I2,I3,I4,I5,I6;
} PayloadTX; // create a data packet for the RFM
PayloadTX emontx;
static void showString (PGM_P s);
 
#define MAX_TEMPS 3                                        // The maximum number of temperature sensors
 
//---------------------------- emonTx Settings - Stored in EEPROM and shared with config.ino ------------------------------------------------
#define DEFAULT_ICAL 60.06                                // 25A / 333mV output = 75.075
#define DEFAULT_LEAD 0.0

struct {
  byte RF_freq = RF69_433MHZ;                              // Frequency of radio module can be RFM_433MHZ, RFM_868MHZ or RFM_915MHZ. 
  byte networkGroup = 210;                                 // wireless network group, must be the same as emonBase / emonPi and emonGLCD. OEM default is 210
  byte nodeID = 17;                                        // node ID for this emonTx.
  byte rf_on = 0;                                          // RF - 0 = no RF, 1 = RF on.
  byte rfPower = 25;                                       // 7 = -10.5 dBm, 25 = +7 dBm for RFM12B; 0 = -18 dBm, 31 = +13 dBm for RFM69CW. Default = 25 (+7 dBm)
  float vCal  = 807.86;                                    // (6 x 10000) / 75 = 800.0
  float assumedVrms = 240.0;                               // Assumed Vrms when no a.c. is detected
  float lineFreq = 50;                                     // Line Frequency = 50 Hz

  float i1Cal =  300.30;
  float i1Lead = DEFAULT_LEAD;
  float i2Cal =  150.15;
  float i2Lead = DEFAULT_LEAD;
  float i3Cal =  150.15;
  float i3Lead = DEFAULT_LEAD;
  float i4Cal =  DEFAULT_ICAL;                                     
  float i4Lead = DEFAULT_LEAD;                                      
  float i5Cal =  DEFAULT_ICAL;
  float i5Lead = DEFAULT_LEAD;
  float i6Cal =  DEFAULT_ICAL;
  float i6Lead = DEFAULT_LEAD;
  
  float period = 9.8;                                      // datalogging period - should be fractionally less than the PHPFINA database period in emonCMS
  bool  json_enabled = false;                              // JSON Enabled - false = key,Value pair, true = JSON, default = false: Key,Value pair.  
} EEProm;

uint16_t eepromSig = 0x0018;                               // oemEProm signature - see oemEProm Library documentation for details.
 
#ifdef EEWL_DEBUG
  extern EEWL EVmem;
#endif

bool  USA=false;

bool calibration_enable = true;                           // Enable on-line calibration when running. 
                                                           // For safety, thus MUST default to false. (Required due to faulty ESP8266 software.)

//----------------------------emonTx V4 hard-wired connections-----------------------------------
const byte LEDpin      = PIN_PC2;  // emonTx V4 LED

//----------------------------------------Setup--------------------------------------------------
void setup() 
{  
  //wdt_enable(WDTO_8S);
  
  pinMode(LEDpin, OUTPUT);
  digitalWrite(LEDpin,HIGH);
  
  // Serial---------------------------------------------------------------------------------
  Serial.begin(115200);
  
  #ifdef DEBUG
    Serial.print(F("emonPi2 CM current only V")); Serial.write(firmware_version);
    Serial.println(F("OpenEnergyMonitor.org"));
  #else
    Serial.println(F("describe:EmonTX4CM"));
  #endif
 
  load_config(true);                                                   // Load RF config from EEPROM (if any exists)
  
  if (EEProm.rf_on)
  {
    #ifdef DEBUG
      #ifdef RFM12B
      Serial.print(F("RFM12B "));
      #endif
      #ifdef RFM69CW
      Serial.print(F("RFM69CW "));
      #endif
      Serial.print(F(" Freq: "));
      if (EEProm.RF_freq == RF69_433MHZ) Serial.print(F("433MHz"));
      if (EEProm.RF_freq == RF69_868MHZ) Serial.print(F("868MHz"));
      if (EEProm.RF_freq == RF69_915MHZ) Serial.print(F("915MHz"));
      Serial.print(F(" Group: ")); Serial.print(EEProm.networkGroup);
      Serial.print(F(" Node: ")); Serial.print(EEProm.nodeID);
      Serial.println(F(" "));
    #endif

    #if RadioFormat == RFM69_LOW_POWER_LABS
      Serial.println("RadioFormat: LowPowerLabs");
    #elif RadioFormat == RFM69_JEELIB_CLASSIC
      Serial.println("RadioFormat: JeeLib Classic");
    #elif RadioFormat == RFM69_JEELIB_NATIVE
      Serial.println("RadioFormat: JeeLib Native");
    #endif
  } else {
    Serial.println("Radio disabled");
  }
  // ---------------------------------------------------------------------------------------

  if (EEProm.rf_on)
  {
    #if RadioFormat == RFM69_JEELIB_CLASSIC
      rf.format(RFM69_JEELIB_CLASSIC);
    #endif
    
    // Frequency is currently hardcoded to 433Mhz in library
    #if RadioFormat == RFM69_LOW_POWER_LABS
    rf.setPins(PIN_PA7,PIN_PA4,PIN_PA5,PIN_PA6);
    #endif
    rf.initialize(RF69_433MHZ, EEProm.nodeID, EEProm.networkGroup); 
    rf.encrypt("89txbe4p8aik5kt3"); // ignored if jeelib classic
    delay(random(EEProm.nodeID * 20));                                 // try to avoid r.f. collisions at start-up
  }
  
  // ---------------------------------------------------------------------------------------
      
#ifdef EEWL_DEBUG
  EVmem.dump_buffer();
#endif

  double reference = read_reference();
  Serial.print(F("Reference voltage calibration: "));
  Serial.println(reference,4);

  // ----------------------------------------------------------------------------
  // EmonLibCM config
  // ----------------------------------------------------------------------------
  // 12 bit ADC = 4096 divisions
  // Time in microseconds for one ADC conversion: 39.333 us 
  EmonLibCM_setADC(12,39.333);

  // Using AVR-DB 1.024V internal voltage reference
  EmonLibCM_ADCCal(reference);
  
  EmonLibCM_SetADC_VChannel(0, EEProm.vCal);                           // ADC Input channel, voltage calibration
  EmonLibCM_SetADC_IChannel(3, EEProm.i1Cal, EEProm.i1Lead);           // ADC Input channel, current calibration, phase calibration
  EmonLibCM_SetADC_IChannel(4, EEProm.i2Cal, EEProm.i2Lead);           // The current channels will be read in this order
  EmonLibCM_SetADC_IChannel(5, EEProm.i3Cal, EEProm.i3Lead);           
  EmonLibCM_SetADC_IChannel(6, EEProm.i4Cal, EEProm.i4Lead);           
  EmonLibCM_SetADC_IChannel(8, EEProm.i5Cal, EEProm.i5Lead);
  EmonLibCM_SetADC_IChannel(9, EEProm.i6Cal, EEProm.i6Lead);

  // mains frequency 50Hz
  if (USA) EmonLibCM_cycles_per_second(60);                            // mains frequency 60Hz
  EmonLibCM_datalog_period(EEProm.period);                             // period of readings in seconds - normal value for emoncms.org  

  EmonLibCM_setAssumedVrms(EEProm.assumedVrms);

  EmonLibCM_setPulseEnable(false);
  EmonLibCM_TemperatureEnable(false);  
 
#ifdef EEWL_DEBUG
  EVmem.dump_control();
  EVmem.dump_buffer();  
#endif

  // Start continuous monitoring.
  EmonLibCM_Init();                                                    
  emontx.Msg = 0;
  // Speed up startup by making first reading 2s
  EmonLibCM_datalog_period(2.0);
}

void loop()             
{
  
  getSettings();
  
  if (EmonLibCM_Ready())   
  {
    
    if (emontx.Msg==0) 
    {
      digitalWrite(LEDpin,LOW);
      EmonLibCM_datalog_period(EEProm.period); 
    }
    delay(5);

    emontx.Msg++;
    
    emontx.I1 = EmonLibCM_getIrms(0)*1000; 
    emontx.I2 = EmonLibCM_getIrms(1)*1000; 
    emontx.I3 = EmonLibCM_getIrms(2)*1000; 
    emontx.I4 = EmonLibCM_getIrms(3)*1000; 
    emontx.I5 = EmonLibCM_getIrms(4)*1000; 
    emontx.I6 = EmonLibCM_getIrms(5)*1000; 
    
    if (EEProm.rf_on) {
      PayloadTX tmp = emontx;

      #if RadioFormat == RFM69_LOW_POWER_LABS
        rf.sendWithRetry(5,(byte *)&tmp, sizeof(tmp));
      #else
        rf.send(0, (byte *)&tmp, sizeof(tmp));
      #endif
      
      delay(50);
    }

    if (EEProm.json_enabled) {
      // ---------------------------------------------------------------------
      // JSON Format
      // ---------------------------------------------------------------------
      Serial.print(F("{\"MSG\":")); Serial.print(emontx.Msg);

      Serial.print(F(",\"I1\":")); Serial.print(emontx.I1*0.001,3);
      Serial.print(F(",\"I2\":")); Serial.print(emontx.I2*0.001,3);
      Serial.print(F(",\"I3\":")); Serial.print(emontx.I3*0.001,3);
      Serial.print(F(",\"I4\":")); Serial.print(emontx.I4*0.001,3);
      Serial.print(F(",\"I5\":")); Serial.print(emontx.I5*0.001,3);
      Serial.print(F(",\"I6\":")); Serial.print(emontx.I6*0.001,3);

      Serial.println(F("}"));
      delay(60);
      
    } else {
  
      // ---------------------------------------------------------------------
      // Key:Value format, used by EmonESP & emonhub EmonHubOEMInterfacer
      // ---------------------------------------------------------------------
      Serial.print(F("MSG:")); Serial.print(emontx.Msg);
      
      Serial.print(F(",I1:")); Serial.print(emontx.I1*0.001,3);
      Serial.print(F(",I2:")); Serial.print(emontx.I2*0.001,3);
      Serial.print(F(",I3:")); Serial.print(emontx.I3*0.001,3);
      Serial.print(F(",I4:")); Serial.print(emontx.I4*0.001,3);
      Serial.print(F(",I5:")); Serial.print(emontx.I5*0.001,3);
      Serial.print(F(",I6:")); Serial.print(emontx.I6*0.001,3);
      
      Serial.println();
      delay(40);
    }
    
    digitalWrite(LEDpin,HIGH); delay(50);digitalWrite(LEDpin,LOW);
    
    
  }
  wdt_reset();
  delay(20);
}


double read_reference() {
  ADC0.SAMPCTRL = 14;
  ADC0.CTRLD |= 0x0;
  VREF.ADC0REF = VREF_REFSEL_1V024_gc;
  ADC0.CTRLC = ADC_PRESC_DIV24_gc;
  ADC0.CTRLA = ADC_ENABLE_bm;
  ADC0.CTRLA |= ADC_RESSEL_12BIT_gc;

  ADC0.MUXPOS = 7;
  unsigned long sum = 0;
  for (int i=0; i<10010; i++) {
    ADC0.COMMAND = ADC_STCONV_bm;
    while (!(ADC0.INTFLAGS & ADC_RESRDY_bm));
    if (i>9) {
      sum += ADC0.RES;
    }
  }
  double mean = sum / 10000.0;
  double reference = 0.9 / (mean/4095.0);
  return reference;
}
