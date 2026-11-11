#define F_CPU 8000000L

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define TICKS_PER_CYCLE 64
#define TICKS_PER_MSEC F_CPU / 1000 / TICKS_PER_CYCLE

volatile unsigned long count = 0;

// 타이머0 비교 매치 인터럽트 발생 시 호출
ISR(TIMER0_COMP_vect)
{
    count++;
}

// 타이머0을 CTC 모드로 설정
void settimer()
{
    cli();  // 타이머 설정 중 인터럽트 비활성화

    TCCR0 = 0x00;
    TCCR0 |= (1 << WGM01);  // CTC 모드 설정
    TCCR0 |= (1 << CS01) | (1 << CS00);  // 64분주 설정

    TIMSK = 0x00;
    TIMSK |= (1 << OCIE0);  // 비교 매치 인터럽트 활성화

    OCR0 = TICKS_PER_MSEC - 1;
    TCNT0 = 0x00;  // 카운터 초기화

    sei();  // 전역 인터럽트 활성화
}

int main()
{
    DDRC = 0xFF;   // PORTC를 출력으로 설정
    PORTC = 0xFF;  // LED 초기 상태

    settimer();

    unsigned long intervalmsec = 1000;
    unsigned char ledstatuse = 0xFF;

    while (1)
    {
        ledstatuse = ~ledstatuse;  // LED 상태 반전
        PORTC = ledstatuse;

        // 설정한 시간이 지날 때까지 대기
        while (intervalmsec > count);

        count = 0;  // 시간 카운터 초기화
    }

    return 0;
}
