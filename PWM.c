/*
 *  Tim Lindbom (tli89)
 *  & Steph Post (spo88)
 *  group 55
 */


/*********************************************************
 * initialiseMainPWM
 * M0PWM7 (J4-05, PC5) is used for the main rotor motor
 *********************************************************/
void
initialiseMainPWM (void)
{
    //initialises the interrupt PWM for the main rotor
    SysCtlPeripheralEnable(PWM_MAIN_PERIPH_PWM);
    SysCtlPeripheralEnable(PWM_MAIN_PERIPH_GPIO);

    GPIOPinConfigure(PWM_MAIN_GPIO_CONFIG);
    GPIOPinTypePWM(PWM_MAIN_GPIO_BASE, PWM_MAIN_GPIO_PIN);

    PWMGenConfigure(PWM_MAIN_BASE, PWM_MAIN_GEN,
                    PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);
    // Set the initial PWM parameters
    setMainPWM (PWM_START_RATE_HZ, PWM_FIXED_DUTY);

    PWMGenEnable(PWM_MAIN_BASE, PWM_MAIN_GEN);

    // Disable the output.  Repeat this call with 'true' to turn O/P on.
    PWMOutputState(PWM_MAIN_BASE, PWM_MAIN_OUTBIT, false);

}


void initialiseTailPWM()
{
       //initialises the interrupt PWM for the tail rotor
       SysCtlPeripheralEnable(TAIL_PWM_PERIPH_PWM);
       //SysCtlPeripheralEnable(TAIL_PWM_PERIPH_GPIO);

       GPIOPinConfigure(TAIL_PWM_GPIO_CONFIG);
       GPIOPinTypePWM(TAIL_PWM_GPIO_BASE, TAIL_PWM_GPIO_PIN);

       PWMGenConfigure(TAIL_PWM_BASE, TAIL_PWM_GEN,
                       PWM_GEN_MODE_UP_DOWN | PWM_GEN_MODE_NO_SYNC);

       PWMGenEnable(TAIL_PWM_BASE, TAIL_PWM_GEN);

       // Disable the output.  Repeat this call with 'true' to turn O/P on.
       PWMOutputState(TAIL_PWM_BASE, TAIL_PWM_OUTBIT, true);

       // Set the initial PWM parameters
       setTailPWM();

}


//***********************************************************
// Function to set the frequency, duty cycle of the Main PWM
//***********************************************************
void
setMainPWM (uint32_t ui32Freq, uint32_t ui32Duty)
{
    // Calculate the PWM period corresponding to the frequency
    uint32_t ui32Period =
        SysCtlClockGet() / PWM_DIVIDER / ui32Freq;

    PWMGenPeriodSet(PWM_MAIN_BASE, PWM_MAIN_GEN, ui32Period);

    PWMPulseWidthSet(PWM_MAIN_BASE, PWM_MAIN_OUTNUM,
        ui32Period * ui32Duty / 100);
}



//***********************************************************
// Function to set the frequency, duty cycle of the Tail PWM
//***********************************************************
void setTailPWM ()
{
    // Calculate the PWM period corresponding to the frequency
        uint32_t ui32Period =
            SysCtlClockGet() / PWM_DIVIDER / TAIL_PWM_START_RATE_HZ;

        PWMGenPeriodSet(TAIL_PWM_BASE, TAIL_PWM_GEN, ui32Period);

        PWMPulseWidthSet(TAIL_PWM_BASE, TAIL_PWM_OUTNUM,
            ui32Period * TAIL_PWM_FIXED_DUTY / 100);
}




