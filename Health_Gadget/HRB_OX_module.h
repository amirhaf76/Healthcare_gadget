#ifndef HRB_OX_MODULE_H
#define HRB_OX_MODULE_H

void HRB_OX_module_setup_at_first();
void HRB_OX_module_wakeup();
void HRB_OX_module_shotdown();
void HRB_OX_module_loop_step();
void HRB_OX_module_loop_step_new();

int32_t get_spo2();
int32_t heartget_Rate();
int8_t get_valid_SPO2();
int8_t get_valid_HeartRate();

#endif