#include "stm32f10x.h"
#include "OLED_Font.h"

/*引脚配置*/
#define OLED_W_SCL(x)		GPIO_WriteBit(GPIOB, GPIO_Pin_10, (BitAction)(x))
#define OLED_W_SDA(x)		GPIO_WriteBit(GPIOB, GPIO_Pin_11, (BitAction)(x))

/*引脚初始化*/
void OLED_I2C_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 ;
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	OLED_W_SCL(1);
	OLED_W_SDA(1);
}

/**
  * @brief  I2C开始
  * @param  无
  * @retval 无
  */
void OLED_I2C_Start(void)
{
	OLED_W_SDA(1);
	OLED_W_SCL(1);
	OLED_W_SDA(0);
	OLED_W_SCL(0);
}

/**
  * @brief  I2C停止
  * @param  无
  * @retval 无
  */
void OLED_I2C_Stop(void)
{
	OLED_W_SDA(0);
	OLED_W_SCL(1);
	OLED_W_SDA(1);
}

/**
  * @brief  I2C发送一个字节
  * @param  Byte 要发送的一个字节
  * @retval 无
  */
void OLED_I2C_SendByte(uint8_t Byte)
{
	uint8_t i;
	for (i = 0; i < 8; i++)
	{
		OLED_W_SDA(Byte & (0x80 >> i));
		OLED_W_SCL(1);
		OLED_W_SCL(0);
	}
	OLED_W_SCL(1);	//额外的一个时钟，不处理应答信号
	OLED_W_SCL(0);
}

/**
  * @brief  OLED写命令
  * @param  Command 要写入的命令
  * @retval 无
  */
void OLED_WriteCommand(uint8_t Command)
{
	OLED_I2C_Start();
	OLED_I2C_SendByte(0x78);		//从机地址
	OLED_I2C_SendByte(0x00);		//写命令
	OLED_I2C_SendByte(Command); 
	OLED_I2C_Stop();
}

/**
  * @brief  OLED写数据
  * @param  Data 要写入的数据
  * @retval 无
  */
void OLED_WriteData(uint8_t Data)
{
	OLED_I2C_Start();
	OLED_I2C_SendByte(0x78);		//从机地址
	OLED_I2C_SendByte(0x40);		//写数据
	OLED_I2C_SendByte(Data);
	OLED_I2C_Stop();
}

/**
  * @brief  OLED设置光标位置
  * @param  Y 以左上角为原点，向下方向的坐标，范围：0~7
  * @param  X 以左上角为原点，向右方向的坐标，范围：0~127
  * @retval 无
  */
void OLED_SetCursor(uint8_t Y, uint8_t X)
{
	OLED_WriteCommand(0xB0 | Y);					//设置Y位置
	OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));	//设置X位置低4位
	OLED_WriteCommand(0x00 | (X & 0x0F));			//设置X位置高4位
}

/**
  * @brief  OLED清屏
  * @param  无
  * @retval 无
  */
