/*
emonPi v2.0 Continuous Monitoring - radio using JeeLib RFM69 "Native" format
------------------------------------------
Part of the openenergymonitor.org project
Authors: Glyn Hudson, Trystan Lea & Robert Wall
Builds upon JCW JeeLabs RF69 Driver and Arduino
Licence: GNU GPL V3
*/

const byte firmware_version[3] = {1,0,0};
/*
V1.0.0   10/7/2021 Derived from emonLibCM examples and original emonPi sketch, that being derived from 
            https://github.com/openenergymonitor/emonpi/blob/master/Atmega328/emonPi_RFM69CW_RF12Demo_DiscreteSampling
            and emonLibCM example sketches, with config input based on emonTx V3 sketches.
*/
// #define EEWL_DEBUG
// #define SAMPPIN 19
#define Serial Serial1

#include <emonLibCM.h>                                                 // OEM CM library
#include <emonEProm.h>

// RFM interface
#include <SPI.h>
#include "RF69n.h"
RF69n rf;

bool rfDataAvailable = false;

byte nativeMsg[66];                                                    // 'Native' format message buffer

#define MAXMSG 66                                                      // Max length of o/g message
char outmsg[MAXMSG];                                                   // outgoing message (to emonGLCD etc)
byte outmsgLength;                                                     // length of message: non-zero length triggers transmission

struct {                                                               // Ancilliary information
  byte srcNode = 0;
  byte msgLength = 0;
  signed char rssi = -127;
  bool crc = false;
} rfInfo;

enum rfband {RFM_433MHZ = 1, RFM_868MHZ, RFM_915MHZ };                 // frequency band.
bool rfChanged = false;                                                // marker to re-initialise radio
#define RFRX 0x01                                                      // Control of RFM - receive enabled
#define RFTX 0x02                                                      // Control of RFM - transmit enabled

#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
#define I2C_ADDRESS 0x3C
SSD1306AsciiWire oled;
int current_lcd_i2c_addr = I2C_ADDRESS;

//----------------------------emonPi Settings------------------------------------------------------------------------------------------------
bool debug                      = true;
bool verbose                    = false;
const unsigned long BAUD_RATE   = 115200;

int i2c_LCD_Detect(int i2c_lcd_address[]);
void emonPi_LCD_Startup();
void Startup_to_LCD(int current_lcd_i2c_addr);
void lcd_print_currents(int current_lcd_i2c_addr, float I1, float I2, float pf1, float pf2);
void single_LED_flash(void);
void double_LED_flash(void);
void getCalibration(void);
void send_emonpi_serial();
void emonPi_startup(void);
static void showString (PGM_P s);

#define PHASECAL 1.5

//---------------------------- emonPi Settings - Stored in EEPROM and shared with config.ino ------------------------------------------------
struct {
  byte RF_freq = RFM_433MHZ;                            // Frequency of radio module can be RFM_433MHZ, RFM_868MHZ or RFM_915MHZ. 
  byte rfPower = 25;                                    // Power when transmitting
  byte networkGroup = 210;                              // wireless network group, must be the same as emonBase / emonPi and emonGLCD. OEM default is 210
  byte  nodeID = 5;                                     // node ID for this emonPi.
  float vCal  = 810.4;                                  // (6 x 10000) / 75 = 800.0
  float assumedVrms = 240.0;                            // Assumed Vrms when no a.c. is detected
  float lineFreq = 50;                                  // Line Frequency = 50 Hz
  float i1Cal = 75.075;                                 // (25A / 333mV output) = 75.075
  float i1Lead = PHASECAL;                              // 1.5° phase lead
  float i2Cal = 75.075;                                 // (25A / 333mV output) = 75.075
  float i2Lead = PHASECAL;                              // 1.5° phase lead
  float i3Cal = 75.075;                                 // (25A / 333mV output) = 75.075
  float i3Lead = PHASECAL;                              // 1.5° phase lead
  float period = 4.85;                                  // datalogging period - should be fractionally less than the PHPFINA database period in emonCMS
  bool  pulse_enable = true;                            // pulse 1 counting 1
  int   pulse_period = 0;                               // pulse 1 min period - 0 = no de-bounce
  bool  pulse2_enable = false;                          // pulse 2 counting 2
  int   pulse2_period = 0;                              // pulse 2 min period - 0 = no de-bounce
  bool  showCurrents = false;                           // Print to serial voltage, current & p.f. values  
  byte  rfOn = 0x03;                                    // Turn transmitter AND receiver on
} EEProm;

