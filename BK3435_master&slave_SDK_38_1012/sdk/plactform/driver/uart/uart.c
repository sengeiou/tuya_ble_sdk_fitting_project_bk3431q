/**
 ****************************************************************************************
 *
 * @file uart.c
 *
 * @brief UART driver
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 *
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup UART
 * @{
 ****************************************************************************************
 */ 
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stddef.h>     // standard definition
#include <stdarg.h>
#include <stdio.h>     // 
#include "timer.h"      // timer definition
#include "uart.h"       // uart definition
#include "BK3435_reg.h"
#include "reg_uart.h"   // uart register
#include "reg_uart2.h"   // uart2 register
#include "rwip.h"       // SW interface
#include "h4tl.h"
#include "nvds.h"       // NVDS


#include "electric_uart_protocol.h"

#include "ea.h"
#include "ke_event.h"

#include "dbg.h"
#include "appm.h"             // Application definitions
#include "appc.h"             // Application definitions
 
/*
 * DEFINES
 *****************************************************************************************
 */

/// Max baudrate supported by this UART (in bps)
#define UART_BAUD_MAX      		  3500000
/// Min baudrate supported by this UART (in bps)
#define UART_BAUD_MIN      		  9600

/// Duration of 1 byte transfer over UART (10 bits) in us (for 921600 default baudrate)
#define UART_CHAR_DURATION        11

/*
 * ENUMERATION DEFINITIONS
 *****************************************************************************************
 */





#define Uart_Write_Byte(v)               (REG_APB3_UART_PORT=v)

#define UART_TX_FIFO_COUNT               (REG_APB3_UART_FIFO_STAT&0xff)
#define UART_RX_FIFO_COUNT               ((REG_APB3_UART_FIFO_STAT>>8)&0xff)
#define UART_TX_FIFO_FULL                (REG_APB3_UART_FIFO_STAT&0x00010000)
#define UART_TX_FIFO_EMPTY               (REG_APB3_UART_FIFO_STAT&0x00020000)
#define UART_RX_FIFO_FULL                (REG_APB3_UART_FIFO_STAT&0x00040000)
#define UART_RX_FIFO_EMPTY               (REG_APB3_UART_FIFO_STAT&0x00080000)
#define UART_TX_WRITE_READY              (REG_APB3_UART_FIFO_STAT&0x00100000)
#define UART_RX_READ_READY               (REG_APB3_UART_FIFO_STAT&0x00200000)

#define UART2_TX_FIFO_COUNT               (REG_APB3_UART2_FIFO_STAT&0xff)
#define UART2_RX_FIFO_COUNT               ((REG_APB3_UART2_FIFO_STAT>>8)&0xff)
#define UART2_TX_FIFO_FULL                (REG_APB3_UART2_FIFO_STAT&0x00010000)
#define UART2_TX_FIFO_EMPTY               (REG_APB3_UART2_FIFO_STAT&0x00020000)
#define UART2_RX_FIFO_FULL                (REG_APB3_UART2_FIFO_STAT&0x00040000)
#define UART2_RX_FIFO_EMPTY               (REG_APB3_UART2_FIFO_STAT&0x00080000)
#define UART2_TX_WRITE_READY              (REG_APB3_UART2_FIFO_STAT&0x00100000)
#define UART2_RX_READ_READY               (REG_APB3_UART2_FIFO_STAT&0x00200000)

#define bit_UART_TXFIFO_NEED_WRITE        0x01
#define bit_UART_RXFIFO_NEED_READ         0x02
#define bit_UART_RXFIFO_OVER_FLOW         0x04
#define bit_UART_RX_PARITY_ERROR          0x08
#define bit_UART_RX_STOP_ERROR            0x10
#define bit_UART_TX_PACKET_END            0x20
#define bit_UART_RX_PACKET_END            0x40
#define bit_UART_RXD_WAKEUP_DETECT        0x80


