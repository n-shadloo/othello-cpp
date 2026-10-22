#include "app.h"
#include "draw.h"

int main()
{
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI | FLAG_VSYNC_HINT);
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Othello");
    SetExitKey(KEY_NULL);
    SetTargetFPS(60);
    load_fonts();

    App app;
    while (!WindowShouldClose() && !app.quit)
    {
        Input input = {GetMousePosition(), IsMouseButtonPressed(MOUSE_BUTTON_LEFT), GetTime()};
        app_update(app, input);

        bool over_button = false;
        for (const Button &button : app_buttons(app))
        {
            over_button = over_button || CheckCollisionPointRec(input.mouse, button.rect);
        }
        SetMouseCursor(over_button ? MOUSE_CURSOR_POINTING_HAND : MOUSE_CURSOR_DEFAULT);

        BeginDrawing();
        draw_app(app, input);
        EndDrawing();
    }

    unload_fonts();
    CloseWindow();
    return 0;
}
