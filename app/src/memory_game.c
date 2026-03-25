/** 
 * @file memory_game.c
 */

#include <zephyr/smf.h>
#include <stdio.h>

#include "LED.h"
#include "memory_game.h"
#include "BTN.h"


// #include <lvgl.h>
// #include "lv_data_obj.h"
// #include <zephyr/drivers/display.h>
#include "lcd_ui.h"



#define SLEEP_TIME_MS 1000


// Function Prototypes for the game states

static void standby_state(void* o);
static enum smf_state_result standby_state_run(void* o);

static void player1_enter_state(void* o);
static enum smf_state_result player1_enter_run(void* o);

static void player2_enter_state(void* o);
static enum smf_state_result player2_enter_run(void* o);


static void player1_play_state(void* o);
static enum smf_state_result player1_play_run(void* o);

static void player2_play_state(void* o);
static enum smf_state_result player2_play_run(void* o);


static void player1_standby_state(void* o);
static enum smf_state_result player1_standby_run(void* o);

static void player2_standby_state(void* o); 
static enum smf_state_result player2_standby_run(void* o);

static void declare_winner_state(void* o);
static enum smf_state_result declare_winner_run(void* o);

static void show_sequence_state(void* o);
static enum smf_state_result show_sequence_run(void* o);


// Type definitions for game states
enum memory_game_states {
  STANDBY_STATE,
  PLAYER1_ENTER,
  PLAYER2_ENTER,
  PLAYER1_PLAY,
  PLAYER2_PLAY,
  WINNER_STATE,
  SHOW_SEQUENCE,
  PLAYER1_STANDBY,
  PLAYER2_STANDBY,
};



// Creates structure type for variables tracked during states
typedef struct {
    // context variable used by Zephyr to track state machine state must be first
    struct smf_ctx ctx;
    enum memory_game_states next_state;
    uint16_t round;
    
    int64_t last_toggle_ms;

    int64_t led_time;

    int leds_status;

    int showing_seq_to_player;

    int someone_error;

} state_object_t;

// Declaring scoped variable for code from struct type
static state_object_t state_object;



static const struct smf_state game_states[] = {
  // Start state for the program and state entered when game is not running
  [STANDBY_STATE] = SMF_CREATE_STATE(standby_state, standby_state_run, 
  NULL, NULL, NULL),                                                      
  
  // STATE FOR PLAYER NAMES....

  // Gets player 1 to enter their sequence
  [PLAYER1_ENTER] = SMF_CREATE_STATE(player1_enter_state, player1_enter_run,
  NULL, NULL, NULL),

  // Gets player 2 to enter their sequence
  [PLAYER2_ENTER] = SMF_CREATE_STATE(player2_enter_state, player2_enter_run,
  NULL, NULL, NULL),

  // Gets player 1 to put in player 2's sequence
  [PLAYER1_PLAY] = SMF_CREATE_STATE(player1_play_state, player1_play_run,
  NULL, NULL, NULL),
  
  // Gets player 2 to put in player 1's sequence
  [PLAYER2_PLAY] = SMF_CREATE_STATE(player2_play_state, player2_play_run,
  NULL, NULL, NULL),
  
  // Declares the winner of the game
  [WINNER_STATE] = SMF_CREATE_STATE(declare_winner_state, declare_winner_run,
  NULL, NULL, NULL),

  // Show sequence of player x
  [SHOW_SEQUENCE] = SMF_CREATE_STATE(show_sequence_state, show_sequence_run,
  NULL, NULL, NULL),

  [PLAYER1_STANDBY] = SMF_CREATE_STATE(player1_standby_state, player1_standby_run,
  NULL, NULL, NULL),

  [PLAYER2_STANDBY] = SMF_CREATE_STATE(player2_standby_state, player2_standby_run,
  NULL, NULL, NULL),  
};






