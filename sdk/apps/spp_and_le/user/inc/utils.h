#ifndef __UTILS_H__
#define __UTILS_H__

#include "system/sys_time.h"
#include "app_config.h"
#include "system/includes.h"
#include "asm/includes.h"

#include <stdint.h>

typedef struct _time
{
	uint8_t year;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
	int8_t timezone;
}T_Time,*pT_Time;

uint32_t mktime(T_Time dt, uint8_t u8TimeZone);
void localtime(uint32_t time, pT_Time t);
uint32_t mkusatime(struct sys_time *dt, uint8_t u8TimeZone);
unsigned int custom_mktime(struct sys_time *time_data); 

void hexstr(uint8_t *src, uint8_t *dst, uint8_t len);

#endif
