
#include "Arduino.h"
#include "time_controlling.hh"

#define DEBUG_TIME_CONTROLLING 0

void check_times(int8_t siz, unsigned long *set_up_times, unsigned long *duration_times, int8_t *indexes)
{
  for (int8_t i = 0; i < siz; ++i)
  {
    if (is_time_pass(&set_up_times[i], duration_times[i]))
    {
#if DEBUG_TIME_CONTROLLING
      Serial.print("main: ");
      Serial.print(i);
      Serial.print(" time: ");
      Serial.print(duration_times[i]);
      Serial.print(" current: ");
      Serial.println(millis());
#endif
      set_time(&set_up_times[i]);

      indexes[i] = 0;
    }
    else
    {
      indexes[i] = -1;
    }
  }
}

void set_times(int8_t siz, unsigned long *set_up_times)
{
  for (int8_t i = 0; i < siz; ++i)
  {
    set_time(&set_up_times[i]);
  }
}

// void do_overdue_tasks(int8_t siz, int8_t indexes[], void * addresses[])
// {
//   for (int8_t i = 0; i < siz; ++i)
//   {
//     if (indexes[i] == 0)
//     {
//       void (*func)() = (void (*)())addresses[i];
//       func();
//       indexes[i] = -1;
//     }
//   }
// }

void set_time(unsigned long *time_set)
{
  *time_set = millis();
}

bool is_time_pass(unsigned long *time_set, unsigned long during)
{
  return (millis() - *time_set) >= during || (millis() - *time_set) < 0;
}
