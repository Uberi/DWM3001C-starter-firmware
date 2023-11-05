
/*! ----------------------------------------------------------------------------
 *  @file    simple_aes.c
 *  @brief   SIMPLE AES example code
 *           This is a simple example to demonstrate the dw3xxx ability to encrypt
 *           and decrypt data using the CCM* methodology.
 *           The sample vectors used in this example are the same as the ones used in
 *           C.3.4 in the IEEE 802.15.4-2020 standard.
 *           The data to be encrypted as well as the nonce key to be used
 *           for the encryption are written to registers before a function is called
 *           for the chip to perform the AES operation.
 *           Once this operation is complete and no errors have been recorded the
 *           encrypted frame which will consist of the unencrypted header and the
 *           encrypted payload can then be read and compared to the expected output in
 *           section C.3.4 of the IEEE 802.15.4 standard.
 *
 *           Once encryption has occurred the data can then be decrypted to confirm
 *           the chip can decrypt data using CCM* correctly.
 *           Once decryption has been completed with no errors ocurring and the key used to
 *           decrypt is the same as the key used to encrypt the decrypted data should be the
 *           same as the original payload.
 *
 * Copyright 2021 (c) Decawave Ltd, Dublin, Ireland.
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

#if defined(TEST_SIMPLE_AES)

extern void test_run_info(unsigned char *data);

/* Example application name and version to display on LCD screen. */
#define APP_NAME "SIMPLE AES v1.0"

