/***************************************************************************//**
* @file ui.c
* @brief User interface with statemachine
*******************************************************************************/

#include <stdint.h>
#include "ui.h"

#include "disp7seg.h"
#include "main.h"
#include "printf.h"
#include "tlv320aic3104_ctrl.h"


// Local macros ================================================================
#define  UI_AUDIO_CHANNELS    8
#define  UI_VOLUME_MAX        20


/// TODO move
//#include "stm32f0xx_tim.h"

// Key count per side (number of keys for each UI instance)
#define  KBD_COUNT_PER_SIDE   (EV_UI_KEY_REL_SEL - EV_UI_KEY_SEL)


// Typedefs ====================================================================
typedef struct UI_SM_st {
   STATE_MACHINE sm;
   uint32_t Events;
   uint16_t Timeout;
   uint8_t  uiID;
   uint8_t  Channel;
   int16_t  Volume;
   uint16_t Delay;
   uint16_t VolumeDisable;
   void     (*DispPutDigit)(uint8_t pos, char chr, uint8_t dp);
   void     (*AudioSetVolume)(uint8_t vol);
   void     (*AudioSetChannel)(uint8_t chan);
   uint8_t  (*Jack_isPlugged)(void);

} UI_SM;

//typedef void UI_STATE_FUNC (UI_SM* const me, uint16_t event);


// Local variables =============================================================
static UI_SM UISM[2] = {{{0}}, {{0}}};
#define DEFINE_UIOBJ()        UI_SM* const __attribute__((unused)) ui = (UISM); \
                              UI_SM* const __attribute__((unused)) ua = (&UISM[0]); \
                              UI_SM* const __attribute__((unused)) ub = (&UISM[1])

#define ITERATE_UIOBJ()       uint_fast8_t i=0; for (UI_SM* me = &UISM[0]; i<2; me = &UISM[++i])

#define MS_TO_TICK(__ms)      ((uint16_t)((__ms)/10))

// Local function declarations =================================================
static void UiSt_Standby(UI_SM* const me, uint8_t event);
static void UiSt_Listen(UI_SM* const me, uint8_t event);
static void UiSt_Channel(UI_SM* const me, uint8_t event);
static void UiSt_Volume(UI_SM* const me, uint8_t event);


// Interface to the hardware ===================================================
static void DispPutDigitA(uint8_t pos, char chr, uint8_t dp) {   DispPutDigit(0+pos, chr, dp);  }
static void DispPutDigitB(uint8_t pos, char chr, uint8_t dp) {   DispPutDigit(2+pos, chr, dp);  }

static void SetDAC(uint8_t codec, uint8_t vol) {
   int32_t vdac = 80 - 80 * (uint32_t)vol / 16;
   uint8_t hpout = 0;
   if (vol > 16) {
      vdac = 0;                  // DAC volume: max
      hpout = (vol - 16) * 2;    // +headphone output level (+0..8dB)
      if (hpout > 9) hpout = 9;  // max possible level: +9dB
   }
   if (vol == 0) vdac = 0x80;                // set mute bit if volume is 0
   TlvWriteReg(codec, TLV_PAGE_0, 43, vdac); // left DAC volume register
   TlvWriteReg(codec, TLV_PAGE_0, 44, vdac); // right DAC volume register
   TlvWriteReg(codec, TLV_PAGE_0, 51, (hpout<<4) | 0x09); // HPLOUT Output Level Control Register
   TlvWriteReg(codec, TLV_PAGE_0, 65, (hpout<<4) | 0x09); // HPROUT Output Level Control Register
}
static void AudioSetVolumeA(uint8_t vol) { SetDAC(CODEC_A, vol); }
static void AudioSetVolumeB(uint8_t vol) { SetDAC(CODEC_B, vol); }

static uint8_t Jack_isPluggedA(void) { return 1; }
static uint8_t Jack_isPluggedB(void) { return 1; }

// Function definitions ========================================================

//u @startuml
//u skinparam defaultTextAlignment left
//u state DigitalAudio485 {

