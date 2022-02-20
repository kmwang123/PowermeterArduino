#include "loadcell_functions.h"
#include <Arduino.h> 
#include "HX711.h"

/*
 *  HX711 Load cell chip
 *  Force and load cell-specific code and helpers
 *  This offset value is obtained by calibrating the scale with known
 *  weights, currently manually with a separate sketch.
 *  It's 'm' as in:
 *  'y = m*x + b'
 *  where 'y' is Newtons (our desired answer, kilograms at
 *  acceleration of gravity), 'x' is the raw reading of the load
 *  cell, and 'b' is the tare offset. So this multiplier is the
 *  scale needed to translate raw readings to units of Newtons.
 *  (defined in main file)
 */
// HX711 pins
#define HX711_DOUT  2
#define HX711_CLK  3
#define LOAD_OFFSET 168151.f
#define HX711_MULT  7050.0

// How many raw readings to take each sample.
#define NUM_RAW_SAMPLES 1


void loadCellSetup(HX711 &LoadCell) {
  Serial.println();
  Serial.println("Starting-up loadcell HX711...");

  LoadCell.begin(HX711_DOUT, HX711_CLK);
  
  // Set the scale for the multiplier to get grams.
  LoadCell.set_scale(HX711_MULT);

  // Set the offset (offset is subtracted from averaged read then
  // divided by the multiplier)
  float offset = LOAD_OFFSET;      
  LoadCell.set_offset(offset);
  
  Serial.print("Load offset set to: ");
  Serial.println(LOAD_OFFSET);
  Serial.print("Load multiplier set to: "); 
  Serial.println(HX711_MULT);

  LoadCell.power_up();
}

/**
 * Get the current force from the load cell. Returns an exponentially
 * rolling average, in Newtons.
 */
float getForce(HX711 &LoadCell) {
  
  float currentData;
  currentData = abs( LoadCell.get_units(NUM_RAW_SAMPLES) );
      
  
  return currentData;
}
