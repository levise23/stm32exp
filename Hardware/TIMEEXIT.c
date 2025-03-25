#include "stm32f10x.h"                  // Device header

void TimerExit_Init(void)
{

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	TIM_InternalClockConfig(TIM4);
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;
	TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStructure.TIM_Period =  100- 1;
	TIM_TimeBaseInitStructure.TIM_Prescaler = 7200 - 1;
	TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseInitStructure);
	
	TIM_ClearFlag(TIM4, TIM_FLAG_Update);
	TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_Cmd(TIM4, ENABLE);
}
uint32_t i=0;
volatile uint8_t key_flag = 0;
void TIM4_IRQHandler(void)
{

	if (TIM_GetITStatus(TIM4, TIM_IT_Update) == SET)
	{
		
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
		
		if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_10) == 0) {
        i += 1; 
			if(i==3){key_flag=1;i=0;}
			
    }
		if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_11) == 0) {
			 i += 10; 
			if(i==30){key_flag=2;i=0;}
    }
		if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_12) == 0) {
        i += 100; 
			if(i==300){key_flag=3;i=0;}
    }
	}
}
