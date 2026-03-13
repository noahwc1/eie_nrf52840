/** 
 * @file lcd_ui.h
 */

#ifndef LCD_UI_H
#define LCD_UI_H
#include "LED.h"

void lcd_ui_init(void);
void lcd_set_led(led_id led, int on);
void lcd_set_status(const char *text);

#endif