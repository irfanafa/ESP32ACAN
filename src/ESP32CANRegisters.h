/******************************************************************************/
/* File name        : ESP32ACANRegisters.h                                    */
/* Project          : ESP32-CAN-DRIVER                                        */
/* Description      : ESP32 CAN Peripheral Registers                          */
/* ---------------------------------------------------------------------------*/
/* Copyright        : Copyright © 2019 Pierre Molinaro. All rights reserved.  */
/* ---------------------------------------------------------------------------*/
/* Author           : Mohamed Irfanulla                                       */
/* Supervisor       : Prof. Pierre Molinaro                                   */
/* Institution      : Ecole Centrale de Nantes                                */
/* ---------------------------------------------------------------------------*/
/*  Version |  Date       | Change                                            */
/* ---------------------------------------------------------------------------*/
/*   V1.0   | 20 May 2019 | Configuration Registers                           */
/*   V1.1   | 03 Jun 2019 | Added Shared Registers                            */
/*   V1.2   | 24 Jun 2019 | Registers defined as 32-bit                       */
/* ---------------------------------------------------------------------------*/


#pragma once

/*------------------------------- Include files ------------------------------*/ 
//---Include Headers defined for ESP32 RTOS; Source ESP-IDF
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_intr.h"
#include "soc/dport_reg.h"
#include "driver/periph_ctrl.h"


//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//   ESP32 CAN REGISTERS
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

static const uint32_t ESP32CAN_BASE = 0x3FF6B000;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

typedef volatile uint32_t vuint32_t;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

/*------------------------------- Configuration and Control Registers ------------------------*/

  #define CAN_MODE        (*((vuint32_t *)(ESP32CAN_BASE)))
    
    /* Bit definitions and macros for CAN_MODE */
    static const uint32_t CAN_MODE_RESET      = 0x01 ;
    static const uint32_t CAN_MODE_LISTENONLY = 0x02 ;
    static const uint32_t CAN_MODE_SELFTEST   = 0x04 ;
    static const uint32_t CAN_MODE_ACCFILTER  = 0x08 ;

  #define CAN_CMD         (*((vuint32_t *)(ESP32CAN_BASE + 0x004)))
    
    /* Bit definitions and macros for CAN_COMMAND */
    static const uint32_t CAN_CMD_TX_REQ             = 0x01 ;
    static const uint32_t CAN_CMD_ABORT_TX           = 0x02 ;
    static const uint32_t CAN_CMD_RELEASE_RXB        = 0x04 ;
    static const uint32_t CAN_CMD_CLEAR_DATAOVERRUN  = 0x08 ;
    static const uint32_t CAN_CMD_SELF_RX_REQ        = 0x10 ;  

  #define CAN_STATUS      (*((const vuint32_t *)(ESP32CAN_BASE + 0x008)))
    
    /* Bit definitions and macros for CAN_STATUS */
    static const uint32_t CAN_STATUS_RXB           = 0x01 ;
    static const uint32_t CAN_STATUS_DATAOVERRUN   = 0x02 ;
    static const uint32_t CAN_STATUS_TXB           = 0x04 ;
    static const uint32_t CAN_STATUS_TX_COMPLETE   = 0x08 ;
    static const uint32_t CAN_STATUS_RX            = 0x10 ;
    static const uint32_t CAN_STATUS_TX            = 0x20 ;
    static const uint32_t CAN_STATUS_ERR           = 0x40 ;
    static const uint32_t CAN_STATUS_BUS           = 0x80 ;


  #define CAN_INTERRUPT   (*((const vuint32_t *)(ESP32CAN_BASE + 0x00C)))

    /* Bit definitions and macros for CAN_INTERRUPT */
    static const uint32_t CAN_INTERRUPT_RX           = 0x01;
    static const uint32_t CAN_INTERRUPT_TX           = 0x02;
    static const uint32_t CAN_INTERRUPT_ERR_WARN     = 0x04;
    static const uint32_t CAN_INTERRUPT_DATAOVERRUN  = 0x08;
    static const uint32_t CAN_INTERRUPT_ERR_PASSIVE  = 0x20;
    static const uint32_t CAN_INTERRUPT_ARB_LOST     = 0x40;
    static const uint32_t CAN_INTERRUPT_BUS_ERR      = 0x80;

  #define CAN_IER         (*((vuint32_t *)(ESP32CAN_BASE + 0x010)))
  #define CAN_BTR0        (*((vuint32_t *)(ESP32CAN_BASE + 0x018)))
  #define CAN_BTR1        (*((vuint32_t *)(ESP32CAN_BASE + 0x01C)))

