/*
 * yaw.c
 *
 *  Created on: 20/04/2023
 *      Author: spo88
 */


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
#include "inc/hw_ints.h"  // Interrupts


#define YAW_A_PERIPH  SYSCTL_PERIPH_GPIOB
#define YAW_A_PORT_BASE GPIO_PORTB_BASE
#define YAW_A_PIN GPIO_PIN_0
#define YAW_A_NORMAL  false

#define YAW_B_PERIPH  SYSCTL_PERIPH_GPIOB
#define YAW_B_PORT_BASE GPIO_PORTB_BASE
#define YAW_B_PIN GPIO_PIN_1
#define YAW_B_NORMAL  false

#define YAW_REF_PERIPH  SYSCTL_PERIPH_GPIOC
#define YAW_REF_PORT_BASE GPIO_PORTC_BASE
#define YAW_REF_PIN GPIO_PIN_4
#define YAW_REF_NORMAL  false


static int8_t current_state;
static int8_t previous_state;



void
initYaw (void)
{

    // Yaw Channel A
    SysCtlPeripheralEnable (YAW_A_PERIPH);
    GPIOPinTypeGPIOInput (YAW_A_PORT_BASE, YAW_A_PIN);
    GPIOPadConfigSet (YAW_A_PORT_BASE, YAW_A_PIN, GPIO_STRENGTH_2MA,
       GPIO_PIN_TYPE_STD_WPU);


    // Yaw Channel B
    GPIOPinTypeGPIOInput (YAW_B_PORT_BASE, YAW_B_PIN);
    GPIOPadConfigSet (YAW_B_PORT_BASE, YAW_B_PIN, GPIO_STRENGTH_2MA,
       GPIO_PIN_TYPE_STD_WPU);

}

// Finite State Machine to determine current state
YawIntHandler(void)
{
    uint32_t encoderA = GPIOPinRead(YAW_A_PORT_BASE, YAW_A_PIN);
    uint32_t encoderB = GPIOPinRead(YAW_B_PORT_BASE, YAW_B_PIN);

    previous_state = current_state;

    if (encoderA)
    {
        if (encoderB)
        {
            current_state = 3;
        }
        else
        {
            current_state = 4;
        }
    }
    else
    {
        if (encoderB)
        {
            current_state = 2;
        }
        else
        {
            current_state = 1;
        }
    }
}


uint32_t changeYaw(yaw)
{
    if (current_state < previous_state)
    {
        yaw++;
    }
    else
    {
        yaw--;
    }

    return yaw;
}

uint32_t yawToDegrees(yaw)
{

    uint32_t degrees = 0;
    return degrees;
}









