/** 
 * @file my_state_machine.h
 */

#include <zephyr/smf.h>

#include "LED.h"
#include "my_state_machine.h"


/*
Function Prototypes
*/
static void led_on_state_entry(void* o);
static enum smf_state_result led_on_state_run(void* o);
static void led_off_state_entry(void* o);
static enum smf_state_result led_off_state_run(void* o);

static void led_exit_on_state_entry(void* o);
static void led_exit_off_state_entry(void* o);


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
} led_state_object_t;


/*
Local Variables
*/
static const struct smf_state led_states[] = {
    [START_STATE] = SMF_CREATE_STATE(),
    [ENTER_STATE] = SMF_CREATE_STATE()
    [SAVE_STATE] = SMF_CREATE_STATE(),
    [STANDBY_STATE] = SMF_CREATE_STATE()
};

static led_state_object_t led_state_object;

void state_machine_init() {
    led_state_object.count = 0;
    smf_set_initial(SMF_CTX(&led_state_object), &led_states[START_STATE]);
    
 }

 int state_machine_run() {
   return smf_run_state(SMF_CTX(&led_state_object));
 }

 static void BTN0_OR_BTN1_PRESSED(void*o) {
    btn0_presses = 0;
    btn1_presses = 0;
 }

 static enum smf_state_result   BTN0_OR_BTN1_PRESSED(void* o) {
    
    while(1) {
        if (BTN_isPressed(BTN0) ) {
            btn0_presses++;
            BTN_clearPress(BTN0);
        }
        if (BTN_isPressed(BTN1) ) {
            btn1_presses++;
            BTN_clearPress(BTN1);
        }
        if (btn0_presses + btn1_presses > 7) {
            smf_set_state(smf_ctx())      return SMF_STATE_EXIT;)
    }
    }
    return SMF_EVENT_HANDLED;
 }

 static void led_off_state_entry(void*o) {
    LED_set(LED0, LED_OFF);
 }

 static enum smf_state_result led_off_state_run(void* o){
    LED_set(LED2, LED_ON);
    if (led_state_object.count > 500){
        // led_state_object.count = 0;
        smf_set_state(SMF_CTX(&led_state_object), &led_states[LED_ON_STATE]);
    
    } else {
        led_state_object.count++;
        printk("LED OFF State Count: %d\n", led_state_object.count);
    }
    return SMF_EVENT_HANDLED;
 }

static void led_exit_off_state_entry(void* o) {
    led_state_object.count = 0;
  
 }
static void led_exit_on_state_entry(void* o) {
    led_state_object.count = 0;
   
 }