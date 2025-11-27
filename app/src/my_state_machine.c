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

static void display_state(void* o);
static enum smf_state_result display_state_run(void* o);



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
    DISPLAY_STATE,
    ENTER_STATE,
    SAVE_STATE,
    STANDBY_STATE,
};

typedef struct {
    // context variable used by Zephyr to track state machine state must be first
    struct smf_ctx ctx;
    enum state_machine_states current_state;
    uint16_t count;
    
    int led3_on;
    int64_t last_toggle_ms;

    int64_t button_time;

} state_object_t;


/*
Local Variables
*/
static const struct smf_state states[] = {
    [DISPLAY_STATE] = SMF_CREATE_STATE(display_state, display_state_run, NULL, NULL, NULL),
    [ENTER_STATE] = SMF_CREATE_STATE(enter_state, enter_state_run, NULL, NULL, NULL),
    [SAVE_STATE] = SMF_CREATE_STATE(save_state, save_state_run, NULL, NULL, NULL),
    [STANDBY_STATE] = SMF_CREATE_STATE(standby_state, standby_state_run, NULL, NULL, NULL),
};

static state_object_t state_object;

void state_machine_init() {
    state_object.count = 1;
    state_object.current_state = ENTER_STATE;
    smf_set_initial(SMF_CTX(&state_object), &states[ENTER_STATE]);  
    printk("initial state set\n");
 }

 int state_machine_run() {
   return smf_run_state(SMF_CTX(&state_object));
 }

int btn_presses[8] = {0,0,0,0,0,0,0,0};
int btn_total_presses = 0;
int saved_string[100] = {};
int saved_number = 0;
int *ptr_btn_presses = btn_presses;
int *ptr_saved_string = saved_string;
int brightness = 0;
int direction = 1;
int m;

