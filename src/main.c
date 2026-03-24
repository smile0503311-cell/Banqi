#include "raylib.h"
#include <stdlib.h>
#include <time.h>
#include <math.h> // 需要用到 abs()

// 陣營與棋子種類
typedef enum { PIECE_RED, PIECE_BLACK } PieceColor;
typedef enum { GENERAL, ADVISOR, ELEPHANT, CHARIOT, HORSE, CANNON, SOLDIER } Role;

// 棋子結構體
typedef struct Piece {
    PieceColor color;
    Role role;
    bool isFlipped;
    bool isEmpty;
} Piece;

const char* RED_NAMES[] = { "帥", "仕", "相", "俥", "傌", "炮", "兵" };
const char* BLACK_NAMES[] = { "將", "士", "象", "車", "馬", "包", "卒" };

Piece boardGrid[4][8];

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
}

// --- 新增：判斷移動與吃子是否合法的函數 ---
bool IsValidMove(int sr, int sc, int dr, int dc) {
    Piece pSrc = boardGrid[sr][sc];
    Piece pDest = boardGrid[dr][dc];

    // 必須是直線移動
    if (sr != dr && sc != dc) return false;

    // 計算距離
    int dist = abs(sr - dr) + abs(sc - dc);

    // 【炮/包】的特殊規則
    if (pSrc.role == CANNON) {
        if (pDest.isEmpty) {
            return dist == 1; // 走到空格只能走一步
        }
        else {
            // 吃子必須跳過剛好一個棋子 (俗稱炮台)
            int count = 0;
            if (sr == dr) {
                int minC = (sc < dc) ? sc : dc;
                int maxC = (sc > dc) ? sc : dc;
                for (int c = minC + 1; c < maxC; c++) {
                    if (!boardGrid[sr][c].isEmpty) count++;
                }
            }
            else {
                int minR = (sr < dr) ? sr : dr;
                int maxR = (sr > dr) ? sr : dr;
                for (int r = minR + 1; r < maxR; r++) {
                    if (!boardGrid[r][sc].isEmpty) count++;
                }
            }
            // 只能吃翻開的敵方棋子
            if (count == 1 && pDest.isFlipped && pDest.color != pSrc.color) return true;
            return false;
        }
    }

    // 【其他一般棋子】
    if (dist != 1) return false; // 只能走一步
    if (pDest.isEmpty) return true; // 走到空格合法
    if (!pDest.isFlipped) return false; // 不能吃還沒翻開的暗棋
    if (pSrc.color == pDest.color) return false; // 不能吃自己人

    // 階級吃子判斷: 將/帥(0) ... 卒/兵(6)
    if (pSrc.role == GENERAL && pDest.role == SOLDIER) return false; // 將不能吃卒
    if (pSrc.role == SOLDIER && pDest.role == GENERAL) return true;  // 卒可以吃將

    // 數字越小階級越大，同階可互吃
    return pSrc.role <= pDest.role;
}