uint16_t eepromSig = 0x0010;                            // EEPROM signature - see oemEProm Library documentation for details.

typedef struct {
    unsigned long Msg;
    int Vrms,P1,P2,P3; 
    long E1,E2,E3;
    unsigned long pulseCount, pulse2Count;
} PayloadTX;
PayloadTX emonPi; 

//-------------------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------------------

bool shutdown_switch_last_state = false;

//--------------------------- hard-wired connections ----------------------------------------------------------------------------------------

const byte LEDpin               = PIN_PA3;                             // emonPi LED - on when HIGH
//const byte emonPi_RJ45_8_IO   = A6;                                  // RJ45 pin 8 - Analog 6 - Aux I/O - D20?
const byte emonPi_int0          = PIN_PF3;                             // RJ45 pin 6 - INT0 - Dig 2 - default pulse count interrupt
const byte emonPi_int0_pin      = PIN_PF3;                             // RJ45 pin 6 - INT0 - Dig 2 - default pulse count input pin
const byte emonPi_int1          = 1;                                   // RJ45 pin 6 - INT1 - Dig 3 - default pulse count interrupt
const byte emonPi_int1_pin      = 3;                                   // RJ45 pin 6 - INT1 - Dig 3 - default pulse count input pin

// Use D5 for ISR timimg checks - only if Pi is not connected!

#ifdef EEWL_DEBUG
  extern EEWL EVmem;
#endif


/**************************************************************************************************************************************************
*
* SETUP        Set up & start the display, the radio & emonLibCM
*
***************************************************************************************************************************************************/
void setup() 
{  

  emonPi_startup();
  delay(100);
  emonPi_LCD_Startup();

  // 12 bit ADC = 4096 divisions
  // Time in microseconds for one ADC conversion: 40 us 
  EmonLibCM_setADC(12,29.5);

  // Using AVR-DB 1.024V internal voltage reference
  EmonLibCM_ADCCal(1.024);

  EmonLibCM_SetADC_VChannel(6, EEProm.vCal);                           // ADC Input channel, voltage calibration - for Ideal UK Adapter = 268.97
  EmonLibCM_SetADC_IChannel(3, EEProm.i1Cal, EEProm.i1Lead);           // ADC Input channel, current calibration, phase calibration
  EmonLibCM_SetADC_IChannel(2, EEProm.i2Cal, EEProm.i2Lead);           // The current channels will be read in this order
  EmonLibCM_SetADC_IChannel(1, EEProm.i3Cal, EEProm.i3Lead);           // The current channels will be read in this order

  EmonLibCM_cycles_per_second(EEProm.lineFreq);                        // mains frequency 50/60Hz
  
  EmonLibCM_setAssumedVrms(EEProm.assumedVrms);

  EmonLibCM_setPulseEnable(0, EEProm.pulse_enable);                    // Enable pulse counting
  EmonLibCM_setPulsePin(0, emonPi_int0_pin, emonPi_int0);              // Pulse input Pin, Interrupt
  EmonLibCM_setPulseMinPeriod(0, EEProm.pulse_period);                 // Minimum pulse period
  EmonLibCM_setPulseEnable(1, EEProm.pulse2_enable);                   // Enable pulse counting
  EmonLibCM_setPulsePin(1, emonPi_int1_pin, emonPi_int1);              // Pulse input Pin, Interrupt
  EmonLibCM_setPulseMinPeriod(1, EEProm.pulse_period);                 // Minimum pulse period

  // EmonLibCM_setTemperatureDataPin(PIN_PA2);                            // OneWire data pin 
  // EmonLibCM_setTemperaturePowerPin(-1);                                // Temperature sensor Power Pin - Not used.
  // EmonLibCM_setTemperatureResolution(TEMPERATURE_PRECISION);           // Resolution in bits, allowed values 9 - 12. 11-bit resolution, reads to 0.125 degC
  // EmonLibCM_setTemperatureAddresses(EEProm.allAddresses, true);        // Name of array of temperature sensors
  // EmonLibCM_setTemperatureArray(allTemps);                             // Name of array to receive temperature measurements
  // EmonLibCM_setTemperatureMaxCount(MaxOnewire);                        // Max number of sensors, limited by wiring and array size.
  
#ifdef EEWL_DEBUG
  Serial.print(F("End of mem="));Serial.print(E2END);
  Serial.print(F(" Avail mem="));Serial.print((E2END>>2) * 3);
  Serial.print(F(" Start addr="));Serial.print(E2END - (((E2END>>2) * 3) / (sizeof(mem)+1))*(sizeof(mem)+1));
  Serial.print(F(" Num blocks="));Serial.println(((E2END>>2) * 3) / 21);
  EVmem.dump_buffer();
#endif
  // Recover saved energy & pulse count from EEPROM
  long E1 = 0, E2 = 0, E3 = 0;
  unsigned long p1 = 0, p2 = 0;
  
  recoverEValues(&E1, &E2, &E3, &p1, &p2);
  EmonLibCM_setWattHour(0, E1);
  EmonLibCM_setWattHour(1, E2);
  EmonLibCM_setWattHour(2, E3);

  EmonLibCM_setPulseCount(0, p1);
  EmonLibCM_setPulseCount(1, p2);

  EmonLibCM_TemperatureEnable(false);                                  // Temperature monitoring enable
  EmonLibCM_min_startup_cycles(10);                                    // number of cycles to let ADC run before starting first actual measurement

  EmonLibCM_datalog_period(0.5);                                       // Get a quick reading for display of voltage, temp sensor count.
  
  EmonLibCM_Init();                                                    // Start continuous monitoring.

  while (!EmonLibCM_Ready())                                           // Quick check for presence of a voltage
    ;
 
  EmonLibCM_datalog_period(EEProm.period);                             // Reset to NORMAL period of readings in seconds for emonCMS
  Startup_to_LCD(current_lcd_i2c_addr);

  rf.init(EEProm.nodeID, EEProm.networkGroup, 
               EEProm.RF_freq == RFM_915MHZ ? 915                      // Fall through to 433 MHz Band @ 434 MHz
            : (EEProm.RF_freq == RFM_868MHZ ? 868 : 434)); 
}

