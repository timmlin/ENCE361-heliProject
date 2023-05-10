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
    // As a precaution, make sure that the peripherals used are reset
    SysCtlPeripheralReset (PWM_MAIN_PERIPH_GPIO); // Used for PWM output
    SysCtlPeripheralReset (PWM_MAIN_PERIPH_PWM);  // Main Rotor PWM
    SysCtlPeripheralReset (TAIL_PWM_PERIPH_GPIO); // Used for PWM output for the tail
    SysCtlPeripheralReset (TAIL_PWM_PERIPH_PWM);  // Tail Rotor PWM

    SysCtlPeripheralReset (UP_BUT_PERIPH);        // UP button GPIO
    SysCtlPeripheralReset (DOWN_BUT_PERIPH);      // DOWN button GPIO
    SysCtlPeripheralReset (LEFT_BUT_PERIPH);      // LEFT button GPIO
    SysCtlPeripheralReset (RIGHT_BUT_PERIPH);     // RIGHT button GPIO

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
           NONE = 0,
           LANDED,
           TAKEOFF,
           FLYING,
           LANDING
       };

    enum States currentState  = LANDED;
    enum States previousState =  NONE;
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
            previousState = currentState;

            switch(currentState)
            {
            case LANDED:
                //
                break;
            case TAKEOFF:
                //
                break;

            case FLYING:

                // Background task: Check for button pushes and control
                // the PWM frequency within a fixed range.
                if ((checkButton (UP) == PUSHED) && (ui32Freq < PWM_RATE_MAX_HZ))
                {
                    ui32Freq += PWM_RATE_STEP_HZ;
                    displayPWM ("MainPWM", "Freq",  ui32Freq, 0);

                }

                if ((checkButton (DOWN) == PUSHED) && (ui32Freq > PWM_RATE_MIN_HZ))
                {
                    ui32Freq -= PWM_RATE_STEP_HZ;

                }


                //increases the duty by 5% and limits it to 95% of the maximum duty cycle
                if ((checkButton (RIGHT) == PUSHED) && (ui32Freq < PWM_RATE_MAX_HZ))
                {
                    newDuty = ui32Duty + 5;

                    if (newDuty <=  PWM_MAX_DUTY)
                    {
                        ui32Duty = newDuty;
                    }
                    else
                    {
                        ui32Duty = PWM_MAX_DUTY;
                    }

                }
                //decreases the duty cycle by 5% and limits it to 5% of the minimum duty cycle
                if ((checkButton (LEFT) == PUSHED) && (ui32Duty > PWM_MIN_DUTY))
                {
                    newDuty = ui32Duty - 5;

                    if (newDuty >=  PWM_MIN_DUTY)
                    {
                        ui32Duty = newDuty;
                    }
                    else
                    {
                        ui32Duty = PWM_MIN_DUTY;

                    }

                }


                setMainPWM (ui32Freq, ui32Duty);

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


