/*
 *  Tim Lindbom (tli89)
 *  & Steph Post (spo88)
 *  group 55
 */

#include <controllerPWM.h>
#include <controllerPWM.h>
#include <display.h>
#include <stdint.h>
#include <stdbool.h>

#include "stdlib.h"
#include "circBufT.h"
#include "altitude.h"
#include "yaw.h"
#include "buttons.h"
#include "uart.h"

//*****************************************************************************
// Constants
//*****************************************************************************
#define DEBUG



#define SYSTICK_RATE_HZ 100
#define SLOWTICK_RATE_HZ 4

#define SAMPLE_RATE_HZ 100
#define ADC_RANGE 1241

#define FIND_REF_YAW_DUTY_CYCLE 10
#define CALIBRATION_YAW_INCREMENT 5

#define MAIN_LANDED_PWM 0
#define TAIL_LANDED_PWM 0

#define HOVER_ALT_PERCENTAGE 10

#define ALTITUDE_INCREMENT 10
#define YAW_INCREMENT 15

#define MAX_ALTITUDE 100

#define DEGREES_IN_REV 360


#define RESET_PERIPH SYSCTL_PERIPH_GPIOA
#define RESET_PORT_BASE GPIO_PORTA_BASE
#define RESET_PIN 6



//*****************************************************************************
// Global variables
//*****************************************************************************

char statusStr[MAX_STR_LEN + 1];
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
   static uint8_t tickCount = 0;
  const uint8_t ticksPerSlow = SYSTICK_RATE_HZ / SLOWTICK_RATE_HZ;

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

void InitReset()
{

    SysCtlPeripheralEnable (RESET_PERIPH);
    GPIOPinTypeGPIOInput (RESET_PORT_BASE, RESET_PIN);
    GPIOPadConfigSet (RESET_PORT_BASE, RESET_PIN, GPIO_STRENGTH_2MA,
       GPIO_PIN_TYPE_STD_WPD);
    GPIOIntRegister(UP_BUT_PORT_BASE, ButtonsIntHandler);
    GPIOIntTypeSet(RESET_PORT_BASE, UP_BUT_PIN, GPIO_FALLING_EDGE);
    GPIOIntEnable(RESET_PORT_BASE, RESET_PIN);

}

