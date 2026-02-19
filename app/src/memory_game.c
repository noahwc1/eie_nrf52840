/** 
 * @file memory_game.h
 */

#include <zephyr/smf.h>
#include <stdio.h>

#include "LED.h"
#include "my_state_machine.h"
#include "BTN.h"
#include "LED.h"


#define SLEEP_TIME_MS 1000


// Function Prototypes for the game states

static void standby_state(void* o);
static enum smf_state_result standby_state_run(void* o);

static void player1_enter_state(void* o);
static enum smf_state_result player1_enter_run(void* o);

static void player2_enter_state(void* o);
static enum smf_state_result player1_enter_run(void* o);

static void player1_play_state(void* o);
static enum smf_state_result player1_play_run(void* o);

static void player1_play_state(void* o);
static enum smf_state_result player2_play_run(void* o);

static void declare_winner_state(void* o);
static enum smf_state_result declare_winner_run(void* o);


// Type definitions for game states
enum memory_game_states {
  STANDBY_STATE,
  PLAYER1_ENTER,
  PLAYER2_ENTER,
  PLAYER1_PLAY,
  PLAYER2_PLAY,
  WINNER_STATE,
};

enum leds {
  LED0,
  LED1,
  LED2,
  LED3,
};

// Creates structure type for variables tracked during states
typedef struct {
    // context variable used by Zephyr to track state machine state must be first
    struct smf_ctx ctx;
    enum memory_game_states next_state;
    uint16_t round;
    
    int64_t last_toggle_ms;

    int64_t button_time;

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
  [PLAYER1_PLAY] = SMF_CREATE_STATE(player2_play_state, player2_play_run,
  NULL, NULL, NULL),
  
  // Declares the winner of the game
  [WINNER_STATE] = SMF_CREATE_STATE(declare_winner_state, declare_winner_run,
  NULL, NULL, NULL),
};




// Initialize memory game for start
void memory_game_innit() {
  state_object.round = 0;
  state_object.current_state = STANDBY_STATE;
  smf_set_initial(SMF_CTX(&state_object), &game_states[STANDBY_STATE]);
 


}

int state_machine_run() {
  return smf_run_state(SMF_CTX(&state_object));
}


// Local Variables

typedef struct {
  int player_sequence = {-1,-1,-1,-1, -1,-1,-1,-1,-1,-1};
  int *ptr_player_sequence = player_sequence;
  int errorflag = 0;
  int sequence_index = 0;
} player1;

typedef struct {
  int player_sequence = {-1,-1,-1,-1, -1,-1,-1,-1,-1,-1};
  int *ptr_player_sequence = player_sequence;
  int errorflag = 0;
  int sequence_index = 0;
} player2;






int sequence_length = 4;
int error_flag;
int i;





int enter_sequence(int which_button, struct player, int next_state);

int enter_sequence(int which_button, struct player, int next_state){
  if (player.player_sequence[player.sequence_index] == -1)
        player.player_sequence[player.sequence_index] = which_button;
  else {
      if (player.player_sequence[player.sequence_index] != which_button){
        smf_state(SMF_CTX(&state_object), &game_states[next_state]);
        return 1;
      }
    }    
  player.sequence_index++;
  return 0;
  }




// Memory Game State functions, what happens in each state


// standby state
static void standby_state(void* 0){
  state_object.round = 0;
  state_object.next_state = PLAYER1_ENTER;

  for(i = 0; i < 10; i++){
    player1.ptr_player_sequence[i] = -1;
    player2.ptr_player_sequence[i] = -1;
  }

  player1.errorflag = 0;
  player2.errorflag = 0;

  player1.sequence_index = 0;
  player2.sequence_index = 0
  
}

static enum smf_state_result standby_state_run(void* o){

  // If BTN 0 is pressed go into player 1 enter
  if (BTN_check_clear_pressed(BTN0)){
    smf_state(SMF_CTX(&state_object), &game_states[PLAYER1_ENTER]);
    return SMF_EVENT_HANDLED;
  }

  return SMF_EVENT_HANDLED;
  }


  // player 1 enter state
static void player1_enter_state(void* o){
  LED_SET(LED1, LED_ON);
  k_sleep(500);
  LED_SET(LED1, LED_OFF);
  
  if (round % 2 == 0) {
    state_object.next_state = PLAYER2_ENTER;
    sequence_length++;
  }
  else {
    state_object.next_state = PLAYER2_PLAY;
    }

  if (player1.errorflag == 1 && player2.errorflag == 0 || player1.errorflag == 0 && player2.errorflag == 1)
    smf_state(SMF_CTX(&state_object), &game_states[WINNER_STATE]);

  state_object.last_toggle_ms = k_uptime_get();
}

