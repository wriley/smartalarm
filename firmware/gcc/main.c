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
volatile u16 repeatOn = 2000;
volatile u16 repeatOff = 1000;
volatile u08 repeatCycle = ON;
volatile u16 repeatCounter = 0;
volatile u16 pulseOn = 0;
volatile u08 pulseStart = 0;
volatile u16 pulseCounter = 0;
volatile u08 mode = OFF;
volatile u08 lastmode = OFF;
volatile u16 UptimeMs = 0;

// functions
void goCmdline(void);
void statusLED(u08);
void systickHandler(void);

void exitFunction(void);
void helpFunction(void);
void testFunction(void);
void alarmFunction(void);
void repeatFunction(void);
void pulseFunction(void);

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
	cmdlineAddCommand("exit",		exitFunction);
	cmdlineAddCommand("alarm",		alarmFunction);
	cmdlineAddCommand("test",		testFunction);
	cmdlineAddCommand("repeat",		repeatFunction);
	cmdlineAddCommand("pulse",		pulseFunction);

	// send a CR to cmdline input to stimulate a prompt
	cmdlineInputFunc('\r');

	// set state to run
	Run = TRUE;

	statusLED(GREEN);

	// main loop
	while(Run)
	{
		// pass characters received on the uart (serial port)
		// into the cmdline processor
		while(uartReceiveByte(&c)) cmdlineInputFunc(c);

		// run the cmdline execution functions
		cmdlineMainLoop();
	}

	rprintfProgStrM("\r\nsmartAlarm exiting!!!\r\n");
	statusLED(RED);
}

void statusLED(u08 color){
	switch(color){
		case RED:
			cbi(PORTB, 1);
			sbi(PORTB, 2);
			break;
		case GREEN:
			sbi(PORTB, 1);
			cbi(PORTB, 2);
			break;
		case YELLOW:
			cbi(PORTB, 1);
			cbi(PORTB, 2);
			break;
		default:
			break;
	}
}

void exitFunction(void)
{
	alarmOff();
	// to exit, we set Run to FALSE
	Run = FALSE;
}

void helpFunction(void)
{
	rprintfCRLF();

	rprintfProgStrM("Available commands are:\r\n");
	rprintfProgStrM("help      - displays available commands\r\n");
	rprintfProgStrM("exit      - shutdown smartAlarm (requires reset to resume)\r\n");
	rprintfProgStrM("alarm     - set alarm on (1) or off (0)\r\n");
	rprintfProgStrM("test      - run test cycle for LEDs and Alarm\r\n");
	rprintfProgStrM("repeat    - sound alarm for <on>ms and <off>ms (optional duration <ms>)\r\n");
	rprintfProgStrM("pulse     - sound alarm for <on>ms\r\n");

	rprintfCRLF();
}

void testFunction(void){
	rprintfCRLF();

	rprintf("Setting statusLED to GREEN\r\n");
	statusLED(GREEN);
	timerPause(TESTPAUSE);

	rprintf("Setting statusLED to RED\r\n");
	statusLED(RED);
	timerPause(TESTPAUSE);

	rprintf("Setting statusLED to YELLOW\r\n");
	statusLED(YELLOW);
	timerPause(TESTPAUSE);

	statusLED(GREEN);
	rprintf("Turning on alarm\r\n");
	alarmOn();
	timerPause(TESTPAUSE);
	rprintf("Turning off alarm\r\n");
	alarmOff();
	timerPause(1000);
	rprintf("Test cycle complete\r\n");

	rprintfCRLF();
}

void alarmFunction(void){
	u08 s = cmdlineGetArgInt(1);
	switch(s){
		case 0:
			alarmOff();
			rprintf("OK - mode = %d\r\n", mode);
			break;
		case 1:
			alarmOn();
			rprintf("OK - mode = %d\r\n", mode);
			break;
	}
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
	repeatDuration = cmdlineGetArgInt(3);
	repeatCycle = START;
	mode = REPEAT;
	rprintf("OK - mode = %d\r\n", mode);
}

void pulseFunction(void){
	pulseOn = cmdlineGetArgInt(1);
	pulseStart = 1;
	mode = PULSE;
	rprintf("OK - mode = %d\r\n", mode);
}

void systickHandler(void){
	// set timer for 10ms interval
	TCNT2 = (unsigned char)-TIMER_INTERVAL;
	// count up on uptime
	UptimeMs += 10;

	switch(mode){
		case OFF:
			if(lastmode != mode){
				cbi(PORTB, 0);
				sbi(PORTC, 1);
				sbi(PORTC, 2);
			}
			break;
		case ON:
			if(lastmode != mode){
				sbi(PORTB, 0);
				cbi(PORTC, 1);
			}
			break;
		case REPEAT:
			if(repeatCycle == START){
				repeatCounter = 0;
				repeatCycle = ON;
				sbi(PORTB, 0);
				sbi(PORTC, 1);
				cbi(PORTC, 2);
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
		case SONG:
			break;
		default:
			rprintf("Uknown mode!: %d\r\n", mode);
			break;
	}
	if(lastmode != mode){
		lastmode = mode;
	}					
}
