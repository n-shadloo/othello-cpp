#include "storage.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include "ai.h"
#include "display.h"

using namespace std;

const string SAVE_DIR = "saves";
const string SCOREBOARD_FILE = "scoreboard.txt";


static bool is_valid_name(const string &name)
{
    if (name.empty())
    {
        return false;
    }
    for (char ch : name)
    {
        if (!isalnum((unsigned char)ch) && ch != '-' && ch != '_')
        {
            return false;
        }
    }
    return true;
}

static void write_state(ofstream &out, const GameState &state)
{
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            out << state.board[i][j] << " ";
        }
        out << "\n";
    }

    out << state.game_mode << " " << state.pvp_timer_mode << " " << state.ai_mode << " "
        << state.human_color << " " << state.current_player << " " << state.move_number << " "
        << state.last_move.row << " " << state.last_move.col << " "
        << state.red_seconds << " " << state.white_seconds << "\n\n";
}

static bool read_state(ifstream &in, GameState &state)
{
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            in >> state.board[i][j];
        }
    }

    in >> state.game_mode >> state.pvp_timer_mode >> state.ai_mode
       >> state.human_color >> state.current_player >> state.move_number
       >> state.last_move.row >> state.last_move.col
       >> state.red_seconds >> state.white_seconds;

    return bool(in);
}

void save_game(const History &history)
{
    string name = read_word("Save name (letters, digits, - or _): ");
    if (!is_valid_name(name))
    {
        cout << "That name can't be used as a file name.\n";
        return;
    }

    filesystem::create_directories(SAVE_DIR);
    string file_name = SAVE_DIR + "/" + name + ".txt";

    ofstream out(file_name);
    if (!out)
    {
        cout << "Error saving the game.\n";
        return;
    }

    out << history.states.size() << " " << history.index << "\n";
    for (const GameState &state : history.states)
    {
        write_state(out, state);
    }

    cout << "Game saved as " << name << ".\n";
}

bool load_game(History &history)
{
    vector<string> names;
    if (filesystem::exists(SAVE_DIR))
    {
        for (const auto &entry : filesystem::directory_iterator(SAVE_DIR))
        {
            if (entry.path().extension() == ".txt")
            {
                names.push_back(entry.path().stem().string());
            }
        }
    }

    if (names.empty())
    {
        cout << "There are no saved games yet.\n";
        return false;
    }

    sort(names.begin(), names.end());
    cout << "\nSaved games:\n";
    for (int i = 0; i < (int)names.size(); i++)
    {
        cout << "  [" << i << "] " << names[i] << "\n";
    }
    int choice = read_choice("Choose: ", 0, (int)names.size() - 1);

    ifstream in(SAVE_DIR + "/" + names[choice] + ".txt");
    int size = 0, index = 0;
    in >> size >> index;
    if (!in || size <= 0 || index < 0 || index >= size)
    {
        cout << "That save file is broken.\n";
        return false;
    }

    History loaded;
    loaded.states.resize(size);
    loaded.index = index;
    for (GameState &state : loaded.states)
    {
        if (!read_state(in, state))
        {
            cout << "That save file is broken.\n";
            return false;
        }
    }

    history = loaded;
    return true;
}


void update_scoreboard(const GameState &state)
{
    int game_number = 1;

    ifstream in(SCOREBOARD_FILE);
    string line;
    while (getline(in, line))
    {
        if (!line.empty())
        {
            game_number++;
        }
    }
    in.close();

    ofstream out(SCOREBOARD_FILE, ios::app);
    if (!out)
    {
        cout << "Error updating the scoreboard.\n";
        return;
    }

    int red_count, white_count;
    count_disks(state.board, red_count, white_count);

    // game red white red_time white_time winner opponent
    string opponent = (state.game_mode == MODE_TWO_PLAYERS) ? "human" : ai_mode_name(state.ai_mode);
    out << game_number << " " << red_count << " " << white_count << " "
        << state.red_seconds << " " << state.white_seconds << " "
        << decide_winner(state) << " " << opponent << "\n";
}

void show_scoreboard()
{
    clear_screen();
    print_title("scoreboard");

    ifstream in(SCOREBOARD_FILE);
    int game, red, white;
    time_t red_time, white_time;
    char winner;
    string opponent;

    int red_wins = 0, white_wins = 0, draws = 0, total = 0;
    bool printed_header = false;

    while (in >> game >> red >> white >> red_time >> white_time >> winner >> opponent)
    {
        if (!printed_header)
        {
            cout << color::DIM << "   #   score     winner   mode        red time  white time" << color::RESET << "\n";
            printed_header = true;
        }

        string result = (winner == RED) ? "Red" : (winner == WHITE) ? "White" : "Draw";
        string mode = (opponent == "human") ? "2 players" : "vs " + opponent;

        printf("  %2d   %2d - %-2d   %-6s   %-10s  %8s  %10s\n",
               game, red, white, result.c_str(), mode.c_str(),
               format_time(red_time).c_str(), format_time(white_time).c_str());

        total++;
        if (winner == RED) red_wins++;
        else if (winner == WHITE) white_wins++;
        else draws++;
    }

    if (total == 0)
    {
        cout << "No games played yet.\n";
    }
    else
    {
        cout << "\n  " << total << " games:  " << color::RED << red_wins << " Red" << color::RESET
             << ",  " << color::WHITE << white_wins << " White" << color::RESET << ",  " << draws << " draws\n";
    }

    cout << "\n";
    wait_for_enter("Press enter to go back...");
}
