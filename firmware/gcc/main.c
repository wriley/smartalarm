#include "global.h"		// include our global settings

#include <avr/io.h>			// include I/O definitions (port names, pin names, etc)
#include <avr/interrupt.h>	// include interrupt support
#include <util/delay.h>

#include "uart.h"		// include uart function library
#include "rprintf.h"	// include printf function library
#include "timer.h"		// include timer function library (timing, PWM, etc)
#include "cmdline.h"	// include cmdline function library

// global variables
u08 Run;

volatile long repeatOn = 2000;
volatile long repeatOff = 1000;
volatile u08 repeatCycle = ON;
volatile long repeatCounter = 0;

volatile long pulseOn = 0;
volatile u08 pulseStart = 0;
volatile long pulseCounter = 0;

volatile long pulse2On = 0;
volatile u08 pulse2Start = 0;
volatile long pulse2Counter = 0;

volatile u08 mode = OFF;
volatile unsigned long UptimeMs = 0;

// functions
void goCmdline(void);
void statusLED(u08);
void systickHandler(void);

void helpFunction(void);
void testFunction(void);
void statusFunction(void);
void alarmFunction(void);
void cancelFunction(void);
void repeatFunction(void);
void pulseFunction(void);
void pulse2Function(void);

void alarmOn(void);
void alarmOff(void);

//----- Begin Code ------------------------------------------------------------
int main(void)
{
	// initialize our libraries
	// initialize the UART (serial port)
	uartInit();
	// set the baud rate of the UART for our debug/reporting output
	uartSetBaudRate(9600);
	// initialize the timer system
	timerInit();
	// initialize rprintf system
	rprintfInit(uartSendByte);


	// initialize systick timer
	timer2SetPrescaler(TIMERRTC_CLK_DIV1024);
	timerAttach(TIMER2OVERFLOW_INT, systickHandler);

	// set status LED pins to output
	sbi(DDRB, 1);
	sbi(DDRB, 2);

	sbi(DDRB, 0);
	cbi(PORTB, 0);

	statusLED(YELLOW);

	// start command line
	goCmdline();

	return 0;
}

void goCmdline(void)
{
	u08 c;

	rprintfProgStrM("\r\nsmartAlarm ready\r\n");

	// initialize cmdline system
	cmdlineInit();

	// direct cmdline output to uart (serial port)
	cmdlineSetOutputFunc(uartSendByte);

	// add commands to the command database
	cmdlineAddCommand("help",		helpFunction);
	cmdlineAddCommand("test",		testFunction);
	cmdlineAddCommand("status",		statusFunction);

	cmdlineAddCommand("alarm",		alarmFunction);
	cmdlineAddCommand("cancel",		cancelFunction);
	
	cmdlineAddCommand("repeat",		repeatFunction);
	cmdlineAddCommand("pulse",		pulseFunction);
	cmdlineAddCommand("pulse2",		pulse2Function);

	// send a CR to cmdline input to stimulate a prompt
	cmdlineInputFunc('\r');

	// set state to run
	Run = TRUE;

	statusLED(GREEN);
	chirp();

	// main loop
	while(Run)
	{
		// pass characters received on the uart (serial port)
		// into the cmdline processor
		while(uartReceiveByte(&c)) cmdlineInputFunc(c);

		// run the cmdline execution functions
		cmdlineMainLoop();
	}

	// we shouldn't get here normally
	rprintfProgStrM("\r\nsmartAlarm exiting!!!\r\n");
	statusLED(RED);
}

void chirp(void){
	alarmOn();
	timerPause(CHIRP_DELAY);
	alarmOff();
	timerPause(CHIRP_DELAY);
	alarmOn();
	timerPause(CHIRP_DELAY);
	alarmOff();
}

void statusLED(u08 color){
	switch(color){
		case RED:
			cbi(PORTB, 1);
			sbi(PORTB, 2);
			break;
		case YELLOW:
			sbi(PORTB, 1);
			sbi(PORTB, 2);
			break;
		case GREEN:
			sbi(PORTB, 1);
			cbi(PORTB, 2);
			break;
		default:
			cbi(PORTB, 1);
			cbi(PORTB, 2);
			break;
	}
}

