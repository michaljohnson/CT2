/* -----------------------------------------------------------------------------
 * --  _____       ______  _____                                               -
 * -- |_   _|     |  ____|/ ____|                                              -
 * --   | |  _ __ | |__  | (___    Institute of Embedded Systems               -
 * --   | | | '_ \|  _|  \__ \   Zurich University of                        -
 * --  | || | | | |____ ____) |  Applied Sciences                            -
 * -- ||| |||/   8401 Winterthur, Switzerland                -
 * -----------------------------------------------------------------------------
 * --
 * -- Project     : MC1 lab - Frequenzmessung
 * -- Description : Implementation of module timers frequency.
 * --               Init timer 1 with an external capture channel on
 * --               GPIO PA8. At initialization the timer sets a prescaler
 * --               for the internal clock source (84 MHz).
 * --               Init timer 8 in output compare mode.
 * --               Outputpin is GPIO PA7.
 * --               At run time the prescaler of timer 8 can be set
 * --               depending on the multiplication factor. This allows setting
 * --               the timer speed. The multiplier factor is displayed on
 * --               the Display.
 * --
 * -- $Id:          timers_freq.c 2019-10-05 frtt $
 * -- $Revision:    1) 20xx-xx-xx Kürzel: "change"$
 * -------------------------------------------------------------------------- */
 

#include <stdio.h>
#include <reg_stm32f4xx.h>

/* user includes */
#include "timers_freq.h"


/* -- Macros
 * ------------------------------------------------------------------------- */
#define PERIPH_GPIOA_ENABLE  (0x00000001)
#define PERIPH_TIM1_ENABLE   (0x00000001)
#define PERIPH_TIM8_ENABLE   (0x00000002)

#define GPIOPA7_MODE_AF      (0x00008000)  // Alternate function mode
#define GPIOPA7_MODE_CLEAR   (0xFFFF3FFF)  // GPIO port mode register CLEAR

#define GPIOPA7_OTYP         (0xFFFFFF7F)  // Output push pull

#define GPIOPA7_SPEED_100MHZ (0x0000C000)  // Very high speed 100 MHz

#define GPIOPA7_AF_TIM8      (0x30000000)  // AR1: Timer 8
#define GPIOPA7_AF_CLEAR     (0x0FFFFFFF)  // GPIO alternate function low register CLEAR

#define GPIOPA7_PUPDR_PD     (0x0000C000)  // Pull-down
#define GPIOPA7_PUPDR_CLEAR  (0xFFFF3FFF)  // GPIO port pull-up/pull-down register CLEAR

#define GPIOPA8_MODE_AF      (0x00020000)  // Alternate function mode
#define GPIOPA8_MODE_CLEAR   (0xFFFCFFFF)  // GPIO port mode register CLEAR

#define GPIOPA8_OTYP         (0xFFFFFEFF)  // Input

#define GPIOPA8_SPEED_100MHZ (0x00030000)  // Very high speed 100 MHz

#define GPIOPA8_PUPDR_PD     (0x00020000)  // Pull-down
#define GPIOPA8_PUPDR_CLEAR  (0xFFFCFFFF)  // GPIO port pull-up/pull-down register CLEAR

#define GPIOPA8_AF_TIM1      (0x00000001)  // AR1: Timer 1
#define GPIOPA8_AF_CLEAR     (0xFFFFFFF0)  // GPIO alternate function high register CLEAR


#define NVIC_ISER0_TIM1_CC   (0x08000000)

/* -- Macros for timer configuration
 * ------------------------------------------------------------------------- */
#define TIM1_SMCR_CLEAR         (0x00000000)
#define TIM1_SMCR_TRIGReset     (0x00000054)  // TI1FP1 as trigger input, also resets counter on trigger
#define TIM1_CCMR1_CLEAR        (0xFFFFFF00)
#define TIM1_CCMR1_CC1          (0x00000001)  // CC1 Channel is configured as input, IC1 is mapped on TI1
#define TIM1_CCMR1_NO_FILT      (0xFFFFFF0F)  // no sampling filter ist needed on TI1
#define TIM1_CCMR1_NO_PRESCALE  (0xFFFFFFF3)  // no prescaler, capture is done each time an edge is detected on the 
#define TIM1_CCER_TI1FP1_SELECT (0xFFFFFFF9)  // Edge aligned mode
#define TIM1_CCER_CLEAR_OUT     (0xfffffffE)  // Capture/Compare 1 output CLEAR
#define TIM1_CCER_EN_CAPTURE    (0x00000001)  // enable capture mode on specified channel
#define TIM1_CR1_CLEAR          (0xFFFFFFB1)  // control register 1 CLEAR
#define TIM1_CR1_SETUPCOUNT     (0xFFFFFFF7)  // Counter used as upcounter
#define TIM1_CR1_AUTOREPRELOAD  (0xFFFFFFFB)  // Auto reload preload enable
#define TIM1_CR1_ENABLE         (0x00000001)  // Start counter of timer 1
#define TIM1_CR2_SETUP          (0xFFFFFFF7)  // Setup CR2
#define TIM1_DIER_CCR1INTENABLE (0x00000002)  // CC1IE Capture/Compare 1 interrupt enable


