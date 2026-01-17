#include "raylib.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

// Forward declarations
typedef struct Word Word;
void DrawWordList(Word* words, int wordCount, Font customFont, Color bgColor);

// Game states
typedef enum {
    MENU,
    EASY_PUZZLE,
    HARD_PUZZLE,
    SCORE_SCREEN
} GameState;

// Level difficulty
typedef enum {
    NONE_SELECTED,
    EASY,
    HARD
} Difficulty;

// Word structure for the puzzle
typedef struct Word {
    char word[20];
    bool found;
    int startX, startY;
    int endX, endY;
} Word;

// Global variables
int currentGridSize = 15;
GameState currentState = MENU;
Difficulty selectedLevel = NONE_SELECTED;
int score = 0;
int totalWords = 0;
bool isSelecting = false;
Vector2 selectionStart = { 0, 0 };
Vector2 selectionEnd = { 0, 0 };

// Easy level words (5 songs)
Word easyWords[] = {
    {"BRUTAL", false, 0, 0, 0, 0},
    {"TRAITOR", false, 0, 0, 0, 0},
    {"DRIVERSLICENSE", false, 0, 0, 0, 0},
    {"HAPPIER", false, 0, 0, 0, 0},
    {"DEJAVU", false, 0, 0, 0, 0}
};

// Hard level words (11 songs from SOUR album)
Word hardWords[] = {
    {"BRUTAL", false, 0, 0, 0, 0},
    {"TRAITOR", false, 0, 0, 0, 0},
    {"DRIVERSLICENSE", false, 0, 0, 0, 0},
    {"FORWARDBACKWARD", false, 0, 0, 0, 0},
    {"ENOUGH", false, 0, 0, 0, 0},
    {"HAPPIER", false, 0, 0, 0, 0},
    {"JEALOUSY", false, 0, 0, 0, 0},
    {"FAVORITECRIME", false, 0, 0, 0, 0},
    {"HOPEURNOK", false, 0, 0, 0, 0},
    {"GOODFORU", false, 0, 0, 0, 0},
    {"DEJAVU", false, 0, 0, 0, 0}
};

// Word search grid
char grid[15][15];
int gridSize = 15;

// Function to initialize grid with random letters
void InitializeGrid(int size) {
    currentGridSize = size;
    srand((unsigned int)time(NULL));
    for (int i = 0; i < currentGridSize; i++) {
        for (int j = 0; j < currentGridSize; j++) {
            grid[i][j] = 'A' + (rand() % 26);
        }
    }
}

void PlaceWords(Word* words, int wordCount) {
    for (int w = 0; w < wordCount; w++) {
        int len = (int)strlen(words[w].word);
        bool placed = false;
        int attempts = 0;

        while (!placed && attempts < 100) {
            int direction = rand() % 3; // 0=horizontal, 1=vertical, 2=diagonal
            int maxStartX = (direction == 0 || direction == 2) ? currentGridSize - len : currentGridSize - 1;
            int maxStartY = (direction == 1 || direction == 2) ? currentGridSize - len : currentGridSize - 1;

            if (maxStartX < 0) maxStartX = 0;
            if (maxStartY < 0) maxStartY = 0;

            int startX = rand() % (maxStartX + 1);
            int startY = rand() % (maxStartY + 1);

            bool canPlace = true;

            // Check if word can be placed
            for (int i = 0; i < len; i++) {
                int x = startX + (direction == 0 ? i : (direction == 2 ? i : 0));
                int y = startY + (direction == 1 ? i : (direction == 2 ? i : 0));

                if (x >= currentGridSize || y >= currentGridSize || x < 0 || y < 0) {
                    canPlace = false;
                    break;
                }
            }

            if (canPlace) {
                // Place the word
                for (int i = 0; i < len; i++) {
                    int x = startX + (direction == 0 ? i : (direction == 2 ? i : 0));
                    int y = startY + (direction == 1 ? i : (direction == 2 ? i : 0));
                    grid[y][x] = words[w].word[i];
                }

                words[w].startX = startX;
                words[w].startY = startY;
                words[w].endX = startX + (direction == 0 ? len - 1 : (direction == 2 ? len - 1 : 0));
                words[w].endY = startY + (direction == 1 ? len - 1 : (direction == 2 ? len - 1 : 0));
                placed = true;
            }
            attempts++;
        }
    }
}

