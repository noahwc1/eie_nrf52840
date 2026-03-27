/** 
 * @file lcd_ui.h
 */

#ifndef LCD_UI_H
#define LCD_UI_H

#include <lvgl.h>
#include "LED.h"

enum led_colours {
    RED,
    GREEN,
    BLUE,
    ORANGE,
    GREY,
};

void lcd_ui_init(void);
void lcd_set_led(led_id led, int on, enum led_colours colour);
void lcd_set_status(const char *text);
void lcd_green_led();
void lv_button_callback(lv_event_t *event);

#endif