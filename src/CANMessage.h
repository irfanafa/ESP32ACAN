/******************************************************************************/
/* File name        : CANMessage.h                                            */
/* Project          : ESP32-CAN-DRIVER                                        */
/* Description      : GENERIC CAN MESSAGE                                     */
/*                    This file is common for the CAN Controllers used        */
/*                    MCP2515,MCP2517 https://github.com/pierremolinaro       */
/* ---------------------------------------------------------------------------*/
/* Copyright        : Copyright © 2019 Pierre Molinaro. All rights reserved.  */
/* ---------------------------------------------------------------------------*/
/* Author           : Mohamed Irfanulla                                       */
/* Supervisor       : Prof. Pierre Molinaro                                   */
/* Institution      : Ecole Centrale de Nantes                                */
/* ---------------------------------------------------------------------------*/

#ifndef GENERIC_CAN_MESSAGE_DEFINED
#define GENERIC_CAN_MESSAGE_DEFINED

/*------------------------------- Include files ------------------------------*/
#include <Arduino.h>

//——————————————————————————————————————————————————————————————————————————————

class CANMessage {
  public : uint32_t id = 0;  // Frame identifier
  public : bool ext = false ; // false -> standard frame, true -> extended frame
  public : bool rtr = false ; // false -> data frame, true -> remote frame
  public : uint8_t idx = 0 ;  // This field is used by the driver
  public : uint8_t len = 0 ;  // Length of data (0 ... 8)
  public : union {
    uint64_t data64     ; // Caution: subject to endianness
    uint32_t data32 [2] ; // Caution: subject to endianness
    uint16_t data16 [4] ; // Caution: subject to endianness
    uint8_t  data   [8] = {0, 0, 0, 0, 0, 0, 0, 0} ;
  } ;
} ;

typedef enum {kStandard, kExtended} tFrameFormat ;
typedef enum {kData, kRemote} tFrameKind ;
typedef void (*ACANCallBackRoutine) (const CANMessage & inMessage) ;


#endif
