/*
 *  Tim Lindbom (tli89)
 *  & Steph Post (spo88)
 *  group 55
 */

#include <buttons.h>
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/adc.h"
#include "driverlib/pwm.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "driverlib/debug.h"
#include "utils/ustdlib.h"
#include "OrbitOLED/OrbitOLEDInterface.h"
#include "stdlib.h"
#include "circBufT.h"
#include "Display.h"
#include "altitude.h"
#include "yaw.h"
#include "inc/hw_ints.h"  // Interrupts

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

    initClock();
    initADC();
    initButtons();
    OLEDInitialise();
    initSysTick();
    initYaw();
    IntMasterEnable(); // Enable interrupts to the processor.


    enum states
    {
        LANDED,
        TAKEOFF,
        FLYING,
        LANDING
    };

    state currentState = LANDED;
    state previousState = NULL;
    int32_t landedADCValue = 0;
    int32_t curADCValue = 0;
    int32_t altitudePercentage = 0;

    int32_t yawInDregrees = 0;
    uint32_t yawRemainder = 0;


    //takes the initial sample mean and uses that as the 0%/ landed value
    landedADCValue = CalculateMeanADC();


    while (true)
    {

        if (currentState != previousState)
        {
            switch(currentState)
            {
            case LANDED:
                //
                break;
            case TAKEOFF:
                //
                break;
            case FLYING:
                //
                break;
            case LANDING:
                //
                break;
            }

            previousState = currentState;
        }
        //*****************************************************************************
        //Altitude
        //*****************************************************************************

        //calculates the current mean ADC value
        curADCValue = CalculateMeanADC();


        altitudePercentage = ((landedADCValue - curADCValue)  * 100 / ADC_RANGE);


        //*****************************************************************************
        //Yaw
        //*****************************************************************************

        // Converts the yaw value to degrees rounded to a whole number
        yawInDregrees = YawToDegrees();


        // Gets the decimal point for the yaw in degrees
        yawRemainder = GetYawRemainder();



        //*****************************************************************************
        //display
        //*****************************************************************************

        displayOLED(altitudePercentage, yawInDregrees, yawRemainder);


        //*****************************************************************************
        //button polling
        //*****************************************************************************

        if(checkButton(UP) == RELEASED)
        {
            //
        }

        if(checkButton(DOWN) == RELEASED)
        {
            //
        }

        if(checkButton(LEFT) == RELEASED)
        {
            //
        }

        if(checkButton(RIGHT) == RELEASED)
        {
            //
        }



    }
}


