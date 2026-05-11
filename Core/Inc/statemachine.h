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

#ifndef __STATEMACHINE_H__
#define __STATEMACHINE_H__

#ifdef __cplusplus
extern "C" {
#endif
#if 0
} // workaround for CB folding bug
#endif

#include <stdint.h>


enum SM_EVENTS_en {
   EV_STATE_EMPTY,
   EV_STATE_ENTER,
   EV_STATE_EXIT,
   EV_TIMER_TICK,

   EV_USER_FIRST
};
typedef enum SM_EVENTS_en SM_EVENTS;

#define  SM_SET_STATE(st)    do { \
   ((STATE_MACHINE*)me)->StateNew = (SM_STATE_FUNC*)(st); \
} while (0)

#define  SM_SET_STATE_DELAYED(st, delay)    do { \
   ((STATE_MACHINE*)me)->StateDelayed = (SM_STATE_FUNC*)(st); \
   ((STATE_MACHINE*)me)->Timer_StateDelay = delay; \
   ((STATE_MACHINE*)me)->StateNew = &SM_ST_StateDelayed; \
} while (0)

struct _STATE_MACHINE;
typedef struct _STATE_MACHINE STATE_MACHINE;
typedef void SM_STATE_FUNC(STATE_MACHINE* me, uint8_t event);

struct _STATE_MACHINE {
   SM_STATE_FUNC*  State;        // actual state
   SM_STATE_FUNC*  StateNew;     // new state
   SM_STATE_FUNC*  StateDelayed; // auto state switch after delay
   SM_STATE_FUNC*  StateReturn;  // return state for nested states
   uint16_t Timer_StateDelay;
};

extern void StateMachineInit(STATE_MACHINE* const me, SM_STATE_FUNC* const init_state);
extern void StateMachine(STATE_MACHINE* const me, uint8_t ev);
extern void SM_ST_StateDelayed(STATE_MACHINE* const me, uint8_t event);

#if 0
{  // workaround for CB folding bug
#endif
#ifdef __cplusplus
}
#endif

#endif // __STATEMACHINE_H__