/**************************************************************************************************************************************************
*
* LOOP         Read the pushbutton, poll the radio for incoming data, the serial input for calibration & emonLibCM for energy readings
*
***************************************************************************************************************************************************/

void loop()             
{
  
  
//-------------------------------------------------------------------------------------------------------------------------------------------
// RF Data handler - inbound ****************************************************************************************************************
//-------------------------------------------------------------------------------------------------------------------------------------------

  int len = rf.receive(&nativeMsg, sizeof(nativeMsg));                 // Poll the RFM buffer and extract the data
  if ((EEProm.rfOn & RFRX) && len > 1)
  {
    rfInfo.crc = true;
    rfInfo.msgLength = len;
    rfInfo.srcNode = nativeMsg[1];
    rfInfo.rssi = -rf.rssi/2;

    // send serial data
    Serial.print(F("OK"));                                              // Bad packets (crc failure) are discarded by RFM69CW
    print_frame(rfInfo.msgLength);		                                  // Print received data
    double_LED_flash();
  }
  
//-------------------------------------------------------------------------------------------------------------------------------------------
// RF Data handler - outbound ***************************************************************************************************************
//-------------------------------------------------------------------------------------------------------------------------------------------


	if ((EEProm.rfOn & RFTX) && outmsgLength) {                           //if command 'outmsg' is waiting to be sent then let's send it
    digitalWrite(LEDpin, HIGH); delay(200); digitalWrite(LEDpin, LOW);
    showString(PSTR(" -> "));
    Serial.print((word) outmsgLength);
    showString(PSTR(" b\n"));
    rf.send(0, (void *)outmsg, outmsgLength);                          //  void RF69<SPI>::send (uint8_t header, const void* ptr, int len) {
    outmsgLength = 0;
	}

  
//-------------------------------------------------------------------------------------------------------------------------------------------
// Calibration Data handler *****************************************************************************************************************
//-------------------------------------------------------------------------------------------------------------------------------------------

  if (Serial.available())                                              // Serial input from RPi for configuration/calibration
  {
    getCalibration();                                                  // If serial input is received from RPi
    double_LED_flash();
    if (rfChanged)
    {
      rf.init(EEProm.nodeID, EEProm.networkGroup,                      // Reset the RFM69CW if NodeID, Group or frequency has changed.
            EEProm.RF_freq == RFM_915MHZ ? 915 : (EEProm.RF_freq == RFM_868MHZ ? 868 : 434)); 
      rfChanged = false;
    }
  }

//-------------------------------------------------------------------------------------------------------------------------------------------
// Energy Monitor ***************************************************************************************************************************
//-------------------------------------------------------------------------------------------------------------------------------------------

  if (EmonLibCM_Ready())   
  {
    single_LED_flash();                                                // single flash of LED on local CT sample

    emonPi.Vrms     = EmonLibCM_getVrms() * 100;                       // Always send the ACTUAL measured voltage.

    emonPi.P1 = EmonLibCM_getRealPower(0);                       // Copy the desired variables ready for transmission 
    emonPi.P2 = EmonLibCM_getRealPower(1); 
    emonPi.P3 = EmonLibCM_getRealPower(2);

    emonPi.E1     = EmonLibCM_getWattHour(0);
    emonPi.E2     = EmonLibCM_getWattHour(1);
    emonPi.E3     = EmonLibCM_getWattHour(2);
    
    // if (EmonLibCM_getTemperatureSensorCount())
    // {
    //   for (byte i=0; i< MaxOnewire; i++)
    //     emonPi.temp[i] = allTemps[i];
    // }
   
    emonPi.pulseCount = EmonLibCM_getPulseCount(0);
    emonPi.pulse2Count = EmonLibCM_getPulseCount(1);
    
    send_emonpi_serial();                                              // Send emonPi data to Pi serial using packet structure

    if (EEProm.showCurrents)
    {
      // to show voltage, current & power factor for calibration:
      Serial.print(F("|Vrms:")); Serial.print(EmonLibCM_getVrms());

      Serial.print(F(", I1:")); Serial.print(EmonLibCM_getIrms(0));
      Serial.print(F(", pf1:")); Serial.print(EmonLibCM_getPF(0),4);

      Serial.print(F(", I2:")); Serial.print(EmonLibCM_getIrms(1));
      Serial.print(F(", pf2:")); Serial.print(EmonLibCM_getPF(1),4);
      
      Serial.print(F(", f:")); Serial.print(EmonLibCM_getLineFrequency());
      Serial.println();
      
      lcd_print_currents(current_lcd_i2c_addr, EmonLibCM_getIrms(0), EmonLibCM_getIrms(1), EmonLibCM_getPF(0), EmonLibCM_getPF(1));
   }
    delay(50);

    // Save energy & pulse count values to EEPROM
    storeEValues(emonPi.E1, emonPi.E2, emonPi.E3, emonPi.pulseCount, emonPi.pulse2Count);
  }
} 


