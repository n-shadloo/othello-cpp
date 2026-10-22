#pragma once

#include <ctime>
#include <string>

#include "board.h"

namespace color
{
    const char *const RESET = "\033[0m";
    const char *const BOLD = "\033[1m";
    const char *const DIM = "\033[2m";
    const char *const RED = "\033[1;31m";
    const char *const WHITE = "\033[1;97m";
    const char *const GREEN = "\033[32m";
    const char *const YELLOW = "\033[1;33m";
    const char *const CYAN = "\033[1;36m";
}

void clear_screen();
void print_title(const std::string &subtitle);
std::string player_name(char player);

// last_move is highlighted, {-1, -1} means nothing to highlight
void print_board(const Board board, char player, Move last_move, bool show_legal_moves);
void print_legal_moves(const Board board, char player);
std::string format_time(time_t seconds);

// all input goes through getline, so there is never a stray newline left in cin
std::string read_word(const std::string &prompt);
int read_choice(const std::string &prompt, int min, int max);
void wait_for_enter(const std::string &message = "Press enter to continue...");
