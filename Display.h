/*
 *  Tim Lindbom (tli89)
 *  & Steph Post (spo88)
 *  group 55
 */


#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <stdint.h>
#include "OrbitOLED/OrbitOLEDInterface.h"


#define SCREEN_LINE_NUM 4
#define MAX_DISPLAY_CHAR 17


void ClearDisplay(void); // clears every line of the contents of the OLED display

// displays the altitude percentage and yaw on the OLED
void DebugDisplayOLED(int32_t altitudePercentage,
                 int32_t yawInDregrees,
                 uint32_t yawRemainder,
                 uint8_t state);

void DisplayOLED(int32_t altitudePercentage,
                 int32_t yawInDregrees,
                 uint32_t yawRemainder,
                 uint32_t mainPWMDuty,
                 uint32_t tailPWMDuty);



#endif /* DISPLAY_H_ */
