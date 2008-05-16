#include "defines.h"

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>

#include <avr/io.h>
#include <avr/pgmspace.h>

#include <util/delay.h>

#include "uart.h"

static void ioinit(void)
{
	uart_init();
}

FILE uart_str = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

int main(void)
{
	char buf[20];
	int i = 0;

	ioinit();

	stdout = stdin = &uart_str;

	PORTB &= ~(_BV(ARMLED));
	fprintf(stdout, "Alarm ready\n");

	for (;;){
		if (fgets(buf, sizeof buf - 1, stdin) == NULL)
			break;

		switch (tolower(buf[0])){
			default:
			printf("Unknown command: %s\n", buf);
			break;

			case 'a':
				if (sscanf(buf, "%*c%d", &i) > 0){
					switch (i){
						case '0':
							printf("Alarm Off\n");
							PORTB |= _BV(SPEAKER);
							break;
						case '1':
							printf("Alarm On\n");
							PORTB &= ~(_BV(SPEAKER));
							break;
					}
				} else {
					printf("sscanf() failed\n");
				}
				break;
			case 'm':
				if (sscanf(buf, "%*c%d", &i) > 0){
					printf("Mode set to %d\n", i);
				} else {
					printf("sscanf() failed\n");
				}
				break;
		}
	}

	return 0;
}
