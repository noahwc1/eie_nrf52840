/** 
 * @file lcd_ui.c
 */

 
#include <lvgl.h>
#include "lcd_ui.h"
#include "lv_data_obj.h"


static lv_obj_t *status_label;

static lv_obj_t *led_regions[NUM_LEDS];

static lv_obj_t *screen = NULL;



void lv_button_callback(lv_event_t *event)
{
    lv_obj_t *data_obj = (lv_obj_t *) lv_event_get_user_data(event);
    led_id led = *(led_id *)lv_data_obj_get_data_ptr(data_obj);

    LED_toggle(led);
}

void lcd_ui_init(void)
{
    screen = lv_screen_active();

    if (screen == NULL) {
        return;
    }
     for (uint8_t i = 0; i < NUM_LEDS; i++) {

        led_regions[i] = lv_obj_create(screen);

        lv_obj_set_size(led_regions[i], 220, 150);

        lv_obj_align(led_regions[i],
                     LV_ALIGN_CENTER,
                     120 * (i % 2 ? 1 : -1),
                     80 * (i < 2 ? -1 : 1));

        lv_obj_set_style_bg_color(led_regions[i],
                                  lv_color_hex(0x303030),
                                  0);
    }
    
    // for (uint8_t i = 0; i < NUM_LEDS; i++) {

    //     lv_obj_t *ui_btn = lv_button_create(screen);

    //     lv_obj_align(ui_btn,
    //                  LV_ALIGN_CENTER,
    //                  70 * (i % 2 ? 1 : -1),
    //                  30 * (i < 2 ? -1 : 1));

    //     lv_obj_t *button_label = lv_label_create(ui_btn);

    //     char label_text[10];
    //     snprintf(label_text, sizeof(label_text), "LED %d", i);

    //     lv_label_set_text(button_label, label_text);
    //     lv_obj_center(button_label);

    //     led_id led = (led_id)i;

    //     lv_obj_t *data_obj =
    //         lv_data_obj_create_alloc_assign(ui_btn, &led, sizeof(led_id));

    //     lv_obj_add_event_cb(ui_btn,
    //                         lv_button_callback,
    //                         LV_EVENT_CLICKED,
    //                         data_obj);
    // }
}

void lcd_set_led(led_id led, int on)
{
    if (led >= NUM_LEDS) return;

    if (on) {
        lv_obj_set_style_bg_color(
            led_regions[led],
            lv_color_hex(0xFF0000),
            0);
    } else {
        lv_obj_set_style_bg_color(
            led_regions[led],
            lv_color_hex(0x303030),
            0);
    }
}

void lcd_set_status(const char *text)
{
    lv_label_set_text(status_label, text);
}