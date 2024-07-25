#ifndef SERIAL_H
#define SERIAL_H

#define TERMINAL    "/dev/serial/by-id/usb-STMicroelectronics_STM32_STLink_066DFF515049657187225529-if02"
#define DISPLAY_STRING 1

#include <errno.h>
#include <fcntl.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <string>

int set_interface_attribs(int fd, int speed);

void set_mincount(int fd, int mcount);

int setup_serial();

void read_serial(std::string &data);

#endif