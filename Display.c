/*
 *  Tim Lindbom (tli89)
 *  & Steph Post (spo88)
 *  group 55
 */

#include "Display.h"
#include "utils/ustdlib.h"






//clears every line of the display
void clearDisplay()
{
    int8_t curLine;

    for(curLine = 0; curLine <= SCREEN_LINE_NUM; curLine++)
    {
        OLEDStringDraw("                ", 0, curLine);
        curLine++;
    }
}



// display the altitude percentage, yaw, and main/tail PWM onto the OLED display
void displayOLED(int32_t altitudePercentage, int32_t yawInDregrees, uint32_t yawRemainder)
{
    char string[MAX_DISPLAY_CHAR];  // 16 characters across the display


    usnprintf (string, sizeof(string), "Alt = %4d %%", altitudePercentage);
    // Update line on display.
    OLEDStringDraw (string, 0, 0);

    usnprintf (string, sizeof(string), "yaw = %2d.%1d DEG  ", yawInDregrees, yawRemainder);
   // Update line on display.
   OLEDStringDraw (string, 0, 1);





}



















