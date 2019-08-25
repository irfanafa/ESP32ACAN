/******************************************************************************/
/* File name        : ESP32ACAN.cpp                                           */
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

/*------------------------------- Include files ------------------------------*/
#include "ESP32ACAN.h"

/*------------------------------- Local defines ------------------------------*/
#define ENABLE_ALL_INTERRUPTS    0xFF
#define DEFAULT_EWLR             (96)
#define DEFAULT_RxECR            (0)
#define DEFAULT_TxECR            (0)

#define CAN_DATA_MAX_LEN         (8)

#define CAN_MSG_STD_ID           0x7FF
#define CAN_MSG_EXT_ID           0x1FFFFFFF

//------- ESP32 Critical Section
// taskENTER_CRITICAL() of FREE-RTOS is deprecated as portENTER_CRITICAL() in ESP32
//------- https://esp32.com/viewtopic.php?t=1703
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;


//static void myESP32Task (void * pData) {
//    ESP32ACAN * canDriver = (ESP32ACAN *) pData ;
//    while (1) {
//      xSemaphoreTake (canDriver->mISRSemaphore, portMAX_DELAY) ;
//    }
//}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//   CONSTRUCTOR,
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

ESP32ACAN::ESP32ACAN (void) :
  
  mDriverReceiveBuffer(),
  mDriverTransmitBuffer(),
  mDriverSending(false),
  mISRSemaphore (xSemaphoreCreateCounting (10, 0))
  {}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//   Set the GPIO pins 
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ESP32ACAN::setGPIOPins(void) {

  gpio_num_t TXPin = GPIO_NUM_5;      //TX Pin - Default set to IO5. /*see schematics*/
  gpio_num_t RXPin = GPIO_NUM_4;      //RX Pin - Default set to IO4. /*see schematics*/

  //Set TX pin
    gpio_set_pull_mode(TXPin, GPIO_FLOATING);
    gpio_matrix_out(TXPin, CAN_TX_IDX, false, false);
    gpio_pad_select_gpio(TXPin);

  //Set RX pin

    gpio_set_pull_mode(RXPin, GPIO_FLOATING);
    gpio_matrix_in(RXPin, CAN_RX_IDX, false);
    gpio_pad_select_gpio(RXPin);
    gpio_set_direction(RXPin, GPIO_MODE_INPUT);
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//   Set the Requested Mode
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ESP32ACAN::setRequestedCANMode(const ESP32ACANSettings &inSettings, const ESP32ACANFilter inFilter) {
  /* ESP32 CAN Operation Mode Configuration
     
     Supported Mode                 MODE Registers
     - Normal Mode                  - Reset             -> bit(0)
     - No ACK                       - ListenOnly        -> bit(1)
     - Acceptance Filter            - SelfTest          -> bit(2)
                                    - Acceptance Filter -> bit(3) */
  
  uint8_t requestedMode = 0 ;
  switch (inSettings.mRequestedCANMode) {
    case ESP32ACANSettings::NormalMode :
      break ;  
    case ESP32ACANSettings::ListenOnlyMode : 
       requestedMode = CAN_MODE_LISTENONLY ;
      break ;  
    case ESP32ACANSettings::LoopBackMode :      
       requestedMode = CAN_MODE_SELFTEST ;
      break ;   
  }
  
  if(inFilter.mAMFSingle){
    requestedMode |= CAN_MODE_ACCFILTER ; 
  }

  CAN_MODE = requestedMode | CAN_MODE_RESET ;

  do{
    CAN_MODE = requestedMode;
  }while ((CAN_MODE & CAN_MODE_RESET) != 0) ;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//   Set the Bus timing Registers
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

inline void ESP32ACAN::setBitTimingSettings(const ESP32ACANSettings &inSettings)
{
  /* BUSTIMMING Configuration of ESP32 CAN
     ESP32ACANSettings calculates the best values for the desired bit Rate. 
  
  BTR0 : bit (0 - 5) -> Baud Rate Prescaller (BRP)
         bit (6 - 7) -> Snychronous Jump Width (SJW)
  
  BTR1 : bit (0 - 3) -> TimeSegment 1 (Tseg1)
         bit (4 - 6) -> TimeSegment 2 (Tseg2)
         bit (7)     -> TripleSampling? (SAM)   */

  CAN_BTR0 = ((inSettings.mSJW - 1) << 6) |                    /* SJW */
             ((((inSettings.mBitRatePrescaler) / 2) - 1) << 0) /* BRP */
      ;

  CAN_BTR1 = ((inSettings.mTripleSampling) << 7)   | /* Sampling */
             ((inSettings.mTimeSegment2 - 1) << 4) | /* Tseg2    */
             ((inSettings.mTimeSegment1 - 1) << 0)   /* Tseg1    */
      ;
}

void ESP32ACAN::setAcceptanceFilter (const ESP32ACANFilter inFilter) {

  /* Write the Code and Mask Registers with Acceptance Filter Settings*/
  if(inFilter.mAMFSingle){
    CAN_MODE |= CAN_MODE_ACCFILTER ;
  }

  CAN_ACC_CODE_FILTER(0) = inFilter.mACR0 ;
  CAN_ACC_CODE_FILTER(1) = inFilter.mACR1 ;
  CAN_ACC_CODE_FILTER(2) = inFilter.mACR2 ;
  CAN_ACC_CODE_FILTER(3) = inFilter.mACR3 ;

  CAN_ACC_MASK_FILTER(0) = inFilter.mAMR0 ;
  CAN_ACC_MASK_FILTER(1) = inFilter.mAMR1 ;
  CAN_ACC_MASK_FILTER(2) = inFilter.mAMR2 ;
  CAN_ACC_MASK_FILTER(3) = inFilter.mAMR3 ;

}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//   BEGIN
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————


uint32_t ESP32ACAN::begin (const ESP32ACANSettings &inSettings) {
  return internalBeginConfiguration (inSettings, ESP32ACANFilter ()) ;
}


uint32_t ESP32ACAN::begin (const ESP32ACANSettings &inSettings,
                           const ESP32ACANFilter inFiltersettings) {

  return internalBeginConfiguration (inSettings, inFiltersettings) ;
}

uint16_t ESP32ACAN::internalBeginConfiguration (const ESP32ACANSettings &inSettings,
                                                const ESP32ACANFilter inFilterSettings) {
  uint16_t errorCode = 0; // Ok be default

  //Enable CAN module
  //----Access the CAN Peripheral registers and initialize the CLOCK
  //https://github.com/ThomasBarth/ESP32-CAN-Driver/blob/master/components/can/CAN.c
  //Function periph_module_enable(); - https://github.com/espressif/esp-idf/blob/master/components/driver/periph_ctrl.c
  periph_module_enable(PERIPH_CAN_MODULE);

  //--------------------------------- Obligatory : It is must to enter RESET Mode to write the Configuration Registers
  while ((CAN_MODE & CAN_MODE_RESET) == 0) {
    CAN_MODE = CAN_MODE_RESET ;
  }
  if((CAN_MODE & CAN_MODE_RESET) ==0) {
    errorCode = kNotInRestModeInConfiguration ;
  }
  //--------------------------------- Use Pelican Mode
  CAN_CLK_DIVIDER = CAN_PELICAN_MODE;

  CAN_MODE |= CAN_MODE_LISTENONLY; 

  //----Check the Register access and bit timing settings before writing to the Bit Timing Registers
  CAN_BTR0 = 0x55 ;
  bool ok = CAN_BTR0 == 0x55 ;
  if (ok) {
    CAN_BTR0 = 0xAA ;
    ok = CAN_BTR0 == 0xAA ;
  }

  if(!ok) {
    errorCode |= kCANRegistersError ;
  }
  //----------------------------------- If ok, check the bit timing settings are correct
  if (!inSettings.mBitRateClosedToDesiredRate) {
    errorCode |= kTooFarFromDesiredBitRate;
  }
  if (inSettings.CANBitSettingConsistency() != 0) {
    errorCode |= kInconsistentBitRateSettings;
  }
    //----------------------------------- Allocate buffer
  if (!mDriverReceiveBuffer.initWithSize (inSettings.mDriverReceiveBufferSize)) {
    errorCode |= kCannotAllocateDriverReceiveBuffer ;
  }
  if (!mDriverTransmitBuffer.initWithSize (inSettings.mDriverTransmitBufferSize)) {
    errorCode |= kCannotAllocateDriverTransmitBuffer ;
  }
  if (errorCode == 0) {
    //--------------------------------- Set Bustiming Registers
    setBitTimingSettings(inSettings);
  }
  
  //--------------------------------- Set the Acceptance Filter
  setAcceptanceFilter(inFilterSettings);

  //--------------------------------- Set the default TX and RX GPIO pins for output and input 
  setGPIOPins();

  //--------------------------------- Set and clear the error counters to default value
  CAN_EWLR = DEFAULT_EWLR;
  CAN_RX_ECR = DEFAULT_RxECR;
  CAN_TX_ECR = DEFAULT_TxECR;

  //--------------------------------- Set to Requested Mode
  setRequestedCANMode(inSettings,inFilterSettings);

  
  switch(inSettings.mControlMessageByMethod) {
    case ESP32ACANSettings::PollingControlled :
      mSendbyPoll = true;
      mReceivebyPoll = true;
    break;
    case ESP32ACANSettings::InterruptControlled :
      mSendbyPoll = false;
      mReceivebyPoll = false;
      
      //--------------------------------- Enable All the Interupts
      CAN_IER = ENABLE_ALL_INTERRUPTS;
      
      //--------------------------------- Clear the Interrupt Registers
      const uint8_t unusedVariable __attribute__((unused)) = CAN_INTERRUPT;
      esp_intr_alloc(ETS_CAN_INTR_SOURCE, 0, isr, this, NULL);
    break;
  }
  return errorCode;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//   Interrupt Handler
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ESP32ACAN::isr(void *arg) {

    ESP32ACAN *myDriver = (ESP32ACAN *)arg;
    //BaseType_t xHigherPriorityTaskWoken = pdFALSE;
   
    portENTER_CRITICAL(&mux);
    uint32_t interrupt = CAN_INTERRUPT;
      if((interrupt & CAN_INTERRUPT_RX) != 0) {
        myDriver->handleRXInterrupt();
      }
      if((interrupt & CAN_INTERRUPT_TX) != 0) {
        myDriver->handleTXInterrupt();
      }
    portEXIT_CRITICAL(&mux);

    //xSemaphoreGiveFromISR(myDriver->mISRSemaphore, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR();
}

void ESP32ACAN::handleTXInterrupt() {
  CANMessage message ;
  const bool sendmsg = mDriverTransmitBuffer.remove (message);
  
  if (sendmsg) {
    internalSendMessage(message);
  }else {
    mDriverSending = false;
  }
}

void ESP32ACAN::handleRXInterrupt(void) {
  
  CANMessage outFrame;
  
  uint8_t RXMcount = CAN_RXM_COUNTER;
  for(uint32_t i=0 ; i< RXMcount; i++) {
    handleMessages(outFrame);
    mDriverReceiveBuffer.append(outFrame);  
  }
}
  
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//   RECEPTION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool ESP32ACAN::receivebypolling(CANMessage &outMessage) {
  portENTER_CRITICAL(&mux);
  bool hasReceivedMessage = (CAN_STATUS & CAN_STATUS_RXB) != 0;
  if (hasReceivedMessage) {
    for (uint32_t i = 0; i < CAN_RXM_COUNTER; i++){
      handleMessages(outMessage);
    }
  }
  portEXIT_CRITICAL(&mux);
  return hasReceivedMessage;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool ESP32ACAN::receive(CANMessage &outMessage) {

  static bool hasReceivedMessage;
  
  if(mReceivebyPoll) {
    hasReceivedMessage = receivebypolling(outMessage);
  }else {
    portENTER_CRITICAL(&mux);
    hasReceivedMessage = mDriverReceiveBuffer.remove(outMessage);
    portEXIT_CRITICAL(&mux);
  }
  return hasReceivedMessage;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ESP32ACAN::handleMessages(CANMessage &outFrame) {

  const uint32_t FrameInfo = CAN_FRAME_INFO;
  
  outFrame.len = FrameInfo & 0xF;
  outFrame.rtr = (FrameInfo & CAN_RTR) != 0;
  outFrame.ext = (FrameInfo & CAN_FRAME_FORMAT_EFF) != 0 ;
  
  //-----------Standard Frame
  if(!outFrame.ext) {
    uint32_t identifier =  ((uint32_t)CAN_ID_SFF(0)) << 3 ;
             identifier |= ((uint32_t)CAN_ID_SFF(1)) >> 5 ;
    outFrame.id = identifier;
    
    for (uint8_t i=0 ; (i<outFrame.len) && (i<CAN_DATA_MAX_LEN) ; i++) {
      outFrame.data[i] = CAN_DATA_SFF(i);
    }
  }else { //-----------Extended Frame
    uint32_t identifier =  ((uint32_t)CAN_ID_EFF(0)) << 21 ;
             identifier |= ((uint32_t)CAN_ID_EFF(1)) << 13 ;
             identifier |= ((uint32_t)CAN_ID_EFF(2)) << 5  ;
             identifier |= ((uint32_t)CAN_ID_EFF(3)) >> 3  ;
    outFrame.id = identifier;
    
    for (uint8_t i=0 ; (i<outFrame.len) && (i<CAN_DATA_MAX_LEN) ; i++) {
      outFrame.data[i] = CAN_DATA_EFF(i);
    }
  }
  
  for (uint8_t i=outFrame.len ; i<CAN_DATA_MAX_LEN; i++){
    outFrame.data[i] = 0;
  }
    
  CAN_CMD = CAN_CMD_RELEASE_RXB;
}



//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————
//   TRANSMISSION
//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool ESP32ACAN::tryToSendbypolling (const CANMessage &inMessage) {
  portENTER_CRITICAL(&mux);
  const uint8_t txstatus = (uint8_t)CAN_STATUS;
  const bool sendMessage = (txstatus & CAN_STATUS_TXB) != 0;
  if(sendMessage) {
    internalSendMessage(inMessage);
  }
  portEXIT_CRITICAL(&mux);
  return sendMessage;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

bool ESP32ACAN::tryToSend(const CANMessage &inMessage) {
  
  bool sendMessage;
  if(mSendbyPoll) {
    sendMessage = tryToSendbypolling(inMessage);
  }else if(mDriverSending) {
    portENTER_CRITICAL(&mux);
    sendMessage = mDriverTransmitBuffer.append(inMessage);
    portEXIT_CRITICAL(&mux);
  }else {
    portENTER_CRITICAL(&mux);
    internalSendMessage(inMessage);
    mDriverSending = true;
    sendMessage = true;
    portEXIT_CRITICAL(&mux);
  }
  return sendMessage;
}

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————————

void ESP32ACAN::internalSendMessage(const CANMessage &inFrame) {

  //--- DLC
  const uint8_t dlc = (inFrame.len <= 8) ? inFrame.len : 8;

  //--- RTR
  const uint8_t rtr = inFrame.rtr ? CAN_RTR : 0 ;

  //--- Frame ID
  const uint8_t id = (inFrame.ext) ? CAN_FRAME_FORMAT_EFF : CAN_FRAME_FORMAT_SFF ;
  
  //--- Set Frame Information
  CAN_FRAME_INFO = id | rtr | CAN_DLC(dlc);

  if (!inFrame.ext) { //-------Standard Frame
  //--- Set ID
  CAN_ID_SFF(0)  = (uint8_t)((inFrame.id) >> 3) ;
  CAN_ID_SFF(1)  = (uint8_t)((inFrame.id) << 5) ;

  //--- Set data
    for (uint8_t i=0 ; (i<dlc) && (i<CAN_DATA_MAX_LEN) ; i++) {
      CAN_DATA_SFF(i) = inFrame.data[i];
    }
  } else { //-------Extended Frame
  //--- Set ID
   CAN_ID_EFF(0) = (uint8_t)((inFrame.id) >> 21);
   CAN_ID_EFF(1) = (uint8_t)((inFrame.id) >> 13);
   CAN_ID_EFF(2) = (uint8_t)((inFrame.id) >> 5);
   CAN_ID_EFF(3) = (uint8_t)((inFrame.id) << 3);

  //--- Set data
    for (uint8_t i=0 ; (i<dlc) && (i<CAN_DATA_MAX_LEN) ; i++) {
      CAN_DATA_EFF(i) = inFrame.data[i];
    }
  }
 
  CAN_CMD = ((CAN_MODE & CAN_MODE_SELFTEST) !=0)?CAN_CMD_SELF_RX_REQ : CAN_CMD_TX_REQ;
}
