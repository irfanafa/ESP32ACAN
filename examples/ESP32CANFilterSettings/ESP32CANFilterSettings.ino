/******************************************************************************/
/* File name        : ESP32CANFilterSettings.ino                              */
/* Project          : ESP32-CAN-DRIVER                                        */
/* Description      : ESP32 CAN Acceptance Filter Settings                    */
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

ESP32ACAN can;

//——————————————————————————————————————————————————————————————————————————————
//  ESP32 Desired Bit Rate
//——————————————————————————————————————————————————————————————————————————————
static const uint32_t DESIRED_BIT_RATE = 125UL * 1000UL; // 125 kb/s

//——————————————————————————————————————————————————————————————————————————————
//   SETUP
//——————————————————————————————————————————————————————————————————————————————

void setup()
{
  //--- Switch on builtin led
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  //--- Start serial
  Serial.begin(115200);
  //--- Wait for serial (blink led at 10 Hz during waiting)
  while (!Serial)
  {
    delay(50);
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }
  //--- Configure ESP32 CAN
  Serial.println("Configure ESP32 CAN");
  ESP32ACANSettings settings(DESIRED_BIT_RATE);                 // CAN bit rate
  settings.mRequestedCANMode = ESP32ACANSettings::LoopBackMode; // Select loopback mode
  settings.mControllMessageByMethod = ESP32ACANSettings::InterruptControlled;
  const ESP32ACANFilter filter = acceptSingleFilterStandard(0x205, 0, 0, 0x0A0, 0xFF, 0xFF); // Single Filter
  //const ESP32ACANFilter filter = acceptDualFilterStandard(0x205,0x2A5,0,0x000,0x000,0); // Dual Filter
  const uint16_t errorCode = can.begin(settings, filter);
  if (errorCode == 0)
  {
    Serial.print("Bit Rate prescaler: ");
    Serial.println(settings.mBitRatePrescaler);
    Serial.print("Time Segment 1:     ");
    Serial.println(settings.mTimeSegment1);
    Serial.print("Time Segment 2:     ");
    Serial.println(settings.mTimeSegment2);
    Serial.print("SJW:                ");
    Serial.println(settings.mSJW);
    Serial.print("Triple Sampling:    ");
    Serial.println(settings.mTripleSampling ? "yes" : "no");
    Serial.print("Actual bit rate:    ");
    Serial.print(settings.actualBitRate());
    Serial.println(" bit/s");
    Serial.print("Exact bit rate ?    ");
    Serial.println(settings.exactBitRate() ? "yes" : "no");
    Serial.print("Sample point:       ");
    Serial.print(settings.samplePointFromBitStart());
    Serial.println("%");
    Serial.println("Configuration OK!");
  }
  else
  {
    Serial.print("Configuration error 0x");
    Serial.println(errorCode, HEX);
  }
}

//——————————————————————————————————————————————————————————————————————————————
static uint32_t gBlinkLedDate = 0;
//——————————————————————————————————————————————————————————————————————————————

//——————————————————————————————————————————————————————————————————————————————
//   LOOP
//——————————————————————————————————————————————————————————————————————————————

void loop()
{
  CANMessage frame;
  if (gBlinkLedDate < millis())
  {
    gBlinkLedDate += 500;
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }

  for (uint32_t sendId = 0x000; sendId <= CAN_MSG_STD_ID; sendId++)
  {
    frame.id = sendid;
    can.tryToSend(frame);
  }
  if (can.receive(frame))
  {
    Serial.print("Received ID : ");
    Serial.println(frame.id, HEX);
  }
}