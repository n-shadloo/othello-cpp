#include "ai.h"

#include <algorithm>
#include <climits>
#include <random>

using namespace std;

const int EXPERT_DEPTH = 5;

const int BOARD_POSITION_SCORE[BOARD_SIZE][BOARD_SIZE] =
{
    {100, -20,  10,  5,  5, 10, -20, 100},
    {-20, -50,  -2, -2, -2, -2, -50, -20},
    { 10,  -2,   5,  1,  1,  5,  -2,  10},
    {  5,  -2,   1,  1,  1,  1,  -2,   5},
    {  5,  -2,   1,  1,  1,  1,  -2,   5},
    { 10,  -2,   5,  1,  1,  5,  -2,  10},
    {-20, -50,  -2, -2, -2, -2, -50, -20},
    {100, -20,  10,  5,  5, 10, -20, 100}
};

const Move CORNERS[4] = {
    {0, 0},
    {0, BOARD_SIZE - 1},
    {BOARD_SIZE - 1, 0},
    {BOARD_SIZE - 1, BOARD_SIZE - 1}
};


const char *ai_mode_name(int ai_mode)
{
    switch (ai_mode)
    {
        case AI_DEFENSIVE: return "Defensive";
        case AI_OFFENSIVE: return "Offensive";
        case AI_GREEDY: return "Greedy";
        case AI_RANDOM: return "Random";
        case AI_EXPERT: return "Expert";
    }
    return "Unknown";
}

static int corners_given_away(const Board board_after_move, char player)
{
    int count = 0;
    for (const Move &corner : CORNERS)
    {
        if (is_legal_move(board_after_move, get_opponent(player), corner.row, corner.col))
        {
            count++;
        }
    }
    return count;
}


int evaluate_defensive_move(const Board board, char player, Move move)
{
    Board copy;
    copy_board(board, copy);
    int player_flipped = apply_move(copy, player, move.row, move.col);

    int highest_opponent_flipped = 0;
    for (const Move &reply : legal_moves(copy, get_opponent(player)))
    {
        highest_opponent_flipped = max(highest_opponent_flipped,
                                       count_flipped_disks(copy, get_opponent(player), reply.row, reply.col));
    }

    return player_flipped - highest_opponent_flipped;
}

int evaluate_offensive_move(const Board board, char player, Move move)
{
    int score = count_flipped_disks(board, player, move.row, move.col) * 8;
    score += BOARD_POSITION_SCORE[move.row][move.col] * 2;

    Board copy;
    copy_board(board, copy);
    apply_move(copy, player, move.row, move.col);
    score -= corners_given_away(copy, player) * 800;

    return score;
}

int evaluate_greedy_move(const Board board, char player, Move move)
{
    Board copy;
    copy_board(board, copy);
    int flipped = apply_move(copy, player, move.row, move.col);

    int score = 0;
    int mobility = (int)legal_moves(copy, player).size() - (int)legal_moves(copy, get_opponent(player)).size();
    score += mobility * 12;
    score += BOARD_POSITION_SCORE[move.row][move.col] * 8;
    score += flipped * 2;

    int red_count, white_count;
    count_disks(copy, red_count, white_count);
    score += (player == RED) ? (red_count - white_count) : (white_count - red_count);

    score -= corners_given_away(copy, player) * 800;

    return score;
}


