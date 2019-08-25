/******************************************************************************/
/* File name        : Set.h                                                   */
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


#pragma once

/*------------------------------- Include files ------------------------------*/
#include <stdlib.h>
#include <vector>

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//  Set class
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

class Set {
//--- Default constructor
  public: Set (const uint32_t inMaxValue) ;

//--- Destructor
  public: ~Set (void) ;

//--- Accessors
  public : std::vector <uint32_t> values (void) const ;

//--- Set operations
  public : bool insert (const uint32_t inItem) ;

//--- Private properties
  private: size_t mMaxValue ;
  private: uint64_t * mArray ;

//--- No copy
  private: Set (const Set &inOperand) ;
  private: Set & operator = (const Set & inOperand) ;
};

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
