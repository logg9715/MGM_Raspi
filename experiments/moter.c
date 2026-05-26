#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <softPwm.h>

int main(void)
{
	int pin1 = 26;
	
    // WiringPi 초기화
    if (wiringPiSetup() == -1)
    {
        printf("WiringPi 초기화 실패\n");
        return 1;
    }


	pinMode (pin1, OUTPUT);
	softPwmCreate(pin1, 0, 128);
	
	softPwmWrite(pin1, 70);
	
	delay(5000);
	
	softPwmWrite(pin1, 0);
	digitalWrite(pin1,0);
	
    return 0;
}
