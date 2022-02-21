
/*
  Karen's Powermeter code
  Based off of code written by tbressers:
  https://gitlab.com/tbressers/power/-/blob/master/power.ino
*/

#include <Arduino.h>
#include "HX711.h"
#include "ble_functions.h"
#include "IMU_functions.h"
#include "loadcell_functions.h"
// If the number of radians per seconds is less than this, we assume the user stopped pedaling
#define STAND_STILL_RAD_PER_SEC (0.25 * PI)
// Interval for publishing the cadence and power to bluetooth when not pedaling
#define STOPPED_BLE_UPDATE_INTERVAL 2000
// The minimum time between two measurements in milliseconds
// to prevent spikes in the avgForce calculation 
// The HX711 rate is 10 Hz, and the HX711 library smoothens the data over the last 32 samples = 3.2 seconds  CHECK THIS!
// and we assume that this 3.2 seconds is the slowest cycle possible (for which we still want all measurements)
#define CRANK_MINIMUM_ROTATION_TIME 1000
#define CRANK_RADIUS 0.1725 // meters

// HX711 constructor:
HX711 LoadCell;

// Bluetooth
bool show_values=true; // print raw values
bool startup = true; //blinking if first start

// CSC variables
// Last measured/calculated values 
static float totalCrankRevs = 0; 
float wz;   // angular veloctiy in radians per second
float x, y, z;

// Power variables
float force;

// Interrupt related variables (must be volatile)
volatile uint8_t newZrotDataReady = 0;

void setup() {
  Serial.begin(9600); // Start serial.
  // Ensure serial port is ready.
  // Comment out the next line if you want to
  // run the Nano without it being connected to
  // a PC/Mac/Linux with a microusb cable.
  while (!Serial);
  startIMU();      // begin IMU initialization
  loadCellSetup(LoadCell); // begin HX711 load cell initialization
  startBLE();      // begin BLE initialization

  Serial.println("Setup completed.\n\n");
  Serial.println("Enter 'h' for help.\n\n");
}

void loop() {

  // wait for a BLE central
  BLEDevice central = BLE.central();

  // if a central is connected to the peripheral:
  if (central) {
    Serial.print("Connected to central: ");
    // print the central's BT address:
    Serial.println(central.address());
    // blink LED five times to indicate the connection:
    if (startup) {
      startup = false;
      for (int i = 0; i <= 5; i++) {
        digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
        delay(500);                       // wait for a second
        digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
        delay(500);                       // wait for a second
      }
    }

    while (central.connected()) {
      // Delay to wait for enough input, since we have a limited transmission buffer
      delay(200);
      static float wz_avg; // moving average wz
      static float force_avg;
      static float power;
      static bool pedaling = false;

      // Initialize timers
      static long lastMeasurement = millis();
      static long lastStopMessage = millis();
      static long lastBluetoothUpdate = millis();

      wz = read_gyroscope(newZrotDataReady);
      read_accelerometer(x, y, z);
      wz_avg = moving_average_cadence(wz);
      
      //Serial.println(x +'\t' + y + '\t' + z + '\t' + wz);
     
      // Check if we stopped pedaling
      if (wz_avg <= STAND_STILL_RAD_PER_SEC) {
        pedaling = false;
        // Reset last-measurement time: we only want to average the applied force over actual pedaled time
        lastMeasurement = millis();

        if ((millis() - lastStopMessage) >= STOPPED_BLE_UPDATE_INTERVAL) {
          // Reset timers
          lastStopMessage = millis();
          // We are not pedaling. Report this to the bluetooth host
          blePublishCadence((long)totalCrankRevs+0.5, millis()); // zero power, no cadence (resend same totalCrankRevs)
          blePublishPower( 0, totalCrankRevs, millis());
          if (show_values) {
            Serial.print(wz_avg); 
            Serial.print(" rad/s, "); 
            Serial.print(force_avg); 
            Serial.print(" N, ");
            Serial.print(power); 
            Serial.println(" W"); 
          }
        }  
      }
      // We are pedaling (not standing still) 
      else {
          pedaling = true;
          // We only calculate values if we have sufficient force-measurements (HX711 measures at 10 Hz)-> CHECK THIS!
          if ((millis() - lastMeasurement) >= CRANK_MINIMUM_ROTATION_TIME) { 
            // compute the power
            force = getForce(LoadCell);
            force_avg = moving_average_force(force);
            power = compute_power(wz_avg, force_avg);
          
            // Reset Zrot measurement counter
            newZrotDataReady = 0;
            // Reset the timer
            lastMeasurement = millis();
          }
          //
          // Publish updated cadence and latest power measurement to the bluetooth host after each crank-rotation
          //
          // Estimate crank revolutions since last bluetooth-update from the last gyroscope crank-speed measurements (wz_avg)
          float crankRevAdd = ((millis() - lastBluetoothUpdate)/1000.f) * (wz_avg / (2 * PI));
          if (crankRevAdd >= 1.0) {
              //print values for testing
              if (show_values) {
                Serial.print(wz_avg); 
                Serial.print(" rad/s, ");
                Serial.print(totalCrankRevs); 
                Serial.print(" revs, ");
                Serial.print(force_avg); 
                Serial.print(" N, "); 
                Serial.print(power); 
                Serial.println(" W");
              }
              totalCrankRevs = totalCrankRevs + crankRevAdd;
              blePublishCadence(totalCrankRevs, millis());
              blePublishPower( power, totalCrankRevs, millis());
              // Reset timer
              lastBluetoothUpdate = millis();
          }
       }
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

float moving_average_cadence(float value) {
  const int nvalues = 256;            // At least the maximum number of values (#ZrotData) per crank-rotation 
 //const int nvalues = 3;            // Average over the last 3 crank-rotations
  static int current = 0;            // Index for current value
  static int cvalues = 0;            // Count of values read (<= nvalues)
  static float sum = 0;               // Rolling sum
  static float values[nvalues];

  sum += value;

  // If the window is full, adjust the sum by deleting the oldest value
  if (cvalues == nvalues)
    sum -= values[current];

  values[current] = value;          // Replace the oldest with the latest

  if (++current >= nvalues)
    current = 0;

  if (cvalues < nvalues)
    cvalues++;

  return sum/cvalues;
}

float moving_average_force(float value) { 
 const int nvalues = 32;            // Average over the last 3 crank-rotations
  static int current = 0;            // Index for current value
  static int cvalues = 0;            // Count of values read (<= nvalues)
  static float sum = 0;               // Rolling sum
  static float values[nvalues];

  sum += value;

  // If the window is full, adjust the sum by deleting the oldest value
  if (cvalues == nvalues)
    sum -= values[current];

  values[current] = value;          // Replace the oldest with the latest

  if (++current >= nvalues)
    current = 0;

  if (cvalues < nvalues)
    cvalues++;

  return sum/cvalues;
}
