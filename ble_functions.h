#ifndef BLE_FUNCTIONS_H
#define BLE_FUNCTIONS_H

#include <ArduinoBLE.h> // bluetooth functionality

void startBLE(void);
void setupBattery(void);
void setupCSC(void);
void setupPWR(void);
void startAdv(void);
void blePublishCadence(uint16_t crankRevs, long millisLast);
void blePublishPower(int16_t instantPwr, uint16_t crankRevs, long millisLast);
void uint16ToLso(uint16_t val, uint8_t* out);

#endif
