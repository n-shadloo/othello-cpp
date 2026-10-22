#include "app.h"

#include <cstring>

#include "../ai.h"
#include "../storage.h"

using namespace std;


void app_init(App &app)
{
    app.state = new_game_state(app.game_mode, app.timer_mode, app.ai_mode, app.human_color);
    init_history(app.history, app.state);
    copy_board(app.state.board, app.previous_board);
    app.turn_start = time(nullptr);
    app.over = false;
    app.result_saved = false;
    app.move_time = -100;
    app.ai_ready_at = 0;
    app.hint = {-1, -1};
    app.message.clear();
}

bool is_human_turn(const App &app)
{
    return app.state.game_mode == MODE_TWO_PLAYERS || app.state.current_player == app.state.human_color;
}

bool square_at(Vector2 point, Move &move)
{
    move.col = (int)((point.x - BOARD_LEFT) / CELL);
    move.row = (int)((point.y - BOARD_TOP) / CELL);
    return point.x >= BOARD_LEFT && point.y >= BOARD_TOP && in_bounds(move.row, move.col);
}

time_t clock_for(const App &app, char player)
{
    time_t seconds = (player == RED) ? app.state.red_seconds : app.state.white_seconds;
    if (!app.over && app.state.current_player == player)
    {
        seconds += time(nullptr) - app.turn_start;
    }
    return seconds;
}

static void say(App &app, const string &text, double now)
{
    app.message = text;
    app.message_time = now;
}


static Button make_button(float x, float y, float w, float h, const string &label, int action,
                          int value = 0, bool selected = false, bool primary = false)
{
    return {{x, y, w, h}, label, action, value, selected, primary};
}

vector<Button> app_buttons(const App &app)
{
    vector<Button> buttons;

    if (app.screen == SCREEN_MENU)
    {
        float x = WINDOW_WIDTH / 2.0f - 300;
        buttons.push_back(make_button(x, 300, 295, 52, "Two players", ACTION_MODE_TWO_PLAYERS, 0,
                                      app.game_mode == MODE_TWO_PLAYERS));
        buttons.push_back(make_button(x + 305, 300, 295, 52, "vs Computer", ACTION_MODE_VS_AI, 0,
                                      app.game_mode == MODE_VS_AI));

        if (app.game_mode == MODE_VS_AI)
        {
            const char *levels[] = {"Defensive", "Offensive", "Greedy", "Random", "Expert"};
            for (int i = 0; i < 5; i++)
            {
                buttons.push_back(make_button(x + i * 122, 410, 112, 44, levels[i], ACTION_LEVEL, i, app.ai_mode == i));
            }
            buttons.push_back(make_button(x, 510, 295, 44, "Red (moves first)", ACTION_COLOR_RED, 0, app.human_color == RED));
            buttons.push_back(make_button(x + 305, 510, 295, 44, "White", ACTION_COLOR_WHITE, 0, app.human_color == WHITE));
        }
        else
        {
            buttons.push_back(make_button(x, 410, 295, 44, "No clock", ACTION_TIMER_OFF, 0, app.timer_mode == 0));
            buttons.push_back(make_button(x + 305, 410, 295, 44, "5 minutes each", ACTION_TIMER_ON, 0, app.timer_mode == 1));
        }

        buttons.push_back(make_button(WINDOW_WIDTH / 2.0f - 150, 610, 300, 60, "Start game", ACTION_START, 0, false, true));
        buttons.push_back(make_button(WINDOW_WIDTH / 2.0f - 60, 690, 120, 40, "Quit", ACTION_QUIT));
        return buttons;
    }

    if (app.over)
    {
        float cx = BOARD_LEFT + BOARD_PIXELS / 2;
        buttons.push_back(make_button(cx - 160, BOARD_TOP + 400, 150, 50, "Play again", ACTION_PLAY_AGAIN, 0, false, true));
        buttons.push_back(make_button(cx + 10, BOARD_TOP + 400, 150, 50, "Menu", ACTION_MENU));
        return buttons;
    }

    float y = BOARD_TOP + BOARD_PIXELS - 52;
    float w = (PANEL_WIDTH - 20) / 3;
    buttons.push_back(make_button(PANEL_LEFT, y - 62, w, 50, "Undo", ACTION_UNDO));
    buttons.push_back(make_button(PANEL_LEFT + w + 10, y - 62, w, 50, "Redo", ACTION_REDO));
    buttons.push_back(make_button(PANEL_LEFT + 2 * (w + 10), y - 62, w, 50, "Hint", ACTION_HINT));
    buttons.push_back(make_button(PANEL_LEFT, y, PANEL_WIDTH, 50, "Back to menu", ACTION_MENU));
    return buttons;
}


