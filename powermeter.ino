
/*
  Karen's Powermeter code
  Based off of code written by tbressers:
  https://gitlab.com/tbressers/power/-/blob/master/power.ino
*/
#include <Arduino.h>
#include "HX711.h"
#include "loadcell_functions.h"
#include "ble_functions.h"
#include "IMU_functions.h"
#include "utils.h"

#define STAND_STILL_RAD_PER_SEC (0.25 * PI)  // theshold for wz to assume the user stopped pedaling
#define STOPPED_BLE_UPDATE_INTERVAL 2000  // Interval in ms for publishing the cadence and power to bluetooth when not pedaling
#define CRANK_MINIMUM_ROTATION_TIME 1000
#define CRANK_RADIUS 0.1725 // meters
#define VBATPIN A0 // Battery pin

// Initialize CSC and power variables
bool show_values=true; // print raw values
static float totalCrankRevs = 0; 
float wz;   // angular veloctiy in rad/s
float force;
HX711 LoadCell;


void setup() {
  Serial.begin(9600); // Start serial.
  // Ensure serial port is ready.
  // Comment out the next line if you want to
  // run the Nano without it being connected to
  // a PC/Mac/Linux with a microusb cable.
  //while (!Serial); 
  startIMU();      // begin IMU initialization
  loadCellSetup(LoadCell); // begin HX711 load cell initialization
  startBLE();      // begin BLE initialization
  
  pinMode(LED_BUILTIN, OUTPUT); // initialize digital pin LED_BUILTIN as an output.
  
  Serial.println("Setup completed.\n\n");

}

void loop() {

   // wait for a BLE central
  BLEDevice central = BLE.central();

  // if a central is connected to the peripheral:
  if (central) {
    
    Serial.print("Connected to central: ");
    Serial.println(central.address()); // print the central's BT address
    
    blink_on_startup(); // blink LED to indicate the connection:
    
    while (central.connected()) {
      // Delay to wait for enough input, since we have a limited transmission buffer
      delay(200);

      // Initialize variables 
      static bool pedaling = true;
      static float wz_avg; // moving average wz
      static float force_avg;
      static float power;
      
      // Initialize timers
      static long lastMeasurement = millis();
      static long lastStopMessage = millis();
      static long lastBluetoothUpdate = millis();

      wz = read_gyroscope();
      wz_avg = moving_average_cadence(wz);
      
      // Check if we stopped pedaling
      if (wz_avg <= STAND_STILL_RAD_PER_SEC && (millis() - lastStopMessage) >= STOPPED_BLE_UPDATE_INTERVAL) {
        pedaling = false;
        lastMeasurement = millis(); // Reset last-measurement time: we only want to average the applied force over actual pedaled time
        
        lastStopMessage = millis(); // Reset timers
        blePublishCadence((long)totalCrankRevs, millis()); // We are not pedaling. Report this to the bluetooth host
        blePublishPower( 0, totalCrankRevs, millis());
          
        Serial.print(wz_avg); 
        Serial.print(" rad/s, "); 
        Serial.print(force_avg); 
        Serial.print(" N, ");
        Serial.print(power); 
        Serial.println(" W"); 
         
        
      }
      // We are pedaling (not standing still) 
      else {
        pedaling = true;
        if ((millis() - lastMeasurement) >= CRANK_MINIMUM_ROTATION_TIME) { 
          // compute the power
          force = getForce(LoadCell);
          force_avg = moving_average_force(force);
          power = compute_power(wz_avg, force_avg);
          lastMeasurement = millis(); // Reset the timer
        }
        // Publish updated cadence and latest power measurement to the bluetooth host after each crank-rotation
        // Estimate crank revolutions since last bluetooth-update from the last gyroscope crank-speed measurements (wz_avg)
        float crankRevAdd = ((millis() - lastBluetoothUpdate)/1000.f) * (wz_avg / (2 * PI));
        if (crankRevAdd >= 1.0) {
              //print values for testing
              Serial.print(wz_avg); 
              Serial.print(" rad/s, ");
              Serial.print(totalCrankRevs); 
              Serial.print(" revs, ");
              Serial.print(force_avg); 
              Serial.print(" N, "); 
              Serial.print(power); 
              Serial.println(" W");
              
              totalCrankRevs = totalCrankRevs + crankRevAdd;
              blePublishCadence(totalCrankRevs, millis());
              blePublishPower( power, totalCrankRevs, millis());
              
              // Reset timer
              lastBluetoothUpdate = millis();
          }
      }
      Serial.println(pedaling);

    }

    // when the central disconnects, turn off the LED:
    digitalWrite(LED_BUILTIN, LOW);
    
    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
    
  }
}

float compute_power(float wz_avg, float force_avg) {
  float torque = 0;
  float power = 0;

  torque = CRANK_RADIUS * force_avg; // r x F
  // Need to multiply by 2 since we only have one sensor on one crank
  power = 2 * torque * wz_avg;

  return power;
}

void blink_on_startup() {
  int startup_cnt = 0; //blinking if first start
  while (startup_cnt < 2) {
     digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
     delay(500);                       // wait for a second
     digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
     delay(500);                       // wait for a second
     startup_cnt++;   
    }
}

uint8_t updateBatteryLevel() {
  /* Read the current voltage level on the A0 analog input pin.
     This is used here to simulate the charge level of a battery.
  */
  float battery = analogRead(VBATPIN);
  uint8_t batteryLevel = map(battery, 0, 1023, 0, 100);

  return batteryLevel;
}

    