#define uart_tx_en    0x1      // 0: Disable TX, 1: Enable TX 
#define uart_rx_en    0x1      // 0: Disable RX, 1: Enable RX
#define irda_mode     0x0      // 0: UART  MODE, 1: IRDA MODE
#define data_len      0x3      // 0: 5 bits, 1: 6 bits, 2: 7 bits, 3: 8 bits
#define parity_en     0x0      // 0: NO Parity, 1: Enable Parity
#define parity_mode   0x1      // 0: Odd Check, 1: Even Check 
#define stop_bits     0x0      // 0: 1 stop-bit, 1: 2 stop-bit 
#define uart_clks     16000000 // UART's Main-Freq, 26M 
#define baud_rate     115200   // UART's Baud-Rate,  1M 


volatile uint16_t uart_rx_buff_tailor=0;///uart_rx_buff_header=0,

unsigned char uart_rx_buf[UART0_RX_FIFO_MAX_COUNT];//volatile 
unsigned char uart_tx_buf[UART0_TX_FIFO_MAX_COUNT];
volatile bool uart_rx_done = 0;
volatile unsigned long uart_rx_index = 0;
volatile unsigned long uart_write_index = 0;
uint8_t cur_read_buf_idx = 0;

unsigned char uart2_rx_buf[UART0_RX_FIFO_MAX_COUNT];
unsigned char uart2_tx_buf[UART0_TX_FIFO_MAX_COUNT];
volatile bool uart2_rx_done = 0;
volatile unsigned long uart2_rx_index = 0;
uint8_t cur_read_buf_idx2 = 0;

//reaord index
volatile uint16_t pbuff_write = 0;
//read index
volatile uint16_t pbuff_read = 0;
void* ph4tl_env;



/*******************************************************************************
 * Function: store_uart_ringbuf_data
 * Description: store data into loop buffer
 * Input: uint8_t*
 * Input: uint16_t
 * Output: void
 * Return: uint8_t
 * Others: void
*******************************************************************************/