int simple_aes()
{
    static dwt_aes_config_t aes_config_z = { .key_load = AES_KEY_Load,
        .key_size = AES_KEY_128bit,
        .key_src = AES_KEY_Src_Register,
        .mic = MIC_16, /* Means 16 bytes tag*/
        .mode = AES_Encrypt,
        .aes_core_type = AES_core_type_CCM,
        .aes_key_otp_type = AES_key_RAM,
        .key_addr = 0 };
    int8_t status;
    uint8_t enc_data[50], plain_data[50], rx_header[50], rx_payload[50];
    /* Set the nonce as per IEEE 802.15.4-2020 C.3.4 MAC Command Frame */
    uint8_t nonce[13] = { 0xac, 0xde, 0x48, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x07, 0x07 };
    /* Set the key as per IEEE 802.15.4-2020 C.3.4 MAC Command Frame
     * 0xc0c1c2c3c4c5c6c7c8c9cacbcccdcecf
     * Rx key should changed as part of test to confirm when different key are used
     * for Rx and Tx decryption will not be completed correctly. */
    const dwt_aes_key_t aes_key_tx = { 0xcccdcecf, 0xc8c9cacb, 0xc4c5c6c7, 0xc0c1c2c3, 0, 0, 0, 0 };
    const dwt_aes_key_t aes_key_rx = { 0xcccdcecf, 0xc8c9cacb, 0xc4c5c6c7, 0xc0c1c2c3, 0, 0, 0, 0 };

    /* MAC Command frame: 4b ea 86 21 43 ff ff 01 00 00 00 00 48 de ac || 07 07 00 00 00 || 00 3f || 03 88
    01 1e 01 00 f8 07
    MAC Command frame : MAC_header + MAC_payload*/

    uint8_t MAC_header[]
        = { 0x4b, 0xea, 0x86, 0x21, 0x43, 0xff, 0xff, 0x01, 0x00, 0x00, 0x00, 0x00, 0x48, 0xde, 0xac, 0x07, 0x07, 0x00, 0x00, 0x00, 0x00, 0x3f };

    uint8_t MAC_payload[] = { 0x03, 0x88, 0x01, 0x1e, 0x01, 0x00, 0xf8, 0x07 }; // data to be encrypted

    // Frame which encrypted frame will be comparaed against
    //	    uint8_t SEC_MAC_frame[] = {0x4b, 0xea, 0x86, 0x21, 0x43, 0xff, 0xff, 0x01, 0x00, 0x00, 0x00, 0x00, 0x48, 0xde, 0xac,
    //	                            0x07, 0x07, 0x00, 0x00, 0x00,
    //	                            0x00, 0x3f,
    //	                            0x3e, 0xd2, 0xad, 0xf2, 0x5f, 0x3a, 0x12, 0x2c, /* encrypted payload */
    //	                            0x81, 0x4a, 0xdc, 0x9a, 0xeb, 0xbe, 0x26, 0x38, 0x41, 0xb8, 0x46, 0x33, 0x5f, 0xb0, 0x76, 0x18}; /* MIC 128 */

    dwt_aes_job_t tx_aes_job, rx_aes_job; // encryption and decryption

    uint8_t header_size = sizeof(MAC_header);

    /* Display application name on LCD. */
    test_run_info((unsigned char *)APP_NAME);

    /* DW3xxx chip can run from high speed from start-up.*/
    port_set_dw_ic_spi_fastrate();

    /* Reset and initialize DW chip. */
    reset_DWIC(); /* Target specific drive of RSTn line into DW3xxx low for a period. */

    Sleep(2); // Time needed for DW3xxx to start up (transition from INIT_RC to IDLE_RC, or could wait for SPIRDY event)

    /* Probe for the correct device driver. */
    dwt_probe((struct dwt_probe_s *)&dw3000_probe_interf);

    /* Need to make sure DW IC is in IDLE_RC before proceeding */
    while (!dwt_checkidlerc()) { };

    if (dwt_initialise(DWT_DW_INIT) == DWT_ERROR)
    {
        test_run_info((unsigned char *)"INIT FAILED");
        while (TRUE) { };
    }

    /* Encryption */
    dwt_set_keyreg_128(&aes_key_tx);
    tx_aes_job.nonce = nonce;                   /* use constructed nonce to encrypt payload */
    tx_aes_job.header = (uint8_t *)&MAC_header; /* plain-text header which will not be encrypted */
    tx_aes_job.header_len = sizeof(MAC_header);
    tx_aes_job.payload = MAC_payload; /* payload to be encrypted */
    tx_aes_job.payload_len = sizeof(MAC_payload);
    tx_aes_job.src_port = AES_Src_Scratch; /* src port is scratch buff */
    tx_aes_job.dst_port = AES_Dst_Scratch; /* dest port is scratch buff */
    tx_aes_job.mode = AES_Encrypt;

    tx_aes_job.mic_size = 16; /*MIC == 128 */

    /* write plain text data to scratch buffer */
    dwt_write_rx_scratch_data(tx_aes_job.header, tx_aes_job.header_len, 0);
    dwt_write_rx_scratch_data(tx_aes_job.payload, tx_aes_job.payload_len, tx_aes_job.header_len);

    /*configure AES engine parameters*/
    aes_config_z.mode = AES_Encrypt;
    dwt_configure_aes(&aes_config_z);

    /*run the AES engine */
    status = dwt_do_aes(&tx_aes_job, aes_config_z.aes_core_type); /* Note, IEEE 802.15.4-2020 adds a MIC size of 16 bytes after tx_payload */
    if (status < 0)
    {
        test_run_info((unsigned char *)"Length AES error");
    }
    else
    {
        // Check for no error when encrypting
        if (status & DWT_AES_ERRORS)
        {
            test_run_info((unsigned char *)"ERROR AES");
            while (1) { };
        }
        else
        {
            dwt_read_rx_scratch_data(enc_data, tx_aes_job.payload_len + tx_aes_job.header_len + tx_aes_job.mic_size, 0);
            // Place Break point 1 here here enc_data should match SEC_MAC_frame
            test_run_info((unsigned char *)"Encrypt Good");
        }
    }

    /* Decryption */
    rx_aes_job.nonce = nonce;
    rx_aes_job.header = (uint8_t *)&rx_header[0]; /* plain-text header which will not be encrypted */
    rx_aes_job.header_len = header_size;
    rx_aes_job.payload = (uint8_t *)&rx_payload[0]; /* payload to be encrypted */
    rx_aes_job.payload_len = sizeof(MAC_payload);

    rx_aes_job.src_port = AES_Src_Scratch; /* Take encrypted frame from the scratch buffer */
    rx_aes_job.dst_port = AES_Dst_Scratch; /* Decrypt the frame to the same scratch buffer : this will destroy original RX frame */
    rx_aes_job.mode = AES_Decrypt;
    rx_aes_job.mic_size = 16; /*MIC == 128 */

    dwt_set_keyreg_128(&aes_key_rx);
    aes_config_z.mode = AES_Decrypt;
    dwt_configure_aes(&aes_config_z);

    /*run the AES engine */
    status = dwt_do_aes(&rx_aes_job, aes_config_z.aes_core_type); /* Note, IEEE 802.15.4-2020 adds a MIC size of 16 bytes after tx_payload */
    if (status < 0)
    {
        test_run_info((unsigned char *)"Length AES error");
    }
    else
    {
        // Check for no error when decrypting
        if (status & DWT_AES_ERRORS)
        {
            test_run_info((unsigned char *)"ERROR AES");
            while (1) { };
        }
        else
        {
            dwt_read_rx_scratch_data(plain_data, rx_aes_job.payload_len + rx_aes_job.header_len + rx_aes_job.mic_size, 0);
            // Place Break point 2 here at this point plain_data should match {MAC_header, MAC_payload, MIC_128}
            test_run_info((unsigned char *)"Decrypt Good");
        }
    }

    return 0;
}
#endif
