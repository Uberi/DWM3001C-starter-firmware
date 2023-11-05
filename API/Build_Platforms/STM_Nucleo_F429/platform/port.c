/*! ----------------------------------------------------------------------------
 * @file    port.c
 * @brief   HW specific definitions and functions for portability
 *
 * @attention
 *
 * Copyright 2016-2021 (c) DecaWave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 * @author DecaWave
 */

#include <port.h>
#include <usbd_cdc_if.h>

/****************************************************************************
 *
 *                              APP global variables
 *
 *******************************************************************************/
extern SPI_HandleTypeDef hspi1,hspi4;
extern SPI_HandleTypeDef *hcurrent_active_spi;
extern uint16_t          pin_io_active_spi;
extern GPIO_PinState     SPI_CS_state;
extern host_using_spi_e host_spi;

/****************************************************************************
 *
 *                  Port private variables and function prototypes
 *
 *******************************************************************************/
static volatile uint32_t signalResetDone;

/* DW IC IRQ handler definition. */
static port_dwic_isr_t port_dwic_isr = NULL;

/****************************************************************************
 *
 *                              Time section
 *
 *******************************************************************************/

/* @fn    portGetTickCnt
 * @brief wrapper for to read a SysTickTimer, which is incremented with
 *        CLOCKS_PER_SEC frequency.
 *        The resolution of time32_incr is usually 1/1000 sec.
 * */
__INLINE uint32_t portGetTickCnt(void)
{
    return HAL_GetTick();
}

/* @fn    usleep
 * @brief precise usleep() delay
 * */
#pragma GCC optimize("O0")
int usleep(useconds_t usec)
{
    unsigned int i;

    usec *= 12;
    for (i = 0; i < usec; i++)
    {
        __NOP();
    }
    return 0;
}

/* @fn    Sleep
 * @brief Sleep delay in ms using SysTick timer
 * */
__INLINE void Sleep(uint32_t x)
{
    HAL_Delay(x);
}

/****************************************************************************
 *
 *                              END OF Time section
 *
 *******************************************************************************/

/****************************************************************************
 *
 *                              Configuration section
 *
 *******************************************************************************/

/* @fn    peripherals_init
 * */
int peripherals_init(void)
{
    /* All has been initialized in the CubeMx code, see main.c */
    return 0;
}

/* @fn    spi_peripheral_init
 * */
void spi_peripheral_init()
{

    /* SPI's has been initialized in the CubeMx code, see main.c */

    port_LCD_RS_clear();

    port_LCD_RW_clear();
}

/**
 * @brief  Checks whether the specified IRQn line is enabled or not.
 * @param  IRQn: specifies the IRQn line to check.
 * @return "0" when IRQn is "not enabled" and !0 otherwise
 */
ITStatus EXTI_GetITEnStatus(IRQn_Type IRQn)
{
    return ((NVIC->ISER[(((uint32_t)(int32_t)IRQn) >> 5UL)] & (uint32_t)(1UL << (((uint32_t)(int32_t)IRQn) & 0x1FUL))) == (uint32_t)RESET) ? (RESET) : (SET);
}
/****************************************************************************
 *
 *                          End of configuration section
 *
 *******************************************************************************/

/****************************************************************************
 *
 *                          DW IC port section
 *
 *******************************************************************************/

/* @fn      reset_DW IC
 * @brief   DW_RESET pin on DW IC has 2 functions
 *          In general it is output, but it also can be used to reset the digital
 *          part of DW IC by driving this pin low.
 *          Note, the DW_RESET pin should not be driven high externally.
 * */
void reset_DWIC(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    // Enable GPIO used for DW1000 reset as open collector output
    GPIO_InitStruct.Pin = DW_RESET_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DW_RESET_GPIO_Port, &GPIO_InitStruct);

    // drive the RSTn pin low
    HAL_GPIO_WritePin(DW_RESET_GPIO_Port, DW_RESET_Pin, GPIO_PIN_RESET);

    usleep(1);

    // put the pin back to output open-drain (not active)
    setup_DWICRSTnIRQ(0);
    Sleep(2);
}

