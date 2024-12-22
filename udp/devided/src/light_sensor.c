#include <stdio.h>
#include <pthread.h>
#include <wiringPi.h>
#include "light_sensor.h"
#include "logger.h" // logger 관련 함수 포함

#define FILENAME "logs/LightPerDay.txt" // 조도 로그 파일 경로
extern pthread_mutex_t mutex;
extern long lightTime;
extern int endStat;
extern int lightLogNum; // 전역 변수로 선언되어야 함

void *light(void *arg) {
    int lightSensor = 4;
    while (!endStat) {
        long waitTime = millis(); // 10초 타이머 시작

        /* start 조도센서 읽는 부분 */
        pinMode(lightSensor, OUTPUT);
        digitalWrite(lightSensor, 1);
        delay(1);
        pinMode(lightSensor, INPUT);
        digitalWrite(lightSensor, 0);
        pthread_mutex_lock(&mutex);
        lightTime = micros();
        while (digitalRead(lightSensor));
        lightTime = micros() - lightTime;
        /* end 조도센서 읽는 부분 */

        FILE *file;
        if (isDateChanged()) { // 날짜 바뀌면 0으로 초기화
            file = openLogFile(FILENAME, "w");
            fprintf(file, "%d", 0);
            fclose(file);
        }

        file = openLogFile(FILENAME, "r");
        if (fscanf(file, "%d", &lightLogNum) != 1) lightLogNum = 0;
        fclose(file);

        if (lightTime < 1000) {
            file = openLogFile(FILENAME, "w");
            /* log.txt, 내용에는 0만 있는 파일을 미리 생성해야 함.*/
            // 10초동안 빛이 밝았으면 1 더함, n / 6 == N분동안 빛을 받음
            fprintf(file, "%d", ++lightLogNum);
            fclose(file);
        }
        pthread_mutex_unlock(&mutex);

        while (millis() - waitTime < 10000); // 10초 타이머 종료
    }
}