struct led_state_object {
    int hold_counter;
}led_state_object;

 /*
 State Functions
 */


 static void display_state(void* o){
    state_object.count = 0;
    state_object.current_state = DISPLAY_STATE;

    state_object.led3_on = 0;
    state_object.last_toggle_ms = k_uptime_get();
    state_object.button_time = k_uptime_get();
 
 }
 static enum smf_state_result display_state_run(void* o){
    
    int64_t now = k_uptime_get();
    

    if ((now - state_object.last_toggle_ms) >= 32) {
        state_object.last_toggle_ms = now;
        state_object.led3_on = !state_object.led3_on;
    }
    int64_t now2 = k_uptime_get();
    if ((now2 - state_object.button_time) >= 500) {
        state_object.button_time = now2;
        LED_set(LED0, LED_OFF);
        LED_set(LED1, LED_OFF);
        LED_set(LED2, LED_OFF);
    }

    LED_set(LED3, state_object.led3_on ? LED_ON : LED_OFF);
    
     if (BTN_is_pressed(BTN0) && BTN_is_pressed(BTN1)) {
        led_state_object.hold_counter++;
        if (led_state_object.hold_counter >= 1000){
            led_state_object.hold_counter = 0;
            smf_set_state(SMF_CTX(&state_object), &states[STANDBY_STATE]);
            return SMF_EVENT_HANDLED;
        }
     }
    else
        led_state_object.hold_counter = 0;


    if (BTN_check_clear_pressed(BTN2)){
        for (int l = 0; l < 100; l++)
            saved_string[l] = 0;
        LED_set(LED2, LED_ON);
        printk("button 2 is pressed, reset all the bits, enter state\n");
        saved_number = 0;
        BTN_clear_pressed(BTN2);
        smf_set_state(SMF_CTX(&state_object), &states[ENTER_STATE]);
        return SMF_EVENT_HANDLED;
    }
    if (BTN_check_clear_pressed(BTN3)){
        printk("button 3 pressed, sending string to serial monitor\n");
        for (m = 0; m < saved_number*8; m++){
            printk("%d ", ptr_saved_string[m]);
            if (m%8) printk("-");
        }
        printk("\n");


        for (int i = 0; i < saved_number; i++) {
            uint8_t number = 0;
            for (int j = 0; j < 8; j++) {
                    number |= (ptr_saved_string[j + i*8] & 1) << (j); // MSB first
            }
        printk("%c", number);
    }
    printk("\n");
    }
    
    return SMF_EVENT_HANDLED;
 }

 static void enter_state(void* o){
    state_object.count = 1;
    state_object.current_state = ENTER_STATE;
    state_object.led3_on = 0;
    state_object.last_toggle_ms = k_uptime_get();
    state_object.button_time = k_uptime_get();
 }

 static enum smf_state_result enter_state_run(void* o) {
    
    int64_t now = k_uptime_get();

    if ((now - state_object.last_toggle_ms) >= 500) {
        state_object.last_toggle_ms = now;
        state_object.led3_on = !state_object.led3_on;
    }
    LED_set(LED3, state_object.led3_on ? LED_ON : LED_OFF);
    
    int64_t now2 = k_uptime_get();
    if ((now2 - state_object.button_time) >= 500) {
        state_object.button_time = now2;
        LED_set(LED0, LED_OFF);
        LED_set(LED1, LED_OFF);
        LED_set(LED2, LED_OFF);
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

    
    if (btn_total_presses>7){
        printk("max string characters reached entering save state\n");
        smf_set_state(SMF_CTX(&state_object), &states[SAVE_STATE]);
        return SMF_EVENT_HANDLED;
    }

    if (BTN_check_clear_pressed(BTN0) ) {
        ptr_btn_presses[btn_total_presses] = 0;
        btn_total_presses++;
        printk("button 0 is pressed, set bit 0\n next: ");
        for (m = 0; m < 8; m++){
            if (m==btn_total_presses) printk("-");
            printk("%d", ptr_btn_presses[m]);
            if (m==btn_total_presses) printk("-");
        }
        printk("\n");
        LED_set(LED0, LED_ON);
        BTN_clear_pressed(BTN0);
        return SMF_EVENT_HANDLED;
        
    }
    if (BTN_check_clear_pressed(BTN1) ) {
        ptr_btn_presses[btn_total_presses] = 1;
        btn_total_presses++;
        LED_set(LED1, LED_ON);
        printk("button 1 is pressed, set bit 1\n next: ");
        for (m = 0; m < 8; m++){
            if (m==btn_total_presses) printk("-");
            printk("%d", ptr_btn_presses[m]);
            if (m==btn_total_presses) printk("-");
        }
        printk("\n");
        BTN_clear_pressed(BTN1);
        return SMF_EVENT_HANDLED;
       
    }
    if (BTN_check_clear_pressed(BTN2)){
        for (int l = 0; l < 8; l++)
            ptr_btn_presses[l] = 0;
        LED_set(LED2, LED_ON);
        printk("button 2 is pressed, reset the current bits\n");
        
        btn_total_presses = 0;
        BTN_clear_pressed(BTN2);
        smf_set_state(SMF_CTX(&state_object), &states[ENTER_STATE]);
        return SMF_EVENT_HANDLED;
    }
    if (BTN_check_clear_pressed(BTN3)){
        printk("button 3 is pressed, entering save state\n");
        smf_set_state(SMF_CTX(&state_object), &states[SAVE_STATE]);
        return SMF_EVENT_HANDLED;
    }
    

    
    return SMF_EVENT_HANDLED;
}

static void save_state(void* o){
    state_object.count = 2;
    state_object.current_state = SAVE_STATE;
    state_object.led3_on = 0;
    state_object.last_toggle_ms = k_uptime_get();
    state_object.button_time = k_uptime_get();

    for (int k = 0; k < 8; k++){
        ptr_saved_string[k+saved_number*8] = ptr_btn_presses[k] ? 1 : 0;
        ptr_btn_presses[k] = 0;
    }
    btn_total_presses = 0;
    saved_number++;
    

}
static enum smf_state_result save_state_run(void* o){
    
    int64_t now = k_uptime_get();

    if ((now - state_object.last_toggle_ms) >= 125) {
        state_object.last_toggle_ms = now;
        state_object.led3_on = !state_object.led3_on;
    }
    LED_set(LED3, state_object.led3_on ? LED_ON : LED_OFF);

    int64_t now2 = k_uptime_get();
    if ((now2 - state_object.button_time) >= 500) {
        state_object.button_time = now2;
        LED_set(LED0, LED_OFF);
        LED_set(LED1, LED_OFF);
        LED_set(LED2, LED_OFF);
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

    if ((BTN_check_clear_pressed(BTN0) || (BTN_check_clear_pressed(BTN1)))){
        printk("entering enter state, string saved\n");
        smf_set_state(SMF_CTX(&state_object), &states[ENTER_STATE]);
    }
    if (BTN_check_clear_pressed(BTN2)){
        printk("button 2 pressed, whole string deleted\n");
        for (int j = 0; j < saved_number; j++){
            for (int k = 0; k < 8; k++){
                    ptr_saved_string[k+j*8] += 0;
            }
        }
        saved_number = 0;
        smf_set_state(SMF_CTX(&state_object), &states[ENTER_STATE]);
        return SMF_EVENT_HANDLED;
    }
    if (BTN_check_clear_pressed(BTN3)){
        printk("saving entire string\n");
        smf_set_state(SMF_CTX(&state_object), &states[DISPLAY_STATE]);
        return SMF_EVENT_HANDLED;
    }
    return SMF_EVENT_HANDLED;  
}

static void standby_state(void* o){
    state_object.count = 3;
    brightness = 0;
    direction = 1;
    state_object.button_time = k_uptime_get();
    
}
static enum smf_state_result standby_state_run(void* o){
    int64_t now3 = k_uptime_get();

    if ((now3 - state_object.button_time) >= 125) {
        
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
        
        state_object.button_time = now3;
        }
        
    
    if (BTN_check_clear_pressed(BTN0)||BTN_check_clear_pressed(BTN1)||BTN_check_clear_pressed(BTN2)||BTN_check_clear_pressed(BTN3)){
            smf_set_state(SMF_CTX(&state_object), &states[state_object.current_state]);
            return SMF_EVENT_HANDLED;
    }
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