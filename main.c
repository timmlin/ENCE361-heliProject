/*
 *  Tim Lindbom (tli89)
 *  & Steph Post (spo88)
 *  group 55
 */

#include <stdint.h>
#include <stdbool.h>

#include "stdlib.h"
#include "circBufT.h"
#include "Display.h"
#include "altitude.h"
#include "yaw.h"
#include "PWM.h"
#include "buttons.h"
#include "Controller.h"

//*****************************************************************************
// Constants
//*****************************************************************************
#define SYSTICK_RATE_HZ 100
#define SLOWTICK_RATE_HZ 4

#define SAMPLE_RATE_HZ 100
#define ADC_RANGE 1241




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

    bool stateChange = false;
    int32_t landedADCValue = 0;
    int32_t curADCValue = 0;
    int32_t altitudePercentage = 0;
    int32_t targetAltitude = 0;

    int32_t yawInDregrees = 0;
    uint32_t yawRemainder = 0;

    // used to calculate deltaT for the timer intergral
    uint32_t loopCounter = 0;

    //takes the initial sample mean and uses that as the 0%/ landed value
    landedADCValue = CalculateMeanADC();


    //*****************************************************************************
    // Finite State Machine
    //*****************************************************************************
    while (true)
    {
        loopCounter++;


        if (currentState != previousState)
        {
            stateChange = true;
            previousState = currentState;
        }

        switch(currentState)
        {
        case LANDED:


            //
            break;
        case TAKEOFF:
            //
            break;

        case FLYING:

            // Background task: Check for button flags are set
            if(UP_BUTTON_FLAG)
            {
                UP_BUTTON_FLAG = false;
                /*
                    do up button stuff

               */

            }

            if (DOWN_BUTTON_FLAG)
            {
                DOWN_BUTTON_FLAG = false;
                /*
                    do down button stuff

               */
            }


            if (LEFT_BUTTON_FLAG)
            {
                LEFT_BUTTON_FLAG = false;
                /*
                    do left button stuff

               */


            }

            if (RIGHT_BUTTON_FLAG)
            {
                RIGHT_BUTTON_FLAG = false;

                targetYawInDegrees += 15

            }

        break;

        case LANDING:
            //
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
        yawInDregrees = YawToDegrees();


        // Gets the decimal point for the yaw in degrees
        yawRemainder = GetYawRemainder();


        //*****************************************************************************
        //PID Control
        //*****************************************************************************
        if (currentAltitudePercentage != targetAltitudePercentage).3

        {
            MainRotorControlUpdate(int32_t TargetAltitude, int32_t altitudePercentage, 0.1);

            loopCounter = 0;
        }

        if (currentYaw != targetYaw)
        {
            TailRotorControlUpdate (int32_t TargetYaw, int32_t CurrentYawInDegreers, 0.1);

            loopCounter = 0;
        }


        //*****************************************************************************
        //display
        //*****************************************************************************

        displayOLED(altitudePercentage, yawInDregrees, yawRemainder);






    }
}


