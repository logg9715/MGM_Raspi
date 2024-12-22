//udpServer.c

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <pthread.h>
#include <wiringPi.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <softPwm.h>

#define THREADSIZE 3
#define UDPCON 0
#define SHT30 1
#define LIGHT 2

// 조도센서 시간/날짜 관련 페이크DB
#define FILENAME "logs/LightPerDay.txt"
#define FILENAME_DATE "logs/Time.txt"

// serverPort
#define SERVERPORT 9200

// packet
#define TAGSENSOR 0
#define TAGACTUATOR 1

// actuator
#define ROTATER 0
#define SPRINKLER 1
#define LED 2

pthread_mutex_t mutex=PTHREAD_MUTEX_INITIALIZER;

//int wetStat;
int endStat;
float cTemp;
float humidity;
long lightTime;
int lightLogNum;

//void *wet(void *arg);
void *udpCon(void *arg);
void *sht30(void *arg);
void *light(void *arg);

int isDateChanged();
void writeSocketBuff(int tag, char buff[BUFSIZ]);
FILE *openLogFile(char *fileName, char *mode);
void init();

// 액추에이터 함수
int tmprot();
int tmpspr();
int tmpled();

main(int argc, char* argv[])
{
	char cmdBuffer[BUFSIZ];
	
	init();
	
	// 스레드 관련
	int thread;
	int sts;
	pthread_t p_thread[THREADSIZE];
	
	// 스레드 : 
	thread=pthread_create(&p_thread[UDPCON],NULL,udpCon,NULL);
	thread=pthread_create(&p_thread[SHT30],NULL,sht30,NULL);
	thread=pthread_create(&p_thread[LIGHT],NULL,light,NULL);
	
	pthread_join(p_thread[UDPCON],(void**)&sts);
	pthread_join(p_thread[SHT30],(void**)&sts);
	pthread_join(p_thread[LIGHT],(void**)&sts);

	sts=pthread_mutex_destroy(&mutex);
}

void init()
{
	// WiringPi 초기화
    if(wiringPiSetup() == -1)
    {
		perror("WiringPi 초기화 실패\n");
		exit(EXIT_FAILURE);
    }	
}

