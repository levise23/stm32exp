#ifndef __TIMEEXIT_H
#define __TIMEEXIT_H

void TimerExit_Init(void);
void TIM4_IRQHandler(void);
extern volatile uint32_t key_flag;

#endif
