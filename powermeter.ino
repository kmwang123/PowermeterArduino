
/*
  Karen's Powermeter code
*/

#include <Arduino.h>
#include "ble_functions.h"
#include "IMU_functions.h"
//#include <ArduinoBLE.h> //comment out later
// setup moving average

// CSC variables
uint16_t crankrev;  // Cadence RPM
uint16_t lastcrank; // Last crank time
uint8_t cadence_rpm;
float x, y, z;
void setup() {
  Serial.begin(9600); // Start serial.
  // Ensure serial port is ready.
  // Comment out the next line if you want to
  // run the Nano without it being connected to
  // a PC/Mac/Linux with a microusb cable.
  while (!Serial);
  startIMU(); // begin IMU initialization
  startBLE(); // begin BLE initialization

  // Initialize values
  crankrev = 0;
  lastcrank = 0;
  cadence_rpm = 0;

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
    // turn on the LED to indicate the connection:
    digitalWrite(LED_BUILTIN, HIGH);

    while (central.connected()) {
      // Delay to wait for enough input, since we have a limited transmission buffer
      delay(200);
      read_gyroscope(cadence_rpm);
      read_accelerometer(x, y, z);
      //cscMeasurementCharacteristics.writeValue(cscmeasurement, 11);
      //cscFeatureCharacteristics.writeValue(cscfeature, 1)
      Serial.print(x);
      Serial.print('\t');
      Serial.print(y);
      Serial.print('\t');
      Serial.print(z);
      Serial.print('\t');
      Serial.println(cadence_rpm);
      //Serial.println(cadenceMovingAvg);
      blePublishCadence(cadence_rpm);
    }

    // when the central disconnects, turn off the LED:
    digitalWrite(LED_BUILTIN, LOW);
    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }
}