/* @fn      setup_DWICRSTnIRQ
 * @brief   setup the DW_RESET pin mode
 *          0 - output Open collector mode
 *          !0 - input mode with connected EXTI0 IRQ
 * */
void setup_DWICRSTnIRQ(int enable)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    if (enable)
    {
        // Enable GPIO used as DECA RESET for interrupt
        GPIO_InitStruct.Pin = DW_RESET_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(DW_RESET_GPIO_Port, &GPIO_InitStruct);

        HAL_NVIC_EnableIRQ(EXTI0_IRQn); // pin #0 -> EXTI #0
        HAL_NVIC_SetPriority(EXTI0_IRQn, 5, 0);
    }
    else
    {
        HAL_NVIC_DisableIRQ(EXTI0_IRQn); // pin #0 -> EXTI #0

        // put the pin back to tri-state ... as
        // output open-drain (not active)
        GPIO_InitStruct.Pin = DW_RESET_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(DW_RESET_GPIO_Port, &GPIO_InitStruct);
        HAL_GPIO_WritePin(DW_RESET_GPIO_Port, DW_RESET_Pin, GPIO_PIN_SET);
    }
}

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn wakeup_device_with_io()
 *
 * @brief This function wakes up the device by toggling io with a delay.
 *
 * input None
 *
 * output -None
 *
 */
void wakeup_device_with_io(void)
{
    SET_WAKEUP_PIN_IO_HIGH;
    WAIT_200uSEC;
    SET_WAKEUP_PIN_IO_LOW;
}

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn make_very_short_wakeup_io()
 *
 * @brief This will toggle the wakeup pin for a very short time. The device should not wakeup
 *
 * input None
 *
 * output -None
 *
 */
void make_very_short_wakeup_io(void)
{
    uint8_t cnt;

    SET_WAKEUP_PIN_IO_HIGH;
    for (cnt = 0; cnt < 10; cnt++)
        __NOP();
    SET_WAKEUP_PIN_IO_LOW;
}

/* @fn      port_is_boot1_low
 * @brief   check the BOOT1 pin status.
 * @return  1 if ON and 0 for OFF
 * */
int port_is_boot1_low(void)
{
    return ((GPIO_ReadInputDataBit(TA_BOOT1_GPIO, TA_BOOT1)) ? (0) : (1));
}

/* @fn      port_is_boot1_on
 * @brief   check the BOOT1 pin is on.
 * @return  1 if ON and 0 for OFF
 * */
int port_is_boot1_on(uint16_t x)
{
    UNUSED(x);
    return ((GPIO_ReadInputDataBit(TA_BOOT1_GPIO, TA_BOOT1)) ? (0) : (1));
}

/* @fn      port_is_switch_on
 * @brief   check the switch status.
 *          when switch (S1) is 'on' the pin is low
 * @return  1 if ON and 0 for OFF
 * */
int port_is_switch_on(uint16_t GPIOpin)
{
    return ((GPIO_ReadInputDataBit(TA_SW1_GPIO, GPIOpin)) ? (0) : (1));
}

/* @fn      led_off
 * @brief   switch off the led from led_t enumeration
 * */
void led_off(led_t led)
{
#if (EVB1000_LCD_SUPPORT == 1)
    switch (led)
    {
    case LED_PC6:
        GPIO_ResetBits(LED5_GPIO_Port, LED5_Pin);
        break;
    case LED_PC7:
        GPIO_ResetBits(LED6_GPIO_Port, LED6_Pin);
        break;
    case LED_PC8:
        GPIO_ResetBits(LED7_GPIO_Port, LED7_Pin);
        break;
    case LED_PC9:
        GPIO_ResetBits(LED8_GPIO_Port, LED8_Pin);
        break;
    case LED_ALL:
        GPIO_ResetBits(LED5_GPIO_Port, LED5_Pin);
        GPIO_ResetBits(LED6_GPIO_Port, LED6_Pin);
        GPIO_ResetBits(LED7_GPIO_Port, LED7_Pin);
        GPIO_ResetBits(LED8_GPIO_Port, LED8_Pin);
        break;
    default:
        // do nothing for undefined led number
        break;
    }
#else
    UNUSED(led);
#endif
}

