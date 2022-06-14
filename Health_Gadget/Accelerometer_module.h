#ifndef ACCELEROMETER_MODULE_H
#define ACCELEROMETER_MODULE_H

void Accelerometer_setup(int xpin, int ypin, int zpin); 
void Accelerometer_loop_step(); 
long int Accelerometer_get_steps();

#endif
