/** 
 * @file memory_game.h
 */

#ifndef MEMORY_GAME_H
#define MEMORY_GAME_H

#include "LED.h"

void memory_game_init();
int memory_game_run();

void memory_game_set_lcd_button(led_id led);

#endif // MEMORY_GAME_H