/***************************************************************************//**
* @brief Initialize state machines
*******************************************************************************/
void UI_Init(void) {
   DEFINE_UIOBJ();
   ua->DispPutDigit = &DispPutDigitA;
   ub->DispPutDigit = &DispPutDigitB;
   ua->AudioSetVolume = &AudioSetVolumeA;
   ub->AudioSetVolume = &AudioSetVolumeB;
   ua->Jack_isPlugged = &Jack_isPluggedA;
   ub->Jack_isPlugged = &Jack_isPluggedB;
   ua->AudioSetChannel = &AudioSetChannelA;
   ub->AudioSetChannel = &AudioSetChannelB;

   ITERATE_UIOBJ() {
      me->uiID = i;
      me->Events = 0;
      me->Volume = 10;
      StateMachineInit(&me->sm, (SM_STATE_FUNC*)&UiSt_Standby); //u [*] -> Standby
   }
}

/***************************************************************************//**
* @brief Execute event handler immediately
*******************************************************************************/
void UI_EventProc(uint8_t event) {
   ITERATE_UIOBJ() {
      StateMachine(&me->sm, event);
   }
}

/***************************************************************************//**
* @brief Execute event handler for keyboard immediately
*******************************************************************************/
void UI_EventProcKbd(uint16_t kbdcode) {
   DEFINE_UIOBJ();
   uint8_t k = kbdcode & 0xFF;
   if (kbdcode < 256) {    // press
      if (k < KBD_COUNT_PER_SIDE) {
         StateMachine(&ua->sm, EV_UI_KEY_SEL + k);
      }else {
         StateMachine(&ub->sm, EV_UI_KEY_SEL - KBD_COUNT_PER_SIDE + k);
      }
   }else {                 // release
      if (k < KBD_COUNT_PER_SIDE) {
         StateMachine(&ua->sm, EV_UI_KEY_REL_SEL + k);
      }else {
         StateMachine(&ub->sm, EV_UI_KEY_REL_SEL - KBD_COUNT_PER_SIDE + k);
      }
   }

}

/***************************************************************************//**
* @brief Check and execute asynchron event (from event buffer)
*******************************************************************************/
void UI_CheckEvent(void) {
   ITERATE_UIOBJ() {
      if (me->Events) {
         uint32_t mask = 1;
         uint8_t ev;
         for (ev = 0; ev < 32; ev++) {
            if (me->Events & mask) {
               me->Events &= ~mask;
               StateMachine(&me->sm, ev);
               break;
            }
            mask <<= 1;    // shift left
         }
      }
   }
}

/***************************************************************************//**
* @brief Send (asynchron) event to state machine
*******************************************************************************/
static inline void EventSend (UI_SM* const me, uint8_t event) {
   me->Events |= (1 << event);
}

/***************************************************************************//**
* @brief Send event display update
*******************************************************************************/
static inline void EventSend_DisplayUpdate(UI_SM* const me) {
   EventSend(me, EV_UI_DISPLAY_UPDATE);
}


// State definitions ===========================================================


//u state Standby {
/***************************************************************************//**
* @brief Standby: display off
*******************************************************************************/
void UiSt_Standby(UI_SM* const me, uint8_t event) {
   switch (event) {
      case EV_STATE_ENTER: {           //u Standby: entry:
         printf("%c Standby ENTER\n", 'A'+me->uiID);
         me->AudioSetVolume(0);        //u Standby: switch off audio (mute)
         me->DispPutDigit(0, '-', 0);
         me->DispPutDigit(1, '-', 0);  //u Standby: switch of display
      }break;
      case EV_STATE_EXIT: {            //u Standby: \nexit:
         me->AudioSetVolume(me->Volume);  //u Standby: switch on audio
      }break;

      case EV_UI_TICK_1S: {
         if (me->Jack_isPlugged()) {      // headphone detected
            SM_SET_STATE(&UiSt_Channel);  //u Standby --> Channel : headphone\nplug in
         }
      }break;
   }
}
//u }

