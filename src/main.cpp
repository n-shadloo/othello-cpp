#include <iostream>

#include "ai.h"
#include "board.h"
#include "display.h"
#include "game.h"
#include "puzzle.h"
#include "storage.h"

using namespace std;


static void start_new_game()
{
    char human_color = RED;
    int pvp_timer_mode = 0;
    int ai_mode = AI_DEFENSIVE;

    cout << "\nGame mode:\n";
    cout << "  [0] Two players\n";
    cout << "  [1] Single player\n";
    int game_mode = read_choice("Choose: ", 0, 1);

    if (game_mode == MODE_TWO_PLAYERS)
    {
        cout << "\nTimer:\n";
        cout << "  [0] No timer\n";
        cout << "  [1] 5 minutes for each player\n";
        pvp_timer_mode = read_choice("Choose: ", 0, 1);
    }
    else
    {
        cout << "\nAI:\n";
        cout << "  [0] Defensive (easy)\n";
        cout << "  [1] Offensive (medium)\n";
        cout << "  [2] Greedy (hard)\n";
        cout << "  [3] Random (?)\n";
        cout << "  [4] Expert (minimax, very hard)\n";
        ai_mode = read_choice("Choose: ", 0, 4);

        cout << "\nYour color (Red moves first):\n";
        cout << "  [0] Red\n";
        cout << "  [1] White\n";
        human_color = read_choice("Choose: ", 0, 1) == 0 ? RED : WHITE;
    }

    History history;
    init_history(history, new_game_state(game_mode, pvp_timer_mode, ai_mode, human_color));
    play_game(history);
}

int main()
{
    while (true)
    {
        clear_screen();
        print_title("");

        cout << "  [0] New game\n";
        cout << "  [1] Load game\n";
        cout << "  [2] Scoreboard\n";
        cout << "  [3] Puzzle\n";
        cout << "  [4] Exit\n\n";
        int choice = read_choice("Choose: ", 0, 4);

        if (choice == 0)
        {
            start_new_game();
        }
        else if (choice == 1)
        {
            History history;
            if (load_game(history))
            {
                play_game(history);
            }
            else
            {
                wait_for_enter("Press enter to go back...");
            }
        }
        else if (choice == 2)
        {
            show_scoreboard();
        }
        else if (choice == 3)
        {
            play_puzzle();
        }
        else
        {
            cout << "Bye!\n";
            break;
        }
    }

    return 0;
}
