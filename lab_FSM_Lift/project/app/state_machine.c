/* ----------------------------------------------------------------------------
 * --  _____       ______  _____                                              -
 * -- |_   _|     |  ____|/ ____|                                             -
 * --   | |  _ __ | |__  | (___    Institute of Embedded Systems              -
 * --   | | | '_ \|  __|  \___ \   Zurich University of                       -
 * --  _| |_| | | | |____ ____) |  Applied Sciences                           -
 * -- |_____|_| |_|______|_____/   8401 Winterthur, Switzerland               -
 * ----------------------------------------------------------------------------
 * --
 * -- Description:  Implementation of module state_machine.
 * --
 * -- $Id: state_machine.c 5526 2022-01-18 07:26:31Z ruan $
 * ------------------------------------------------------------------------- */

/* standard includes */
#include <stdint.h>

/* user includes */
#include "state_machine.h"
#include "action_handler.h"
#include "timer.h"

/* -- Macros
 * ------------------------------------------------------------------------- */

#define SAFETY_DURATION 150u // 150 * 10ms = 1.5s
#define SIGNAL_DURATION 100u // 100 * 10ms = 1s

#define TEXT_F0_OPENED "F0_OPENED"
#define TEXT_F0_CLOSED "F0_CLOSED"
#define TEXT_F1_OPENED "F1_OPENED"
#define TEXT_F1_CLOSED "F1_CLOSED"
#define TEXT_MOVING_UP "MOVING_UP"
#define TEXT_MOVING_DOWN "MOVING_DOWN"

/// STUDENTS: To be programmed

/// END: To be programmed

/* -- Type definitions
 * ------------------------------------------------------------------------- */

// definition of FSM states
typedef enum
{
    /* task 4.1 */
    F0_OPENED,
    F0_CLOSED,

    /* task 4.2 */
    F1_OPENED,
    F1_CLOSED,
    MOVING_UP,
    MOVING_DOWN,
    WAIT_UP,
    WAIT_DOWN,

    /// STUDENTS: To be programmed

    /// END: To be programmed

} state_t;

/* Module-wide variables & constants
 * ------------------------------------------------------------------------- */

// current FSM state
static state_t state = F0_CLOSED;

/* Public function definitions
 * ------------------------------------------------------------------------- */

/*
 * See header file
 */
void fsm_init(void)
{
    action_handler_init();
    ah_show_exception(NORMAL, "");

    /* go to initial state & do initial actions */

    /// STUDENTS: To be programmed

    ah_show_state(TEXT_F0_CLOSED);
    state = F0_CLOSED;

    /// END: To be programmed
}

/*
 * See header file
 */
void fsm_handle_event(event_t event)
{
    switch (state)
    {
    case F0_CLOSED:
        if (event == EV_DOOR0_OPEN_REQ)
        {
            ah_door(DOOR_OPEN);
            ah_show_state(TEXT_F0_OPENED);
            state = F0_OPENED;
        }
        else if (event == EV_BUTTON_F1)
        {
            ah_door(DOOR_LOCK);
            timer_start(SAFETY_DURATION); 
            state = WAIT_UP;
        }
        break;

    case F0_OPENED:
        if (event == EV_DOOR0_CLOSE_REQ)
        {
            ah_door(DOOR_CLOSE);
            ah_show_state(TEXT_F0_CLOSED);
            state = F0_CLOSED;
        }
        break;

    case F1_CLOSED:
        if (event == EV_DOOR1_OPEN_REQ)
        {
            ah_door(DOOR_OPEN);
            ah_show_state(TEXT_F1_OPENED);
            state = F1_OPENED;
        }
        else if (event == EV_BUTTON_F0)
        {
            ah_door(DOOR_LOCK);
            timer_start(SAFETY_DURATION); 
            state = WAIT_DOWN;
        }
        break;

    case F1_OPENED:
        if (event == EV_DOOR1_CLOSE_REQ)
        {
            ah_door(DOOR_CLOSE);
            ah_show_state(TEXT_F1_CLOSED);
            state = F1_CLOSED;
        }
        break;

    case MOVING_UP:
        if (event == EV_F1_REACHED)
        {
            ah_motor(MOTOR_OFF);
            ah_door(DOOR_UNLOCK);
            ah_show_state(TEXT_F1_CLOSED);
            state = F1_CLOSED;
        }
        break;

    case MOVING_DOWN:
        if (event == EV_F0_REACHED)
        {
            ah_motor(MOTOR_OFF);
            ah_door(DOOR_UNLOCK);
            ah_show_state(TEXT_F0_CLOSED);
            state = F0_CLOSED;
        }
        break;

    case WAIT_UP:
        if (event == EV_TIMEOUT)
        {
            ah_motor(MOTOR_UP); 
            ah_show_state(TEXT_MOVING_UP);
            state = MOVING_UP;
        }
        break;

    case WAIT_DOWN:
        if (event == EV_TIMEOUT)
        {
            ah_motor(MOTOR_DOWN); 
            ah_show_state(TEXT_MOVING_DOWN);
            state = MOVING_DOWN;
        }
        break;
    }
}
