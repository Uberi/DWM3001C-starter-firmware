/*! ----------------------------------------------------------------------------
 *  @file    timer_example.c
 *  @brief   This example demonstrates how to enable one of DW IC internal timers
 *
 *           In this example TIMER0 is configured in the repeating mode with
 *           period set to approx. 1s. Every second host count of timer events is printed.
 *           Every 20 seconds both host count and DW count of timer event is printed.
 *
 * @attention
 *
 * Copyright 2020 - 2021 (c) Qorvo Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 * @author Qorvo
 */
#include "deca_probe_interface.h"
#include <deca_device_api.h>
#include <deca_spi.h>
#include <example_selection.h>
#include <port.h>

#if defined(TEST_TIMER)

extern void test_run_info(unsigned char *data);

/* Example application name and version to display on LCD screen. */
#define APP_NAME "TIMER        v1.0"

#define TIMER_PERIOD (602 * 1000) // 602 - this gives approx. 1ms period based on XTAL/64 config below

int timer_count, timer_expired;

char disp_str[50] = { 0 };

static void timer_cb(const dwt_cb_data_t *cb_data);

/**
 * Application entry point.
 */
int timer_example(void)
{
    uint32_t dev_id;
    dwt_timer_cfg_t timer_cfg;

    timer_cfg.timer = DWT_TIMER0;
    timer_cfg.timer_div = DWT_XTAL_DIV64; // timer freq is 19.2 MHz
    timer_cfg.timer_mode = DWT_TIM_REPEAT;
    timer_cfg.timer_gpio_stop = 0;
    timer_cfg.timer_coexout = 0;

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

    dev_id = dwt_readdevid();

    if ((dev_id == (uint32_t)DWT_DW3000_DEV_ID) || (dev_id == (uint32_t)DWT_DW3000_PDOA_DEV_ID))
    {
        test_run_info((unsigned char *)"TIMER EXAMPLE IS NOT SUPPORTED BY DW3000 C0     ");
        while (1) { };
    }

    /* Need to make sure DW IC is in IDLE_RC before proceeding */
    while (!dwt_checkidlerc()) { };

    if (dwt_initialise(DWT_DW_IDLE) == DWT_ERROR)
    {
        test_run_info((unsigned char *)"INIT FAILED     ");
        while (1) { };
    }

    /* Register the call-backs (only SPI ready callback is used). */
    dwt_setcallbacks(NULL, NULL, NULL, NULL, NULL, &timer_cb, NULL);

    /* Enable wanted interrupts (TX confirmation, RX good frames, RX timeouts and RX errors). */
    dwt_setinterrupt(DWT_INT_TIMER0_BIT_MASK, 0, DWT_ENABLE_INT);

    /*Clearing the SPI ready interrupt*/
    dwt_writesysstatuslo(DWT_INT_RCINIT_BIT_MASK | DWT_INT_SPIRDY_BIT_MASK);

    /* Install DW IC IRQ handler. */
    port_set_dwic_isr(dwt_isr);

    // clear the timer expired flag
    timer_count = timer_expired = 0;

    /* set timer period */
    dwt_set_timer_expiration(DWT_TIMER0, TIMER_PERIOD);

    /* configuration will enable the timer */
    dwt_configure_timer(&timer_cfg);

    /* enable the timer as configured above */
    dwt_timer_enable(DWT_TIMER0);

    while (1)
    {
        if (timer_expired == 1)
        {
            /* Display timer count */
            snprintf(disp_str, sizeof(disp_str), "T: %08X\r\n", timer_count);
            test_run_info((unsigned char *)disp_str);

            timer_expired = 0;
        }
    }
}

/*! ------------------------------------------------------------------------------------------------------------------
 *
 * @brief Callback to process timer events
 *
 * @param  cb_data  callback data
 *
 * @return  none
 */
static void timer_cb(const dwt_cb_data_t *cb_data)
{

    (void)cb_data;
    timer_expired = 1; // timer expired

    timer_count++; // timer count
}

#endif
/*****************************************************************************************************************************************************
 * NOTES:
 *
 *
 *
 *
 ****************************************************************************************************************************************************/
