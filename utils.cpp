#include "utils.h"
#include <Arduino.h> //include this if you're using arduino 
/*
 *  Misc Utility Functions
 */
 
 float moving_average_cadence(float value) {
  const int nvalues = 32;            // At least the maximum number of values (#ZrotData) per crank-rotation 
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
 const int nvalues = 3;            // Average over the last 3 crank-rotations
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