/* @fn      led_on
 * @brief   switch on the led from led_t enumeration
 * */
void led_on(led_t led)
{
#if (EVB1000_LCD_SUPPORT == 1)
    switch (led)
    {
    case LED_PC6:
        GPIO_SetBits(LED5_GPIO_Port, LED5_Pin);
        break;
    case LED_PC7:
        GPIO_SetBits(LED6_GPIO_Port, LED6_Pin);
        break;
    case LED_PC8:
        GPIO_SetBits(LED7_GPIO_Port, LED7_Pin);
        break;
    case LED_PC9:
        GPIO_SetBits(LED8_GPIO_Port, LED8_Pin);
        break;
    case LED_ALL:
        GPIO_SetBits(LED5_GPIO_Port, LED5_Pin);
        GPIO_SetBits(LED6_GPIO_Port, LED6_Pin);
        GPIO_SetBits(LED7_GPIO_Port, LED7_Pin);
        GPIO_SetBits(LED8_GPIO_Port, LED8_Pin);
        break;
    default:
        // do nothing for undefined led number
        break;
    }
#else
    UNUSED(led);
#endif
}


/* @fn      change_SPI
 * @brief   Select Host to work with (SPI1 or SPI4)
 *
 * @param   spi - HOST enum to work with
 * */
void change_SPI(host_using_spi_e    spi)
{
    if (spi==SPI_1)
    {
        hcurrent_active_spi=&hspi1;
        pin_io_active_spi=DW_NSS_Pin;
        SPI_CS_state=GPIO_PIN_RESET;
    }
    else
    {//SPI 2
        hcurrent_active_spi=&hspi4;
        pin_io_active_spi=DW_NSS1_WAKEUP_Pin;
        SPI_CS_state=GPIO_PIN_SET;
    }

    host_spi = spi;
}


/* @fn      port_set_dw_ic_spi_slowrate
 * @brief   set 4.5MHz
 *          note: hspi1 is clocked from 72MHz
 * */
void port_set_dw_ic_spi_slowrate(void)
{
    hcurrent_active_spi->Init.BaudRatePrescaler=SPI_BAUDRATEPRESCALER_16;
    HAL_SPI_Init(hcurrent_active_spi);
}

/* @fn      port_set_dw_ic_spi_fastrate
 * @brief   set 18MHz
 *          note: hspi1 is clocked from 72MHz
 * */
void port_set_dw_ic_spi_fastrate(void)
{
    hcurrent_active_spi->Init.BaudRatePrescaler=SPI_BAUDRATEPRESCALER_2;
    HAL_SPI_Init(hcurrent_active_spi);
}

/* @fn      port_LCD_RS_set
 * @brief   wrapper to set LCD_RS pin
 * */
void port_LCD_RS_set(void)
{
#if (EVB1000_LCD_SUPPORT == 1)
    HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_SET);
#endif
}

/* @fn      port_LCD_RS_clear
 * @brief   wrapper to clear LCD_RS pin
 * */
void port_LCD_RS_clear(void)
{
#if (EVB1000_LCD_SUPPORT == 1)
    HAL_GPIO_WritePin(LCD_RS_GPIO_Port, LCD_RS_Pin, GPIO_PIN_RESET);
#endif
}

/* @fn      port_LCD_RW_clear
 * @brief   wrapper to set LCD_RW pin
 * */
