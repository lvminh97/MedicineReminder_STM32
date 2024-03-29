#ifndef _DS3231_H
#define _DS3231_H

#include "main.h"


#define DS3231_I2C_ADDR 	0x68

#define DS3231_REG_SECOND 	0x00
#define DS3231_REG_MINUTE 	0x01
#define DS3231_REG_HOUR  	0x02
#define DS3231_REG_DOW    	0x03

#define DS3231_REG_DATE   	0x04
#define DS3231_REG_MONTH  	0x05
	#define DS3231_CENTURY 		7
#define DS3231_REG_YEAR   	0x06

#define DS3231_A1_SECOND	0x07
#define DS3231_A1_MINUTE	0x08
#define DS3231_A1_HOUR		0x09
#define DS3231_A1_DATE		0x0a

#define DS3231_A2_MINUTE	0x0b
#define DS3231_A2_HOUR		0x0c
#define DS3231_A2_DATE		0x0d

#define DS3231_AXMY			7
#define DS3231_DYDT			6

#define DS3231_REG_CONTROL 	0x0e
	#define DS3231_EOSC			7
	#define DS3231_BBSQW		6
	#define DS3231_CONV			5
	#define DS3231_RS2			4
	#define DS3231_RS1			3
	#define DS3231_INTCN		2
	#define DS3231_A2IE			1
	#define DS3231_A1IE			0

#define DS3231_REG_STATUS	0x0f
	#define DS3231_OSF			7
	#define DS3231_EN32KHZ		3
	#define DS3231_BSY			2
	#define DS3231_A2F			1
	#define DS3231_A1F			0

#define DS3231_AGING		0x10

#define DS3231_TEMP_MSB		0x11
#define DS3231_TEMP_LSB		0x12

#define DS3231_TIMEOUT		HAL_MAX_DELAY
/*----------------------------------------------------------------------------*/
typedef enum DS3231_Rate{
	DS3231_1HZ, DS3231_1024HZ, DS3231_4096HZ, DS3231_8192HZ
} DS3231_Rate;

typedef enum DS3231_InterruptMode{
	DS3231_SQUARE_WAVE_INTERRUPT, DS3231_ALARM_INTERRUPT
} DS3231_InterruptMode;

typedef enum DS3231_State{
	DS3231_DISABLED, DS3231_ENABLED
} DS3231_State;

typedef enum {
	SUNDAY = 1, MONDAY, TUESDAY, WEDNESDAY, THURSDAY, FRIDAY, SATURDAY
} RTC_DOW;

typedef enum {
	JANUARY = 1,
	FEBRUARY,
	MARCH,
	APRIL,
	MAY,
	JUNE,
	JULY,
	AUGUST,
	SEPTEMBER,
	OCTOBER,
	NOVEMBER,
	DECEMBER
} RTC_Month;

typedef struct rtc_tm {
	uint8_t seconds;
	uint8_t minutes;
	uint8_t hours;
	RTC_DOW dow;
	uint8_t day;
	RTC_Month month;
	uint8_t year;
} RTC_Time;

extern I2C_HandleTypeDef *_ds3231_ui2c;

void DS3231_Init(I2C_HandleTypeDef *hi2c);

void DS3231_SetRegByte(uint8_t regAddr, uint8_t val);
uint8_t DS3231_GetRegByte(uint8_t regAddr);

uint8_t DS3231_GetDayOfWeek(void);
uint8_t DS3231_GetDate(void);
uint8_t DS3231_GetMonth(void);
uint16_t DS3231_GetYear(void);

uint8_t DS3231_GetHour(void);
uint8_t DS3231_GetMinute(void);
uint8_t DS3231_GetSecond(void);

void DS3231_SetDayOfWeek(uint8_t dow);
void DS3231_SetDate(uint8_t date);
void DS3231_SetMonth(uint8_t month);
void DS3231_SetYear(uint16_t year);

void DS3231_SetHour(uint8_t hour_24mode);
void DS3231_SetMinute(uint8_t minute);
void DS3231_SetSecond(uint8_t second);

void DS3231_SetFullTime(uint8_t hour_24mode, uint8_t minute, uint8_t second);
void DS3231_SetFullDate(uint8_t date, uint8_t month, uint8_t dow, uint16_t year);
void DS3231_GetFullDateTime(RTC_Time *rtc_time);

uint8_t DS3231_DecodeBCD(uint8_t bin);
uint8_t DS3231_EncodeBCD(uint8_t dec);

int8_t DS3231_GetTemperatureInteger();
uint8_t DS3231_GetTemperatureFraction();

#endif
