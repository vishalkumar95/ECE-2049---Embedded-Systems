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
#include <stdio.h>
#include <stdlib.h>
/* Peripherals.c and .h are where the functions that implement
 * the LEDs and cap touch buttons are implemented. It is useful
 * to organize your code by putting like functions together in
 * files. You include the header associated with that file(s)
 * into the main file of your project. */
#include "peripherals.h"



// Define global variables
long stringWidth = 0;
CAP_BUTTON keypressed_state;
int columnLength = 20;
int rowLength = 8;
int yPos[10][5];
char lvl = 0;
char limit;
char aliens;
char aliensLeft;

// Function prototypes for this file
void swDelay(char numLoops);
void displayWelcomeScreen();
void CountDown();
void drawAlien();
void moveAlien();
void displayLoseScreen();
void createWave();
void resetAliens();
void CheckCapPads();
void nextlvl();
void displayWinScreen();
enum status
{
	welcome,
	countdown,
	draw,
	move,
	nextWave,
	CheckPads,
	win,
	lose

}state;

enum status state = welcome;

void main(void)
{
	P1SEL = P1SEL & ~BIT0;          // Select P1.0 for digital IO
	P1DIR |= BIT0;			// Set P1.0 to output direction
	__disable_interrupt();          // Not using interrupts so disable them

	long int count = 0;
	long int capCount = 0;
	int y1;
	int x1;
	for (x1=0; x1<5; x1++) {
		for (y1=0; y1<=9; y1++) {
			yPos[y1][x1] = 0;
		}
	}
	int n;
	int n1;

    // Define some local variables
    // Variable to record button state for later

	// Stop WDT
    WDTCTL = WDTPW | WDTHOLD;		// Stop watchdog timer

    //Perform initializations (see peripherals.c)
    configTouchPadLEDs();
    configDisplay();
    configCapButtons();

    while(1) {
		count++;
    	/* Monitor Capacitive Touch Pads in endless "forever" loop */

    	P1OUT |= (LED4+LED5+LED6+LED7+LED8);   // turn on all 5 LEDs
    	P1OUT = P1OUT ^ BIT0;

//		if ((capCount%5)==0){
//		state = nextWave;
//		}
//		capCount++;

    	switch(state) {
    	case welcome:
    		lvl =0;
        	resetAliens();
    		displayWelcomeScreen(); //perform function
    		keypressed_state = CapButtonRead();
    		if (keypressed_state == 0x01) { //next state logic
    			BuzzerFn(0);
    			swDelay(1);
    			BuzzerOff();
    			state = countdown;
    		}
    		break;
    	case countdown: //includes the function which implements the next level
    		nextlvl();
    		resetAliens();
    		CountDown();
    		state = nextWave; //next state logic
    		break;
    	case nextWave:
    		capCount = 0;
    		createWave();
    		state = move;
    		break;
    	case draw:
    		for(n1=0;n1<5;n1++) {
    			for(n=0;n<=9;n++) {
    				drawAlien(yPos[n][n1]*n, yPos[n][n1]*(n1+1));
    			}
    		}
    			state = CheckPads;
        			//state = move;
    		break;
    	case move:
    		moveAlien();
    		GrClearDisplay(&g_sContext);
    		state = draw;
    		for(n=0;n<5;n++) {
    			if (yPos[9][n]) {
    				state = lose;
    			}
    		}
    		break;
    	case CheckPads:
    			CheckCapPads();
    			state = draw;
    			if (lvl == 1) {
    				if (capCount == 5) {
    					state = nextWave;
    					BuzzerOff();
    				} else if (aliensLeft == 0) {
    					BuzzerOff();
    					state = countdown;
    				}
    			}
    			else if (lvl == 2) {
    				if (capCount == 3) {
    					state = nextWave;
    					BuzzerOff();
    				} else if (aliensLeft == 0) {
    					BuzzerOff();
    					state = countdown;
    				}
    			}
    			else if (lvl == 3) {
    				if (capCount == 1) {
    					state = nextWave;
    					BuzzerOff();
    				} else if (aliensLeft == 0) {
    					BuzzerOff();
    					state = countdown;
    				}
    			}
    			if ((aliensLeft == 0) && (lvl == 3)) {
    				state = win;
    			}
    			capCount++;
    		break;
    	case lose:
    		displayLoseScreen();
    		state = welcome; //next state logic
    		break;
    	case win:
    		displayWinScreen();
    		state = welcome;
    	}
    // Refresh the display now that we have finished writing to it
    //if ((count%2) == 0) {
    	GrFlush(&g_sContext);
    //}
    if (count == 10) {
    	count = 0;
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
    	i = 35000;					// SW Delay
   	    while (i > 0){				// could also have used while (i)
	       i--;
   	    }
    }
}
//displays welcome screen//////
void displayWelcomeScreen() {

    //Intro Screen -- Write to the display screen
	GrClearDisplay(&g_sContext);
    GrStringDrawCentered(&g_sContext, "Space Invaders", AUTO_STRING_LENGTH, 51, 16, TRANSPARENT_TEXT);
    GrStringDrawCentered(&g_sContext, "Press X to start", AUTO_STRING_LENGTH, 51, 32, TRANSPARENT_TEXT);
    //GrFlush(&g_sContext);
}
//counts down at the beggining of game////
void CountDown() {
	GrClearDisplay(&g_sContext);
	GrStringDrawCentered(&g_sContext, "3", AUTO_STRING_LENGTH, 51, 32, TRANSPARENT_TEXT);
	GrFlush(&g_sContext);
	swDelay(2);

	GrClearDisplay(&g_sContext);
	GrStringDrawCentered(&g_sContext, "2", AUTO_STRING_LENGTH, 51, 32, TRANSPARENT_TEXT);
	GrFlush(&g_sContext);
	swDelay(2);

	GrClearDisplay(&g_sContext);
	GrStringDrawCentered(&g_sContext, "1", AUTO_STRING_LENGTH, 51, 32, TRANSPARENT_TEXT);
	GrFlush(&g_sContext);
	swDelay(2);
	GrClearDisplay(&g_sContext);
}
//draws aliens to the LCD screen without refreshing it///
void drawAlien(int y, int x) {
		GrCircleDraw(&g_sContext, x*columnLength-10, 8*(y)-4, 3);
}
//moves alien down by one grid unit on LCD display and clears
void moveAlien() {
	int x;
	int y;
	for(x=4;x>=0;x--) {
		for (y=9; y>=0; y--) {
			yPos[y][x]=yPos[y-1][x];
		}
	}
		//GrClearDisplay(&g_sContext);
}
void displayLoseScreen() {
	GrStringDrawCentered(&g_sContext, "GG (Git Good)", AUTO_STRING_LENGTH, 51, 32, TRANSPARENT_TEXT);
}
void createWave() {
	int x;
	if (lvl ==1) {
		for(x=0;x<5;x++) {
			int r = (rand()%100);
			if (r>80 && (aliens != limit)) {
				yPos[0][x] = 1;
				aliens++;
			} else {
				yPos[0][x] = 0;
			}
		}
	} else if (lvl == 2) {
		for(x=0;x<5;x++) {
			int r = (rand()%100);
			if (r>60 && (aliens != limit)) {
				yPos[0][x] = 1;
				aliens++;
			} else {
				yPos[0][x] = 0;
			}
		}
	} else if (lvl == 3) {
		for(x=0;x<5;x++) {
			int r = (rand()%100);
			if (r>50 && (aliens != limit)) {
				yPos[0][x] = 1;
				aliens++;
			} else {
				yPos[0][x] = 0;
			}
		}
	}
}
void resetAliens() {
	int y1;
	int x1;
	for (x1=0; x1<5; x1++) {
		for (y1=0; y1<=9; y1++) {
			yPos[y1][x1] = 0;
		}
	}
}

void CheckCapPads(){
	keypressed_state = CapButtonRead();
    if (keypressed_state) // If any key was pressed
    {
    	// turn off LED of pressed key
    	P1OUT &= ~(keypressed_state << 1);
    }
	int y;
	if (keypressed_state == 0x01) {
		BuzzerFn(1);
		for (y=9; y>=0; y--){
			if(yPos[y][0] == 1){
				yPos[y][0] = 0;
				state = draw;
		    	GrClearDisplay(&g_sContext);
		    	aliensLeft--;
				break;
			}
		}
	}
	if (keypressed_state == 0x02) {
		BuzzerFn(2);
		for (y=9; y>=0; y--){
			if(yPos[y][1] == 1){
				yPos[y][1] = 0;
				state = draw;
		    	GrClearDisplay(&g_sContext);
		    	aliensLeft--;
				break;
			}
		}
	}
	if (keypressed_state == 0x04) {
		BuzzerFn(3);
		for (y=9; y>=0; y--){
			if(yPos[y][2] == 1){
				yPos[y][2] = 0;
				state = draw;
		    	GrClearDisplay(&g_sContext);
		    	aliensLeft--;
				break;
			}
		}
	}
	if (keypressed_state == 0x08) {
		BuzzerFn(4);
		for (y=9; y>=0; y--){
			if(yPos[y][3] == 1){
				yPos[y][3] = 0;
				state = draw;
		    	GrClearDisplay(&g_sContext);
		    	aliensLeft--;
				break;
			}
		}
	}
	if (keypressed_state == 0x10) {
		BuzzerFn(5);
		for (y=9; y>=0; y--){
			if(yPos[y][4] == 1){
				yPos[y][4] = 0;
				state = draw;
		    	GrClearDisplay(&g_sContext);
		    	aliensLeft--;
				break;
			}
		}
	}
}
void nextlvl() {
	aliens = 0;
	lvl++;
	if (lvl ==1) {
		limit = 10;
		GrClearDisplay(&g_sContext);
		GrStringDrawCentered(&g_sContext, "level 1", AUTO_STRING_LENGTH, 51, 32, TRANSPARENT_TEXT);
		GrFlush(&g_sContext);
		swDelay(2);
	} else if (lvl == 2) {
		limit = 20;
		GrClearDisplay(&g_sContext);
		GrStringDrawCentered(&g_sContext, "level 2", AUTO_STRING_LENGTH, 51, 32, TRANSPARENT_TEXT);
		GrFlush(&g_sContext);
		swDelay(2);
	} else if (lvl == 3) {
		limit = 30;
		GrClearDisplay(&g_sContext);
		GrStringDrawCentered(&g_sContext, "level 3", AUTO_STRING_LENGTH, 51, 32, TRANSPARENT_TEXT);
		GrFlush(&g_sContext);
		swDelay(2);
	}
	aliensLeft = limit;
}
void displayWinScreen() {
	GrStringDrawCentered(&g_sContext, "You win!", AUTO_STRING_LENGTH, 51, 32, TRANSPARENT_TEXT);
}