void ResetIntHandler()
{
    GPIOIntClear(RESET_PORT_BASE, RESET_PIN);
    SysCtlReset();
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
    InitialiseUSB_UART();
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

    bool atHoverLocation = false;

    int16_t landedADCValue = 0;

    int16_t curADCValue = 0;
    int8_t currentAltitudePercentage = 0;
    int8_t targetAltitudePercentage = 0;

    int16_t targetYaw = 0;
    int16_t currentYaw = 0;
    int8_t yawRemainder = 0;

    int8_t mainDuty = 0;
    int8_t tailDuty = 0;



    //takes the initial sample mean and uses that as the intitial 0% landed value
    landedADCValue = CalculateMeanADC();


    while (true)
    {


        switch(currentState)
        {
        case LANDED:
            stateNum = 0;

            SetMainPWM(MAIN_LANDED_PWM);
            SetTailPWM(TAIL_LANDED_PWM);

            if((SWITCH1_FLAG) && GPIOPinRead(SWITCH1_PORT_BASE,SWITCH1_PIN)) // only called on a rising edge
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
                targetAltitudePercentage = HOVER_ALT_PERCENTAGE;
                DisableRefYawInt(false); // enable ref yaw interrupt



                targetYaw = currentYaw + CALIBRATION_YAW_INCREMENT;


                SetMainPWM(mainDuty);
                SetTailPWM(tailDuty);

            }
            else
            {
                atHoverLocation = true;
                DisableRefYawInt(true); // disable ref yaw interrupt
                currentState = FLYING;
                targetYaw = 0;
                stateChange = true;
            }


            if((SWITCH1_FLAG) && !GPIOPinRead(SWITCH1_PORT_BASE,SWITCH1_PIN))
            {
                SWITCH1_FLAG = false;

                currentState = LANDING;
                stateChange = true;
            }

            usprintf (statusStr, "Altitude =%4d%%  Yaw=%2d.% | \r\n mainPWM = %2d  tailPWM = %2d | \r\n TAKE OFF  \r\n",
                                 currentAltitudePercentage,
                                 currentYaw,
                                 yawRemainder,
                                 mainDuty,
                                 tailDuty);
            break;

        case FLYING:

           stateNum = 2;

           atHoverLocation = false;

            // Background task: Check for button flags set
            if(UP_BUTTON_FLAG)
            {
                UP_BUTTON_FLAG = false;

            if (targetAltitudePercentage >= MAX_ALTITUDE - 10)
            {
                targetAltitudePercentage = MAX_ALTITUDE;
            }
            else
            {
                targetAltitudePercentage += ALTITUDE_INCREMENT;
            }

            }

            if (DOWN_BUTTON_FLAG)
            {
                DOWN_BUTTON_FLAG = false;

                if (targetAltitudePercentage <= HOVER_ALT_PERCENTAGE)
                {
                    currentState = LANDING;
                    stateChange = true;
                }
                else
                {
                    targetAltitudePercentage -= HOVER_ALT_PERCENTAGE;
                }
            }



            if (LEFT_BUTTON_FLAG)
            {
                LEFT_BUTTON_FLAG = false;

                targetYaw -= YAW_INCREMENT;

                if (targetYaw < -((DEGREES_IN_REV/2) - 1))
                {
                    targetYaw  += DEGREES_IN_REV;
                }
            }



            if (RIGHT_BUTTON_FLAG)
            {
                RIGHT_BUTTON_FLAG = false;

                targetYaw += YAW_INCREMENT;

                if (targetYaw > DEGREES_IN_REV/2)
                {
                    targetYaw -= DEGREES_IN_REV;
                }
            }

            if((SWITCH1_FLAG) && !GPIOPinRead(SWITCH1_PORT_BASE,SWITCH1_PIN))
            {
                SWITCH1_FLAG = false;

                currentState = LANDING;
                targetAltitudePercentage = HOVER_ALT_PERCENTAGE;
                stateChange = true;
            }


            SetMainPWM(mainDuty);
            SetTailPWM(tailDuty);

            usprintf (statusStr, "Altitude =%4d%%  Yaw=%2d.% | \r\n mainPWM = %2d  tailPWM = %2d | \r\n FLYING  \r\n",
                                 currentAltitudePercentage,
                                 currentYaw, yawRemainder,
                                 mainDuty,
                                 tailDuty);
        break;
        case LANDING:

            stateNum = 3;

            targetYaw = 0;


            SetMainPWM(mainDuty);
            SetTailPWM(tailDuty);

            if(currentYaw == 0 && currentAltitudePercentage == 10)
            {
                atHoverLocation = true;
            }

            if (atHoverLocation)
            {
                targetAltitudePercentage = currentAltitudePercentage - 1;
            }

            if (currentYaw == 0 && currentAltitudePercentage == 0)
            {
                currentState = LANDED;
            }

            usprintf (statusStr, "Altitude =%4d%%  Yaw=%2d.% | \r\n mainPWM = %2d  tailPWM = %2d | \r\n LANDING  \r\n",
                                 currentAltitudePercentage,
                                 currentYaw, yawRemainder,
                                 mainDuty,
                                 tailDuty);

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

        mainDuty = MainRotorControlUpdate(targetAltitudePercentage, currentAltitudePercentage, 10);

        tailDuty = TailRotorControlUpdate (targetYaw, currentYaw,  10);

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
            // Form and send a status message to the console depending on the state

            switch(stateNum)
            {
                case 0:
                    usprintf (statusStr, "Altitude =%4d%%  Yaw=%2d.%2d  \r\nMainPWM = %2d  tailPWM = %2d  \r\nLANDED  \r\n----------------------\r\n",
                                               currentAltitudePercentage,
                                               currentYaw, yawRemainder,
                                               mainDuty,
                                               tailDuty);
                case 1:
                    usprintf (statusStr, "Altitude =%4d%%  Yaw=%2d.%2d  \r\nMainPWM = %2d  tailPWM = %2d  \r\nTAKE OFF  \r\n----------------------\r\n",
                                  currentAltitudePercentage,
                                  currentYaw, yawRemainder,
                                  mainDuty,
                                  tailDuty);
                case 2:
                    usprintf (statusStr, "Altitude =%4d%%  Yaw=%2d.%2d  \r\nMainPWM = %2d  tailPWM = %2d  \r\nFLYING  \r\n----------------------\r\n",
                                                               currentAltitudePercentage,
                                                               currentYaw, yawRemainder,
                                                               mainDuty,
                                                               tailDuty);
                case 3:
                    usprintf (statusStr, "Altitude =%4d%%  Yaw=%2d.%2d  \r\nMainPWM = %2d  tailPWM = %2d  \r\nLANDING  \r\n----------------------\r\n",
                                                               currentAltitudePercentage,
                                                               currentYaw, yawRemainder,
                                                               mainDuty,
                                                               tailDuty);
            }

           //UARTSend (statusStr);
        }


    }
}


