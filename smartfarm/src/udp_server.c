#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include "udp_server.h"
#include "actuators.h"

extern pthread_mutex_t mutex;
extern int endStat;

void *udpCon(void *arg) {
    // 소켓 관련
	int     sd;
	struct  sockaddr_in s_addr, c_addr;
	char    buff[BUFSIZ];
	int     n, n_recv;
	int     addr_len;
	int		socketTag;
	
	sd = socket(AF_INET, SOCK_DGRAM, 0);

	bzero(&s_addr, sizeof(s_addr));
	s_addr.sin_family = AF_INET;
	s_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	s_addr.sin_port = htons(SERVERPORT);

	if (bind(sd, (struct sockaddr*)&s_addr, sizeof(s_addr)) < 0)
	{
		fprintf(stderr, "bind() error");
		close(sd);
		exit(-2);
	}

	while (!endStat)
	{
		fprintf(stderr, "waiting\n");

		addr_len = sizeof(c_addr);
		if ((n_recv = recvfrom(sd, buff, sizeof(buff), 0, (struct sockaddr*)&c_addr, &addr_len)) < 0)
		{
			fprintf(stderr, "recvfrom() error");
			close(sd);
			exit(-3);
		}
		buff[n_recv] = '\0';
		
		// exit code
		if (!strcmp(buff, "quit_server\n"))	
		{
			pthread_mutex_lock(&mutex);
			fprintf(stderr, "[end] quit_server 감지됨!!!\n");
			endStat = 1;
			pthread_mutex_unlock(&mutex);
			break;
		}
		// exit code
		
		writeSocketBuff(buff[0] - '0', buff);
		
		if ((n = sendto(sd, buff, strlen(buff), 0, (struct sockaddr*)&c_addr, sizeof(c_addr))) < 0)
		{
			fprintf(stderr, "sendto() error");
			close(sd);
			exit(-3);
		}
	}
	close(sd);
}

void writeSocketBuff(int tag, char buff[BUFSIZ]) {
    /* case 1 : 센서 정보 요청 */
	if(tag == TAGSENSOR)
	{
		pthread_mutex_lock(&mutex);
		//wetStat = (wetStat) ? 0 : 1;
		
		int lightPer = (int)lightTime;
		if (lightPer <= 500) lightPer = 100;
		else if (lightPer > 3000) lightPer = 0;
		else lightPer = (int)(100.0f - (float)lightPer / 3000.0f * 100.0f);
		
		sprintf(buff, "%d,%.2f,%.2f,%d,%d", tag, cTemp, humidity, lightPer, lightLogNum/6);	
		pthread_mutex_unlock(&mutex);
	}
	
	/* case 2 : 액츄에이터 동작 명령 */
	int actuType;
	int result;
	if(tag == TAGACTUATOR)
	{
		actuType = buff[2] - '0';
		result = -1;
		
		switch (actuType) 
		{
			case ROTATER: {
				result = tmprot();
				break;
			}
			case SPRINKLER: {
				result = tmpspr();
				break;
			}
			case LED: {
				result = tmpled();
				break;
			}
		}
		sprintf(buff, "%d,%d,%d", tag, actuType, result);	
	}
}
