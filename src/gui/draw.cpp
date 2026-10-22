#include "draw.h"

#include <cmath>
#include <cstdio>
#include <string>

#include "../ai.h"

using namespace std;

const Color BACKGROUND = {16, 20, 28, 255};
const Color SURFACE = {26, 32, 44, 255};
const Color SURFACE_HOVER = {36, 44, 60, 255};
const Color OUTLINE = {50, 60, 80, 255};
const Color TEXT = {236, 240, 246, 255};
const Color MUTED = {140, 150, 170, 255};
const Color FELT = {30, 120, 76, 255};
const Color FELT_DARK = {22, 98, 61, 255};
const Color FRAME = {14, 52, 34, 255};
const Color DISK_RED = {232, 70, 74, 255};
const Color DISK_RED_DARK = {150, 32, 40, 255};
const Color DISK_WHITE = {244, 244, 238, 255};
const Color DISK_WHITE_DARK = {176, 178, 172, 255};
const Color ACCENT = {72, 214, 140, 255};
const Color HIGHLIGHT = {255, 204, 77, 255};

static string format_time(time_t seconds)
{
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%ld:%02ld", (long)(seconds / 60), (long)(seconds % 60));
    return buffer;
}

static Font regular_font;
static Font bold_font;
static bool fonts_loaded = false;

static const char *REGULAR_FONTS[] = {
    "/System/Library/Fonts/Supplemental/Arial.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
    "C:/Windows/Fonts/arial.ttf",
};
static const char *BOLD_FONTS[] = {
    "/System/Library/Fonts/Supplemental/Arial Bold.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
    "C:/Windows/Fonts/arialbd.ttf",
};

static Font load_first(const char *const *paths, int count)
{
    for (int i = 0; i < count; i++)
    {
        if (FileExists(paths[i]))
        {
            Font font = LoadFontEx(paths[i], 96, nullptr, 0);
            SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
            return font;
        }
    }
    return GetFontDefault();
}

void load_fonts()
{
    regular_font = load_first(REGULAR_FONTS, 3);
    bold_font = load_first(BOLD_FONTS, 3);
    fonts_loaded = true;
}

void unload_fonts()
{
    if (!fonts_loaded)
    {
        return;
    }
    if (regular_font.texture.id != GetFontDefault().texture.id) UnloadFont(regular_font);
    if (bold_font.texture.id != GetFontDefault().texture.id) UnloadFont(bold_font);
}


enum Align { LEFT_ALIGN, CENTER_ALIGN, RIGHT_ALIGN };

static void text(const string &value, float x, float y, float size, Color color, bool bold = false, Align align = LEFT_ALIGN)
{
    Font font = bold ? bold_font : regular_font;
    Vector2 measured = MeasureTextEx(font, value.c_str(), size, 0);
    if (align == CENTER_ALIGN) x -= measured.x / 2;
    if (align == RIGHT_ALIGN) x -= measured.x;
    DrawTextEx(font, value.c_str(), {x, y}, size, 0, color);
}

static void rounded(Rectangle rect, float radius, Color color)
{
    float roundness = radius * 2 / fminf(rect.width, rect.height);
    DrawRectangleRounded(rect, roundness, 12, color);
}

static void rounded_outline(Rectangle rect, float radius, Color color)
{
    float roundness = radius * 2 / fminf(rect.width, rect.height);
    DrawRectangleRoundedLines(rect, roundness, 12, color);
}


static void draw_disk(Vector2 center, float radius, char player, float squash)
{
    Color main = (player == RED) ? DISK_RED : DISK_WHITE;
    Color dark = (player == RED) ? DISK_RED_DARK : DISK_WHITE_DARK;

    // squash < 1 makes the disk look like it is turning on its side
    float width = radius * fmaxf(squash, 0.04f);
    DrawEllipse((int)center.x, (int)center.y + 4, width, radius, Fade(BLACK, 0.35f));
    DrawEllipse((int)center.x, (int)center.y, width, radius, dark);
    DrawEllipse((int)center.x, (int)center.y - 2, width * 0.92f, radius * 0.92f, main);
    if (squash > 0.5f)
    {
        DrawEllipse((int)(center.x - width * 0.3f), (int)(center.y - radius * 0.35f),
                    width * 0.28f, radius * 0.18f, Fade(Color{255, 255, 255, 255}, 0.35f));
    }
}

