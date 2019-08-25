/******************************************************************************/
/* File name        : ESP32ACANSettings.cpp                                   */
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

/*------------------------------- Include files ------------------------------*/
#include "ESP32ACANSettings.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//    CAN Settings
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

ESP32ACANSettings::ESP32ACANSettings(const uint32_t inClockFrequencyAPB,
                                     const uint32_t inDesiredBitRate,
                                     const uint32_t inTolerancePPM) :

mSourceClockAPB(inClockFrequencyAPB),
mDesiredBitRate(inDesiredBitRate) {
     const uint32_t clock = mSourceClockAPB ;   // CAN Source Clock APB
        uint32_t TQCount = MAX_TQ ;             // TQ: min(3) max(25)
        uint32_t bestBRP = MAX_BRP ;            // Setting for slowest bit rate
        uint32_t bestTQCount = MAX_TQ ;         // Setting for slowest bit rate
        uint32_t smallestError = UINT32_MAX ;

        uint32_t BRP = clock / (inDesiredBitRate * TQCount); // BRP: min(2) max(128)
        //--- Loop for finding best BRP and best TQCount
        while ((TQCount >= MIN_TQ) && (BRP <= MAX_BRP))
        {
            //--- Compute error using BRP (caution: BRP should be > 2 and even number)
            if (BRP > MIN_BRP)
            {
                const uint32_t error = clock - (inDesiredBitRate * TQCount * BRP); // error is always >= 0
                if (error < smallestError)
                {
                    smallestError = error;
                    bestBRP = BRP;
                    bestTQCount = TQCount;
                }
            }
            //--- Compute error using BRP+1 (caution: BRP+1 should be <= 128 and even number)
            if (BRP < MAX_BRP)
            {
                const uint32_t error = (inDesiredBitRate * TQCount * (BRP + 1)) - clock; // error is always >= 0
                if (error < smallestError)
                {
                    smallestError = error;
                    bestBRP = BRP + 1;
                    bestTQCount = TQCount;
                }
            }
            //--- Continue with next value of TQCount
            TQCount--;
            BRP = clock / (inDesiredBitRate * TQCount);
        }

    //--- Set the BRP
    mBitRatePrescaler = (uint8_t)bestBRP;
    mTQcount = (uint8_t)bestTQCount;

    //--- Compute PS2 (1 <= TSeg2 <= 8)
    //----Sampling Point must be in the range 60% - 90%
    uint8_t Tseg2 = bestTQCount / 5;  // For sampling point at 80%

        if (Tseg2 == 0) {
            Tseg2 = 1;
        }
        else if (Tseg2 > MAX_TIME_SEGMENT_2) {
            Tseg2 = MAX_TIME_SEGMENT_2;
        }

    //--- Compute PS1 (1 <= PS1 <= 16)
    uint8_t Tseg1 = bestTQCount - Tseg2 - Sync_Seg;

        if (Tseg1 > MAX_TIME_SEGMENT_1) {
            Tseg2 += Tseg1 - MAX_TIME_SEGMENT_1;
            Tseg1 = MAX_TIME_SEGMENT_1;
        }

    // set Timing Segment 1 and 2
    mTimeSegment1 = (uint8_t)Tseg1;
    mTimeSegment2 = (uint8_t)Tseg2;

    // SJW (1...4) min of Tseg2
    mSJW = (mTimeSegment2 > 4) ? 4 : (3);

    //--- Triple sampling ?
    mTripleSampling = (inDesiredBitRate <= 125000) && (mTimeSegment1 >= 2);

    //--- Final check of the configuration
    const uint32_t W = bestTQCount * mDesiredBitRate * mBitRatePrescaler;
    const uint64_t diff = (clock > W) ? (clock - W) : (W - clock);
    const uint64_t ppm = (uint64_t)(1000UL * 1000UL); // UL suffix is required for Arduino Uno
    mBitRateClosedToDesiredRate = (diff * ppm) <= (((uint64_t)W) * inTolerancePPM);
};

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32_t ESP32ACANSettings::actualBitRate(void) const {
    const uint32_t TQCount = Sync_Seg + mTimeSegment1 + mTimeSegment2;
    return mSourceClockAPB / mBitRatePrescaler / TQCount;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32_t ESP32ACANSettings::ppmFromDesiredBitRate(void) const {
    const uint32_t TQCount = Sync_Seg + mTimeSegment1 + mTimeSegment2;
    const uint32_t W = TQCount * mDesiredBitRate * mBitRatePrescaler;
    const uint64_t diff = (mSourceClockAPB > W) ? (mSourceClockAPB - W) : (W - mSourceClockAPB);
    const uint64_t ppm = (uint64_t)(1000UL * 1000UL); // UL suffix is required for Arduino Uno
    return (uint32_t)((diff * ppm) / W);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint32_t ESP32ACANSettings::samplePointFromBitStart(void) const {
    const uint32_t TQCount = Sync_Seg + mTimeSegment1 + mTimeSegment2;
    const uint32_t samplePoint = Sync_Seg + mTimeSegment1 - mTripleSampling;
    const uint32_t partPerCent = 100;
    return (samplePoint * partPerCent) / TQCount;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

uint16_t ESP32ACANSettings::CANBitSettingConsistency (void) const {
  uint16_t errorCode = 0 ;              // No error
  if (mBitRatePrescaler < MIN_BRP) {
    errorCode |= kBitRatePrescalerIsLowerThan2 ;
  }else if (mBitRatePrescaler > MAX_BRP) {
    errorCode |= kBitRatePrescalerIsGreaterThan128 ;
  }
  if (mTimeSegment1 == 0) {
    errorCode |= kTimeSegment1IsZero ;
  }else if ((mTimeSegment1 == 1) && mTripleSampling) {
    errorCode |= kTimeSegment1Is1AndTripleSampling ;
  }else if (mTimeSegment1 > MAX_TIME_SEGMENT_1) {
    errorCode |= kTimeSegment1IsGreaterThan16 ;
  }
  if (mTimeSegment2 == 0) {
    errorCode |= kTimeSegment2IsZero ;
  }else if (mTimeSegment2 > MAX_TIME_SEGMENT_2) {
    errorCode |= kTimeSegment2IsGreaterThan8 ;
  }
  if (mSJW == 0) {
    errorCode |= kSJWIsZero ;
  }else if (mSJW > MAX_SJW) {
    errorCode |= kSJWIsGreaterThan4 ;
  }
  return errorCode ;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
