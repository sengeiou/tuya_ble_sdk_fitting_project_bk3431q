
#include "soft_spi.h"
#include "gpio.h"
#include "uart.h"
#include "rf.h"


void SPI_Delay(uint8_t num)
{
    //Delay_us(num);
}

void SPI_Initializes(void)
{
    gpio_config(SOFT_SPI_CS,OUTPUT,PULL_NONE);
    gpio_config(SOFT_SPI_SCK,OUTPUT,PULL_NONE);
    gpio_config(SOFT_SPI_MISO,INPUT,PULL_HIGH);
    gpio_config(SOFT_SPI_MOSI,OUTPUT,PULL_HIGH);
    UART_PRINTF("%08x %08x\r\n", REG_APB5_GPIOA_CFG, REG_APB5_GPIOA_DATA);
    SPI_CS_DISABLE;
    SPI_SCK_HIGH;///SPI_SCK_LOW;///
    SPI_MOSI_HIGH;///SPI_MOSI_LOW;//
}

void SPI_WriteByte(uint8_t TxData)
{
    uint8_t cnt;

  for(cnt=0; cnt<8; cnt++)
  {
    SPI_SCK_LOW;   //SPI_SCK_HIGH;//                              //时钟 - 低
    //Delay_us(1);//SPI_Delay(1);

    if(TxData & 0x80)//if(TxData & 0x01)//                            //发送数据
      SPI_MOSI_HIGH;
    else
      SPI_MOSI_LOW;
    //TxData <<= 1;//TxData >>= 1;//

    //SPI_Delay();
    SPI_SCK_HIGH;   //SPI_SCK_LOW;//                             //时钟 - 高
    //SPI_Delay(3);
    TxData <<= 1;
    //Delay_us(1);
  }
  //SPI_SCK_LOW;
  SPI_MOSI_HIGH;///SPI_MOSI_LOW;////
  //Delay_us(10);
}

uint8_t SPI_ReadByte(void)
{
    uint8_t cnt;
    uint8_t RxData = 0;

    for(cnt=0; cnt<8; cnt++)
    {
        
        SPI_SCK_LOW;//    SPI_SCK_HIGH;//                             //时钟 - 低
        //Delay_us(1);//SPI_Delay(1);

        //RxData <<= 1;
        if(SPI_MISO_READ)        //if(gpio_get_input(0x06)) //                    //读取数据
        {
            RxData |= 0x01;
        }

        SPI_SCK_HIGH;  //SPI_SCK_LOW;//                              //时钟 - 高
        if(cnt<7)RxData <<= 1;
        //Delay_us(1);//SPI_Delay(3);
    }
    //SPI_SCK_LOW;//
    //Delay_us(10);     
    //UART_PRINTF("%08x ", REG_APB5_GPIOA_DATA);
    //UART_PRINTF("%08x %08x\r\n", REG_APB5_GPIOA_CFG, REG_APB5_GPIOA_DATA);
    return RxData;    
}
//uint8_t electric_esam_get_info[] = {0xAA, 0x81, 0x40, 0x00, 0x00, 0x00, 0x24, 0xE0, 0x19, 0xB8, 0x90, 0x14, 
//                                    0x66, 0xFd, 0xAB, 0xE3, 0x00, 0xD2, 0x22, 0x4A, 0xD4, 0x0F, 0x3A, 0x26, 
//                                    0x94, 0x6E, 0x60, 0x65, 0x80, 0x6C, 0x3F, 0xBD, 0xE2, 0xCC, 0x82, 0x05, 
//                                    0x5C, 0xD0, 0xA1, 0x0C, 0xD4, 0x15, 0xCD, 0x64};
//static uint8_t electric_esam_get_info[8] = {0x55, 0x80, 0x36, 0x00, 0x02, 0x00, 0x00, 0x4B};
//static uint8_t electric_esam_get_info[8] = {0x55, 0x00, 0x36, 0x00, 0x00, 0x00, 0x00, 0xC9};
//static uint8_t electric_esam_get_info[8] = {0x55, 0x80, 0x36, 0x00, 0x04, 0x00, 0x00, 0x4D};
//uint8_t electric_esam_get_info[6] = {0x55, 0x80, 0x0E, 0x00, 0x02, 0x00, 0x00};
//static uint8_t electric_esam_get_info[8] = {0x55, 0x80, 0x0E, 0x00, 0x02, 0x00, 0x00, 0xe5};
//static uint8_t electric_esam_get_info[8] = {0x55, 0x00, 0xa0, 0x01, 0x01, 0x00, 0x00, 0x5f};//55 + 90 00 + len + data
void spi_write_buffer(uint8_t *data, uint16_t len)
{
    SPI_CS_ENABLE;
    Delay_us(65);
    for(int i = 0; i < len; i++)
    {        
        SPI_WriteByte(data[i]);        
    }
    Delay_us(65);
    SPI_CS_DISABLE;
    Delay_us(65);
}

void spi_read_data(uint8_t *data, uint16_t len)
{
    //uint8_t rsp_data[10];
    SPI_CS_ENABLE;
    Delay_us(65);
    for(int i = 0; i < len; i++)
    {        
        *data = SPI_ReadByte();
        data++;        
    }
    Delay_us(65);
    SPI_CS_DISABLE;
    Delay_us(65);
}

void spi_read_rsp(uint8_t *data, uint16_t *len)
{
    uint8_t rsp_data = 0;
    uint16_t read_len = 0;
    uint32_t timeout_cnt = 0;
    
    SPI_CS_ENABLE;
    Delay_us(65);
    while(rsp_data != 0x55)
    {
        Delay_us(8);
        rsp_data = SPI_ReadByte();//spi_read_data(rsp_data, 1);
        //UART_PRINTF("%02x ", rsp_data);         
        timeout_cnt++;
        if(timeout_cnt > 133333)return;
    }
    //UART_PRINTF("rsp_data %02x\r\n", rsp_data);

    *data = SPI_ReadByte();
    data++;

    *data = SPI_ReadByte();
    data++;
    read_len = *data = SPI_ReadByte();
    data++;
    read_len = (read_len<<8);
    read_len += *data = SPI_ReadByte();
    data++;

    for(int i = 0; i < read_len + 1; i++)
    {
        *data = SPI_ReadByte();
        data++;
    }
    *len = read_len + 5;
    Delay_us(65);
    SPI_CS_DISABLE;
    Delay_us(65);
}

void spi_test1(uint8_t *data, uint16_t len)
{
    uint8_t rsp_data[255];
    uint16_t read_len = 0;
    uint32_t timeout_cnt = 0;
    
//    SPI_Initializes();
    UART_PRINTF("soft spi test\r\n");
    spi_write_buffer(data, len);
    //Delay_ms(5);
    spi_read_rsp(rsp_data, &read_len);

    UART_PRINTF("soft spi rsp data:\r\n");
    for(int i = 0; i < read_len/*sizeof(electric_esam_get_info)*/; i++)
    {
        UART_PRINTF("%02x ", rsp_data[i]);
    }
    UART_PRINTF("\r\n");
}