// static evaluation of a whole position from player's point of view
static int evaluate_position(const Board board, char player)
{
    char opponent = get_opponent(player);

    int red_count, white_count;
    count_disks(board, red_count, white_count);
    int mine = (player == RED) ? red_count : white_count;
    int theirs = (player == RED) ? white_count : red_count;

    // the game is decided, only the disk count matters now
    if (is_game_over(board))
    {
        if (mine > theirs) return 100000 + mine - theirs;
        if (mine < theirs) return -100000 + mine - theirs;
        return 0;
    }

    int position = 0;
    for (int row = 0; row < BOARD_SIZE; row++)
    {
        for (int col = 0; col < BOARD_SIZE; col++)
        {
            if (board[row][col] == player) position += BOARD_POSITION_SCORE[row][col];
            else if (board[row][col] == opponent) position -= BOARD_POSITION_SCORE[row][col];
        }
    }

    int my_moves = (int)legal_moves(board, player).size();
    int their_moves = (int)legal_moves(board, opponent).size();
    int mobility = 0;
    if (my_moves + their_moves > 0)
    {
        mobility = 100 * (my_moves - their_moves) / (my_moves + their_moves);
    }

    // disk count is a bad guide early on, so it only starts to weigh in near the end
    int empty = BOARD_SIZE * BOARD_SIZE - red_count - white_count;
    int parity_weight = empty < 16 ? 10 : 1;

    return position * 4 + mobility * 5 + (mine - theirs) * parity_weight;
}

static int minimax(const Board board, char me, char to_move, int depth, int alpha, int beta)
{
    if (depth == 0 || is_game_over(board))
    {
        return evaluate_position(board, me);
    }

    vector<Move> moves = legal_moves(board, to_move);
    if (moves.empty())
    {
        return minimax(board, me, get_opponent(to_move), depth - 1, alpha, beta);
    }

    bool maximizing = (to_move == me);
    int best = maximizing ? INT_MIN : INT_MAX;

    for (const Move &move : moves)
    {
        Board next;
        copy_board(board, next);
        apply_move(next, to_move, move.row, move.col);

        int score = minimax(next, me, get_opponent(to_move), depth - 1, alpha, beta);
        if (maximizing)
        {
            best = max(best, score);
            alpha = max(alpha, score);
        }
        else
        {
            best = min(best, score);
            beta = min(beta, score);
        }

        if (beta <= alpha)
        {
            break;
        }
    }

    return best;
}

Move ai_choose_expert_move(const Board board, char player)
{
    vector<Move> moves = legal_moves(board, player);
    if (moves.empty())
    {
        return {-1, -1};
    }

    // trying corner-ish moves first makes alpha-beta cut a lot more branches
    sort(moves.begin(), moves.end(), [](const Move &a, const Move &b) {
        return BOARD_POSITION_SCORE[a.row][a.col] > BOARD_POSITION_SCORE[b.row][b.col];
    });

    Move best_move = moves[0];
    int alpha = INT_MIN;
    for (const Move &move : moves)
    {
        Board next;
        copy_board(board, next);
        apply_move(next, player, move.row, move.col);

        int score = minimax(next, player, get_opponent(player), EXPERT_DEPTH - 1, alpha, INT_MAX);
        if (score > alpha)
        {
            alpha = score;
            best_move = move;
        }
    }

    return best_move;
}

Move ai_choose_random_move(const Board board, char player)
{
    vector<Move> moves = legal_moves(board, player);
    if (moves.empty())
    {
        return {-1, -1};
    }

    static mt19937 gen(random_device{}());
    uniform_int_distribution<> random(0, (int)moves.size() - 1);
    return moves[random(gen)];
}

static Move best_by(const Board board, char player, int (*evaluate)(const Board, char, Move))
{
    vector<Move> moves = legal_moves(board, player);
    if (moves.empty())
    {
        return {-1, -1};
    }

    Move best_move = moves[0];
    int best_score = INT_MIN;
    for (const Move &move : moves)
    {
        int score = evaluate(board, player, move);
        if (score > best_score)
        {
            best_score = score;
            best_move = move;
        }
    }

    return best_move;
}

Move ai_choose_move(const Board board, char player, int ai_mode)
{
    switch (ai_mode)
    {
        case AI_DEFENSIVE: return best_by(board, player, evaluate_defensive_move);
        case AI_OFFENSIVE: return best_by(board, player, evaluate_offensive_move);
        case AI_GREEDY: return best_by(board, player, evaluate_greedy_move);
        case AI_EXPERT: return ai_choose_expert_move(board, player);
        default: return ai_choose_random_move(board, player);
    }
}
