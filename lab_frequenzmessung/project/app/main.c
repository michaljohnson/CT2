/* -----------------------------------------------------------------------------
 * --  _____       ______  _____                                               -
 * -- |_   _|     |  ____|/ ____|                                              -
 * --   | |  _ __ | |__  | (___    Institute of Embedded Systems               -
 * --   | | | '_ \|  _|  \__ \   Zurich University of                        -
 * --  | || | | | |____ ____) |  Applied Sciences                            -
 * -- ||| |||/   8401 Winterthur, Switzerland                -
 * -----------------------------------------------------------------------------
 * --
 * -- Project     : MC1 lab Frequenzmessung
 * -- Description :
 * --   #Reciprocal Counting 4.1 (Input GPIO Port A Pin 9):
 * --    This program counts with an internal fast clock using
 * --    timer 1. Timer 1 is setup in input capture mode.
 * --    The external fequency on PA 8 triggers on rising edge a
 * --    capture. Timer 1 fires a capture interrupt.
 * --    Now the capture value can be read and the frequency can
 * --    be calculated and displayed on lcd.
 * --   #Frequency multiplier 4.2 (Output GPIO Port A Pin 8):
 * --    With an interrupt the capture
 * --    value of timer 1 is loaded into timer 8 autoreload register.
 * --    By multiplying (dividing) the prescaler from timer 1 with the
 * --    dip-switch value and setting them as prescaler of timer 8, the
 * --    timer 8 now generates a multiplied frequency.
 * --    Timer 8 is set in output mode channel 1N, so the multiplied
 * --    frequency can be shown on the oscilloscope at gpio PA7.
 * --
 * -- $Id:          main.c 2020-09-23 ruan $
 * -- $Revision:    1) 2024-05-17 kapd: unified clockconfig with mc_board.c$
 * -- $Revision:    2) 2025-03-24 meso: portet project to CT-Board$
 * -------------------------------------------------------------------------- */

#include <reg_stm32f4xx.h>
#include "reg_ctboard.h"
#include <stdio.h>
#include "timers_freq.h"

#define PRESCALER              100u           // prescaler timer 1 
#define COUNTER_CLOCK (uint32_t)(((uint32_t)84000000u) / PRESCALER)
/* -----------------------------------------------------------------------------
 * constants
 * -------------------------------------------------------------------------- */

const char TXTSetup[40] = "Freq. In          HzFreq. Out         Hz";
/* -----------------------------------------------------------------------------
 * function prototypes
 * -------------------------------------------------------------------------- */
static uint8_t read_dip_switch(void);
static void init_system(void);
static void system_clock_config(void);
static void write_display(uint32_t inputfreq, uint32_t outputfreq, uint8_t multiplier);
/* -----------------------------------------------------------------------------
 * Main
 * -------------------------------------------------------------------------- */
int main(void)
{ 
    /* local vars */
    /// STUDENTS: To be programmed
		uint32_t inputfreq = 0;
    uint32_t outputfreq = 0;
    uint8_t multiplier = 0;

    /// END: To be programmed

    init_system();

    /* init timer 1 */
    init_measure_timer(PRESCALER);

    /* init timer 8, task 4.2 */
    init_frequency_multiplier();
    
    while (1) {
        /* main loop */

        /* Task 4.1
         *  -> get the newest capture value of timer 1
         *  -> calculate the frequency from the capture value
         *  -> call function to update frequency no Display
         *
         * Task 4.2
         *  -> before Display update: call function to set  
         *     prescaler of timer 8 depending on Dip-Switches
         */

        /// STUDENTS: To be programmed
				
			
			  uint32_t capture = get_capture_value();

        if (capture != 0) {
            inputfreq = COUNTER_CLOCK / capture;
        } else {
            inputfreq = 0;
        }

        multiplier =2* read_dip_switch();
        set_prescaler_freq_mul(multiplier);

        outputfreq = inputfreq * multiplier;
        write_display(inputfreq, outputfreq, multiplier);


        /// END: To be programmed
        
        //Start meso Test
        
    }
}

/* -----------------------------------------------------------------------------
 *  \brief  read and save the Dip-Switch value
 *          -> use GPIOF->IDR
 *  \return Dip Switch value.
 * -------------------------------------------------------------------------- */
static uint8_t read_dip_switch(void)
{

    /// STUDENTS: To be programmed

    return CT_DIPSW->BYTE.S7_0 & 0x0F;

    /// END: To be programmed
}


/* -----------------------------------------------------------------------------
 *  \brief  performs all the configurations and initializations
 * -------------------------------------------------------------------------- */
static void init_system(void)
{
    /* CT-Board Display Setup
        - Configure the CT-Board LCD display
        - Configure the CT-Board 7 segment display
     */
    for(uint8_t i=0;i<40; i++){
        CT_LCD->ASCII[i] = TXTSetup[i];
        CT_LCD->BG.BLUE = 0xFFFF;
        CT_LCD->BG.GREEN = 0xFFFF;
        CT_LCD->BG.RED = 0xFFFF;
        CT_SEG7->BIN.HWORD = 0u; 
    }
}

/* -----------------------------------------------------------------------------
 *  \brief  Outputs measurement data
 *
 *  \param  inputfreq: Frequency from function generator
 *  \param  outputfreq: Frequency that is outputted
 *  \param  multiplier: the multiplier from dip-switches
 * -------------------------------------------------------------------------- */
static void write_display(uint32_t inputfreq, uint32_t outputfreq, uint8_t multiplier)
{
    char display_text[6];
    /* update frequency on lcd */
    snprintf(display_text,sizeof(display_text), "%d    ", inputfreq);
    for(uint8_t i=0;i<6; i++){
        CT_LCD->ASCII[i+10] = display_text[i];
    }
    snprintf(display_text,sizeof(display_text), "%d    ", outputfreq);
    for(uint8_t i=0;i<6; i++){
        CT_LCD->ASCII[i+30] = display_text[i];
    }
    CT_SEG7->BIN.BYTE.DS1_0 = multiplier;
}