#define TIM8_ARR                (0x00000001)  // Set autoreload value
#define TIM8_CR1_CLEAR          (0xFFFFFF87)  // Clear CR1
#define TIM8_CR1_DOWNCOUNT      (0x00000010)  // Set counter as downcounter
#define TIM8_CR1_AUTORELOAD     (0xFFFFFFF7)  // enable auto reload and preload
#define TIM8_CR2                (0xFFFFFF8F)  // Setup CR2
#define TIM8_CCMR1_CLEAR        (0xFFFFFF00)  // Clear CCMR1
#define TIM8_CCMR1_OC1MSETUP    (0x00000030)  // Set output toggle on CNT = CCR1
#define TIM8_CCER_POLARITY      (0xFFFFFFE7)  // set output polarity of OC1N to active high
#define TIM8_CCER_COMPAREMODEEN (0x00000004)  // enable compare mode on specified channel
#define TIM8_CCR1_CLEAR         (0xFFFF0000)  // Clear CCR1
#define TIM8_BDTR_OUTEN         (0x00008000)  // Enable output
#define TIM8_CR1_START          (0x00000001)  // start counters

/* module-wide global variables */
static volatile uint16_t prescaler_tim1;

/* memory for capture value */
static volatile uint32_t capture_value = 0;

/* Prototype def */
extern void TIM1_CC_IRQHandler(void);
static void init_GIPO8(void);
static void init_GIPO7(void);
/*
 * See header file
 */
void init_measure_timer(uint16_t prescaler)
{
    /* store parameter to module-wide global variable */
    prescaler_tim1 = prescaler;

    /* enable peripheral clock for timer 1 and gpio */
    RCC->APB2ENR |= PERIPH_TIM1_ENABLE;        // Timer 1
    // Initialize GPIO's
    init_GIPO8();
    /* Rest SMCR */
    TIM1->SMCR = TIM1_SMCR_CLEAR;

    /* enable counter reset at capture */
   
    /* select the filtered timer input 1 (TI1FP1) as the trigger input source 
       for the trigger input (TRGI)*/
    // trigger input (TRGI) resets the counter of timer 1
    TIM1->SMCR  |=  TIM1_SMCR_TRIGReset; 


    /* configure and enable the input capture channel 1 (IC1) */
    // CLEAR register
    TIM1->CCMR1  &= TIM1_CCMR1_CLEAR;      
    // CC1 Channel is configured as input, IC1 is mapped on TI1
    TIM1->CCMR1  |= TIM1_CCMR1_CC1;      
    // no sampling filter ist needed on TI1
    TIM1->CCMR1  &= TIM1_CCMR1_NO_FILT;       
    // select TI1FP1 as input source for caption, such that the rising edge of 
    // TI1 triggers a caption
    TIM1->CCER   &= TIM1_CCER_TI1FP1_SELECT;   
    // no prescaler, capture is done each time an edge is detected on the 
    // capture input
    TIM1->CCMR1  &= TIM1_CCMR1_NO_PRESCALE;   
    // Capture/Compare 1 output CLEAR
    TIM1->CCER   &= TIM1_CCER_CLEAR_OUT;       
    // enable capture mode on specified channel
    TIM1->CCER   |= TIM1_CCER_EN_CAPTURE;       


    /* init timer 1 as 16 bit upcounter
     *    -> set the autoreload value
     *    -> set the prescaler for timer 1
     *    -> set timer 1 as upcounter
     *    -> set timer 1 in continuous
     *    -> see ennis.zhaw -> Timer
     * enable interrupt
     *    -> see ennis.zhaw -> NVIC
     */

    // STUDENTS: To be programmed
    TIM1->ARR = 0xFFFF;
    TIM1->PSC = prescaler_tim1 - 1;
    TIM1->CR1 &= TIM1_CR1_SETUPCOUNT;
    TIM1->CR1 &= TIM1_CR1_AUTOREPRELOAD;
    TIM1->DIER |= TIM1_DIER_CCR1INTENABLE;
    NVIC->ISER0 |= NVIC_ISER0_TIM1_CC;



    /// END: To be programmed
    
    /* start counter of timer 1, task 4.1 */
    TIM1->CR1 |= TIM1_CR1_ENABLE;
}


/*
 * See header file
 */
