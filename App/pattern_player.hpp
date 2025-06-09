#ifndef PATTERN_PLAYER_HPP_
#define PATTERN_PLAYER_HPP_

#include "ADAFRUIT_DRV2605.hpp"
#include "tim.h"

typedef struct {
    Adafruit_DRV2605* driver;
    I2C_HandleTypeDef *hi2c; // ✅ Yeni satır
    int (*pattern)[2];
    int patternLength;
    int millis_counter;
    int pair_index;
    bool isDisabled;
    int currentDuty;
        int targetDuty;
        bool ramping;
} MotorControl;

typedef enum {
    MOTOR_1,
    MOTOR_2
} Motor_t;

//int GetDelayMs(int x, int patternArray[][2], int patternLength);
//void SetTIM3DutyCycle(int dutyCycle);
//int CalculateDuration(int patternArray[][2], int patternLength);
extern bool pendingUpdate;
extern int next_duty_motor1;
extern int next_duty_motor2;
extern int next_period;


void PatternTimerCallback();  // This is for 1 ms timer counter to drive motors
void PatternPlayer_Init();
void SetDuty(Motor_t motor, int duty,int period);

#endif /* PATTERN_PLAYER_HPP_ */
