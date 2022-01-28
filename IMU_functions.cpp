#include "IMU_functions.h"
#include <Arduino.h> //include this if you're using arduino 
/* For the use of the IMU sensor */
#include <Arduino_LSM6DS3.h>
/*
 *LSM6DS3 IMU sensor
 */
void startIMU() {
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");

    while (1);
  }
 
  Serial.print("Gyroscope sample rate = ");
  Serial.print(IMU.gyroscopeSampleRate());
  Serial.println(" Hz");
  Serial.println();
  Serial.println("Gyroscope in rpm");
  Serial.print("Accelerometer sample rate = ");
  Serial.print(IMU.accelerationSampleRate());
  Serial.println(" Hz");
  Serial.println();
  Serial.println("Acceleration in G's");
  Serial.println("X\tY\tZ\twz");
}
void read_gyroscope(uint8_t &cadence_rpm) {
  if (IMU.gyroscopeAvailable()) {
      float wx, wy, wz;
      IMU.readGyroscope(wx, wy, wz);
      //converts from deg/s to rpm
      cadence_rpm = wz*(1./6);
    }
}

void read_accelerometer(float &x, float &y, float &z) {
  if (IMU.accelerationAvailable()) {
      IMU.readAcceleration(x, y, z);
    }
}
