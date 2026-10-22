#include "board.h"

#include <cctype>

using namespace std;


void init_board(Board board)
{
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            board[i][j] = EMPTY;
        }
    }

    board[4][3] = RED;
    board[4][4] = WHITE;
    board[3][3] = WHITE;
    board[3][4] = RED;
}

void copy_board(const Board from, Board to)
{
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        for (int j = 0; j < BOARD_SIZE; j++)
        {
            to[i][j] = from[i][j];
        }
    }
}


bool in_bounds(int row, int col)
{
    return row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE;
}

char get_opponent(char player)
{
    return player == RED ? WHITE : RED;
}


bool check_direction(const Board board, char player, int start_row, int start_col, int dir_row, int dir_col)
{
    // the disk can only be placed on an EMPTY and valid position
    if (!in_bounds(start_row, start_col) || board[start_row][start_col] != EMPTY)
    {
        return false;
    }

    char opponent = get_opponent(player);
    int current_row = start_row + dir_row;
    int current_col = start_col + dir_col;

    // the first disk in that direction must be opponent's disk
    if (!in_bounds(current_row, current_col) || board[current_row][current_col] != opponent)
    {
        return false;
    }

    while (in_bounds(current_row, current_col) && board[current_row][current_col] == opponent)
    {
        current_row += dir_row;
        current_col += dir_col;
    }

    return in_bounds(current_row, current_col) && board[current_row][current_col] == player;
}

bool is_legal_move(const Board board, char player, int row, int col)
{
    if (!in_bounds(row, col) || board[row][col] != EMPTY)
    {
        return false;
    }

    for (int dir_row = -1; dir_row <= 1; dir_row++)
    {
        for (int dir_col = -1; dir_col <= 1; dir_col++)
        {
            if (dir_row == 0 && dir_col == 0)
            {
                continue;
            }

            if (check_direction(board, player, row, col, dir_row, dir_col))
            {
                return true;
            }
        }
    }

    return false;
}


// walks every valid direction and either flips the disks or just counts them
static int walk_flips(Board board, char player, int row, int col, bool flip)
{
    int count = 0;
    char opponent = get_opponent(player);

    for (int dir_row = -1; dir_row <= 1; dir_row++)
    {
        for (int dir_col = -1; dir_col <= 1; dir_col++)
        {
            if (dir_row == 0 && dir_col == 0)
            {
                continue;
            }

            if (!check_direction(board, player, row, col, dir_row, dir_col))
            {
                continue;
            }

            int current_row = row + dir_row;
            int current_col = col + dir_col;
            while (board[current_row][current_col] == opponent)
            {
                if (flip)
                {
                    board[current_row][current_col] = player;
                }
                count++;

                current_row += dir_row;
                current_col += dir_col;
            }
        }
    }

    return count;
}

int apply_move(Board board, char player, int row, int col)
{
    int flipped = walk_flips(board, player, row, col, true);
    board[row][col] = player;
    return flipped;
}

int count_flipped_disks(const Board board, char player, int row, int col)
{
    Board copy;
    copy_board(board, copy);
    return walk_flips(copy, player, row, col, false);
}

void count_disks(const Board board, int &red_count, int &white_count)
{
    red_count = 0;
    white_count = 0;

    for (int row = 0; row < BOARD_SIZE; row++)
    {
        for (int col = 0; col < BOARD_SIZE; col++)
        {
            if (board[row][col] == RED)
            {
                red_count++;
            }
            else if (board[row][col] == WHITE)
            {
                white_count++;
            }
        }
    }
}


vector<Move> legal_moves(const Board board, char player)
{
    vector<Move> moves;
    for (int row = 0; row < BOARD_SIZE; row++)
    {
        for (int col = 0; col < BOARD_SIZE; col++)
        {
            if (is_legal_move(board, player, row, col))
            {
                moves.push_back({row, col});
            }
        }
    }

    return moves;
}

bool has_any_legal_move(const Board board, char player)
{
    for (int row = 0; row < BOARD_SIZE; row++)
    {
        for (int col = 0; col < BOARD_SIZE; col++)
        {
            if (is_legal_move(board, player, row, col))
            {
                return true;
            }
        }
    }

    return false;
}

bool is_game_over(const Board board)
{
    return !has_any_legal_move(board, RED) && !has_any_legal_move(board, WHITE);
}


string move_to_string(Move move)
{
    string text;
    text += char('a' + move.col);
    text += char('1' + (BOARD_SIZE - 1 - move.row));
    return text;
}

bool parse_move(const string &text, Move &move)
{
    if (text.length() != 2)
    {
        return false;
    }

    move.col = tolower(text[0]) - 'a';
    move.row = '8' - text[1];
    return in_bounds(move.row, move.col);
}
