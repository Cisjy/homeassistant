#include "w25qxx.h"
SPI_HandleTypeDef w25qxx_hspi;

/**
 * @brief 获取Flash状态
 * @return {*}
 */
static uint8_t W25QXX_GetStatus(void)
{
	uint8_t cmd = W25QXX_READ_STATUS_REG1_CMD;
	uint8_t status;

	W25QXX_Enable();
	/* Send the read status command */
	HAL_SPI_Transmit(&w25qxx_hspi, &cmd, 1, W25QXX_TIMEOUT_VALUE);
	/* Reception of the data */
	HAL_SPI_Receive(&w25qxx_hspi, &status, 1, W25QXX_TIMEOUT_VALUE);
	W25QXX_Disable();

	/* Check the value of the register */
	return status;
}

static uint8_t W25QXX_Wait_Busy(void)
{
	uint32_t tickstart = HAL_GetTick();

	while ((W25QXX_GetStatus() & W25QXX_FSR_BUSY) == 1)
	{
#if SYS_SUPPORT_OS

		if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)
		{
			vTaskDelay(1);
		}
#endif
		if (HAL_GetTick() - tickstart > W25QXX_TIMEOUT_VALUE)
		{
			return W25QXX_TIMEOUT;
		}
	}
	return W25QXX_OK;
}

