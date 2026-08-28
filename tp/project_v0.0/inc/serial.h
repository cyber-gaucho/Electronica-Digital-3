/**
 * @file serial.h
 * @brief Unified serial communication interface (UART/USB)
 * @note Define SERIAL_UART or SERIAL_USB before including this header
 */
#ifndef SERIAL_H
#define SERIAL_H

#include "lpc_types.h"
#include <stdint.h>
#include <stdarg.h>

#ifdef SERIAL_UART
#include "lpc17xx_uart.h"
#include "lpc_types.h"
#define UART_PORT ((LPC_UART_TypeDef *) LPC_UART0_BASE)
#endif

#ifdef SERIAL_USB
#include "lpc17xx_usb.h"
#include "usbcore.h"
#include "usbdesc.h"
#include "usbuser.h"
#include "cdc.h"
#endif

// Unified interface functions
void serial_init(void);
void serial_printf(const char *format, ...);
void serial_send_string(const char *str);
void serial_send_char(char c);

#ifdef SERIAL_UART
// UART-specific functions
void UART_Init(void);
void UART_command(FunctionalState NewState);
void UART_SendString(uint8_t *str);
#endif

#ifdef SERIAL_USB
// USB-specific functions
void USB_InitDevice(void);
void USB_SendString(const char *str);
void USB_Run(void);  // Call periodically in main loop
#endif

#endif /* SERIAL_H */