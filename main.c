#include "raylib.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

typedef int64_t Value;

static const int SCR_WIDTH = 820;
static const int SCR_HEIGHT = SCR_WIDTH;

static const int BOARD_SIZE = 8;

static const Color BLOCK_COLORS[] = {
    RAYWHITE, RED,  ORANGE, YELLOW, PINK,  GREEN, LIME,
    SKYBLUE,  BLUE, PURPLE, VIOLET, BEIGE, BROWN, MAROON,
};

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
      DrawText(text, top_left_x + font_padding, top_left_y + font_padding,
               font_size, LIGHTGRAY);
    }
  }
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
  if (c == 'a') {
    push_left(board);
  }
  if (c == 'd') {
    push_right(board);
  }
  if (c == 's') {
    push_up(board);
  }
  if (c == 'w') {
    push_down(board);
  }
}

int main(void) {
  Value board[BOARD_SIZE][BOARD_SIZE] = {
      [0] = {2, 4, 8, 16},
      [1] = {32, 64, 128, 256},
      [2] = {512, 1024, 2048, 4096},
      [7] = {2, 0, 4, 2, 0, 0, 2, 16},
  };

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
