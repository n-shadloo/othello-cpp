#pragma once

#include <string>
#include <vector>

const int BOARD_SIZE = 8;
const char EMPTY = '.';
const char RED = 'R';
const char WHITE = 'W';

struct Move
{
    int row;
    int col;
};

typedef char Board[BOARD_SIZE][BOARD_SIZE];

// set the starting position
void init_board(Board board);
void copy_board(const Board from, Board to);

// it checks if the given row and col are inside the board
bool in_bounds(int row, int col);
char get_opponent(char player);

// it starts checking the given direction from the starting position
// it continues only if the next position is occupied by an opponent disk
// it returns true if it reaches player's disk / it returns false if it reaches EMPTY
// NOTE: starting position must be EMPTY
bool check_direction(const Board board, char player, int start_row, int start_col, int dir_row, int dir_col);
// it returns true if any of the 8 directions returns true in check_direction()
bool is_legal_move(const Board board, char player, int row, int col);

// it flips every opponent line that check_direction() accepts, then places the disk
// it returns how many disks were flipped
int apply_move(Board board, char player, int row, int col);
int count_flipped_disks(const Board board, char player, int row, int col);
void count_disks(const Board board, int &red_count, int &white_count);

std::vector<Move> legal_moves(const Board board, char player);
bool has_any_legal_move(const Board board, char player);
// it returns true only if neither of the two players has any legal move
bool is_game_over(const Board board);

// "c4" <-> {row, col}
std::string move_to_string(Move move);
bool parse_move(const std::string &text, Move &move);