// Initialize memory game for start
void memory_game_init() {
  state_object.round = 0;
  state_object.next_state = PLAYER1_ENTER;
  state_object.leds_status = 0;


  smf_set_initial(SMF_CTX(&state_object), &game_states[STANDBY_STATE]);
 printk("initial state set\n");
}

int memory_game_run() {
  return smf_run_state(SMF_CTX(&state_object));
}


// Local Variables

typedef struct {
  int player_sequence[10];
  // int *ptr_player_sequence;
  int errorflag;
  int sequence_index;
} player;

player player1 = {{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 0, 0};
player player2 = {{-1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, 0, 0};

player *pointer_player1 = &player1;
player *pointer_player2 = &player2;

player *player_seq_to_show;



int sequence_length = 4;
int error_flag;
int i;
char buff[100];


// Memory game helper functions
int enter_sequence(int which_button, player *player, int next_state);
int enter_sequence(int which_button, player *player, int next_state){
  if (player->player_sequence[player->sequence_index] == -1)
        player->player_sequence[player->sequence_index] = which_button;
        
  else {
      if (player->player_sequence[player->sequence_index] != which_button){
        return 1;
      }
    }    
  player->sequence_index++;
  return 0;
  }


static int lcd_button_pressed[NUM_LEDS] = {0};

void memory_game_set_lcd_button(led_id led);
void memory_game_set_lcd_button(led_id led){
  if (led < NUM_LEDS) {
    lcd_button_pressed[led] = 1;
  }
}

int check_button(int btn);
int check_button(int btn){
  if (BTN_check_clear_pressed(btn) || lcd_button_pressed[btn]){
    lcd_button_pressed[btn] = 0;
    return 1;
  }
  return 0;
}

void all_leds_on();
void all_leds_on(){
  for(int i = 0; i < 4; i++){
    LED_set(i, LED_ON);
    lcd_set_led(i, 1);
  }
  

}

void all_leds_off();
void all_leds_off(){
  for(int i = 0; i < 4; i++){
    LED_set(i, LED_OFF);
    lcd_set_led(i, 0);
  }
}

void show_leds(player *player);
void show_leds(player *player){
  enum led_id_t led =
            (led_id)player->player_sequence[player->sequence_index];
      
        if (!state_object.leds_status) {
          LED_set(led, LED_ON);
          lcd_set_led(led, 1);
          state_object.leds_status= 1;
        } 
        else {
            LED_set(led, LED_OFF);
            lcd_set_led(led, 0);
            state_object.leds_status = 0;
            player->sequence_index++;
        }
}

void btns_clear();
void btns_clear(){
  for (int i = 0; i < 4; i++){
    BTN_clear_pressed(i);
  }
}



// Memory Game State functions, what happens in each state


// standby state
static void standby_state(void* o){
  all_leds_off();
  state_object.leds_status = 0;

  printk("STANDBY STATE\n");
  lcd_set_status("Memory Game - Press BUTTON1 to Start");

  state_object.round = 0;
  sequence_length = 4;
  state_object.next_state = PLAYER1_ENTER;

  for(i = 0; i < 10; i++){
    pointer_player1->player_sequence[i] = -1;
    pointer_player2->player_sequence[i] = -1;
  }

  pointer_player1->errorflag = 0;
  pointer_player2->errorflag = 0;

  pointer_player1->sequence_index = 0;
  pointer_player2->sequence_index = 0;

  state_object.last_toggle_ms = k_uptime_get();

  state_object.someone_error = 0;
  error_flag = 0;

  
}

static enum smf_state_result standby_state_run(void* o){
  int64_t now = k_uptime_get();
  
  // If BTN 0 is pressed go into player 1 enter
  if (check_button(0)){
    smf_set_state(SMF_CTX(&state_object), &game_states[PLAYER1_STANDBY]);
  }


  if ((now - state_object.last_toggle_ms) >= 500) {
        state_object.last_toggle_ms = now;
        for(int i = 0; i < 4; i++){
          LED_toggle(i);
          lcd_set_led(i, (state_object.leds_status ? 0 : 1));
        }
        state_object.leds_status = !state_object.leds_status;
    }

  return SMF_EVENT_HANDLED;
  }


  // player 1 enter state
static void player1_enter_state(void* o){
  all_leds_off();
  state_object.next_state = PLAYER2_ENTER;
  error_flag = 0;

// Code for alternate player sequencing

  // if (state_object.round % 2 ==  1) {   // odd round, player 2 play next
  //   state_object.next_state = SHOW_SEQUENCE;
  //   sequence_length++;
  //   state_object.showing_seq_to_player = 2;
  // }
  // else {
  //   state_object.next_state = PLAYER2_ENTER;
  //   }

  // if ((pointer_player1->errorflag == 1 && state_object.someone_error == 1)){
  //   printk("error flag detected, going to winner state\n");
  //   smf_set_state(SMF_CTX(&state_object), &game_states[WINNER_STATE]);
  // }
  if (state_object.round == 0){
      printk("PLAYER 1 ENTER STATE, enter %d characters\n", sequence_length);
  snprintf(buff, sizeof(buff), "PLAYER 1 ENTER STATE, enter %d characters", sequence_length);
  lcd_set_status(buff);
  }
  else {
      printk("PLAYER 1 ENTER STATE, enter %d characters\n", sequence_length);
    snprintf(buff, sizeof(buff), "PLAYER 1 ENTER STATE \nAdd 1 to your sequence for a total of %d", sequence_length);
    lcd_set_status(buff);
  }
 
  state_object.last_toggle_ms = k_uptime_get();
  btns_clear();
}

static enum smf_state_result player1_enter_run(void* o){
  int64_t now = k_uptime_get();

  // only accept button presses if total sequence length has not been reached yet
  if (pointer_player1->sequence_index < sequence_length && error_flag == 0){

    if (check_button(0)){
      error_flag = enter_sequence(0, pointer_player1, state_object.next_state);
      LED_set(LED0, LED_ON);
      lcd_set_led(LED0, 1);
      state_object.last_toggle_ms = now;
    }
    else if (check_button(1)){
      error_flag = enter_sequence(1, pointer_player1, state_object.next_state);
      LED_set(LED1, LED_ON);
      lcd_set_led(LED1, 1);
      state_object.last_toggle_ms = now;
    }
    else if (check_button(2)){
      error_flag = enter_sequence(2, pointer_player1, state_object.next_state);
      LED_set(LED2, LED_ON);
      lcd_set_led(LED2, 1);
      state_object.last_toggle_ms = now;
    }
    else if (check_button(3) ){
      error_flag = enter_sequence(3, pointer_player1, state_object.next_state);
      LED_set(LED3, LED_ON);
      lcd_set_led(LED3, 1);
      state_object.last_toggle_ms = now;
    }
  }

  // Check for error in player 1 play
  if (error_flag == 1){
    lcd_set_status("Error! Incorrect Button Pressed");
    all_leds_on();
    pointer_player1->errorflag = 1;
    printk("error flag set for player 1\n");
    if (now - state_object.last_toggle_ms >= 1500){
      all_leds_off();
      smf_set_state(SMF_CTX(&state_object), &game_states[PLAYER2_STANDBY]);
      // if (pointer_player1->errorflag == 1 && pointer_player2->errorflag == 1){
      //   smf_set_state(SMF_CTX(&state_object), &game_states[PLAYER2_STANDBY]);
      //   state_object.next_state = SHOW_SEQUENCE;
      //   state_object.showing_seq_to_player = 2;
      //   pointer_player1->errorflag = 0;
      //   pointer_player2->errorflag = 0;
      // }
      // else if (pointer_player1->errorflag == 1 && pointer_player2->errorflag == 0){
      //   smf_set_state(SMF_CTX(&state_object), &game_states[WINNER_STATE]);
      // }
      // else {
      //   error_flag = 0;
      //   smf_set_state(SMF_CTX(&state_object), &game_states[state_object.next_state]);
      // }
    }
  }

  // No error check for sequence length being reached
  else if (pointer_player1->sequence_index == sequence_length){
    if (player2.errorflag == 1) {
      state_object.someone_error = 1;
      // player 1 wins
      smf_set_state(SMF_CTX(&state_object), &game_states[WINNER_STATE]);
    }
    else if ((now-state_object.last_toggle_ms) >= 250){
      smf_set_state(SMF_CTX(&state_object), &game_states[PLAYER2_STANDBY]);
      pointer_player1->sequence_index = 0;
    }
  }

  // reseting LEDs if still in gameplay
  else {
    if ((now - state_object.last_toggle_ms) >= 250) {
      state_object.last_toggle_ms = now;
      all_leds_off();
    }
    // if ((now - state_object.last_toggle_ms) >= 250) {
    //   LED_set(LED0, LED_ON);

    // }
  }
  
  return SMF_EVENT_HANDLED;
}


  // Player 2 enter state
static void player2_enter_state(void* o){

  // Code for alternate player sequencing

  // all_leds_off();
  // if (state_object.round % 2 == 1 ){   // odd round, player 1 enter next
  //   state_object.next_state =PLAYER1_ENTER ;
  //   sequence_length++;

  // }
  // else {
    state_object.next_state = SHOW_SEQUENCE;
    state_object.showing_seq_to_player = 1;
    error_flag = 0;
    
  // }

  // if ((pointer_player2->errorflag == 1 && state_object.someone_error == 1)){
  //     printk("error flag detected, going to winner state\n");
  //     smf_set_state(SMF_CTX(&state_object), &game_states[WINNER_STATE]);
  // }

 if (state_object.round == 0){
      printk("PLAYER 2 ENTER STATE, enter %d characters\n", sequence_length);
  snprintf(buff, sizeof(buff), "PLAYER 2 ENTER STATE, enter %d characters", sequence_length);
  lcd_set_status(buff);
  }
  else {
      printk("PLAYER 2 ENTER STATE, enter %d characters\n", sequence_length);
    snprintf(buff, sizeof(buff), "PLAYER 2 ENTER STATE \nadd 1 to your sequence for a total of %d", sequence_length);
    lcd_set_status(buff);
  }
  btns_clear();
}

static enum smf_state_result player2_enter_run(void* o){
  int64_t now = k_uptime_get();
  
  // only accept button presses if total sequence length has not been reached yet
  if (pointer_player2->sequence_index < sequence_length && error_flag == 0){
    if (check_button(0)){
      error_flag = enter_sequence(0, pointer_player2, state_object.next_state);
      LED_set(LED0, LED_ON);
      lcd_set_led(LED0, 1);
      state_object.last_toggle_ms = now;
    }
    else if (check_button(1) ){
      error_flag = enter_sequence(1, pointer_player2, state_object.next_state);
      LED_set(LED1, LED_ON);
      lcd_set_led(LED1, 1);
      state_object.last_toggle_ms = now;
    }
    else if (check_button(2) ){
      error_flag = enter_sequence(2, pointer_player2, state_object.next_state);
      LED_set(LED2, LED_ON);
      lcd_set_led(LED2, 1);
      state_object.last_toggle_ms = now;
    }
    else if (check_button(3) ){
      error_flag = enter_sequence(3, pointer_player2, state_object.next_state);
      LED_set(LED3, LED_ON);
      lcd_set_led(LED3, 1);
      state_object.last_toggle_ms = now;
    }
  }

  // Check for error in player 2 play
  if (error_flag == 1) {
      all_leds_on();
      lcd_set_status("Error! Incorrect Button Pressed");
      pointer_player2->errorflag = 1;
      printk("error flag set for player 2\n");
     
      if (now - state_object.last_toggle_ms >= 1500){
        all_leds_off();
        if (pointer_player1->errorflag == 1 && pointer_player2->errorflag == 1){
          smf_set_state(SMF_CTX(&state_object), &game_states[PLAYER1_STANDBY]);
          state_object.next_state = PLAYER1_ENTER;
          pointer_player1->errorflag = 0;
          pointer_player2->errorflag = 0;
        }
        else {
          smf_set_state(SMF_CTX(&state_object), &game_states[WINNER_STATE]);
        }
        // else if(pointer_player2->errorflag == 1 && pointer_player1->errorflag == 0){
        //   smf_set_state(SMF_CTX(&state_object), &game_states[WINNER_STATE]);
        // }
        // else {
        // error_flag = 0;
        // smf_set_state(SMF_CTX(&state_object), &game_states[state_object.next_state]);
        // }
      }
  }
  
  // Check for sequence length being reached with no error
  else if (pointer_player2->sequence_index == sequence_length){
    if (player1.errorflag == 1) {
      state_object.someone_error = 1;
      smf_set_state(SMF_CTX(&state_object), &game_states[WINNER_STATE]);
    }
    else if ((now - state_object.last_toggle_ms) >= 250){
      smf_set_state(SMF_CTX(&state_object), &game_states[PLAYER1_STANDBY]);
      pointer_player2->sequence_index = 0;
    }
  }

  // reseting LEDs if still in gameplay
  else {
    if ((now - state_object.last_toggle_ms) >= 500) {
        state_object.last_toggle_ms = now;
        all_leds_off();
    }
    // if ((now - state_object.last_toggle_ms) >= 250) {
    //   LED_set(LED1, LED_ON);
    // }
  }
  return SMF_EVENT_HANDLED;
  }


  // player 1 play
static void player1_play_state(void* o){
  all_leds_off();

  // alternate sequencing

  // if (state_object.round % 2 == 1){  // odd round, player1 enter next
  //   state_object.next_state = PLAYER1_ENTER;
  //   state_object.round++;
  // }
  // else {
  // state_object.next_state = SHOW_SEQUENCE;
  // state_object.showing_seq_to_player = 2;  // even round, show sequence to player 2
       
  // }
    state_object.next_state = SHOW_SEQUENCE;
    state_object.showing_seq_to_player = 2;
    error_flag = 0;
    
  // if ((pointer_player2->errorflag == 1 && state_object.someone_error == 1)){
  //     printk("error flag detected, going to winner state\n");
  //     smf_set_state(SMF_CTX(&state_object), &game_states[WINNER_STATE]);
  // }

  printk("PLAYER 1 PLAY STATE, enter player 2's sequence: %d length, round: %d\n", sequence_length, state_object.round);
  snprintf(buff, sizeof(buff), "PLAYER 1 PLAY STATE, \nenter player 2's sequence: length %d", sequence_length);
  lcd_set_status(buff);
  btns_clear();
}

static enum smf_state_result player1_play_run(void* o){
  int64_t now = k_uptime_get();
  
  // only accept button presses if total sequence length has not been reached yet
  if (pointer_player2->sequence_index < sequence_length && error_flag == 0){
    if (check_button(0)){
      error_flag = enter_sequence(0, pointer_player2, state_object.next_state);
      LED_set(LED0, LED_ON);
      lcd_set_led(LED0, 1);
      state_object.last_toggle_ms = now;
    }
    else if (check_button(1)){
      error_flag = enter_sequence(1, pointer_player2, state_object.next_state);
      LED_set(LED1, LED_ON);
      lcd_set_led(LED1, 1);
      state_object.last_toggle_ms = now;
    }
    else if (check_button(2) ){
      error_flag = enter_sequence(2, pointer_player2, state_object.next_state);
      LED_set(LED2, LED_ON);
      lcd_set_led(LED2, 1);
      state_object.last_toggle_ms = now;
    }
    else if (check_button(3) ){
      error_flag = enter_sequence(3, pointer_player2, state_object.next_state);
      LED_set(LED3, LED_ON);
      lcd_set_led(LED3, 1);
      state_object.last_toggle_ms = now;
    }
  }

  // Check for error in player 1 play
  if (error_flag == 1){
      all_leds_on();
      lcd_set_status("Error! Incorrect Button Pressed");
      pointer_player1->errorflag = 1;
      printk("error flag set for player 1\n");
      if(now - state_object.last_toggle_ms >= 1500){
        all_leds_off();
        smf_set_state(SMF_CTX(&state_object), &game_states[PLAYER2_STANDBY]);
        // if (pointer_player1->errorflag == 1 && pointer_player2->errorflag == 1){
        //     smf_set_state(SMF_CTX(&state_object), &game_states[PLAYER2_STANDBY]);
        //     state_object.next_state = PLAYER2_ENTER;
        //     pointer_player1->errorflag = 0;
        //     pointer_player2->errorflag = 0;
        // }
        // else if (pointer_player1->errorflag == 1 && pointer_player2->errorflag == 0){
        //   smf_set_state(SMF_CTX(&state_object), &game_states[WINNER_STATE]);
        // }
        // else {
        //   error_flag = 0;
        //   smf_set_state(SMF_CTX(&state_object), &game_states[state_object.next_state]);
        // }
      }
  }
  
  // Check for sequence length being reached with no error
  else if (pointer_player2->sequence_index == sequence_length){
    if (player2.errorflag == 1) {
      state_object.someone_error = 1;
      smf_set_state(SMF_CTX(&state_object), &game_states[WINNER_STATE]);
    }
    else if ((now - state_object.last_toggle_ms) >= 250){
      smf_set_state(SMF_CTX(&state_object), &game_states[PLAYER2_STANDBY]);
      pointer_player2->sequence_index = 0;
    }
  }

  // reseting LEDs if still in gameplay
  else {
    if ((now - state_object.last_toggle_ms) >= 250) {
      state_object.last_toggle_ms = now;
      all_leds_off();
    }
    // if ((now - state_object.last_toggle_ms) >= 250) {
    //   LED_set(LED0, LED_ON);
    // }
  }
  return SMF_EVENT_HANDLED;
}


//  player 2 play
static void player2_play_state(void* o){
  all_leds_off();
  // alternate sequencing

  // if (state_object.round % 2 == 1){  // odd round, player 1 plays
  // state_object.next_state = SHOW_SEQUENCE;
  // state_object.showing_seq_to_player = 1;
  // }
  // else {
  //   state_object.next_state = PLAYER2_ENTER;
  //   state_object.round++;
  // }

  state_object.next_state = PLAYER1_ENTER;
  error_flag = 0;
  // if ((pointer_player2->errorflag == 1 && state_object.someone_error == 1)){
  //     printk("error flag detected, going to winner state\n");
  //     smf_set_state(SMF_CTX(&state_object), &game_states[WINNER_STATE]);
  // }

  printk("PLAYER 2 PLAY STATE, enter player 1's sequence: %d length\n", sequence_length);
  snprintf(buff, sizeof(buff), "PLAYER 2 PLAY STATE \nenter player 1's sequence: length %d", sequence_length);
  lcd_set_status(buff);
  btns_clear();
}

static enum smf_state_result player2_play_run(void* o){
  int64_t now = k_uptime_get();

  // only accept button presses if total sequence length has not been reached yet
  if (pointer_player2->sequence_index < sequence_length && error_flag == 0){
    if (check_button(0)){
      error_flag = enter_sequence(0, pointer_player1, state_object.next_state);
      LED_set(LED0, LED_ON);
      lcd_set_led(LED0, 1);
      state_object.last_toggle_ms = now;
    }
    else if (check_button(1) ){
      error_flag = enter_sequence(1, pointer_player1, state_object.next_state);
      LED_set(LED1, LED_ON);
      lcd_set_led(LED1, 1);
      state_object.last_toggle_ms = now;
    }
    else if (check_button(2) ){
      error_flag = enter_sequence(2, pointer_player1, state_object.next_state);
      LED_set(LED2, LED_ON);
      lcd_set_led(LED2, 1);
      state_object.last_toggle_ms = now;
    }
    else if (check_button(3) ){
      error_flag = enter_sequence(3, pointer_player1, state_object.next_state);
      LED_set(LED3, LED_ON);
      lcd_set_led(LED3, 1);
      state_object.last_toggle_ms = now;
    }
  }

// Check for error in player 2 play
if (error_flag == 1){
    all_leds_on();
    lcd_set_status("Error! Incorrect Button Pressed");
    pointer_player2->errorflag = 1;
    printk("error flag set for player 2\n");
    if(now - state_object.last_toggle_ms >= 1500){
      all_leds_off();
      if (pointer_player1->errorflag == 1 && pointer_player2->errorflag == 1){
        smf_set_state(SMF_CTX(&state_object), &game_states[PLAYER1_STANDBY]);
        state_object.next_state = SHOW_SEQUENCE;
        state_object.showing_seq_to_player = 1;
        pointer_player1->errorflag = 0;
        pointer_player2->errorflag = 0;
      }
      else {
        smf_set_state(SMF_CTX(&state_object), &game_states[WINNER_STATE]);
      }
      // else if (pointer_player2->errorflag == 1 && pointer_player1->errorflag == 0){
      //   smf_set_state(SMF_CTX(&state_object), &game_states[WINNER_STATE]);
      // }
      // else {
      //   error_flag = 0;
      //   smf_set_state(SMF_CTX(&state_object), &game_states[state_object.next_state]);
      // }
    }
}

// Check for sequence length being reached with no error
else if (pointer_player1->sequence_index == sequence_length){
    if (player1.errorflag == 1) {
      state_object.someone_error = 1;
      smf_set_state(SMF_CTX(&state_object), &game_states[WINNER_STATE]);
    }
    else if ((now - state_object.last_toggle_ms) >= 250){
      state_object.round++;
      sequence_length++;
      smf_set_state(SMF_CTX(&state_object), &game_states[PLAYER1_STANDBY]);
      pointer_player1->sequence_index = 0;
    }
}

// reseting LEDs if still in gameplay
else {
    if ((now - state_object.last_toggle_ms) >= 250) {
      state_object.last_toggle_ms = now;
      all_leds_off();
    }
    // if ((now - state_object.last_toggle_ms) >= 250) {
    //   LED_set(LED1, LED_ON);
    // }
  }

return SMF_EVENT_HANDLED;
}


// Winner state
static void declare_winner_state(void* o) {
    
    all_leds_off();
    printk("Delcare Winner State\n");
    lcd_set_status("We have a Winner...");
    state_object.leds_status = 0;

state_object.last_toggle_ms = k_uptime_get();
}

static enum smf_state_result declare_winner_run(void* o){

  int64_t now = k_uptime_get();

  if ((now - state_object.last_toggle_ms) > 2000 && state_object.leds_status == 0) {
    state_object.last_toggle_ms = now;
    state_object.leds_status = 1;
    lcd_green_led();

    if (pointer_player1->errorflag == 1 && pointer_player2->errorflag == 0){
      printk("PLAYER 2 IS WINNER!!!\n");
      lcd_set_status("PLAYER 2 IS WINNER!!!");
      LED_set(LED1, LED_ON);
    }
    else {
      printk("PLAYER 1 IS WINNER!!!\n");
      lcd_set_status("PLAYER 1 IS WINNER!!!");
      LED_set(LED0, LED_ON);
    }
  }
  else if ((now - state_object.last_toggle_ms) >= 8000) {
    all_leds_off();
    smf_set_state(SMF_CTX(&state_object), &game_states[STANDBY_STATE]);
  }

  return SMF_EVENT_HANDLED;
}


// Show sequence state

static void show_sequence_state(void* o) {
  all_leds_off();
  printk("Show Sequence State\n");
  snprintf(buff, sizeof(buff), "Showing PLAYER %d's Sequence", state_object.showing_seq_to_player == 1 ? 2 : 1);
  lcd_set_status(buff);

  if (state_object.showing_seq_to_player == 1){  
    state_object.next_state = PLAYER1_PLAY;
    pointer_player2->sequence_index = 0;
    player_seq_to_show = pointer_player2;
  }
  else {
    state_object.next_state = PLAYER2_PLAY;
    pointer_player1->sequence_index = 0;
    player_seq_to_show = pointer_player1;
  }
  state_object.last_toggle_ms = k_uptime_get();
  state_object.leds_status = 0;
}

static enum smf_state_result show_sequence_run(void* o){

  int64_t now = k_uptime_get();
  
  // player *player_to_show;
  // if (state_object.showing_seq_to_player == 1){
  //   player_to_show = pointer_player2;
  // }
  // else {
  //   player_to_show = pointer_player1
  // }

  if (player_seq_to_show->sequence_index == 0 && (now - state_object.last_toggle_ms) > 1000) {  // extra delay for first one
    state_object.last_toggle_ms = now;
    show_leds(player_seq_to_show);
  }
  else if (player_seq_to_show->sequence_index > 0){
    if (player_seq_to_show->sequence_index >= sequence_length) {
      player_seq_to_show->sequence_index = 0;
      printk("Done showing sequence\n");
      smf_set_state(SMF_CTX(&state_object), &game_states[state_object.next_state]);
      printk("test for exit\n");
      // sequence done
  }
    else if ((now - state_object.last_toggle_ms) > 500) {
        state_object.last_toggle_ms = now;
        show_leds(player_seq_to_show);
    }
  }
  return SMF_EVENT_HANDLED;
}



static void player1_standby_state(void* o){
  all_leds_off();
  state_object.last_toggle_ms = k_uptime_get();
  printk("PLAYER 1 STANDBY STATE\n");
  lcd_set_status("PLAYER 1 Standby - Press BUTTON1 \nto Start Turn");
  btns_clear();
  pointer_player1->sequence_index = 0;
  state_object.leds_status = 0;
}

static enum smf_state_result player1_standby_run(void* o){
  int64_t now = k_uptime_get();

  if (check_button(0)){
    smf_set_state(SMF_CTX(&state_object), &game_states[state_object.next_state]);
  }

  if ((now - state_object.last_toggle_ms) > 500) {
        state_object.last_toggle_ms = now;
        LED_toggle(0);
        lcd_set_led(0, (state_object.leds_status ? 0 : 1));
        state_object.leds_status = !state_object.leds_status;
    }
  
  return SMF_EVENT_HANDLED;
}


static void player2_standby_state(void* o){
  all_leds_off();
  state_object.last_toggle_ms = k_uptime_get();
  printk("PLAYER 2 STANDBY STATE\n");
  lcd_set_status("PLAYER 2 Standby - Press BUTTON2 \nto Start Turn");
  btns_clear();
  pointer_player2->sequence_index = 0;
  state_object.leds_status = 0;
}

static enum smf_state_result player2_standby_run(void* o){
  int64_t now = k_uptime_get();

  if (check_button(1)){
    smf_set_state(SMF_CTX(&state_object), &game_states[state_object.next_state]);
  }

  if ((now - state_object.last_toggle_ms) > 500) {
        state_object.last_toggle_ms = now;
        LED_toggle(1);
        lcd_set_led(1, (state_object.leds_status ? 0 : 1));
        state_object.leds_status = !state_object.leds_status;
    }

  
  return SMF_EVENT_HANDLED;
}