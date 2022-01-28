#include "ble_functions.h"
#include <Arduino.h> //include this if you're using arduino 
#include <ArduinoBLE.h> // bluetooth functionality
/*
 *  BLUETOOTH
 */

#define BLE_DEVICE_NAME "ArduinoNano33IOT" // device name for ble scan
#define BLE_LOCAL_NAME "Cycle Power and Cadence" // local name


// Service and character constants at:
// https://www.bluetooth.com/specifications/gatt/services
/*------------------------------------------------------------------*/
/* Service UUID
 * https://www.bluetooth.com/specifications/gatt/services
 *------------------------------------------------------------------*/
#define UUID16_SVC_BATTERY                                    "180F"
#define UUID16_SVC_CYCLING_SPEED_AND_CADENCE                  "1816"
#define UUID16_SVC_CYCLING_POWER                              "1818"

/*------------------------------------------------------------------*/
/* Characteristic UUID
 * https://www.bluetooth.com/specifications/gatt/characteristics
 *------------------------------------------------------------------*/
#define UUID16_CHR_BATTERY_LEVEL                              "2A19"     
 
#define UUID16_CHR_CSC_FEATURE                                "2A5C"
#define UUID16_CHR_CSC_MEASUREMENT                            "2A5B"

#define UUID16_CHR_CYCLING_POWER_MEASUREMENT                  "2A63"
#define UUID16_CHR_CYCLING_POWER_FEATURE                      "2A65"
#define UUID16_CHR_SENSOR_LOCATION                            "2A5D"

/* Battery level Service Definitions 0x180F
 *  Battery Level Char               0x2A19
 */
BLEService batteryService(UUID16_SVC_BATTERY);
BLEUnsignedCharCharacteristic batteryLevelChar(UUID16_CHR_BATTERY_LEVEL, BLERead | BLENotify);
/* Cadence Service Definitions 0x1816
 *  Cadence Measurement Char    0x2A5B
 *  Cadence Feature Char:      0x2A5C
 */

BLEService cscService(UUID16_SVC_CYCLING_SPEED_AND_CADENCE);
BLECharacteristic cscFeatChar = BLECharacteristic(UUID16_CHR_CSC_FEATURE, BLERead, 1); // the characteristic is 1 byte long
BLEUnsignedCharCharacteristic cscMeasChar(UUID16_CHR_CSC_MEASUREMENT, BLERead | BLENotify); // remote clients will be able to get notifications if this characteristic changes
//BLEUnsignedCharCharacteristic
/* 
 * Pwr Service Definitions
 *  Cycling Power Service:      0x1818
 *  Power Measurement Char:     0x2A63
 *  Cycling Power Feature Char: 0x2A65
 *  Sensor Location Char:       0x2A5D
 */
//BLEService        pwrService(UUID16_SVC_CYCLING_POWER);
//BLECharacteristic pwrMeasChar = BLECharacteristic(UUID16_CHR_CYCLING_POWER_MEASUREMENT);
//BLECharacteristic pwrFeatChar = BLECharacteristic(UUID16_CHR_CYCLING_POWER_FEATURE);
//BLECharacteristic pwrLocChar  = BLECharacteristic(UUID16_CHR_SENSOR_LOCATION);
//BLECharacteristic pwrVector   = BLECharacteristic(UUID16_CHR_CYCLING_POWER_VECTOR);


void startBLE(void) {
  if (!BLE.begin())
  {
    Serial.println("starting BLE failed!");
    while (1);
  }
  else {
    
    /* Set a local name for the BLE device
     This name will appear in advertising packets
     and can be used by remote devices to identify this BLE device
     The name can be changed but maybe be truncated based on space left in advertisement packet
    */
    BLE.setDeviceName(BLE_DEVICE_NAME);
    BLE.setLocalName(BLE_LOCAL_NAME);

    //setupBattery();
    setupCSC();
    startAdv();
    
  }
}
/*
 * Set up the battery service
 */
void setupBattery(void) {
  BLE.setAdvertisedService(batteryService); // add the service UUID
  batteryService.addCharacteristic(batteryLevelChar); // add the battery level characteristic
  BLE.addService(batteryService); // Add the battery service
}
/*
 * Set up the cadence service
 */
void setupCSC(void) {
   
    BLE.setAdvertisedService(cscService); 
    cscService.addCharacteristic(cscFeatChar); 
    cscService.addCharacteristic(cscMeasChar); 
    BLE.addService(cscService); 
    // do i need to initialize these?
    //cscFeatChar.writeValue(?)
    //cscMeasChar.writeValue(?)
  //
  //https://github.com/sputnikdev/bluetooth-gatt-parser/blob/master/src/main/resources/gatt/characteristic/org.bluetooth.characteristic.csc_feature.xml
  //


  //
  //https://github.com/sputnikdev/bluetooth-gatt-parser/blob/master/src/main/resources/gatt/characteristic/org.bluetooth.characteristic.csc_measurement.xml
  //
}

void startAdv(void) {
    /* Start advertising BLE.  It will start continuously transmitting BLE
     advertising packets and will be visible to remote BLE central devices
     until it receives a new connection */

   // start advertising
   BLE.advertise();
   Serial.println("Bluetooth device active, waiting for connections...");
}

//void blePublishBattery() {
//  
//}
void blePublishCadence(uint8_t &cadence_rpm) {
  byte cscfeature[1] = { 0b0000000000000010 }; // specifies that this is crank revolution data
  byte cscmeasurement[11] = { 0b00000010, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; 
  cscMeasChar.writeValue(cadence_rpm);
}
