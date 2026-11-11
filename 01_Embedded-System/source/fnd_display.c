#define F_CPU 8000000L

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

// 4자리 FND 자리 선택 데이터
unsigned char COM[4] = {
    0xFB, 0xF7, 0xEF, 0xDF
};

// 숫자별 FND 출력 데이터
unsigned char num[10] = {
    0x3F, // 0
    0x06, // 1
    0x5B, // 2
    0x4F, // 3
    0x66, // 4
    0x6D, // 5
    0x7D, // 6
    0x07, // 7
    0x7F, // 8
    0x6F  // 9
};

int numidx[4] = {0, 0, 0, 0};
unsigned long count = 0;

// 초 단위 값을 분과 초의 네 자리 숫자로 변환
inline void calnumidx(int numidx[], int cnt)
{
    if (cnt >= 3600 || cnt == 0)
    {
        numidx[0] = numidx[1] = numidx[2] = numidx[3] = 0;
        return;
    }

    int min = cnt / 60;
    int sec = cnt % 60;

    numidx[0] = sec % 10;
    numidx[1] = sec / 10;
    numidx[2] = min % 10;
    numidx[3] = min / 10;
}

// 여러 자리를 빠르게 전환하여 FND 표시
void display(int numidx[], int nseg, int delay)
{
    int k, j, iter;
    iter = delay / (nseg * 5);

    for (j = 0; j < iter; j++)
    {
        for (k = 0; k < nseg; k++)
        {
            PORTE = COM[k];       // 표시할 자리 선택
            PORTA = num[numidx[k]]; // 숫자 출력
            _delay_ms(5);
        }
    }
}

// 버튼 입력 및 채터링 방지
int get_button_state(void)
{
    if (PIND & 0x01)
    {
        _delay_ms(10);

        if (PIND & 0x01)
            return 1;
    }

    return 0;
}

// 입출력 포트 초기화
int setup()
{
    DDRA = 0xFF;
    DDRE = 0xFF;
    DDRD = 0xF0;

    _delay_ms(10);

    return get_button_state();
}

// 타이머0 오버플로우 인터럽트
ISR(TIMER0_OVF_vect)
{
    count++;
}

// 타이머0 설정
void settimer()
{
    cli();

    TCCR0 = 0x00;
    TCCR0 |= (1 << CS02);  // 256분주 설정
    TIMSK |= (1 << TOIE0); // 오버플로우 인터럽트 활성화
    TCNT0 = 0x00;

    sei();
}

int main(void)
{
    int bcurstate, bprevstate;

    bprevstate = setup();
    settimer();

    while (1)
    {
        bcurstate = get_button_state();

        // 버튼 상태가 바뀌면 카운터 초기화
        if (bcurstate != bprevstate)
        {
            count = 0;
        }

        // 일정 카운트에 도달하면 숫자 데이터 갱신
        if (count >= 100)
        {
            count = 0;
            calnumidx(numidx, count);
        }

        display(numidx, 4, 990); // FND 표시
        bprevstate = bcurstate;
    }

    return 0;
}
