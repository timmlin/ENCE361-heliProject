/*
 *  Tim Lindbom (tli89)
 *  & Steph Post (spo88)
 *  group 55
 */



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

// *******************************************************
// Main Rotor PID control for Altitude
// *******************************************************
(void) MainRotorControlUpdate (int32_t TargetAltitude, int32_t altitudePercentage, uint32_t deltaT)
{
    int32_t errorMain = TargetAltitude - CurrentAltitude; // Error calculation for altitude

    //PID controller calculated

    PMain = KP_MAIN * errorMain; // Proportional control
    dIMain = KI_MAIN * errorMain * deltaT; // constantly updating part of integral control
    DMain = KD_MAIN * (errorMain - prevErrorMain)/deltaT; // derivative control

    controlMain = PMain + (IMain + dIMain) + DMain;

    prevErrorMain = errorMain; // updates previous error for altitude

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

(void) TailRotorControlUpdate (int32_t TargetYaw, int32_t CurrentYawInDegreers, uint32_t deltaT)
{

    PTail = KpTail * errorTail;
    dITail = KiTail * errorTail * TTail;
    DTail = (KdTail/TTail) * (errorTail - prevErrorTail);

    controlTail = PTail + (ITail + dITail) + DTail;

    prevErrorTail = errorTail; // updates previous error for yaw

    // Place limits on the output

    if (controlTail > MAX_OUTPUT_TAIL)
        controlTail = MAX_OUTPUT_TAIL;
    else if (controlTail < MIN_OUTPUT_TAIL)
        controlTail = MIN_OUTPUT_TAIL;
    else
        ITail += dITail; // accumulates error signal from tail rotor only if controller output is within the specified limits
}
