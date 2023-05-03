/*
 * Display.h
 *
 *  Created on: 19/03/2023
 *      Author: spo88
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <stdint.h>
#include "OrbitOLED/OrbitOLEDInterface.h"



void initDisplay(void); // initialise the Orbit display

void clearDisplay(void); // clears every line of the contents of the OLED display

void displayMeanVal(uint16_t meanVal, uint32_t count); // displays the rounded mean value of the buffer

void displayAltitudeYaw(int32_t altitudePercentage, uint32_t yaw, uint32_t yawRemainder); // displays the altitude percentage and yaw on the OLED







#endif /* DISPLAY_H_ */
