#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "ros/ros.h"
#include "std_msgs/String.h"

#define CO2_ADDR 0x15        // default I2C slave address
#define CO2_DEV "/dev/i2c-1" // default I2C device file

int main(int argc, char **argv)
{
    ros::init(argc, argv, "telaire");
    ros::NodeHandle n;
    ros::Publisher ppm_publisher = n.advertise<std_msgs::String>("ppm", 1);

    int file;
    std::string filename = "/dev/i2c-1";
    int ppmReading;

    if ((file = open(filename.c_str(), O_RDWR)) < 0)
    {
        ROS_INFO("Failed to open the bus.");
        /* ERROR HANDLING; you can check errno to see what went wrong */
        exit(1);
    }

    if (ioctl(file, I2C_SLAVE, CO2_ADDR) < 0)
    {
        ROS_INFO("Failed to acquire bus access and/or talk to slave.\n");
        /* ERROR HANDLING; you can check errno to see what went wrong */
        exit(1);
    }

    unsigned char buffer_write[5] = {0x04, 0x13, 0x8b, 0x00, 0x01};
    unsigned char buffer_read[4];

    while (ros::ok())
    {
        std_msgs::String msg;
        
        if( write(file, buffer_write, 5) != 5)
        {
            ROS_INFO("Failed to write to I2C bus \n");
        }
        else
        {
            usleep(50000);
            read(file, buffer_read, 4);

            // printf("buffer_read value: %d, %d, %d, %d \n", buffer_read[0], buffer_read[1], buffer_read[2], buffer_read[3]);

            ppmReading = buffer_read[2] << 8 | buffer_read[3];
            msg.data = std::to_string(ppmReading);
            ppm_publisher.publish(msg);
            // printf("C02 ppm: %d \n", ppmReading);
        }
        usleep(500000);
    }

    return EXIT_SUCCESS;
}
