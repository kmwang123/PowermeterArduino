#ifndef LOADCELL_FUNCTIONS_H
#define LOADCELL_FUNCTIONS_H

#include "HX711.h"
void loadCellSetup(HX711 &LoadCell);
float getForce(HX711 &LoadCell);

#endif
