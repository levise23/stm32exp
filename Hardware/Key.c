#include "stm32f10x.h"                  // Device header
#include "Delay.h"

void Key_Init(void)
{
	   RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource10);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource11);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource12);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11| GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	 EXTI_InitTypeDef EXTI_InitStruct = {
        .EXTI_Line = EXTI_Line10|EXTI_Line11|EXTI_Line12,
        .EXTI_Mode = EXTI_Mode_Interrupt,
        .EXTI_Trigger = EXTI_Trigger_Falling, 
        .EXTI_LineCmd = ENABLE
    };
    EXTI_Init(&EXTI_InitStruct);

    // ??NVIC
    NVIC_InitTypeDef NVIC_InitStruct = {
        .NVIC_IRQChannel = EXTI15_10_IRQn,
        .NVIC_IRQChannelPreemptionPriority = 1,
        .NVIC_IRQChannelSubPriority = 1,
				.NVIC_IRQChannelCmd = ENABLE
    };
		NVIC_Init(&NVIC_InitStruct);
}

uint8_t Key_GetNum(void)
{
	uint8_t KeyNum = 0;
	if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_3) == 0)
	{
		Delay_ms(20);
		while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_3) == 0);
		Delay_ms(20);
		KeyNum = 1;
	}
	if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4) == 0)
	{
		Delay_ms(20);
		while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4) == 0);
		Delay_ms(20);
		KeyNum = 2;
	}
	if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5) == 0)
	{
		Delay_ms(20);
		while (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5) == 0);
		Delay_ms(20);
		KeyNum = 3;
	}
	return KeyNum;
}
//volatile uint8_t key_flag = 0;
//void EXTI15_10_IRQHandler(void) {
//    if (EXTI_GetITStatus(EXTI_Line10) != RESET) {
//        key_flag = 1; 
//        EXTI_ClearITPendingBit(EXTI_Line10); 
//    }
//		if (EXTI_GetITStatus(EXTI_Line11) != RESET) {
//        key_flag = 2; 
//        EXTI_ClearITPendingBit(EXTI_Line11); 
//    }
//		if (EXTI_GetITStatus(EXTI_Line12) != RESET) {
//        key_flag = 3; 
//        EXTI_ClearITPendingBit(EXTI_Line12); 
//    }
////		EXTI_ClearITPendingBit(EXTI_Line13); 
//		EXTI_ClearITPendingBit(EXTI_Line14); 
//		EXTI_ClearITPendingBit(EXTI_Line15); Donot write to clear UNuse bit

