/*
 *  Tim Lindbom (tli89)
 *  & Steph Post (spo88)
 *  group 55
 */

#include <ControllerPWM.h>
#include <stdint.h>
#include <stdbool.h>

#include "stdlib.h"
#include "circBufT.h"
#include "Display.h"
#include "altitude.h"
#include "yaw.h"
#include "ControllerPWM.h"
#include "buttons.h"

//*****************************************************************************
// Constants
//*****************************************************************************
#define SYSTICK_RATE_HZ 100
#define SLOWTICK_RATE_HZ 4

#define SAMPLE_RATE_HZ 100
#define ADC_RANGE 1241

#define FIND_REF_YAW_DUTY_CYCLE 10



//*****************************************************************************
// Global variables
//*****************************************************************************
static uint32_t g_ulSampCnt;    // Counter for the interrupts

volatile uint8_t slowTick = false;


//*****************************************************************************
//
// The interrupt handler for the for SysTick interrupt.
//
//*****************************************************************************
void
SysTickIntHandler (void)
{
    static uint8_t tickCount = 0;
       const uint8_t ticksPerSlow = SYSTICK_RATE_HZ / SLOWTICK_RATE_HZ;

       updateButtons ();       // Poll the buttons
       if (++tickCount >= ticksPerSlow)
       {                       // Signal a slow tick
           tickCount = 0;
           slowTick = true;
       }

    ADCProcessorTrigger(ADC0_BASE, 3);
    g_ulSampCnt++;
}


//*******************************************************************
void
initSysTick (void)
{
    //
    // Set up the period for the SysTick timer.  The SysTick timer period is
    // set as a function of the system clock.
    SysTickPeriodSet (SysCtlClockGet () / SYSTICK_RATE_HZ);
    //
    // Register the interrupt handler
    SysTickIntRegister (SysTickIntHandler);
    //
    // Enable interrupt and device
    SysTickIntEnable ();
    SysTickEnable ();
}

//*****************************************************************************
// Initialisation functions for the clock (incl. SysTick), ADC
//*****************************************************************************
void
initClock (void)
{
    // Set the clock rate to 20 MHz
    SysCtlClockSet (SYSCTL_SYSDIV_10 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN |
                   SYSCTL_XTAL_16MHZ);
    //
    // Set up the period for the SysTick timer.  The SysTick timer period is
    // set as a function of the system clock.
    SysTickPeriodSet(SysCtlClockGet() / SAMPLE_RATE_HZ);
    //
    // Register the interrupt handler
    SysTickIntRegister(SysTickIntHandler);
    //
    // Enable interrupt and device
    SysTickIntEnable();
    SysTickEnable();
}



