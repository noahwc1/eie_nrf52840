/*
 * main.c
 */

#include <inttypes.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#include "BTN.h"
#include "LED.h"
#include "my_state_machine.h"

#define SLEEP_MS 1
#define LED0_node DT_ALIAS(led0)
#define LED1_node DT_ALIAS(led1)
#define LED2_node DT_ALIAS(led2)
#define LED3_node DT_ALIAS(led3)

static const struct gpio_dt_spec LED0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec LED1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec LED2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
static const struct gpio_dt_spec LED3 = GPIO_DT_SPEC_GET(LED3_NODE, gpios);

int main(void) {

  if (0 > BTN_init()) {
    return 0;
  }
  if (0 > LED_init()) {
    return 0;
  }
  if (!gpio_is_ready_dt(&led0))
    return -1;
  if (!gpio_is_ready_dt(&led1))
    return -1;
  if (!gpio_is_ready_dt(&led2))
      return -1;
    state_machine_init();
  if (!gpio_is_ready_dt(&led3))
      return -1;
  
  int test;
  test=gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
  if (test < 0)
    return test;
  test=gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
  if (test < 0)
    return test;
  test=gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE);
  if (test < 0)
    return test;
  test=gpio_pin_configure_dt(&led3, GPIO_OUTPUT_ACTIVE);
  if (test < 0)
    return test;

  while(1) {
    int ret = state_machine_run();
    if (ret > 0) {
      return 0;
    }

    k_msleep(SLEEP_MS);
  }
	return 0;
}

