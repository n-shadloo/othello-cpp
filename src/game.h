#pragma once

#include <ctime>
#include <vector>

#include "board.h"

const int MODE_TWO_PLAYERS = 0;
const int MODE_VS_AI = 1;
const time_t TIME_LIMIT_SECONDS = 5 * 60;
const char DRAW = 'D';

struct GameState
{
    Board board;

    int game_mode;
    int pvp_timer_mode;
    int ai_mode;
    int move_number;

    char human_color;
    char current_player;
    Move last_move;

    time_t red_seconds;
    time_t white_seconds;
};

struct History
{
    std::vector<GameState> states;
    int index = 0;
};

GameState new_game_state(int game_mode, int pvp_timer_mode, int ai_mode, char human_color);

// it clears history and stores current_state as the only entry
void init_history(History &history, const GameState &current_state);
// it drops any redo states after index, then appends current_state
void save_current_state(History &history, const GameState &current_state);
// against the AI one undo goes back two plies, so it is the human's turn again
bool can_undo(const History &history, int game_mode);
void undo(History &history, GameState &current_state);
bool can_redo(const History &history, int game_mode);
void redo(History &history, GameState &current_state);

// R, W or DRAW, a player running out of time loses no matter the disk count
char decide_winner(const GameState &state);

void play_game(History &history);
