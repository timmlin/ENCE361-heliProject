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

#define MAIN_LANDED_PWM 0
#define TAIL_LANDED_PWM 0

#define HOVER_ALT_PERCENTAGE 10



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

   updateButtons();       // Poll the buttons and runs a debouncing algorithm

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
    uint8_t stateNum = 0;

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


        switch(currentState)
        {
        case LANDED:
            stateNum = 0;
            setMainPWM(MAIN_LANDED_PWM);
            setTailPWM(TAIL_LANDED_PWM);

            if(SWITCH1_FLAG)
            {
                SWITCH1_FLAG = false;
                currentState = TAKEOFF;
                stateChange = true;
            }


            break;

        case TAKEOFF:

            stateNum = 1;

            if(stateChange)
            {
                stateChange = false;
                isYawCalibrated = false;
            }

            if(!isYawCalibrated)
            {
                targetYawInDegrees = 361; //cause heli to rotate untill ref yaw is found
                targetAltitudePercentage = HOVER_ALT_PERCENTAGE;
                disableRefYawInt(false); // enable ref yaw interrupt

                setMainPWM(mainDuty);
                setTailPWM(tailDuty);

                if (currentAltitudePercentage > 0) //stop heli from rotating while on ground
                {
                    setTailPWM(tailDuty);
                }
            }
            else
            {

               disableRefYawInt(true); // disable ref yaw interrupt
               currentState = FLYING;
               targetYawInDegrees = 0;
               stateChange = true;
            }

            if(SWITCH1_FLAG)
            {
                SWITCH1_FLAG = false;

                currentState = LANDING;
                stateChange = true;
            }

            break;

        case FLYING:
               stateNum = 2;

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

            if(SWITCH1_FLAG)
            {
                SWITCH1_FLAG = false;

                currentState = LANDING;
                targetAltitudePercentage = 10;
                stateChange = true;
            }


            setMainPWM(mainDuty);
            setTailPWM(tailDuty);


        break;
        case LANDING:
            // something here about Is landing position found (same actually as in calibration stage)
            // set target yaw to 0
            stateNum = 3;

            targetYawInDegrees = 0;


            setMainPWM(mainDuty);
            setTailPWM(tailDuty);

            if(currentYawInDegrees == 0 && currentAltitudePercentage == 10)
            {
                targetAltitudePercentage = 0;
            }
            if (currentYawInDegrees == 0 && currentAltitudePercentage == 0)
            {
                currentState = LANDED;
            }


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

        mainDuty = mainRotorControlUpdate(targetAltitudePercentage, currentAltitudePercentage, 0.1);

        tailDuty = tailRotorControlUpdate (targetYawInDegrees, currentYawInDegrees,  1);

        //*****************************************************************************
        //display
        //*****************************************************************************

        displayOLED(currentAltitudePercentage, currentYawInDegrees, yawRemainder, stateNum, mainDuty, tailDuty );






    }
}