// Function to check if a word is selected
// Function to check if a word is selected
bool CheckWordSelection(Word* words, int wordCount, Vector2 start, Vector2 end) {
    int cellSize = (currentState == EASY_PUZZLE) ? 35 : 22; // Smaller cells for hard level
    int startX = 150;  // Same X position for both levels
    int startY = (currentState == EASY_PUZZLE) ? 150 : 180; // Different Y positions

    int gridX1 = (int)((start.x - startX) / cellSize);
    int gridY1 = (int)((start.y - startY) / cellSize);
    int gridX2 = (int)((end.x - startX) / cellSize);
    int gridY2 = (int)((end.y - startY) / cellSize);

    // Boundary checks
    if (gridX1 < 0 || gridX1 >= currentGridSize || gridY1 < 0 || gridY1 >= currentGridSize ||
        gridX2 < 0 || gridX2 >= currentGridSize || gridY2 < 0 || gridY2 >= currentGridSize) {
        return false;
    }

    for (int w = 0; w < wordCount; w++) {
        if (words[w].found) continue;

        if ((gridX1 == words[w].startX && gridY1 == words[w].startY &&
            gridX2 == words[w].endX && gridY2 == words[w].endY) ||
            (gridX1 == words[w].endX && gridY1 == words[w].endY &&
                gridX2 == words[w].startX && gridY2 == words[w].startY)) {
            words[w].found = true;
            return true;
        }
    }
    return false;
}