int main(void)
{
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "Game of Banqi");

    Texture2D boardTex = LoadTexture("texture/board.png");
    Texture2D pieceBack = LoadTexture("texture/piece_back.png");
    Texture2D pieceRed = LoadTexture("texture/piece_red.png");
    Texture2D pieceBlack = LoadTexture("texture/piece_black.png");

    const char* chessChars = "帥仕相俥傌炮兵將士象車馬包卒輪到紅黑方：請翻開第一顆棋子以決定陣營";
    int codepointCount = 0;
    int* codepoints = LoadCodepoints(chessChars, &codepointCount);
    Font chineseFont = LoadFontEx("texture/LXGWWenKaiTC-Bold.ttf", 50, codepoints, codepointCount);
    UnloadCodepoints(codepoints);

    InitAndShuffleBoard();

    float boardX = screenWidth / 2.0f - boardTex.width / 2.0f;
    float boardY = screenHeight / 2.0f - boardTex.height / 2.0f;

    float startX = 62.0f;
    float startY = 89.0f;
    float rightMargin = 62.0f;
    float bottomMargin = 18.0f;

    float gridRealWidth = boardTex.width - startX - rightMargin;
    float gridRealHeight = boardTex.height - startY - bottomMargin;
    float cellWidth = gridRealWidth / 8.0f;
    float cellHeight = gridRealHeight / 4.0f;

    // --- 新增：遊戲狀態變數 ---
    int currentTurn = -1; // -1: 未決定, 0: 紅方回合, 1: 黑方回合
    int selectedRow = -1; // -1 代表目前沒有選取任何棋子
    int selectedCol = -1;

    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        // --- 邏輯更新 ---
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mousePoint = GetMousePosition();

            if (mousePoint.x >= boardX + startX && mousePoint.x <= boardX + boardTex.width - rightMargin &&
                mousePoint.y >= boardY + startY && mousePoint.y <= boardY + boardTex.height - bottomMargin) {

                int col = (int)((mousePoint.x - (boardX + startX)) / cellWidth);
                int row = (int)((mousePoint.y - (boardY + startY)) / cellHeight);
                Piece* target = &boardGrid[row][col];

                if (selectedRow == -1 && selectedCol == -1) {
                    // 狀況A：目前【沒有】選取棋子
                    if (!target->isEmpty && !target->isFlipped) {
                        // 1. 點擊暗棋：翻牌
                        target->isFlipped = true;

                        // 如果是第一步，決定陣營
                        if (currentTurn == -1) {
                            currentTurn = (target->color == PIECE_RED) ? 1 : 0; // 翻出紅，換黑下
                        }
                        else {
                            currentTurn = 1 - currentTurn; // 換對手回合
                        }
                    }
                    else if (!target->isEmpty && target->isFlipped) {
                        // 2. 點擊明棋：如果是自己的回合，則選取它
                        if (currentTurn != -1 && (int)target->color == currentTurn) {
                            selectedRow = row;
                            selectedCol = col;
                        }
                    }
                }
                else {
                    // 狀況B：目前【已經】選取了一顆自己的棋子
                    if (row == selectedRow && col == selectedCol) {
                        // 1. 點擊自己：取消選取
                        selectedRow = -1;
                        selectedCol = -1;
                    }
                    else if (!target->isEmpty && target->isFlipped && (int)target->color == currentTurn) {
                        // 2. 點擊自己的其他棋子：改為選取新棋子
                        selectedRow = row;
                        selectedCol = col;
                    }
                    else {
                        // 3. 點擊空格 或 敵方明棋：嘗試移動或吃子
                        if (IsValidMove(selectedRow, selectedCol, row, col)) {
                            boardGrid[row][col] = boardGrid[selectedRow][selectedCol]; // 覆蓋目標 (吃子或移動)
                            boardGrid[selectedRow][selectedCol].isEmpty = true;        // 清空原位

                            selectedRow = -1; // 動作完畢，取消選取狀態
                            selectedCol = -1;
                            currentTurn = 1 - currentTurn; // 換對手回合
                        }
                    }
                }
            }
        }

        // --- 畫面繪製 ---
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // 畫棋盤
        DrawTexture(boardTex, boardX, boardY, WHITE);

        // 畫棋子
        for (int row = 0; row < 4; row++) {
            for (int col = 0; col < 8; col++) {
                Piece p = boardGrid[row][col];
                if (p.isEmpty) continue;

                float pieceX = boardX + startX + (col * cellWidth) + (cellWidth / 2.0f) - (pieceBack.width / 2.0f);
                float pieceY = boardY + startY + (row * cellHeight) + (cellHeight / 2.0f) - (pieceBack.height / 2.0f);

                if (!p.isFlipped) {
                    DrawTexture(pieceBack, pieceX, pieceY, WHITE);
                }
                else {
                    Texture2D faceTex = (p.color == PIECE_RED) ? pieceRed : pieceBlack;
                    DrawTexture(faceTex, pieceX, pieceY, WHITE);

                    const char* text = (p.color == PIECE_RED) ? RED_NAMES[p.role] : BLACK_NAMES[p.role];
                    Color textColor = (p.color == PIECE_RED) ? MAROON : DARKGRAY;

                    Vector2 textSize = MeasureTextEx(chineseFont, text, 24, 1);
                    Vector2 textPos = {
                        pieceX + (faceTex.width / 2.0f) - (textSize.x / 2.0f) - 10.0f,
                        pieceY + (faceTex.height / 2.0f) - (textSize.y / 2.0f) - 13.0f
                    };
                    DrawTextEx(chineseFont, text, textPos, 50, 1, textColor);
                }
            }
        }

        // --- 新增：畫選取框提示 ---
        if (selectedRow != -1 && selectedCol != -1) {
            float sx = boardX + startX + (selectedCol * cellWidth);
            float sy = boardY + startY + (selectedRow * cellHeight);
            // 畫一個金黃色的粗框在格子周圍
            DrawRectangleLinesEx((Rectangle) { sx, sy, cellWidth, cellHeight }, 4.0f, GOLD);
        }

        DrawRectangle(10, 10, 420, 50, LIGHTGRAY);
        DrawRectangleLines(10, 10, 420, 50, GRAY);

        const char* turnText = "等待開始";
        Color turnColor = DARKGRAY;

        if (currentTurn == 0) {
            turnText = "輪到：紅方";
            turnColor = MAROON;
        }
        else if (currentTurn == 1) {
            turnText = "輪到：黑方";
            turnColor = BLACK;
        }
        else {
            turnText = "請翻開第一顆棋子以決定陣營";
            turnColor = DARKBLUE;
        }

        // 設定顯示位置（例如畫面的左上角或正上方）
        Vector2 turnPos = { 20.0f, 20.0f };
        DrawTextEx(chineseFont, turnText, turnPos, 30, 2, turnColor);
        EndDrawing();
    }

    UnloadTexture(boardTex);
    UnloadTexture(pieceBack);
    UnloadTexture(pieceRed);
    UnloadTexture(pieceBlack);
    UnloadFont(chineseFont);

    CloseWindow();
    return 0;
}