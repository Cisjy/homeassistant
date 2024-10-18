#include "OLED_Font.h"
#include "OLED.h"

static void Init(oledHandleTypeDef *oled_handle);
static void Clear(oledHandleTypeDef *oled_handle);
static void ClearLine(oledHandleTypeDef *oled_handle, uint8_t line);
static void ClearPoint(oledHandleTypeDef *oled_handle, uint8_t line, uint8_t column);
static void ShowChar(oledHandleTypeDef *oled_handle, uint8_t Line, uint8_t Column, char Char);
static void ShowString(oledHandleTypeDef *oled_handle, uint8_t Line, uint8_t Column, char *String);
static void ShowNum(oledHandleTypeDef *oled_handle, uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
static void ShowSignedNum(oledHandleTypeDef *oled_handle, uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length);
static void ShowHexNum(oledHandleTypeDef *oled_handle, uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
static void ShowBinNum(oledHandleTypeDef *oled_handle, uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);

/**
 * @brief :oled 写命令
 * @param {oledHandleTypeDef} *oled_handle
 * @param {uint8_t} Command
 * @return {*}
 */
void OLED_WriteCommand(oledHandleTypeDef *oled_handle, uint8_t Command)
{

	oled_handle->i2c_driver_interface->Start(oled_handle->i2c_driver_interface->i2c_gpio_pin);

	oled_handle->i2c_driver_interface->SendByte(oled_handle->i2c_driver_interface->i2c_gpio_pin, OLED_SLAVE_ADDR << 1 | I2C_WRITE);

	oled_handle->i2c_driver_interface->WaitAck(oled_handle->i2c_driver_interface->i2c_gpio_pin);

	oled_handle->i2c_driver_interface->SendByte(oled_handle->i2c_driver_interface->i2c_gpio_pin, 0x00);
	oled_handle->i2c_driver_interface->WaitAck(oled_handle->i2c_driver_interface->i2c_gpio_pin);

	oled_handle->i2c_driver_interface->SendByte(oled_handle->i2c_driver_interface->i2c_gpio_pin, Command);
	oled_handle->i2c_driver_interface->WaitAck(oled_handle->i2c_driver_interface->i2c_gpio_pin);
	oled_handle->i2c_driver_interface->Stop(oled_handle->i2c_driver_interface->i2c_gpio_pin);
}

/**
 * @brief  OLED写数据
 * @param  Data 要写入的数据
 * @retval 无
 */
void OLED_WriteData(oledHandleTypeDef *oled_handle, uint8_t Data)
{

	oled_handle->i2c_driver_interface->Start(oled_handle->i2c_driver_interface->i2c_gpio_pin);

	oled_handle->i2c_driver_interface->SendByte(oled_handle->i2c_driver_interface->i2c_gpio_pin, OLED_SLAVE_ADDR << 1 | I2C_WRITE); // 从机地址
	oled_handle->i2c_driver_interface->WaitAck(oled_handle->i2c_driver_interface->i2c_gpio_pin);

	oled_handle->i2c_driver_interface->SendByte(oled_handle->i2c_driver_interface->i2c_gpio_pin, 0x40); // 写数据
	oled_handle->i2c_driver_interface->WaitAck(oled_handle->i2c_driver_interface->i2c_gpio_pin);

	oled_handle->i2c_driver_interface->SendByte(oled_handle->i2c_driver_interface->i2c_gpio_pin, Data);
	oled_handle->i2c_driver_interface->WaitAck(oled_handle->i2c_driver_interface->i2c_gpio_pin);

	oled_handle->i2c_driver_interface->Stop(oled_handle->i2c_driver_interface->i2c_gpio_pin);
}

/**
 * @brief  OLED设置光标位置
 * @param  Y 以左上角为原点，向下方向的坐标，范围：0~7
 * @param  X 以左上角为原点，向右方向的坐标，范围：0~127
 * @retval 无
 */
void OLED_SetCursor(oledHandleTypeDef *oled_handle, uint8_t Y, uint8_t X)
{
	OLED_WriteCommand(oled_handle, 0xB0 | Y);				  // 设置Y位置
	OLED_WriteCommand(oled_handle, 0x10 | ((X & 0xF0) >> 4)); // 设置X位置高4位
	OLED_WriteCommand(oled_handle, 0x00 | (X & 0x0F));		  // 设置X位置低4位
}

/**
 * @brief  OLED清屏
 * @param  无
 * @retval 无
 */
static void Clear(oledHandleTypeDef *oled_handle)
{

	uint8_t i, j;
	for (j = 0; j < 8; j++)
	{
		OLED_SetCursor(oled_handle, j, 0);
		for (i = 0; i < 128; i++)
		{
			OLED_WriteData(oled_handle, 0x00);
		}
	}
}
/**
 * @brief oled清除指定行
 * @param {oledHandleTypeDef} *oled_handle
 * @param {uint8_t} line  1~4
 * @return {*}
 */
static void ClearLine(oledHandleTypeDef *oled_handle, uint8_t line)
{
	uint8_t i, j;
	uint8_t startPage = (line - 1) * 2; // Calculate the starting page based on the line number

	for (j = 0; j < 2; j++)
	{
		OLED_SetCursor(oled_handle, startPage + j, 0); // Move to the correct page
		for (i = 0; i < 128; i++)
		{
			OLED_WriteData(oled_handle, 0x00); // Clear the entire row
		}
	}
}

/**
 * @brief oled清除指定点
 * @param {oledHandleTypeDef} *oled_handle
 * @param {uint8_t} line 1~4
 * @param {uint8_t} column 列位置，范围：1~16
 * @return {*}
 */
static void ClearPoint(oledHandleTypeDef *oled_handle, uint8_t line, uint8_t column)
{
	uint8_t page = (line - 1) * 2;
	uint8_t pos = (column - 1) * 8;
	for (uint8_t i = 0; i < 2; i++)
	{
		OLED_SetCursor(oled_handle, page + i, pos);
		for (uint8_t j = 0; j < 8; j++)
		{
			OLED_WriteData(oled_handle, 0x00);
		}
	}
}
/**
 * @brief  OLED显示一个字符
 * @param  Line 行位置，范围：1~4
 * @param  Column 列位置，范围：1~16
 * @param  Char 要显示的一个字符，范围：ASCII可见字符
 * @retval 无
 */
static void ShowChar(oledHandleTypeDef *oled_handle, uint8_t Line, uint8_t Column, char Char)
{

	uint8_t i;
	OLED_SetCursor(oled_handle, (Line - 1) * 2, (Column - 1) * 8); // 设置光标位置在上半部分
	for (i = 0; i < 8; i++)
	{
		OLED_WriteData(oled_handle, OLED_F8x16[Char - ' '][i]); // 显示上半部分内容
	}
	OLED_SetCursor(oled_handle, (Line - 1) * 2 + 1, (Column - 1) * 8); // 设置光标位置在下半部分
	for (i = 0; i < 8; i++)
	{
		OLED_WriteData(oled_handle, OLED_F8x16[Char - ' '][i + 8]); // 显示下半部分内容
	}
}

/**
 * @brief  OLED显示字符串
 * @param  Line 起始行位置，范围：1~4
 * @param  Column 起始列位置，范围：1~16
 * @param  String 要显示的字符串，范围：ASCII可见字符
 * @retval 无
 */
static void ShowString(oledHandleTypeDef *oled_handle, uint8_t Line, uint8_t Column, char *String)
{

	uint8_t i;
	for (i = 0; String[i] != '\0'; i++)
	{
		ShowChar(oled_handle, Line, Column + i, String[i]);
	}
}

/**
 * @brief  OLED次方函数
 * @retval 返回值等于X的Y次方
 */
uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;
	while (Y--)
	{
		Result *= X;
	}
	return Result;
}

/**
 * @brief  OLED显示数字（十进制，正数）
 * @param  Line 起始行位置，范围：1~4
 * @param  Column 起始列位置，范围：1~16
 * @param  Number 要显示的数字，范围：0~4294967295
 * @param  Length 要显示数字的长度，范围：1~10
 * @retval 无
 */
static void ShowNum(oledHandleTypeDef *oled_handle, uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{

	uint8_t i;
	for (i = 0; i < Length; i++)
	{
		ShowChar(oled_handle, Line, Column + i, Number / OLED_Pow(10, Length - i - 1) % 10 + '0');
	}
}

/**
 * @brief  OLED显示数字（十进制，带符号数）
 * @param  Line 起始行位置，范围：1~4
 * @param  Column 起始列位置，范围：1~16
 * @param  Number 要显示的数字，范围：-2147483648~2147483647
 * @param  Length 要显示数字的长度，范围：1~10
 * @retval 无
 */
static void ShowSignedNum(oledHandleTypeDef *oled_handle, uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length)
{

	uint8_t i;
	uint32_t Number1;
	if (Number >= 0)
	{
		ShowChar(oled_handle, Line, Column, '+');
		Number1 = Number;
	}
	else
	{
		ShowChar(oled_handle, Line, Column, '-');
		Number1 = -Number;
	}
	for (i = 0; i < Length; i++)
	{
		ShowChar(oled_handle, Line, Column + i + 1, Number1 / OLED_Pow(10, Length - i - 1) % 10 + '0');
	}
}

/**
 * @brief  OLED显示数字（十六进制，正数）
 * @param  Line 起始行位置，范围：1~4
 * @param  Column 起始列位置，范围：1~16
 * @param  Number 要显示的数字，范围：0~0xFFFFFFFF
 * @param  Length 要显示数字的长度，范围：1~8
 * @retval 无
 */
static void ShowHexNum(oledHandleTypeDef *oled_handle, uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{

	uint8_t i, SingleNumber;
	for (i = 0; i < Length; i++)
	{
		SingleNumber = Number / OLED_Pow(16, Length - i - 1) % 16;
		if (SingleNumber < 10)
		{
			ShowChar(oled_handle, Line, Column + i, SingleNumber + '0');
		}
		else
		{
			ShowChar(oled_handle, Line, Column + i, SingleNumber - 10 + 'A');
		}
	}
}

/**
 * @brief  OLED显示数字（二进制，正数）
 * @param  Line 起始行位置，范围：1~4
 * @param  Column 起始列位置，范围：1~16
 * @param  Number 要显示的数字，范围：0~1111 1111 1111 1111
 * @param  Length 要显示数字的长度，范围：1~16
 * @retval 无
 */
static void ShowBinNum(oledHandleTypeDef *oled_handle, uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{

	uint8_t i;
	for (i = 0; i < Length; i++)
	{
		ShowChar(oled_handle, Line, Column + i, Number / OLED_Pow(2, Length - i - 1) % 2 + '0');
	}
}

/**
 * @brief  OLED初始化
 * @param  无
 * @retval 无
 */
static void Init(oledHandleTypeDef *oled_handle)
{
	uint32_t i, j;

	for (i = 0; i < 1000; i++) // 上电延时
	{
		for (j = 0; j < 1000; j++)
			;
	}

	oled_handle->i2c_driver_interface->Init(oled_handle->i2c_driver_interface->i2c_gpio_pin);
	OLED_WriteCommand(oled_handle, 0xAE); // 关闭显示

	OLED_WriteCommand(oled_handle, 0xD5); // 设置显示时钟分频比/振荡器频率

	OLED_WriteCommand(oled_handle, 0x80);

	OLED_WriteCommand(oled_handle, 0xA8); // 设置多路复用率
	OLED_WriteCommand(oled_handle, 0x3F);

	OLED_WriteCommand(oled_handle, 0xD3); // 设置显示偏移
	OLED_WriteCommand(oled_handle, 0x00);

	OLED_WriteCommand(oled_handle, 0x40); // 设置显示开始行

	OLED_WriteCommand(oled_handle, 0xA1); // 设置左右方向，0xA1正常 0xA0左右反置

	OLED_WriteCommand(oled_handle, 0xC8); // 设置上下方向，0xC8正常 0xC0上下反置

	OLED_WriteCommand(oled_handle, 0xDA); // 设置COM引脚硬件配置
	OLED_WriteCommand(oled_handle, 0x12);

	OLED_WriteCommand(oled_handle, 0x81); // 设置对比度控制
	OLED_WriteCommand(oled_handle, 0xCF);

	OLED_WriteCommand(oled_handle, 0xD9); // 设置预充电周期
	OLED_WriteCommand(oled_handle, 0xF1);

	OLED_WriteCommand(oled_handle, 0xDB); // 设置VCOMH取消选择级别
	OLED_WriteCommand(oled_handle, 0x30);

	OLED_WriteCommand(oled_handle, 0xA4); // 设置整个显示打开/关闭

	OLED_WriteCommand(oled_handle, 0xA6); // 设置正常/倒转显示

	OLED_WriteCommand(oled_handle, 0x8D); // 设置充电泵
	OLED_WriteCommand(oled_handle, 0x14);

	OLED_WriteCommand(oled_handle, 0xAF); // 开启显示

	Clear(oled_handle); // OLED清屏
}

/**
 * @brief :
 * @param {oledHandleTypeDef} *oled_handle
 * @param {i2c_driver_interface_t} *i2c_driver_interface
 * @return {*}
 */
uint8_t OLED_Init(oledHandleTypeDef *oled_handle,
				  i2c_driver_interface_t *i2c_driver_interface)
{

	oled_handle->i2c_driver_interface = i2c_driver_interface;
	oled_handle->Clear = Clear;
	oled_handle->ClearLine = ClearLine;
	oled_handle->ClearPoint = ClearPoint;
	oled_handle->ShowChar = ShowChar;
	oled_handle->ShowString = ShowString;
	oled_handle->ShowNum = ShowNum;
	oled_handle->ShowSignedNum = ShowSignedNum;
	oled_handle->ShowHexNum = ShowHexNum;
	oled_handle->ShowBinNum = ShowBinNum;
	Init(oled_handle); // OLED内部初始化
	return 0;
}
