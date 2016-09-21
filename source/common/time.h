#ifndef _TIME_H_
#define _TIME_H_
#include <stdint.h>

typedef struct{
    unsigned short year;
    unsigned char month;
    unsigned char day;
    unsigned char hour;
    unsigned char minute;
    unsigned char second;
    unsigned char weekdays;
}s_tm;

extern unsigned int system_sec_get(void);
extern void system_sec_set(unsigned int time);

extern void system_time_set(s_tm tm);
extern void system_time_get(s_tm *tm) ;

void system_timezone_set(unsigned char timezone);

unsigned char system_timezone_get(void);

extern void system_time_init(void);



void system_time_tick(void * p_context);

#endif



