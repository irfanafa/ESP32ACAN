/******************************************************************************/
/* File name        : ESP32CANTestwith-ACAN2515.ino                           */
/* Project          : ESP32-CAN-DRIVER                                        */
/* Description      : ESP32 CAN Test with MCP 2515 CAN Controller             */
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
#include <ACAN2515.h>


static const byte MCP_SCK = 26 ; // SCK input of MCP2517
static const byte MCP_SDI = 19 ; // SI input of MCP2517
static const byte MCP_SDO = 18 ; // SO output of MCP2517

static const byte MCP2515_CS  = 17 ; // CS input of MCP2515
static const byte MCP2515_INT = 0 ; // INT output of MCP2515

//——————————————————————————————————————————————————————————————————————————————
//  MCP2515 Driver object
//——————————————————————————————————————————————————————————————————————————————

ACAN2515 can2515 (MCP2515_CS, SPI, MCP2515_INT) ;

//——————————————————————————————————————————————————————————————————————————————
//  MCP2515 Quartz: adapt to your design
//——————————————————————————————————————————————————————————————————————————————

static const uint32_t MCP2515_QUARTZ_FREQUENCY = 20UL * 1000UL * 1000UL ; // 20 MHz

//——————————————————————————————————————————————————————————————————————————————
//  ESP32 CAN Driver
//——————————————————————————————————————————————————————————————————————————————

ESP32ACAN canESP32 ;

//——————————————————————————————————————————————————————————————————————————————
// Desired Bit Rate
//——————————————————————————————————————————————————————————————————————————————
static const uint32_t DESIRED_BIT_RATE = 25UL * 1000UL ; // 1 Mb/s

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
  settings.mRequestedCANMode = ESP32ACANSettings::NormalMode ;  
  const uint16_t errorCode = canESP32.begin (settings) ;
  if (errorCode == 0) {
    Serial.println ("Configuration ESP32 OK!");
  }else{
    Serial.print ("Configuration error 0x") ;
    Serial.println (errorCode, HEX) ;
  }

  //--- Begin SPI1
  SPI.begin (MCP_SCK, MCP_SDO, MCP_SDI) ;
  Serial.println ("Configure ACAN2515") ;
  ACAN2515Settings settings2515 (MCP2515_QUARTZ_FREQUENCY, DESIRED_BIT_RATE) ;
  const uint32_t errorCode2515 = can2515.begin (settings2515, [] {can2515.isr () ; }) ;
  if (errorCode2515 == 0) {
    Serial.println ("ACAN2515 configuration: ok") ;
  }else{
    Serial.print ("Configuration error 0x") ;
    Serial.println (errorCode2515, HEX) ;
  }
}

//——————————————————————————————————————————————————————————————————————————————
static uint32_t gBlinkLedDate = 0;
static uint32_t gReceivedFrameCount2515 = 0 ;
static uint32_t gReceivedFrameCountESP32 = 0 ;
static uint32_t gSentFrameCount2515 = 0 ;
static uint32_t gSentFrameCountESP32 = 0 ;

static const uint32_t MESSAGE_COUNT = 10 * 1000 ;
//——————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————
//   LOOP
//——————————————————————————————————————————————————————————————————————————————

void loop() {

  if (gBlinkLedDate < millis ()) {
    gBlinkLedDate += 500 ;
    digitalWrite (LED_BUILTIN, !digitalRead (LED_BUILTIN)) ;
    Serial.print("SentESP32: ");
    Serial.print(gSentFrameCountESP32);
    Serial.print("\t");
    Serial.print("Receive2515: ");
    Serial.print(gReceivedFrameCount2515);
    Serial.print("\t");
    Serial.print("Sent2515: ");
    Serial.print(gSentFrameCount2515);
    Serial.print("\t");
    Serial.print("ReceiveESP32: ");
    Serial.println(gReceivedFrameCountESP32);
  }
  
  CANMessage frame ;
  
  if (gSentFrameCountESP32 < MESSAGE_COUNT) {
    //frame.ext=true;
    frame.id= millis () & 0x7FE;
    frame.len = 8;
    const bool ok = canESP32.tryToSend (frame) ;
    if (ok) {
      gSentFrameCountESP32 += 1 ;
    }
  }

  if (gSentFrameCount2515 < MESSAGE_COUNT) {
  //--- Make an odd identifier for 2515
    //frame.ext=true;
    frame.id = millis () & 0x7FE ;
    frame.id |= 1 ;
    frame.len = 8 ;
  //--- Send frame via the MCP2515, using transmit buffer 0
    bool ok = can2515.tryToSend (frame) ;
    if (ok) {
      gSentFrameCount2515 += 1 ;
    }
  }
  
  while (can2515.receive (frame)) {
    gReceivedFrameCount2515 += 1 ;
  }
  while (canESP32.receive (frame)) {
    gReceivedFrameCountESP32 += 1 ;
  }    
}
