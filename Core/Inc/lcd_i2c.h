#ifndef _LCD_I2C_H
#define _LCD_I2C_H

#include "main.h"

void lcd_init (void);   // initialize lcd

void lcd_send_cmd (char cmd);  // send command to the lcd

void lcd_send_data (char data);  // send data to the lcd

void lcd_send_string (char *str);  // send string to the lcd

void lcd_clear_display (void);	//clear display lcd

void lcd_gotoxy (int row, int col); //set proper location on screen

#endif
