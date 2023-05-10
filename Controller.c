/*
 *  Tim Lindbom (tli89)
 *  & Steph Post (spo88)
 *  group 55
 */


#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "driverlib/debug.h"
#include "utils/ustdlib.h"
#include "stdlib.h"
#include "inc/hw_ints.h"


#define MAX_OUTPUT_MAIN
#define MIN_OUTPUT_MAIN
#define MAX_OUTPUT_TAIL
#define MIN_OUTPUT_TAIL


// Initialise Error signal

int32_t errorMain = 0;
int32_t prevErrorMain = 0;

int32_t errorTail = 0;
int32_t prevErrorTail = 0;

int32_t TargetAltitude = 0;
int32_t TargetYaw = 0;

errorMain = TargetAltitude - CurrentAltitude;

// *******************************************************
// Main Rotor PID control for Altitude
// *******************************************************
(void) MainRotorControlUpdate ()
{

    PMain = KpMain * errorMain; // Proportional control
    dIMain = KiMain * errorMain * TMain;
    DMain = (KdMain/TMain) * (errorMain - prevErrorMain);

    controlMain = PMain + (IMain + dIMain) + DMain;

    prevErrorMain = errorMain;

// Place limits on the output

    if (controlMain > MAX_OUTPUT_MAIN)
        controlMain = MAX_OUTPUT_MAIN;
    else if (controlMain < MIN_OUTPUT_MAIN)
        controlMain = MIN_OUTPUT_MAIN;
    else
        IMain += dIMain; // accumulates error signal from main rotor only if controller output is within the specified limits
    }

// *******************************************************
// Tail Rotor PID control for Yaw
// *******************************************************

(void) TailRotorControlUpdate ()
{

    PTail = KpTail * errorTail;
    dITail = KiTail * errorTail * TTail;
    DTail = (KdTail/TTail) * (errorTail - prevErrorTail);

    controlTail = PTail + (ITail + dITail) + DTail;

    prevErrorTail = errorTail;

    // Place limits on the output

    if (controlTail > MAX_OUTPUT_TAIL)
        controlTail = MAX_OUTPUT_TAIL;
    else if (controlTail < MIN_OUTPUT_TAIL)
        controlTail = MIN_OUTPUT_TAIL;
    else
        ITail += dITail; // accumulates error signal from tail rotor only if controller output is within the specified limits
}