static Vector2 cell_center(int row, int col)
{
    return {BOARD_LEFT + col * CELL + CELL / 2, BOARD_TOP + row * CELL + CELL / 2};
}

static void draw_board(const App &app, const Input &input)
{
    Rectangle frame = {BOARD_LEFT - 14, BOARD_TOP - 14, BOARD_PIXELS + 28, BOARD_PIXELS + 28};
    rounded(frame, 18, FRAME);
    DrawRectangle((int)BOARD_LEFT, (int)BOARD_TOP, (int)BOARD_PIXELS, (int)BOARD_PIXELS, FELT);

    for (int i = 0; i <= BOARD_SIZE; i++)
    {
        float offset = i * CELL;
        DrawLineEx({BOARD_LEFT + offset, BOARD_TOP}, {BOARD_LEFT + offset, BOARD_TOP + BOARD_PIXELS}, 2, FELT_DARK);
        DrawLineEx({BOARD_LEFT, BOARD_TOP + offset}, {BOARD_LEFT + BOARD_PIXELS, BOARD_TOP + offset}, 2, FELT_DARK);
    }
    for (int r : {2, 6})
    {
        for (int c : {2, 6})
        {
            DrawCircleV({BOARD_LEFT + c * CELL, BOARD_TOP + r * CELL}, 5, FELT_DARK);
        }
    }

    for (int i = 0; i < BOARD_SIZE; i++)
    {
        text(string(1, char('a' + i)), BOARD_LEFT + i * CELL + CELL / 2, BOARD_TOP - 40, 18, MUTED, true, CENTER_ALIGN);
        text(to_string(BOARD_SIZE - i), BOARD_LEFT - 34, BOARD_TOP + i * CELL + CELL / 2 - 10, 18, MUTED, true, CENTER_ALIGN);
    }

    double since_move = input.now - app.move_time;
    float progress = (float)fmin(1.0, since_move / FLIP_SECONDS);
    bool human = is_human_turn(app) && !app.over && app.screen == SCREEN_GAME;

    Move hover = {-1, -1};
    bool hovering = human && square_at(input.mouse, hover);

    for (int row = 0; row < BOARD_SIZE; row++)
    {
        for (int col = 0; col < BOARD_SIZE; col++)
        {
            Vector2 center = cell_center(row, col);
            char now_cell = app.state.board[row][col];
            char before = app.previous_board[row][col];
            bool is_last = app.state.last_move.row == row && app.state.last_move.col == col;

            if (now_cell == EMPTY)
            {
                if (human && is_legal_move(app.state.board, app.state.current_player, row, col))
                {
                    bool hovered = hovering && hover.row == row && hover.col == col;
                    if (hovered)
                    {
                        draw_disk(center, CELL * 0.38f, app.state.current_player, 1);
                        DrawCircleV(center, CELL * 0.38f, Fade(FELT, 0.45f));
                    }
                    else
                    {
                        DrawCircleV(center, 8, Fade(BLACK, 0.28f));
                    }
                }
                continue;
            }

            float squash = 1;
            char shown = now_cell;
            if (progress < 1 && before != EMPTY && before != now_cell)
            {
                // first half shows the old color shrinking, second half the new one growing back
                squash = fabsf(cosf(progress * PI));
                shown = progress < 0.5f ? before : now_cell;
            }
            else if (progress < 1 && is_last && before == EMPTY)
            {
                squash = 1;
                float pop = 0.7f + 0.3f * fminf(1, progress * 2.5f);
                draw_disk(center, CELL * 0.4f * pop, shown, squash);
                continue;
            }

            draw_disk(center, CELL * 0.4f, shown, squash);
            if (is_last && progress >= 1)
            {
                DrawCircleV(center, 5, Fade(HIGHLIGHT, 0.95f));
            }
        }
    }

    if (app.hint.row != -1)
    {
        Vector2 center = cell_center(app.hint.row, app.hint.col);
        float pulse = 0.5f + 0.5f * sinf((float)input.now * 6);
        DrawRing(center, CELL * 0.36f, CELL * 0.36f + 4 + 3 * pulse, 0, 360, 48, HIGHLIGHT);
    }
}


