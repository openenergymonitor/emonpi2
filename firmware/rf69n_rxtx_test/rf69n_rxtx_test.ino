#define Serial Serial1

#include <SPI.h>
#include "RF69n.h"
RF69n rf;

byte rxMsg[66];                                                        

typedef struct {
  unsigned long Msg;
  int P1,P2,P3;
} PayloadTX;
PayloadTX emonpi;

unsigned long last_tx = 0;

void setup() 
{  
  Serial.begin(115200);
  Serial.println("rfm69n rxtx test");
  
  rf.init(5, 210, 434);

  emonpi.Msg = 1;
  emonpi.P1 = 100;
  emonpi.P2 = 200;
  emonpi.P3 = 300;
}

void loop()             
{
  // Poll the RFM buffer and extract the data
  int len = rf.receive(&rxMsg, sizeof(rxMsg));                 
  if (len > 1)
  {
    byte srcNode = rxMsg[1];
    signed char rssi = -rf.rssi/2;
    
    Serial.print("OK");
    Serial.print(" ");
    Serial.print(srcNode);
    Serial.print(" ");
    for (byte i = 2; i < len; ++i) 
    {
      Serial.print((word)rxMsg[i]);
      Serial.print(" ");
    }
    Serial.print("(");
    Serial.print(rssi);
    Serial.println(")");
  }
  
  // Send a radio packet
  if ((millis()-last_tx)>5000) {
    last_tx = millis();
    Serial.println("tx");
    rf.send(0, (byte *)&emonpi, sizeof(emonpi));
    emonpi.Msg ++;
  }
}
