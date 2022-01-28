#ifndef IMU_FUNCTIONS_H
#define IMU_FUNCTIONS_H

#include <Arduino_LSM6DS3.h>

void startIMU();
float read_gyroscope();
void read_accelerometer(float &x, float &y, float &z);

#endif
