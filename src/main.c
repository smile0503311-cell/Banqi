#include "raylib.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>

typedef enum { PIECE_RED, PIECE_BLACK } PieceColor;
typedef enum { GENERAL, ADVISOR, ELEPHANT, CHARIOT, HORSE, CANNON, SOLDIER } Role;
typedef enum { STATE_PLAYING, STATE_RED_WIN, STATE_BLACK_WIN } GameState;

typedef struct Piece {
    PieceColor color;
    Role role;
    bool isFlipped;
    bool isEmpty;
} Piece;

const char* RED_NAMES[] = { "帥", "仕", "相", "俥", "傌", "炮", "兵" };
const char* BLACK_NAMES[] = { "將", "士", "象", "車", "馬", "包", "卒" };

Piece boardGrid[4][8];
GameState gameState = STATE_PLAYING;
int currentTurn = -1;
int selectedRow = -1;
int selectedCol = -1;

void InitAndShuffleBoard() {
    Piece deck[32];
    int index = 0;
    int counts[7] = { 1, 2, 2, 2, 2, 2, 5 };
    for (int c = 0; c <= 1; c++) {
        for (int r = 0; r < 7; r++) {
            for (int i = 0; i < counts[r]; i++) {
                deck[index].color = (PieceColor)c;
                deck[index].role = (Role)r;
                deck[index].isFlipped = false;
                deck[index].isEmpty = false;
                index++;
            }
        }
    }
    srand((unsigned int)time(NULL));
    for (int i = 31; i > 0; i--) {
        int j = rand() % (i + 1);
        Piece temp = deck[i];
        deck[i] = deck[j];
        deck[j] = temp;
    }
    index = 0;
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 8; col++) {
            boardGrid[row][col] = deck[index++];
        }
    }
    gameState = STATE_PLAYING;
    currentTurn = -1;
    selectedRow = -1;
    selectedCol = -1;
}

bool IsValidMove(int sr, int sc, int dr, int dc) {
    Piece pSrc = boardGrid[sr][sc];
    Piece pDest = boardGrid[dr][dc];
    if (sr != dr && sc != dc) return false;
    int dist = abs(sr - dr) + abs(sc - dc);
    if (pSrc.role == CANNON) {
        if (pDest.isEmpty) return dist == 1;
        int count = 0;
        if (sr == dr) {
            int minC = (sc < dc) ? sc : dc;
            int maxC = (sc > dc) ? sc : dc;
            for (int c = minC + 1; c < maxC; c++) if (!boardGrid[sr][c].isEmpty) count++;
        }
        else {
            int minR = (sr < dr) ? sr : dr;
            int maxR = (sr > dr) ? sr : dr;
            for (int r = minR + 1; r < maxR; r++) if (!boardGrid[r][sc].isEmpty) count++;
        }
        return (count == 1 && pDest.isFlipped && pDest.color != pSrc.color);
    }
    if (dist != 1) return false;
    if (pDest.isEmpty) return true;
    if (!pDest.isFlipped || pSrc.color == pDest.color) return false;
    if (pSrc.role == GENERAL && pDest.role == SOLDIER) return false;
    if (pSrc.role == SOLDIER && pDest.role == GENERAL) return true;
    return pSrc.role <= pDest.role;
}

void CheckWinCondition() {
    bool redGeneralAlive = false;
    bool blackGeneralAlive = false;
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 8; c++) {
            if (!boardGrid[r][c].isEmpty && boardGrid[r][c].role == GENERAL) {
                if (boardGrid[r][c].color == PIECE_RED) redGeneralAlive = true;
                else blackGeneralAlive = true;
            }
        }
    }
    if (currentTurn != -1) {
        if (!redGeneralAlive) gameState = STATE_BLACK_WIN;
        else if (!blackGeneralAlive) gameState = STATE_RED_WIN;
    }
}

