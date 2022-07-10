#ifndef HRB_OX_MODULE_H
#define HRB_OX_MODULE_H

void HRB_OX_module_setup();
void HRB_OX_module_down();
void HRB_OX_module_loop_step();
void get_beatsPerMinute_beatAvg(float * _beatsPerMinute, int * _beatAvg);

#endif