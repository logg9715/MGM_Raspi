#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// pin : 23 24 25

int main(int argc, char* argv[])
{
	
    // WiringPi 초기화
    if (wiringPiSetup() == -1)
    {
        printf("WiringPi 초기화 실패\n");
        return 1;
    }

	pinMode (25, OUTPUT);
	digitalWrite(25, 1);
	
	delay(2000);
	
	digitalWrite(25, 0);

    return 0;
}
