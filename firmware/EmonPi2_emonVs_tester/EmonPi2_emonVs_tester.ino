#define Serial Serial3
#define DEBUG                                              // Debug level print out
#define EMONTX4
#include <Arduino.h>
#include <avr/wdt.h>
#include <emonLibCM.h>                                     // OEM Continuous Monitoring library

#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
SSD1306AsciiWire oled;

const byte LEDpin      = PIN_PC2;  // emonTx V4 LED

void setup() 
{ 
  delay(100);
  Wire1.swap(2);
  Wire1.begin();
  Wire1.setClock(400000L);

  oled.begin(&Adafruit128x64, 0x3C);

  oled.setFont(CalLite24);
  oled.setLetterSpacing(2);

  oled.clear();
  oled.setCursor(15,1);
  oled.print("emonVs");
  oled.setFont(Arial14);
  oled.setCursor(45,6);
  oled.print("tester");
  oled.setFont(CalLite24);
  oled.setLetterSpacing(2);

  pinMode(LEDpin, OUTPUT);
  digitalWrite(LEDpin,HIGH);
  Serial.begin(115200);
  EmonLibCM_setADC(12,39.333);
  EmonLibCM_ADCCal(1.024);
  EmonLibCM_SetADC_VChannel(0, 807.86);
  EmonLibCM_setAssumedVrms(123);
  EmonLibCM_TemperatureEnable(false);  
  EmonLibCM_Init();
  EmonLibCM_datalog_period(1.0);
}

void loop()             
{
  if (EmonLibCM_Ready())   
  {
    
    double Vrms = 0;
    if (EmonLibCM_acPresent()) {
      Vrms = EmonLibCM_getVrms();
    }
    Serial.print(F("Vrms:")); 
    Serial.println(Vrms);
  
    oled.clear();
    oled.setCursor(15,2);
    oled.print(Vrms);
  }
}
