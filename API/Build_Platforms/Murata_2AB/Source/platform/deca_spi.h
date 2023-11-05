/*! ----------------------------------------------------------------------------
 * @file    deca_spi.h
 * @brief   SPI access functions
 *
 * @attention
 *
 * Copyright 2015 - 2021 (c) DecaWave Ltd, Dublin, Ireland.
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

#include <boards.h>
#include <deca_types.h>
#include <nrf_drv_spi.h>
#include <sdk_config.h>

#define DECA_MAX_SPI_HEADER_LENGTH (3) // max number of bytes in header (for formating & sizing)

#define DATALEN1 200

    typedef enum
    {
        DW_HAL_NODE_UNLOCKED = NRF_SUCCESS,
        DW_HAL_NODE_LOCKED = NRF_ERROR_BUSY
    } dw_hal_lockTypeDef;

#define __HAL_LOCK(__HANDLE__)                                                                                                                                 \
    do                                                                                                                                                         \
    {                                                                                                                                                          \
        if ((__HANDLE__)->lock == DW_HAL_NODE_LOCKED)                                                                                                           \
        {                                                                                                                                                      \
            return NRF_ERROR_BUSY;                                                                                                                             \
        }                                                                                                                                                      \
        else                                                                                                                                                   \
        {                                                                                                                                                      \
            (__HANDLE__)->lock = DW_HAL_NODE_LOCKED;                                                                                                            \
        }                                                                                                                                                      \
    } while (0U)

#define __HAL_UNLOCK(__HANDLE__)                                                                                                                               \
    do                                                                                                                                                         \
    {                                                                                                                                                          \
        (__HANDLE__)->lock = DW_HAL_NODE_UNLOCKED;                                                                                                              \
    } while (0U)

    /* description of spi interface to DW3000 chip */
    typedef struct
    {
        nrf_drv_spi_t spi_inst;
        uint32_t frequency_slow;
        uint32_t frequency_fast;
        uint32_t csPin;
        nrf_drv_spi_config_t spi_config;
        dw_hal_lockTypeDef lock;
    } spi_handle_t;

/* description of connection to the DW3700 chip */
typedef struct
{
  uint16_t        irqPin;
  uint16_t        rstPin;
  uint16_t        wkupPin;
  uint16_t        csPin;
  spi_handle_t    *pSpi;
}dw_t;

/* @fn    nrf52840_dk_spi_init
 * Initialise nRF52840-DK SPI
 * */
void nrf52840_dk_spi_init(void);

/* @fn      port_set_dw_ic_spi_slowrate
 * @brief   set 2MHz
 * */
void port_set_dw_ic_spi_slowrate(void);

    /* @fn      port_set_dw_ic_spi_fastrate
     * @brief   set 16MHz
     * */
    void port_set_dw_ic_spi_fastrate(void);

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
    int readfromspi(uint16_t headerLength, uint8_t *headerBuffer, uint16_t readLength, uint8_t *readBuffer);

#ifdef __cplusplus
}
#endif

#endif /* _DECA_SPI_H_ */