uint8_t W25QXX_Init(void)
{

	W25QXX_SPI_CLK_ENABLE();

	w25qxx_hspi.Instance = W25QXX_SPI_INSTANCE;
	w25qxx_hspi.Init.Mode = SPI_MODE_MASTER;
	w25qxx_hspi.Init.Direction = SPI_DIRECTION_2LINES;
	w25qxx_hspi.Init.DataSize = SPI_DATASIZE_8BIT;
	w25qxx_hspi.Init.CLKPolarity = SPI_POLARITY_LOW;
	w25qxx_hspi.Init.CLKPhase = SPI_PHASE_1EDGE;
	w25qxx_hspi.Init.NSS = SPI_NSS_SOFT;
	w25qxx_hspi.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
	w25qxx_hspi.Init.FirstBit = SPI_FIRSTBIT_MSB;
	w25qxx_hspi.Init.TIMode = SPI_TIMODE_DISABLE;
	w25qxx_hspi.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	w25qxx_hspi.Init.CRCPolynomial = 10;
	if (HAL_SPI_Init(&w25qxx_hspi) != HAL_OK)
	{
		Error_Handler();
	}

	/**SPI1 GPIO Configuration
	PA4     ------> SPI1_NSS
	PA5     ------> SPI1_SCK
	PA6     ------> SPI1_MISO
	PA7     ------> SPI1_MOSI
	*/
	__HAL_RCC_GPIOA_CLK_ENABLE();
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	HAL_GPIO_WritePin(W25QXX_CS_GPIO_Port, W25QXX_CS_Pin, GPIO_PIN_SET);

	GPIO_InitStruct.Pin = W25QXX_CS_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(W25QXX_CS_GPIO_Port, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = GPIO_PIN_6;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	return 1;
}

uint8_t W25QXX_WriteEnable(void)
{
	uint8_t cmd[1] = {W25QXX_WRITE_ENABLE_CMD};

	/*Select the FLASH: Chip Select low */
	W25QXX_Enable();
	/* Send the read ID command */
	HAL_SPI_Transmit(&w25qxx_hspi, cmd, 1, W25QXX_TIMEOUT_VALUE);
	/*Deselect the FLASH: Chip Select high */
	W25QXX_Disable();

	/* Wait the end of Flash writing */
	return W25QXX_Wait_Busy();
}

uint8_t W25QXX_WriteDisable(void)
{
	uint8_t cmd = W25QXX_WRITE_DISABLE_CMD;

	W25QXX_Enable();
	HAL_SPI_Transmit(&w25qxx_hspi, &cmd, 1, W25QXX_TIMEOUT_VALUE);
	W25QXX_Disable();
	return W25QXX_Wait_Busy();
}

/**
 * @brief : 读取制造/器件ID
 * @param {uint8_t} *ID
 * @return {*}
 */
void W25QXX_Read_Manufacturer_ID(uint8_t *ID)
{
	uint8_t cmd[4] = {W25QXX_READ_MANUFACTURER_ID_CMD, 0x00, 0x00, 0x00};

	W25QXX_Enable();
	/* Send the read ID command */
	HAL_SPI_Transmit(&w25qxx_hspi, cmd, 4, W25QXX_TIMEOUT_VALUE);
	/* Reception of the data */
	HAL_SPI_Receive(&w25qxx_hspi, ID, 2, W25QXX_TIMEOUT_VALUE);
	W25QXX_Disable();
}

/**
 * @brief :读取JEDEC的ID 一共3个字节
 * @param {uint8_t} *ID
 * @return {*}
 */
void W25QXX_Read_JEDEC_ID(uint8_t *ID)
{
	uint8_t cmd = W25QXX_READ_JEDEC_ID_CMD;
	W25QXX_Enable();
	HAL_SPI_Transmit(&w25qxx_hspi, &cmd, 1, W25QXX_TIMEOUT_VALUE);
	HAL_SPI_Receive(&w25qxx_hspi, ID, 3, W25QXX_TIMEOUT_VALUE);
	W25QXX_Disable();
}

uint8_t W25QXX_Read(uint8_t *pData, uint32_t ReadAddr, uint32_t Size)
{
	uint8_t cmd[4];

	/* Configure the command */
	cmd[0] = W25QXX_READ_CMD;
	cmd[1] = (uint8_t)(ReadAddr >> 16);
	cmd[2] = (uint8_t)(ReadAddr >> 8);
	cmd[3] = (uint8_t)(ReadAddr);

	W25QXX_Enable();
	/* Send the read ID command */
	HAL_SPI_Transmit(&w25qxx_hspi, cmd, 4, W25QXX_TIMEOUT_VALUE);
	/* Reception of the data */
	if (HAL_SPI_Receive(&w25qxx_hspi, pData, Size, W25QXX_TIMEOUT_VALUE) != HAL_OK)
	{
		return W25QXX_ERROR;
	}
	W25QXX_Disable();
	return W25QXX_OK;
}

uint8_t W25QXX_Write(uint8_t *pData, uint32_t WriteAddr, uint32_t Size)
{
	uint8_t cmd[4];
	uint32_t end_addr, current_size, current_addr;

	/* Calculation of the size between the write address and the end of the page */
	current_addr = 0;

	while (current_addr <= WriteAddr)
	{
		current_addr += W25QXX_PAGE_SIZE;
	}
	current_size = current_addr - WriteAddr;

	/* Check if the size of the data is less than the remaining place in the page */
	if (current_size > Size)
	{
		current_size = Size;
	}

	/* Initialize the adress variables */
	current_addr = WriteAddr;
	end_addr = WriteAddr + Size;

	/* Perform the write page by page */
	do
	{
		/* Configure the command */
		cmd[0] = W25QXX_PAGE_PROG_CMD;
		cmd[1] = (uint8_t)(current_addr >> 16);
		cmd[2] = (uint8_t)(current_addr >> 8);
		cmd[3] = (uint8_t)(current_addr);

		/* Enable write operations */
		W25QXX_WriteEnable();

		W25QXX_Enable();
		/* Send the command */
		if (HAL_SPI_Transmit(&w25qxx_hspi, cmd, 4, W25QXX_TIMEOUT_VALUE) != HAL_OK)
		{
			return W25QXX_ERROR;
		}

		/* Transmission of the data */
		if (HAL_SPI_Transmit(&w25qxx_hspi, pData, current_size, W25QXX_TIMEOUT_VALUE) != HAL_OK)
		{
			return W25QXX_ERROR;
		}
		W25QXX_Disable();
		/* Wait the end of Flash writing */
		if (W25QXX_Wait_Busy() != W25QXX_OK)
		{
			return W25QXX_TIMEOUT;
		}

		/* Update the address and size variables for next page programming */
		current_addr += current_size;
		pData += current_size;
		current_size = ((current_addr + W25QXX_PAGE_SIZE) > end_addr) ? (end_addr - current_addr) : W25QXX_PAGE_SIZE;
	} while (current_addr < end_addr);

	return W25QXX_OK;
}

uint8_t W25QXX_Erase_Block(uint32_t Address)
{
	uint8_t cmd[4];

	cmd[0] = W25QXX_BLOCK_ERASE_CMD;
	cmd[1] = (uint8_t)(Address >> 16);
	cmd[2] = (uint8_t)(Address >> 8);
	cmd[3] = (uint8_t)(Address);

	/* Enable write operations */
	W25QXX_WriteEnable();

	/*Select the FLASH: Chip Select low */
	W25QXX_Enable();
	/* Send the read ID command */
	HAL_SPI_Transmit(&w25qxx_hspi, cmd, 4, W25QXX_TIMEOUT_VALUE);
	/*Deselect the FLASH: Chip Select high */
	W25QXX_Disable();

	/* Wait the end of Flash writing */
	if (W25QXX_Wait_Busy() != W25QXX_OK)
	{
		return W25QXX_TIMEOUT;
	}
	return W25QXX_OK;
}
uint8_t W25QXX_Erase_Sector(uint32_t Address)
{
	uint8_t cmd[4];

	cmd[0] = W25QXX_SECTOR_ERASE_CMD;
	cmd[1] = (uint8_t)(Address >> 16);
	cmd[2] = (uint8_t)(Address >> 8);
	cmd[3] = (uint8_t)(Address);

	/* Enable write operations */
	W25QXX_WriteEnable();

	/*Select the FLASH: Chip Select low */
	W25QXX_Enable();
	/* Send the read ID command */
	HAL_SPI_Transmit(&w25qxx_hspi, cmd, 4, W25QXX_TIMEOUT_VALUE);
	/*Deselect the FLASH: Chip Select high */
	W25QXX_Disable();

	/* Wait the end of Flash writing */
	if (W25QXX_Wait_Busy() != W25QXX_OK)
	{
		return W25QXX_TIMEOUT;
	}
	return W25QXX_OK;
}
#if W25QXX_TEST
void W25QXX_test(void)
{
	uint8_t Manufacturer_ID, w25qxx_read_buf[20], i;
	uint8_t w25qxx_write_buf[20] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14};
	W25QXX_Read_Manufacturer_ID(&Manufacturer_ID);
	USART1_printf("Manufacturer ID: %d\r\n", Manufacturer_ID);
	W25QXX_Read(w25qxx_read_buf, 0x000000, 20);
	for (i = 0; i < 20; i++)
	{
		USART1_printf("0x%x ", w25qxx_read_buf[i]);
	}
	USART1_printf("\r\n");
	W25QXX_Erase_Sector(0x000000);

	W25QXX_Read(w25qxx_read_buf, 0x000000, 20);
	for (i = 0; i < 20; i++)
	{
		USART1_printf("0x%x ", w25qxx_read_buf[i]);
	}
	USART1_printf("\r\n");
	W25QXX_Write(w25qxx_write_buf, 0x000000, 20);
	W25QXX_Read(w25qxx_read_buf, 0x000000, 20);
	for (i = 0; i < 20; i++)
	{
		USART1_printf("0x%x ", w25qxx_read_buf[i]);
	}
	USART1_printf("\r\n");
}
#endif
// /**********************************************************************************
//  * ��������: оƬ����
//  */
// uint8_t BSP_W25Qx_Erase_Chip(void)
// {
// 	uint8_t cmd[4];
// 	uint32_t tickstart = HAL_GetTick();
// 	cmd[0] = CHIP_ERASE_CMD;

// 	/* Enable write operations */
// 	BSP_W25Qx_WriteEnable();

// 	/*Select the FLASH: Chip Select low */
// 	W25Qx_Enable();
// 	/* Send the read ID command */
// 	HAL_SPI_Transmit(&w25qxx_hspi, cmd, 1, W25Qx_TIMEOUT_VALUE);
// 	/*Deselect the FLASH: Chip Select high */
// 	W25Qx_Disable();

// 	/* Wait the end of Flash writing */
// 	while (BSP_W25Qx_GetStatus() != W25Qx_BUSY)
// 		;
// 	{
// 		/* Check for the Timeout */
// 		if ((HAL_GetTick() - tickstart) > W25QXX_BULK_ERASE_MAX_TIME)
// 		{
// 			return W25Qx_TIMEOUT;
// 		}
// 	}
// 	return W25Qx_OK;
// }
