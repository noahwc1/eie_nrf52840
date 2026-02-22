/** 
 * @file memory_game.c
 */

#include <zephyr/smf.h>
#include <stdio.h>

#include "LED.h"
#include "memory_game.h"
#include "BTN.h"



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



// Creates structure type for variables tracked during states
typedef struct {
    // context variable used by Zephyr to track state machine state must be first
    struct smf_ctx ctx;
    enum memory_game_states next_state;
    uint16_t round;
    
    int64_t last_toggle_ms;

    int64_t button_time;

    int64_t leds_status;

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







int sequence_length = 4;
int error_flag;
int i;




  
int enter_sequence(int which_button, player *player, int next_state);
int enter_sequence(int which_button, player *player, int next_state){
  if (player->player_sequence[player->sequence_index] == -1)
        player->player_sequence[player->sequence_index] = which_button;
  else {
      if (player->player_sequence[player->sequence_index] != which_button){
        smf_set_state(SMF_CTX(&state_object), &game_states[next_state]);
        return 1;
      }
    }    
  player->sequence_index++;
  return 0;
  }




// Memory Game State functions, what happens in each state


// standby state
static void standby_state(void* o){
  state_object.round = 0;
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

  
}

static enum smf_state_result standby_state_run(void* o){
  int64_t now = k_uptime_get();
  
  // If BTN 0 is pressed go into player 1 enter
  if (BTN_check_clear_pressed(BTN0)){
    smf_set_state(SMF_CTX(&state_object), &game_states[PLAYER1_ENTER]);
  }


  if ((now - state_object.last_toggle_ms) >= 500) {
        state_object.button_time = now;
        LED_set(LED0, state_object.leds_status ? LED_ON : LED_OFF);
        LED_set(LED1, state_object.leds_status ? LED_ON : LED_OFF);
        LED_set(LED2, state_object.leds_status ? LED_ON : LED_OFF);
        LED_set(LED3, state_object.leds_status ? LED_ON : LED_OFF);
        state_object.leds_status = (state_object.leds_status ? 0 : 1);
    }

  return SMF_EVENT_HANDLED;
  }


  // player 1 enter state
static void player1_enter_state(void* o){
  LED_set(LED1, LED_ON);
  k_msleep(500);
  LED_set(LED1, LED_OFF);
  
  if (state_object.round % 2 == 0) {
    state_object.next_state = PLAYER2_ENTER;
    sequence_length++;
  }
  else {
    state_object.next_state = PLAYER2_PLAY;
    }

  if ((pointer_player1->errorflag == 1 && pointer_player2->errorflag == 0)|| (pointer_player1->errorflag == 0 && pointer_player2->errorflag == 1))
    smf_set_state(SMF_CTX(&state_object), &game_states[WINNER_STATE]);

  state_object.last_toggle_ms = k_uptime_get();
}

static enum smf_state_result player1_enter_run(void* o){
  int64_t now = k_uptime_get();

  if (BTN_check_clear_pressed(BTN0)){
    error_flag = enter_sequence(0, pointer_player1, state_object.next_state);
    LED_set(LED0, LED_ON);
  }
  else if (BTN_check_clear_pressed(BTN1) ){
    error_flag = enter_sequence(1, pointer_player1, state_object.next_state);
    LED_set(LED1, LED_ON);
  }
  else if (BTN_check_clear_pressed(BTN2) ){
    error_flag = enter_sequence(2, pointer_player1, state_object.next_state);
    LED_set(LED2, LED_ON);
  }
  else if (BTN_check_clear_pressed(BTN3) ){
    error_flag = enter_sequence(3, pointer_player1, state_object.next_state);
    LED_set(LED3, LED_ON);
  }

  if (error_flag == 1){
    pointer_player1->errorflag = 1;
    for (i = 0; i < 4; i++){
      LED_set(LED0, LED_ON);
      k_msleep(250);
      LED_set(LED0, LED_OFF);
      k_msleep(250);
    }
  } 
  else if (pointer_player1->sequence_index == sequence_length){
    smf_set_state(SMF_CTX(&state_object), &game_states[state_object.next_state]);
    pointer_player1->sequence_index = 0;
  }

  if ((now - state_object.last_toggle_ms) >= 500) {
    state_object.last_toggle_ms = now;
    LED_set(LED0, LED_OFF);
    LED_set(LED1, LED_OFF);
    LED_set(LED2, LED_OFF);
    LED_set(LED3, LED_OFF);
  }
  return SMF_EVENT_HANDLED;
  }


  // Player 2 enter state
static void player2_enter_state(void* o){
  LED_set(LED2, LED_ON);
  k_msleep(500);
  LED_set(LED2, LED_OFF);


  if (state_object.round % 2 == 0){
    state_object.next_state = PLAYER1_PLAY;
    
  }
  else {
    state_object.next_state = PLAYER1_ENTER;
    sequence_length++;
  }
  if ((pointer_player1->errorflag == 1 && pointer_player2->errorflag == 0)|| (pointer_player1->errorflag == 0 && pointer_player2->errorflag == 1))
    smf_set_state(SMF_CTX(&state_object), &game_states[WINNER_STATE]);
 
  
}

static enum smf_state_result player2_enter_run(void* o){

  int64_t now = k_uptime_get();
  
  if (BTN_check_clear_pressed(BTN0)){
    error_flag = enter_sequence(0, pointer_player2, state_object.next_state);
    LED_set(LED0, LED_ON);
  }
  else if (BTN_check_clear_pressed(BTN1) ){
    error_flag = enter_sequence(1, pointer_player2, state_object.next_state);
    LED_set(LED1, LED_ON);
  }
  else if (BTN_check_clear_pressed(BTN2) ){
    error_flag = enter_sequence(2, pointer_player2, state_object.next_state);
    LED_set(LED2, LED_ON);
  }
  else if (BTN_check_clear_pressed(BTN3) ){
    error_flag = enter_sequence(3, pointer_player2, state_object.next_state);
    LED_set(LED3, LED_ON);
  }

  if (error_flag == 1){
    pointer_player2->errorflag = 1;
    for (i = 0; i < 4; i++){
      LED_set(LED0, LED_ON);
      k_msleep(250);
      LED_set(LED0, LED_OFF);
      k_msleep(250);
    }
  }
  else if (pointer_player2->sequence_index == sequence_length){
    smf_set_state(SMF_CTX(&state_object), &game_states[state_object.next_state]);
    pointer_player2->sequence_index = 0;
  }
  if ((now - state_object.last_toggle_ms) >= 500) {
      state_object.last_toggle_ms = now;
      LED_set(LED0, LED_OFF);
      LED_set(LED1, LED_OFF);
      LED_set(LED2, LED_OFF);
      LED_set(LED3, LED_OFF);
    }

  return SMF_EVENT_HANDLED;
  }


  // player 1 play
static void player1_play_state(void* o){
  LED_set(LED1, LED_ON);
  k_msleep(500);
  LED_set(LED1, LED_OFF);

  if (state_object.round % 2 == 0){
    state_object.next_state = PLAYER2_PLAY;
  }
  else {
    state_object.next_state = PLAYER1_ENTER;
    state_object.round++;
  }
    
  if ((pointer_player1->errorflag == 1 && pointer_player2->errorflag == 0)|| (pointer_player1->errorflag == 0 && pointer_player2->errorflag == 1))
    smf_set_state(SMF_CTX(&state_object), &game_states[WINNER_STATE]);

  for (int i = 0; i < sequence_length; i++){
     enum led_id_t led = (led_id)pointer_player2->player_sequence[i];
      LED_set(led, LED_ON);
      k_msleep(SLEEP_TIME_MS);
      LED_set(led, LED_OFF);
  }
  
}

static enum smf_state_result player1_play_run(void* o){

  int64_t now = k_uptime_get();

  if (BTN_check_clear_pressed(BTN0)){
    error_flag = enter_sequence(0, pointer_player2, state_object.next_state);
    LED_set(LED0, LED_ON);
  }
  else if (BTN_check_clear_pressed(BTN1) ){
    error_flag = enter_sequence(1, pointer_player2, state_object.next_state);
    LED_set(LED1, LED_ON);
  }
  else if (BTN_check_clear_pressed(BTN2) ){
    error_flag = enter_sequence(2, pointer_player2, state_object.next_state);
    LED_set(LED2, LED_ON);
  }
  else if (BTN_check_clear_pressed(BTN3) ){
    error_flag = enter_sequence(3, pointer_player2, state_object.next_state);
    LED_set(LED3, LED_ON);
  }

  if (error_flag == 1){
    pointer_player1->errorflag = 1;
    for (i = 0; i < 4; i++){
      LED_set(LED0, LED_ON);
      k_msleep(250);
      LED_set(LED0, LED_OFF);
      k_msleep(250);
    }
  }
  else if (pointer_player2->sequence_index == sequence_length){
    smf_set_state(SMF_CTX(&state_object), &game_states[state_object.next_state]);
    pointer_player2->sequence_index = 0;
  }

  if ((now - state_object.last_toggle_ms) >= 500) {
    state_object.last_toggle_ms = now;
    LED_set(LED0, LED_OFF);
    LED_set(LED1, LED_OFF);
    LED_set(LED2, LED_OFF);
    LED_set(LED3, LED_OFF);
  }
  return SMF_EVENT_HANDLED;
}



//  player 2 play

static void player2_play_state(void* o){
  LED_set(LED2, LED_ON);
  k_msleep(500);
  LED_set(LED2, LED_OFF);

  if (state_object.round % 2 == 1){
    state_object.next_state = PLAYER1_PLAY;
  }
  else {
    state_object.next_state = PLAYER2_ENTER;
    state_object.round++;
  }
    
  if ((pointer_player1->errorflag == 1 && pointer_player2->errorflag == 0)|| (pointer_player1->errorflag == 0 && pointer_player2->errorflag == 1))
    smf_set_state(SMF_CTX(&state_object), &game_states[WINNER_STATE]);
  
  for (int i = 0; i < sequence_length; i++){
    enum led_id_t led = (led_id)pointer_player2->player_sequence[i];
    LED_set(led, LED_ON);
    k_msleep(SLEEP_TIME_MS);
    LED_set(led, LED_OFF);
  }
  

}

static enum smf_state_result player2_play_run(void* o){
  int64_t now = k_uptime_get();

  if (BTN_check_clear_pressed(BTN0)){
    error_flag = enter_sequence(0, pointer_player1, state_object.next_state);
    LED_set(LED0, LED_ON);
  }
  else if (BTN_check_clear_pressed(BTN1) ){
    error_flag = enter_sequence(1, pointer_player1, state_object.next_state);
    LED_set(LED1, LED_ON);
  }
  else if (BTN_check_clear_pressed(BTN2) ){
    error_flag = enter_sequence(2, pointer_player1, state_object.next_state);
    LED_set(LED2, LED_ON);
  }
  else if (BTN_check_clear_pressed(BTN3) ){
    error_flag = enter_sequence(3, pointer_player1, state_object.next_state);
    LED_set(LED3, LED_ON);
  }

  if (error_flag == 1){
    pointer_player2->errorflag = 1;
    for (i = 0; i < 4; i++){
      LED_set(LED0, LED_ON);
      k_msleep(250);
      LED_set(LED0, LED_OFF);
      k_msleep(250);
    }
  }
  else if (pointer_player1->sequence_index == sequence_length){
    smf_set_state(SMF_CTX(&state_object), &game_states[state_object.next_state]);
    pointer_player1->sequence_index = 0;
  }

  if ((now - state_object.last_toggle_ms) >= 500) {
    state_object.last_toggle_ms = now;
    LED_set(LED0, LED_OFF);
    LED_set(LED1, LED_OFF);
    LED_set(LED2, LED_OFF);
    LED_set(LED3, LED_OFF);
  }
  return SMF_EVENT_HANDLED;
  }



// Winner state
static void declare_winner_state(void* o) {

}
 

static enum smf_state_result declare_winner_run(void* o){
  if (pointer_player1->errorflag == 1 && pointer_player2->errorflag == 0){
    printk("PLAYER 2 IS WINNER!!!\n");
    LED_set(LED2, LED_ON);
    k_msleep(3*SLEEP_TIME_MS);
  }
  else{
    printk("PLAYER 1 IS WINNER!!!\n");
    LED_set(LED1, LED_ON);
    k_msleep(3*SLEEP_TIME_MS);
  }
  smf_set_state(SMF_CTX(&state_object), &game_states[STANDBY_STATE]);
  return SMF_EVENT_HANDLED;
}