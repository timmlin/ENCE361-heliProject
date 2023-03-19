/*
 * Display.c
 *
 *  Created on: 19/03/2023
 *      Author: spo88
 */

#include "Display.h"
#include "utils/ustdlib.h"


#define SCREEN_LINE_NUM 4


//initiliases the dispaly
void initDisplay (void)
{
    // intialise the Orbit OLED display
    OLEDInitialise ();
}



void clearDisplay()
{
    int8_t curLine;

    for(curLine = 0; curLine < SCREEN_LINE_NUM; curLine++)
    {
        OLEDStringDraw("                ", 0, curLine);
        curLine++;
    }
}



//*****************************************************************************
//
// Function to display the mean ADC value (10-bit value, note) and sample count.
//
//*****************************************************************************
void
displayMeanVal(uint16_t meanVal, uint32_t count)
{
    char string[17];  // 16 characters across the display

    OLEDStringDraw ("ADC demo 1", 0, 0);

    // Form a new string for the line.  The maximum width specified for the
    //  number field ensures it is displayed right justified.
    usnprintf (string, sizeof(string), "Mean ADC = %4d", meanVal);
    // Update line on display.
    OLEDStringDraw (string, 0, 1);

    usnprintf (string, sizeof(string), "Sample # %5d", count);
    OLEDStringDraw (string, 0, 3);
}

