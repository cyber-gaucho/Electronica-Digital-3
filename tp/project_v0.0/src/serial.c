/**
 * @file serial.c
 * @brief Unified serial communication implementation (UART/USB)
 */
#include "serial.h"
#include "lpc17xx_pinsel.h"
#include <stdio.h>
#include <string.h>

#ifdef SERIAL_UART

// Internal buffer for printf formatting
static char printf_buffer[256];

/**
 * @brief Initialize UART0 (TX P0.2, RX P0.3)
 */
void uart_init(void)
{
	PINSEL_CFG_Type cfgPinTXD0;
	PINSEL_CFG_Type cfgPinRXD0;

	cfgPinTXD0.Portnum = 0;
	cfgPinTXD0.Pinnum = 2;
	cfgPinTXD0.Funcnum = 1;
	cfgPinTXD0.Pinmode = PINSEL_PINMODE_NORMAL;
	cfgPinTXD0.OpenDrain = PINSEL_PINMODE_NORMAL;
	PINSEL_ConfigPin(&cfgPinTXD0);

	cfgPinRXD0.Portnum = 0;
	cfgPinRXD0.Pinnum = 3;
	cfgPinRXD0.Funcnum = 1;
	cfgPinRXD0.Pinmode = PINSEL_PINMODE_NORMAL;
	cfgPinRXD0.OpenDrain = PINSEL_PINMODE_NORMAL;
	PINSEL_ConfigPin(&cfgPinRXD0);

	UART_CFG_Type UARTConfig;
	UART_FIFO_CFG_Type FIFOConfig;

	UART_ConfigStructInit(&UARTConfig);
	UART_FIFOConfigStructInit(&FIFOConfig);

	UART_Init(UART_PORT, &UARTConfig);
	UART_FIFOConfig(UART_PORT, &FIFOConfig);
	UART_TxCmd(UART_PORT, ENABLE);
}

/**
 * @brief Send a string via UART
 */
void uart_SendString(uint8_t *str)
{
    UART_Send(UART_PORT, str, strlen((char *)str), BLOCKING);
}

/**
 * @brief Unified initialization - UART version
 */
void serial_init(void)
{
    uart_init();
}

/**
 * @brief Unified printf - UART version
 */
void serial_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int len = vsnprintf(printf_buffer, sizeof(printf_buffer), format, args);
    va_end(args);
    
    if (len > 0 && len < sizeof(printf_buffer))
    {
        UART_Send(UART_PORT, (uint8_t *)printf_buffer, len, BLOCKING);
    }
}

/**
 * @brief Unified send string - UART version
 */
void serial_send_string(const char *str)
{
    uart_SendString((uint8_t *)str);
}

/**
 * @brief Unified send char - UART version
 */
void serial_send_char(char c)
{
    UART_Send(UART_PORT, (uint8_t *)&c, 1, BLOCKING);
}

#endif /* SERIAL_UART */

#ifdef SERIAL_USB

// Internal buffer for printf formatting
static char printf_buffer[256];

/**
 * @brief Initialize USB CDC device
 */
void USB_InitDevice(void)
{
    USB_Init();
    USB_Connect(TRUE);
}

/**
 * @brief Send a string via USB CDC
 */
void USB_SendString(const char *str)
{
    const char *p = str;
    while (*p)
    {
        CDC_WrInBuf((uint8_t *)p, 1);
        p++;
    }
}

/**
 * @brief Process USB tasks (call periodically in main loop)
 */
void USB_Run(void)
{
    CDC_Run();
}

/**
 * @brief Unified initialization - USB version
 */
void serial_init(void)
{
    USB_InitDevice();
}

/**
 * @brief Unified printf - USB version
 */
void serial_printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int len = vsnprintf(printf_buffer, sizeof(printf_buffer), format, args);
    va_end(args);
    
    if (len > 0 && len < sizeof(printf_buffer))
    {
        USB_SendString(printf_buffer);
    }
}

/**
 * @brief Unified send string - USB version
 */
void serial_send_string(const char *str)
{
    USB_SendString(str);
}

/**
 * @brief Unified send char - USB version
 */
void serial_send_char(char c)
{
    CDC_WrInBuf((uint8_t *)&c, 1);
}

#endif /* SERIAL_USB */
