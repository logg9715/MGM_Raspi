#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <softPwm.h>

int main(void)
{
	long lightTime;
    // WiringPi 초기화
    if (wiringPiSetup() == -1)
    {
        printf("WiringPi 초기화 실패\n");
        return 1;
    }
	
	int lightSensor = 4;
	long waitTime = millis();	// 10초 타이머 시작 
		
	/* start 조도센서 읽는 부분 */
	pinMode(lightSensor, OUTPUT);	
	digitalWrite(lightSensor, 1);
	delay(1);
	pinMode(lightSensor, INPUT);
	digitalWrite(lightSensor, 0);
	lightTime = micros();
	while(digitalRead(lightSensor));
	lightTime = micros() - lightTime;

	printf("%d\n", ++lightTime); 

	return 0;
}
