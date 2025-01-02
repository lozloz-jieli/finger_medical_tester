
#include "utils.h"

/*
**********************************************************************
函数功能：unix -> 北京 time
函数形参：None
函数返回值：None
备注：
日期：2022年10月17日
作者：lozloz
版本：V0.0
**********************************************************************
*/

const char Days[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
void localtime(uint32_t time, pT_Time t)
{
    unsigned int Pass4year;
    unsigned int in4year;
    int hours_per_year;
	int year;

    if(time < 0)
    {
        time = 0;
	}
	//get second
    t->second = (uint8_t)(time % 60);
    time /= 60;
    //get minute
    t->minute = (uint8_t)(time % 60);
    time /= 60; //hour
	//get number of four years，every four year has 1461*24 hours 3*365 +356 = 1461 day
    Pass4year= ((unsigned int)time / (1461L * 24L));
    // in4year= ((unsigned int)time % (1461L * 24L));
    // printf("Pass4year = %d",Pass4year);
    // printf("in4year = %d",in4year);

	//calc year
    year = (Pass4year << 2) + 1970;
	//left hours every four year
    time %= 1461L * 24L;
    // printf("time = %d",time);

	//adjust leap year, calc left hours in one year
    for (;;)
    {
        //hours of one year
        hours_per_year = 365 * 24;
        //check leap year
        if ((year & 3) == 0)
        {
            //if leap year, there has more 24 hours.
            hours_per_year += 24;
        }
        if (time < hours_per_year)
        {
            break;
        }
        year++;
        time -= hours_per_year;
    }
	// number of hours
    t->hour = (uint8_t)(time % 24);
	//left days in one year
    time /= 24;
	//assumpt leap year
    time++;
	//adjust month and day of leap year
    if((year & 3) == 0)
    {
        if (time > 60)
        {
            time--;
        }
        else
        {
            if (time == 60)
            {
                t->month = 1;
                t->day = 29;
				t->year = year%2000;
                return ;
            }
        }
    }
	//calc month and day
    for (t->month = 0; Days[t->month] < time;t->month++)
    {
        time -= Days[t->month];
    }

    t->day = (int)(time);
	t->year = year%2000;
}


/*
**********************************************************************
函数功能： pd是否是闰年
函数形参：None
函数返回值：None
备注：
日期：2022年10月17日
作者：lozloz
版本：V0.0
**********************************************************************
*/

const static int16_t mon_yday[2][12] =
{
{0,31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
{0,31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335},
};

static int isleap(int year)
{
    return (year) % 4 == 0 && ((year) % 100 != 0 || (year) % 400 == 0);
}

uint32_t mktime(T_Time dt, uint8_t u8TimeZone)
{
    uint32_t result;
	int year = dt.year + 2000;
    int i =0;
    // seconds of year
    result = (year - 1970) * 365 * 24 * 3600 +
    (mon_yday[isleap(year)][dt.month-1] + dt.day - 1) * 24 * 3600 +
    dt.hour * 3600 + dt.minute * 60 + dt.second;
    // adjust sencod of lamp
    for(i=1970; i < year; i++)
    {
        if(isleap(i))
        {
            result += 24 * 3600;
        }
    }
	if (u8TimeZone <= 12)
	{
		result -= u8TimeZone*3600L;
	}
	else
	{
		result += (24-u8TimeZone)*3600L;
	}

    return(result);
}

// 判断是否是闰年
int is_leap_year(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

// 每个月的天数（非闰年）
const int days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

// 计算从1970年到给定年份的总天数
int days_from_1970_to_year(int year) {
    int days = 0;
    for (int y = 1970; y < year; y++) {
        days += is_leap_year(y) ? 366 : 365;
    }
    return days;
}

// 计算从1月到给定月份的总天数（考虑闰年）
int days_from_months(int year, int month) {
    int days = 0;
    for (int m = 0; m < month - 1; m++) {
        days += days_in_month[m];
        if (m == 1 && is_leap_year(year)) { // 2月额外一天
            days += 1;
        }
    }
    return days;
}

// 自定义 mktime 实现
unsigned int custom_mktime(struct sys_time *time_data) 
{
    int year = time_data->year;
    int month = time_data->month;
    int day = time_data->day;
    int hour = time_data->hour;
    int min = time_data->min;
    int sec = time_data->sec;

    // 1. 计算年份、月份和天数的总和
    int days = days_from_1970_to_year(year);     // 从1970到当前年的天数
    days += days_from_months(year, month);      // 当前年从1月到当前月的天数
    days += day - 1;                            // 当前月的天数

    // 2. 将天数转为秒数
    unsigned int total_seconds = (unsigned int)days * 24 * 3600;

    // 3. 加上当天的时间部分
    total_seconds += hour * 3600 + min * 60 + sec;

    // printf("total_seconds = %u",total_seconds);

    
    return total_seconds; // 返回 Unix 时间戳（UTC 时间）
}

/*
**********************************************************************
函数功能： 北京time -> unix
函数形参：
函数返回值：None
备注：
日期：2022年10月17日
作者：lozloz
版本：V0.0
**********************************************************************
*/
//u8TimeZone usa-8 = china
uint32_t mkusatime(struct sys_time *dt, uint8_t u8TimeZone)
{
    uint32_t result;
    int i =0;
    // seconds of year
    result = (dt->year - 1970) * 365 * 24 * 3600 +
    (mon_yday[isleap(dt->year)][dt->month-1] + dt->day - 1) * 24 * 3600 +
    dt->hour * 3600 + dt->min * 60 + dt->sec;
    // adjust sencod of lamp
    for(i=1970; i < dt->year; i++)
    {
        if(isleap(i))
        {
            result += 24 * 3600;
        }
    }
	if (u8TimeZone <= 12)
	{
		result -= u8TimeZone*3600L;
	}
	else
	{
		result += (24-u8TimeZone)*3600L;
	}

    // printf("unix = %d",result);
    return(result);
}

const static uint8_t HEX_TABLE[] = "0123456789abcdef";
void hexstr(uint8_t *src, uint8_t *dst, uint8_t len)
{
	uint8_t idx;
	for (idx = 0; idx < len; idx++)
	{
		dst[idx*2] = HEX_TABLE[(src[idx]>>4)&0x0f];
		dst[idx*2 + 1] = HEX_TABLE[src[idx]&0x0f];
	}
	dst[idx*2] = '\0';
}

