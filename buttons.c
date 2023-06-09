// *******************************************************
// 
// buttons4.c
//
// Support for a set of FOUR specific buttons on the Tiva/Orbit.
// ENCE361 sample code.
// The buttons are:  UP and DOWN (on the Orbit daughterboard) plus
// LEFT and RIGHT on the Tiva.
//
// Note that pin PF0 (the pin for the RIGHT pushbutton - SW2 on
//  the Tiva board) needs special treatment - See PhilsNotesOnTiva.rtf.
//
// P.J. Bones UCECE
// Last modified:  7.2.2018

// Modified by Stephanie Post and Tim Lindbom
// 
// *******************************************************


#include "inc/tm4c123gh6pm.h"  // Board specific defines (for PF0)
#include "buttons.h"

// *******************************************************
// Globals to module
// *******************************************************
static bool but_state[NUM_BUTS];	// Corresponds to the electrical state
static uint8_t but_count[NUM_BUTS];
static bool but_flag[NUM_BUTS];
static bool but_normal[NUM_BUTS];   // Corresponds to the electrical state

volatile bool UP_BUTTON_FLAG = false;
volatile bool DOWN_BUTTON_FLAG = false;
volatile bool LEFT_BUTTON_FLAG = false;
volatile bool RIGHT_BUTTON_FLAG = false;

volatile bool SWITCH1_FLAG = false;

// *******************************************************
// initButtons: Initialise the variables associated with the set of buttons
// defined by the constants in the buttons2.h header file.
void InitButtons (void)
{
    //resets peripherals as a precaution
    SysCtlPeripheralReset (UP_BUT_PERIPH);        // UP button GPIO
    SysCtlPeripheralReset (DOWN_BUT_PERIPH);      // DOWN button GPIO
    SysCtlPeripheralReset (LEFT_BUT_PERIPH);      // LEFT button GPIO
    SysCtlPeripheralReset (RIGHT_BUT_PERIPH);     // RIGHT button GPIO
	int i;

	// UP button (active HIGH)
    SysCtlPeripheralEnable (UP_BUT_PERIPH);
    GPIOPinTypeGPIOInput (UP_BUT_PORT_BASE, UP_BUT_PIN);
    GPIOPadConfigSet (UP_BUT_PORT_BASE, UP_BUT_PIN, GPIO_STRENGTH_2MA,
       GPIO_PIN_TYPE_STD_WPD);
    GPIOIntRegister(UP_BUT_PORT_BASE, ButtonsIntHandler);
    GPIOIntTypeSet(UP_BUT_PORT_BASE, UP_BUT_PIN, GPIO_FALLING_EDGE);
    GPIOIntEnable(UP_BUT_PORT_BASE, UP_BUT_PIN);
    but_normal[UP] = UP_BUT_NORMAL;


	// DOWN button (active HIGH)
    SysCtlPeripheralEnable (DOWN_BUT_PERIPH);
    GPIOPinTypeGPIOInput (DOWN_BUT_PORT_BASE, DOWN_BUT_PIN);
    GPIOPadConfigSet (DOWN_BUT_PORT_BASE, DOWN_BUT_PIN, GPIO_STRENGTH_2MA,
       GPIO_PIN_TYPE_STD_WPD);
    GPIOIntRegister(DOWN_BUT_PORT_BASE, ButtonsIntHandler);
    GPIOIntTypeSet(DOWN_BUT_PORT_BASE, DOWN_BUT_PIN, GPIO_FALLING_EDGE);
    GPIOIntEnable(DOWN_BUT_PORT_BASE, DOWN_BUT_PIN);
    but_normal[DOWN] = DOWN_BUT_NORMAL;


    // LEFT button (active LOW)
    SysCtlPeripheralEnable (LEFT_BUT_PERIPH);
    GPIOPinTypeGPIOInput (LEFT_BUT_PORT_BASE, LEFT_BUT_PIN);
    GPIOPadConfigSet (LEFT_BUT_PORT_BASE, LEFT_BUT_PIN, GPIO_STRENGTH_2MA,
       GPIO_PIN_TYPE_STD_WPU);
    GPIOIntRegister(LEFT_BUT_PORT_BASE, ButtonsIntHandler);
    //left and right buttons on the same port base
    GPIOIntTypeSet(LEFT_BUT_PORT_BASE, LEFT_BUT_PIN | RIGHT_BUT_PIN, GPIO_RISING_EDGE);
    GPIOIntEnable(LEFT_BUT_PORT_BASE, LEFT_BUT_PIN | RIGHT_BUT_PIN);
    but_normal[LEFT] = LEFT_BUT_NORMAL;


    // RIGHT button (active LOW)
    // Note that PF0 is one of a handful of GPIO pins that need to be
    // "unlocked" before they can be reconfigured.  This also requires
    // #include "inc/tm4c123gh6pm.h"
    SysCtlPeripheralEnable (RIGHT_BUT_PERIPH);
    //---Unlock PF0 for the right button:
    GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY;
    GPIO_PORTF_CR_R |= GPIO_PIN_0; //PF0 unlocked
    GPIO_PORTF_LOCK_R = GPIO_LOCK_M;
    GPIOPinTypeGPIOInput (RIGHT_BUT_PORT_BASE, RIGHT_BUT_PIN);
    GPIOPadConfigSet (RIGHT_BUT_PORT_BASE, RIGHT_BUT_PIN, GPIO_STRENGTH_2MA,
       GPIO_PIN_TYPE_STD_WPU);
    but_normal[RIGHT] = RIGHT_BUT_NORMAL;

    //SWITCH 1 for take-off and landing (active HIGH)
    SysCtlPeripheralEnable (SWITCH1_PERIPH);
    GPIOPinTypeGPIOInput (SWITCH1_PORT_BASE, SWITCH1_PIN);
    GPIOPadConfigSet (SWITCH1_PORT_BASE, SWITCH1_PIN, GPIO_STRENGTH_2MA,
       GPIO_PIN_TYPE_STD_WPD);
    //interrupt for switch up
    GPIOIntRegister(SWITCH1_PORT_BASE, SwitchIntHandler);
   GPIOIntTypeSet(SWITCH1_PORT_BASE, SWITCH1_PIN, GPIO_BOTH_EDGES);
   GPIOIntEnable(SWITCH1_PORT_BASE, SWITCH1_PIN);
    but_normal[SWITCH1] = SWITCH1_NORMAL;


	for (i = 0; i < NUM_BUTS; i++)
	{
		but_state[i] = but_normal[i];
		but_count[i] = 0;
		but_flag[i] = false;
	}


    IntEnable(INT_GPIOB);
}

