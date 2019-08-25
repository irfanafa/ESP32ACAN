/******************************************************************************/
/* File name        : LoopBackCheck-PollingIntensive.ino                      */
/* Project          : ESP32-CAN-DRIVER                                        */
/* Description      : ESP32 CAN Self Test by Polling                          */
/* ---------------------------------------------------------------------------*/
/* Copyright        : Copyright © 2019 Pierre Molinaro. All rights reserved.  */
/* ---------------------------------------------------------------------------*/
/* Author           : Mohamed Irfanulla                                       */
/* Supervisor       : Prof. Pierre Molinaro                                   */
/* Institution      : Ecole Centrale de Nantes                                */
/* ---------------------------------------------------------------------------*/

/*------------------------------- Board Check --------------------------------*/
#ifndef ARDUINO_ARCH_ESP32
  #error "Select an ESP32 board" 
#endif

/*------------------------------- Include files ------------------------------*/
#include "ESP32ACAN.h"

//——————————————————————————————————————————————————————————————————————————————
//  ESP32 CAN Driver
//——————————————————————————————————————————————————————————————————————————————

ESP32ACAN can ;

//——————————————————————————————————————————————————————————————————————————————
//  ESP32 Desired Bit Rate
//——————————————————————————————————————————————————————————————————————————————
static const uint32_t DESIRED_BIT_RATE = 125UL * 1000UL ; // 125 kb/s

//——————————————————————————————————————————————————————————————————————————————
//   SETUP
//——————————————————————————————————————————————————————————————————————————————

void setup() {
 //--- Switch on builtin led
  pinMode (LED_BUILTIN, OUTPUT) ;
  digitalWrite (LED_BUILTIN, HIGH) ;
//--- Start serial
  Serial.begin (115200) ;
//--- Wait for serial (blink led at 10 Hz during waiting)
  while (!Serial) {
    delay (50) ;
    digitalWrite (LED_BUILTIN, !digitalRead (LED_BUILTIN)) ;
  }
//--- Configure ESP32 CAN
  Serial.println ("Configure ESP32 CAN") ;
  ESP32ACANSettings settings (DESIRED_BIT_RATE);           // CAN bit rate 
  settings.mRequestedCANMode = ESP32ACANSettings::LoopBackMode ;  // Select loopback mode
  const uint16_t errorCode = can.begin (settings) ;
  if (errorCode == 0) {
    Serial.print ("Bit Rate prescaler: ") ;
    Serial.println (settings.mBitRatePrescaler) ;
    Serial.print ("Time Segment 1:     ") ;
    Serial.println (settings.mTimeSegment1) ;
    Serial.print ("Time Segment 2:     ") ;
    Serial.println (settings.mTimeSegment2) ;
    Serial.print ("SJW:                ") ;
    Serial.println (settings.mSJW) ;
    Serial.print ("Triple Sampling:    ") ;
    Serial.println (settings.mTripleSampling ? "yes" : "no") ;
    Serial.print ("Actual bit rate:    ") ;
    Serial.print (settings.actualBitRate ()) ;
    Serial.println (" bit/s") ;
    Serial.print ("Exact bit rate ?    ") ;
    Serial.println (settings.exactBitRate () ? "yes" : "no") ;
    Serial.print ("Sample point:       ") ;
    Serial.print (settings.samplePointFromBitStart ()) ;
    Serial.println ("%") ;
    Serial.println ("Configuration OK!");
  }else {
    Serial.print ("Configuration error 0x") ;
    Serial.println (errorCode, HEX) ;
  }
}

//——————————————————————————————————————————————————————————————————————————————
static uint32_t gBlinkLedDate = 0;
static uint32_t gReceivedFrameCount = 0;
static uint32_t gSentFrameCount = 0;

static const uint32_t MESSAGE_COUNT = 10 * 1000;
//——————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————
//   LOOP
//——————————————————————————————————————————————————————————————————————————————

void loop() {

  if (gBlinkLedDate < millis ()) {
    gBlinkLedDate += 500 ;
    digitalWrite (LED_BUILTIN, !digitalRead (LED_BUILTIN)) ;
    Serial.print ("Sent: ") ;
    Serial.print (gSentFrameCount) ;
    Serial.print ("\t") ;
    Serial.print ("Receive: ") ;
    Serial.print (gReceivedFrameCount) ;
    Serial.print ("\t") ;
    Serial.print (" STATUS 0x") ;
    Serial.print (CAN_STATUS, HEX) ;
    Serial.print (" RXERR ") ;
    Serial.print (CAN_RX_ECR, HEX) ;
    Serial.print (" TXERR ") ;
    Serial.println (CAN_TX_ECR, HEX) ;
  }
  
  CANMessage frame ;
  
  while (can.receive (frame)) {
    gReceivedFrameCount += 1;
  }
  
  if (gSentFrameCount < MESSAGE_COUNT) {
    //frame.ext=true;
    //frame.id=0x555;
    frame.len = 1;
    const bool ok = can.tryToSend (frame) ;
    if (ok) {
      gSentFrameCount += 1 ;
      }
  }
}
