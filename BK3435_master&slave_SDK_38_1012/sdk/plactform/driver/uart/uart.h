/**
 ****************************************************************************************
 *
 * @file uart.h
 *
 * @brief UART Driver for HCI over UART operation.
 *
 * Copyright (C) Beken 2009-2015
 *
 *
 ****************************************************************************************
 */

#ifndef _UART_H_
#define _UART_H_

/**
 ****************************************************************************************
 * @defgroup UART UART
 * @ingroup DRIVERS
 * @brief UART driver
 *
 * @{
 *
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdbool.h>          // standard boolean definitions
#include <stdint.h>           // standard integer functions
#include "rwip_config.h"
#include "user_config.h"

#if (0)

#if  !BLE_TESTER
#define UART_PRINTF	uart_printf //uart_printf
#else
#define UART_PRINTF uart2_printf //uart_printf 
#define UART2_PRINTF uart2_printf
#endif //!BLE_TESTER

#else
#define UART_PRINTF(...) 
#define UART2_PRINTF uart2_printf
#endif // #if UART_PRINTF_EN
 
#define TX_FIFO_LENGTH 15
#define UART_SEND_LENGTH 128

#define UART_WRITE_BYTE(v)               (REG_APB3_UART_PORT=v)

#define BK_REG_CONSOLE      0


#define CLI_CONSOLE         1

/*
 * ENUMERATION DEFINITIONS
 *****************************************************************************************
 */

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @brief Initializes the UART to default values.
 *****************************************************************************************
 */
uint8_t uart_init(uint32_t baudrate);
uint8_t uart2_init(uint32_t baudrate);

void dbg_initial(void);

void uart_clear_rxfifo(void);

uint8_t Uart_Read_Byte(void);
uint8_t read_uart_ringbuf_data(uint8_t *buf,uint16_t len);
void uart_ringbuf_clean(void);
uint8_t Read_Uart_Buf(void);
void uart_data_send(uint8_t * buffer, uint16_t len);
int dbg_putchar(char * st);
int uart_putchar(char * st);
int uart_printf(const char *fmt,...);
int uart2_printf(const char *fmt,...);
int uart_printf_null(const char *fmt,...);
int uart2_printf_null(const char *fmt,...);
int dbg_printf(const char *fmt,...);
void uart_print_int(unsigned int num);
uint8_t check_uart_stop(void);

void cpu_delay( volatile unsigned int times);


/****** REG  ****************/
void uart_send(unsigned char *buff, int len);
void uart2_send(unsigned char *buff, int len);
void TRAhcit_UART_Rx(void);


#define TOTAL_BUF_NUM  512

#define UART0_RX_FIFO_MAX_COUNT  TOTAL_BUF_NUM

#define UART0_TX_FIFO_MAX_COUNT  TOTAL_BUF_NUM

extern volatile uint16_t uart_rx_buff_tailor;///uart_rx_buff_header,
extern unsigned char uart_rx_buf[UART0_RX_FIFO_MAX_COUNT];//volatile 
extern unsigned char uart_tx_buf[UART0_TX_FIFO_MAX_COUNT];
extern volatile bool uart_rx_done ;
extern volatile unsigned long uart_rx_index ;
/****** REG  ****************/


/**
 ****************************************************************************************
 * @brief Enable UART flow.
 *****************************************************************************************
 */
void uart_flow_on(void);

/**
 ****************************************************************************************
 * @brief Disable UART flow.
 *****************************************************************************************
 */
bool uart_flow_off(void);


/**
 ****************************************************************************************
 * @brief Finish current UART transfers
 *****************************************************************************************
 */
void uart_finish_transfers(void);

/**
 ****************************************************************************************
 * @brief Starts a data reception.
 *
 * @param[out] bufptr   Pointer to the RX buffer
 * @param[in]  size     Size of the expected reception
 * @param[in]  callback Pointer to the function called back when transfer finished
 * @param[in]  dummy    Dummy data pointer returned to callback when reception is finished
 *****************************************************************************************
 */
void uart_read(uint8_t *bufptr, uint32_t size, void (*callback) (void*, uint8_t), void* dummy);

/**
 ****************************************************************************************
 * @brief Starts a data transmission.
 *
 * @param[in] bufptr   Pointer to the TX buffer
 * @param[in] size     Size of the transmission
 * @param[in] callback Pointer to the function called back when transfer finished
 * @param[in] dummy    Dummy data pointer returned to callback when transmission is finished
 *****************************************************************************************
 */
void uart_write(uint8_t *bufptr, uint32_t size, void (*callback) (void*, uint8_t), void* dummy);


/**
 ****************************************************************************************
 * @brief Serves the data transfer interrupt requests.
 *
 * It clears the requests and executes the appropriate callback function.
 *****************************************************************************************
 */
void uart_isr(void);
void uart2_isr(void);

typedef void (*UART_RX_CALLBACK_T)(uint8_t *buf, uint8_t len);   

void uart_cb_register(UART_RX_CALLBACK_T cb);
void uart2_cb_register(UART_RX_CALLBACK_T cb);

void uart_cb_clear(void);
void uart2_cb_clear(void);

void Device_uart_rx(void);

void send_bt_rx_data(void);

/// @} UART
#endif /* _UART_H_ */
