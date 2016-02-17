/* --COPYRIGHT--,BSD
 * Copyright (c) 2012, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/

/**************************************************************************************/
/* This program demonstrates some of the features of the TI MSP EXP430F5529 Experimenter
 * Board that we will be using in lab this term. Specifically it demonstrates the use
 * of the chip-specific graphics library as well as the capacitive touch library.
 * This demo code is based off some of the TI-supplied example code hence the retention
 * of the legal header stuff above.
 *
 * This is the type of informative comment block that you should include at the
 * beginning of any program or function that you write.
 *
 * Inputs: None
 * Outputs None
 *
 * Written by : Susan Jarvis
 * 				ECE2049
 * 				25 Aug 2013
 *
 * Modified by : Nick DeMarinis, Jan 2014, Aug 2014, Jan 2015
 */

#include <msp430.h>
#include <stdint.h>
#include "inc\hw_memmap.h"
#include "driverlibHeaders.h"
#include "CTS_Layer.h"

#include "grlib.h"
#include "LcdDriver/Dogs102x64_UC1701.h"

/* Peripherals.c and .h are where the functions that implement
 * the LEDs and cap touch buttons are implemented. It is useful
 * to organize your code by putting like functions together in
 * files. You include the header associated with that file(s)
 * into the main file of your project. */
#include "peripherals.h"

// Define global variables
long unsigned int timer = 0;
int count;
int BPM = 120;
int counttolose;
int erik = 0;

// Implementing interrupts for the timer
#pragma vector = TIMER2_A0_VECTOR
interrupt void Timer_A2_ISR(void)
{
	timer++;

	if (timer == 5)
		count = 3;
	if (timer == 200)
		count = 2;
	if (timer == 400)
		count = 1;
	if (timer == 600)
		count = 0;
}
// song 1
pitch notesong[26] = {E1, E1, E1, E1, E1, E1, E1, G1, c1, D1, E1, F1, F1, F1, F1, F1, E1, E1, E1, E1, E1, D1, D1, E1, D1, G1};
duration durationsong[26] = {q, q, h, q, q, h, q, q, q, q, w, q, q, q, q, q, q, q, e, e, q, q, q, q, h, h};

// song 2
pitch newsong[28] = {c1, c1, G1, G1, A1, A1, G1, F1, F1, E1, E1, D1, D1, c1, G1, G1, F1, F1, E1, E1, D1, G1, G1, F1, F1, E1, E1, D1};
duration dursong[28] = {q, q, q, q, q, q, h, q, q, q, q, q, q, h, q, q, q, q, q, q, h, q, q, q, q, q, q, h};

CAP_BUTTON keypressed_state;

// Function prototypes for this file
void swDelay(char numLoops);
void configLED1_3(char inbits);
int button_config();
void displayWelcomeScreen();
void CountDown();
void TimerA2(void);
void LoseScreen();
void WinScreen();
void CheckCapPads(pitch inpitch);
void CheckLEDs(pitch inpitch);
void CountDownSong1();
void CountDownSong2();

// enumerating the state machine
enum status{
	welcome,
	countdown,
	choosesong,
	playsong1,
	playsong2,
	CapPads,
	lose,
	win
}
state;
enum status state = welcome;