/**************************************************************************************************************************************************
*
* SEND RECEIVED RF DATA TO RPi (/dev/ttyAMA0)
*
***************************************************************************************************************************************************/

void print_frame(int len) 
{
  Serial.print(F(" "));
  Serial.print(rfInfo.srcNode);        // Extract and print node ID
  Serial.print(F(" "));
  for (byte i = 2; i < len; ++i) 
  {
    Serial.print((word)nativeMsg[i]);
    Serial.print(F(" "));
  }
  Serial.print(F("("));
  Serial.print(rfInfo.rssi);
  Serial.print(F(")"));
  Serial.println();
}


/**************************************************************************************************************************************************
*
* LED flash
*
***************************************************************************************************************************************************/

void single_LED_flash(void)
{
  digitalWrite(LEDpin, HIGH);  delay(30); digitalWrite(LEDpin, LOW);
}

void double_LED_flash(void)
{
  digitalWrite(LEDpin, HIGH);  delay(20); digitalWrite(LEDpin, LOW); delay(60); 
  digitalWrite(LEDpin, HIGH);  delay(20); digitalWrite(LEDpin, LOW);
}


/**************************************************************************************************************************************************
*
* Startup      Set I/O pins, print initial message, read configuration from EEPROM
*
***************************************************************************************************************************************************/