uint8_t store_uart_ringbuf_data(uint8_t *buf,uint16_t len)
{
	uint16_t free_cnt;
	uint8_t status ;
	
	//Calculates the number of empty buffer in the circular queue (the 
	//data stored in this queue is encoded)
	if(pbuff_write >= pbuff_read)
	{
		free_cnt = (TOTAL_BUF_NUM - pbuff_write + pbuff_read);
	}else
	{
		free_cnt = pbuff_read - pbuff_write;
	}
	//UART_PRINTF("free cnt: %d,store len : %d\r\n", free_cnt,len);
	
	//If there are at least two empty buffer in the loop queue, the current 
	//encoded data will be stored in buffer. 
	if(free_cnt >= len) 
	{
		memcpy((uint8_t *)&uart_rx_buf[pbuff_write],buf,len);
		
		if(0){
			UART_PRINTF("buf %d:",uart_rx_index);
			for(int i =0;i < uart_rx_index;i++)
			{
				UART_PRINTF("%02x ",uart_rx_buf[i]);
			}
			UART_PRINTF("%\r\n");
		}
		//Update the buffer index of the data 
		//(in fact, the index is between 0-79)
		pbuff_write = ((pbuff_write + len )% TOTAL_BUF_NUM);
		status = 1;
		//UART_PRINTF("enough data write!! : pbuff_write : %d, pbuff_read :%d\r\n",pbuff_write,pbuff_read);	
	}else
	{
		UART_PRINTF("no enough buff fill data %d,%d!!!\r\n",pbuff_write,pbuff_read); // for test show
		status = 0;
	}
		
	return status;
}


 /*******************************************************************************
 * Function: read_uart_ringbuf_data
 * Description: read data from loop buffer
 * Input: uint8_t*
 * Output: void
 * Return: uint8_t readed cnt
 * Others: void
*******************************************************************************/
uint8_t  read_uart_ringbuf_data(uint8_t *buf,uint16_t len)
{
	uint16_t unread_cnt;
	//Read 20 encode data from loop buffer to designated buffer 
	if(pbuff_write >= pbuff_read)
	{
		unread_cnt = pbuff_write - pbuff_read;
		
		//UART_PRINTF("read 0x%x\r\n",pbuff_read);	
	}else
	{
		unread_cnt = (TOTAL_BUF_NUM - pbuff_read  + pbuff_write);
		//UART_PRINTF("buff empty!!0x%x\r\n",pbuff_read);	
	} 
	//UART_PRINTF("unread_cnt : %d,read len :%d\r\n", unread_cnt,len); 
	
	
	if(unread_cnt >= len)
	{
        memcpy(buf,(uint8_t *)&uart_rx_buf[pbuff_read],len);

        if(0){
            UART_PRINTF("bufR %d:",uart_rx_index);
            for(int i =0;i < len;i++)
            {
            UART_PRINTF("%02x ",buf[i]);
            }
            UART_PRINTF("%\r\n");
        }
					//Update the buffer index of the data 
		//(in fact, the index is between 0-255)
        pbuff_read = ((pbuff_read + len )% TOTAL_BUF_NUM);
      //  UART_PRINTF("enough data read!! pbuff_write : %d,pbuff_read :%d\r\n",pbuff_write,pbuff_read);	
        return len;
	}else
	{
       // UART_PRINTF("buff no enough data read!!pbuff_write :%d,pbuff_read :%d\r\n",pbuff_write,pbuff_read);	
        return 0;
	}
	
}
#if 0
void app_uart_clean(void)
{
    memset((uint8_t *)uart_tx_buf,0,sizeof(uart_tx_buf));
		//memset((uint8_t *)uart_rx_buf,0,sizeof(uart_rx_buf));
		uart_rx_index = 0;///uart_rx_buff_header = 0;
		uart_rx_buff_tailor = 0;
}
#endif
 /*******************************************************************************
 * Function: uart_ringbuf_clean
 * Description: read data from loop buffer
 * Input: uint8_t*
 * Output: void
 * Return: uint8_t readed cnt
 * Others: void
*******************************************************************************/
void uart_ringbuf_clean(void)
{
    pbuff_write = pbuff_read = 0;
    memset((void *)uart_rx_buf,0,TOTAL_BUF_NUM);
		memset((uint8_t *)uart_tx_buf,0,TOTAL_BUF_NUM);
		uart_rx_index = 0;///uart_rx_buff_header = 0;
		uart_rx_buff_tailor = 0;
  //  UART_PRINTF("uart_ringbuf_clean \r\n");
}


extern uint8_t system_mode;
#define UART_READ_BYTE()                 (REG_APB3_UART_PORT&0xff)
#define UART2_READ_BYTE()                 (REG_APB3_UART2_PORT&0xff)

///UART Character format
enum UART_CHARFORMAT
{
    UART_CHARFORMAT_8 = 0,
    UART_CHARFORMAT_7 = 1
};

///UART Stop bit
enum UART_STOPBITS
{
    UART_STOPBITS_1 = 0,
    UART_STOPBITS_2 = 1  /* Note: The number of stop bits is 1.5 if a character format
                            with 5 bit is chosen*/
};

///UART Parity enable
enum UART_PARITY
{
    UART_PARITY_DISABLED = 0,
    UART_PARITY_ENABLED  = 1
};

///UART Parity type
enum UART_PARITYBIT
{
    UART_PARITYBIT_EVEN  = 0,
    UART_PARITYBIT_ODD   = 1,
    UART_PARITYBIT_SPACE = 2, // The parity bit is always 0.
    UART_PARITYBIT_MARK  = 3  // The parity bit is always 1.
};