// main loop
void main(void)
{
	// Stop WDT
	WDTCTL = WDTPW | WDTHOLD;		// Stop watchdog timer

	//Perform initializations (see peripherals.c)
	configTouchPadLEDs();
	configDisplay();
	configCapButtons();

	_BIS_SR(GIE); // Globally enable interrupts

	while(1)
	{
		// local variables
		int pressed = button_config();
		int i;

		// state machine
		switch(state){
		case welcome:
			displayWelcomeScreen(); //perform function
			if (pressed == 1) {
				state = countdown;
			}
			break;
		case countdown:
			timer = 0;
			TimerA2();             //perform function
			CountDown();          //perform function
			state = choosesong;
			break;
		case choosesong:
			keypressed_state = CapButtonRead();
			if (keypressed_state == 0x01){
				timer = 0;
				TimerA2();             //perform function
				CountDownSong1();
				state = playsong1;
			}
			if (keypressed_state == 0x10){
				timer = 0;
				TimerA2();             //perform function
				CountDownSong2();          //perform function
				state = playsong2;
			}
			break;
		case playsong1:
			counttolose = 0;
			state = win;
			for (i = 0 ; i < 26; i++){
				if (counttolose >= 15) {
					state = lose;
					break;
				}
				CheckLEDs(notesong[i]);        //perform function
				BuzzerFn(notesong[i]);        //perform function
				timer = 0;
				while(1) {
					CheckCapPads(notesong[i]);
					if ((timer >= (durationsong[i] * 200) / BPM) & (timer <= (durationsong[i] * 200) / BPM + 3)){
						BuzzerOff();          //perform function
						P1OUT &= ~(LED4 | LED5 | LED6 | LED7 | LED8);
						timer = 0;
						if (erik == 1)
							counttolose++;
						if (erik == 2)
							counttolose++;
						if (erik == 3)
							counttolose++;
						if (erik == 4)
							counttolose++;
						if (erik == 5)
							counttolose++;
						while (1) {
							if (timer == 50)
								timer= 0;
							break;
						}
						break;
					}
				}
			}
			break;
		case playsong2:
			counttolose = 0;
			state = win;
			for (i = 0 ; i < 28; i++){
				if (counttolose >= 15) {
					state = lose;
					break;
				}
				CheckLEDs(newsong[i]);        //perform function
				BuzzerFn(newsong[i]);        //perform function
				timer = 0;
				while(1) {
					CheckCapPads(newsong[i]);
					if ((timer >= (dursong[i] * 200) / BPM) & (timer <= (dursong[i] * 200) / BPM + 3)){
						BuzzerOff();          //perform function
						P1OUT &= ~(LED4 | LED5 | LED6 | LED7 | LED8);
						timer = 0;
						if (erik == 1)
							counttolose++;
						if (erik == 2)
							counttolose++;
						if (erik == 3)
							counttolose++;
						if (erik == 4)
							counttolose++;
						if (erik == 5)
							counttolose++;
						while (1) {
							if (timer == 50)
								timer= 0;
							break;
						}
						break;
					}
				}
			}
			break;
		case win:
			WinScreen();             //perform function
			state = welcome;
			break;
		case lose:
			LoseScreen();           //perform function
			state = welcome;
			break;
		}
		if (pressed == 0) {
			configLED1_3(BIT0);
		} else if (pressed == 1) {
			configLED1_3(BIT1);
		} else if (pressed == 2) {
			configLED1_3(BIT2);
		} else if (pressed == 3) {
			configLED1_3(BIT3);
		}

	}
}

void swDelay(char numLoops)
{
	// This function is a software delay. It performs
	// useless loops to waste a bit of time
	//
	// Input: numLoops = number of delay loops to execute
	// Output: none
	//
	// smj, ECE2049, 25 Aug 2013

	volatile unsigned int i,j;	// volatile to prevent optimization
	// by compiler

	for (j=0; j<numLoops; j++)
	{
		i = 50000 ;					// SW Delay
		while (i > 0)				// could also have used while (i)
			i--;
	}
}

// Configure and light LEDs 1,2 and 3 based on the char argument passed
void configLED1_3(char inbits){
	// Setting up selection registers
	P1SEL &= ~(BIT0);
	P8SEL &= ~(BIT2|BIT1);

	// Setting up direction registers
	P1DIR |= (BIT0);
	P8DIR |= (BIT2|BIT1);

	// determine if the LEDs light or not
	if ((inbits & BIT1) == (BIT1)){
		P1OUT |= BIT0;
		P8OUT &= ~(BIT1);
		P8OUT &= ~(BIT2);
	}
	if ((inbits & BIT2) == (BIT2)){
		P8OUT |= BIT1;
		P1OUT &= ~(BIT0);
		P8OUT &= ~(BIT2);
	}
	if ((inbits & BIT3) == (BIT3)){
		P8OUT |= BIT2;
		P1OUT &= ~(BIT0);
		P8OUT &= ~(BIT1);
	}
	if ((inbits & BIT0) == (BIT0)) {
		P1OUT &= ~(BIT0);
		P8OUT &= ~(BIT1);
		P8OUT &= ~(BIT2);
	}
	if ((inbits & BIT4) == (BIT4)) {
		P1OUT |= (BIT0);
		P8OUT |= (BIT1);
		P8OUT |= (BIT2);
	}
}

