
#ifndef CONTROL_MANAGER_H
#define CONTROL_MANAGER_H

void set_time(unsigned long * time_set);
bool is_time_pass(unsigned long * time_set, unsigned long during);

void check_times(int8_t siz, unsigned long *set_up_times, unsigned long *duration_times, int8_t * indexes);
void set_times(int8_t siz, unsigned long *set_up_times);
// void do_overdue_tasks(int8_t siz, int8_t indexes[], void *addresses[]);


#endif