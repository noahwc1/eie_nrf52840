/*
 * main.c
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>
#include <inttypes.h>

#include "LED.h"
#include "BTN.h"

#define  SLEEP_TIME_MS   1

int main(void)
{
    if (0 > LED_init()) {
        return 0;
    }
    if (0 > BTN_init()) {
        return 0;
    }
    while (1) {
        if (BTN_check_clear_pressed(BTN0)){
            LED_toggle(LED0);
            printk("Button 0 pressed\n");
        }
        k_msleep(SLEEP_TIME_MS);
    }
    return 0;
}