// Configuring buttons
int button_config(){
	int buttonpressed = 0;
	// Setting up selection registers
	P1SEL &= ~(BIT7);
	P2SEL &= ~(BIT2);

	// Setting up direction registers
	P1DIR &= ~(BIT7);
	P2DIR &= ~(BIT2);

	// Enabling pull-up resistor
	P1REN |= (BIT7);
	P2REN |= (BIT2);

	// Using POUT to pull up or pull down the resistor
	P1OUT &= (BIT7);
	P2OUT &= (BIT2);

	// Returning based on the button pressed
	if (!(P1IN & BIT7)){
		buttonpressed = 1;
	}
	if (!(P2IN & BIT2)){
		buttonpressed = 2;
	}
	if (!(P1IN & BIT7) & !(P2IN & BIT2)) {
		buttonpressed = 3;
	}
	return buttonpressed;
}

//displays welcome screen//
void displayWelcomeScreen() {

	//Introduction Screen -- Write to the display screen
	GrClearDisplay(&g_sContext);
	GrStringDrawCentered(&g_sContext, "MSP430 Hero", AUTO_STRING_LENGTH, 51, 16, TRANSPARENT_TEXT);
	GrStringDrawCentered(&g_sContext, "Press S1 to start", AUTO_STRING_LENGTH, 51, 32, TRANSPARENT_TEXT);
	GrFlush(&g_sContext);
}

//counts down at the beginning of game//
void CountDown() {
	int done = 0;
	count =3;
	while (!(done)) {
		if (count == 3) {
			GrClearDisplay(&g_sContext);
			configLED1_3(BIT1);
			GrStringDrawCentered(&g_sContext, "3", AUTO_STRING_LENGTH, 51, 32, TRANSPARENT_TEXT);
			GrFlush(&g_sContext);
		}
		if (count == 2) {
			GrClearDisplay(&g_sContext);
			configLED1_3(BIT2);
			GrStringDrawCentered(&g_sContext, "2", AUTO_STRING_LENGTH, 51, 32, TRANSPARENT_TEXT);
			GrFlush(&g_sContext);
		}
		if (count == 1) {
			GrClearDisplay(&g_sContext);
			configLED1_3(BIT3);
			GrStringDrawCentered(&g_sContext, "1", AUTO_STRING_LENGTH, 51, 32, TRANSPARENT_TEXT);
			GrFlush(&g_sContext);
		}
		if (count == 0) {
			GrClearDisplay(&g_sContext);
			configLED1_3(BIT4);
			GrStringDrawCentered(&g_sContext, "Choose Song!", AUTO_STRING_LENGTH, 51, 15, TRANSPARENT_TEXT);
			GrStringDrawCentered(&g_sContext, "X: Jingle Bells", AUTO_STRING_LENGTH, 51, 25, TRANSPARENT_TEXT);
			GrStringDrawCentered(&g_sContext, "O: Twinkle star", AUTO_STRING_LENGTH, 51, 35, TRANSPARENT_TEXT);
			GrFlush(&g_sContext);
			GrClearDisplay(&g_sContext);
			done = 1;
		}
	}

}

//counts down at the choosing of the song
void CountDownSong1() {
	int done = 0;
	count =3;
	while (!(done)) {
		if (count == 3) {
			GrClearDisplay(&g_sContext);
			configLED1_3(BIT1);
			GrStringDrawCentered(&g_sContext, "3", AUTO_STRING_LENGTH, 51, 32, TRANSPARENT_TEXT);
			GrFlush(&g_sContext);
		}
		if (count == 2) {
			GrClearDisplay(&g_sContext);
			configLED1_3(BIT2);
			GrStringDrawCentered(&g_sContext, "2", AUTO_STRING_LENGTH, 51, 32, TRANSPARENT_TEXT);
			GrFlush(&g_sContext);
		}
		if (count == 1) {
			GrClearDisplay(&g_sContext);
			configLED1_3(BIT3);
			GrStringDrawCentered(&g_sContext, "1", AUTO_STRING_LENGTH, 51, 32, TRANSPARENT_TEXT);
			GrFlush(&g_sContext);
		}
		if (count == 0) {
			GrClearDisplay(&g_sContext);
			configLED1_3(BIT4);
			GrStringDrawCentered(&g_sContext, "Jingle Bells!", AUTO_STRING_LENGTH, 51, 32, TRANSPARENT_TEXT);
			GrFlush(&g_sContext);
			GrClearDisplay(&g_sContext);
			done = 1;
		}
	}

}

