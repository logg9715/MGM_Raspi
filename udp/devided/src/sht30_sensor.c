#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "sht30_sensor.h"

extern pthread_mutex_t mutex;
extern float cTemp, humidity;
extern int endStat;

void *sht30(void *arg) {
    int file;
    char *bus = "/dev/i2c-1";
    if ((file = open(bus, O_RDWR)) < 0) 
    {
        printf("Failed to open the bus. \n");
        exit(1);
    }
    
    // SHT30 I2C -> 0x44(68)
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
		
		// data : [Temp msb, Temp lsb, Temp CRC, Humidity msb, Humidity lsb, Humidity CRC]
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