void *udpCon(void *arg)
{
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

void writeSocketBuff(int tag, char buff[BUFSIZ])
{
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

void *sht30(void *arg) 
{
    // Create I2C bus
    int file;
    char *bus = "/dev/i2c-1";
    if ((file = open(bus, O_RDWR)) < 0) 
    {
        printf("Failed to open the bus. \n");
        exit(1);
    }
    
    // Get I2C device, SHT30 I2C address is 0x44(68)
    if (ioctl(file, I2C_SLAVE, 0x44) < 0) // 68
    {
        printf("Failed to acquire bus access and/or talk to slave.\n");
		close(file);
        exit(1);
    }
	
	while(!endStat)
	{
		char config[2] = {0};
		config[0] = 0x2C;	//44
		config[1] = 0x06;	// 6
		if (write(file, config, 2) != 2)
		{
			printf("Failed to write to the i2c bus.\n");
			exit(1);
		}
		sleep(1);
		
		// Read 6 bytes of data
		// Temp msb, Temp lsb, Temp CRC, Humidity msb, Humidity lsb, Humidity CRC
		char data[6] = {0};
		if(read(file, data, 6) != 6)
		{
			printf("Error: Input/output Error \n");
		}
		else
		{
			int temp = (data[0] * 256 + data[1]);
			
			pthread_mutex_lock(&mutex);
			cTemp = -45 + (175 * temp / 65535.0);
			humidity = 100 * (data[3] * 256 + data[4]) / 65535.0;
			pthread_mutex_unlock(&mutex);
		}
		sleep(5);
	}
	close(file);
}

void *light(void *arg)
{
	int lightSensor = 4;
	while(1) 
	{
		long waitTime = millis();	// 10초 타이머 시작 
		
		/* start 조도센서 읽는 부분 */
		pinMode(lightSensor, OUTPUT);	
		digitalWrite(lightSensor, 1);
		delay(1);
		pinMode(lightSensor, INPUT);
		digitalWrite(lightSensor, 0);
		pthread_mutex_lock(&mutex);
		lightTime = micros();
		while(digitalRead(lightSensor));
		lightTime = micros() - lightTime;
		/* end 조도센서 읽는 부분 */
		
		FILE *file;
		if(isDateChanged())	// 날짜 바뀌면 0으로 초기화
		{
			file = openLogFile(FILENAME, "w");
			fprintf(file,"%d", 0); 
			fclose(file);
		}
		
		file = openLogFile(FILENAME, "r");
		if(fscanf(file, "%d", &lightLogNum) != 1) lightLogNum = 0;
		fclose(file);
		
		if (lightTime < 1000)
		{
			file = openLogFile(FILENAME, "w");
			/* log.txt, 내용에는 0만 있는 파일을 미리 생성해야 함.*/
			// 10초동안 빛이 밝았으면 1 더함, n / 6 == N분동안 빛을 받음
			fprintf(file, "%d", ++lightLogNum); 
			fclose(file);
		}
		pthread_mutex_unlock(&mutex);
		
		while(millis() - waitTime < 10000);	// 10초 타이머 종료
	}
	
}

int isDateChanged() {
	// 날짜 관련
	static struct tm initial_date = {0};
    time_t now = time(NULL);
    struct tm current_date;
	
	FILE *file1 = openLogFile(FILENAME_DATE, "r");
    
	localtime_r(&now, &current_date); // DateTime read
	
    // 초기 날짜가 설정되지 않은 경우, 초기 날짜를 현재 날짜로 설정
    if (initial_date.tm_year == 0 && initial_date.tm_mon == 0 && initial_date.tm_mday == 0) 
	{
		if (fscanf(file1, "%d %d %d", &initial_date.tm_year, &initial_date.tm_mon, &initial_date.tm_mday) == EOF)
		{
			perror("Error opening file \nTime.txt, 내용에는 0 0 0만 있는 파일을 미리 생성해야 함.\n");
			fclose(file1);
			exit(EXIT_FAILURE);
		}
    }
	else
		fscanf(file1, "%d %d %d", &initial_date.tm_year, &initial_date.tm_mon, &initial_date.tm_mday);
	
	fclose(file1);
	
    if (initial_date.tm_year != current_date.tm_year || initial_date.tm_mon != current_date.tm_mon || initial_date.tm_mday != current_date.tm_mday) 
	{
		file1 = openLogFile(FILENAME_DATE, "w");
		fprintf(file1, "%d %d %d", current_date.tm_year, current_date.tm_mon, current_date.tm_mday);
		fclose(file1);
        return 1;
    }
	
    return 0;
}

FILE *openLogFile(char *fileName, char *mode)
{
	// close 안하면 메모리에 남아서 나중에 다운됨. 
	// open 할때마다 fclose()해줘야함.
	FILE *file = fopen(fileName, mode);
	if (file == NULL) 
	{
		perror("Error opening file \n");
		exit(EXIT_FAILURE);
	}
	
	return file;
}


int tmprot() // 액추에이터 임시 함수 26 : 바닥, 28 : 스프링클러
{
	//fprintf(stderr, "actuator !!! \n");
	
	pinMode (26, OUTPUT);
	softPwmCreate(26, 0, 128);
	
	softPwmWrite(26, 70);
	
	delay(500);
	
	softPwmWrite(26, 0);
	digitalWrite(26,0);
	
    return 0;
}


int tmpspr()	// 스프링클러 임시 함수
{
	pinMode (28, OUTPUT);
	softPwmCreate(28, 0, 128);
	
	softPwmWrite(28, 70);
	
	delay(5000);
	
	softPwmWrite(28, 0);
	digitalWrite(28,0);
	
    return 0;
}


int tmpled()	// 조명 임시 함수
{
	pinMode (25, OUTPUT);
	static int stat = 0;
	stat = (stat == 0) ? 1 : 0;
	digitalWrite(25, stat);
	
    return stat;
}