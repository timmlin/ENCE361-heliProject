/*
 *  Tim Lindbom (tli89)
 *  & Steph Post (spo88)
 *  group 55
 */

#include <controllerPWM.h>





// Initialise Error signal

int32_t mainError = 0;
int32_t mainPrevError = 0;
int32_t mainControl = 0;
int32_t mainI = 0;

int32_t tailI = 0;
int32_t tailError = 0;
int32_t tailPrevError = 0;
int32_t tailControl = 0;

/*********************************************************
 * initialiseMainPWM
 * M0PWM7 (J4-05, PC5) is used for the main rotor motor
 *********************************************************/

void InitialiseMainPWM (void)
{
    //initialises the interrupt PWM for the main rotor
    SysCtlPeripheralEnable(PWM_MAIN_PERIPH_PWM);
    SysCtlPeripheralEnable(PWM_MAIN_PERIPH_GPIO);

    GPIOPinConfigure(PWM_MAIN_GPIO_CONFIG);
    GPIOPinTypePWM(PWM_MAIN_GPIO_BASE, PWM_MAIN_GPIO_PIN);

    PWMGenConfigure(PWM_MAIN_BASE, PWM_MAIN_GEN,
                    PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);

    PWMGenEnable(PWM_MAIN_BASE, PWM_MAIN_GEN);

    // Disable the output.  Repeat this call with 'true' to turn O/P on.
    PWMOutputState(PWM_MAIN_BASE, PWM_MAIN_OUTBIT, true);

}


void InitialiseTailPWM()
{
       //initialises the interrupt PWM for the tail rotor
       SysCtlPeripheralEnable(TAIL_PWM_PERIPH_PWM);
       //SysCtlPeripheralEnable(TAIL_PWM_PERIPH_GPIO);

       GPIOPinConfigure(TAIL_PWM_GPIO_CONFIG);
       GPIOPinTypePWM(TAIL_PWM_GPIO_BASE, TAIL_PWM_GPIO_PIN);

       PWMGenConfigure(TAIL_PWM_BASE, TAIL_PWM_GEN,
                       PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);

       PWMGenEnable(TAIL_PWM_BASE, TAIL_PWM_GEN);

       // Disable the output.  Repeat this call with 'true' to turn O/P on.
       PWMOutputState(TAIL_PWM_BASE, TAIL_PWM_OUTBIT, true);


}


//***********************************************************
// Function to set the frequency, duty cycle of the Main PWM
//***********************************************************
void SetMainPWM (uint32_t mainPWMDuty)
{
    // Calculate the PWM period corresponding to the frequency
    uint32_t mainPWMPeriod =
        SysCtlClockGet() / PWM_DIVIDER / PWM_RATE_HZ;

    PWMGenPeriodSet(PWM_MAIN_BASE, PWM_MAIN_GEN, mainPWMPeriod);

    PWMPulseWidthSet(PWM_MAIN_BASE, PWM_MAIN_OUTNUM,
                     mainPWMPeriod * mainPWMDuty / 100);
}



//***********************************************************
// Function to set the frequency, duty cycle of the Tail PWM
//***********************************************************
void SetTailPWM (uint32_t tailPWMDuty)
{
    // Calculate the PWM period corresponding to the frequency
        uint32_t tailPWMPeriod =
            SysCtlClockGet() / PWM_DIVIDER / PWM_RATE_HZ;

        PWMGenPeriodSet(TAIL_PWM_BASE, TAIL_PWM_GEN, tailPWMPeriod);

        PWMPulseWidthSet(TAIL_PWM_BASE, TAIL_PWM_OUTNUM,
                         tailPWMPeriod * tailPWMDuty / 100);
}



// *******************************************************
// Main Rotor PID control for Altitude
// *******************************************************
int32_t MainRotorControlUpdate (int32_t targetAltitudePercentage, int32_t currentAltitudePercentage, uint8_t DELTA_T)
{
    mainError = targetAltitudePercentage - currentAltitudePercentage; // Error calculation for altitude

    //PID controller calculated
    int32_t mainP = MAIN_KP * mainError; // Proportional control
    int32_t mainDI = (MAIN_KI * mainError)/DELTA_T; // constantly updating part of integral control
    int32_t mainD = MAIN_KD * (mainError - mainPrevError)*DELTA_T; // derivative control

    mainControl = (mainP + (mainI + mainDI) + mainD)/PWM_DIVISOR;

    mainPrevError = mainError; // updates previous error for altitude

    // Place limits on the output

    if (mainControl > MAX_OUTPUT)
    {
        mainControl = MAX_OUTPUT;
    }
    else if (mainControl < MIN_OUTPUT)
    {
        mainControl = MIN_OUTPUT;
    }
    else
    {
        mainI += mainDI; // accumulates error signal from main rotor only if controller output is within the specified limits
    }

    return mainControl;
}

// *******************************************************
// Tail Rotor PID control for Yaw
// *******************************************************


int32_t TailRotorControlUpdate (int32_t targetYawInDegrees, int32_t currentYawInDegreers, uint8_t DELTA_T)
{
    tailError = targetYawInDegrees - currentYawInDegreers; // Error calculation for yaw


    if (tailError > DEGREES_IN_REV/2)
    {
        tailError -= DEGREES_IN_REV;
    }
    else if (tailError < -((DEGREES_IN_REV/2)-1))
    {
           tailError += DEGREES_IN_REV;
    }

    //PID controller calculated
    int32_t tailP = TAIL_KP * tailError; // Proportional control
    int32_t tailDI = (TAIL_KI * tailError)/DELTA_T; // constantly updating part of integral control
    int32_t tailD = TAIL_KD * (tailError - tailPrevError)* DELTA_T; // derivative control

    tailControl = (tailP + (tailI + tailDI) + tailD)/PWM_DIVISOR;

    tailPrevError = tailError; // updates previous error for altitude

    // Place limits on the output

    if (tailControl > MAX_OUTPUT)
    {
        tailControl = MAX_OUTPUT;
    }
    else if (tailControl < MIN_OUTPUT)
    {
        tailControl = MIN_OUTPUT;
    }
    else
    {
        tailI += tailDI; // accumulates error signal from tail rotor only if controller output is within the specified limits

    }

    return tailControl;
}


