/*
 * Controller.h
 *
 *  Created on: 11/05/2023
 *      Author: spo88
 */

#ifndef CONTROLLER_H_
#define CONTROLLER_H_

#include <stdint.h>
#include <stdbool.h>

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "utils/ustdlib.h"
#include "stdlib.h"


#define KP_MAIN
#define KI_MAIN
#define KD_MAIN

#define KP_TAIL
#define KI_TAIL
#define KD_TAIL


(void) MainRotorControlUpdate (int32_t TargetAltitude, int32_t altitudePercentage, uint32_t deltaT);

(void) TailRotorControlUpdate (int32_t TargetYaw, int32_t CurrentYawInDegreers, uint32_t deltaT);




#endif /* CONTROLLER_H_ */
