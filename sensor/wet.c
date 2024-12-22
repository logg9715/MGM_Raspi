#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    // WiringPi 초기화
    if (wiringPiSetup() == -1)
    {
        printf("WiringPi 초기화 실패\n");
        return 1;
    }

    // GPIO 4번 핀을 입력으로 설정
    int pin =0; // WiringPi에서 GPIO 4는 핀 7에 매핑됨
    pinMode(pin, INPUT);
    // 디지털 신호 읽기
    int pinRead = digitalRead(pin);

    if (pinRead == HIGH)
    {
        printf("GPIO 4번 핀: HIGH\n");
    }
    else
    {
        printf("GPIO 4번 핀: LOW\n");
    }

    return 0;
}
