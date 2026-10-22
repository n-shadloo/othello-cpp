#include "display.h"

#include <cctype>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace std;


void clear_screen()
{
    cout << "\033[2J\033[H";
}

void print_title(const string &subtitle)
{
    cout << color::CYAN << "  O T H E L L O" << color::RESET;
    if (!subtitle.empty())
    {
        cout << color::DIM << "   " << subtitle << color::RESET;
    }
    cout << "\n\n";
}

string player_name(char player)
{
    if (player == RED)
    {
        return string(color::RED) + "Red" + color::RESET;
    }
    return string(color::WHITE) + "White" + color::RESET;
}

static void print_border(const char *left, const char *middle, const char *right)
{
    cout << "    " << left;
    for (int j = 0; j < BOARD_SIZE; j++)
    {
        cout << "───" << (j == BOARD_SIZE - 1 ? right : middle);
    }
    cout << "\n";
}

static void print_cell(char cell, bool is_legal, bool is_last_move)
{
    if (cell == EMPTY)
    {
        if (is_legal)
        {
            cout << " " << color::GREEN << "·" << color::RESET << " ";
        }
        else
        {
            cout << "   ";
        }
        return;
    }

    const char *disk_color = (cell == RED) ? color::RED : color::WHITE;
    if (is_last_move)
    {
        cout << color::YELLOW << "[" << disk_color << "●" << color::YELLOW << "]" << color::RESET;
    }
    else
    {
        cout << " " << disk_color << "●" << color::RESET << " ";
    }
}

void print_board(const Board board, char player, Move last_move, bool show_legal_moves)
{
    cout << "     ";
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        cout << " " << color::DIM << char('a' + i) << color::RESET << "  ";
    }
    cout << "\n";

    print_border("┌", "┬", "┐");
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        cout << "  " << color::DIM << BOARD_SIZE - i << color::RESET << " │";
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            bool is_legal = show_legal_moves && is_legal_move(board, player, i, j);
            bool is_last = (last_move.row == i && last_move.col == j);
            print_cell(board[i][j], is_legal, is_last);
            cout << "│";
        }
        cout << " " << color::DIM << BOARD_SIZE - i << color::RESET << "\n";

        if (i == BOARD_SIZE - 1)
        {
            print_border("└", "┴", "┘");
        }
        else
        {
            print_border("├", "┼", "┤");
        }
    }

    cout << "     ";
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        cout << " " << color::DIM << char('a' + i) << color::RESET << "  ";
    }
    cout << "\n\n";
}

void print_legal_moves(const Board board, char player)
{
    cout << "  Legal moves: " << color::GREEN;
    vector<Move> moves = legal_moves(board, player);
    if (moves.empty())
    {
        cout << "(none)";
    }
    for (const Move &move : moves)
    {
        cout << move_to_string(move) << " ";
    }
    cout << color::RESET << "\n";
}

string format_time(time_t seconds)
{
    ostringstream out;
    out << seconds / 60 << ":" << setw(2) << setfill('0') << seconds % 60;
    return out.str();
}


string read_word(const string &prompt)
{
    cout << prompt;
    string line;
    if (!getline(cin, line))
    {
        // stdin is closed, nothing sensible left to do
        cout << "\n";
        exit(0);
    }

    istringstream words(line);
    string word;
    words >> word;
    for (char &ch : word)
    {
        ch = (char)tolower(ch);
    }
    return word;
}

int read_choice(const string &prompt, int min, int max)
{
    while (true)
    {
        string word = read_word(prompt);
        if (!word.empty() && word.find_first_not_of("0123456789") == string::npos && word.length() < 4)
        {
            int value = stoi(word);
            if (value >= min && value <= max)
            {
                return value;
            }
        }
        cout << "Please enter a number between " << min << " and " << max << ".\n";
    }
}

void wait_for_enter(const string &message)
{
    cout << color::DIM << message << color::RESET;
    string line;
    if (!getline(cin, line))
    {
        cout << "\n";
        exit(0);
    }
}
