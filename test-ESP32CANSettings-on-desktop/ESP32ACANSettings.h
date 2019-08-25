/******************************************************************************/
/* File name        : ESP32ACANSettings.h                                     */
/* Project          : ESP32-CAN-DRIVER                                        */
/* Compiler         : Desktop C++ COMPILER (Visual Studio Code)               */
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
/*   V1.1   | Actual BitRate Calculation                                      */
/*   V1.2   | Added Error Conditions                                          */
/* ---------------------------------------------------------------------------*/

#pragma once

/*------------------------------- Include files ------------------------------*/
#include <stdint.h>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//  ESP32 ACANSettings class
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

class ESP32ACANSettings {

//······················································································································
//   CONSTRUCTOR
//······················································································································

    public: ESP32ACANSettings (const uint32_t inClockFrequencyAPB,
                               const uint32_t inDesiredBitRate,
                               const uint32_t inTolerancePPM = 1000);

//······················································································································
//   CAN BIT TIMING
//······················································································································

    public: uint32_t mSourceClockAPB ;               // In Hz
    public: uint32_t mDesiredBitRate ;               // In kb/s
    public: uint8_t mBitRatePrescaler ;              // 2...128
    public: uint8_t mTQcount ;                       // 3...25
    public: uint8_t mTimeSegment1 ;                  // 2...16
    public: uint8_t mTimeSegment2 ;                  // 1...8
    public: uint8_t mSJW ;                           // 1...4;
    public: bool mTripleSampling = false;            // true --> triple sampling, false --> single sampling
    public: bool mBitRateClosedToDesiredRate = true; // The above configuration is correct


//······················································································································
//   Max values
//······················································································································
    public: static const uint8_t Sync_Seg            = 1 ;    // Fixed Sync Segment
    public: static const uint8_t MAX_BRP             = 128 ;
    public: static const uint8_t MIN_BRP             = 2 ;
    public: static const uint8_t MAX_TQ              = 25 ;
    public: static const uint8_t MIN_TQ              = 3 ;
    public: static const uint8_t MAX_TIME_SEGMENT_1  = 16 ;
    public: static const uint8_t MAX_TIME_SEGMENT_2  = 8 ;
    public: static const uint8_t MAX_SJW             = 4 ;

//······················································································································
//    Compute actual bit rate
//······················································································································

    public: uint32_t actualBitRate(void) const;

//······················································································································
//    Distance between actual bit rate and requested bit rate (in ppm, part-per-million)
//······················································································································

    public: uint32_t ppmFromDesiredBitRate(void) const;

//······················································································································
//    Distance of sample point from bit start (in ppc, part-per-cent, denoted by %)
//······················································································································

    public: uint32_t samplePointFromBitStart(void) const;

//······················································································································
//    Bit settings are consistent ? (returns 0 if ok)
//······················································································································

    public: uint16_t CANBitSettingConsistency (void) const ;


//······················································································································
//    Constants returned by CANBitSettingConsistency
//······················································································································

    public: static const uint16_t kBitRatePrescalerIsLowerThan2                = 1 <<  0 ;
    public: static const uint16_t kBitRatePrescalerIsGreaterThan128            = 1 <<  1 ;
    public: static const uint16_t kTimeSegment1IsZero                          = 1 <<  2 ;
    public: static const uint16_t kTimeSegment1IsGreaterThan16                 = 1 <<  3 ;
    public: static const uint16_t kTimeSegment2IsZero                          = 1 <<  4 ;
    public: static const uint16_t kTimeSegment2IsGreaterThan8                  = 1 <<  5 ;
    public: static const uint16_t kTimeSegment1Is1AndTripleSampling            = 1 <<  6 ;
    public: static const uint16_t kSJWIsZero                                   = 1 <<  7 ;
    public: static const uint16_t kSJWIsGreaterThan4                           = 1 <<  8 ;

//······················································································································

} ;
