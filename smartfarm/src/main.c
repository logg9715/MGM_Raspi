#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include "udp_server.h"
#include "sht30_sensor.h"
#include "light_sensor.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int endStat = 0;
float cTemp = 0, humidity = 0;
long lightTime = 0;
int lightLogNum = 0;

void init() {
    if (wiringPiSetup() == -1) {
        perror("WiringPi 초기화 실패\n");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    init();

    pthread_t threads[3];
    pthread_create(&threads[0], NULL, udpCon, NULL);
    pthread_create(&threads[1], NULL, sht30, NULL);
    pthread_create(&threads[2], NULL, light, NULL);

    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    return 0;
}