void init_frequency_multiplier(void)
{
    /* enable peripheral clock for timer and gpio */
    RCC->APB2ENR |= PERIPH_TIM8_ENABLE;        // Timer 8
    init_GIPO7();
    /* Init timer 8 as 16 bit downcounter
     *    -> set autoreload value
     *    -> set the prescaler for timer 8
     *    -> set timer 8 as a downcounter to avoid counter overrun
     *    -> set timer 8 in continous mode
     *    -> see ennis.zhaw -> Timer
     */

    /// STUDENTS: To be programmed
    TIM8->ARR = 100;
    TIM8->PSC = 0;
    TIM8->CR1 &= TIM8_CR1_CLEAR;
    TIM8->CR1 |= TIM8_CR1_DOWNCOUNT;
    TIM8->CR1 &= TIM8_CR1_AUTORELOAD;
    TIM8->CR2 &= TIM8_CR2;


    /// END: To be programmed


    /* Configure timer 8 in output compare mode
     *    -> set the vlaue in compare reg (CCR1)
     *    -> toggle the output if counter value reaches ccr value
     *    -> set output polarity to active high
     *    -> enable compare mode on specified channel
     *    -< use TIM8->BDTR
     *    -> see ennis.zhaw -> timer
     */
     
    /// STUDENTS: To be programmed

    TIM8->CCR1 = 0;
    TIM8->CCMR1 &= TIM8_CCMR1_CLEAR;
    TIM8->CCMR1 |= TIM8_CCMR1_OC1MSETUP;
    TIM8->CCER &= TIM8_CCER_POLARITY;
    TIM8->CCER |= TIM8_CCER_COMPAREMODEEN;
    TIM8->BDTR |= TIM8_BDTR_OUTEN;



    /// END: To be programmed

    /* start counter of timer 8, task 4.2 */
    TIM8->CR1 |= TIM8_CR1_START;
    
}


/*
 * See header file
 */
uint32_t get_capture_value(void)
{
    return capture_value;
}


/*
 * See header file
 */
void set_prescaler_freq_mul(uint8_t multiplier_value)
{
    /* Set prescaler of timer
     *    -> Set the prescaler only if the multiplication factor has changed.
     */
    /// STUDENTS: To be programmed

    static uint8_t last_multiplier = 0xFF;
    if (multiplier_value != last_multiplier && multiplier_value > 0) {
        TIM8->PSC = (prescaler_tim1 / multiplier_value) - 1;
        last_multiplier = multiplier_value;
    }




    /// END: To be programmed
}


/**
 *  \brief  Interrupt service routine for timer 1.
 *          Read out the new capture value and save.
 *          Load capture value into reload reg of timer 8.
 */
extern void TIM1_CC_IRQHandler(void)
{
    /// STUDENTS: To be programmed

    capture_value = TIM1->CCR1;
    TIM8->ARR = capture_value;
    TIM1->SR &= ~0x0002;  // Clear CC1IF flag

    /// END: To be programmed
}
static void init_GIPO7(void)
{
    RCC->AHB1ENR |= PERIPH_GPIOA_ENABLE;        // GPIOA
    /* select alternate function mode to use gpio for timer purpose.
     * gpio PA7 is directly mapped to timer 8 CH1N 
     * (CH1N can be used only for output purpose)
     * init PA7 as an output pin for timer 8 (output compare mode)
     */
    // GPIO port mode register CLEAR
    GPIOA->MODER   &= GPIOPA7_MODE_CLEAR; 
    // Alternate function mode
    GPIOA->MODER   |= GPIOPA7_MODE_AF;
    // Output push pull
    GPIOA->OTYPER  &= GPIOPA7_OTYP;
    // Very high speed 100 MHz
    GPIOA->OSPEEDR |= GPIOPA7_SPEED_100MHZ;
    // GPIO port pull-up/pull-down register CLEAR
    GPIOA->PUPDR   &= GPIOPA7_PUPDR_CLEAR;
    // Pull-down
    GPIOA->PUPDR   |= GPIOPA7_PUPDR_PD;
    // GPIO alternate function low register CLEAR
    GPIOA->AFRL    &= GPIOPA7_AF_CLEAR;
    // AR1: Timer 8
    GPIOA->AFRL    |= GPIOPA7_AF_TIM8;
}
static void init_GIPO8(void)
{
    RCC->AHB1ENR |= PERIPH_GPIOA_ENABLE;        // GPIOA

    /* select alternate function mode to use gpio port for timer purpose.
     * gpio PA8 is directly mapped to timer 1 CH1.
     * althought PA8 is initialized as output pin, timer 1 can use 
     * PA8 either on CH1 as an output pin (for output compare mode)
     * or as an input pin (for input capture mode).
     */
     
    // GPIO port mode register CLEAR
    GPIOA->MODER   &= GPIOPA8_MODE_CLEAR;
    // Alternate function mode
    GPIOA->MODER   |= GPIOPA8_MODE_AF;
    // Output push pull
    GPIOA->OTYPER  &= GPIOPA8_OTYP;    
    // Very high speed 100 MHz
    GPIOA->OSPEEDR |= GPIOPA8_SPEED_100MHZ;    
    // GPIO port pull-up/pull-down register CLEAR
    GPIOA->PUPDR   &= GPIOPA8_PUPDR_CLEAR;   
    // Pull-down
    GPIOA->PUPDR   |= GPIOPA8_PUPDR_PD;   
    // GPIO alternate function high register CLEAR
    GPIOA->AFRH  &= GPIOPA8_AF_CLEAR;
    // AR1: Timer 1
    GPIOA->AFRH  |= GPIOPA8_AF_TIM1;

}