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

#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)
#define LED3_NODE DT_ALIAS(led3)

static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(LED3_NODE, gpios);
int main(void)
{
    if (0 > LED_init()) {
        return 0;
    }
    if (0 > BTN_init()) 
        return 0;
    if (!gpio_is_ready_dt(&led0) || !gpio_is_ready_dt(&led1) ||
        !gpio_is_ready_dt(&led2) || !gpio_is_ready_dt(&led3)){
            printk("Error: LED device %s is not ready\n", led0.port->name);
            return -1;
        }
    int ret;
    int ret1;
    int ret2;
    int ret3;
    
    ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
    if (ret < 0) {
        printk("Error %d: failed to configure LED pin %d\n",
               ret, led0.pin);
        return ret;
    }
    ret1 = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
    if (ret1 < 0) {
        printk("Error %d: failed to configure LED pin %d\n",
               ret, led1.pin);
        return ret1;
    }
    ret2 = gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE);
    if (ret2 < 0) {
        printk("Error %d: failed to configure LED pin %d\n",
               ret, led2.pin);
        return ret2;
    }
    ret3 = gpio_pin_configure_dt(&led3, GPIO_OUTPUT_ACTIVE);
    if (ret3 < 0) {
        printk("Error %d: failed to configure LED pin %d\n",
               ret, led3.pin);
        return ret3;
    }
    int i = 0;
    while (1) {
        int dig0, dig1, dig2, dig3;
        if (BTN_check_clear_pressed(BTN0)){
            printk("Button 0 pressed\n");

            gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);
            gpio_pin_configure_dt(&led1, GPIO_OUTPUT_INACTIVE);
            gpio_pin_configure_dt(&led2, GPIO_OUTPUT_INACTIVE);
            gpio_pin_configure_dt(&led3, GPIO_OUTPUT_INACTIVE);

            i += 1;
            if (i >15)
                i = 0;
            dig0 = i%2;
            dig1 = (i/2)%2;
            dig2 = ((i/2)/2)%2;
            dig3 = (((i/2)/2)/2)%2;
            
            if (dig0 == 1) 
                gpio_pin_toggle_dt(&led0);
            
            if (dig1 == 1)
                gpio_pin_toggle_dt(&led1);
            
            if (dig2 == 1)
                gpio_pin_toggle_dt(&led2);
            
            if (dig3 == 1)
                gpio_pin_toggle_dt(&led3);      

            printk("LED state: %d%d%d%d\n", dig3, dig2, dig1, dig0);
            
            k_msleep(SLEEP_TIME_MS);
        }
    }
    return 0;  
    }