#pragma once

#include "game.h"

void save_game(const History &history);
// returns false (and leaves history untouched) if the save could not be read
bool load_game(History &history);

void update_scoreboard(const GameState &state);
void show_scoreboard();
