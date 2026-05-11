/***************************************************************************//**
* Copyright (c) 2026 Unicod
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

#ifndef __UI_H__
#define __UI_H__

#include "stdint.h"
#include "statemachine.h"

enum UI_SM_EVENTS_en {
   EV_UI_TICK_10MS = EV_USER_FIRST,
   EV_UI_TICK_1S,

   // Pressed key codes
   EV_UI_KEY_SEL,
   EV_UI_KEY_UP,
   EV_UI_KEY_DOWN,
   // Released key codes
   EV_UI_KEY_REL_SEL,
   EV_UI_KEY_REL_UP,
   EV_UI_KEY_REL_DOWN,
   // End of key codes

   //EV_UI_JACK_PLUG,        // headphone plugged
   //EV_UI_JACK_UNPLUG,      // headphone unplugged
   EV_UI_DISPLAY_UPDATE,
};

extern void UI_Init(void);
extern void UI_CheckEvent(void);
extern void UI_EventProc(uint8_t event);
extern void UI_EventProcKbd(uint16_t kbdcode);
extern void UI_EventSend(uint8_t event);



#endif
