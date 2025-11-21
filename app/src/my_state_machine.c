/** 
 * @file my_state_machine.h
 */

#include <zephyr/smf.h>
#include <stdio.h>

#include "LED.h"
#include "my_state_machine.h"
#include "BTN.h"
#include "LED.h"

#define SLEEP_TIME_MS 1000


/*
Function Prototypes
*/
static void start_state(void* o);
static enum smf_state_result start_state_run(void* o);

static void enter_state(void*o);
static enum smf_state_result enter_state_run(void* o);

static void save_state(void*o);
static enum smf_state_result save_state_run(void* o);

static void standby_state(void* o);
static enum smf_state_result standby_state_run(void* o);

/*
Typedefs
*/
enum state_machine_states {
    START_STATE,
    ENTER_STATE,
    SAVE_STATE,
    STANDBY_STATE,
};

typedef struct {
    // context variable used by Zephyr to track state machine state must be first
    struct smf_ctx ctx;
    enum state_machine_states current_state;
    uint16_t count;
} state_object_t;


/*
Local Variables
*/
static const struct smf_state states[] = {
    [START_STATE] = SMF_CREATE_STATE(start_state, start_state_run, NULL, NULL, NULL),
    [ENTER_STATE] = SMF_CREATE_STATE(enter_state, enter_state_run, NULL, NULL, NULL),
    [SAVE_STATE] = SMF_CREATE_STATE(save_state, save_state_run, NULL, NULL, NULL),
    [STANDBY_STATE] = SMF_CREATE_STATE(standby_state, standby_state_run, NULL, NULL, NULL),
};

static state_object_t state_object;

void state_machine_init() {
    state_object.count = 0;
    state_object.current_state = START_STATE;
    smf_set_initial(SMF_CTX(&state_object), &states[START_STATE]);  
    printk("initial state set\n");
 }

 int state_machine_run() {
   return smf_run_state(SMF_CTX(&state_object));
 }

int btn_presses[8] = {0,0,0,0,0,0,0,0};
int saved_string[100] = {};
int saved_number = 0;
int *ptr_btn_presses = btn_presses;
int *ptr_saved_string = saved_string;

struct led_state_object {
    int hold_counter;
}led_state_object;

 /*
 State Functions
 */


 static void start_state(void* o){
    LED_set(LED0, LED_ON);
 }
 static enum smf_state_result start_state_run(void* o){
    state_object.count = 0;
    state_object.current_state = START_STATE;
    if (BTN_check_clear_pressed(BTN0) ) {
        ptr_btn_presses[0] = 0;
        printk("button 0 is pressed, set bit 0");
        LED_set(LED0, LED_ON);
        BTN_clear_pressed(BTN0);
        smf_set_state(SMF_CTX(&state_object), &states[ENTER_STATE]);
        return SMF_EVENT_HANDLED;
    }
    if (BTN_check_clear_pressed(BTN1) ) {
        ptr_btn_presses[0] = 1;
        LED_set(LED1, LED_ON);
        printk("button 1 is pressed, set bit 1");
        BTN_clear_pressed(BTN1);
        smf_set_state(SMF_CTX(&state_object), &states[ENTER_STATE]);
        return SMF_EVENT_HANDLED;
    }

    if (BTN_is_pressed(BTN0) && BTN_is_pressed(BTN1)) {
        led_state_object.hold_counter++;
        if (led_state_object.hold_counter >= 3000){
            led_state_object.hold_counter = 0;
            smf_set_state(SMF_CTX(&state_object), &states[STANDBY_STATE]);
            return SMF_EVENT_HANDLED;
        }
    }
    else
        led_state_object.hold_counter = 0;
    return SMF_EVENT_HANDLED;
 }

 static void enter_state(void* o){}
 static enum smf_state_result enter_state_run(void* o) {
    state_object.count = 1;
    state_object.current_state = ENTER_STATE;
    int btn_total_presses = 1;
    if (BTN_check_clear_pressed(BTN0) ) {
        ptr_btn_presses[btn_total_presses] = 0;
        printk("button 0 is pressed, set bit 0");
        LED_set(LED0, LED_ON);
        BTN_clear_pressed(BTN0);
        btn_total_presses++;
    }
    if (BTN_check_clear_pressed(BTN1) ) {
        ptr_btn_presses[btn_total_presses] = 1;
        LED_set(LED1, LED_ON);
        printk("button 1 is pressed, set bit 1");
        BTN_clear_pressed(BTN1);
        btn_total_presses++;
    }
    if (BTN_check_clear_pressed(BTN2)){
        for (int l = 0; l < 8; l++)
            ptr_btn_presses[l] = 0;
        LED_set(LED2, LED_ON);
        printk("button 2 is pressed, reset the bits");
        saved_number = 0;
        BTN_clear_pressed(BTN2);
        smf_set_state(SMF_CTX(&state_object), &states[START_STATE]);
        return SMF_EVENT_HANDLED;
    }
    if (BTN_check_clear_pressed(BTN3)){
        printk("button 3 is pressed, entering save state");
        smf_set_state(SMF_CTX(&state_object), &states[SAVE_STATE]);
        return SMF_EVENT_HANDLED;
    }
    
    if (btn_total_presses > 7) {
        printk("Total button presses reached");
        smf_set_state(SMF_CTX(&state_object), &states[SAVE_STATE]);      
        return SMF_EVENT_HANDLED;
    }

    if (BTN_is_pressed(BTN0) && BTN_is_pressed(BTN1)) {
        led_state_object.hold_counter++;
        if (led_state_object.hold_counter >= 3000){
            led_state_object.hold_counter = 0;
            smf_set_state(SMF_CTX(&state_object), &states[STANDBY_STATE]);
            return SMF_EVENT_HANDLED;
        }
    }
    else
        led_state_object.hold_counter = 0;

    LED_set(LED3, LED_ON);
    k_msleep(0.5*SLEEP_TIME_MS);
    LED_set(LED3, LED_OFF);
    LED_set(LED0, LED_OFF);
    LED_set(LED1, LED_OFF);
    LED_set(LED2, LED_OFF);
    k_msleep(0.5*SLEEP_TIME_MS);

return SMF_EVENT_HANDLED;
 }