// Function to draw the word search grid
void DrawWordSearchGrid(Word* words, int wordCount) {
    int cellSize = (currentState == EASY_PUZZLE) ? 35 : 22; // Smaller cells for hard level
    int startX = 150;  // Same X position for both levels
    int startY = (currentState == EASY_PUZZLE) ? 150 : 180; // Different Y positions

    // Draw grid
    for (int i = 0; i < currentGridSize; i++) {
        for (int j = 0; j < currentGridSize; j++) {
            Rectangle cell = { (float)(startX + j * cellSize), (float)(startY + i * cellSize), (float)cellSize, (float)cellSize };

            // Check if this cell is part of a found word
            bool isPartOfFoundWord = false;
            for (int w = 0; w < wordCount; w++) {
                if (words[w].found) {
                    // Check if current cell (j, i) is part of this word
                    int dx = words[w].endX - words[w].startX;
                    int dy = words[w].endY - words[w].startY;
                    int steps = (dx == 0) ? abs(dy) : abs(dx);

                    for (int step = 0; step <= steps; step++) {
                        int wordX = words[w].startX + (dx == 0 ? 0 : (dx > 0 ? step : -step));
                        int wordY = words[w].startY + (dy == 0 ? 0 : (dy > 0 ? step : -step));

                        if (wordX == j && wordY == i) {
                            isPartOfFoundWord = true;
                            break;
                        }
                    }
                    if (isPartOfFoundWord) break;
                }
            }

            // Draw cell background
            if (isPartOfFoundWord) {
                Color foundWordColor = { 147, 112, 219, 150 };
                DrawRectangleRec(cell, foundWordColor);
            }
            else {
                DrawRectangleRec(cell, WHITE);
            }

            DrawRectangleLinesEx(cell, 1, BLACK);

            char letter[2] = { grid[i][j], '\0' };
            int fontSize = (currentState == EASY_PUZZLE) ? 20 : 14; // Smaller font for hard level
            int offsetX = (currentState == EASY_PUZZLE) ? 12 : 6;
            int offsetY = (currentState == EASY_PUZZLE) ? 8 : 4;
            DrawText(letter, startX + j * cellSize + offsetX, startY + i * cellSize + offsetY, fontSize, DARKPURPLE);
        }
    }

    // Draw current selection
    if (isSelecting) {
        DrawLineEx(selectionStart, selectionEnd, 2, RED);
    }
}
// Function to draw word list
void DrawWordList(Word* words, int wordCount, Font customFont, Color bgColor) {
    // Title at the top with background rectangle
    int titleX = 200;
    int titleY = 50;
    const char* titleText = "Find These Songs:";
    Vector2 titleSize = MeasureTextEx(customFont, titleText, 32, 0);

    Rectangle titleBox = { (float)(titleX - 20), (float)(titleY - 10), titleSize.x + 40, titleSize.y + 20 };
    DrawRectangleRec(titleBox, bgColor);
    DrawRectangleLinesEx(titleBox, 3, WHITE);

    Vector2 titlePos = { (float)titleX, (float)titleY };
    DrawTextEx(customFont, titleText, titlePos, 32, 0, WHITE);

    // Songs list box - bigger for hard level
    int boxX = 50;
    int boxY = 80;
    int boxWidth = 500;
    int boxHeight = (wordCount == 5) ? 60 : 100; // Taller box for hard level

    // Draw songs list background box
    Rectangle songsBox = { (float)boxX, (float)boxY, (float)boxWidth, (float)boxHeight };
    DrawRectangleRec(songsBox, bgColor);
    DrawRectangleLinesEx(songsBox, 2, WHITE);

    const char* easyDisplayNames[] = {
        "brutal",
        "traitor",
        "drivers license",
        "happier",
        "deja vu"
    };

    const char* hardDisplayNames[] = {
        "brutal",
        "traitor",
        "drivers license",
        "forward backward",
        "enough for you",
        "happier",
        "jealousy, jealousy",
        "favorite crime",
        "hope ur ok",
        "good 4 u",
        "deja vu"
    };

    const char** displayNames = (wordCount == 5) ? easyDisplayNames : hardDisplayNames;

    // Draw songs in horizontal layout inside the box
    int songsPerRow = (wordCount == 5) ? 3 : 3; // 3 per row for both levels
    int songStartX = boxX + 10;
    int songStartY = boxY + 10;

    for (int i = 0; i < wordCount; i++) {
        int row = i / songsPerRow;
        int col = i % songsPerRow;

        int x = songStartX + col * (boxWidth / songsPerRow);
        int y = songStartY + row * 18; // Reduced spacing for hard level

        Color color = words[i].found ? GREEN : WHITE;
        Vector2 songPos = { (float)x, (float)y };
        DrawTextEx(customFont, displayNames[i], songPos, 14, 0, color); // Smaller font
    }

    // Status box at bottom - same position for both levels
    int statusY = 450; // Same position for both levels
    Rectangle statusBox = { (float)boxX, (float)statusY, (float)boxWidth, 80 };
    DrawRectangleRec(statusBox, bgColor);
    DrawRectangleLinesEx(statusBox, 2, WHITE);

    // Status text inside the box
    Vector2 foundPos = { (float)(boxX + 20), (float)(statusY + 10) };
    Vector2 finishPos = { (float)(boxX + 20), (float)(statusY + 35) };

    DrawTextEx(customFont, TextFormat("Found: %d/%d", score, totalWords), foundPos, 24, 0, WHITE);
    DrawTextEx(customFont, "Press F to finish", finishPos, 20, 0, WHITE);
}

