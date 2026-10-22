#include "game.h"

#include <iostream>
#include <string>

#include "ai.h"
#include "display.h"
#include "storage.h"

using namespace std;


GameState new_game_state(int game_mode, int pvp_timer_mode, int ai_mode, char human_color)
{
    GameState state;
    init_board(state.board);
    state.game_mode = game_mode;
    state.pvp_timer_mode = pvp_timer_mode;
    state.ai_mode = ai_mode;
    state.move_number = 1;
    state.human_color = human_color;
    state.current_player = RED;
    state.last_move = {-1, -1};
    state.red_seconds = 0;
    state.white_seconds = 0;
    return state;
}


void init_history(History &history, const GameState &current_state)
{
    history.states.clear();
    history.states.push_back(current_state);
    history.index = 0;
}

void save_current_state(History &history, const GameState &current_state)
{
    history.states.resize(history.index + 1);
    history.states.push_back(current_state);
    history.index++;
}

static bool is_human_turn(const GameState &state)
{
    return state.game_mode == MODE_TWO_PLAYERS || state.current_player == state.human_color;
}

// nearest state in the given direction where a human is to move, -1 if there is none
static int find_human_state(const History &history, int step)
{
    int i = history.index + step;
    while (i >= 0 && i < (int)history.states.size())
    {
        if (is_human_turn(history.states[i]))
        {
            return i;
        }
        i += step;
    }
    return -1;
}

bool can_undo(const History &history, int game_mode)
{
    if (game_mode == MODE_TWO_PLAYERS)
    {
        return history.index > 0;
    }
    return find_human_state(history, -1) != -1;
}

void undo(History &history, GameState &current_state)
{
    history.index = (current_state.game_mode == MODE_TWO_PLAYERS) ? history.index - 1 : find_human_state(history, -1);
    current_state = history.states[history.index];
}

bool can_redo(const History &history, int game_mode)
{
    if (game_mode == MODE_TWO_PLAYERS)
    {
        return history.index + 1 < (int)history.states.size();
    }
    return find_human_state(history, 1) != -1;
}

void redo(History &history, GameState &current_state)
{
    history.index = (current_state.game_mode == MODE_TWO_PLAYERS) ? history.index + 1 : find_human_state(history, 1);
    current_state = history.states[history.index];
}


char decide_winner(const GameState &state)
{
    if (state.red_seconds >= TIME_LIMIT_SECONDS)
    {
        return WHITE;
    }
    if (state.white_seconds >= TIME_LIMIT_SECONDS)
    {
        return RED;
    }

    int red_count, white_count;
    count_disks(state.board, red_count, white_count);
    if (red_count == white_count)
    {
        return DRAW;
    }
    return red_count > white_count ? RED : WHITE;
}

static bool is_timed(const GameState &state)
{
    return state.game_mode == MODE_TWO_PLAYERS && state.pvp_timer_mode == 1;
}

static void add_turn_time(GameState &state, time_t &turn_start_time)
{
    time_t now = time(nullptr);
    if (state.current_player == RED)
    {
        state.red_seconds += now - turn_start_time;
    }
    else
    {
        state.white_seconds += now - turn_start_time;
    }
    turn_start_time = now;
}

static string side_label(const GameState &state, char player)
{
    string label = player_name(player);
    if (state.game_mode == MODE_VS_AI && player != state.human_color)
    {
        label += string(color::DIM) + " (" + ai_mode_name(state.ai_mode) + " AI)" + color::RESET;
    }
    return label;
}

static void print_status(const GameState &state)
{
    int red_count, white_count;
    count_disks(state.board, red_count, white_count);

    cout << "  " << color::RED << "● " << red_count << color::RESET << "   "
         << color::WHITE << "● " << white_count << color::RESET
         << color::DIM << "     move " << state.move_number << color::RESET << "\n";

    if (is_timed(state))
    {
        cout << "  Time left:  Red " << format_time(max<time_t>(0, TIME_LIMIT_SECONDS - state.red_seconds))
             << "  |  White " << format_time(max<time_t>(0, TIME_LIMIT_SECONDS - state.white_seconds)) << "\n";
    }
    else
    {
        cout << color::DIM << "  Time played:  Red " << format_time(state.red_seconds)
             << "  |  White " << format_time(state.white_seconds) << color::RESET << "\n";
    }

    cout << "  Turn: " << side_label(state, state.current_player) << "\n\n";
}

static void print_help()
{
    cout << color::DIM
         << "  c4    place a disk          hint  ask the AI for a move\n"
         << "  undo  take back a move      redo  replay an undone move\n"
         << "  save  save this game        quit  back to the main menu\n"
         << color::RESET << "\n";
}

static string subtitle_for(const GameState &state)
{
    if (state.game_mode == MODE_TWO_PLAYERS)
    {
        return is_timed(state) ? "two players, 5 min each" : "two players";
    }
    return string("vs ") + ai_mode_name(state.ai_mode) + " AI";
}

