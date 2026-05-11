/***************************************************************************//**
* @file  statemachine.c
* @brief Simple finite state machine
*
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
*//****************************************************************************/

#include <stdint.h>
#include <string.h>

#include "statemachine.h"

void StateMachine(STATE_MACHINE* const me, uint8_t ev) {
   me->State(me, ev);                     // call actual state
   if (me->StateNew != NULL) {            // new state request
      me->StateReturn = me->State;        // remeber last state
      me->State(me, EV_STATE_EXIT);       // closing last state
      me->State = me->StateNew;           // switching state
      me->StateNew = NULL;
      me->State(me, EV_STATE_ENTER);      // init new state
   }
}

void SM_ST_StateDelayed(STATE_MACHINE* const me, uint8_t event) {
   switch (event) {
      case EV_TIMER_TICK: {
         if (me->Timer_StateDelay) {
            me->Timer_StateDelay--;
         }else {
            SM_SET_STATE(me->StateDelayed);
         }
      }break;
      default: break;
   }
}

void StateMachineInit(STATE_MACHINE* const me, SM_STATE_FUNC* const init_state) {
   me->State = init_state;
   me->StateNew = NULL;
   me->StateDelayed = init_state;
   me->StateReturn = init_state;

   me->State(me, EV_STATE_ENTER); // enter into initial state (initial transition)
}

