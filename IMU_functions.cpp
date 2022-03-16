#include "IMU_functions.h"
#include <Arduino.h> 
#include <Arduino_LSM9DS1.h>

/*
 *LSM9DS1 IMU sensor
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
//  Serial.println("Gyroscope in rpm");
//  Serial.print("Accelerometer sample rate = ");
//  Serial.print(IMU.accelerationSampleRate());
//  Serial.println(" Hz");
//  Serial.println();
//  Serial.println("Acceleration in G's");
//  Serial.println("X\tY\tZ\twz");
}

float read_gyroscope() {
  float wz_radps, wx, wy, wz;
  if (IMU.gyroscopeAvailable()) {
      IMU.readGyroscope(wx, wy, wz); // Get new sensor events with the readings
      //converts from deg/s to rad/s
      wz_radps = abs(wz*(PI/180.0)); //absolute value so can rotate either way
      return wz_radps;
    }
  else {
      return 0;
    
  }
}

void read_accelerometer(float &x, float &y, float &z) {
  if (IMU.accelerationAvailable()) {
      IMU.readAcceleration(x, y, z);
    }
}
