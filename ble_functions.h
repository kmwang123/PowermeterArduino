#ifndef BLE_FUNCTIONS_H
#define BLE_FUNCTIONS_H

#include <ArduinoBLE.h> // bluetooth functionality

void startBLE(void);
void setupBattery(void);
void setupCSC(void);
void startAdv(void);
void blePublishCadence(uint8_t &cadence_rpm);

#endif
