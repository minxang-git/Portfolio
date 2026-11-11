#define F_CPU 8000000L
#include <avr/io.h>
#include <avr/interrupt.h>

#define TICKS_PER_CYCLE 64
#define TICKS_PER_MSEC F_CPU/1000/TICKS_PER_CYCLE

volatile unsigned long count = 0;

ISR(TIMER0_COMP_vect)
{
    count++;
}

void settimer()
{
    cli();
    TCCR0 = 0x00;
    TCCR0 |= (1 << WGM01);
    TCCR0 |= (1 << CS01) | (1 << CS00);
    TIMSK = 0x00;
    TIMSK |= (1 << OCIE0);
    OCR0 = TICKS_PER_MSEC - 1;
    TCNT0 = 0x00;
    sei();
}

int main()
{
    DDRC = 0xFF;
    PORTC = 0xFF;
    settimer();

    unsigned long intervalmsec = 1000;
    unsigned char ledstatuse = 0xFF;

    while(1)
    {
        ledstatuse = ~ledstatuse;
        PORTC = ledstatuse;

        while(intervalmsec > count);
        count = 0;
    }

    return 0;
}