static void add_turn_time(App &app)
{
    time_t now = time(nullptr);
    if (app.state.current_player == RED)
    {
        app.state.red_seconds += now - app.turn_start;
    }
    else
    {
        app.state.white_seconds += now - app.turn_start;
    }
    app.turn_start = now;
}

static void finish(App &app)
{
    app.over = true;
    if (!app.result_saved)
    {
        update_scoreboard(app.state);
        app.result_saved = true;
    }
}

static void play(App &app, Move move, double now)
{
    add_turn_time(app);
    copy_board(app.state.board, app.previous_board);

    apply_move(app.state.board, app.state.current_player, move.row, move.col);
    app.state.last_move = move;
    app.state.current_player = get_opponent(app.state.current_player);
    app.state.move_number++;
    app.move_time = now;
    app.ai_ready_at = now + FLIP_SECONDS + AI_DELAY_SECONDS;
    app.hint = {-1, -1};

    // a player with no move passes straight away
    if (!is_game_over(app.state.board) && !has_any_legal_move(app.state.board, app.state.current_player))
    {
        say(app, string(app.state.current_player == RED ? "Red" : "White") + " has no move and passes", now);
        app.state.current_player = get_opponent(app.state.current_player);
    }

    save_current_state(app.history, app.state);

    if (is_game_over(app.state.board))
    {
        finish(app);
    }
}

static void step_history(App &app, bool back, double now)
{
    bool possible = back ? can_undo(app.history, app.state.game_mode) : can_redo(app.history, app.state.game_mode);
    if (!possible)
    {
        say(app, back ? "Nothing to undo" : "Nothing to redo", now);
        return;
    }

    add_turn_time(app);
    time_t red = app.state.red_seconds, white = app.state.white_seconds;
    back ? undo(app.history, app.state) : redo(app.history, app.state);
    app.state.red_seconds = red;
    app.state.white_seconds = white;

    copy_board(app.state.board, app.previous_board);
    app.hint = {-1, -1};
    app.ai_ready_at = now + AI_DELAY_SECONDS;
}

static void run_action(App &app, const Button &button, double now)
{
    switch (button.action)
    {
        case ACTION_MODE_TWO_PLAYERS: app.game_mode = MODE_TWO_PLAYERS; break;
        case ACTION_MODE_VS_AI: app.game_mode = MODE_VS_AI; break;
        case ACTION_LEVEL: app.ai_mode = button.value; break;
        case ACTION_COLOR_RED: app.human_color = RED; break;
        case ACTION_COLOR_WHITE: app.human_color = WHITE; break;
        case ACTION_TIMER_OFF: app.timer_mode = 0; break;
        case ACTION_TIMER_ON: app.timer_mode = 1; break;
        case ACTION_QUIT: app.quit = true; break;
        case ACTION_START:
        case ACTION_PLAY_AGAIN:
            app_init(app);
            app.screen = SCREEN_GAME;
            app.ai_ready_at = now + AI_DELAY_SECONDS;
            break;
        case ACTION_MENU: app.screen = SCREEN_MENU; break;
        case ACTION_UNDO: step_history(app, true, now); break;
        case ACTION_REDO: step_history(app, false, now); break;
        case ACTION_HINT:
            app.hint = ai_choose_expert_move(app.state.board, app.state.current_player);
            break;
    }
}

void app_update(App &app, const Input &input)
{
    if (input.clicked)
    {
        for (const Button &button : app_buttons(app))
        {
            if (CheckCollisionPointRec(input.mouse, button.rect))
            {
                run_action(app, button, input.now);
                return;
            }
        }
    }

    if (app.screen != SCREEN_GAME || app.over)
    {
        return;
    }

    if (app.state.game_mode == MODE_TWO_PLAYERS && app.state.pvp_timer_mode == 1)
    {
        if (clock_for(app, RED) >= TIME_LIMIT_SECONDS || clock_for(app, WHITE) >= TIME_LIMIT_SECONDS)
        {
            add_turn_time(app);
            finish(app);
            return;
        }
    }

    if (!is_human_turn(app))
    {
        if (input.now >= app.ai_ready_at)
        {
            Move move = ai_choose_move(app.state.board, app.state.current_player, app.state.ai_mode);
            if (move.row != -1)
            {
                play(app, move, input.now);
            }
        }
        return;
    }

    Move move;
    if (input.clicked && square_at(input.mouse, move))
    {
        if (is_legal_move(app.state.board, app.state.current_player, move.row, move.col))
        {
            play(app, move, input.now);
        }
        else
        {
            say(app, "You can't play there", input.now);
        }
    }
}
