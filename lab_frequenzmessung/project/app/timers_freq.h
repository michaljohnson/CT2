/* ------------------------------------------------------------------
 * --  _____       ______  _____                                    -
 * -- |_   _|     |  ____|/ ____|                                   -
 * --   | |  _ __ | |__  | (___    Institute of Embedded Systems    -
 * --   | | | '_ \|  __|  \___ \   Zurich University of             -
 * --  _| |_| | | | |____ ____) |  Applied Sciences                 -
 * -- |_____|_| |_|______|_____/   8401 Winterthur, Switzerland     -
 * ------------------------------------------------------------------
 * --
 * -- Project     : MC1 lab - Frequenzmessung
 * -- Description : Interface timers_freq.c for module frequency
 * --               measure and multipier timers.
 * --
 * -- $Id:          timers_freq.h 2018-10-25 gruj $
 * -- $Revision:    1) 20xx-xx-xx Kürzel: "change"$
 * --------------------------------------------------------------- */

#ifndef _TIMERS_FREQ
#define _TIMERS_FREQ

#include <stdint.h>


/**
 *  \brief  Init a timer in input capture mode.
            Setup timer 1 as 16 bit up counter with external trigger.
 *          Input GPIO PA8 is used to trigger a capture from counter
 *          into capture compare register 1.
 *          Configure timer in reset mode, so the counter register is
 *          reseted (cleared) after a capture.
 *  \param  prescaler : prescaler for the clock source (84 MHz).
 */
void init_measure_timer(uint16_t prescaler);


/**
 *  \brief  Initialize timer 8 in output mode mapped to GPIO PA7.
 *          The timer runs in toggle mode.
 *          After inititialization the prescaler can be divided by the
 *          multiplier value, by calling set_prescaler_freq_mul(). Like this
 *          the frequency can multiplied by the input value.
 */
void init_frequency_multiplier(void);


/**
 *  \brief  Returns the newest capture value of timer 1.
 *  \return Current capture value of timer 1.
 */
uint32_t get_capture_value(void);


/**
 * \brief   Set prescaler of timer 8 (init prescaler value).
 *  \param  multiplier_value : the multiplication factor.
 */
void set_prescaler_freq_mul(uint8_t multiplier_value);




/* end header ----------------------------------------------------- */
#endif
