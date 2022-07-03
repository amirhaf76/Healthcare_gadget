#ifndef ACCELEROMETER_MODULE_H
#define ACCELEROMETER_MODULE_H

void accelerometer_setup(int xpin, int ypin, int zpin); 
void accelerometer_loop_step(); 
unsigned long accelerometer_get_steps();

#endif