static void draw_button(const Button &button, Vector2 mouse)
{
    bool hovered = CheckCollisionPointRec(mouse, button.rect);
    Color fill = button.primary ? ACCENT : (button.selected ? SURFACE_HOVER : SURFACE);
    if (hovered && !button.primary) fill = SURFACE_HOVER;
    if (hovered && button.primary) fill = {96, 232, 160, 255};

    rounded(button.rect, 12, fill);
    if (!button.primary)
    {
        rounded_outline(button.rect, 12, button.selected ? ACCENT : OUTLINE);
    }

    Color color = button.primary ? Color{10, 30, 20, 255} : (button.selected || hovered ? TEXT : MUTED);
    float size = button.rect.height > 50 ? 22 : 18;
    text(button.label, button.rect.x + button.rect.width / 2, button.rect.y + button.rect.height / 2 - size / 2 - 1,
         size, color, true, CENTER_ALIGN);
}

static void draw_player_card(const App &app, char player, float y)
{
    const GameState &state = app.state;
    bool active = !app.over && state.current_player == player;
    Rectangle card = {PANEL_LEFT, y, PANEL_WIDTH, 112};
    rounded(card, 16, SURFACE);
    rounded_outline(card, 16, active ? ACCENT : OUTLINE);

    draw_disk({card.x + 42, card.y + 56}, 22, player, 1);

    string name = player == RED ? "Red" : "White";
    string who = "Player";
    if (state.game_mode == MODE_VS_AI)
    {
        who = (player == state.human_color) ? "You" : string(ai_mode_name(state.ai_mode)) + " AI";
    }
    text(name, card.x + 80, card.y + 24, 24, TEXT, true);
    text(who, card.x + 80, card.y + 56, 17, MUTED);

    int red_count, white_count;
    count_disks(state.board, red_count, white_count);
    text(to_string(player == RED ? red_count : white_count), card.x + card.width - 22, card.y + 26, 50, TEXT, true, RIGHT_ALIGN);

    if (state.game_mode == MODE_TWO_PLAYERS && state.pvp_timer_mode == 1)
    {
        time_t left = TIME_LIMIT_SECONDS - clock_for(app, player);
        text(format_time(left > 0 ? left : 0), card.x + 80, card.y + 80, 17, left < 30 ? DISK_RED : HIGHLIGHT, true);
    }
}

