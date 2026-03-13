/**
 * @file main.c
 */

#include <inttypes.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#include "BTN.h"
#include "LED.h"

// #include <lvgl.h>
// #include "lv_data_obj.h"
#include <zephyr/drivers/display.h>

#include <lvgl.h>
#include "lcd_ui.h"

#include "memory_game.h"

#define SLEEP_MS 1


static const struct device *display_dev =
    DEVICE_DT_GET(DT_CHOSEN(zephyr_display));




// command to build for screen:
//west build -b nrf52840dk/nrf52840 --shield=adafruit_2_8_tft_touch_v2 app



//enabling touch controller via SPI


// #define ARDUINO_SPI_NODE DT_NODELABEL(arduino_spi)
// #define ZEPHYR_USER_NODE DT_PATH(zephyr_user)

// //commands needed for LCD initialization and operation
// #define CMD_SOFTWARE_RESET 0x01
// #define CMD_SLEEP_OUT 0x11
// #define CMD_DISPLAY_ON 0x29
// #define CMD_COLUMN_ADDRESS_SET 0x2A
// #define CMD_ROW_ADDRESS_SET 0x2B
// #define CMD_MEMORY_WRITE 0x2C

// static const struct gpio_dt_spec dcx_gpio = GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, dcx_gpios);
// static const struct spi_cs_control cs_ctrl = (struct spi_cs_control){
//   .gpio = GPIO_DT_SPEC_GET(ARDUINO_SPI_NODE, cs_gpios),
//   .delay = 1u,  // delay for lcd hold time
// };

// static const struct device * dev = DEVICE_DT_GET(ARDUINO_SPI_NODE ); // might want SPI_DT_SPEC_GET here
// static const struct spi_config spi_cfg = {
//   .frequency = 1000000, // sets clock frequency of communication to 1 MHz
//   .operation = SPI_OP_MODE_MASTER | SPI_WORD_SET(8) | SPI_TRANSFER_MSB,
//   .slave = 0, 
//   .cs = cs_ctrl
// };

// //helper function to send command and data to LCD
// static void lcd_cmd(uint8_t cmd, struct spi_buf * data);

// static void lcd_cmd(uint8_t cmd, struct spi_buf * data) {
//   struct spi_buf cmd_buf[1] = {[0] ={.buf = &cmd, .len = 1}};
//   struct spi_buf_set cmd_set = {.buffers = cmd_buf, .count = 1};

//   // D/C select must be low to send command
//   gpio_pin_set_dt(&dcx_gpio, 0);

//   spi_write(dev, &spi_cfg, &cmd_set);

//   if (data != NULL) {
//     struct spi_buf_set data_set = {.buffers = data, .count = 1};

//     // D/C select must be high to send data
//     gpio_pin_set_dt(&dcx_gpio,  1);

//     spi_write(dev, &spi_cfg, &data_set);
//   }
// }


int main(void) {

  if (0 > BTN_init()) {
    return 0;
  }

  if (0 > LED_init()){
    return 0;
  }

  if (!device_is_ready(display_dev)) {
    return 0;
  }

  // screen = lv_screen_active();
  // if (screen == NULL) {
  //   return 0;
  // }
  
  // for (uint8_t i = 0; i < NUM_LEDS; i++) {
  //   lv_obj_t *ui_btn = lv_button_create(screen);
  //   lv_obj_align(ui_btn, LV_ALIGN_CENTER, 70 * (i % 2 ? 1 : -1), 30 * (i < 2 ? -1 : 1));
  //   lv_obj_t *button_label = lv_label_create(ui_btn);
  //   char label_text[10];
  //   snprintf(label_text, 10, "LED %d", i);
  //   lv_label_set_text(button_label, label_text);
  //   lv_obj_align(button_label, LV_ALIGN_CENTER, 0, 0);

  //   led_id led = (led_id)i;
  //   lv_obj_t *data_obj = lv_data_obj_create_alloc_assign(ui_btn, &led, sizeof(led_id));
  //   lv_obj_add_event_cb(ui_btn, lv_button_callback, LV_EVENT_CLICKED, data_obj);
  // }

  
  display_blanking_off(display_dev);
  lcd_ui_init();

  memory_game_init();
  printk("Memory Game Initialized\n");
  while(1){
    
    int ret = memory_game_run();
    if(0 > ret){
      return 0;
    }
    lv_timer_handler();

    k_msleep(SLEEP_MS);

  }
  return 0;



  // LCD Initialization Sequence
  // lcd_cmd( CMD_SOFTWARE_RESET, NULL);
  // k_msleep(120); // wait for 120 ms

  // lcd_cmd( CMD_SLEEP_OUT,  NULL);
  // lcd_cmd( CMD_DISPLAY_ON, NULL);

  // //test square

  // uint8_t column_data[] = {[0] = 0x00, [1] = 0x95, [2] = 0x00, [3] = 0x9f}; // column 149 to 159
  // uint8_t row_data[] = {[0] = 0x00, [1] = 0x75, [2] = 0x00, [3] = 0x7f};    // row 117 to 127
  
  // uint8_t color_data[300];
  // for (int i =0; i < 300; i+=3){
  //   color_data[i] = 0xFC;
  //   color_data[i+1] = 0;
  //   color_data[i+2] = 0;
  // }

  // struct spi_buf column_data_buf = {.buf = column_data, .len=4};
  // struct spi_buf row_data_buf = {.buf = row_data, .len=4};
  // struct spi_buf color_data_buf = {.buf = color_data, .len=300};

  // lcd_cmd( CMD_COLUMN_ADDRESS_SET, &column_data_buf);
  // lcd_cmd( CMD_ROW_ADDRESS_SET, &row_data_buf);
  // lcd_cmd( CMD_MEMORY_WRITE,  &color_data_buf);

  // while (1) {
  //   k_msleep(SLEEP_MS);
  // }
  // return 0;
}

