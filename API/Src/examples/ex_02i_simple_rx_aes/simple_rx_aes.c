/*! ----------------------------------------------------------------------------
 *  @file    simple_rx_aes.c
 *  @brief   Simple RX + AES example code
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
#include <deca_spi.h>
#include <example_selection.h>
#include <mac_802_15_8.h>
#include <port.h>
#include <shared_defines.h>
#include <shared_functions.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#if defined(TEST_SIMPLE_RX_AES)

extern void test_run_info(unsigned char *data);

/* Example application name and version to display on LCD screen. */
#define APP_NAME "AES RX AES"

/*
 * APP_KEY_0-APP_KEY_4 is a 128 bit AES key which should be set the same
 * for both Encryption and Decryption.
 * This should match complementary RX example.
 *
 * The dwt_aes_key_t structure is actually 256-bits in length. As we are only using
 * a 128-bit key in this example, we will pad the rest of the structure out with zeroes.
 */
static const dwt_aes_key_t aes_key
    = { 0x41424344, 0x45464748, 0x49505152, 0x53545556, 0x00000000, 0x00000000, 0x00000000, 0x00000000 }; /*Initialize 128bits key*/

static const dwt_aes_config_t aes_config = { .key_load = AES_KEY_Load,
    .key_size = AES_KEY_128bit,
    .key_src = AES_KEY_Src_Register,
    .mic = MIC_16, /* Means 16 bytes tag*/
    .mode = AES_Decrypt,
    .aes_core_type = AES_core_type_GCM, // Use GCM core
    .aes_key_otp_type = AES_key_RAM,
    .key_addr = 0 };

/* Default communication configuration. We use default non-STS DW mode. */
static dwt_config_t config = {
    5,                              /* Channel number. */
    DWT_PLEN_128,                   /* Preamble length. Used in TX only. */
    DWT_PAC8,                       /* Preamble acquisition chunk size. Used in RX only. */
    9,                              /* TX preamble code. Used in TX only. */
    9,                              /* RX preamble code. Used in RX only. */
    1,                              /* 0 to use standard SFD, 1 to use non-standard SFD. */
    DWT_BR_6M8,                     /* Data rate. */
    DWT_PHRMODE_STD,                /* PHY header mode: extended frame mode, up to 1023-16-2 in the payload */
    DWT_PHRRATE_STD, (129 + 8 - 8), /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
    DWT_STS_MODE_OFF,               /* STS mode*/
    DWT_STS_LEN_64,                 /* STS length see allowed values in Enum dwt_sts_lengths_e */
    DWT_PDOA_M0                     /*pdoa mode*/
};

/**
 * Application entry point.
 */
int simple_rx_aes(void)
{
    uint32_t status_reg;
    dwt_aes_job_t aes_job;
    uint8_t mic_size;
    aes_results_e aes_results;
    uint8_t payload[128] = { 0 };

    if (aes_config.mic == 0)
    {
        mic_size = 0;
    }
    else
    {
        mic_size = aes_config.mic * 2 + 2;
    }

    /* Display application name on LCD. */
    test_run_info((unsigned char *)APP_NAME);

    /* DW3000 chip can run from high speed from start-up.*/
    port_set_dw_ic_spi_fastrate();

    /* Reset and initialize DW chip. */
    reset_DWIC(); /* Target specific drive of RSTn line into DW3000 low for a period. */

    Sleep(2); // Time needed for DW3000 to start up (transition from INIT_RC to IDLE_RC, or could wait for SPIRDY event)

    /* Probe for the correct device driver. */
    dwt_probe((struct dwt_probe_s *)&dw3000_probe_interf);

    while (!dwt_checkidlerc()) /* Need to make sure DW IC is in IDLE_RC before proceeding */ { };

    if (dwt_initialise(DWT_DW_INIT) == DWT_ERROR)
    {
        test_run_info((unsigned char *)"INIT FAILED");
        while (TRUE) { };
    }

    /* Configure DW3000. */
    /* if the dwt_configure returns DWT_ERROR either the PLL or RX calibration has failed the host should reset the device */
    if (dwt_configure(&config))
    {
        test_run_info((unsigned char *)"CONFIG FAILED     ");
        while (1) { };
    }

    dwt_set_keyreg_128(&aes_key);
    dwt_configure_aes(&aes_config);

    aes_job.src_port = AES_Src_Rx_buf_0; /* Take encrypted frame from the RX buffer */
    aes_job.dst_port = AES_Dst_Rx_buf_0; /* Decrypt the frame to the same RX buffer : this will destroy original RX frame */
    aes_job.mode = aes_config.mode;
    aes_job.mic_size = mic_size;

    while (TRUE)
    {
        /* Activate reception immediately. See NOTE 2 below. */
        dwt_rxenable(DWT_START_RX_IMMEDIATE);

        /* Poll until a frame is properly received or an error/timeout occurs.*/
        waitforsysstatus(&status_reg, NULL, (DWT_INT_RXFCG_BIT_MASK | SYS_STATUS_ALL_RX_ERR), 0);

        if (status_reg & DWT_INT_RXFCG_BIT_MASK)
        {
            uint16_t finfo16;
            finfo16 = dwt_getframelength();

            /* Decrypt received packet */
            aes_results = rx_aes_802_15_8((finfo16 & RX_BUFFER_MAX_LEN), &aes_job, payload, sizeof(payload), aes_config.aes_core_type);
            if (aes_results != AES_RES_OK)
            {
                switch (aes_results)
                {
                case AES_RES_ERROR_LENGTH:
                    test_run_info((unsigned char *)"Length AES error");
                    break;
                case AES_RES_ERROR:
                    test_run_info((unsigned char *)"ERROR AES");
                    break;
                case AES_RES_ERROR_FRAME:
                    test_run_info((unsigned char *)"Error Frame");
                    break;
                case AES_RES_ERROR_IGNORE_FRAME:
                case AES_RES_OK:
                    break;
                }
                break; // Exit on error
            }
            else
            {
                char str[20];
                static int cnt;
                sprintf(str, "AES TX OK %d", cnt++);
            }
            /* Clear good RX frame event in the DW chip status register. */
            dwt_writesysstatuslo(DWT_INT_RXFCG_BIT_MASK);
        }
        else
        {
            /* Clear RX error events in the status register. */
            dwt_writesysstatuslo(SYS_STATUS_ALL_RX_ERR);
        }
    }
    return 0;
}
#endif

/*****************************************************************************************************************************************************
 * NOTES:
 *
 * TODO: Correct and/or required notes for this example can be added later.
 *
 ****************************************************************************************************************************************************/
