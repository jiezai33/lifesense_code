#include "time.h"
#include "debug.h"

#define YEAR_BASE 		(1970)
#define DAY_SEC      	(86400)		/* one day second = 24*60*60 */

#define APP_TIMER_PRESCALER         0

static unsigned int gTime_sec = 0;
static unsigned char g_timezone = 0x50;//¶«8Çø

static char isleap(unsigned short year)
{
    return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
}

static unsigned short get_yeardays(unsigned short year) 
{
    if (isleap(year))
        return 366;
    return 365;
}

static unsigned char CaculateWeekDay(unsigned short y,unsigned char m, unsigned char d)
{
	unsigned char week = 0;
    if(m==1||m==2) {
        m+=12;
        y--;
    }
    week=(d+2*m+3*(m+1)/5+y+y/4-y/100+y/400)%7;	
	return week;
}

void system_time_set(s_tm tm)
{
	unsigned int retDay = 0;
	unsigned short year = tm.year;
	unsigned char i,mons[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	if(tm.month>12 || tm.month==0 || tm.day>31 || tm.day == 0 || tm.hour > 23 || tm.minute >59)
		return;
	if(tm.second > 59)
		tm.second = 0;
	
	while((year--)>YEAR_BASE)
	{
		retDay += get_yeardays(year);
	}

	if (isleap(tm.year)) 
		mons[1] += 1;
	
	for (i = 1; i < tm.month; i++) 
	{
		retDay += mons[i-1];
	}

	retDay += tm.day-1;

	retDay *= DAY_SEC;

	retDay += tm.hour*3600;
	retDay += tm.minute*60;
	retDay += tm.second;
	gTime_sec = retDay;
}

void system_time_get(s_tm *tm) 
{
	unsigned char i, mons[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	unsigned short curr_day = 0;
	unsigned int days = 0;
	days = gTime_sec/DAY_SEC;
	
	curr_day = get_yeardays(tm->year = YEAR_BASE);
    while (days >= curr_day)
	{
        days -= curr_day;
        tm->year += 1;
        curr_day = get_yeardays(tm->year);
    }
	
    if (isleap(tm->year))
		mons[1] += 1;
    for (i = 0; i < 12; i++) 
	{
        if (days < mons[i]) 
		{
            tm->month = i+1;
            tm->day = days+1;
            break;
        }
		else
        	days -= mons[i];
    }
	
	tm->hour = (gTime_sec % DAY_SEC)/(3600);
    tm->minute = (gTime_sec % DAY_SEC)%(3600)/60;
    tm->second = (gTime_sec % DAY_SEC)%60;
	tm->weekdays = CaculateWeekDay(tm->year,tm->month,tm->day);
}

void system_sec_set(unsigned int time)
{
	unsigned int timezone = 0;
  	gTime_sec = time;

	if(g_timezone > 0x30)
	{
		timezone = (g_timezone - 0x30)*900;
		gTime_sec += timezone;
	}
	else
	{
		timezone = (0x30 - g_timezone)*900;
		gTime_sec -= timezone;
	}
}

unsigned int system_sec_get(void)
{
	return gTime_sec;
}

void system_timezone_set(unsigned char timezone)
{
  	g_timezone = timezone;
}

unsigned char system_timezone_get(void)
{
	return g_timezone;
}


void system_time_tick(void * p_context)
{
	gTime_sec++;
}

void system_time_init(void)
{
	s_tm tm;
	tm.year 	= 2016;
	tm.month 	= 1;
	tm.day 		= 1;
	tm.hour 	= 12;
	tm.minute 	= 0;
	tm.second	= 0;
	system_time_set(tm);
}



