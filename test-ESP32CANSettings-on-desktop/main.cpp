/******************************************************************************/
/* File name        : main.cpp                                                */
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
/*   V1.1   | Exact Bit Rate Computation                                      */
/*   V1.2   | Exhaustive Search for All Exact Settings                        */
/* ---------------------------------------------------------------------------*/

/*------------------------------- Include files ------------------------------*/
#include <iostream>
#include "ESP32ACANSettings.cpp"
#include "Set.cpp"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

using namespace std;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
static const uint32_t inSourceClockAPB = 80 * 1000 * 1000;  // CAN Controller Source Clock (APB 80 MHz)

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static const uint32_t firstTestedBitRate = 1;          // 1 bit/s
static const uint32_t lastTestedBitRate = 1000 * 1000; // 1 Mbit/s

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void compute(const uint32_t inDesiredBaudRate) {
    ESP32ACANSettings settings(inSourceClockAPB,inDesiredBaudRate);
    cout << "--------------------------------------------------" << endl;
    cout << " Source Clock APB  : " << settings.mSourceClockAPB << " Hz" << endl;
    cout << " Desired baud rate : " << settings.mDesiredBitRate << " bit/s" << endl;
    cout << " BRP               : " << (unsigned)settings.mBitRatePrescaler << endl;
    cout << " TQ                : " << (unsigned)settings.mTQcount << endl;
    cout << " TimeSegment1      : " << (unsigned)settings.mTimeSegment1 << endl;
    cout << " TimeSegment2      : " << (unsigned)settings.mTimeSegment2 << endl;
    cout << " SJW               : " << (unsigned)settings.mSJW << endl;
    cout << " Sampling          : " << (settings.mTripleSampling ? "triple" : "single") << endl;
    cout << " Settings OK       : " << (settings.mBitRateClosedToDesiredRate ? "yes" : "no") << endl;
    cout << " Actual baud rate  : " << settings.actualBitRate() << " bit/s" << endl;
    cout << " ppm               : " << settings.ppmFromDesiredBitRate() << endl;
    cout << " Sample Point      : " << settings.samplePointFromBitStart() << "%" << endl;
    cout << " Bit setting closed to desired bit rate ok: " << ((settings.ppmFromDesiredBitRate() < 1000) ? "yes" : "no") << endl << endl;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static void exploreAllSettings (void) {
  cout << "Explore all settings" << endl ;
  for (uint32_t br = firstTestedBitRate ; br <= lastTestedBitRate ; br ++) {
    ESP32ACANSettings settings (inSourceClockAPB, br) ;
    const uint32_t errorCode = settings.CANBitSettingConsistency () ;
    if (errorCode != 0) {
      cout << "Error 0x" << hex << errorCode << " for br : " << dec << br << endl ;
      if ((errorCode & ESP32ACANSettings::kBitRatePrescalerIsLowerThan2) != 0) {
        cout << "  -> kBitRatePrescalerIsZero" << endl ;
      }
      if ((errorCode & ESP32ACANSettings::kBitRatePrescalerIsGreaterThan128) != 0) {
        cout << "  -> kBitRatePrescalerIsGreaterThan64" << endl ;
      }
      if ((errorCode & ESP32ACANSettings::kTimeSegment1IsZero) != 0) {
        cout << "  -> kTimeSegment1IsZero" << endl ;
      }
      if ((errorCode & ESP32ACANSettings::kTimeSegment1IsGreaterThan16) != 0) {
        cout << "  -> kTimeSegment1IsGreaterThan8" << endl ;
      }
      if ((errorCode & ESP32ACANSettings::kTimeSegment2IsZero) != 0) {
        cout << "  -> kTimeSegment2IsLowerThan2" << endl ;
      }
      if ((errorCode & ESP32ACANSettings::kTimeSegment2IsGreaterThan8) != 0) {
        cout << "  -> kTimeSegment2IsGreaterThan8" << endl ;
      }
      if ((errorCode & ESP32ACANSettings::kTimeSegment1Is1AndTripleSampling) != 0) {
        cout << "  -> kTimeSegment1Is1AndTripleSampling" << endl ;
      }
      if ((errorCode & ESP32ACANSettings::kSJWIsZero) != 0) {
        cout << "  -> kSJWIsZero" << endl ;
      }
      if ((errorCode & ESP32ACANSettings::kSJWIsGreaterThan4) != 0) {
        cout << "  -> kSJWIsGreaterThan4" << endl ;
      }
      cout << "  BRP : " << (unsigned) settings.mBitRatePrescaler << endl ;
      cout << "  TQ  : " << (unsigned)settings.mTQcount << endl;
      cout << "  Segment1: " << (unsigned) settings.mTimeSegment1 << endl ;
      cout << "  Segment2: " << (unsigned) settings.mTimeSegment2 << endl ;
      cout << "  SJW     : " << (unsigned) settings.mSJW << endl ;
      cout << "  Sampling: " << (settings.mTripleSampling ? "triple" : "single") << endl ;
      cout << "  Actual baud rate: " << settings.actualBitRate () << " bit/s" << endl ;
      cout << "  ppm: " << settings.ppmFromDesiredBitRate () << endl ;
      cout << "  Sample Point: " << settings.samplePointFromBitStart () << "%" << endl ;
      cout << "  Bit setting closed to desired bit rate ok: " << ((settings.ppmFromDesiredBitRate () < 1000) ? "yes" : "no") << endl ;
      exit (1) ;
    }
  }
  cout << "  All Settings Explored, Ok" << endl <<endl ;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static std::vector <uint32_t> allPossibleBitRates (void) {
  cout << "All bit rates" << endl ;
  Set allBitRates (lastTestedBitRate) ;
  for (uint32_t br = firstTestedBitRate ; br <= lastTestedBitRate ; br ++) {
    ESP32ACANSettings settings (inSourceClockAPB, br) ;
    if (settings.mBitRateClosedToDesiredRate) {
      allBitRates.insert (br) ;
    }
  }
  const std::vector <uint32_t> result = allBitRates.values () ;
  cout << "  Completed, " << result.size () << " valid settings" << endl << endl;
  return result ;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//  EXACT SETTINGS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static std::vector <uint32_t> allExactSettings (void) {
  cout << "All exact bit rates" << endl ;
  Set allExactBitRates (lastTestedBitRate) ;
  for (uint32_t br = firstTestedBitRate ; br <= lastTestedBitRate ; br ++) {
    ESP32ACANSettings settings (inSourceClockAPB, br, 0) ;
    if (settings.mBitRateClosedToDesiredRate) {
      allExactBitRates.insert (br) ;
    }
  }
  const std::vector <uint32_t> result = allExactBitRates.values () ;
  cout << "  Completed, " << result.size () << " exact settings" << endl <<endl ;
  return result ;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static std::vector <uint32_t> exhaustiveSearchOfAllExactSettings (void) {
  cout << "Exact settings ehaustive search" << endl ;
  Set allExactBitRates (lastTestedBitRate) ;
  const uint32_t maxTQ = ESP32ACANSettings::Sync_Seg + ESP32ACANSettings::MAX_TIME_SEGMENT_1 + ESP32ACANSettings::MAX_TIME_SEGMENT_2 ;
  const uint32_t SYSCLOCK = inSourceClockAPB ;
  for (uint32_t brp = 1 ; brp <= ESP32ACANSettings::MAX_BRP ; brp ++) {
    for (uint32_t TQCount = 3 ; TQCount <= maxTQ ; TQCount ++) {
      const uint32_t bitRate = SYSCLOCK / brp / TQCount ;
      const bool exact = (bitRate * brp * TQCount) == SYSCLOCK ;
      if (exact && (bitRate <= 1 * 1000 * 1000)) {
        allExactBitRates.insert (bitRate) ;
      }
    }
  }
  const std::vector <uint32_t> result = allExactBitRates.values () ;
  cout << "  Exhaustive search completed, " << result.size () << " exact settings" << endl <<endl ;
  return result ;
}


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//   MAIN
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

int main(int /* argc */, const char * /* argv */ []) {

    compute(125 * 1000);
    compute(250 * 1000);
    compute(500 * 1000);
    compute(1000 * 1000);

    //--- Explore all settings
    exploreAllSettings();

    //--- Check valid settings
    allPossibleBitRates () ;
    //--- Check all exact settings
    const std::vector <uint32_t> exactBitRates = allExactSettings () ;

    const std::vector <uint32_t> exhaustiveExactBitRates = exhaustiveSearchOfAllExactSettings () ;
    if (exactBitRates != exhaustiveExactBitRates) {
        cout << "  EQUALITY ERROR" << endl ;
        exit (1) ;
    }
    else{
        for (size_t i=0 ; i<exactBitRates.size () ; i++) {
            const uint32_t bitRate = exactBitRates.at (i) ;
            if ((bitRate % 1000) == 0) {
                cout << "  " << (bitRate / 1000) << " kbit/s" << endl ;
            }else{
                cout << "  " << bitRate << " bit/s" << endl ;
            }
        }
    }
  return 0;
}
