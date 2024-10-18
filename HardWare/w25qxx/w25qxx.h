#ifndef __W25Qxx_H
#define __W25Qxx_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* W25QXX Configuration */
extern SPI_HandleTypeDef w25qxx_hspi;
#define W25QXX_SPI_PORT 1 // 1 spi1 2 spi2 3 spi3

#ifdef W25QXX_SPI_PORT
#if (1 == W25QXX_SPI_PORT)
#define W25QXX_SPI_CLK_ENABLE() __HAL_RCC_SPI1_CLK_ENABLE()
#define W25QXX_SPI_INSTANCE SPI1
#elif (2 == W25QXX_SPI_PORT)
#define W25QXX_SPI_CLK_ENABLE() __HAL_RCC_SPI2_CLK_ENABLE()
#define W25QXX_SPI_INSTANCE SPI2
#elif (3 == W25QXX_SPI_PORT)
#define W25QXX_SPI_CLK_ENABLE() __HAL_RCC_SPI3_CLK_ENABLE()
#define W25QXX_SPI_INSTANCE SPI3
#endif
#endif

#define W25QXX_CS_Pin GPIO_PIN_4
#define W25QXX_CS_GPIO_Port GPIOA

/*W25QXX FLASH memory*/
#define W25QXX_FLASH_ADDRESS 0x000000

/* W25QXX Sector Size and Page Size */
#define W25QXX_PAGE_SIZE 0x100    /* 65536 pages of 256 bytes */
#define W25QXX_SECTOR_SIZE 0x1000 /* 4096 sectors of 4096 bytes */

/* W25QXX OFFSET USE FOR YMODEM*/
#define W25QXX_YMODEM_OFFSET W25QXX_FLASH_ADDRESS + W25QXX_SECTOR_SIZE

/*Wait time for different operations*/
#define W25QXX_BULK_ERASE_MAX_TIME 250000
#define W25QXX_BLOCK_ERASE_MAX_TIME 3000
#define W25QXX_SECTOR_ERASE_MAX_TIME 800
#define W25QXX_TIMEOUT_VALUE 1000

/* Identification Operations */
#define W25QXX_READ_MANUFACTURER_ID_CMD 0x90
#define W25QXX_READ_JEDEC_ID_CMD 0x9F

/* Read Operations */
#define W25QXX_READ_CMD 0x03
#define W25QXX_FAST_READ_CMD 0x0B
#define W25QXX_DUAL_OUT_FAST_READ_CMD 0x3B

/* Write Operations */
#define W25QXX_WRITE_ENABLE_CMD 0x06
#define W25QXX_WRITE_DISABLE_CMD 0x04

/* Register Operations */
#define W25QXX_READ_STATUS_REG1_CMD 0x05

#define W25QXX_WRITE_STATUS_REG1_CMD 0x01

/* Program Operations */
#define W25QXX_PAGE_PROG_CMD 0x02

/* Erase Operations */
#define W25QXX_BLOCK_ERASE_CMD 0xD8
#define W25QXX_SECTOR_ERASE_CMD 0x20
#define W25QXX_CHIP_ERASE_CMD 0xC7

/* Flag Status Register */
#define W25QXX_FSR_BUSY ((uint8_t)0x01) /*!< busy */
#define W25QXX_FSR_WREN ((uint8_t)0x02) /*!< write enable */
#define W25QXX_FSR_QE ((uint8_t)0x02)   /*!< quad enable */

#define W25QXX_Enable() HAL_GPIO_WritePin(W25QXX_CS_GPIO_Port, W25QXX_CS_Pin, GPIO_PIN_RESET)
#define W25QXX_Disable() HAL_GPIO_WritePin(W25QXX_CS_GPIO_Port, W25QXX_CS_Pin, GPIO_PIN_SET)

#define W25QXX_OK ((uint8_t)0x00)
#define W25QXX_ERROR ((uint8_t)0x01)
#define W25QXX_BUSY ((uint8_t)0x02)
#define W25QXX_TIMEOUT ((uint8_t)0x03)

uint8_t W25QXX_Init(void);
uint8_t W25QXX_WriteEnable(void);
uint8_t W25QXX_WriteDisable(void);
void W25QXX_Read_Manufacturer_ID(uint8_t *ID);
void W25QXX_Read_JEDEC_ID(uint8_t *ID);
uint8_t W25QXX_Read(uint8_t *pData, uint32_t ReadAddr, uint32_t Size);
uint8_t W25QXX_Write(uint8_t *pData, uint32_t WriteAddr, uint32_t Size);
uint8_t W25QXX_Erase_Block(uint32_t Address);
uint8_t W25QXX_Erase_Sector(uint32_t Address);

/*test*/
#define W25QXX_TEST 1
#if W25QXX_TEST
void W25QXX_test(void);
#endif

#endif
/******************how to use *******************/
/*
    W25QXX_Init();
    W25QXX_test();
*/