///UART HW flow control
enum UART_HW_FLOW_CNTL
{
    UART_HW_FLOW_CNTL_DISABLED = 0,
    UART_HW_FLOW_CNTL_ENABLED = 1
};

///UART Input clock select
enum UART_INPUT_CLK_SEL
{
    UART_INPUT_CLK_SEL_0 = 0,
    UART_INPUT_CLK_SEL_1 = 1,
    UART_INPUT_CLK_SEL_2 = 2,
    UART_INPUT_CLK_SEL_3 = 3
};

///UART Interrupt enable/disable
enum UART_INT
{
    UART_INT_DISABLE = 0,
    UART_INT_ENABLE = 1
};

///UART Error detection
enum UART_ERROR_DETECT
{
    UART_ERROR_DETECT_DISABLED = 0,
    UART_ERROR_DETECT_ENABLED  = 1
};

/*
 * STRUCT DEFINITIONS
 *****************************************************************************************
 */
/* TX and RX channel class holding data used for asynchronous read and write data
 * transactions
 */
/// UART TX RX Channel
struct uart_txrxchannel
{
    /// call back function pointer
    void (*callback) (void*, uint8_t);
    /// Dummy data pointer returned to callback when operation is over.
    void* dummy;
};

/// UART environment structure
struct uart_env_tag
{
    /// tx channel
    struct uart_txrxchannel tx;
    /// rx channel
    struct uart_txrxchannel rx;
    /// error detect
    uint8_t errordetect;
    /// external wakeup
    bool ext_wakeup;
};

/*
 * GLOBAL VARIABLE DEFINITIONS
 ****************************************************************************************
 */

/// uart environment structure
struct uart_env_tag uart_env;
char uart_buff[256];
struct uart_env_tag uart2_env;
char uart2_buff[UART0_RX_FIFO_MAX_COUNT];



static UART_RX_CALLBACK_T uart_rx_cb = NULL; 
static UART_RX_CALLBACK_T usrt2_rx_cb = NULL;

/*
 * LOCAL FUNCTION DEFINITIONS
 ****************************************************************************************
 */


/*
 * EXPORTED FUNCTION DEFINITIONS
 ****************************************************************************************
 */
uint8_t Read_Uart_Buf(void)
{
	return uart_rx_buf[cur_read_buf_idx++];
}	


uint8_t Uart_Read_Byte(void)
{
	return (REG_APB3_UART_PORT&0xff);
}	

 
int uart_putchar(char * st)
{
	uint8_t num = 0;
    while (*st) 
    {
		if(UART_TX_WRITE_READY)
		{
			REG_APB3_UART_PORT = *st;
	    	st++;
		 	num++;
	    }
	} 
    return num; 
}  

int uart2_putchar(char * st)
{
	uint8_t num = 0;
	while (*st)
	{
		if(UART2_TX_WRITE_READY)
		{
			REG_APB3_UART2_PORT = *st;
			st++;
			num++;
		}
	}
	return num;
}

void uart_buff_send(volatile uint8_t **buf, volatile uint32_t *length)
{   
    uint32_t count = *length;
    uint32_t fifo_length;
    do
    {
        fifo_length=UART_TX_FIFO_COUNT&0xff;
    }while(fifo_length>(TX_FIFO_LENGTH/2));    
    /* Wait until the UART transmitter is empty */      

    fifo_length=TX_FIFO_LENGTH-fifo_length;//need tune again
    if(count > fifo_length)
        count = fifo_length;       

    (*length)-= count; 
     do
    {
        while(!UART_TX_WRITE_READY);
        UART_WRITE_BYTE(*((*buf)++));
        count--;            
    }
    while(count);
     
}

