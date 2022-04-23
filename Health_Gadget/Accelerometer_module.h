#ifndef ACCELEROMETER_MODULE_H
#define ACCELEROMETER_MODULE_H

#include "health_helper.h"

void Accelerometer_Meter_setup(int , int , int); 
void Accelerometer_loop_step(); 
long int Accelerometer_get_steps();

#endif
