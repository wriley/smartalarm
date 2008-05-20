#ifndef GLOBAL_H
#define GLOBAL_H

// global AVRLIB defines
#include "avrlibdefs.h"
// global AVRLIB types definitions
#include "avrlibtypes.h"

// project/system dependent defines

// CPU clock speed
//#define F_CPU        16000000               		// 16MHz processor
#define F_CPU        12000000               		// 12MHz processor
//#define F_CPU        14745000               		// 14.745MHz processor
//#define F_CPU        8000000               		// 8MHz processor
//#define F_CPU        7372800               		// 7.37MHz processor
//#define F_CPU        4000000               		// 4MHz processor
//#define F_CPU        3686400               		// 3.69MHz processor
#define CYCLES_PER_US ((F_CPU+500000)/1000000) 	// cpu cycles per microsecond

// Status LED colors
#define RED 0
#define YELLOW 1
#define GREEN 2

// Alarm modes
#define OFF 0
#define ON 1
#define REPEAT 2
#define PULSE 3
#define SONG 4

// for repeat mode
#define START 2

// ms to pause between tests in test cycle
#define TESTPAUSE 3000

// timer defines
#define TIMER_PRESCALE		1024
#define TIMER_INTERVAL		(F_CPU/TIMER_PRESCALE/100)	// 100ms interval


#endif