static void save_state(void* o){
    for (int k = 0; k < 8; k++){
        ptr_saved_string[saved_number] += ptr_btn_presses[k];
    }
}
static enum smf_state_result save_state_run(void* o){
    state_object.count = 2;
    state_object.current_state = SAVE_STATE;
    if (BTN_check_clear_pressed(BTN2)){
        printk("button 2 pressed, string deleted\n");
        saved_number--;
        smf_set_state(SMF_CTX(&state_object), &states[START_STATE]);
        return SMF_EVENT_HANDLED;

    }
    if (BTN_check_clear_pressed(BTN3)){
        printk("button 3 pressed, sending string to serial monitor\n");
        int len = sizeof(saved_string) / sizeof(saved_string[0]);

        for (int i = 0; i < len/8; i++){
            int number = 0;
            for (int j = 0; j < 8; j++)
                number |= (saved_string[j] << j);
            printk("%c", number);
        }
        printk("\n");
    }
    
    if (BTN_is_pressed(BTN0) && BTN_is_pressed(BTN1)) {
        led_state_object.hold_counter++;
        if (led_state_object.hold_counter >= 3000){
            led_state_object.hold_counter = 0;
            smf_set_state(SMF_CTX(&state_object), &states[STANDBY_STATE]);
            return SMF_EVENT_HANDLED;
        }
    }
    else
        led_state_object.hold_counter = 0; 
    LED_set(LED3, LED_ON);
    k_msleep(0.25*SLEEP_TIME_MS);
    LED_set(LED3, LED_OFF);
    return SMF_EVENT_HANDLED;  
}

static void standby_state(void* o){}
static enum smf_state_result standby_state_run(void* o){
    state_object.count = 3;
    int brightness = 0;
    int direction = 1;

    if (brightness >= 100){
        brightness = 100;
        direction = -1;
    }
    else if (brightness <=0){
        brightness = 0;
        direction = 1;
    }
    brightness += direction * 10;

    LED_pwm(LED0, brightness);
    LED_pwm(LED1, brightness);
    LED_pwm(LED2, brightness);
    LED_pwm(LED3, brightness);
    if (BTN_check_clear_pressed(BTN0)||BTN_check_clear_pressed(BTN1)||BTN_check_clear_pressed(BTN2)||BTN_check_clear_pressed(BTN3)){
        smf_set_state(SMF_CTX(&state_object), &states[state_object.current_state]);
        return SMF_EVENT_HANDLED;
    }
    k_msleep(SLEEP_TIME_MS);
    return SMF_EVENT_HANDLED;
    }



//  static void led_off_state_entry(void*o) {
//     LED_set(LED0, LED_OFF);
//  }

//  static enum smf_state_result led_off_state_run(void* o){
//     LED_set(LED2, LED_ON);
//     if (led_state_object.count > 500){
//         // led_state_object.count = 0;
//         smf_set_state(SMF_CTX(&led_state_object), &led_states[LED_ON_STATE]);
    
//     } else {
//         led_state_object.count++;
//         printk("LED OFF State Count: %d\n", led_state_object.count);
//     }
//     return SMF_EVENT_HANDLED;
//  }

// static void led_exit_off_state_entry(void* o) {
//     led_state_object.count = 0;
  
//  }
// static void led_exit_on_state_entry(void* o) {
//     led_state_object.count = 0;
   
//  }