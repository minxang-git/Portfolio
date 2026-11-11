
#define F_CPU 8000000L

#include <avr/io.h>
#include <util/delay.h>

// 8x8 도트 매트릭스의 하트 모양 데이터
// 각 비트는 한 행의 LED 점등 위치를 의미
unsigned char heart[8] = {
    0b00000000,
    0b01100110,
    0b11111111,
    0b11111111,
    0b11111111,
    0b01111110,
    0b00111100,
    0b00011000
};

// 도트 매트릭스 출력 포트 초기화
void matrix_init()
{
    DDRA = 0xFF;  // 행 선택 포트 출력 설정
    DDRC = 0xFF;  // 열 데이터 포트 출력 설정

    PORTA = 0x00;
    PORTC = 0xFF;
}

// 도트 매트릭스 한 행 출력
void display_row(int row)
{
    PORTA = (1 << row);  // 출력할 행 선택
    PORTC = ~heart[row]; // 해당 행의 LED 데이터 출력
}

// 하트 모양을 반복 출력
void display_heart()
{
    int row;

    for (row = 0; row < 8; row++)
    {
        display_row(row);
        _delay_ms(2);  // 잔상 효과를 위한 짧은 대기
    }
}

// 메인 함수
int main(void)
{
    matrix_init();

    while (1)
    {
        display_heart(); // 하트 모양 반복 표시
    }

    return 0;
}