int
main(void)
{
    // As a precaution, make sure that the peripherals used are reset
    SysCtlPeripheralReset (PWM_MAIN_PERIPH_GPIO); // Used for PWM output
    SysCtlPeripheralReset (PWM_MAIN_PERIPH_PWM);  // Main Rotor PWM
    SysCtlPeripheralReset (TAIL_PWM_PERIPH_GPIO); // Used for PWM output for the tail
    SysCtlPeripheralReset (TAIL_PWM_PERIPH_PWM);  // Tail Rotor PWM



    initClock();
    initADC();
    initButtons();
    OLEDInitialise();
    initSysTick();
    initYaw();
    initialiseMainPWM (); //initilises the main PWM signal
    initialiseTailPWM(); // initilises  and sets the PWM signal for the tail motor
    IntMasterEnable(); // Enable interrupts to the processor.


    enum States
       {
           LANDED = 0,
           TAKEOFF,
           FLYING,
           LANDING
       };
    enum States currentState  = LANDED;
    enum States previousState = LANDED;
    int stateNum = 0;

    bool stateChange = false;
    int32_t landedADCValue = 0;

    int32_t curADCValue = 0;
    int32_t currentAltitudePercentage = 0;
    int32_t targetAltitudePercentage = 0;

    int32_t targetYawInDegrees = 0;
    int32_t currentYawInDegrees = 0;
    uint32_t yawRemainder = 0;

    int32_t mainDuty = 0;
    int32_t tailDuty = 0;



    //takes the initial sample mean and uses that as the 0%/ landed value
    landedADCValue = CalculateMeanADC();


    while (true)
    {

    //*****************************************************************************
    // Finite State Machine
    //*****************************************************************************
        if (currentState != previousState)
        {
            stateChange = true;
            previousState = currentState;
        }

        switch(currentState)
        {
        case LANDED:

            if (SWITCH1_FLAG)
            {
                SWITCH1_FLAG = false;


                currentState = TAKEOFF;
                stateNum = 1;
            }


            break;

        case TAKEOFF:

            if(stateChange)
            {   // enables and disables for TAKEOFF state
                stateChange = false;
                isYawCalibrated = false;

                //go up here to TransitionHoverAltitude = 5% ??
                //PID control used here but could possibly be disabled if it's a problem
               // targetAltitudePercentage = 5;

            }

            if(!isYawCalibrated)
            {
                targetAltitudePercentage = 10;
                disableRefYawInt(false); // enable ref yaw interrupt

                setMainPWM(mainDuty);
                setTailPWM(43);
            }
            else
            {

               disableRefYawInt(true); // disable ref yaw interrupt
               currentState = FLYING;
               stateNum = 2;
               stateChange = true;
            }
            break;

        case FLYING:

            // Background task: Check for button flags are set
            if(UP_BUTTON_FLAG)
            {
                UP_BUTTON_FLAG = false;

                if (targetAltitudePercentage >= 90)
                {
                    targetAltitudePercentage = 100;
                }
                else
                {
                    targetAltitudePercentage += 10;
                }

            }



            if (DOWN_BUTTON_FLAG)
            {
                DOWN_BUTTON_FLAG = false;
                if (targetAltitudePercentage <= 10)
                {
                    currentState = LANDING;
                    stateChange = true;
                }
                else
                {
                    targetAltitudePercentage -= 10;
                }
            }



            if (LEFT_BUTTON_FLAG)
            {
                LEFT_BUTTON_FLAG = false;

                targetYawInDegrees -= 15;

                if (targetYawInDegrees < -180)
                {
                    targetYawInDegrees  += 360;
                }
            }



            if (RIGHT_BUTTON_FLAG)
            {
                RIGHT_BUTTON_FLAG = false;

                targetYawInDegrees += 15;

                if (targetYawInDegrees > 180)
                {
                    targetYawInDegrees -= 360;
                }
            }

        break;

        case LANDING:
            // something here about Is landing position found (same actually as in calibration stage)
            // set target yaw to 0

            //enable ref yaw interrupt
            disableRefYawInt(false);
            //setTailPWM(FIND_REF_YAW_DUTY_CYCLE);


            break;
        }



        //*****************************************************************************
        //Altitude
        //*****************************************************************************

        //calculates the current mean ADC value
        curADCValue = CalculateMeanADC();


        currentAltitudePercentage = ((landedADCValue - curADCValue)  * 100 / ADC_RANGE);


        //*****************************************************************************
        //Yaw
        //*****************************************************************************

        // Converts the yaw value to degrees rounded to a whole number
        currentYawInDegrees = YawToDegrees();


        // Gets the decimal point for the yaw in degrees
        yawRemainder = GetYawRemainder();


        //*****************************************************************************
        //PID Control
        //*****************************************************************************
        if (currentAltitudePercentage != targetAltitudePercentage);

        {
            mainDuty = mainRotorControlUpdate(targetAltitudePercentage, currentAltitudePercentage, 0.1);

        }

        if (currentYawInDegrees != targetYawInDegrees)
        {
            tailDuty = tailRotorControlUpdate ( targetYawInDegrees,  currentYawInDegrees,  0.1);


        }

        //*****************************************************************************
        //display
        //*****************************************************************************

        displayOLED(currentAltitudePercentage, currentYawInDegrees, yawRemainder, stateNum);






    }
}


