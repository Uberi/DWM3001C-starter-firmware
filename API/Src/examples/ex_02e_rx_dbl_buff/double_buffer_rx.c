/*! ----------------------------------------------------------------------------
 *  @file    double_buffer_rx.c
 *  @brief   RX using double buffering example code
 *
 *           This example keeps listening for any incoming frames, storing in a local buffer any frame received before going back to listening. This
 *           examples activates interrupt handling and the double buffering feature of the DW IC (either auto or manual re-enable of receiver can be used).
 *           Frame processing is performed in the RX good frame callback.
 *
 * @attention
 *
 * Copyright 2019 - 2021 (c) Decawave Ltd, Dublin, Ireland.
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
#include <shared_defines.h>
#include <shared_functions.h>

#if defined(TEST_DOUBLE_BUFFER_RX)

extern void test_run_info(unsigned char *data);

/* Example application name and version to display on LCD screen. */
#define APP_NAME "RX DBL BUFF v1.0"

/* The following can be enabled to use manual RX enable instead of auto RX re-enable
 * NOTE: when using DW30xx devices, only the manual RX enable should be used
 *       with DW37xx devices either manual or auto RX enable can be used. */
#define USE_MANUAL_RX_ENABLE 0

/* Default communication configuration. We use default non-STS DW mode. */
static dwt_config_t config = {
    5,                /* Channel number. */
    DWT_PLEN_128,     /* Preamble length. Used in TX only. */
    DWT_PAC8,         /* Preamble acquisition chunk size. Used in RX only. */
    9,                /* TX preamble code. Used in TX only. */
    9,                /* RX preamble code. Used in RX only. */
    1,                /* 0 to use standard 8 symbol SFD, 1 to use non-standard 8 symbol, 2 for non-standard 16 symbol SFD and 3 for 4z 8 symbol SDF type */
    DWT_BR_6M8,       /* Data rate. */
    DWT_PHRMODE_STD,  /* PHY header mode. */
    DWT_PHRRATE_STD,  /* PHY header rate. */
    (129 + 8 - 8),    /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
    DWT_STS_MODE_OFF, /* STS disabled */
    DWT_STS_LEN_64,   /* STS length see allowed values in Enum dwt_sts_lengths_e */
    DWT_PDOA_M0       /* PDOA mode off */
};

/* Buffer to store received frame. See NOTE 1 below. */
static uint8_t rx_buffer[FRAME_LEN_MAX];

/* Declaration of static functions. */
static void rx_ok_cb(const dwt_cb_data_t *cb_data);
static void rx_err_cb(const dwt_cb_data_t *cb_data);

/**
 * Application entry point.
 */
int double_buffer_rx(void)
{
    uint32_t dev_id;
    /* Sends application name to test_run_info function. */
    test_run_info((unsigned char *)APP_NAME);

    port_set_dw_ic_spi_fastrate();

    /* Reset DW IC */
    reset_DWIC(); /* Target specific drive of RSTn line into DW IC low for a period. */

    Sleep(2); // Time needed for DW3000 to start up (transition from INIT_RC to IDLE_RC, or could wait for SPIRDY event)

    /* Probe for the correct device driver. */
    dwt_probe((struct dwt_probe_s *)&dw3000_probe_interf);

    dev_id = dwt_readdevid();

#if (USE_MANUAL_RX_ENABLE == 0)
    if ((dev_id == (uint32_t)DWT_DW3000_DEV_ID) || (dev_id == (uint32_t)DWT_DW3000_PDOA_DEV_ID))
    {
        /* Double buffer example in auto RX re-enable mode in not supported by DW3x00 */
        test_run_info((unsigned char *)"ERROR - NOT SUPPORTED ");
        while (1) { };
    }
    else
#endif
    {
        while (!dwt_checkidlerc()) /* Need to make sure DW IC is in IDLE_RC before proceeding */ { };

        if (dwt_initialise(DWT_DW_IDLE) == DWT_ERROR)
        {
            test_run_info((unsigned char *)"INIT FAILED");
            while (1) { };
        }

        /* Configure DW3xxx */
        if (dwt_configure(&config)) /* if the dwt_configure returns DWT_ERROR either the PLL or RX calibration has failed the host should reset the device */
        {
            test_run_info((unsigned char *)"CONFIG FAILED     ");
            while (1) { };
        }

        /* Register RX call-back. When using automatic RX re-enable is used below the RX error will not be reported */
        dwt_setcallbacks(NULL, rx_ok_cb, NULL, rx_err_cb, NULL, NULL, NULL);

        /*Clearing the SPI ready interrupt*/
        dwt_writesysstatuslo(DWT_INT_RCINIT_BIT_MASK | DWT_INT_SPIRDY_BIT_MASK);

        /* Enable RX interrupts for double buffer (RX good frames and RX errors). */
        if ((dev_id == (uint32_t)DWT_DW3000_DEV_ID) || (dev_id == (uint32_t)DWT_DW3000_PDOA_DEV_ID))
        {
            dwt_setinterrupt(DWT_INT_RXFCG_BIT_MASK | SYS_STATUS_ALL_RX_ERR, 0, DWT_ENABLE_INT);
        }
        else
        {
            dwt_setinterrupt_db(RDB_STATUS_RXOK, DWT_ENABLE_INT);
        }

        /* Install DW IC IRQ handler. */
        port_set_dwic_isr(dwt_isr);

#if (USE_MANUAL_RX_ENABLE == 0)
        dwt_setdblrxbuffmode(DBL_BUF_STATE_EN, DBL_BUF_MODE_AUTO); // Enable double buffer - auto RX re-enable mode, see NOTE 4.
#else
        dwt_setdblrxbuffmode(DBL_BUF_STATE_EN, DBL_BUF_MODE_MAN); // Enable double buffer - manual RX re-enable mode, see NOTE 4.
#endif

        dwt_rxenable(DWT_START_RX_IMMEDIATE); // Enable RX

        /* Loop forever receiving frames. See NOTE 4 below. */
        while (1) { };
    }
}

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn rx_ok_cb()
 *
 * @brief Callback to process RX good frame events
 *
 * @param  cb_data  callback data
 *
 * @return  none
 */

