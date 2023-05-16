/*
 *  Tim Lindbom (tli89)
 *  & Steph Post (spo88)
 *  group 55
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

//flag to be set to true when ref yaw is found
extern volatile bool isYawCalibrated;


#define TEETH_NUM 112 // number of teeth on the disc
#define STATE_NUM 4 // number of states per tooth
#define DEGREES_IN_REV 360 // number of degrees in one revolution, scaled by a factor of 10


static int8_t current_state = 0;
static int8_t previous_state = 0;

static int32_t yaw = 0;
static int32_t currentYaw = 0;



// Setting up Interrupt for yaw quadrature encoder
void InitYaw (void);


// Finite State Machine to determine current state
void YawIntHandler(void);

// reference yaw interrupt handler
void RefYawIntHandler(void);


// Disables or enables the reference yaw interrupt
void DisableRefYawInt(bool disable);


void ChangeYaw();

int32_t YawToDegrees();

uint32_t GetYawRemainder();




#endif /* YAW_H_ */
