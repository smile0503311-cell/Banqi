#include "raylib.h"

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "Game of Banqi");
    Texture2D board = LoadTexture("texture/board.png");
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawTexture(board, screenWidth/2 - board.width/2, screenHeight/2 - board.height/2, WHITE);
        EndDrawing();
    }

    UnloadTexture(board);
    CloseWindow();
    return 0;
}