// *******************************************************
// updateButtons: Function designed to be called regularly. It polls all
// buttons once and updates variables associated with the buttons if
// necessary.  It is efficient enough to be part of an ISR, e.g. from
// a SysTick interrupt.
// Debounce algorithm: A state machine is associated with each button.
// A state change occurs only after NUM_BUT_POLLS consecutive polls have
// read the pin in the opposite condition, before the state changes and
// a flag is set.  Set NUM_BUT_POLLS according to the polling rate.
void UpdateButtons (void)
{
	bool but_value[NUM_BUTS];
	int i;
	
	// Read the pins; true means HIGH, false means LOW
	but_value[UP] = (GPIOPinRead (UP_BUT_PORT_BASE, UP_BUT_PIN) == UP_BUT_PIN);
	but_value[DOWN] = (GPIOPinRead (DOWN_BUT_PORT_BASE, DOWN_BUT_PIN) == DOWN_BUT_PIN);
    but_value[LEFT] = (GPIOPinRead (LEFT_BUT_PORT_BASE, LEFT_BUT_PIN) == LEFT_BUT_PIN);
    but_value[RIGHT] = (GPIOPinRead (RIGHT_BUT_PORT_BASE, RIGHT_BUT_PIN) == RIGHT_BUT_PIN);
	// Iterate through the buttons, updating button variables as required
	for (i = 0; i < NUM_BUTS; i++)
	{
        if (but_value[i] != but_state[i])
        {
        	but_count[i]++;
        	if (but_count[i] >= NUM_BUT_POLLS)
        	{
        		but_state[i] = but_value[i];
        		but_flag[i] = true;	   // Reset by call to checkButton()
        		but_count[i] = 0;
        	}
        }
        else
        	but_count[i] = 0;
	}
}

// *******************************************************
// checkButton: Function returns the new button logical state if the button
// logical state (PUSHED or RELEASED) has changed since the last call,
// otherwise returns NO_CHANGE.
uint8_t CheckButton (uint8_t butName)
{
	if (but_flag[butName])
	{
		but_flag[butName] = false;
		if (but_state[butName] == but_normal[butName])
			return RELEASED;
		else
			return PUSHED;
	}
	return NO_CHANGE;
}


void ButtonsIntHandler()
{

        GPIOIntClear(UP_BUT_PORT_BASE, UP_BUT_PIN);
        GPIOIntClear(DOWN_BUT_PORT_BASE, DOWN_BUT_PIN);
        GPIOIntClear(LEFT_BUT_PORT_BASE, LEFT_BUT_PIN);
        GPIOIntClear(RIGHT_BUT_PORT_BASE,  RIGHT_BUT_PIN);


    if (CheckButton (UP) == PUSHED)
    {
        UP_BUTTON_FLAG = true;
    }
    if (CheckButton (DOWN) == PUSHED)
    {
        DOWN_BUTTON_FLAG = true;
    }
    if (CheckButton (LEFT) == PUSHED)
    {
        LEFT_BUTTON_FLAG = true;
    }
    if (CheckButton (RIGHT) == PUSHED)
    {
        RIGHT_BUTTON_FLAG = true;
    }
}


void SwitchIntHandler()
{
    SWITCH1_FLAG = true;

    GPIOIntClear(SWITCH1_PORT_BASE, SWITCH1_PIN);
}



