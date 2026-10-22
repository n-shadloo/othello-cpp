#pragma once

#include <string>
#include <vector>

#include "raylib.h"

// raylib defines RED and WHITE as colors, the game uses them for the two players
#undef RED
#undef WHITE

#include "../game.h"

const int WINDOW_WIDTH = 1120;
const int WINDOW_HEIGHT = 780;

const float BOARD_LEFT = 50;
const float BOARD_TOP = 70;
const float CELL = 80;
const float BOARD_PIXELS = CELL * BOARD_SIZE;
const float PANEL_LEFT = BOARD_LEFT + BOARD_PIXELS + 50;
const float PANEL_WIDTH = WINDOW_WIDTH - PANEL_LEFT - 50;

const double FLIP_SECONDS = 0.35;
const double AI_DELAY_SECONDS = 0.55;

enum Screen
{
    SCREEN_MENU,
    SCREEN_GAME
};

enum Action
{
    ACTION_NONE,
    ACTION_MODE_TWO_PLAYERS,
    ACTION_MODE_VS_AI,
    ACTION_LEVEL,      // + ai mode
    ACTION_COLOR_RED,
    ACTION_COLOR_WHITE,
    ACTION_TIMER_OFF,
    ACTION_TIMER_ON,
    ACTION_START,
    ACTION_QUIT,
    ACTION_UNDO,
    ACTION_REDO,
    ACTION_HINT,
    ACTION_MENU,
    ACTION_PLAY_AGAIN
};

struct Button
{
    Rectangle rect;
    std::string label;
    int action;
    int value;
    bool selected;
    bool primary;
};

struct Input
{
    Vector2 mouse;
    bool clicked;
    double now;
};

struct App
{
    Screen screen = SCREEN_MENU;
    bool quit = false;

    // menu choices
    int game_mode = MODE_VS_AI;
    int ai_mode = 4;
    char human_color = RED;
    int timer_mode = 0;

    History history;
    GameState state;
    time_t turn_start = 0;
    bool over = false;
    bool result_saved = false;

    // the board as it looked before the last move, for the flip animation
    Board previous_board;
    double move_time = -100;
    double ai_ready_at = 0;
    Move hint = {-1, -1};
    std::string message;
    double message_time = -100;
};

void app_init(App &app);
std::vector<Button> app_buttons(const App &app);
void app_update(App &app, const Input &input);
bool is_human_turn(const App &app);
bool square_at(Vector2 point, Move &move);
time_t clock_for(const App &app, char player);
