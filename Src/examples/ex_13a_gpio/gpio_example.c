/*! ----------------------------------------------------------------------------
 *  @file    gpio_example.c
 *  @brief   This example demonstrates how to enable DW IC GPIOs as inputs and
 *           outputs. It also demonstrates how to drive the output to turn
 *           on/off LEDs on DW3000 HW.
 *
 * @attention
 *
 * Copyright 2017 - 2021 (c) Decawave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 * @author Decawave
 */
#include "deca_probe_interface.h"
#include <deca_device_api.h>
#include <deca_spi.h>
#include <example_selection.h>
#include <port.h>

#if defined(TEST_GPIO)

extern void test_run_info(unsigned char *data);

/* Example application name and version to display on LCD screen. */
#define APP_NAME   "GPIO        v1.0"
#define SLOW_BLINK 500 // 500ms between blinks. When GPIO0='0'
#define FAST_BLINK 100 // 100ms between blinks. When GPIO0='1'
/* Enable all GPIOs (See MFIO_MODE register) */
#define ENABLE_ALL_GPIOS_MASK_C0    0x200000 /* Configure all GPIOs as inputs for C0 */
#define ENABLE_ALL_GPIOS_MASK_D0_E0 0x1200492 /* Configure all GPIOs as inputs for D0 and E0*/
/* Set GPIOs 2 & 3 as outputs (See GPIO_DIR register) */
#define SET_OUTPUT_GPIO2_GPIO3 0xFFF3

/**
 * Application entry point.
 */

int gpio_example(void)
{
    uint16_t blink_delay, read_ios_val;
    int32_t dev_id;

    /* Display application name on LCD. */
    test_run_info((unsigned char *)APP_NAME);

    /* Configure SPI rate, DW3000 supports up to 36 MHz */
    port_set_dw_ic_spi_fastrate();

    /* Reset DW IC */
    /* Target specific drive of RSTn line into DW IC low for a period. */
    reset_DWIC();

    /* Time needed for DW3000 to start up
     * (transition from INIT_RC to IDLE_RC) */
    Sleep(2);

    /* Probe for the correct device driver. */
    dwt_probe((struct dwt_probe_s *)&dw3000_probe_interf);

    /* Need to make sure DW IC is in IDLE_RC before proceeding */
    while (!dwt_checkidlerc()) { };

    if (dwt_initialise(DWT_DW_IDLE) == DWT_ERROR)
    {
        test_run_info((unsigned char *)"INIT FAILED     ");
        while (1) { };
    }

    /* See NOTE 1. */
    /* 1st enable GPIO clocks */
    dwt_enablegpioclocks();

    dev_id = dwt_readdevid();

    /* See NOTE 2.
     * At this point we need to adjust the MFIO_MODE register to change the
     * mode of the GPIO pins (GPIO, LED, etc.). Note that GPIO_4, GPIO_5, and
     * GPIO_6 are already set to GPIO by default. */
    /*Set mode to GPIOs */
    if ((dev_id == DWT_DW3000_DEV_ID) || (dev_id == DWT_DW3000_PDOA_DEV_ID))
    {
        dwt_setgpiomode(GPIO_ALL, ENABLE_ALL_GPIOS_MASK_C0);
    }
    else
    {
        dwt_setgpiomode(GPIO_ALL, ENABLE_ALL_GPIOS_MASK_D0_E0);
    }
    /* Set output level for output pin to low. */
    dwt_setgpiovalue(GPIO_ALL, 0x0);

    /* Set GPIOs 2 & 3 as outputs and all other GPIOs as input */
    dwt_setgpiodir(SET_OUTPUT_GPIO2_GPIO3);

    /* This function will loop around forever turning the GPIOs controlling
     * the LEDs on and off */
    /* The blink rate will be set according to the GPIO0 read level */
    while (1)
    {
        read_ios_val = dwt_readgpiovalue();
        /* Checks if GPIO0 input is high */
        if (read_ios_val & GPIO0_BIT_MASK)
        {
            blink_delay = FAST_BLINK;
        }
        else
        {
            blink_delay = SLOW_BLINK;
        }

        /* Set GPIO2 and GPIO3 high */
        /* This will turn D1 (Green LED) and D2 (Red LED) on */
        dwt_setgpiovalue(GPIO3_BIT_MASK | GPIO2_BIT_MASK, 1);
        Sleep(blink_delay);
        /* set GPIO2 & GPIO3 low (LEDs will be turned off) */
        /* Clear bits 2,3 */
        dwt_setgpiovalue(GPIO3_BIT_MASK | GPIO2_BIT_MASK, 0);
        Sleep(blink_delay);
    }
}

#endif
/*****************************************************************************************************************************************************
 * NOTES:
 *
 * 1. When enabling the GPIO mode/value, the GPIO clock needs to be enabled.
 *
 * 2. By default, all the available GPIO pins on the DW3000 B0 red evaluation boards (GPIO0, GPIO1, GPIO2, GPIO3 & GPIO4) are set to work as GPIO.
 *    Other modes available are LED, AOA_SW, DBG_MUX, etc. Please see MFIO_MODE register details in datasheet for more information on this. You can
 *    see examples of how to set this register in functions like dwt_setleds().
 *
 * 3. The code above focuses mostly on how to test the GPIO outputs of the DW3000 chip. However, there is some code commented out that illustrates
 *    how to also test the input functionality of the GPIOs. On the DW3000 HW (Decawave shield / red board), there are a number of test points
 *    that are linked to the GPIO pins. In total, there are 9 GPIO pins available on the DW3000. However, only 5 of them are wired to test points on
 *    the Decawave shield / red board. The details of these GPIO pins are as follows:
 *    - GPIO0 - Test Point 3 (TP3)
 *    - GPIO1 - Test Point 4 (TP4)
 *    - GPIO2 - Test Point 6 (TP6) - Also controls RX LED (Green)
 *    - GPIO3 - Test Point 7 (TP7) - Also controls TX LED (Red)
 *    - GPIO4 - Test Point 8 (TP8)
 *    To use these GPIO pins, you can solder connections to your circuit to the test points on the Decawave Shield / red board. The simple GPIO input
 *    code example that is commented out above can be used if there are wires soldered to TP3 and TP4. These wires can be used as 'switches' to pull
 *    them high and act as an input on those GPIO input pins. This then controls the flashing frequency of the LEDs.
 *
 ****************************************************************************************************************************************************/
