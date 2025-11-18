/** 
 * @file my_state_machine.h
 */

#include <zephyr/smf.h>

#include "LED.h"
#include "my_state_machine.h"

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
enum led_state_machine_states {
    START_STATE,
    ENTER_STATE,
    SAVE_STATE,
    STANDBY_STATE
};

typedef struct {
    // context variable used by Zephyr to track state machine state must be first
    struct smf_ctx ctx;

    uint16_t count;
} state_object_t;


/*
Local Variables
*/
static const struct smf_state states[] = {
    [START_STATE] = SMF_CREATE_STATE(start_state, start_state_run, NULL, NULL, NULL),
    [ENTER_STATE] = SMF_CREATE_STATE(enter_state, enter_state_run, NULL, NULL, NULL),
    [SAVE_STATE] = SMF_CREATE_STATE(save_state_run, save_state_run, NULL, NULL, NULL),
    [STANDBY_STATE] = SMF_CREATE_STATE(standby_state, standby_state_run, NULL, NULL, NULL),
};

static state_object_t state_object;

void state_machine_init() {
    led_state_object.count = 0;
    smf_set_initial(SMF_CTX(&state_object), &states[START_STATE]);
    
 }

 int state_machine_run() {
   return smf_run_state(SMF_CTX(&state_object));
 }

 /*
 State Functions
 */

 static void enter_state(void* o){
    btn_presses[8] = {0,0,0,0,0,0,0,0};
 }

 static enum smf_state_result  enter_state_run(void* o) {
    int btn_total_presses = 0;
    while(1) {
        if (BTN_isPressed(BTN0) ) {
            btn_presses[btn_total_presses] = 0;
            printk("button 0 is pressed, set bit 0");
            LED_set(LED0, LED_ON);
            BTN_clearPress(BTN0);
            btn_presses++;
        }
        if (BTN_isPressed(BTN1) ) {
            btn_presses[btn_total_presses] = 1;
            LED_set(LED1, LED_ON);
            printk("button 1 is pressed, set bit 1");
            BTN_clearPress(BTN1);
            btn_presses++;
        }
        if (BTN_isPressed(BTN2)){
            btn_presses[8] = {0,0,0,0,0,0,0,0};
            LED_set(LED2, LED_ON);
            printk("button 2 is pressed, reset the bits");
            BTN_clearPress(BTN2);
        }
        if (BTN_isPressed(BTN3)){
            printk("button 3 is pressed, entering save state")
            return smf_state_exit;
        }
        
        if (btn_total_presses > 7) {
            printk("Total button presses reached")
            smf_set_state(SMF_CTX(&state_object), &states[SAVE_STATE]);      
            return SMF_STATE_EXIT;
        }
        k_msleep(SLEEP_TIME_MS);
        LED_set(LED0, LED_OFF);
        LED_set(LED1, LED_OFF);
        LED_set(LED2, LED_OFF);
    
    }
    return SMF_EVENT_HANDLED;
 }

static void standby_state(void* -)
static enum smf_state_result standby_state_run(void*o){
    int brightness = 0;
    while(1) {
        brightness = (brightness % 2) ? (brightness == 100) ? (brightness == 99 : (brightness + 10));
        brightness = (brightness % 2) ? : ((brightness == 9) ? brightness == 0 : (brightness - 10));
        if (BTN_isPressed(BTN0)||BTN_isPressed(BTN1)||BTN_isPressed(BTN2)||BTN_isPressed(BTN3))
            return SMF_STATE_EXIT;
        k_msleep(SLEEP_TIME_MS);
    }
}

static enum smf_state_result 

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