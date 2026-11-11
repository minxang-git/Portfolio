#define F_CPU 8000000L

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

volatile unsigned long millisec = 0;

#define TIME_INTERVAL 1

// 타이머0 비교 매치 인터럽트 발생 시 1ms 증가
ISR(TIMER0_COMP_vect)
{
    millisec++;
}

// 타이머0 설정
void settimer()
{
    cli();  // 전역 인터럽트 비활성화

    TCCR0 = 0x00;
    TCCR0 |= (1 << CS02);  // 256분주 설정
    TCCR0 |= (1 << WGM01); // CTC 모드 설정

    TIMSK = 0x00;
    TIMSK |= (1 << OCIE0); // 비교 매치 인터럽트 활성화

    OCR0 = 124;  // 약 1ms 주기 설정
    TCNT0 = 0x00;

    sei();  // 전역 인터럽트 활성화
}

// 현재까지 측정된 시간 반환
unsigned long GetMillisec()
{
    unsigned long m;

    uint8_t oldSREG = SREG; // 기존 인터럽트 상태 저장
    cli();

    m = millisec;
    SREG = oldSREG; // 기존 상태 복원

    return m;
}

int main()
{
    DDRC = 0xFF;   // PORTC 출력 설정
    PORTC = 0xFF;  // LED 초기 점등

    settimer();

    unsigned long prevtime, curtime;

    prevtime = GetMillisec();

    while (1)
    {
        curtime = GetMillisec();

        // 1초가 경과하면 LED 상태 반전
        if ((curtime - prevtime) > 1000)
        {
            prevtime = curtime;
            PORTC = ~PORTC;
        }
    }

    return 0;
}