static enum smf_state_result player1_enter_run(void* o){
  int64_t now = k_uptime_get();

  if (BTN_check_clear_pressed(BTN0)){
    error_flag = enter_sequence(0, player1, state_object.next_state);
    LED_SET(LED0, LED_ON);
  }
  else if (BTN_check_clear_pressed(BTN1) ){
    error_flag = enter_sequence(1, player1, state_object.next_state);
    LED_SET(LED1, LED_ON);
  }
  else if (BTN_check_clear_pressed(BTN2) ){
    error_flag = enter_sequence(2, player1, state_object.next_state);
    LED_SET(LED2, LED_ON);
  }
  else if (BTN_check_clear_pressed(BTN3) ){
    error_flag = enter_sequence(3, player1, state_object.next_state);
    LED_SET(LED0, LED_ON);
  }

  if (error_flag == 1){
    player1.errorflag = 1;
    for (i = 0; i < 4; i++){
      LED_set(LED0, LED_ON);
      k_sleep(250);
      LED_set(LED0, LED_OFF);
      k_sleep(250);
    }
  } 
  else if (player1.sequence_index == sequence_length){
    smf_state(SMF_CTX(&state_object), &game_states[state_object.next]);
    player1.sequence_index == 0;
  }

  if ((now - state_object.last_toggle_ms) >= 125) {
    state_object.last_toggle_ms = now;
    LED_SET(LED0, LED_OFF);
    LED_SET(LED1, LED_OFF);
    LED_SET(LED2, LED_OFF);
    LED_SET(LED3, LED_OFF);
  }
  return SMF_EVENT_HANDLED;
  }


  // Player 2 enter state
static void player2_enter_state(void* o){
  LED_SET(LED2, LED_ON);
  k_sleep(500);
  LED_SET(LED2, LED_OFF);


  if (round % 2 == 0){
    state_object.next_state = PLAYER1_PLAY;
    
  }
  else {
    state_object.next_state = PLAYER1_ENTER;
    sequence_length++;
    
  if (player1.errorflag == 1 && player2.errorflag == 0 || player1.errorflag == 0 && player2.errorflag == 1)
    smf_state(SMF_CTX(&state_object), &game_states[WINNER_STATE]);

  }
}

static enum smf_state_result player2_enter_run(void* o){

  int64_t now = k_uptime_get();
  
  if (BTN_check_clear_pressed(BTN0)){
    error_flag = enter_sequence(0, player2, state_object.next_state);
    LED_SET(LED0, LED_ON);
  }
  else if (BTN_check_clear_pressed(BTN1) ){
    error_flag = enter_sequence(1, player2, state_object.next_state);
    LED_SET(LED1, LED_ON);
  }
  else if (BTN_check_clear_pressed(BTN2) ){
    error_flag = enter_sequence(2, player2, state_object.next_state);
    LED_SET(LED2, LED_ON);
  }
  else if (BTN_check_clear_pressed(BTN3) ){
    error_flag = enter_sequence(3, player2, state_object.next_state);
    LED_SET(LED0, LED_ON);
  }

  if (error_flag == 1){
    player2.errorflag = 1;
    for (i = 0; i < 4; i++){
      LED_set(LED0, LED_ON);
      k_sleep(250);
      LED_set(LED0, LED_OFF);
      k_sleep(250);
    }
  }
  else if (player2.sequence_index == sequence_length){
    smf_state(SMF_CTX(&state_object), &game_states[state_object.next]);
    player2.sequence_index == 0;
  }
  if ((now - state_object.last_toggle_ms) >= 125) {
      state_object.last_toggle_ms = now;
      LED_SET(LED0, LED_OFF);
      LED_SET(LED1, LED_OFF);
      LED_SET(LED2, LED_OFF);
      LED_SET(LED3, LED_OFF);
    }

  return SMF_EVENT_HANDLED;
  }


  // player 1 play
static void player1_play_state(void* o){
  LED_SET(LED1, LED_ON);
  k_sleep(500);
  LED_SET(LED1, LED_OFF);

  if (round % 2 == 0){
    state_object.next_state = PLAYER2_PLAY;
  }
  else {
    state_object.next_state = PLAYER1_ENTER;
    state_object.round++;
  }
    
  if (player1.errorflag == 1 && player2.errorflag == 0 || player1.errorflag == 0 && player2.errorflag == 1)
    smf_state(SMF_CTX(&state_object), &game_states[WINNER_STATE]);

  for (int i = 0; i < sequence_length; i++){
      LED_set(leds[player2.sequence[i]], LED_ON);
      k_sleep(SLEEP_TIME_MS);
      LED_set(leds[player2.sequence[i]], LED_OFF);
  }
  
}