int main() {
    const int screenWidth = 600;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "SOUR Word Search");
    SetTargetFPS(60);

    // Load resources
    Texture2D background = LoadTexture("sour_background.jpg");
    Font font = LoadFont("font.ttf");

    Color backgroundColor = { 117, 23, 136, 255 }; // Dark Purple
    int fontSize = 40;

    while (!WindowShouldClose()) {
        Vector2 mousePos = GetMousePosition();

        if (currentState == MENU) {
            // Handle level selection
            float squareSize = 100;
            float spacing = 40;
            float squareY = 250;
            float easyX = (screenWidth / 2.0f) - squareSize - spacing / 2;
            float hardX = (screenWidth / 2.0f) + spacing / 2;

            Rectangle easyRect = { easyX, squareY, squareSize, squareSize };
            Rectangle hardRect = { hardX, squareY, squareSize, squareSize };

            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                if (CheckCollisionPointRec(mousePos, easyRect)) {
                    selectedLevel = EASY;
                }
                else if (CheckCollisionPointRec(mousePos, hardRect)) {
                    selectedLevel = HARD;
                }

                // Start puzzle button
                float buttonWidth = 250;
                float buttonHeight = 80;
                float centerX = (screenWidth - buttonWidth) / 2;
                float bottomY = screenHeight - buttonHeight - 150;
                Rectangle startRect = { centerX, bottomY, buttonWidth, buttonHeight };

                if (CheckCollisionPointRec(mousePos, startRect) && selectedLevel != NONE_SELECTED) {
                    if (selectedLevel == EASY) {
                        InitializeGrid(8);  // 8x8 grid for easy
                        PlaceWords(easyWords, 5);
                        totalWords = 5;
                        currentState = EASY_PUZZLE;
                    }
                    else {
                        InitializeGrid(12);  // 12x12 grid for hard
                        PlaceWords(hardWords, 11);
                        totalWords = 11;
                        currentState = HARD_PUZZLE;
                    }
                    score = 0;
                }
            }
        }
        else if (currentState == EASY_PUZZLE || currentState == HARD_PUZZLE) {
            Word* currentWords = (currentState == EASY_PUZZLE) ? easyWords : hardWords;
            int wordCount = (currentState == EASY_PUZZLE) ? 5 : 11;

            // Handle word selection
            if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                selectionStart = mousePos;
                isSelecting = true;
            }

            if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && isSelecting) {
                selectionEnd = mousePos;
            }

            if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON) && isSelecting) {
                if (CheckWordSelection(currentWords, wordCount, selectionStart, selectionEnd)) {
                    score++;
                }
                isSelecting = false;
            }

            // Check if finish button is pressed
            if (IsKeyPressed(KEY_F)) {
                currentState = SCORE_SCREEN;
            }
        }

        else if (currentState == SCORE_SCREEN) {
            if (IsKeyPressed(KEY_R)) {
                currentState = MENU;
                selectedLevel = NONE_SELECTED;
                score = 0;

                // Reset word states
                for (int i = 0; i < 5; i++) easyWords[i].found = false;
                for (int i = 0; i < 11; i++) hardWords[i].found = false;
            }
        }

        // =================== DRAW ===================

        BeginDrawing();
        ClearBackground(backgroundColor);

        if (currentState == MENU) {
            DrawTexture(background, 40, 45, WHITE);

            float buttonWidth = 250;
            float buttonHeight = 80;
            float centerX = (screenWidth - buttonWidth) / 2;

            // SELECT LEVEL (top)
            float selectY = 150;
            Rectangle select = { centerX, selectY, buttonWidth, buttonHeight };
            DrawRectangleRec(select, LIGHTGRAY);
            DrawRectangleLinesEx(select, 4, DARKPURPLE);

            const char* selectText = "Select Level";
            Vector2 selectTextPos = {
                select.x + (buttonWidth - MeasureTextEx(font, selectText, (float)fontSize, 0).x) / 2,
                select.y + (buttonHeight - fontSize) / 2
            };
            DrawTextEx(font, selectText, selectTextPos, (float)fontSize, 0, DARKPURPLE);

            // EASY + HARD buttons
            float squareSize = 100;
            float spacing = 40;
            float squareY = 250;
            float easyX = (screenWidth / 2.0f) - squareSize - spacing / 2;
            float hardX = (screenWidth / 2.0f) + spacing / 2;

            Rectangle easy = { easyX, squareY, squareSize, squareSize };
            Rectangle hard = { hardX, squareY, squareSize, squareSize };

            Color easyColor = (selectedLevel == EASY) ? DARKPURPLE : LIGHTGRAY;
            Color hardColor = (selectedLevel == HARD) ? DARKPURPLE : LIGHTGRAY;

            DrawRectangleRec(easy, easyColor);
            DrawRectangleLinesEx(easy, 4, DARKPURPLE);
            DrawRectangleRec(hard, hardColor);
            DrawRectangleLinesEx(hard, 4, DARKPURPLE);

            const char* easyText = "Easy";
            const char* hardText = "Hard";

            Vector2 easyTextPos = {
                easy.x + (squareSize - MeasureTextEx(font, easyText, (float)fontSize, 0).x) / 2,
                easy.y + (squareSize - fontSize) / 2
            };
            Vector2 hardTextPos = {
                hard.x + (squareSize - MeasureTextEx(font, hardText, (float)fontSize, 0).x) / 2,
                hard.y + (squareSize - fontSize) / 2
            };

            DrawTextEx(font, easyText, easyTextPos, (float)fontSize, 0, (selectedLevel == EASY) ? WHITE : DARKPURPLE);
            DrawTextEx(font, hardText, hardTextPos, (float)fontSize, 0, (selectedLevel == HARD) ? WHITE : DARKPURPLE);

            // START PUZZLE (bottom)
            float bottomY = screenHeight - buttonHeight - 150;
            Rectangle start = { centerX, bottomY, buttonWidth, buttonHeight };
            Color startColor = (selectedLevel != NONE_SELECTED) ? DARKPURPLE : GRAY;
            DrawRectangleRec(start, startColor);
            DrawRectangleLinesEx(start, 4, DARKPURPLE);

            const char* startText = "Start Puzzle";
            Vector2 startTextPos = {
                start.x + (buttonWidth - MeasureTextEx(font, startText, (float)fontSize, 0).x) / 2,
                start.y + (buttonHeight - fontSize) / 2
            };
            DrawTextEx(font, startText, startTextPos, (float)fontSize, 0, (selectedLevel != NONE_SELECTED) ? WHITE : DARKPURPLE);
        }

        else if (currentState == EASY_PUZZLE) {
            Word* currentWords = easyWords;
            int wordCount = 5;

            // Draw purple background over the image
            DrawTexture(background, 40, 45, WHITE);

            DrawWordList(currentWords, wordCount, font, backgroundColor);
            DrawWordSearchGrid(currentWords, wordCount);
        }

        else if (currentState == HARD_PUZZLE) {
            Word* currentWords = hardWords;
            int wordCount = 11;

            // Draw purple background over the image (same as easy)
            DrawTexture(background, 40, 45, WHITE);

            DrawWordList(currentWords, wordCount, font, backgroundColor);
            DrawWordSearchGrid(currentWords, wordCount);
        }

        else if (currentState == SCORE_SCREEN) {
            // Draw background image on top of purple background
            DrawTexture(background, 40, 45, WHITE);

            // Main completion box
            Rectangle completeBox = { 100, 180, 400, 120 };
            DrawRectangleRec(completeBox, backgroundColor);
            DrawRectangleLinesEx(completeBox, 3, WHITE);

            Vector2 completePos = { 150, 200 };
            DrawTextEx(font, "Puzzle Complete!", completePos, 32, 0, WHITE);

            Vector2 scorePos = { 120, 240 };
            DrawTextEx(font, TextFormat("You found %d/%d words!", score, totalWords), scorePos, 24, 0, WHITE);

            // Result message box
            Rectangle resultBox = { 100, 320, 400, 60 };
            DrawRectangleRec(resultBox, backgroundColor);
            DrawRectangleLinesEx(resultBox, 3, WHITE);

            Vector2 resultPos = { 150, 340 };
            if (score == totalWords) {
                DrawTextEx(font, "Perfect! Amazing job!", resultPos, 22, 0, GREEN);
            }
            else {
                DrawTextEx(font, "Good effort! Try again!", resultPos, 22, 0, YELLOW);
            }

            // Instructions box
            Rectangle instructBox = { 100, 400, 400, 50 };
            DrawRectangleRec(instructBox, backgroundColor);
            DrawRectangleLinesEx(instructBox, 3, WHITE);

            Vector2 instructPos = { 150, 415 };
            DrawTextEx(font, "Press R to return to menu", instructPos, 20, 0, WHITE);
        }

        EndDrawing();
    }

    // Cleanup
    UnloadTexture(background);
    UnloadFont(font);
    CloseWindow();

    return 0;
}
