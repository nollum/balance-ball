#include <gyro.h>
#include <serial.h>
#include <stdio.h>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cmath>

std::vector<float> processData(const std::string &data);

Gyro::Gyro(int deltaT) {
    this->deltaT = deltaT;
    vel_x = vel_y = vel_z = 0;
    accel_x = accel_y = accel_z = 0; // accelerometer values
    avg_vel_x = avg_vel_y = avg_vel_z = 0;
    pitch = roll = 0;

    int status = setup_serial();

    if (status == -1) {
        printf("Error initializing serial connection\n");
    }
}

void Gyro::updateData() {
    std::string rawData;
    read_serial(rawData);
    std::vector<float> values = processData(rawData);

    vel_x = values[0];
    vel_y = values[1];
    vel_z = values[2];
    accel_x = values[3];
    accel_y = values[4];
    accel_z = values[5];

    avg_vel_x += vel_x;
    avg_vel_y += vel_y;
    avg_vel_z += vel_z;

    if (cur_time - prev_time >= deltaT) {
        // sample rate is 200Hz
        avg_vel_x /= 200;
        avg_vel_y /= 200; 

        float norm_accel_x = atan(accel_y / sqrt(pow(accel_x, 2) + pow(accel_z, 2)));
        float norm_accel_y = atan(-1 * accel_x / sqrt(pow(accel_y, 2) + pow(accel_z, 2)));
        float norm_accel_z = atan2(accel_y, accel_x);

        pitch = 0.98f * (pitch + avg_vel_x * (cur_time - prev_time)) + 0.02f * norm_accel_x; 
        roll = 0.98f * (roll + avg_vel_y * (cur_time - prev_time)) + 0.02f * norm_accel_y;
        // yaw doesn't work well as the error accumulates pretty fast

        avg_vel_x = 0;
        avg_vel_y = 0;

        prev_time = cur_time;
        
    }

}

std::vector<float> processData(const std::string &data) {
    std::string cleanedData = data;
    cleanedData.erase(std::remove(cleanedData.begin(), cleanedData.end(), '\n'), cleanedData.end());
    std::vector<float> values;
    std::istringstream stream(cleanedData);
    std::string token;
    while (stream >> token) {
        values.push_back(std::stof(token));
    }
    
    return values;
}