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
#include "uart.h"

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

#define DEBUG

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
void SysTickIntHandler (void)
{

   UpdateButtons();       // Poll the buttons and runs a debouncing algorithm

   if (++tickCount >= ticksPerSlow)
   {                       // Signal a slow tick
       tickCount = 0;
       slowTick = true;
   }
    ADCProcessorTrigger(ADC0_BASE, 3);
    g_ulSampCnt++;
}


//*******************************************************************
void initSysTick (void)
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
void initClock (void)
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



int main(void)

{
    // As a precaution, make sure that the peripherals used are reset
    SysCtlPeripheralReset (PWM_MAIN_PERIPH_GPIO); // Used for PWM output
    SysCtlPeripheralReset (PWM_MAIN_PERIPH_PWM);  // Main Rotor PWM
    SysCtlPeripheralReset (TAIL_PWM_PERIPH_GPIO); // Used for PWM output for the tail
    SysCtlPeripheralReset (TAIL_PWM_PERIPH_PWM);  // Tail Rotor PWM



    initClock();
    InitADC();
    InitButtons();
    OLEDInitialise();
    initSysTick();
    InitYaw();
    InitialiseMainPWM (); //initilises the main PWM signal
    InitialiseTailPWM(); // initilises  and sets the PWM signal for the tail motor
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

    int32_t targetYaw = 0;
    int32_t currentYaw = 0;
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
            SetMainPWM(MAIN_LANDED_PWM);
            SetTailPWM(TAIL_LANDED_PWM);

            if(SWITCH1_FLAG)
            {
                SWITCH1_FLAG = false;
                currentState = TAKEOFF;
                stateChange = true;
            }

            usprintf (statusStr, "Altitude =%4d%%  Yaw=%2d.% | \r\n mainPWM = %2d  tailPWM = %2d | \r\n LANDED",
                                 currentAltitudePercentage,
                                 currentYaw, yawRemainder,
                                 mainDuty,
                                 tailDuty); // * usprintf

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
                targetAltitudePercentage = HOVER_ALT_PERCENTAGE;
                DisableRefYawInt(false); // enable ref yaw interrupt


                if (currentAltitudePercentage > 0) //stop heli from rotating while on ground
                {
                    targetYaw = currentYaw + 15;
                }

                SetMainPWM(mainDuty);
                SetTailPWM(tailDuty);
            }
            else
            {

               DisableRefYawInt(true); // disable ref yaw interrupt
               currentState = FLYING;
               targetYaw = 0;
               stateChange = true;
            }

            if(SWITCH1_FLAG)
            {
                SWITCH1_FLAG = false;

                currentState = LANDING;
                stateChange = true;
            }

            usprintf (statusStr, "Altitude =%4d%%  Yaw=%2d.% | \r\n mainPWM = %2d  tailPWM = %2d | \r\n TAKE OFF",
                                 currentAltitudePercentage,
                                 currentYaw, yawRemainder,
                                 mainDuty,
                                 tailDuty); // * usprintf
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

                targetYaw -= 15;

                if (targetYaw < -179)
                {
                    targetYaw  += 360;
                }
            }



            if (RIGHT_BUTTON_FLAG)
            {
                RIGHT_BUTTON_FLAG = false;

                targetYaw += 15;

                if (targetYaw > 180)
                {
                    targetYaw -= 360;
                }
            }

            if(SWITCH1_FLAG)
            {
                SWITCH1_FLAG = false;

                currentState = LANDING;
                targetAltitudePercentage = 10;
                stateChange = true;
            }


            SetMainPWM(mainDuty);
            SetTailPWM(tailDuty);

            usprintf (statusStr, "Altitude =%4d%%  Yaw=%2d.% | \r\n mainPWM = %2d  tailPWM = %2d | \r\n FLYING",
                                 currentAltitudePercentage,
                                 currentYaw, yawRemainder,
                                 mainDuty,
                                 tailDuty); // * usprintf
        break;
        case LANDING:
            // something here about Is landing position found (same actually as in calibration stage)
            // set target yaw to 0
            stateNum = 3;

            targetYaw = 0;


            SetMainPWM(mainDuty);
            SetTailPWM(tailDuty);

            if(currentYaw == 0 && currentAltitudePercentage == 10)
            {
                targetAltitudePercentage = 0;
            }
            if (currentYaw == 0 && currentAltitudePercentage == 0)
            {
                currentState = LANDED;
            }

            usprintf (statusStr, "Altitude =%4d%%  Yaw=%2d.% | \r\n mainPWM = %2d  tailPWM = %2d | \r\n LANDING",
                                 currentAltitudePercentage,
                                 currentYaw, yawRemainder,
                                 mainDuty,
                                 tailDuty); // * usprintf

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
        currentYaw = YawToDegrees();


        // Gets the decimal point for the yaw in degrees
        yawRemainder = GetYawRemainder();


        //*****************************************************************************
        //PID Control
        //*****************************************************************************

        mainDuty = MainRotorControlUpdate(targetAltitudePercentage, currentAltitudePercentage, 0.1);

        tailDuty = TailRotorControlUpdate (targetYaw, currentYaw,  0.1);

        //*****************************************************************************
        //display
        //*****************************************************************************

        #ifdef DEBUG
            DebugDisplayOLED(currentAltitudePercentage, currentYaw, yawRemainder, stateNum);
        #endif

        #ifndef DEBUG
            DisplayOLED(currentAltitudePercentage, currentYaw, yawRemainder, mainDuty, tailDuty );
        #endif

        // Is it time to send a message?
        if (slowTick)
        {
            slowTick = false;
            // Form and send a status message to the console


            UARTSend (statusStr);
        }


    }
}


