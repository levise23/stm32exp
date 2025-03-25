#include "stm32f10x.h"
#include "Delay.h"
#include "OLED.h"
#include "LED.h"
#include "Timer.h"
#include "PWM.h"
#include "Key.h"
#include "Servo.h"
#include "TIMEEXIT.h"
#include <stdlib.h>
#define KEY_NUM         3  
int16_t Ang=90;
uint8_t Flag=0;
volatile int32_t Speed = 00;
volatile uint8_t key_state[KEY_NUM] = {0}; 
volatile uint8_t key_event[KEY_NUM] = {0}; 
volatile uint8_t key_debounce[KEY_NUM] = {0}; 
 

void TIM4_IRQHandler(void) {
    static uint16_t key_pins[KEY_NUM] = {GPIO_Pin_10, GPIO_Pin_11, GPIO_Pin_12};
    
    if (TIM_GetITStatus(TIM4, TIM_IT_Update)) 
			{
				TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
        for(uint8_t i=0; i<KEY_NUM; i++) 
				{
            uint8_t curr_state = !GPIO_ReadInputDataBit(GPIOA, key_pins[i]);
            
					if (curr_state != key_state[i]) 
						{
                if (++key_debounce[i] >= 4) 
									{  
                    key_state[i] = 0;  
                    key_event[i] = 1;  
                    key_debounce[i] = 0; 
									}
            } 
					else 
						{
                key_debounce[i] = 0;
            }
        }
        
			}

}
void OLED_Display_Init(void) 
{ 
		OLED_Clear();
    OLED_ShowCHI(3,10,01);
		OLED_ShowGround(4,1);
}
void OLED_Display_tree(void) 
{ 
	for(int i=128;i>0;i--){
    OLED_ShowCHI(3,10,01);
		//OLED_ShowGround;
		
	}
}
void OLED_Display_Signed(int32_t Uk,int32_t Ek,int16_t Vel,int32_t Ek_sum,uint8_t Flag,int16_t Speed,uint8_t Ang) 
{
			OLED_ShowSignedNum(1, 3,Uk, 4);

}
void ProcessKey(void) {
    for (uint8_t i = 0; i < KEY_NUM; i++) {
        if (key_event[i]==1) {
            key_event[i] = 0;  
            
            switch (i) {
            case 0:  // KEY1(PA10)
                Speed += 10;
								Ang+= 10;
                if (Speed == 10) {
                    LED1_Turn();
                }
                break;
                
            case 1:  // KEY2(PA11)
                Speed -= 10;
								Ang-= 10;
                if (Speed == -10) {
                    LED2_Turn();
                }
                break;
                
            case 2:  // KEY3(PA12)
                Speed = 0;
                break;
            }
        }
    }

//	if(key_flag==1){
//		Delay_ms(100);
//		while(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_10) == 0){};
//			Ang += 10;}
//	if(key_flag==2){Ang -= 10;}
//	if(key_flag==3){Ang = 90;}
//	key_flag=0;
//	
//	
};
int main(void) {
    OLED_Init();
    LED_Init();
    OLED_Display_Init();
	  TimerExit_Init();
	int16_t i=127;
    while (1) {
			//KeyNum = Key_GetNum();
				ProcessKey();
			 OLED_ShowCloud();
				OLED_ShowTree();
			//OLED_ShowCHI(5,i--,0);
			if (i==0)i=127-16;
				//OLED_Display_Signed(Uk,Ek,Vel,Ek_sum,Flag,Speed,Ang);
				Delay_ms(20);


		

	}
}
