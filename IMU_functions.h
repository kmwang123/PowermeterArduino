#ifndef IMU_FUNCTIONS_H
#define IMU_FUNCTIONS_H

#include <Arduino_LSM6DS3.h>

void startIMU();
void read_gyroscope(uint8_t &cadence_rpm);
void read_accelerometer(float &x, float &y, float &z);

#endif
