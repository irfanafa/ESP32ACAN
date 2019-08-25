/******************************************************************************/
/* File name        : ESP32ACAN.h                                             */
/* Project          : ESP32-CAN-DRIVER                                        */
/* Description      : ESP32 CAN Driver                                        */
/* ---------------------------------------------------------------------------*/
/* Copyright        : Copyright © 2019 Pierre Molinaro. All rights reserved.  */
/* ---------------------------------------------------------------------------*/
/* Author           : Mohamed Irfanulla                                       */
/* Supervisor       : Prof. Pierre Molinaro                                   */
/* Institution      : Ecole Centrale de Nantes                                */
/* ---------------------------------------------------------------------------*/
/*  Version | Change                                                          */
/* ---------------------------------------------------------------------------*/
/*   V1.0   | Creation                                                        */
/*   V1.1   | Working by Polling (Extended Frames)                            */
/*   V1.2   | Handling Standard Frames                                        */
/*   V1.3   | Added Interrupt Handlers                                        */
/*   V2.0   | Acceptance Filter Settings                                      */
/* ---------------------------------------------------------------------------*/

#pragma once

/*------------------------------- Include files ------------------------------*/
#include "ESP32CANRegisters.h"
#include "ESP32ACANSettings.h"
#include "CANMessage.h"
#include "ACANBuffer16.h"
#include "ESP32AcceptanceFilters.h"
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//   ESP32 CAN class
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

class ESP32ACAN {

//······················································································································
//   CONSTRUCTOR
//······················································································································

  public: ESP32ACAN (void) ; 

//······················································································································
//    Initialisation: returns 0 if ok, otherwise see error codes below
//······················································································································
    /* Default Filter Settings*/
  public: uint32_t begin (const ESP32ACANSettings & inSettings) ;

    /* With Acceptance Filter Settings */
  public: uint32_t begin (const ESP32ACANSettings & inSettings,
                          const ESP32ACANFilter inFilterSettings) ;


//······················································································································
//    CAN  Configuration Private Methods
//······················································································································
  private: void setGPIOPins (void);
  private: void setBitTimingSettings(const ESP32ACANSettings &inSettings) ;
  private: void setRequestedCANMode (const ESP32ACANSettings &inSettings, const ESP32ACANFilter inFilter) ;
  private: void setAcceptanceFilter (const ESP32ACANFilter inFilter) ;

  private: uint16_t internalBeginConfiguration (const ESP32ACANSettings & inSettings,
                                                const ESP32ACANFilter inFilterSettings) ;
//······················································································································
//    Receiving messages
//······················································································································

 //--- Handling messages to send and receiving messages
  public: bool mReceivebyPoll = false;
  public: bool receivebypoll (CANMessage &outMessage) ;
  public: bool receive (CANMessage &outMessage) ;
  public: static void handleMessages (CANMessage &outFrame) ;
  
//······················································································································
//    Receive buffer
//······················································································································

  private: ACANBuffer16 mDriverReceiveBuffer ;

  public: inline uint16_t driverreceiveBufferSize (void) const { return mDriverReceiveBuffer.size () ;  }
  public: inline uint16_t driverreceiveBufferCount (void) const { return mDriverReceiveBuffer.count() ;  }
  public: inline uint16_t driverreceiveBufferPeakCount (void) const { return mDriverReceiveBuffer.peakCount () ; }


//······················································································································
//    Transmitting messages
//······················································································································
  public: bool tryToSendbypoll (const CANMessage & inMessage) ;
  public: bool tryToSend (const CANMessage & inMessage) ;
  public: static void internalSendMessage (const CANMessage & inFrame);

//······················································································································
//    Transmit buffer
//······················································································································

  private: ACANBuffer16 mDriverTransmitBuffer ;
  private: bool mDriverSending;
  public: bool mSendbyPoll = false;

  public: inline uint16_t driverTransmitBufferSize (void) const { return mDriverTransmitBuffer.size () ; }
  public: inline uint16_t driverTransmitBufferCount (void) const { return mDriverTransmitBuffer.count () ; }
  public: inline uint16_t driverTransmitBufferPeakCount (void) const { return mDriverTransmitBuffer.peakCount () ; }


//······················································································································
//    Error codes returned by begin
//······················································································································
  public: static const uint32_t kNotInRestModeInConfiguration             = 1 <<  0 ;
  public: static const uint32_t kCANRegistersError                        = 1 <<  1 ;
  public: static const uint32_t kTooFarFromDesiredBitRate                 = 1 <<  2 ;
  public: static const uint32_t kInconsistentBitRateSettings              = 1 <<  3 ;
  public: static const uint32_t kCannotAllocateDriverReceiveBuffer        = 1 <<  4 ;
  public: static const uint32_t kCannotAllocateDriverTransmitBuffer       = 1 <<  5 ;

//······················································································································
//    Interrupt Handler
//······················································································································

  public: static void isr (void *arg) ;

  public: void handleTXInterrupt(void) ;
  public: void handleRXInterrupt(void) ;

//······················································································································
//    Properties
//······················································································································
 
  public: SemaphoreHandle_t mISRSemaphore ;

//······················································································································
//    No Copy
//······················································································································

  private: ESP32ACAN (const ESP32ACAN &) = delete ;
  private: ESP32ACAN & operator = (const ESP32ACAN &) = delete ;

};