void uart_data_send(uint8_t * buffer, uint16_t len)
{
    uint32_t length2 = len;   
    uint8_t* buffer2 = &buffer[0];
    
    while(length2)
    {
        uart_buff_send((volatile uint8_t**)&buffer2, (uint32_t*)&length2);
    }
    
}
int uart_printf(const char *fmt,...)
{
#if 1///(UART_PRINTF_EN && UART_DRIVER)
	
	va_list args;
    int ret;	
	
	va_start(args, fmt);
	ret = vsprintf(uart_buff, fmt, args);
	va_end(args);
    
    uart_putchar(uart_buff);
	if(ret > sizeof(uart_buff))
	{
		uart_putchar("buff full \r\n");
	}
		
	return ret;
#else
    return 0;
#endif
}

int uart2_printf(const char *fmt,...)
{
//#if (UART_PRINTF_EN && UART_DRIVER)
#if (1)
	int n;

	va_list ap;
	va_start(ap, fmt);
	n=vsprintf(uart2_buff, fmt, ap);
	va_end(ap);
	uart2_putchar(uart2_buff);
	if(n > sizeof(uart2_buff))
	{
		uart2_putchar("buff full \r\n");
	}

	return n;
#else
    return 0;
#endif
}

int uart_printf_null(const char *fmt,...)
{
	return 0;
}

int uart2_printf_null(const char *fmt,...)
{
	return 0;
}


char *hex2Str( uint8_t data)
{

	char hex[] = "0123456789ABCDEF";
	static char str[3];
	char *pStr = str;

	*pStr++ = hex[data >> 4];
	*pStr++ = hex[data & 0x0F];
	*pStr = 0;

	return str;
}

void uart_print_int(unsigned int num)
{
#if UART_PRINTF_EN
	uint8_t i;
	uint8_t m;

	uart_putchar((char *)"0x");
	for (i = 4; i > 0; i--)
	{
		m = ((num >> (8 * (i - 1)))& 0xff);
		uart_putchar(hex2Str(m));
	}
	uart_putchar("\r\n");
#endif    
}



void cpu_delay( volatile unsigned int times)
{
	while(times--)
	{
		for(uint32_t i = 0; i < 1000; i++)
			;
	}
}


uint8_t uart_init(uint32_t baudrate)
{
	uint8_t ret = 0;
	unsigned int baud_divisor ;
	REG_AHB0_ICU_UARTCLKCON   &= ~(0x1 << 0) ;  // Enable Uart's Clocks
	switch(baudrate)
	{
	case 9600:
		baud_divisor = 0x681;
		break;
	case 19200:
		baud_divisor = 0x340;
		break;
	case 38400:
		baud_divisor = 0x19F;
		break;
	case 57600:
		baud_divisor = 0x114;
		break;
	case 115200:
		baud_divisor = 0x89;
		break;
	case 128000:
		baud_divisor = 0x7C;
		break;
	case 256000:
		baud_divisor = 0x3D;
		break;
	case 460800:
		baud_divisor = 0x21;
		break;
	case 512000:
		baud_divisor = 0x1E;
		break;
	case 921600:
		baud_divisor = 0x10;
		break;
	case 1000000:
		baud_divisor = 0x0F;
		break;
	case 1500000:
		baud_divisor = 0x09;
		break;
	case 2000000:
		baud_divisor = 0x07;
		break;
	//case 3250000:
		//baud_divisor = 0x04;
		//break;
	default:
		ret = 1;
		baud_divisor = 0x89;
		break;
	}
	REG_APB3_UART_CFG  = (baud_divisor<<8) +
	                     (stop_bits   <<7) +
	                     (data_len    <<3) +
	                     (irda_mode   <<2) +
	                     (uart_rx_en  <<1) +
	                     uart_tx_en ;

	REG_APB3_UART_FIFO_CFG = (1<<BIT_TX_FIFO_THRESHOLD)|(16<<BIT_RX_FIFO_THRESHOLD)|(0x1 << BIT_STOP_DETECT_TIME);//(0x3 << BIT_STOP_DETECT_TIME);
	REG_APB3_UART_INT_ENABLE = ((0x01 << 1) | (0x01 << 6) | (0x01 << 7));
	REG_APB3_UART_FLOW_CFG  = 0x00000000 ;  // No Flow Control
	REG_APB3_UART_WAKE_CFG  = ((0x01 << 0 )| (0x01 << 20) |  (0x01 << 21)| (0x01 << 22));  // No Wake Control

	REG_APB5_GPIOA_CFG  &= ~((0x3<<BIT_GPIO_PULL_UP)  + (0x3<<BIT_GPIO_PERI_EN));
	REG_APB5_GPIOA_CFG  |= ((0x3<<BIT_GPIO_PULL_UP));
	REG_APB5_GPIOA_CFG  |=   (0x3<<BIT_GPIO_OUT_EN_N);

	REG_APB5_GPIOA_DATA &= ~ (0x3<<BIT_GPIO_INPUT_EN);


	uart_env.rx.callback = NULL;
	uart_env.rx.dummy    = NULL;

	REG_AHB0_ICU_INT_ENABLE |= (0x01 << 5);
	return ret;
}

