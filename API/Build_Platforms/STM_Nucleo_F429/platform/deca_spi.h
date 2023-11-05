/*! ----------------------------------------------------------------------------
 * @file    deca_spi.h
 * @brief   SPI access functions
 *
 * @attention
 *
 * Copyright 2015-2021 (c) DecaWave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 * @author DecaWave
 */

#ifndef _DECA_SPI_H_
#define _DECA_SPI_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <deca_types.h>

#define DECA_MAX_SPI_HEADER_LENGTH (3) // max number of bytes in header (for formating & sizing)
    /*! ------------------------------------------------------------------------------------------------------------------
     * Function: openspi()
     *
     * Low level abstract function to open and initialise access to the SPI device.
     * returns 0 for success, or -1 for error
     */
    int openspi(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * Function: closespi()
     *
     * Low level abstract function to close the the SPI device.
     * returns 0 for success, or -1 for error
     */
    int closespi(void);

    /*! ------------------------------------------------------------------------------------------------------------------
     * Function: writetospiwithcrc()
     *
     * Low level abstract function to write to the SPI when SPI CRC mode is used
     * Takes two separate byte buffers for write header and write data, and a CRC8 byte which is written last
     * returns 0 for success, or -1 for error
     */
    int writetospiwithcrc(uint16_t headerLength, const uint8_t *headerBuffer, uint16_t bodyLength, const uint8_t *bodyBuffer, uint8_t crc8);

    /*! ------------------------------------------------------------------------------------------------------------------
     * Function: writetospi()
     *
     * Low level abstract function to write to the SPI
     * Takes two separate byte buffers for write header and write data
     * returns 0 for success, or -1 for error
     */
    int writetospi(uint16_t headerLength, const uint8_t *headerBuffer, uint16_t bodyLength, const uint8_t *bodyBuffer);

    /*! ------------------------------------------------------------------------------------------------------------------
     * Function: readfromspi()
     *
     * Low level abstract function to read from the SPI
     * Takes two separate byte buffers for write header and read data
     * returns the offset into read buffer where first byte of read data may be found,
     * or returns -1 if there was an error
     */
    //#pragma GCC optimize ("O3")
    int readfromspi(uint16_t headerLength, uint8_t *headerBuffer, uint16_t readlength, uint8_t *readBuffer);

#ifdef __cplusplus
}
#endif

#endif /* _DECA_SPI_H_ */
