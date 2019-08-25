/******************************************************************************/
/* File name        : Set.cpp                                                 */
/* Project          : ESP32-CAN-DRIVER                                        */
/* Compiler         : Desktop C++ COMPILER (Visual Studio Code)               */
/* ---------------------------------------------------------------------------*/
/* Copyright        : Copyright © 2019 Pierre Molinaro. All rights reserved.  */
/* ---------------------------------------------------------------------------*/
/* Author           : Mohamed Irfanulla                                       */
/* Supervisor       : Prof. Pierre Molinaro                                   */
/* Institution      : Ecole Centrale de Nantes                                */
/* ---------------------------------------------------------------------------*/
/*  Version |  Date       | Change                                            */
/* ---------------------------------------------------------------------------*/
/*   V1.0   | 25 Apr 2019 | Creation                                          */
/* ---------------------------------------------------------------------------*/

/*------------------------------- Include files ------------------------------*/
#include <stdint.h>
#include "Set.h"

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Set::Set (const uint32_t inMaxValue) :
mMaxValue (inMaxValue),
mArray (new uint64_t [(inMaxValue + sizeof (uint64_t) - 1) / sizeof (uint64_t)]) {
  for (size_t i=0 ; i < ((inMaxValue + sizeof (uint64_t) - 1) / sizeof (uint64_t)) ; i++) {
    mArray [i] = 0 ;
  }
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

Set::~Set() {
  delete [] mArray ;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool Set::insert (const uint32_t inItem) {
  const size_t index = inItem / 64 ;
  const uint64_t mask = 1ULL << (inItem % 64) ;
  const bool alreadyInSet = (mArray [index] & mask) != 0 ;
  mArray [index] |= mask ;
  return alreadyInSet ;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

std::vector <uint32_t> Set::values (void) const {
  std::vector <uint32_t> result ;
  for (uint32_t i=0 ; i < ((mMaxValue + sizeof (uint64_t) - 1) / sizeof (uint64_t)) ; i++) {
    uint64_t v = mArray [i] ;
    uint32_t idx = i * 64 ;
    while (v > 0) {
      if ((v & 1) != 0) {
        result.push_back (idx) ;
      }
      idx += 1 ;
      v >>= 1 ;
    }
  }
  return result ;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
