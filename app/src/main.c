/**
 * @file main.c
 */


#include <inttypes.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/display.h>

#include <lvgl.h>
#include "lcd_ui.h"

// command to build for screen:
//west build -b nrf52840dk/nrf52840 --shield=adafruit_2_8_tft_touch_v2 app

#include "BTN.h"
#include "LED.h"
#include "memory_game.h"

#define SLEEP_MS 1

// Set display device
static const struct device *display_dev =
    DEVICE_DT_GET(DT_CHOSEN(zephyr_display));


int main(void) {

  // Check proper initialization of BTN, LED, and display
  if (0 > BTN_init()) {
    return 0;
  }

  if (0 > LED_init()){
    return 0;
  }

  if (!device_is_ready(display_dev)) {
    return 0;
  }

  // Initialize LCD
  display_blanking_off(display_dev);
  lcd_ui_init();

  // Initialize Memory Game from .h file
  memory_game_init();
  printk("Memory Game Initialized\n");
 
  while(1){
    
    // Run Memory Game, which is in .c file
    int ret = memory_game_run();
    
    if(0 > ret){
      return 0;
    }

    // Allow LVGL to update screen and process input every loop
    lv_timer_handler();
    k_msleep(SLEEP_MS);

  }

  return 0;
}