static void draw_game(const App &app, const Input &input)
{
    draw_board(app, input);

    text("OTHELLO", PANEL_LEFT, BOARD_TOP - 46, 34, TEXT, true);
    draw_player_card(app, RED, BOARD_TOP + 10);
    draw_player_card(app, WHITE, BOARD_TOP + 136);

    string status;
    if (app.over)
    {
        status = "Game over";
    }
    else if (is_human_turn(app))
    {
        status = app.state.game_mode == MODE_VS_AI ? "Your move" : (app.state.current_player == RED ? "Red to move" : "White to move");
    }
    else
    {
        status = "Computer is thinking...";
    }
    text(status, PANEL_LEFT, BOARD_TOP + 278, 22, TEXT, true);
    string move_line = "Move " + to_string(app.state.move_number);
    if (app.state.last_move.row != -1)
    {
        char last_player = app.state.board[app.state.last_move.row][app.state.last_move.col];
        move_line += string("   |   ") + (last_player == RED ? "Red" : "White") + " played " + move_to_string(app.state.last_move);
    }
    text(move_line, PANEL_LEFT, BOARD_TOP + 308, 17, MUTED);

    int red_count, white_count;
    count_disks(app.state.board, red_count, white_count);
    Rectangle balance = {PANEL_LEFT, BOARD_TOP + 390, PANEL_WIDTH, 14};
    rounded(balance, 7, DISK_WHITE);
    float red_share = (float)red_count / (red_count + white_count);
    rounded({balance.x, balance.y, fmaxf(14, balance.width * red_share), balance.height}, 7, DISK_RED);
    text("BOARD CONTROL", PANEL_LEFT, BOARD_TOP + 364, 14, MUTED, true);

    if (!app.message.empty() && input.now - app.message_time < 2.5)
    {
        text(app.message, PANEL_LEFT, BOARD_TOP + 430, 17, HIGHLIGHT, true);
    }

    for (const Button &button : app_buttons(app))
    {
        if (!app.over)
        {
            draw_button(button, input.mouse);
        }
    }

    if (app.over)
    {
        DrawRectangle((int)BOARD_LEFT, (int)BOARD_TOP, (int)BOARD_PIXELS, (int)BOARD_PIXELS, Fade(BLACK, 0.55f));
        Rectangle box = {BOARD_LEFT + 110, BOARD_TOP + 170, BOARD_PIXELS - 220, 310};
        rounded(box, 22, SURFACE);
        rounded_outline(box, 22, OUTLINE);

        char winner = decide_winner(app.state);
        string headline = winner == DRAW ? "Draw!" : string(winner == RED ? "Red" : "White") + " wins!";
        if (app.state.game_mode == MODE_VS_AI && winner != DRAW)
        {
            headline = winner == app.state.human_color ? "You win!" : "The computer wins";
        }
        float cx = BOARD_LEFT + BOARD_PIXELS / 2;
        text(headline, cx, box.y + 40, 44, winner == DRAW ? TEXT : HIGHLIGHT, true, CENTER_ALIGN);

        int red_count, white_count;
        count_disks(app.state.board, red_count, white_count);
        text(to_string(red_count) + "  -  " + to_string(white_count), cx, box.y + 110, 36, TEXT, true, CENTER_ALIGN);
        string reason = (app.state.red_seconds >= TIME_LIMIT_SECONDS || app.state.white_seconds >= TIME_LIMIT_SECONDS)
                        ? "lost on time" : "saved to the scoreboard";
        text(reason, cx, box.y + 160, 17, MUTED, false, CENTER_ALIGN);

        for (const Button &button : app_buttons(app))
        {
            draw_button(button, input.mouse);
        }
    }
}

static void draw_menu(const App &app, const Input &input)
{
    float cx = WINDOW_WIDTH / 2.0f;

    // a row of disks mid-flip as a little logo
    for (int i = 0; i < 6; i++)
    {
        float phase = (float)input.now * 1.6f - i * 0.45f;
        float squash = fabsf(cosf(phase));
        bool flipped = fmodf(phase / PI + 100, 2) < 1;
        char player = (flipped == (i % 2 == 0)) ? RED : WHITE;
        draw_disk({cx - 150 + i * 60.0f, 90}, 22, player, squash);
    }

    text("OTHELLO", cx, 140, 72, TEXT, true, CENTER_ALIGN);
    text("Flip your opponent's disks. Most disks at the end wins.", cx, 222, 19, MUTED, false, CENTER_ALIGN);

    text("MODE", cx - 300, 272, 15, MUTED, true);
    text(app.game_mode == MODE_VS_AI ? "COMPUTER LEVEL" : "CLOCK", cx - 300, 382, 15, MUTED, true);
    if (app.game_mode == MODE_VS_AI)
    {
        text("YOUR COLOR", cx - 300, 482, 15, MUTED, true);
    }

    for (const Button &button : app_buttons(app))
    {
        draw_button(button, input.mouse);
    }
}

void draw_app(const App &app, const Input &input)
{
    ClearBackground(BACKGROUND);
    DrawRectangleGradientV(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, {20, 26, 36, 255}, {10, 12, 18, 255});

    if (app.screen == SCREEN_MENU)
    {
        draw_menu(app, input);
    }
    else
    {
        draw_game(app, input);
    }
}
