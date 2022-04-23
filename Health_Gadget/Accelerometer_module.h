#ifndef ACCELEROMETER_MODULE_H
#define ACCELEROMETER_MODULE_H

#include "health_helper.h"

void Accelerometer_setup(int xpin, int ypin, int zpin); 
void Accelerometer_loop_step(); 
long int Accelerometer_get_steps();

#endif
