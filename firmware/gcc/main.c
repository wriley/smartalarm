#include <avr/io.h>			// include I/O definitions (port names, pin names, etc)
#include <avr/interrupt.h>	// include interrupt support

#include "global.h"		// include our global settings
#include "uart.h"		// include uart function library
#include "rprintf.h"	// include printf function library
#include "timer.h"		// include timer function library (timing, PWM, etc)
#include "pulse.h"		// include pulse output support
#include "cmdline.h"	// include cmdline function library

// global variables
u08 Run;

// functions
void goCmdline(void);
void statusLED(u08);

void exitFunction(void);
void helpFunction(void);
void testFunction(void);
void freqFunction(void);
void alarmonFunction(void);
void alarmoffFunction(void);

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

	// set pulse pin to output	
	sbi(DDRD, 4);

	// set statusLED pins to output
	sbi(DDRB, 1);
	sbi(DDRB, 2);

	sbi(DDRB, 4);

	// initialize pulse library
	pulseInit();
	pulseT1ASetFreq(1500);

	timer1PWMInit(8);
	timer1PWMBOn();
	timer1PWMBSet(0);

	statusLED(ORANGE);

	// start command line
	goCmdline();

	return 0;
}

void goCmdline(void)
{
	u08 c;

	rprintf("smartAlarm ready\r\n");
	
	// initialize cmdline system
	cmdlineInit();

	// direct cmdline output to uart (serial port)
	cmdlineSetOutputFunc(uartSendByte);

	// add commands to the command database
	cmdlineAddCommand("help",		helpFunction);
	cmdlineAddCommand("exit",		exitFunction);
	cmdlineAddCommand("alarmon",	alarmonFunction);
	cmdlineAddCommand("alarmoff",	alarmoffFunction);
	cmdlineAddCommand("freq",		freqFunction);
	cmdlineAddCommand("test",		testFunction);

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

	rprintfCRLF();
	rprintf("smartAlarm exiting!!!\r\n");
	statusLED(RED);
}

void statusLED(u08 color){
	switch(color){
		case RED:
			timer1PWMBSet(0);
			sbi(PORTB, 4);
			break;
		case GREEN:
			timer1PWMBSet(255);
			cbi(PORTB, 4);
			break;
		case ORANGE:
			timer1PWMBSet(127);
			sbi(PORTB, 4);
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

	rprintf("Available commands are:\r\n");
	rprintf("help      - displays available commands\r\n");
	rprintf("exit      - shutdown smartAlarm (requires reset to resume)\r\n");
	rprintf("alarmon   - activate Alarm\r\n");
	rprintf("alarmoff  - de-activate Alarm\r\n");
	rprintf("freq      - set frequency in Hertz for Alarm\r\n");
	rprintf("test      - run test cycle for LEDs and Alarm\r\n");

	rprintfCRLF();
}

void testFunction(void){
	rprintf("Beginning test sequence\r\n");
	rprintfCRLF();

	rprintf("Setting statusLED to GREEN\r\n");
	statusLED(GREEN);
	timerPause(TESTPAUSE);

	rprintf("Setting statusLED to RED\r\n");
	statusLED(RED);
	timerPause(TESTPAUSE);

	rprintf("Setting statusLED to GREEN\r\n");
	statusLED(GREEN);
	rprintf("Turning on alarm\r\n");
	alarmOn();
	timerPause(TESTPAUSE);
	rprintf("Turning off alarm\r\n");
	alarmOff();
	rprintf("Test cycle complete\r\n");

	rprintfCRLF();
}

void alarmonFunction(void){
	alarmOn();
	rprintf("Alarm ON\r\n");
}

void alarmoffFunction(void){
	alarmOff();
	rprintf("Alarm OFF\r\n");
}

void freqFunction(void){
	u16 f = cmdlineGetArgInt(1);
	pulseT1ASetFreq(f);
	rprintf("Frequency set to %dHz\r\n", f);
}

void alarmOn(void){
	pulseT1ARun(0);
}

void alarmOff(void){
	pulseT1AStop();
}