//counts down at the choosing of the song
void CountDownSong2() {
	int done = 0;
	count =3;
	while (!(done)) {
		if (count == 3) {
			GrClearDisplay(&g_sContext);
			configLED1_3(BIT1);
			GrStringDrawCentered(&g_sContext, "3", AUTO_STRING_LENGTH, 51, 32, TRANSPARENT_TEXT);
			GrFlush(&g_sContext);
		}
		if (count == 2) {
			GrClearDisplay(&g_sContext);
			configLED1_3(BIT2);
			GrStringDrawCentered(&g_sContext, "2", AUTO_STRING_LENGTH, 51, 32, TRANSPARENT_TEXT);
			GrFlush(&g_sContext);
		}
		if (count == 1) {
			GrClearDisplay(&g_sContext);
			configLED1_3(BIT3);
			GrStringDrawCentered(&g_sContext, "1", AUTO_STRING_LENGTH, 51, 32, TRANSPARENT_TEXT);
			GrFlush(&g_sContext);
		}
		if (count == 0) {
			GrClearDisplay(&g_sContext);
			configLED1_3(BIT4);
			GrStringDrawCentered(&g_sContext, "Twinkle Star!", AUTO_STRING_LENGTH, 51, 32, TRANSPARENT_TEXT);
			GrFlush(&g_sContext);
			GrClearDisplay(&g_sContext);
			done = 1;
		}
	}

}

// checking touch pads for the game
void CheckCapPads(pitch inpitch){
	keypressed_state = CapButtonRead();
	erik = 0;
	if ((inpitch == A1) | (inpitch == B1)){
		if (keypressed_state != 0x01){
			erik = 1;
		}
	}
	if ((inpitch == c1) | (inpitch == D1)){
		if (keypressed_state != 0x02){
			erik = 2;
		}
	}
	if (inpitch == E1){
		if (keypressed_state != 0x04){
			erik = 3;
		}
	}
	if (inpitch == F1){
		if (keypressed_state != 0x08){
			erik = 4;
		}
	}
	if (inpitch == G1){
		if (keypressed_state != 0x10){
			erik = 5;
		}
	}
}

// checking LEDs for the game
void CheckLEDs(pitch inpitch){

	if ((inpitch == A1) | (inpitch == B1)){
		P1OUT |= LED4;
	}
	if ((inpitch == c1) | (inpitch == D1)){
		P1OUT |= LED5;
	}
	if (inpitch == E1){
		P1OUT |= LED6;
	}
	if (inpitch == F1){
		P1OUT |= LED7;
	}
	if (inpitch == G1){
		P1OUT |= LED8;
	}
}

// displaying lose screen
void LoseScreen(){
	GrClearDisplay(&g_sContext);
	GrStringDrawCentered(&g_sContext, "You lose!", AUTO_STRING_LENGTH, 51, 32, TRANSPARENT_TEXT);
	GrFlush(&g_sContext);
}

// displaying win screen
void WinScreen(){
	GrClearDisplay(&g_sContext);
	GrStringDrawCentered(&g_sContext, "You Win!", AUTO_STRING_LENGTH, 51, 32, TRANSPARENT_TEXT);
	GrFlush(&g_sContext);
}

// implementing timer for the program
void TimerA2(void)
{
	TA2CTL = TASSEL_1 + ID_0 + MC_1;
	TA2CCR0 = 163; // 1/200 seconds
	TA2CCTL0 = CCIE;
}

