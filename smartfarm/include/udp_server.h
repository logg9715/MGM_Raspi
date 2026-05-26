#ifndef UDP_SERVER_H
#define UDP_SERVER_H

#include <pthread.h>

#define SERVERPORT 9200
#define TAGSENSOR 0
#define TAGACTUATOR 1
#define ROTATER 0
#define SPRINKLER 1
#define LED 2

extern pthread_mutex_t mutex;
extern int endStat;
extern float cTemp, humidity;
extern long lightTime;
extern int lightLogNum;

void *udpCon(void *arg);
void writeSocketBuff(int tag, char buff[BUFSIZ]);

#endif
