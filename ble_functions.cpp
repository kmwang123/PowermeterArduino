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
#define CSC_MEAS_CHAR_LEN 5 //the characteristic is 5 bytes long

BLEService cscService(UUID16_SVC_CYCLING_SPEED_AND_CADENCE);
BLECharacteristic cscFeatChar = BLECharacteristic(UUID16_CHR_CSC_FEATURE, BLERead, sizeof(uint16_t)); // the characteristic is 16 bits long
BLECharacteristic cscMeasChar = BLECharacteristic(UUID16_CHR_CSC_MEASUREMENT, BLERead | BLENotify, CSC_MEAS_CHAR_LEN); //remote clients will be able to get notifications if this characteristic changes
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
  
  //


  //
  ///https://www.bluetooth.com/wp-content/uploads/Sitecore-Media-Library/Gatt/Xml/Characteristics/org.bluetooth.characteristic.csc_measurement.xml
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
void blePublishCadence(uint16_t crankRevs, long millisLast) {
  /**
   * Fields
   * https://github.com/oesmith/gatt-xml/blob/master/org.bluetooth.characteristic.csc_feature.xml
   * Flags (16 bits):
   *   b0 Wheel Revolution Data Supported
   *   b1 Crank Revolution Data Supported
   *   b2 Multiple Sensor Locations Supported
   *   
  */
  uint16_t cscfeature = 0b0000000000000010; // flag specifies that this is crank revolution data
  //byte cscfeature[1] = { 0b0000000000000010 }; // specifies that this is crank revolution data
  cscFeatChar.writeValue(cscfeature);
  

  /**
   * Fields
   * https://github.com/oesmith/gatt-xml/blob/master/org.bluetooth.characteristic.csc_feature.xml
   * Flags (8 bits):
   *   b0 Wheel Revolution Data Present
   *   b1 Crank Revolution Data Present
   *   b2-b7 Reserved for future use
   *
   * Cumulative Wheel Revolutions:
   *   uint32, field does not exist if key of bit 0 of the Flags is set to 0
   *   
   * Last Wheel Event Time:
   *   uint16, field does not exist if key of bit 0 of the Flags is set to 0
   *
   * Cumulative Crank Revolutions
   *  uint16, field exists if the key of bit 1 of the Flags field is set to 1
   *  
   * Last Crank Event Time
   *   uint16, Field exists if the key of bit 1 of the Flags field is set to 1.
   *   Unit has a resolution of 1/1024s
   *   
   *   The fields in the above table are in the order of LSO to MSO. Where LSO = Least Significant Octet and MSO =
   *   Most Significant Octet.
   */
   uint8_t flag = 0b00000010; // Flag for cadence
   
   uint8_t cumCrankRev[2]; //split 16 bits into two 8 bit arrays, LSO is first in array
   uint16ToLso(crankRevs, cranks);
   
   uint8_t lastCrankEventTime[2];
   // Cadence last event time is time of last event, in 1/1024 second resolution
   uint16_t lastEventTime = uint16_t(millisLast / 1000.f * 1024.f) % 65536;
   uint16ToLso(lastEventTime, lastTime);
   
   uint8_t cscmeasdata[CSC_MEAS_CHAR_LEN] = { flag,
                                              cumCrankRev[0], cumCrankRev[1],
                                              lastCrankEventTime[0], lastCrankEventTime[1]}; 
   cscMeasChar.writeValue(cscmeasdata);
}

/*
 * Given a 16-bit uint16_t, convert it to 2 8-bit ints, and set
 * them in the provided array. Assume the array is of correct
 * size, allocated by caller. Least-significant octet is place
 * in output array first.
 */
void uint16ToLso(uint16_t val, uint8_t* out) {
  uint8_t lso = val & 0xff;
  uint8_t mso = (val >> 8) & 0xff;
  out[0] = lso;
  out[1] = mso;
}
