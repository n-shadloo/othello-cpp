#include "puzzle.h"

#include <iostream>
#include <random>
#include <string>

#include "ai.h"
#include "board.h"
#include "display.h"

using namespace std;


static void make_random_board(Board board, char &current_player)
{
    static mt19937 gen(random_device{}());
    uniform_int_distribution<> random_move_number(10, 20);
    uniform_int_distribution<> random_ai_mode(AI_DEFENSIVE, AI_RANDOM);

    // retry until the position is still open and the side to move has a real choice
    while (true)
    {
        init_board(board);
        current_player = RED;

        int board_move_number = random_move_number(gen);
        for (int i = 0; i < board_move_number; i++)
        {
            Move move = ai_choose_move(board, current_player, random_ai_mode(gen));
            if (move.row != -1)
            {
                apply_move(board, current_player, move.row, move.col);
            }
            current_player = get_opponent(current_player);
        }

        if (!is_game_over(board) && legal_moves(board, current_player).size() >= 3)
        {
            return;
        }
    }
}

void play_puzzle()
{
    Board board;
    char current_player;
    make_random_board(board, current_player);

    Move best = ai_choose_expert_move(board, current_player);
    Move good = ai_choose_move(board, current_player, AI_GREEDY);

    int attempts = 0;
    string message;

    while (true)
    {
        clear_screen();
        print_title("puzzle");
        print_board(board, current_player, {-1, -1}, true);

        cout << "  " << player_name(current_player) << " to move. Find the best move in this position.\n\n";
        if (!message.empty())
        {
            cout << "  " << message << "\n\n";
        }

        string input = read_word("Your move (or 'give up'): ");
        if (input == "give")
        {
            cout << "\n  The best move was " << color::GREEN << move_to_string(best) << color::RESET << ".\n\n";
            wait_for_enter("Press enter to go back...");
            return;
        }

        Move move;
        if (!parse_move(input, move) || !is_legal_move(board, current_player, move.row, move.col))
        {
            message = "That is not a legal move here.";
            continue;
        }

        attempts++;
        if (move.row == best.row && move.col == best.col)
        {
            cout << "\n  " << color::GREEN << "Correct!" << color::RESET << " Best possible move";
            cout << (attempts == 1 ? ", first try.\n\n" : " (" + to_string(attempts) + " tries).\n\n");
            wait_for_enter("Press enter to go back...");
            return;
        }

        if (move.row == good.row && move.col == good.col)
        {
            message = string(color::YELLOW) + "Good guess!" + color::RESET + " Strong move, but not the best.";
        }
        else
        {
            message = "Incorrect. Try again.";
        }
    }
}