void port_LCD_RW_set(void)
{
#if (EVB1000_LCD_SUPPORT == 1)
    HAL_GPIO_WritePin(LCD_RW_GPIO_Port, LCD_RW_Pin, GPIO_PIN_SET);
#endif
}

/* @fn      port_LCD_RW_clear
 * @brief   wrapper to clear LCD_RW pin
 * */
void port_LCD_RW_clear(void)
{
#if (EVB1000_LCD_SUPPORT == 1)
    HAL_GPIO_WritePin(LCD_RW_GPIO_Port, LCD_RW_Pin, GPIO_PIN_RESET);
#endif
}

/****************************************************************************
 *
 *                          End APP port section
 *
 *******************************************************************************/

/****************************************************************************
 *
 *                              IRQ section
 *
 *******************************************************************************/

/* @fn         HAL_GPIO_EXTI_Callback
 * @brief      EXTI line detection callback from HAL layer
 * @param      GPIO_Pin: Specifies the port pin connected to corresponding EXTI line.
 *             i.e. DW_RESET_Pin and DW_IRQn_Pin
 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    switch (GPIO_Pin)
    {
    case DW_RESET_Pin:
        signalResetDone = 1;
        break;

    case DW_IRQn_Pin :
    case DW_IRQ2_Pin :
        {
            {
                process_deca_irq();
            }

        break;
    }

    default:
        break;
    }
}

/* @fn      process_deca_irq
 * @brief   main call-back for processing of DW3000 IRQ
 *          it re-enters the IRQ routing and processes all events.
 *          After processing of all events, DW3000 will clear the IRQ line.
 * */
__INLINE void process_deca_irq(void)
{
    while (port_CheckEXT_IRQ() != 0)
    {
        if (port_dwic_isr)
        {
            port_dwic_isr();
        }
    } // while DW3000 IRQ line active
}

/* @fn      port_DisableEXT_IRQ
 * @brief   wrapper to disable DW_IRQ pin IRQ
 *          in current implementation it disables all IRQ from lines 5:9
 * */
__INLINE void port_DisableEXT_IRQ(void)
{
    if(SPI_1 == host_spi)
    {
        NVIC_DisableIRQ(DECAIRQ_EXTI_IRQn);
    }
    else
    {
        NVIC_DisableIRQ(DECAIRQ_EXTI_IRQn2);
    }

}

/* @fn      port_EnableEXT_IRQ
 * @brief   wrapper to enable DW_IRQ pin IRQ
 *          in current implementation it enables all IRQ from lines 5:9
 * */
__INLINE void port_EnableEXT_IRQ(void)
{
    if(SPI_1 == host_spi)
    {
        NVIC_EnableIRQ(DECAIRQ_EXTI_IRQn);
    }
    else
    {
        NVIC_EnableIRQ(DECAIRQ_EXTI_IRQn2);
    }
}

/* @fn      port_GetEXT_IRQStatus
 * @brief   wrapper to read a DW_IRQ pin IRQ status
 * */
__INLINE uint32_t port_GetEXT_IRQStatus(void)
{
    uint32_t status;

    if(SPI_1 == host_spi)
    {
        status = EXTI_GetITEnStatus(DECAIRQ_EXTI_IRQn);
    }
    else
    {
        status = EXTI_GetITEnStatus(DECAIRQ_EXTI_IRQn2);
    }
    return status;
}

/* @fn      port_CheckEXT_IRQ
 * @brief   wrapper to read DW_IRQ input pin state
 * */
__INLINE uint32_t port_CheckEXT_IRQ(void)
{
    uint32_t status;

    if(SPI_1 == host_spi)
    {
        status = HAL_GPIO_ReadPin(DECAIRQ_GPIO, DW_IRQn_Pin);;
    }
    else
    {
        status = HAL_GPIO_ReadPin(DECAIRQ2_GPIO, DW_IRQ2_Pin);
    }
    return status;
}

