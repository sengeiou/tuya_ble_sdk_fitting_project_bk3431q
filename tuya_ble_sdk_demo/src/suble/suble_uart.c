#include "suble_common.h"




/*********************************************************************
 * LOCAL CONSTANT
 */

/*********************************************************************
 * LOCAL STRUCT
 */

/*********************************************************************
 * LOCAL VARIABLE
 */

/*********************************************************************
 * VARIABLE
 */

/*********************************************************************
 * LOCAL FUNCTION
 */




/*********************************************************
FN: 
*/
static void uart1_rx_handler(uint8_t *buf, uint8_t len)
{
    tuya_ble_common_uart_send_full_instruction_received(buf, len);
//    SUBLE_HEXDUMP("uart1", buf, len);
}

/*********************************************************
FN: 
*/
static void uart2_rx_handler(uint8_t *buf, uint8_t len)
{
//    SUBLE_HEXDUMP("uart2", buf, len);
    tuya_ble_common_uart_send_full_instruction_received(buf, len);
}

/*********************************************************
FN: 
*/
void suble_uart1_init(void)
{
	uart_init(115200);
	uart_cb_register(uart1_rx_handler);
}

/*********************************************************
FN: 
*/
void suble_uart2_init(void)
{
	uart2_init(115200);
	uart2_cb_register(uart2_rx_handler);
}

/*********************************************************
FN: 
*/
void suble_uart1_send(const uint8_t* buf, uint32_t size)
{
    uart_send((void*)buf, size);
}

/*********************************************************
FN: 
*/
void suble_uart2_send(const uint8_t* buf, uint32_t size)
{
    uart2_send((void*)buf, size);
}

















