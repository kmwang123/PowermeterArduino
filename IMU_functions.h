#ifndef IMU_FUNCTIONS_H
#define IMU_FUNCTIONS_H

#include <Arduino_LSM9DS1.h>

void startIMU(void);
float read_gyroscope(void);
void read_accelerometer(float &x, float &y, float &z);

#endif
