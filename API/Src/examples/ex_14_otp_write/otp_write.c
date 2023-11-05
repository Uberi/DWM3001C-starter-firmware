/*! ----------------------------------------------------------------------------
 *  @file    otp_write.c
 *  @brief   This example writes to the OTP memory and check if the write was successful.
 *
 * @attention
 *
 * Copyright 2018 - 2021 (c) Decawave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 * @author Decawave
 */

#include "deca_probe_interface.h"
#include <deca_device_api.h>
#include <example_selection.h>
#include <port.h>
#include <stdio.h>

#if defined(TEST_OTP_WRITE)

extern void test_run_info(unsigned char *data);

/* Example application name and version to display on LCD screen/VCOM port. */
#define APP_NAME    "OTP Write      "
#define OTP_ADDRESS 0x50 // Address to write - OTP
#define OTP_DATA    0x87654321 // Data to write - OTP

/**
 * Application entry point.
 */
int otp_write(void)
{
    int err;
    /* Display application name on LCD. */
    test_run_info((unsigned char *)APP_NAME);

    /* Configure SPI rate, DW3000 supports up to 36 MHz */
    port_set_dw_ic_spi_fastrate();

    /* Reset DW IC */
    reset_DWIC(); /* Target specific drive of RSTn line into DW IC low for a period. */

    Sleep(2); // Time needed for DW3000 to start up (transition from INIT_RC to IDLE_RC, or could wait for SPIRDY event)

    /* Probe for the correct device driver. */
    dwt_probe((struct dwt_probe_s *)&dw3000_probe_interf);

    err = dwt_otpwriteandverify(OTP_DATA, OTP_ADDRESS);
    if (err == DWT_SUCCESS)
    {
        test_run_info((unsigned char *)"OTP write PASS");
    }
    else
    {
        test_run_info((unsigned char *)"OTP write FAIL");
    }

    return err;
}

#endif
/*****************************************************************************************************************************************************
 * NOTES:
 * 1. You can write only 1 time to the memory - OTP memory.
 * 2. You can write only to a specific address range (see specification/manual).
 * 3. Data size is 32bit.
 ****************************************************************************************************************************************************/