/****************************************************************************
 *
 *                              END OF IRQ section
 *
 *******************************************************************************/

/****************************************************************************
 *
 *                              USB report section
 *
 *******************************************************************************/
#include <usb_device.h>

#define REPORT_BUFSIZE 0x2000

extern USBD_HandleTypeDef hUsbDeviceFS;

static struct
{
    HAL_LockTypeDef Lock; /*!< locking object */
} txhandle = { .Lock = HAL_UNLOCKED };

static char rbuf[REPORT_BUFSIZE]; /**< circular report buffer, data to be transmitted in flush_report_buff() Thread */
static struct circ_buf report_buf = { .buf = rbuf, .head = 0, .tail = 0 };

static uint8_t ubuf[CDC_DATA_FS_MAX_PACKET_SIZE]; /**< used to transmit new chunk of data in single USB flush */

/* @fn      port_tx_msg()
 * @brief   put message to circular report buffer
 *          it will be transmitted in background ASAP from flushing Thread
 * @return  HAL_BUSY - can not do it now, wait for release
 *          HAL_ERROR- buffer overflow
 *          HAL_OK   - scheduled for transmission
 * */
HAL_StatusTypeDef port_tx_msg(uint8_t *str, int len)
{
    int head, tail, size;
    HAL_StatusTypeDef ret;

    /* add TX msg to circular buffer and increase circular buffer length */

    __HAL_LOCK(&txhandle); // return HAL_BUSY if locked
    head = report_buf.head;
    tail = report_buf.tail;
    __HAL_UNLOCK(&txhandle);

    size = REPORT_BUFSIZE;

    if (CIRC_SPACE(head, tail, size) > (len))
    {
        while (len > 0)
        {
            report_buf.buf[head] = *(str++);
            head = (head + 1) & (size - 1);
            len--;
        }

        __HAL_LOCK(&txhandle); // return HAL_BUSY if locked
        report_buf.head = head;
        __HAL_UNLOCK(&txhandle);

#ifdef CMSIS_RTOS
        osSignalSet(usbTxTaskHandle, signalUsbFlush); // RTOS multitasking signal start flushing
#endif
        ret = HAL_OK;
    }
    else
    {
        /* if packet can not fit, setup TX Buffer overflow ERROR and exit */
        ret = HAL_ERROR;
    }

    return ret;
}

/* @fn      flush_report_buff
 * @brief   FLUSH should have higher priority than port_tx_msg()
 *          it shall be called periodically from process, which can not be locked,
 *          i.e. from independent high priority thread
 * */
HAL_StatusTypeDef flush_report_buff(void)
{
    USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef *)(hUsbDeviceFS.pClassData);

    int i, head, tail, len, size = REPORT_BUFSIZE;

    __HAL_LOCK(&txhandle); //"return HAL_BUSY;" if locked
    head = report_buf.head;
    tail = report_buf.tail;
    __HAL_UNLOCK(&txhandle);

    len = CIRC_CNT(head, tail, size);

    if (len > 0)
    {
        /*  check USB status - ready to TX */
        if ((hUsbDeviceFS.dev_state != USBD_STATE_CONFIGURED) || (hcdc->TxState != 0))
        {
            return HAL_BUSY; /**< USB is busy. Let it send now, will return next time */
        }

        /* copy MAX allowed length from circular buffer to non-circular TX buffer */
        len = MIN(CDC_DATA_FS_MAX_PACKET_SIZE, len);

        for (i = 0; i < len; i++)
        {
            ubuf[i] = report_buf.buf[tail];
            tail = (tail + 1) & (size - 1);
        }

        __HAL_LOCK(&txhandle); //"return HAL_BUSY;" if locked
        report_buf.tail = tail;
        __HAL_UNLOCK(&txhandle);

        /* setup USB IT transfer */
        if (CDC_Transmit_FS(ubuf, (uint16_t)len) != USBD_OK)
        {
            /**< indicate USB transmit error */
        }
    }

    return HAL_OK;
}

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn port_set_dwic_isr()
 *
 * @brief This function is used to install the handling function for DW IC IRQ.
 *
 * NOTE:
 *   - The user application shall ensure that a proper handler is set by calling this function before any DW IC IRQ occurs.
 *   - This function deactivates the DW IC IRQ line while the handler is installed.
 *
 * @param deca_isr function pointer to DW IC interrupt handler to install
 *
 * @return none
 */
