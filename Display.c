/*
 *  Tim Lindbom (tli89)
 *  & Steph Post (spo88)
 *  group 55
 */

#include <display.h>
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




void DebugDisplayOLED(int32_t altitudePercentage,
                 int32_t yawInDregrees,
                 uint32_t yawRemainder,
                 uint8_t state)
{
    char string[MAX_DISPLAY_CHAR];  // 16 characters across the display


    usnprintf (string, sizeof(string), "Alt = %4d %%", altitudePercentage);
    // Update line on display.
    OLEDStringDraw (string, 0, 0);

    usnprintf (string, sizeof(string), "yaw = %2d.%1d DEG  ", yawInDregrees, yawRemainder);
   // Update line on display.
   OLEDStringDraw (string, 0, 1);


   usnprintf (string, sizeof(string), "%d", state);
     // Update line on display.
     OLEDStringDraw (string, 0, 3);



}


// display the altitude percentage, yaw, and main/tail PWM onto the OLED display
void DisplayOLED(int32_t altitudePercentage,
                 int32_t yawInDregrees,
                 uint32_t yawRemainder,
                 uint32_t mainPWMDuty,
                 uint32_t tailPWMDuty)

{
    char string[MAX_DISPLAY_CHAR];  // 16 characters across the display

    usnprintf (string, sizeof(string), "Alt = %4d %%", altitudePercentage);
    // Update line on display.
    OLEDStringDraw (string, 0, 0);

    usnprintf (string, sizeof(string), "Yaw = %2d.%1d DEG  ", yawInDregrees, yawRemainder);
   // Update line on display.
   OLEDStringDraw (string, 0, 1);

   usnprintf (string, sizeof(string), "Main PWM = %4d %%  ", mainPWMDuty);
   // Update line on display.
   OLEDStringDraw (string, 0, 2);

   usnprintf (string, sizeof(string), "Tail PWM = %4d %%  ", tailPWMDuty);
      // Update line on display.
      OLEDStringDraw (string, 0, 3);

}