//u state Listen {
/***************************************************************************//**
* @brief Listen audio: one decimal point
*******************************************************************************/
void UiSt_Listen(UI_SM* const me, uint8_t event) {
   switch (event) {
      case EV_STATE_ENTER: {        //u Listen: entry:
         me->Delay = MS_TO_TICK(1000);
         me->DispPutDigit(0, ' ', 0);   //u Listen: display only one decimal point
         me->DispPutDigit(1, ' ', 1);
      }break;
      case EV_STATE_EXIT: {
      }break;

      case EV_UI_KEY_SEL: {
         SM_SET_STATE(&UiSt_Channel);   //u Listen -> Channel : key SEL
      }break;

      case EV_UI_KEY_UP:
      case EV_UI_KEY_DOWN: {
         SM_SET_STATE(&UiSt_Volume);    //u Listen --> Volume : key UP/DOWN
      }break;

      case EV_UI_TICK_10MS: {
         if (me->Jack_isPlugged()) {         // headphone present
            me->Delay = MS_TO_TICK(1000);
         }else {                             // no headphone
            if (me->Delay) {                    // delayed switch to standby
               if (--me->Delay == 0) {
                  SM_SET_STATE(&UiSt_Standby);  //u Listen -up-> Standby : headphone\nunplug
               }
            }
         }
      }break;
   }
}
//u }

//u state Channel {
/***************************************************************************//**
* @brief Display/set audio channel
*******************************************************************************/
void UiSt_Channel(UI_SM* const me, uint8_t event) {
   switch (event) {
      case EV_STATE_ENTER: {           //u Channel: entry:
         me->Timeout = 0;              // timeout to leave channel display
         me->Delay = 0;
         me->VolumeDisable = 0;
         EventSend_DisplayUpdate(me);  //u Channel: display channel number
      }break;
      case EV_STATE_EXIT: {
      }break;

      case EV_UI_DISPLAY_UPDATE: {
         me->DispPutDigit(0, 'C', 1);
         me->DispPutDigit(1, '0' + me->Channel, 0);
      }break;

      case EV_UI_KEY_SEL: {
         me->Timeout = 0;                       // restart timer
         if (++me->Channel >= UI_AUDIO_CHANNELS) me->Channel = 0; // select next channel
         me->AudioSetChannel(me->Channel);
         EventSend_DisplayUpdate(me);
      }break;

      case EV_UI_KEY_DOWN:
      case EV_UI_KEY_UP: {
         SM_SET_STATE(&UiSt_Volume);         //u Channel --> Volume : key\nUP/DOWN
      }break;

      case EV_UI_TICK_10MS: {
         if (++me->Timeout >= MS_TO_TICK(5000)) {
            SM_SET_STATE(&UiSt_Listen);    //u Channel -left-> Listen : timeout\n5s
         }
      }break;
   }
}
//u }

//u state Volume {
/***************************************************************************//**
* @brief Display/set volume
*******************************************************************************/
void UiSt_Volume(UI_SM* const me, uint8_t event) {
   switch (event) {
      case EV_STATE_ENTER: {           //u Volume: entry:
         me->Timeout = 0;              // timeout to leave volume display
         EventSend_DisplayUpdate(me);  //u Volume: display volume level
      }break;
      case EV_STATE_EXIT: {
      }break;

      case EV_UI_DISPLAY_UPDATE: {
         char s[16];
         sprintf(s, "%2u", me->Volume);
         me->DispPutDigit(0, s[0], 0);
         me->DispPutDigit(1, s[1], 0);
      }break;

      case EV_UI_KEY_SEL: {
         SM_SET_STATE(&UiSt_Channel); //u Volume -up-> Channel : key\nSEL
      }break;

      case EV_UI_KEY_DOWN:
      case EV_UI_KEY_UP: {
         me->Timeout = 0;                 // restart timer
         me->Volume += (event == EV_UI_KEY_UP ? 1 : -1);
         if (me->Volume < 0) me->Volume = 0;
         if (me->Volume > UI_VOLUME_MAX) me->Volume = UI_VOLUME_MAX;
         me->AudioSetVolume(me->Volume);
         EventSend_DisplayUpdate(me);
      }break;

      case EV_UI_TICK_10MS: {
         if (++me->Timeout >= MS_TO_TICK(5000)) {
            SM_SET_STATE(&UiSt_Listen);    //u Volume -up-> Listen : timeout\n5s
         }
      }break;
   }
}
//u }

//u }

//u @enduml