uint8_t uart2_init(uint32_t baudrate)
{
	uint8_t ret = 0;
	unsigned int baud_divisor ;
	REG_AHB0_ICU_UARTCLKCON   &= ~(0x1 << 0) ;  // Enable Uart's Clocks
	switch(baudrate)
	{
	case 9600:
		baud_divisor = 0x681;
		break;
	case 19200:
		baud_divisor = 0x340;
		break;
	case 38400:
		baud_divisor = 0x19F;
		break;
	case 57600:
		baud_divisor = 0x114;
		break;
	case 115200:
		baud_divisor = 0x89;
		break;
	case 128000:
		baud_divisor = 0x7C;
		break;
	case 256000:
		baud_divisor = 0x3D;
		break;
	case 460800:
		baud_divisor = 0x21;
		break;
	case 512000:
		baud_divisor = 0x1E;
		break;
	case 921600:
		baud_divisor = 0x10;
		break;
	case 1000000:
		baud_divisor = 0x0F;
		break;
	case 1500000:
		baud_divisor = 0x09;
		break;
	case 2000000:
		baud_divisor = 0x07;
		break;
	//case 3250000:
		//baud_divisor = 0x04;
		//break;
	default:
		ret = 1;
		baud_divisor = 0x89;
		break;
	}
	REG_APB3_UART2_CFG  = (baud_divisor<<8) +
	                     (stop_bits   <<7) +
	                     (data_len    <<3) +
	                     (irda_mode   <<2) +
	                     (uart_rx_en  <<1) +
	                     uart_tx_en ; 

	REG_APB3_UART2_FIFO_CFG = (1<<BIT_TX_FIFO_THRESHOLD)|(16<<BIT_RX_FIFO_THRESHOLD)|(0x1 << BIT_STOP_DETECT_TIME);//(0x3 << BIT_STOP_DETECT_TIME);
	REG_APB3_UART2_INT_ENABLE = ((0x01 << 1) | (0x01 << 6) | (0x01 << 7));
	REG_APB3_UART2_FLOW_CFG  = 0x00000000 ;  // No Flow Control
	REG_APB3_UART2_WAKE_CFG  = ((0x01 << 0 )| (0x01 << 20) |  (0x01 << 21)| (0x01 << 22));  // No Wake Control

	REG_APB5_GPIOB_CFG  &= ~((0xC0<<BIT_GPIO_PULL_UP)  + (0xC0<<BIT_GPIO_PERI_EN));
	REG_APB5_GPIOB_CFG  |= ((0xC0<<BIT_GPIO_PULL_UP));
	REG_APB5_GPIOB_CFG  |=   (0xC0<<BIT_GPIO_OUT_EN_N);

	REG_APB5_GPIOB_DATA &= ~ (0xC0<<BIT_GPIO_INPUT_EN);


	uart2_env.rx.callback = NULL;
	uart2_env.rx.dummy    = NULL;

	REG_AHB0_ICU_INT_ENABLE |= (0x01 << 16);
	return ret;
}



