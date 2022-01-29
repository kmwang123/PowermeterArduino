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

float read_gyroscope(volatile uint8_t &newZrotDataReady) {
  if (IMU.gyroscopeAvailable()) {
      /* Get new sensor events with the readings */
      float wz_radps, wx, wy, wz;
      IMU.readGyroscope(wx, wy, wz);
      //converts from deg/s to rad/s
      wz_radps = abs(wz*(PI/180.0)); //absolute value so can rotate either way
      newZrotDataReady++;
      return wz_radps;
    }
}

void read_accelerometer(float &x, float &y, float &z) {
  if (IMU.accelerationAvailable()) {
      IMU.readAcceleration(x, y, z);
    }
}