void port_set_dwic_isr(port_dwic_isr_t dwic_isr)
{
    /* Check DW IC IRQ activation status. */
    ITStatus en = port_GetEXT_IRQStatus();

    /* If needed, deactivate DW IC IRQ during the installation of the new handler. */
    port_DisableEXT_IRQ();

    port_dwic_isr = dwic_isr;

    if (!en)
    {
        port_EnableEXT_IRQ();
    }
}

#if (EVB1000_LCD_SUPPORT == 1)
/*! ------------------------------------------------------------------------------------------------------------------
 * @fn writetoLCD()
 *
 * @brief this is the SPI write function to send data to the LCD display
 *
 * @param  rs_enable   when this is set to 0 the bodyBuffer should contain a command byte - just a single byte,
 *                     when this is 1, the data in the bodyBuffer should contain string to display on the LCD
 * @param  bodyBuffer  array of the bytes of the string to display
 *
 * @return none
 */
void writetoLCD(uint32_t bodylength, uint8_t rs_enable, const uint8_t *bodyBuffer)
{

    while (HAL_SPI_GetState(&hspi2) != HAL_SPI_STATE_READY)
        ;

    int sleep = 0;

    if (rs_enable)
    {
        port_LCD_RS_set();
    }
    else
    {
        if (bodylength == 1)
        {
            if (bodyBuffer[0] & 0x3) // if this is command = 1 or 2 - execution time is > 1ms
                sleep = 1;
        }
        port_LCD_RS_clear();
    }

    port_SPIy_clear_chip_select(); // CS low for SW controllable SPI_NSS

    HAL_SPI_Transmit(&hspi2, (uint8_t *)bodyBuffer, bodylength, HAL_MAX_DELAY);

    port_LCD_RS_clear();
    port_SPIy_set_chip_select(); // CS high for SW controllable SPI_NSS

    if (sleep)
        Sleep(2);
} // end writetoLCD()

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn lcd_display_str()
 *
 * @brief Display a string on the LCD screen.
 * /!\ The string must be 16 chars long maximum!
 *
 * @param  string  the string to display
 *
 * @return none
 */
void lcd_display_str(const char *string1)
{
    uint8_t command;
    /* Return cursor home and clear screen. */
    command = 0x2;
    writetoLCD(1, 0, &command);
    command = 0x1;
    writetoLCD(1, 0, &command);
    /* Write the string to display. */
    // writetoLCD(strlen(string), 1, (const uint8_t *)string);
    writetoLCD(40, 1, (const uint8_t *)string1);
}

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn lcd_display_str2()
 *
 * @brief Display a string on the LCD screen.
 * /!\ Both of the strings must be 16 chars long maximum!
 *
 * @param  string  the string to display on the 1st line
 * @param  string  the string to display on the second line
 *
 * @return none
 */
void lcd_display_str2(const char *string1, const char *string2)
{
    uint8_t command;
    /* Return cursor home and clear screen. */
    command = 0x2;
    writetoLCD(1, 0, &command);
    command = 0x1;
    writetoLCD(1, 0, &command);
    /* Write the strings to display. */
    writetoLCD(40, 1, (const uint8_t *)string1); // sending 40 bytes to align the second line
    writetoLCD(16, 1, (const uint8_t *)string2); // send the data for second line of the display
}

#endif
/****************************************************************************
 *
 *                              END OF Report section
 *
 *******************************************************************************/