void helpFunction(void)
{
	rprintfCRLF();

	rprintfProgStrM("Available commands are:\r\n");
	rprintfProgStrM("help      - displays available commands\r\n");
	rprintfProgStrM("test      - run test cycle for LED and Alarm\r\n");
	rprintfProgStrM("status    - set status LED (0)Off (1)Red (2)Yellow (3)Green\r\n");

	rprintfProgStrM("alarm     - sound continuous alarm\r\n");
	rprintfProgStrM("cancel    - cancel any alarm mode\r\n");
	rprintfProgStrM("repeat    - cycle alarm for <on>ms and <off>ms\r\n");
	rprintfProgStrM("pulse     - sound alarm for <on>ms\r\n");
	rprintfProgStrM("pulse2    - alternate alarm every second for <on>seconds\r\n");

	rprintfCRLF();
}

void testFunction(void){
	rprintfCRLF();

	rprintf("Setting statusLED to RED\r\n");
	statusLED(RED);
	timerPause(TESTPAUSE);

	rprintf("Setting statusLED to YELLOW\r\n");
	statusLED(YELLOW);
	timerPause(TESTPAUSE);

	rprintf("Setting statusLED to GREEN\r\n");
	statusLED(GREEN);
	timerPause(TESTPAUSE);

	rprintf("Turning on alarm\r\n");
	alarmOn();
	timerPause(TESTPAUSE);
	rprintf("Turning off alarm\r\n");
	alarmOff();
	timerPause(1000);
	rprintf("OK\r\n");

	rprintfCRLF();
}

void statusFunction(void){
	u08 status = cmdlineGetArgInt(1);
	if((status >= 0) && (status < 4)){
		statusLED(status);
		rprintfProgStrM("OK\r\n");
	} else {
		rprintfProgStrM("ERROR - Value out of range\r\n");
	}
}

void alarmFunction(void){
	alarmOn();
	rprintfProgStrM("OK\r\n");
}

void cancelFunction(void){
	alarmOff();
	rprintfProgStrM("OK\r\n");
}

void alarmOn(void){
	mode = ON;
}

void alarmOff(void){
	mode = OFF;
}

void repeatFunction(void){
	repeatOn = cmdlineGetArgInt(1);
	repeatOff = cmdlineGetArgInt(2);
	if((repeatOn > 0) && (repeatOff > 0)){
		repeatCycle = START;
		mode = REPEAT;
		rprintfProgStrM("OK\r\n");
	} else {
		rprintfProgStrM("ERROR - Value out of range\r\n");
	}
}

void pulseFunction(void){
	pulseOn = cmdlineGetArgInt(1);
	if(pulseOn > 0){
		pulseStart = 1;
		mode = PULSE;
		rprintfProgStrM("OK\r\n");
	} else {
		rprintfProgStrM("ERROR - Value out of range\r\n");
	}
}

void pulse2Function(void){
	pulse2On = cmdlineGetArgInt(1);
	if(pulse2On > 0){
		pulse2Start = 1;
		mode = PULSE2;
		rprintfProgStrM("OK\r\n");
	} else {
		rprintfProgStrM("ERROR - Value out of range\r\n");
	}
}

void systickHandler(void){
	// set timer for 10ms interval
	TCNT2 = (unsigned char)-TIMER_INTERVAL;
	// count up on uptime
	UptimeMs += 10;

	switch(mode){
		case OFF:
			cbi(PORTB, 0);
			break;
		case ON:
			sbi(PORTB, 0);
			break;
		case REPEAT:
			if(repeatCycle == START){
				repeatCounter = 0;
				repeatCycle = ON;
				sbi(PORTB, 0);
			} else {
				repeatCounter += 10;
			}
			
			switch(repeatCycle){
				case OFF:
					if(repeatCounter > repeatOff){
						sbi(PORTB, 0);
						repeatCounter = 0;
						repeatCycle = ON;
					}
					break;
				case ON:
					if(repeatCounter > repeatOn){
						cbi(PORTB, 0);
						repeatCounter = 0;
						repeatCycle = OFF;
					}
					break;
			}
			break;
		case PULSE:
			if(pulseStart == 1){
				sbi(PORTB, 0);
				pulseStart = 0;
				pulseCounter = 0;
			} else {
				if(pulseCounter > pulseOn){
					cbi(PORTB, 0);
				} else {
					pulseCounter += 10;
				}
			}

			break;
		case PULSE2:
			if(pulse2Start == 1){
				sbi(PORTB, 0);
				pulse2Start = 0;
				pulse2Counter = 0;
			} else {
				if(pulse2Counter > pulse2On){
					cbi(PORTB, 0);
				} else {
					if((UptimeMs % 1000) == 0){
						pulse2Counter++;
						PORTB ^= (1<<0);
					}
				}
			}
			break;
		default:
			break;
	}
}
