/*
 *  Tim Lindbom (tli89)
 *  & Steph Post (spo88)
 *  group 55
 */

#include "ControllerPWM.h"


#define MAX_OUTPUT_MAIN
#define MIN_OUTPUT_MAIN
#define MAX_OUTPUT_TAIL
#define MIN_OUTPUT_TAIL


// Initialise Error signal

int32_t errorMain = 0;
int32_t prevErrorMain = 0;

int32_t errorTail = 0;
int32_t prevErrorTail = 0;

int32_t TargetAltitudePercentage = 0;
int32_t TargetYawInDegrees = 0;

/*********************************************************
 * initialiseMainPWM
 * M0PWM7 (J4-05, PC5) is used for the main rotor motor
 *********************************************************/

void
initialiseMainPWM (void)
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


void initialiseTailPWM()
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
void setMainPWM (uint32_t MainPWMFreq, uint32_t MainPWMDuty)
{
    // Calculate the PWM period corresponding to the frequency
    uint32_t MainPWMPeriod =
        SysCtlClockGet() / PWM_DIVIDER / MainPWMFreq;

    PWMGenPeriodSet(PWM_MAIN_BASE, PWM_MAIN_GEN, MainPWMPeriod);

    PWMPulseWidthSet(PWM_MAIN_BASE, PWM_MAIN_OUTNUM,
                     MainPWMPeriod * MainPWMDuty / 100);
}



//***********************************************************
// Function to set the frequency, duty cycle of the Tail PWM
//***********************************************************
void setTailPWM (uint32_t TailPWMFreq, uint32_t TailPWMduty)
{
    // Calculate the PWM period corresponding to the frequency
        uint32_t TailPWMPeriod =
            SysCtlClockGet() / PWM_DIVIDER / TailPWMFreq;

        PWMGenPeriodSet(TAIL_PWM_BASE, TAIL_PWM_GEN, TailPWMPeriod);

        PWMPulseWidthSet(TAIL_PWM_BASE, TAIL_PWM_OUTNUM,
                         TailPWMPeriod * TailPWMduty / 100);
}



// *******************************************************
// Main Rotor PID control for Altitude
// *******************************************************
/*void MainRotorControlUpdate (int32_t TargetAltitudePercentage, int32_t currentAltitudePercentage, float deltaT)
{
    int32_t errorMain = TargetAltitudePercentage - CurrentAltitudePercentage; // Error calculation for altitude

    //PID controller calculated

    PMain = KP_MAIN * errorMain; // Proportional control
    dIMain = KI_MAIN * errorMain * deltaT; // constantly updating part of integral control
    DMain = KD_MAIN * (errorMain - prevErrorMain)/deltaT; // derivative control

    controlMain = PMain + (IMain + dIMain) + DMain;

    prevErrorMain = errorMain; // updates previous error for altitude

// Place limits on the output

    if (controlMain > MAX_OUTPUT_MAIN)
        controlMain = MAX_OUTPUT_MAIN;
    else if (controlMain < MIN_OUTPUT_MAIN)
        controlMain = MIN_OUTPUT_MAIN;
    else
        IMain += dIMain; // accumulates error signal from main rotor only if controller output is within the specified limits

}*/
// *******************************************************
// Tail Rotor PID control for Yaw
// *******************************************************

/*void TailRotorControlUpdate (int32_t TargetYawInDegrees, int32_t CurrentYawInDegreers, float deltaT)
{

    PTail = KpTail * errorTail;
    dITail = KiTail * errorTail * TTail;
    DTail = (KdTail/TTail) * (errorTail - prevErrorTail);

    controlTail = PTail + (ITail + dITail) + DTail;

    prevErrorTail = errorTail; // updates previous error for yaw

    // Place limits on the output

    if (controlTail > MAX_OUTPUT_TAIL)
        controlTail = MAX_OUTPUT_TAIL;
    else if (controlTail < MIN_OUTPUT_TAIL)
        controlTail = MIN_OUTPUT_TAIL;
    else
        ITail += dITail; // accumulates error signal from tail rotor only if controller output is within the specified limits
}*/


