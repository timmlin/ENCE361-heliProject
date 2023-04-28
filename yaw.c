/*
 * yaw.c
 *
 *  Created on: 20/04/2023
 *      Author: spo88
 */



#include "yaw.h"

// Setting up Interrupt for yaw quadrature encoder
void initYaw (void)
{

    SysCtlPeripheralEnable (YAW_PERIPH);

    // Yaw Channel A
    GPIOPinTypeGPIOInput (YAW_PORT_BASE, YAW_A_PIN);
    GPIOPadConfigSet (YAW_PORT_BASE, YAW_A_PIN, GPIO_STRENGTH_2MA,
       GPIO_PIN_TYPE_STD_WPU);


    // Yaw Channel B
    GPIOPinTypeGPIOInput (YAW_PORT_BASE, YAW_B_PIN);
    GPIOPadConfigSet (YAW_PORT_BASE, YAW_B_PIN, GPIO_STRENGTH_2MA,
       GPIO_PIN_TYPE_STD_WPU);


    // Initialisation of the interrupt for quadrature encoding
    GPIOIntRegister(YAW_PORT_BASE, YawIntHandler);
    GPIOIntTypeSet(YAW_PORT_BASE, YAW_A_PIN, GPIO_BOTH_EDGES);
    GPIOIntTypeSet(YAW_PORT_BASE, YAW_B_PIN, GPIO_BOTH_EDGES);
    GPIOIntEnable(YAW_PORT_BASE, YAW_A_PIN);
    GPIOIntEnable(YAW_PORT_BASE, YAW_B_PIN);

}

// Finite State Machine to determine current state of yaw
void YawIntHandler(void)
{
    uint32_t encoderA = GPIOPinRead(YAW_PORT_BASE, YAW_A_PIN);
    uint32_t encoderB = GPIOPinRead(YAW_PORT_BASE, YAW_B_PIN);

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


uint32_t changeYaw(uint32_t yaw)
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

uint32_t yawToDegrees(uint32_t yaw)
{

    uint32_t degrees = ((yaw * DEGRESS_IN_REV) / (TEETH_NUM * STATE_NUM));
    return degrees;
}









