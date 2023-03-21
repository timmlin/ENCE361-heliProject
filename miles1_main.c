/*
 * miles1_v1.c
 *
 *  Created on: 13/03/2023
 *      Author: spo88
 */

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
#include "buttons4.h"
#include "circBufT.h"
#include "Display.h"
#include "inc/hw_ints.h"  // Interrupts

//*****************************************************************************
// Constants
//*****************************************************************************
#define BUF_SIZE 25
#define SAMPLE_RATE_HZ 100
#define ADC_RANGE 1241


//*****************************************************************************
// Global variables
//*****************************************************************************
static circBuf_t g_inBuffer;        // Buffer of size BUF_SIZE integers (sample values)
static uint32_t g_ulSampCnt;    // Counter for the interrupts

//*****************************************************************************
//
// The interrupt handler for the for SysTick interrupt.
//
//*****************************************************************************
void
SysTickIntHandler(void)
{
    //
    // Initiate a conversion
    //
    ADCProcessorTrigger(ADC0_BASE, 3);
    g_ulSampCnt++;
}

//*****************************************************************************
//
// The handler for the ADC conversion complete interrupt.
// Writes to the circular buffer.
//
//*****************************************************************************
void
ADCIntHandler(void)
{
    uint32_t ulValue;

    //
    // Get the single sample from ADC0.  ADC_BASE is defined in
    // inc/hw_memmap.h
    ADCSequenceDataGet(ADC0_BASE, 3, &ulValue);
    //
    // Place it in the circular buffer (advancing write index)
    writeCircBuf (&g_inBuffer, ulValue);
    //
    // Clean up, clearing the interrupt
    ADCIntClear(ADC0_BASE, 3);
}

//*****************************************************************************
// Initialisation functions for the clock (incl. SysTick), ADC, display
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

void
initADC (void)
{

    initCircBuf (&g_inBuffer, BUF_SIZE);

    //
    // The ADC0 peripheral must be enabled for configuration and use.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

    // Enable sample sequence 3 with a processor signal trigger.  Sequence 3
    // will do a single sample when the processor sends a signal to start the
    // conversion.
    ADCSequenceConfigure(ADC0_BASE, 3, ADC_TRIGGER_PROCESSOR, 0);

    //
    // Configure step 0 on sequence 3.  Sample channel 0 (ADC_CTL_CH0) in
    // single-ended mode (default) and configure the interrupt flag
    // (ADC_CTL_IE) to be set when the sample is done.  Tell the ADC logic
    // that this is the last conversion on sequence 3 (ADC_CTL_END).  Sequence
    // 3 has only one programmable step.  Sequence 1 and 2 have 4 steps, and
    // sequence 0 has 8 programmable steps.  Since we are only doing a single
    // conversion using sequence 3 we will only configure step 0.  For more
    // on the ADC sequences and steps, refer to the LM3S1968 datasheet.
    ADCSequenceStepConfigure(ADC0_BASE, 3, 0, ADC_CTL_CH9 | ADC_CTL_IE |
                             ADC_CTL_END);

    //
    // Since sample sequence 3 is now configured, it must be enabled.
    ADCSequenceEnable(ADC0_BASE, 3);

    //
    // Register the interrupt handler
    ADCIntRegister (ADC0_BASE, 3, ADCIntHandler);

    //
    // Enable interrupts for ADC0 sequence 3 (clears any outstanding interrupts)
    ADCIntEnable(ADC0_BASE, 3);
}


/*void
IntPrioritySet(uint32_t INT_GPIOE_TM4C123, uint8_t 4) // setting priority for LEFT button push interrupt (for altitude reset) initially at interrupt priority 4
{

}*/




int
main(void)

{
    uint16_t i;
    int32_t sum;

    bool TakeLandedSample = true;
    int8_t displayNumber = 0;
    int32_t bufferRoundedMean;
    int32_t landedADCValue = 0;
    int32_t curADCValue = 0;
    int32_t altitudePercentage = 0;



    initClock ();
    initADC ();
    initDisplay ();
    initButtons ();



    // Enable interrupts to the processor.
    IntMasterEnable();

    while (true)
    {


        // Background task: calculate the (approximate) mean of the values in the
        // circular buffer and display it, together with the sample number.
        sum = 0;
        for (i = 0; i < BUF_SIZE; i++)
        {
            sum = sum + readCircBuf (&g_inBuffer);
        }

        curADCValue = sum / BUF_SIZE;


        //takes the first sample mean and uses that as the 0%/ landed value
        if(TakeLandedSample)
        {
            landedADCValue = curADCValue;
            TakeLandedSample = false;
        }

        altitudePercentage = ((landedADCValue - curADCValue)  * 100 / ADC_RANGE);

        updateButtons ();

        //updates the display number
        if (checkButton(UP) == PUSHED)
        {
            displayNumber++;
            displayNumber %= 3;
        }

        // sets the landed sampling boolean to true
        // to recalculate the 0% value
        if(checkButton(LEFT) == PUSHED)
        {
            TakeLandedSample = true;
        }

        // Calculate and display the rounded mean of the buffer contents
        bufferRoundedMean = (2 * sum + BUF_SIZE) / 2 / BUF_SIZE;


        switch (displayNumber)
        {
            case(0):
                //Displays the altitude percentage
                displayAltitude(altitudePercentage);
                break;

            case(1):
                //Displays the rounded mean value of the buffer
                displayMeanVal (bufferRoundedMean, g_ulSampCnt);
                break;

            case(2):
                // Clears the display
                clearDisplay();
                break;

            default:
                break;
        }

        SysCtlDelay (SysCtlClockGet() / 6);  // Update display at ~ 2 Hz
    }
}