void uart_flow_on(void)
{
	// Configure modem (HW flow control enable)
	// uart_flow_en_setf(0);
}

void uart_clear_rxfifo(void)
{

	while(uart_rx_fifo_rd_ready_getf())
	{
		Uart_Read_Byte();
	}
	memset((void *)uart_rx_buf,0,UART0_RX_FIFO_MAX_COUNT);
    uart_rx_index = 0;
    cur_read_buf_idx = 0;

}
bool uart_flow_off(void)
{

	return true;

}

void uart_finish_transfers(void)
{
    uart_flow_en_setf(1);

    // Wait TX FIFO empty
    while(!uart_tx_fifo_empty_getf());
}


void uart_read(uint8_t *bufptr, uint32_t size, void(*callback) (void*, uint8_t), void* dummy)
{
	// Sanity check
	ASSERT_ERR(bufptr != NULL);
	ASSERT_ERR(size != 0);
	ASSERT_ERR(callback != NULL);
	uart_env.rx.callback = callback;

    uart_env.rx.dummy    = dummy;
		ph4tl_env = dummy;
}



void uart_write(uint8_t *bufptr, uint32_t size, void (*callback) (void*, uint8_t), void* dummy)
{
	// Sanity check
	ASSERT_ERR(bufptr != NULL);
	ASSERT_ERR(size != 0);
	ASSERT_ERR(callback != NULL);
	
	uint8_t len;
	len = size;
		
	uart_env.tx.callback = callback;
	uart_env.tx.dummy    = dummy;
	ph4tl_env = dummy;
	//Delay_ms(100);
	while(len)
	{
		//cpu_delay(10);
		if(UART_TX_WRITE_READY)
		{
			REG_APB3_UART_PORT = *bufptr;
            bufptr++;
			len--;
        }
	}
		
	if(callback != NULL)
	{
		// Clear callback pointer
		uart_env.tx.callback = NULL;
		uart_env.tx.dummy    = NULL;
		// Call handler
		callback(dummy, RWIP_EIF_STATUS_OK);
	}
}

static void uart_send_byte(unsigned char data)
{
	while (!uart_tx_fifo_empty_getf());
    
	REG_APB3_UART_PORT = data ;
}

static void uart2_send_byte(unsigned char data)
{
	while (!uart2_tx_fifo_empty_getf());

	REG_APB3_UART2_PORT = data ;
}

void uart_send(unsigned char *buff, int len)
{
    while (len--)
        uart_send_byte(*buff++);
}

void uart2_send(unsigned char *buff, int len)
{
	while (len--)
		uart2_send_byte(*buff++);
}

void uart_cb_register(UART_RX_CALLBACK_T cb)
{
	if(cb)
	{
		uart_rx_cb = cb;
	}
}

void uart2_cb_register(UART_RX_CALLBACK_T cb)
{
	if(cb)
	{
		usrt2_rx_cb = cb;
	}
}

void uart_cb_clear(void)
{
	if(uart_rx_cb) 
	{
		uart_rx_cb = NULL;
	}
}

void uart2_cb_clear(void)
{
	if(usrt2_rx_cb)
	{
		usrt2_rx_cb = NULL;
	}
}

volatile uint8_t uart_dut_reg_flag = 0;
extern void pn9_test_process(uint8_t* buffer, uint8_t len);

enum
{
    NORMAL_MODE = 0,
    DUT_MODE = 1,
    REG_MODE = 2,
};

