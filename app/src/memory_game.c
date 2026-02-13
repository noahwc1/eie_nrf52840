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

// Creates structure type for variables tracked during states
typedef struct {
    // context variable used by Zephyr to track state machine state must be first
    struct smf_ctx ctx;
    enum memory_game_states current_state;
    uint16_t count;
    uint16_t round;
    
    int led3_on;
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
}




// Initialize memory game for start
void memory_game_innit() {
  state_object.round = 0;
  state_object.current_state = STANDBY_STATE;
  smf_set_initial(SMF_CTX(&state_object), &states[STANDBY_STATE]);


}

int state_machine_run() {
  return smf_run_state(SMF_CTX(&state_object));
}


// Local Variables

typedef struct {
  int player_sequence = {-1,-1,-1,-1, -1,-1,-1,-1,-1,-1};
  int *ptr_player_sequence = player_sequence;
  int player_errorflag = 1;
} player1;

typedef struct {
  int player_sequence = {-1,-1,-1,-1, -1,-1,-1,-1,-1,-1};
  int *ptr_player_sequence = player_sequence;
  int player_errorflag = 1;
} player2;


int sequence_length = 4;
int sequence_index = 0;

int enter_sequence(int)
int enter_sequence(int which_button, struct player, int next_state){
  if (player.player_sequence[sequence_index] == -1)
        player.player_sequence[sequence_index] = which_button;
  else {
      if (player.player_sequence[sequence_index] != which_button){
        player_errorflag = 0;
        smf_state(SMF_CTX(&state_object), &states[next_state]);
        return;
      }
    }    
  }

// Memory Game State functions, what happens in each state

static void standby_state(void* 0){
  state_object.round = 0;
  state_object.current_state = STANDBY_STATE;
}

static enum smf_state_result standby_state_run(void* o){

// If BTN 0 is pressed go into player 1 enter
if (BTN_check_clear_pressed(BTN0)){
  smf_state(SMF_CTX(&state_object), &states[PLAYER1_ENTER]);
  return SMF_EVENT_HANDLED;
}

return SMF_EVENT_HANDLED;
}

static void player1_enter_state(void* o){
  state_object.count = 0;
  state_object.current_state = PLAYER1_ENTER;

}

static enum smf_state_result player1_enter_run(void* o){
  
  
  if (BTN_check_clear_pressed(BTN0) ){
        return SMF_RETURN
        
  }


}