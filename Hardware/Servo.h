#ifndef __SERVO_H
#define __SERVO_H

void Servo_Init(void);
void Servo_SetAngle(float Angle);
void PWMServo_Init(void);
void PWM_SetCompare4(uint16_t Compare);

#endif