static void show_result(const GameState &state)
{
    clear_screen();
    print_title("game over");
    print_board(state.board, state.current_player, state.last_move, false);

    int red_count, white_count;
    count_disks(state.board, red_count, white_count);
    cout << "  Final score:  " << color::RED << "Red " << red_count << color::RESET
         << "  -  " << color::WHITE << "White " << white_count << color::RESET << "\n\n";

    char winner = decide_winner(state);
    if (state.red_seconds >= TIME_LIMIT_SECONDS)
    {
        cout << color::YELLOW << "  White Won." << color::RESET << "\n";
        cout << "  Red lost on time like a fool. The clock played better.\n";
    }
    else if (state.white_seconds >= TIME_LIMIT_SECONDS)
    {
        cout << color::YELLOW << "  Red Won." << color::RESET << "\n";
        cout << "  White thought forever and still lost on time.\n";
    }
    else if (winner == RED)
    {
        cout << color::YELLOW << "  RED WINS" << color::RESET << "\n";
        cout << "  White got absolutely FARMED.\n";
        cout << "  Skill issue. Board domination complete.\n";
    }
    else if (winner == WHITE)
    {
        cout << color::YELLOW << "  WHITE WINS" << color::RESET << "\n";
        cout << "  Red just got OUTPLAYED.\n";
        cout << "  Better luck next game, champ.\n";
    }
    else
    {
        cout << color::YELLOW << "  DRAW" << color::RESET << "\n";
        cout << "  Both players too scared to win.\n";
    }

    update_scoreboard(state);
    cout << "\n";
    wait_for_enter("Result saved in the scoreboard. Press enter to go back to the menu...");
}

void play_game(History &history)
{
    // a copy, not a reference: history.states can reallocate while we play
    GameState state = history.states[history.index];
    time_t turn_start_time = time(nullptr);
    string message;

    while (!is_game_over(state.board))
    {
        clear_screen();
        print_title(subtitle_for(state));

        bool human = is_human_turn(state);
        print_board(state.board, state.current_player, state.last_move, human);
        print_status(state);

        if (!message.empty())
        {
            cout << "  " << message << "\n\n";
            message.clear();
        }

        if (!has_any_legal_move(state.board, state.current_player))
        {
            add_turn_time(state, turn_start_time);
            message = side_label(state, state.current_player) + " had no legal move and passed.";
            state.current_player = get_opponent(state.current_player);
            state.move_number++;
            save_current_state(history, state);
            continue;
        }

        Move move = {-1, -1};
        if (human)
        {
            print_legal_moves(state.board, state.current_player);
            string input = read_word("  Your move (type help for commands): ");

            if (input == "help")
            {
                print_help();
                wait_for_enter();
                continue;
            }
            if (input == "quit")
            {
                return;
            }
            if (input == "save")
            {
                add_turn_time(state, turn_start_time);
                history.states[history.index] = state;
                save_game(history);
                wait_for_enter();
                continue;
            }
            if (input == "hint")
            {
                Move hint = ai_choose_expert_move(state.board, state.current_player);
                message = string("Hint: try ") + color::GREEN + move_to_string(hint) + color::RESET;
                continue;
            }
            if (input == "undo" || input == "redo")
            {
                bool is_undo = (input == "undo");
                if (is_undo ? !can_undo(history, state.game_mode) : !can_redo(history, state.game_mode))
                {
                    message = is_undo ? "Nothing to undo." : "Nothing to redo.";
                    continue;
                }

                // the clocks keep running across undo, otherwise undo would refund time
                add_turn_time(state, turn_start_time);
                time_t red_seconds = state.red_seconds;
                time_t white_seconds = state.white_seconds;

                is_undo ? undo(history, state) : redo(history, state);
                state.red_seconds = red_seconds;
                state.white_seconds = white_seconds;
                continue;
            }

            if (!parse_move(input, move))
            {
                message = "That is not a square. Use a letter and a number, like c4.";
                continue;
            }
            if (!is_legal_move(state.board, state.current_player, move.row, move.col))
            {
                message = move_to_string(move) + " is not a legal move.";
                continue;
            }
        }
        else
        {
            cout << color::DIM << "  thinking..." << color::RESET << flush;
            move = ai_choose_move(state.board, state.current_player, state.ai_mode);
            message = side_label(state, state.current_player) + " played " + color::GREEN + move_to_string(move) + color::RESET;
        }

        add_turn_time(state, turn_start_time);
        if (is_timed(state) && (state.red_seconds >= TIME_LIMIT_SECONDS || state.white_seconds >= TIME_LIMIT_SECONDS))
        {
            break;
        }

        apply_move(state.board, state.current_player, move.row, move.col);
        state.last_move = move;
        state.current_player = get_opponent(state.current_player);
        state.move_number++;

        save_current_state(history, state);
    }

    show_result(state);
}
