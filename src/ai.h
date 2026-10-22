#pragma once

#include "board.h"

enum AiMode
{
    AI_DEFENSIVE = 0,
    AI_OFFENSIVE = 1,
    AI_GREEDY = 2,
    AI_RANDOM = 3,
    AI_EXPERT = 4
};

const char *ai_mode_name(int ai_mode);

// it evaluates a move by minimizing the opponent's best possible response
// it calculates how many disks the player flips and subtracts the maximum number of disks the opponent can flip in the next move
int evaluate_defensive_move(const Board board, char player, Move move);
// it evaluates a move by considering the number of disks flipped, board position value and moves that give the opponent access to corners
int evaluate_offensive_move(const Board board, char player, Move move);
// it evaluates a move by combining mobility, board position, disk difference and the number of disks flipped
int evaluate_greedy_move(const Board board, char player, Move move);

// looks EXPERT_DEPTH plies ahead with minimax and alpha-beta pruning
Move ai_choose_expert_move(const Board board, char player);
Move ai_choose_random_move(const Board board, char player);

// returns {-1, -1} when the player has no legal move
Move ai_choose_move(const Board board, char player, int ai_mode);
