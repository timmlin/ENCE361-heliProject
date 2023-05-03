/*
 * Display.c
 *
 *  Created on: 19/03/2023
 *      Author: spo88
 */

#include "Display.h"
#include "utils/ustdlib.h"


#define SCREEN_LINE_NUM 4


//initiliases the display
void initDisplay (void)
{
    // intialise the Orbit OLED display
    OLEDInitialise ();
}


//*****************************************************************************
//
// Function to clear every line of the display
//
//*****************************************************************************
void clearDisplay()
{
    int8_t curLine;

    for(curLine = 0; curLine <= SCREEN_LINE_NUM; curLine++)
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

    // Form a new string for the line.  The maximum width specified for the
    //  number field ensures it is displayed right justified.
    usnprintf (string, sizeof(string), "Mean ADC = %4d", meanVal);
    // Update line on display.
    OLEDStringDraw (string, 0, 0);

    usnprintf (string, sizeof(string), "Sample # %5d", count);
    OLEDStringDraw (string, 0, 3);
}


//*****************************************************************************
//
// Function to display the mean ADC value (10-bit value, note) and sample count.
//
//*****************************************************************************
void
displayAltitudeYaw(int32_t altitudePercentage, uint32_t yaw, uint32_t yawRemainder)
{
    char string[17];  // 16 characters across the display

    OLEDStringDraw ("Altitude and Yaw", 0, 0);

    // Form a new string for the line.  The maximum width specified for the
    //  number field ensures it is displayed right justified.
    usnprintf (string, sizeof(string), "Alt = %4d %%", altitudePercentage);
    // Update line on display.
    OLEDStringDraw (string, 0, 2);

    usnprintf (string, sizeof(string), "yaw = %2d.%1d  DEG", yaw, yawRemainder);
   // Update line on display.
   OLEDStringDraw (string, 0, 3);
}



















