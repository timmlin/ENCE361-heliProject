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




    // Initialisation of the interrupt for quadrature encoding on channel A & B
    GPIOPadConfigSet (YAW_PORT_BASE, YAW_A_PIN | YAW_B_PIN, GPIO_STRENGTH_2MA,
       GPIO_PIN_TYPE_STD_WPU);
    GPIOIntRegister(YAW_PORT_BASE, YawIntHandler);
    GPIOIntTypeSet(YAW_PORT_BASE, YAW_A_PIN | YAW_B_PIN, GPIO_BOTH_EDGES);
    GPIOIntEnable(YAW_PORT_BASE, YAW_A_PIN | YAW_B_PIN);
    IntEnable(INT_GPIOB);

}

// Finite State Machine to determine current state of yaw
void YawIntHandler(void)
{
    uint32_t encoderA = GPIOPinRead(YAW_PORT_BASE, YAW_A_PIN);
    uint32_t encoderB = GPIOPinRead(YAW_PORT_BASE, YAW_B_PIN);

    previous_state = current_state;

    if (encoderA) // if channel A leads channel B
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
    else // if channel b leads channel A

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

    GPIOIntClear(YAW_PORT_BASE, YAW_A_PIN | YAW_B_PIN);

    yaw = changeYaw();

}


uint32_t changeYaw()
{

    //yaw %= DEGREES_IN_REV;

    if (current_state > previous_state)
    {
        yaw += 2;
    }
    else if (current_state < previous_state)
    {
        yaw -= 2;
    }

    yaw += TEETH_NUM * STATE_NUM;

    return yaw;
}

uint32_t yawToDegrees()
{


    uint32_t yawInDegrees = ((yaw * SCALED_DEGREES_IN_REV) / (TEETH_NUM * STATE_NUM)) / 10;
    return yawInDegrees % 360;
}









