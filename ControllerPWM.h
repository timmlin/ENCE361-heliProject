/*
 * Controller.h
 *
 *  Created on: 11/05/2023
 *      Author: spo88
 */

#ifndef CONTROLLERPWM_H_
#define CONTROLLERPWM_H_

#include <stdint.h>
#include <stdbool.h>

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "utils/ustdlib.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h" //Needed for pin configure
#include "driverlib/pwm.h"
#include "stdlib.h"


// Systick configuration
#define SYSTICK_RATE_HZ    100

// PWM configuration
#define PWM_RATE_HZ  100
//#define PWM_RATE_STEP_HZ   50
//#define PWM_RATE_MIN_HZ    50
//#define PWM_RATE_MAX_HZ    400
//#define PWM_FIXED_DUTY     67
//#define PWM_MIN_DUTY       5
//#define PWM_MAX_DUTY       95
//#define PWM_DIVIDER_CODE   SYSCTL_PWMDIV_4
#define PWM_DIVIDER        4

/**********************************************************
 * PWM Constants
 **********************************************************/


//  PWM Hardware Details M0PWM7

//  ---Main Rotor PWM: PC5, J4-05

#define PWM_MAIN_BASE        PWM0_BASE
#define PWM_MAIN_GEN         PWM_GEN_3
#define PWM_MAIN_OUTNUM      PWM_OUT_7
#define PWM_MAIN_OUTBIT      PWM_OUT_7_BIT
#define PWM_MAIN_PERIPH_PWM  SYSCTL_PERIPH_PWM0
#define PWM_MAIN_PERIPH_GPIO SYSCTL_PERIPH_GPIOC
#define PWM_MAIN_GPIO_BASE   GPIO_PORTC_BASE
#define PWM_MAIN_GPIO_CONFIG GPIO_PC5_M0PWM7
#define PWM_MAIN_GPIO_PIN    GPIO_PIN_5



//--Tail Rotor PWM: PF1
#define TAIL_PWM_BASE            PWM1_BASE
#define TAIL_PWM_GEN             PWM_GEN_2
#define TAIL_PWM_OUTNUM          PWM_OUT_5
#define TAIL_PWM_OUTBIT          PWM_OUT_5_BIT
#define TAIL_PWM_PERIPH_PWM      SYSCTL_PERIPH_PWM1
#define TAIL_PWM_PERIPH_GPIO     SYSCTL_PERIPH_GPIOC
#define TAIL_PWM_GPIO_BASE       GPIO_PORTF_BASE
#define TAIL_PWM_GPIO_CONFIG     GPIO_PF1_M1PWM5
#define TAIL_PWM_GPIO_PIN        GPIO_PIN_1

/**********************************************************
 * PID Constants
 **********************************************************/
#define PWM_DIVISOR 100

#define MAIN_KP 400
#define MAIN_KI 20
#define MAIN_KD 150

#define TAIL_KP 500
#define TAIL_KI 20
#define TAIL_KD 10

void InitialiseMainPWM (void);

void InitialiseTailPWM(void);

void SetMainPWM (uint32_t mainPWMDuty);

void SetTailPWM (uint32_t tailPWMDuty);

int32_t MainRotorControlUpdate (int32_t targetAltitude, int32_t altitudePercentage, float deltaT);

int32_t TailRotorControlUpdate (int32_t targetYaw, int32_t currentYawInDegreers, float deltaT);




#endif /* CONTROLLERPWM_H_ */
