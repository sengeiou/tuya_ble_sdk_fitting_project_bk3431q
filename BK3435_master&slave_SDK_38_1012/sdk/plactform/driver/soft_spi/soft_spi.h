#ifndef __SOFT_SPI_H__
#define __SOFT_SPI_H__

#include <stdbool.h>          // standard boolean definitions
#include <stdint.h>           // standard integer functions
#include "BK3435_reg.h"


#define SOFT_SPI_SCK              0x04
#define SOFT_SPI_MOSI             0x05
#define SOFT_SPI_MISO             0x06///0x07///
#define SOFT_SPI_CS               0x07///0x06///

#if 0
#define SPI_CS_ENABLE             REG_APB5_GPIOA_DATA &= 0xFFFFFF7F, REG_APB5_GPIOA_CFG &= 0xFFFF7FFF//gpio_set(SOFT_SPI_CS,0)
#define SPI_CS_DISABLE            REG_APB5_GPIOA_DATA |= 0x80, REG_APB5_GPIOA_CFG |= 0x8000//gpio_set(SOFT_SPI_CS,1)

#define SPI_SCK_LOW               REG_APB5_GPIOA_DATA &= 0xFFFFFFEF, REG_APB5_GPIOA_CFG &= 0xFFFFEFFF//gpio_set(SOFT_SPI_SCK,0)
#define SPI_SCK_HIGH              REG_APB5_GPIOA_DATA |= 0x10, REG_APB5_GPIOA_CFG |= 0x1000//gpio_set(SOFT_SPI_SCK,1)

#define SPI_MISO_READ             REG_APB5_GPIOA_DATA & 0x4000///((REG_APB5_GPIOA_DATA>>8)&0x40)>>6//gpio_get_input(SOFT_SPI_MISO)

#define SPI_MOSI_LOW              REG_APB5_GPIOA_DATA &= 0xFFFFFFDF, REG_APB5_GPIOA_CFG &= 0xFFFFDFFF//gpio_set(SOFT_SPI_MOSI,0)
#define SPI_MOSI_HIGH             REG_APB5_GPIOA_DATA |= 0x20, REG_APB5_GPIOA_CFG |= 0x2000//gpio_set(SOFT_SPI_MOSI,1)
#else
#define SPI_CS_ENABLE             REG_APB5_GPIOA_DATA &= 0xFFFFFF7F, REG_APB5_GPIOA_CFG &= 0xFFFF7FFF//gpio_set(SOFT_SPI_CS,0)
#define SPI_CS_DISABLE            REG_APB5_GPIOA_DATA |= 0x80, REG_APB5_GPIOA_CFG |= 0x8000//gpio_set(SOFT_SPI_CS,1)

#define SPI_SCK_LOW               REG_APB5_GPIOA_DATA &= 0xFFFFFFEF//gpio_set(SOFT_SPI_SCK,0)
#define SPI_SCK_HIGH              REG_APB5_GPIOA_DATA |= 0x10//gpio_set(SOFT_SPI_SCK,1)

#define SPI_MISO_READ             ((REG_APB5_GPIOA_DATA & 0x4000) >> (8 + 6))///((REG_APB5_GPIOA_DATA>>8)&0x40)>>6//gpio_get_input(SOFT_SPI_MISO)

#define SPI_MOSI_LOW              REG_APB5_GPIOA_DATA &= 0xFFFFFFDF//gpio_set(SOFT_SPI_MOSI,0)
#define SPI_MOSI_HIGH             REG_APB5_GPIOA_DATA |= 0x20//gpio_set(SOFT_SPI_MOSI,1)
#endif

void SPI_Initializes(void);
void SPI_WriteByte(uint8_t TxData);
uint8_t SPI_ReadByte(void);
void spi_test1(uint8_t *data, uint16_t len);
#endif