static enum smf_state_result player1_play_run(void* o){

  int64_t now = k_uptime_get();

  if (BTN_check_clear_pressed(BTN0)){
    error_flag = enter_sequence(0, player2 state_object.next_state);
    LED_SET(LED0, LED_ON);
  }
  else if (BTN_check_clear_pressed(BTN1) ){
    error_flag = enter_sequence(1, player2 state_object.next_state);
    LED_SET(LED1, LED_ON);
  }
  else if (BTN_check_clear_pressed(BTN2) ){
    error_flag = enter_sequence(2, player2, state_object.next_state);
    LED_SET(LED2, LED_ON);
  }
  else if (BTN_check_clear_pressed(BTN3) ){
    error_flag = enter_sequence(3, player2, state_object.next_state);
    LED_SET(LED0, LED_ON);
  }

  if (error_flag == 1){
    player1.errorflag = 1;
    for (i = 0; i < 4; i++){
      LED_set(LED0, LED_ON);
      k_sleep(250);
      LED_set(LED0, LED_OFF);
      k_sleep(250);
    }
  }
  else if (player2.sequence_index == sequence_length){
    smf_state(SMF_CTX(&state_object), &game_states[state_object.next]);
    player2.sequence_index == 0;
  }

  if ((now - state_object.last_toggle_ms) >= 125) {
    state_object.last_toggle_ms = now;
    LED_SET(LED0, LED_OFF);
    LED_SET(LED1, LED_OFF);
    LED_SET(LED2, LED_OFF);
    LED_SET(LED3, LED_OFF);
  }
  return SMF_EVENT_HANDLED;
}



//  player 2 play

static void player2_play_state(void* o){
  LED_SET(LED2, LED_ON);
  k_sleep(500);
  LED_SET(LED2, LED_OFF);

  if (round % 2 == 1){
    state_object.next_state = PLAYER1_PLAY;
  }
  else {
    state_object.next_state = PLAYER2_ENTER;
    state_object.round++;
  }
    
  if (player1.errorflag == 1 && player2.errorflag == 0 || player1.errorflag == 0 && player2.errorflag == 1)
    smf_state(SMF_CTX(&state_object), &game_states[WINNER_STATE]);
  
  for (int i = 0; i < sequence_length; i++){
    LED_set(leds[player2.sequence[i]], LED_ON);
    k_sleep(SLEEP_TIME_MS);
    LED_set(leds[player2.sequence[i]], LED_OFF);
  }
  

}

static enum smf_state_result player2_play_run(void* o){
  int64_t now = k_uptime_get();
  
  if (BTN_check_clear_pressed(BTN0)){
    error_flag = enter_sequence(0, player1, state_object.next_state);
    LED_SET(LED0, LED_ON);
  }
  else if (BTN_check_clear_pressed(BTN1) ){
    error_flag = enter_sequence(1, player1, state_object.next_state);
    LED_SET(LED1, LED_ON);
  }
  else if (BTN_check_clear_pressed(BTN2) ){
    error_flag = enter_sequence(2, player1, state_object.next_state);
    LED_SET(LED2, LED_ON);
  }
  else if (BTN_check_clear_pressed(BTN3) ){
    error_flag = enter_sequence(3, player1, state_object.next_state);
    LED_SET(LED0, LED_ON);
  }

  if (error_flag == 1){
    player2.errorflag = 1;
    for (i = 0; i < 4; i++){
      LED_set(LED0, LED_ON);
      k_sleep(250);
      LED_set(LED0, LED_OFF);
      k_sleep(250);
    }
  }
  else if (player1.sequence_index == sequence_length){
    smf_state(SMF_CTX(&state_object), &game_states[state_object.next]);
    player1.sequence_index == 0;
  }

  if ((now - state_object.last_toggle_ms) >= 125) {
    state_object.last_toggle_ms = now;
    LED_SET(LED0, LED_OFF);
    LED_SET(LED1, LED_OFF);
    LED_SET(LED2, LED_OFF);
    LED_SET(LED3, LED_OFF);
  }
  return SMF_EVENT_HANDLED;
  }



// Winner state
static void declare_winner_state(void* o) {

}
 

static enum smf_state_result declare_winner_run(void* o){
  if (player1.errorflag == 1 && player2.errorflag == 0)
    printk("PLAYER 2 IS WINNER!!!")
  else
    printk("PLAYER 1 IS WINNER!!!")
  
  smf_state(SMF_CTX(&state_object), &game_states[STANDBY_STATE]);
  return SMF_EVENT_HANDLED;
  }