/*
 *  Tim Lindbom (tli89)
 *  & Steph Post (spo88)
 *  group 55
 */


#include "yaw.h"


volatile bool isYawCalibrated = false;

// Setting up Interrupt for yaw quadrature encoder
void InitYaw (void)
{

    SysCtlPeripheralEnable (YAW_PERIPH);
    SysCtlPeripheralEnable(YAW_REF_PERIPH);

    // Initialisation of the interrupt for quadrature encoding on channel A & B
    GPIOPadConfigSet (YAW_PORT_BASE, YAW_A_PIN | YAW_B_PIN, GPIO_STRENGTH_2MA,
       GPIO_PIN_TYPE_STD_WPU);
    GPIOIntRegister(YAW_PORT_BASE, YawIntHandler);
    GPIOIntTypeSet(YAW_PORT_BASE, YAW_A_PIN | YAW_B_PIN, GPIO_BOTH_EDGES);
    GPIOIntEnable(YAW_PORT_BASE, YAW_A_PIN | YAW_B_PIN);
    IntEnable(INT_GPIOB);


    //Initialise the reference yaw interrupt

    GPIOPadConfigSet (YAW_REF_PORT_BASE, YAW_REF_PIN, GPIO_STRENGTH_2MA,
         GPIO_PIN_TYPE_STD_WPU);
      GPIOIntRegister(YAW_REF_PORT_BASE, RefYawIntHandler);
      GPIOIntTypeSet(YAW_REF_PORT_BASE, YAW_REF_PIN, GPIO_RISING_EDGE);
      GPIOIntEnable(YAW_REF_PORT_BASE, YAW_REF_PIN);
      IntEnable(INT_GPIOC);

}

// Function to Disable Reference Yaw Interrupt once calibrated
void DisableRefYawInt(bool disable)
{
    if (disable)
    {
        GPIOIntDisable(YAW_REF_PORT_BASE, YAW_REF_PIN);
    }
    else
    {
        GPIOIntEnable(YAW_REF_PORT_BASE, YAW_REF_PIN);
    }
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

   ChangeYaw();

}

void RefYawIntHandler(void)
{
    GPIOIntClear(YAW_REF_PORT_BASE, YAW_REF_PIN);
    yaw = 0;
    isYawCalibrated = true;
}


void ChangeYaw()
{


    switch (current_state)
    {
    case 1:
        if (previous_state == 4)
        {
            yaw ++;
        }
        if (previous_state == 2)
        {
         yaw --;
        }
        break;
    case 2:
        if (previous_state == 1)
        {
            yaw ++;
        }
        if (previous_state == 3)
        {
         yaw --;
        }
        break;
    case 3:
        if (previous_state == 2)
        {
            yaw ++;
        }
        if (previous_state == 4)
        {
            yaw --;
        }
        break;
    case 4:
        if (previous_state == 3)
        {
            yaw ++;
        }
        if (previous_state == 1)
        {
            yaw --;
        }
        break;
    }
    yaw %= (TEETH_NUM * STATE_NUM);
}

int32_t YawToDegrees()
{
    currentYaw = yaw;


    if (currentYaw > (TEETH_NUM * STATE_NUM) /2)
    {
        currentYaw -= TEETH_NUM * STATE_NUM;
    }

    if (currentYaw < - ((TEETH_NUM * STATE_NUM) /2) -1)
    {
        currentYaw += TEETH_NUM * STATE_NUM;
    }

    int32_t yawInDegrees = ((currentYaw * DEGREES_IN_REV * SCALE_FACTOR) / (TEETH_NUM * STATE_NUM));

    return (yawInDegrees/SCALE_FACTOR)  % DEGREES_IN_REV; // returns the whole part of the yaw value in degrees
}

uint32_t GetYawRemainder()
{

    uint32_t YawRemainder = ((currentYaw * DEGREES_IN_REV * SCALE_FACTOR ) / (TEETH_NUM * STATE_NUM));
    return YawRemainder % 10; // will return the decimal point of the angle in degrees
}