void OLED_Clear(void)
{  
	uint8_t i, j;
	for (j = 0; j < 8; j++)
	{
		OLED_SetCursor(j, 0);
		for(i = 0; i < 128; i++)
		{
			OLED_WriteData(0x00);
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
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char)
{      	
	uint8_t i;
	OLED_SetCursor(Line, Column );		//设置光标位置在上半部分
	for (i = 0; i < 8; i++)
	{
		OLED_WriteData(OLED_F8x16[Char - ' '][i]);			//显示上半部分内容
	}
	OLED_SetCursor(Line + 1, Column );	//设置光标位置在下半部分
	for (i = 0; i < 8; i++)
	{
		OLED_WriteData(OLED_F8x16[Char - ' '][i + 8]);		//显示下半部分内容
	}
}

/**
  * @brief  OLED显示字符串
  * @param  Line 起始行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~16
  * @param  String 要显示的字符串，范围：ASCII可见字符
  * @retval 无
  */
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i++)
	{
		OLED_ShowChar(Line, Column + i, String[i]);
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
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i++)							
	{
		OLED_ShowChar(Line, Column + i, Number / OLED_Pow(10, Length - i - 1) % 10 + '0');
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
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length)
{
	uint8_t i;
	uint32_t Number1;
	if (Number >= 0)
	{
		OLED_ShowChar(Line, Column, '+');
		Number1 = Number;
	}
	else
	{
		OLED_ShowChar(Line, Column, '-');
		Number1 = -Number;
	}
	for (i = 0; i < Length; i++)							
	{
		OLED_ShowChar(Line, Column + i + 1, Number1 / OLED_Pow(10, Length - i - 1) % 10 + '0');
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
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i, SingleNumber;
	for (i = 0; i < Length; i++)							
	{
		SingleNumber = Number / OLED_Pow(16, Length - i - 1) % 16;
		if (SingleNumber < 10)
		{
			OLED_ShowChar(Line, Column + i, SingleNumber + '0');
		}
		else
		{
			OLED_ShowChar(Line, Column + i, SingleNumber - 10 + 'A');
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
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i++)							
	{
		OLED_ShowChar(Line, Column + i, Number / OLED_Pow(2, Length - i - 1) % 2 + '0');
	}
}

/**
  * @brief  OLED初始化
  * @param  无
  * @retval 无
  */
void OLED_Init(void)
{
	uint32_t i, j;
	
	for (i = 0; i < 1000; i++)			//上电延时
	{
		for (j = 0; j < 1000; j++);
	}
	
	OLED_I2C_Init();			//端口初始化
	
	OLED_WriteCommand(0xAE);	//关闭显示
	
	OLED_WriteCommand(0xD5);	//设置显示时钟分频比/振荡器频率
	OLED_WriteCommand(0x80);
	
	OLED_WriteCommand(0xA8);	//设置多路复用率
	OLED_WriteCommand(0x3F);
	
	OLED_WriteCommand(0xD3);	//设置显示偏移
	OLED_WriteCommand(0x00);
	
	OLED_WriteCommand(0x40);	//设置显示开始行
	
	OLED_WriteCommand(0xA1);	//设置左右方向，0xA1正常 0xA0左右反置
	
	OLED_WriteCommand(0xC8);	//设置上下方向，0xC8正常 0xC0上下反置

	OLED_WriteCommand(0xDA);	//设置COM引脚硬件配置
	OLED_WriteCommand(0x12);
	
	OLED_WriteCommand(0x81);	//设置对比度控制
	OLED_WriteCommand(0xCF);

	OLED_WriteCommand(0xD9);	//设置预充电周期
	OLED_WriteCommand(0xF1);

	OLED_WriteCommand(0xDB);	//设置VCOMH取消选择级别
	OLED_WriteCommand(0x30);

	OLED_WriteCommand(0xA4);	//设置整个显示打开/关闭

	OLED_WriteCommand(0xA6);	//设置正常/倒转显示

	OLED_WriteCommand(0x8D);	//设置充电泵
	OLED_WriteCommand(0x14);

	OLED_WriteCommand(0xAF);	//开启显示
		
	OLED_Clear();				//OLED清屏
}
/**
  * @brief  OLED显示一个汉字
  * @param  Line 行位置，范围：1~4
  * @param  Column 列位置，范围：1~16
  * @param  Char 要显示的一个字符，范围：树0，云1
  * @retval 无
  */
void OLED_ShowCHI(uint8_t Line, uint8_t Column, uint8_t chi)
{      	
	uint8_t i;
	OLED_SetCursor(Line , Column );		//设置光标位置在上半部分
	for (i = 0; i < 16; i++)
	{
		OLED_WriteData(OLED_F8x16CHI[chi][i]);			//显示上半部分内容
	}
	OLED_SetCursor(Line + 1, Column );	//设置光标位置在下半部分
	for (i = 0; i < 16; i++)
	{
		OLED_WriteData(OLED_F8x16CHI[chi][i + 16]);		//显示下半部分内容
	}
}

#define SCREEN_WIDTH    128
#define CLOUD_WIDTH     16
#define CLOUD_HEIGHT    16  // 使用2页（16像素高度）
//cloud
void OLED_ShowCloud(void) {
    static int16_t pos = SCREEN_WIDTH;  // 初始位置在屏幕右侧外
    const int8_t move_speed = 1;        // 每次移动3像素
    
    // 云朵位图（16x16像素）
    const uint8_t cloud_bmp[] = {
        // 上半部分（Page 2）
        0x40,0x40,0x42,0x42,0x42,0x42,0xC2,0x42,0x42,0x42,0x42,0x42,0x42,0x40,0x40,0x00,
	0x00,0x20,0x70,0x28,0x24,0x23,0x20,0x20,0x20,0x24,0x28,0x30,0xE0,0x00,0x00,0x00,
    };

    // 1. 更新位置
    pos -= move_speed;
    
    // 2. 重置判断：当云朵完全移出屏幕左侧
    if(pos + CLOUD_WIDTH <= 0) {
        pos = SCREEN_WIDTH;  // 重置到右侧
    }

    // 3. 计算有效绘制区域
    int16_t draw_start = (pos < 0) ? -pos : 0;       // 起始列
    int16_t draw_end = (pos + CLOUD_WIDTH > SCREEN_WIDTH) ? 
                      (SCREEN_WIDTH - pos) : CLOUD_WIDTH; // 结束列

    // 4. 分页绘制（Page 0 & 1）
    for(uint8_t page = 0; page < 2; page++) {
        OLED_SetCursor(page, (pos < 0) ? 0 : pos);  // 设置起始列
        
        for(int16_t col = draw_start; col < draw_end; col++) {
            // 计算位图数据索引
            uint8_t data_index = col;
            
            // 上半页数据：0-15字节，下半页数据：16-31字节
            uint8_t data = cloud_bmp[page == 1 ? (16 + data_index) : data_index];
            
            OLED_WriteData(data);
        }
    }
}
void OLED_ShowTree(void) {
    static int16_t pos = SCREEN_WIDTH;  // 初始位置在屏幕右侧外
    const int8_t move_speed = 1;        // 每次移动3像素
    
    // 云朵位图（16x16像素）
    const uint8_t tree_bmp[] = {
        // 上半部分（Page 2）
        0x10,0x90,0xFF,0x90,0x10,0x24,0xC4,0x04,0xFC,0x00,0x90,0x10,0x10,0xFF,0x10,0x00,
	0x06,0x01,0xFF,0x00,0x21,0x10,0x0C,0x03,0x1C,0x00,0x00,0x43,0x80,0x7F,0x00,0x00
    };

    // 1. 更新位置
    pos -= move_speed;
    
    // 2. 重置判断：当云朵完全移出屏幕左侧
    if(pos + CLOUD_WIDTH <= 0) {
        pos = SCREEN_WIDTH;  // 重置到右侧
    }

    // 3. 计算有效绘制区域
    int16_t draw_start = (pos < 0) ? -pos : 0;       // 起始列
    int16_t draw_end = (pos + CLOUD_WIDTH > SCREEN_WIDTH) ? 
                      (SCREEN_WIDTH - pos) : CLOUD_WIDTH; // 结束列

    // 4. 分页绘制（Page 0 & 1）
    for(uint8_t page = 5; page < 7; page++) {
        OLED_SetCursor(page, (pos < 0) ? 0 : pos);  // 设置起始列
        
        for(int16_t col = draw_start; col < draw_end; col++) {
            // 计算位图数据索引
            uint8_t data_index = col;
            
            // 上半页数据：0-15字节，下半页数据：16-31字节
            uint8_t data = tree_bmp[page == 4 ? (16 + data_index) : data_index];
            
            OLED_WriteData(data);
        }
    }
}
/**
  * @brief  OLED显示地面线
  * @param  Line 行位置，范围：1~4
  * @param  Column 列位置，范围：1~16
  * @retval 无
  */
void OLED_ShowGround(uint8_t Line)
{      	
	static unsigned int pos=0;
	unsigned char step=5;
	uint8_t i;
	OLED_SetCursor(7,0);		//设置光标位置在上半部分
	for (i = 0; i < 128; i++)
	{
		OLED_WriteData(0x80);			//显示上半部分内容
	}
	
}
