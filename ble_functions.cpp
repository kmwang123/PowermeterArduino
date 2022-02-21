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
#define CSC_MEAS_CHAR_LEN 5 //the characteristic is 5 bytes long, 11 if there is speed data

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
#define PWR_MEAS_CHAR_LEN 8 //characteristic is 8 bytes long
BLEService        pwrService(UUID16_SVC_CYCLING_POWER);
BLECharacteristic pwrFeatChar = BLECharacteristic(UUID16_CHR_CYCLING_POWER_FEATURE, BLERead, sizeof(uint32_t));
BLECharacteristic pwrLocChar  = BLECharacteristic(UUID16_CHR_SENSOR_LOCATION, BLERead, sizeof(uint8_t));
BLECharacteristic pwrMeasChar = BLECharacteristic(UUID16_CHR_CYCLING_POWER_MEASUREMENT, BLERead | BLENotify, PWR_MEAS_CHAR_LEN);
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
    setupPWR();
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

/*
 * Set up the cadence service
 */
void setupPWR(void) {
   
    BLE.setAdvertisedService(pwrService); 
    pwrService.addCharacteristic(pwrFeatChar); 
    pwrService.addCharacteristic(pwrLocChar); 
    pwrService.addCharacteristic(pwrMeasChar);
    BLE.addService(pwrService); 
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
  /** CSC Feature
   * Fields
   * https://github.com/oesmith/gatt-xml/blob/master/org.bluetooth.characteristic.csc_feature.xml
   * Flags (16 bits):
   *   b0 Wheel Revolution Data Supported
   *   b1 Crank Revolution Data Supported
   *   b2 Multiple Sensor Locations Supported
   *   
  */
  uint16_t cscfeature = 0b0000000000000011; // flag specifies both crank and wheel revolution data present
  //uint16_t cscfeature = 0b0000000000000010; // flag specifies that this is crank revolution data
  cscFeatChar.writeValue(cscfeature, sizeof(uint16_t));
  

  /** CSC Measurement
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
   //uint8_t flag = 0b00000011; // Flag for cadence
   uint8_t flag = 0b00000010; // Flag for cadence

   // do not output any speed data
   //uint8_t wheelRev1[2];
   //uint8_t wheelRev2[2];
   //uint16ToLso(0, wheelRev1);
   //uint16ToLso(0, wheelRev2);
   
   uint8_t cumCrankRev[2]; //split 16 bits into two 8 bit arrays, LSO is first in array
   uint16ToLso(crankRevs, cumCrankRev);
   
   uint8_t lastCrankEventTime[2];
   // Cadence last event time is time of last event, in 1/1024 second resolution
   uint16_t lastEventTime = uint16_t(millisLast / 1000.f * 1024.f) % 65536;
   uint16ToLso(lastEventTime, lastCrankEventTime);

   //unsigned char cscmeasdata[CSC_MEAS_CHAR_LEN] = { flag, wheelRev1[0], wheelRev1[1], wheelRev2[0],
   //                                                 wheelRev2[1], lastCrankEventTime[0], lastCrankEventTime[1],
   //                                                 cumCrankRev[0], cumCrankRev[1],
   //                                                 lastCrankEventTime[0], lastCrankEventTime[1]}; 
   unsigned char cscmeasdata[CSC_MEAS_CHAR_LEN] = { flag,
                                                    cumCrankRev[0], cumCrankRev[1],
                                                    lastCrankEventTime[0], lastCrankEventTime[1]}; 
   cscMeasChar.writeValue(cscmeasdata, CSC_MEAS_CHAR_LEN);
}

/*
 * Publish the instantaneous power measurement.
 */
void blePublishPower(int16_t instantPwr, uint16_t crankRevs, long millisLast) {
  /** Power feature
   * Fields
   * https://github.com/sputnikdev/bluetooth-gatt-parser/blob/master/src/main/resources/gatt/characteristic/org.bluetooth.characteristic.cycling_power_feature.xml
   * Flags (32 bits):
   *   b0 Pedal Power Balance Supported
   *   b1 Accumulated Torque Supported
   *   b2 Wheel Revolution Data Supported
   *   b3 Crank Revolution Data Supported
   *   ... don't need these
   *   
  */
  uint32_t pwrfeature = 0b00000000000000000000000000001000; // flag specifies crank revolution data is supported
  pwrFeatChar.writeValue(pwrfeature, sizeof(uint32_t));

  /** Power Location characteristic
   * Fields
   * https://github.com/oesmith/gatt-xml/blob/master/org.bluetooth.characteristic.sensor_location.xml
   * Flags (8 bits):
   *   <Enumeration key="5" value="Left Crank" />
   *   
  */
  uint8_t pwrlocation = 0b00000111; // flag specifies sensor location is on left crank
  pwrLocChar.writeValue(pwrlocation, sizeof(uint8_t));
  
  
  /** Power measure characteristic
   * Fields
   *
   * Flags (16 bits):
   *   b0 pedal power balance present
   *   b1 pedal power balance reference
   *   b2 accumulated torque present
   *   b3 accumulated torque source
   *   b4 wheel revolution data present
   *   b5 crank revolution data present
   *   b6 extreme force magnitudes present
   *   b7 extreme torque magnitudes present
   *   b8 extreme angles present
   *   b9 top dead spot angle present
   *   b10 bottom dead spot angle present
   *   b11 accumulated energy present
   *   b12 offset compenstation indicator
   *   b13 reserved
   *
   *   https://github.com/sputnikdev/bluetooth-gatt-parser/blob/master/src/main/resources/gatt/characteristic/org.bluetooth.characteristic.cycling_power_measurement.xml
   * 
   * Instananous Power:
   *   16 bits signed int
   *   
   * Cumulative Crank Revolutions:
   *   16 bits signed int
   *
   * Last Crank Event Time
   *   16 bits signed int
   */
   // Flag cadence for power measurement
   uint16_t flag = 0b0000000000100000;
   uint8_t flags[2];
   uint16ToLso(flag, flags);

   // instantaneous power data
   uint8_t pwr[2];
   uint16ToLso(instantPwr, pwr);

   uint8_t cumCrankRev[2]; //split 16 bits into two 8 bit arrays, LSO is first in array
   uint16ToLso(crankRevs, cumCrankRev);
   
   uint8_t lastCrankEventTime[2];
   // Cadence last event time is time of last event, in 1/1024 second resolution
   uint16_t lastEventTime = uint16_t(millisLast / 1000.f * 1024.f) % 65536;
   uint16ToLso(lastEventTime, lastCrankEventTime);

   unsigned char pwrmeasdata[PWR_MEAS_CHAR_LEN] = { flags[0], flags[1],
                                                    pwr[0], pwr[1], 
                                                    cumCrankRev[0], cumCrankRev[1],
                                                    lastCrankEventTime[0], lastCrankEventTime[1]}; 
   pwrMeasChar.writeValue(pwrmeasdata, PWR_MEAS_CHAR_LEN);
  



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