/*------------------------------- Error and Counter Registers ------------------------------*/

  #define CAN_ALC         (*((vuint32_t *)(ESP32CAN_BASE + 0x02C)))
  #define CAN_ECC         (*((vuint32_t *)(ESP32CAN_BASE + 0x030)))
  #define CAN_EWLR        (*((vuint32_t *)(ESP32CAN_BASE + 0x034)))
  #define CAN_RX_ECR      (*((vuint32_t *)(ESP32CAN_BASE + 0x038)))
  #define CAN_TX_ECR      (*((vuint32_t *)(ESP32CAN_BASE + 0x03C)))

/*------------------------------- Shared Registers -----------------------------------------*/
    
    //-----CAN Frame Information Register
  #define CAN_FRAME_INFO (*((vuint32_t *)(ESP32CAN_BASE + 0x040)))

    /* Bit definitions and macros for CAN_TX_RX_FRAME */
    static const uint32_t CAN_FRAME_FORMAT_SFF = 0x00;
    static const uint32_t CAN_FRAME_FORMAT_EFF = 0x80;
    static const uint32_t CAN_RTR              = 0x40;
    //static const uint32_t CAN_DLC              = 1U << 0;
    #define CAN_DLC(x) ((uint8_t(x)) << 0)

    //-----CAN Frame Identifier Register
    //----- SFF : Standard Frame Format - length [2]
    //----- EFF : Extended Frame Format - length [4]
  #define CAN_ID_SFF(idx) (*((vuint32_t *)(ESP32CAN_BASE + 0x044 + 4 * (idx))))
  #define CAN_ID_EFF(idx) (*((vuint32_t *)(ESP32CAN_BASE + 0x044 + 4 * (idx))))

    //-----CAN Frame Data Register
    //----- DATA : length [8]
  #define CAN_DATA_SFF(idx) (*((vuint32_t *)(ESP32CAN_BASE + 0x04C + 4 * (idx))))
  #define CAN_DATA_EFF(idx) (*((vuint32_t *)(ESP32CAN_BASE + 0x054 + 4 * (idx))))

    //-----CAN Acceptance Filter Register
    //----- CODE : length [4]
    //----- MASK : length [4]
  #define CAN_ACC_CODE_FILTER(idx) (*((vuint32_t *)(ESP32CAN_BASE + 0x040 + 4 * (idx))))
  #define CAN_ACC_MASK_FILTER(idx) (*((vuint32_t *)(ESP32CAN_BASE + 0x050 + 4 * (idx))))

/*------------------------------- Misc Registers ------------------------------------------*/

  #define CAN_RXM_COUNTER (*((vuint32_t *)(ESP32CAN_BASE + 0x074)))

    //-----CAN Clock Divider Register
  #define CAN_CLK_DIVIDER (*((vuint32_t *)(ESP32CAN_BASE + 0x07C)))
    static const uint32_t CAN_PELICAN_MODE = 0x80;
    static const uint32_t CAN_CLK_OFF      = 0x08;
    #define CAN_CLK_DIV(idx)           ((uint8_t(idx)) << 0)

    //--- For Accessing ALL ESP32 CAN Registers
  #define REGALL(idx) (*((vuint32_t *)(ESP32CAN_BASE + 0x000 + 4 *(idx))))

#define CAN_MSG_STD_ID 0x7FF
#define CAN_MSG_EXT_ID 0x1FFFFFFF