int main(void) {
    const int screenWidth = 800;
    const int screenHeight = 450;
    InitWindow(screenWidth, screenHeight, "Banqi - King Survival");
    Texture2D boardTex = LoadTexture("texture/board.png");
    Texture2D pieceBack = LoadTexture("texture/piece_back.png");
    Texture2D pieceRed = LoadTexture("texture/piece_red.png");
    Texture2D pieceBlack = LoadTexture("texture/piece_black.png");
    int cpCount = 0;
    int* cps = LoadCodepoints("帥仕相俥傌炮兵將士象車馬包卒請翻牌決定陣營輪到：紅黑方獲勝！按下R鍵重新開始", &cpCount);
    Font font = LoadFontEx("texture/LXGWWenKaiTC-Bold.ttf", 50, cps, cpCount);
    UnloadCodepoints(cps);
    InitAndShuffleBoard();
    float bx = screenWidth / 2.0f - boardTex.width / 2.0f;
    float by = screenHeight / 2.0f - boardTex.height / 2.0f;
    float sx = 62.0f, sy = 89.0f, rm = 62.0f, bm = 18.0f;
    float cw = (boardTex.width - sx - rm) / 8.0f;
    float ch = (boardTex.height - sy - bm) / 4.0f;
    SetTargetFPS(60);
    while (!WindowShouldClose()) {
        if (gameState == STATE_PLAYING) {
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                Vector2 m = GetMousePosition();
                if (m.x >= bx + sx && m.x <= bx + boardTex.width - rm && m.y >= by + sy && m.y <= by + boardTex.height - bm) {
                    int col = (int)((m.x - (bx + sx)) / cw);
                    int row = (int)((m.y - (by + sy)) / ch);
                    Piece* target = &boardGrid[row][col];
                    if (selectedRow == -1) {
                        if (!target->isEmpty && !target->isFlipped) {
                            target->isFlipped = true;
                            if (currentTurn == -1) currentTurn = (target->color == PIECE_RED) ? 1 : 0;
                            else currentTurn = 1 - currentTurn;
                        }
                        else if (!target->isEmpty && target->isFlipped && (int)target->color == currentTurn) {
                            selectedRow = row; selectedCol = col;
                        }
                    }
                    else {
                        if (row == selectedRow && col == selectedCol) {
                            selectedRow = -1; selectedCol = -1;
                        }
                        else if (!target->isEmpty && target->isFlipped && (int)target->color == currentTurn) {
                            selectedRow = row; selectedCol = col;
                        }
                        else if (IsValidMove(selectedRow, selectedCol, row, col)) {
                            boardGrid[row][col] = boardGrid[selectedRow][selectedCol];
                            boardGrid[selectedRow][selectedCol].isEmpty = true;
                            selectedRow = -1; selectedCol = -1;
                            currentTurn = 1 - currentTurn;
                        }
                    }
                    CheckWinCondition();
                }
            }
        }
        else if (IsKeyPressed(KEY_R)) InitAndShuffleBoard();
        BeginDrawing();
        ClearBackground(RAYWHITE);
        DrawTexture(boardTex, bx, by, WHITE);
        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < 8; c++) {
                Piece p = boardGrid[r][c];
                if (p.isEmpty) continue;
                float px = bx + sx + (c * cw) + (cw / 2.0f) - (pieceBack.width / 2.0f);
                float py = by + sy + (r * ch) + (ch / 2.0f) - (pieceBack.height / 2.0f);
                if (!p.isFlipped) DrawTexture(pieceBack, px, py, WHITE);
                else {
                    DrawTexture((p.color == PIECE_RED) ? pieceRed : pieceBlack, px, py, WHITE);
                    const char* n = (p.color == PIECE_RED) ? RED_NAMES[p.role] : BLACK_NAMES[p.role];
                    Vector2 v = MeasureTextEx(font, n, 40, 1);
                    DrawTextEx(font, n, (Vector2) { px + pieceRed.width / 2 - v.x / 2, py + pieceRed.height / 2 - v.y / 2 }, 40, 1, (p.color == PIECE_RED) ? MAROON : BLACK);
                }
            }
        }
        if (selectedRow != -1) DrawRectangleLinesEx((Rectangle) { bx + sx + selectedCol * cw, by + sy + selectedRow * ch, cw, ch }, 4, GOLD);
        const char* t = (currentTurn == -1) ? "請翻牌決定陣營" : (currentTurn == 0 ? "輪到：紅方" : "輪到：黑方");
        DrawRectangle(10, 10, 250, 40, Fade(LIGHTGRAY, 0.8f));
        DrawTextEx(font, t, (Vector2) { 20, 15 }, 24, 1, (currentTurn == 0) ? RED : BLACK);
        if (gameState != STATE_PLAYING) {
            DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.6f));
            const char* w = (gameState == STATE_RED_WIN) ? "紅方獲勝！" : "黑方獲勝！";
            Vector2 ws = MeasureTextEx(font, w, 40, 1);
            DrawTextEx(font, w, (Vector2) { screenWidth / 2 - ws.x / 2, screenHeight / 2 - 20 }, 40, 1, YELLOW);
            
            const char* restartMsg = "按下 R 鍵重新開始";
            Vector2 textSize = MeasureTextEx(font, restartMsg, 20, 1);
            Vector2 pos = {
                (float)screenWidth / 2 - textSize.x / 2,
                (float)screenHeight / 2 + 40
            };
            DrawTextEx(font, restartMsg, pos, 20, 1, RAYWHITE);
        }
        EndDrawing();
    }
    UnloadTexture(boardTex); UnloadTexture(pieceBack); UnloadTexture(pieceRed); UnloadTexture(pieceBlack); UnloadFont(font);
    CloseWindow();
    return 0;
}