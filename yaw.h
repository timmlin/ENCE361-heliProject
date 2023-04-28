/*
 * yaw.h
 *
 *  Created on: 20/04/2023
 *      Author: spo88
 */

#ifndef YAW_H_
#define YAW_H_


#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "driverlib/debug.h"
#include "utils/ustdlib.h"
#include "stdlib.h"
#include "inc/hw_ints.h"

#define YAW_PERIPH  SYSCTL_PERIPH_GPIOB
#define YAW_PORT_BASE GPIO_PORTB_BASE
#define YAW_A_PIN GPIO_PIN_0
#define YAW_A_NORMAL  false

#define YAW_B_PIN GPIO_PIN_1
#define YAW_B_NORMAL  false

#define YAW_REF_PERIPH  SYSCTL_PERIPH_GPIOC
#define YAW_REF_PORT_BASE GPIO_PORTC_BASE
#define YAW_REF_PIN GPIO_PIN_4
#define YAW_REF_NORMAL  false

#define TEETH_NUM 112 // number of teeth on the disc
#define STATE_NUM 4 // number of states per tooth
#define DEGRESS_IN_REV 360 // number of degrees in one revolution


static int8_t current_state;
static int8_t previous_state;


// Setting up Interrupt for yaw quadrature encoder
void initYaw (void);


// Finite State Machine to determine current state
void YawIntHandler(void);


uint32_t changeYaw(uint32_t yaw);

uint32_t yawToDegrees(uint32_t yaw);





#endif /* YAW_H_ */
