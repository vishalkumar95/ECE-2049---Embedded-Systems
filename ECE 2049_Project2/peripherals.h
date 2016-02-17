/*
 * peripherals.h
 *
 *  Created on: Jan 29, 2014
 *      Author: ndemarinis
 */

#ifndef PERIPHERALS_H_
#define PERIPHERALS_H_

#include <msp430.h>
#include <stdint.h>
#include "inc\hw_memmap.h"
#include "driverlibHeaders.h"
#include "CTS_Layer.h"

#include "grlib.h"
#include "LcdDriver/Dogs102x64_UC1701.h"

#define NUM_KEYS	5
#define LED4		BIT5
#define LED5		BIT4
#define LED6		BIT3
#define LED7		BIT2
#define LED8		BIT1


// You may find it useful to define a new type for representing the
// capacitive buttons. Since we have fewer than 8 buttons, we can use
// a char which will allow one bit to represent the state of each button.
// Which corresponds to which button?  Find out using the debugger and
// write #define statements for each button
typedef uint8_t CAP_BUTTON;

#define BUTTON_NONE 0x00   // No buttons pressed
// Add definitions here...

/*some notes for the thing*/
typedef enum note{
	R,
	A0,
	B0,
	c0,
	D0,
	E0,
	F0,
	G0,
	A1,
	B1,
	c1,
	D1,
	E1,
	F1,
	G1,
	A2
}pitch;

typedef enum noteduration{
			w = 240,
			h = 120,
			q = 60,
			e = 30,
			s = 15
		}duration;


// Globals
extern tContext g_sContext;	// user defined type used by graphics library

// Prototypes for functions defined implemented in peripherals.c
void configDisplay(void);
void configTouchPadLEDs(void);
void BuzzerOn(void);
void BuzzerOff(void);
void BuzzerFn(pitch x);
void configCapButtons(void);
CAP_BUTTON CapButtonRead(void);



#endif /* PERIPHERALS_H_ */
