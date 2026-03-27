/** 
 * @file lcd_ui.c
 */

 // This file implements the LCD user interface for the Memory Game. 
 // It uses the LVGL library to create a simple UI with a status label and four LED regions that can be toggled on and off.
 // The UI also includes event handling for button presses on the LCD, which are integrated with the game logic in memory_game.c.

#include <lvgl.h>

#include "lv_data_obj.h"
#include "BTN.h"
#include "LED.h"

#include "memory_game.h"
#include "lcd_ui.h"

// Forward declarations for internal functions
static lv_obj_t *status_label;

static lv_obj_t *led_regions[NUM_LEDS];

static lv_obj_t *screen = NULL;



// Initialize the LCD UI, creating the status label and LED regions
void lcd_ui_init(void)
{
    screen = lv_screen_active();
    
    if (screen == NULL) {
        return;
    }

    status_label = lv_label_create(screen);

    

     for (uint8_t i = 0; i < NUM_LEDS; i++) {

        led_regions[i] = lv_obj_create(screen);

        lv_obj_set_size(led_regions[i], 120, 80);   // 220 150
 
        lv_obj_align(led_regions[i],
                     LV_ALIGN_CENTER,
                     120 * (i % 2 ? 1 : -1),
                     80 * (i < 2 ? -1 : 1));

        lv_obj_set_style_bg_color(led_regions[i],
                                  lv_color_hex(0x303030),
                                  0);
        
        led_id led = (led_id)i;                          
        lv_obj_t *data_obj = lv_data_obj_create_alloc_assign(led_regions[i], &led, sizeof(led_id));
        lv_obj_add_event_cb(led_regions[i], lv_button_callback, LV_EVENT_CLICKED, data_obj);
        
    }
}

// Callback function for an LCD touchscreen button press event
void lv_button_callback(lv_event_t *event)
{
    lv_obj_t *data_obj = (lv_obj_t *) lv_event_get_user_data(event);
    led_id led = *(led_id *)lv_data_obj_get_data_ptr(data_obj);

    memory_game_set_lcd_button(led);


    lv_label_set_text(status_label, "Memory Game");
    lv_obj_align(status_label, LV_ALIGN_CENTER, 0, 10);
    lv_obj_set_style_text_color(status_label, lv_color_hex(0x000000), 0);
    lv_obj_set_style_text_font(status_label, &lv_font_montserrat_14, 0);

    lv_obj_move_foreground(status_label);
}

// Function to set the state of an LED on the LCD
void lcd_set_led(led_id led, int on, enum led_colours colour)
{
    if (led >= NUM_LEDS) return;

    if (on) {
        if (colour == RED) {
            lv_obj_set_style_bg_color(
                led_regions[led],
                lv_color_hex(0xFF0000),
                0);
        }
        else if (colour == GREEN) {
            lv_obj_set_style_bg_color(
            led_regions[led],
            lv_color_hex(0x00FF00),
            0);
        }
        else if (colour == BLUE) {
            lv_obj_set_style_bg_color(
            led_regions[led],
            lv_color_hex(0x0000FF),
            0);
        }
        else { // ORANGE
            lv_obj_set_style_bg_color(
            led_regions[led],
            lv_color_hex(0xCC5500),
            0);
        }
    } else {
        lv_obj_set_style_bg_color(
            led_regions[led],
            lv_color_hex(0x303030),
            0);
    }
    lv_obj_move_foreground(status_label);
}

// Function for the winner state display for LCD
void lcd_green_led(){
        for(int i = 0; i < NUM_LEDS; i++){
            lv_obj_set_style_bg_color(
                led_regions[i],
                lv_color_hex(0x00FF00),
                0);
        }
        lv_obj_move_foreground(status_label);
}

// Function to set the text on the LCD, always bringing it to the foreground
void lcd_set_status(const char *text)
{
    if (!status_label) return;

    lv_label_set_text(status_label, text);

    lv_obj_align(status_label, LV_ALIGN_CENTER, 0, 0);
    
    lv_obj_move_foreground(status_label);
   

}