void uart_isr(void)
{	
	uint32_t IntStat;
	 
	IntStat = uart_isr_stat_get();	
	if(uart_rx_fifo_need_rd_isr_getf() || uart_rx_end_isr_getf()|| uart_rxd_wakeup_isr_getf())
	{
		while((REG_APB3_UART_FIFO_STAT & (0x01 << 21)))
		{
			uint8_t ch = UART_READ_BYTE();
            
//			#if CLI_CONSOLE
//				store_uart_ringbuf_data(&ch,1);
//			#endif//
//			#if USER_PROFILE
				uart_rx_buf[uart_rx_index] = ch;			
//			#endif//
			uart_rx_index++;			
			if( UART0_RX_FIFO_MAX_COUNT == uart_rx_index ) 
			{
				uart_rx_index = 0;
			}			 			 
		}
		
//		uart_rx_done = 1;
//		#if CLI_CONSOLE
//			ke_event_set(KE_EVENT_AOS_CLI);
//		#endif// 
	}
    
#if 1
	if((system_mode & RW_DUT_MODE) == RW_DUT_MODE)
	{
		void (*callback) (void*, uint8_t) = NULL;
		void* data =NULL;
		if( uart_rx_end_isr_getf())
		{
			if((uart_rx_buf[0] == 0x01) &&(uart_rx_buf[1] == 0xe0)&& (uart_rx_buf[2] == 0xfc))
			{
				uart_dut_reg_flag = REG_MODE;
			}
			else if((uart_rx_buf[0] == 0x01) && (uart_rx_index > 3))
			{
				uart_dut_reg_flag = DUT_MODE;
			}

			if(uart_dut_reg_flag != NORMAL_MODE)
			{
				uart_rx_done = 1 ;
			}
			else
			{
				uart_rx_index = 0;
			}
		}
		if(uart_dut_reg_flag == DUT_MODE)
		{
			uart_rx_index = 0;
			callback = uart_env.rx.callback;
			data = uart_env.rx.dummy;
			if(callback != NULL)
			{
				// Clear callback pointer
				uart_env.rx.callback = NULL;
				uart_env.rx.dummy    = NULL;
				callback(data, RWIP_EIF_STATUS_OK);
			}
		}
		uart_dut_reg_flag = NORMAL_MODE;
	}
	else if((system_mode & RW_FCC_MODE) == RW_FCC_MODE)
	{
		uart_rx_done = 1;
	}
	else if((system_mode & RW_PN9_MODE) == RW_PN9_MODE)
	{
		uart_rx_done = 1;
		pn9_test_process(uart_rx_buf,uart_rx_index);
		uart_rx_index = 0;
	}
	else
	{
        if(uart_rx_end_isr_getf())
        {
            uart_rx_done = 1;
            if(uart_rx_cb)
            {
                (*uart_rx_cb)(uart_rx_buf, uart_rx_index);
            }
            uart_rx_index = 0;
        }
	}
    
#endif // 
	uart_isr_stat_set(IntStat);
}

void uart2_isr(void)
{
	uint32_t IntStat;

	IntStat = uart2_isr_stat_get();
	if(uart2_rx_fifo_need_rd_isr_getf() || uart2_rx_end_isr_getf()|| uart2_rxd_wakeup_isr_getf())
	{
		while((REG_APB3_UART2_FIFO_STAT & (0x01 << 21)))
		{
			uart2_rx_buf[uart2_rx_index++] = UART2_READ_BYTE();
			if( UART0_RX_FIFO_MAX_COUNT == uart2_rx_index )
			{
				uart2_rx_index = 0;
			}
		}
	}
    
	{
        if(uart2_rx_end_isr_getf())
        {
            uart2_rx_done = 1;
            if(usrt2_rx_cb)
            {
                (*usrt2_rx_cb)(uart2_rx_buf, uart2_rx_index);
            }
            uart2_rx_index = 0;
        }
	}
	uart2_isr_stat_set(IntStat);
}

uint8_t check_uart_stop(void)
{
	return uart_tx_fifo_empty_getf();
}


int fputc(int ch,FILE *f)
{
	uart_send_byte(ch);
	
	return ch;
}

/// @} UART
