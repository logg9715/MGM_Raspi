// Distributed with a free-will license.
// Use it any way you want, profit or free, provided it fits in the licenses of its associated works.
// SHT30
// This code is designed to work with the SHT30_I2CS I2C Mini Module available from ControlEverything.com.
// https://www.controleverything.com/content/Humidity?sku=SHT30_I2CS#tabs-0-product_tabset-2

#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>  // for sleep function

int main() 
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
    if (ioctl(file, I2C_SLAVE, 0x44) < 0) 
    {
        printf("Failed to acquire bus access and/or talk to slave.\n");
        exit(1);
    }

    // Send measurement command(0x2C)
    // High repeatability measurement(0x06)
    char config[2] = {0};
    config[0] = 0x2C;
    config[1] = 0x06;
    if (write(file, config, 2) != 2)
    {
        printf("Failed to write to the i2c bus.\n");
        exit(1);
    }
    
    // Wait for the measurement to be taken
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
        // Convert the data
        int temp = (data[0] * 256 + data[1]);
        float cTemp = -45 + (175 * temp / 65535.0);
        float fTemp = -49 + (315 * temp / 65535.0);
        float humidity = 100 * (data[3] * 256 + data[4]) / 65535.0;

        // Output data to screen
        printf("Relative Humidity : %.2f%% RH \n", humidity);
        printf("Temperature in Celsius : %.2f °C \n", cTemp);
        printf("Temperature in Fahrenheit : %.2f °F \n", fTemp);
    }

    close(file);
    return 0;
}
