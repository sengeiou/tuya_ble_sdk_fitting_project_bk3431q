/**
 ****************************************************************************************
 *
 * @file electric_uart_protocol.h
 *
 * @brief electric_uart Driver for HCI over UART operation.
 *
 * Copyright (C) Beken 2019-2022
 *
 *
 ****************************************************************************************
 */

#ifndef _ELECTRIC_H_
#define _ELECTRIC_H_
#include <stdint.h>



//void electric_rx_buf(uint8_t ch);
void send_bt_rx_data_process(void);
void uart_protocol_task(void);
#endif //
