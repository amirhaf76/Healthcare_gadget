#ifndef THERMOMETER_MODULE_H
#define THERMOMETER_MODULE_H

#define DEBUG_THERMOMETER_MODULE 1

/* Declarations */
void Thermometer_module_setup();
void Thermometer_module_loop_step();
int Thermometer_get_temperature();

#endif