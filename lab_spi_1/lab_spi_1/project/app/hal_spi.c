/* ------------------------------------------------------------------
 * --  _____       ______  _____                                    -
 * -- |_   _|     |  ____|/ ____|                                   -
 * --   | |  _ __ | |__  | (___    Institute of Embedded Systems    -
 * --   | | | '_ \|  __|  \___ \   Zuercher Hochschule Winterthur   -
 * --  _| |_| | | | |____ ____) |  (University of Applied Sciences) -
 * -- |_____|_| |_|______|_____/   8401 Winterthur, Switzerland     -
 * ------------------------------------------------------------------
 * --
 * -- Module      : SPI Library
 * -- Description :
 * --
 * -- $Id: hal_spi.c 4707 2019-02-26 09:32:59Z ruan $
 * ------------------------------------------------------------------
 */
#include <reg_stm32f4xx.h>
#include "hal_spi.h"

#define BIT_TXE (uint32_t)0x00000002
#define BIT_RXNE (uint32_t)0x00000001
#define BIT_BSY (uint32_t)0x00000080

static void set_ss_pin_low(void);
static void set_ss_pin_high(void);
static void wait_10_us(void);

/*
 * according to description in header file
 */
void hal_spi_init(void)
{
    RCC->APB2ENR |= 0x00001000; // enable SPI clock
    RCC->AHB1ENR |= 0x00000001; // start clock on GPIO A

    // configure GPIOs
    // P5.5 --> PA4 (NSS)       --> output
    // P5.6 --> PA5 (SCLK)      --> AF5 (see datasheet)
    // P5.7 --> PA6 (MISO)      --> AF5 (see datasheet)
    // P5.8 --> PA7 (MOSI)      --> AF5 (see datasheet)
    // P5.9 --> PA8 (NSBUF)     --> input

    GPIOA->OSPEEDR &= 0xFFFF00FF; // clear PA4-PA7
    GPIOA->OSPEEDR |= 0x0000FF00; // set PA4-PA7 to very high speed

    GPIOA->MODER &= 0xFFFC00FF; // clear mode on PA4-PA8 --> PA8 = input

    GPIOA->MODER |= 0x0000A800; // set alternate function on PA5-PA7
    GPIOA->MODER |= 0x00000100; // set PA4 to output

    GPIOA->AFRL &= 0x000FFFFF; // clear alternate function
    GPIOA->AFRL |= 0x55500000; // Set SPI1 (AF5) alternate function PA5-PA7

    // configure SPI
    SPI1->CR2 = 0x0000; // set spi to default state
    SPI1->CR1 = 0x0000; // set spi to default state

    // add your SPI configs below (based on reference manual)

    /// STUDENTS: To be programmed
    SPI1->CR1 |= 0x00000300; // set SSM and SSI == 1
    SPI1->CR1 |= 0x00000038; // set BR[2:0] to 111 (f_pclk/256)
    SPI1->CR1 |= 0x00000004; // set MSTR == 1 (master mode)
    SPI1->CR1 |= 0x00000040; // set SPE == 1 (enable SPI)
    /// END: To be programmed

    set_ss_pin_high();
}

/*
 * according to description in header file
 */
uint8_t hal_spi_read_write(uint8_t send_byte)
{
    /// STUDENTS: To be programmed
    // set SS pin low (activate)
    set_ss_pin_low();

    // write data to be transmitted to the SPI data register
    SPI1->DR = send_byte;
    // wait until the transmit buffer is empty
    while ((SPI1->SR & BIT_TXE) == 0);

    // wait until the busy flag is reset
    while ((SPI1->SR & BIT_RXNE) == 0);

    uint8_t received = SPI1->DR;

    while ((SPI1->SR & BIT_BSY) != 0);

    //set SS pin high (deactivate)
    set_ss_pin_high();

    wait_10_us();

    return received;
    /// END: To be programmed
}

/**
 * \brief  Set Slave-Select Pin (P5.5 --> PA4) low
 *
 * No parameters
 *
 * No returns
 */
static void set_ss_pin_low(void)
{
    GPIOA->BSRR = 0x00100000; // Set P5.5 --> PA4 low
}

/**
 * \brief  Set Slave-Select Pin (P5.5 --> PA4) high
 *
 * No parameters
 *
 * No returns
 */
static void set_ss_pin_high(void)
{
    GPIOA->BSRR = 0x00000010; // Set P5.5 --> PA4 high
}

/**
 * \brief  Wait for approximately 10us
 *
 * No parameters
 *
 * No returns
 */
static void wait_10_us(void)
{
    uint8_t counter = 0;
    while (counter < 160)
    {
        counter++;
    }
}
