/*
 * altitude.h
 *
 *  Created on: 20/04/2023
 *      Author: spo88
 */


#ifndef ALTITUDE_H_
#define ALTITUDE_H_

void initADC (void);

void ADCIntHandler(void);

int32_t CalculateMeanADC();




#endif // ALTITUDE_H_
