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

void displayAltitude(int32_t altitudePercentage); // displays the altitude percentage on the OLED







#endif /* DISPLAY_H_ */
