#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "raylib.h"

typedef int64_t Value;

static const int SCR_WIDTH = 820;
static const int SCR_HEIGHT = SCR_WIDTH;

static const int BOARD_SIZE = 8;

static const Color BLOCK_COLORS[] = {
    {225, 225, 225, 255}, // Block 0 - None
    {225, 225, 225, 255}, // Block 1 - None
    {255, 204, 21, 255},  // Block 2
    {251, 146, 60, 255},  // Block 4
    {248, 113, 113, 255}, // Block 8
    {96, 165, 250, 255},  // Block 16
    {74, 222, 128, 255},  // Block 32
    {163, 230, 53, 255},  // Block 64
    {52, 211, 153, 255},  // Block 128
    {45, 212, 191, 255},  // Block 256
    {129, 140, 248, 255}, // Block 512
    {167, 139, 250, 255}, // Block 1024
    {192, 132, 252, 255}, // Block 2048
    {232, 121, 249, 255}, // Block 4096
    {251, 113, 133, 255}, // Block 8192
};

// static const Color BLOCK_COLORS[] = {
//     RAYWHITE, RAYWHITE, RED,    ORANGE, YELLOW, PINK,  GREEN,  LIME,
//     SKYBLUE,  BLUE,     PURPLE, VIOLET, BEIGE,  BROWN, MAROON,
// };

int get_bit_num(Value value) {
  int counter = 0;
  while (value != 0) {
    value >>= 1;
    counter++;
  }
  return counter;
}

void draw_game_board(const Value board[BOARD_SIZE][BOARD_SIZE]) {
  // const int block_width = 180;
  // const int padding = 10;
  // const int font_size = 50;
  // const int font_padding = 10;

  const int block_width = 100;
  const int padding = 2;
  const int font_size = 30;
  const int font_padding = 5;
  char text[20];

  for (int i = 0; i < BOARD_SIZE; ++i) {
    for (int j = 0; j < BOARD_SIZE; ++j) {
      const int top_left_x = j * (block_width + padding);
      const int top_left_y = i * (block_width + padding);
      const int value = board[i][j];
      const int color_idx = get_bit_num(value);

      sprintf(text, "%d", value);

      DrawRectangle(top_left_x, top_left_y, block_width, block_width,
                    BLOCK_COLORS[color_idx]);
      if (value != 0) {
        DrawText(text, top_left_x + font_padding, top_left_y + font_padding,
                 font_size, WHITE);
      }
    }
  }
}

void generate_new_title(Value board[BOARD_SIZE][BOARD_SIZE]) {
  int x = GetRandomValue(0, BOARD_SIZE);
  int y = GetRandomValue(0, BOARD_SIZE);
  while (board[x][y] != 0) {
    x = GetRandomValue(0, BOARD_SIZE);
    y = GetRandomValue(0, BOARD_SIZE);
  }
  board[x][y] = 2;
}

void merge_row_left(Value row[BOARD_SIZE]) {
  int left_index = -1;

  // merge
  for (int i = 0; i < BOARD_SIZE; ++i) {
    Value v = row[i];
    if (v == 0) {
      continue;
    }
    if (left_index >= 0 && v == row[left_index]) {
      row[left_index] *= 2;
      row[i] = 0;
      left_index = -1;
    } else {
      left_index = i;
    }
  }

  // push non-zero to left
  left_index = 0;
  for (int i = 0; i < BOARD_SIZE; ++i) {
    Value v = row[i];
    if (v == 0) {
      continue;
    }
    if (left_index != i) {
      row[left_index] = v;
      row[i] = 0;
    }
    left_index++;
  }
}

void reverse_row(Value row[BOARD_SIZE]) {
  for (int i = 0; i < BOARD_SIZE / 2; ++i) {
    Value t = row[BOARD_SIZE - i - 1];
    row[BOARD_SIZE - i - 1] = row[i];
    row[i] = t;
  }
}

void transpose(Value board[BOARD_SIZE][BOARD_SIZE]) {
  for (int i = 0; i < BOARD_SIZE; i++) {
    for (int j = i + 1; j < BOARD_SIZE; j++) {
      Value temp = board[i][j];
      board[i][j] = board[j][i];
      board[j][i] = temp;
    }
  }
}

void rotate_cw(Value board[BOARD_SIZE][BOARD_SIZE]) {
  transpose(board);
  for (int i = 0; i < BOARD_SIZE; ++i) {
    reverse_row(board[i]);
  }
}

void rotate_ccw(Value board[BOARD_SIZE][BOARD_SIZE]) {
  transpose(board);
  for (int i = 0; i < BOARD_SIZE / 2; i++) {
    for (int j = 0; j < BOARD_SIZE; j++) {
      int r = BOARD_SIZE - i - 1;
      Value temp = board[i][j];
      board[i][j] = board[r][j];
      board[r][j] = temp;
    }
  }
}

void push_left(Value board[BOARD_SIZE][BOARD_SIZE]) {
  for (int i = 0; i < BOARD_SIZE; ++i) {
    merge_row_left(board[i]);
  }
}

void push_right(Value board[BOARD_SIZE][BOARD_SIZE]) {
  for (int i = 0; i < BOARD_SIZE; ++i) {
    reverse_row(board[i]);
    merge_row_left(board[i]);
    reverse_row(board[i]);
  }
}

void push_up(Value board[BOARD_SIZE][BOARD_SIZE]) {
  rotate_cw(board);
  push_left(board);
  rotate_ccw(board);
}

void push_down(Value board[BOARD_SIZE][BOARD_SIZE]) {
  rotate_ccw(board);
  push_left(board);
  rotate_cw(board);
}

void update_board(Value board[BOARD_SIZE][BOARD_SIZE]) {
  int c = GetCharPressed();
  bool is_updated = false;
  if (c == 'a') {
    push_left(board);
    is_updated = true;
  }
  if (c == 'd') {
    push_right(board);
    is_updated = true;
  }
  if (c == 's') {
    push_up(board);
    is_updated = true;
  }
  if (c == 'w') {
    push_down(board);
    is_updated = true;
  }
  if (is_updated) {
    generate_new_title(board);
  }
}

int main(void) {
  SetRandomSeed(124);

  Value board[BOARD_SIZE][BOARD_SIZE] = {
      [0] = {2, 4, 8, 16},
      [1] = {32, 64, 128, 256},
      [2] = {512, 1024, 2048, 4096},
      [7] = {2, 0, 4, 2, 0, 0, 2, 16},
  };
  // Value board[BOARD_SIZE][BOARD_SIZE] = {0};

  generate_new_title(board);

  InitWindow(SCR_WIDTH, SCR_HEIGHT, "2048");

  SetExitKey(KEY_Q);

  while (!WindowShouldClose()) {
    // PollInputEvents();

    update_board(board);

    BeginDrawing();
    ClearBackground(GRAY);
    draw_game_board(board);
    EndDrawing();
  }

  CloseWindow();
}
