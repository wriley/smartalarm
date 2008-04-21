#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/io.h>

#define BAUDRATE 9600
#define UBRRVAL ((F_CPU/(BAUDRATE*16UL))-1)
#define SPEAKER PB0
#define ARMLED PB1

volatile uint8_t rx_int = 0;
volatile char rxbuff;

const char greeting[] __attribute__((progmem)) =
"\nsmartAlarm\n";

SIGNAL(SIG_UART_RECV)
{
  uint8_t c;

  c = UDR;
  if (bit_is_clear(UCSRA, FE))
    {
      rxbuff = c;
      rx_int = 1;
    }
}

void ioinit(){
	//Set Speaker and Armed LED pins HIGH
	PORTB |= _BV(SPEAKER);
	PORTB |= _BV(ARMLED);

	DDRB = 0xff;
	DDRC = 0xff;
	DDRD = 0xff;

	PORTB = 0x00;
	PORTC = 0x00;
	PORTD = 0x00;

	

	//Set baud rate
	UBRRL = UBRRVAL;
	UBRRH = (UBRRVAL>>8);

	//Set data frame format: asynchronous mode,no parity, 1 stop bit, 8 bit size
	UCSRC=(1<<URSEL)|(0<<UMSEL)|(0<<UPM1)|(0<<UPM0)|(0<<USBS)|(0<<UCSZ2)|(1<<UCSZ1)|(1<<UCSZ0);

	//Enable Transmitter and Receiver
	UCSRB=(1<<RXEN)|(1<<TXEN);

	//Enable interrupts
	sei();
}

void putchr( uint8_t data )
{
   while ( !( UCSRA & (1<<UDRE)) ) {};
   UDR = data;
}

static void printstr(const char *s)
{
	while (*s){
		if (*s == '\n')
			putchr('\r');
      	putchr(*s++);
	}
}

static void printstr_p(const char *s)
{
	char c;

	for (c = pgm_read_byte(s); c; ++s, c = pgm_read_byte(s)){
    	if (c == '\n')
			putchr('\r');
      	putchr(c);
    }
}

int main(){
	ioinit();
	printstr_p(greeting);

	while(1){
		if(rx_int == 1){
			printstr("Received: ");
			putchr(rxbuff);
			printstr("\n");
		}
	}

	return 0;
}