void emonPi_startup()
{
  pinMode(LEDpin, OUTPUT);
  digitalWrite(LEDpin,HIGH);
  
  Serial.begin(BAUD_RATE);
  Serial.print(F("|emonPiCM V")); printVersion();
  Serial.println(F("|OpenEnergyMonitor.org"));
  load_config();                                                      // Load RF config from EEPROM (if any exists)

#ifdef SAMPPIN
  pinMode(SAMPPIN, OUTPUT);
  digitalWrite(SAMPPIN, LOW);
#endif
  
}


/***************************************************************************************************************************************************
*
* lcd - I²C   Set up 16 x 2 I²C LCD display, get startup data & print startup message, print currents as required
*
***************************************************************************************************************************************************/

int i2c_LCD_Detect(int i2c_lcd_address[])
{
  Wire.begin();
  byte error=1;
  for (int i=0; i<2; i++)
  {
    Wire.beginTransmission(i2c_lcd_address[i]);
    error = Wire.endTransmission();
    if (error == 0)
    {
      Serial.print(F("LCD found i2c 0x")); Serial.println(i2c_lcd_address[i], HEX);
      return (i2c_lcd_address[i]);
      break;
    }
  }
  Serial.println(F("LCD not found"));
  return(0);
}


void emonPi_LCD_Startup() 
{
  Wire.swap(2);
  Wire.begin();
  Wire.setClock(400000L);
  oled.begin(&Adafruit128x32, I2C_ADDRESS);

  oled.setFont(Adafruit5x7);
  //oled.setLetterSpacing(2);
  
  oled.clear();
  oled.set1X();
  oled.print(F("emonPi V"));
  oled.print(firmware_version[0]);
  oled.print(".");
  oled.print(firmware_version[1]);
  oled.print(".");
  oled.println(firmware_version[2]);
  oled.print(F("OpenEnergyMonitor"));
  delay(1000);
}

void Startup_to_LCD(int current_lcd_i2c_addr)                          // Print to LCD
{
  //-----------------------------------------------------------------------------------------------------------------------------------------------
  oled.clear();

  if (EmonLibCM_acPresent())
    oled.print(F("AC Wave Detected"));
  else
    oled.print(F("AC NOT Detected"));
  delay(4000);

  //oled.clear();
  //oled.print(F("Detected: ")); oled.print(EmonLibCM_getTemperatureSensorCount());
  //oled.setCursor(0, 1); oled.print(F("DS18B20 Temp"));
  //delay(2000);

  oled.clear();
  oled.print(F("Pi is booting..."));
  oled.setCursor(0, 1); oled.print(F("Please wait"));
  delay(20);
}


void lcd_print_currents(int current_lcd_i2c_addr, float I1, float I2, float pf1, float pf2)
{
  oled.clear();
  oled.setCursor(0, 0);
  oled.print(F("1 "));oled.print(I1);oled.print(F(" "));oled.print(pf1,4);
  oled.setCursor(0, 1);
  oled.print(F("2 "));oled.print(I2);oled.print(F(" "));oled.print(pf2,4);
}


/**************************************************************************************************************************************************
*
* SEND OWN SERIAL DATA TO RPi (/dev/ttyAMA0) using packet structure
*
***************************************************************************************************************************************************/

void send_emonpi_serial()
{
  Serial.print(F("Vrms:")); Serial.print(emonPi.Vrms*0.01);

  Serial.print(F(",P1:")); Serial.print(emonPi.P1);
  Serial.print(F(",P2:")); Serial.print(emonPi.P2);
  Serial.print(F(",P3:")); Serial.print(emonPi.P3);
  
  Serial.print(F(",E1:")); Serial.print(emonPi.E1);
  Serial.print(F(",E2:")); Serial.print(emonPi.E2);
  Serial.print(F(",E3:")); Serial.print(emonPi.E3);

  Serial.println();

  delay(10);
}

void printVersion(void)
{
  Serial.print(firmware_version[0]);
  Serial.print(".");
  Serial.print(firmware_version[1]);
  Serial.print(".");
  Serial.println(firmware_version[2]);
}  
