#ifndef GYRO_H
#define GYRO_H

#define TIME_INT 5

class Gyro
{

float vel_x, vel_y, vel_z;
float accel_x, accel_y, accel_z;
float avg_vel_x, avg_vel_y, avg_vel_z; 
float deltaT; 

public:
    float pitch, roll;
    float disp_x, disp_y, disp_z;
    float prev_time, cur_time;

    Gyro(int deltaT);
    void updateData();
};

#endif