static void rx_ok_cb(const dwt_cb_data_t *cb_data)
{
#if (USE_MANUAL_RX_ENABLE == 1)
    dwt_rxenable(DWT_START_RX_IMMEDIATE); // When using manual RX re-enable then we can re-enable RX before processing the received packet.
#endif
    /* TESTING BREAKPOINT LOCATION #1 */

    /* A frame has been received, copy it to our local buffer. See NOTE 5 below. */
    if (cb_data->datalength <= FRAME_LEN_MAX)
    {
        dwt_readrxdata(rx_buffer, cb_data->datalength, 0);
    }

    /* TESTING BREAKPOINT LOCATION #2 */
}

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn rx_err_cb()
 *
 * @brief Callback to process RX error and timeout events
 *
 * @param  cb_data  callback data
 *
 * @return  none
 */
static void rx_err_cb(const dwt_cb_data_t *cb_data)
{
    (void)cb_data;
    dwt_rxenable(DWT_START_RX_IMMEDIATE);
}
#endif

/*****************************************************************************************************************************************************
 * NOTES:
 *
 * 1. In this example, maximum frame length is set to 127 bytes which is 802.15.4 UWB standard maximum frame length. DW IC supports an extended
 *    frame length (up to 1023 bytes long) mode which is not used in this example.
 * 2. This example shows how automatic or manual reception activation is performed. The DW IC offers several other features that can be used to handle more
 *    complex scenarios or to optimise system's overall performance (e.g. timeout after a given time, etc.).
 *    DW30xx only supports manual re-enable mode. DW37xx supports both modes.
 * 3. There is nothing to do in the loop here as frame reception and RX re-enabling is automatic and switching inside the INT. In a less trivial real-world
 *    application the RX data callback would generally signal the reception event to some background protocol layer to further process each RX frame.
 * 4. When using double buffering either a manual or automatic mode can be used. In the manual mode, the RX can be re-enabled before reading all the frame
 *    data as this is precisely the purpose of having two buffers.
 *    All the registers needed to process the received frame are also double buffered with the exception of the CIR memory, and can be accessed by calling
 *    dwt_readdiagnostics.
 *    In an actual application where these values might be needed for any processing or diagnostics purpose, they would have to be read before
 *    RX re-enabling is performed so that they are not corrupted by a frame being received while they are being read.
 *    Typically, in this example, any such diagnostic data access would be done at the very beginning of the rx_ok_cb function.
 *    Alternatively, if CIR memory does not need to be read. The auto RX re-enable mode can be used. This means that the device will automatically re-enable the
 *    receiver
 *    when an RX error is detected, it will go back to searching for preamble, and also on good frame reception. On good frame reception the device will signal
 *    RXFR event
 *    and then re-enable the receiver to receive the next packet into the other RX buffer & diagnostics set. If both RX buffers are full, waiting for host to
 *    process them,
 *    the device will trigger an RX overrun event and wait for a free buffer before proceeding to receive any further packets.
 * 5. A real application might get an operating system (OS) buffer for this data reading and then pass the buffer onto a queue into the next layer
 *    of processing task via an appropriate OS call.
 * 6. The user is referred to DecaRanging ARM application for additional practical example of usage, and to the
 *    DW IC API Guide for more details on the DW IC driver functions.
 * 7. This mode of operation - double buffer with auto RX re-enable, can be used in TDOA anchor which does not care about RX errors and just reports good
 *    receptions.
 ****************************************************************************************************************************************************/
