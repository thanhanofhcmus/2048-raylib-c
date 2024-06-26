#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "raylib.h"

// configure these values
static const int BOARD_SIZE = 4;
static const int GAME_WIDTH = 400;

static const int BLOCK_WIDTH = GAME_WIDTH / BOARD_SIZE;
static const int FONT_SIZE = BLOCK_WIDTH / 4;

static const int GAME_PADDING = 10;
static const int UI_HEIGHT = FONT_SIZE + 10;
static const int GAME_HEIGHT = GAME_WIDTH;
static const int SCR_WIDTH = GAME_WIDTH + GAME_PADDING;
static const int SCR_HEIGHT = GAME_HEIGHT + UI_HEIGHT + GAME_PADDING;

static const Color BLOCK_COLORS[] = {
    {225, 225, 225, 255}, // 0 - None
    {225, 225, 225, 255}, // 1 - None
    {255, 204, 21, 255},  // 2
    {251, 146, 60, 255},  // 4
    {248, 113, 113, 255}, // 8
    {96, 165, 250, 255},  // 16
    {74, 222, 128, 255},  // 32
    {163, 230, 53, 255},  // 64
    {52, 211, 153, 255},  // 128
    {45, 212, 191, 255},  // 256
    {129, 140, 248, 255}, // 512
    {167, 139, 250, 255}, // 1024
    {192, 132, 252, 255}, // 2048
    {232, 121, 249, 255}, // 4096
    {251, 113, 133, 255}, // 8192
};

typedef int64_t Value;
typedef void (*PushFn)(Value board[BOARD_SIZE][BOARD_SIZE]);

int get_bit_num(Value value) {
  int counter = 0;
  while (value != 0) {
    value >>= 1;
    counter++;
  }
  return counter;
}

void generate_new_title(Value board[BOARD_SIZE][BOARD_SIZE]) {
  int x = GetRandomValue(0, BOARD_SIZE - 1);
  int y = GetRandomValue(0, BOARD_SIZE - 1);
  while (board[x][y] != 0) {
    x = GetRandomValue(0, BOARD_SIZE - 1);
    y = GetRandomValue(0, BOARD_SIZE - 1);
  }
  board[x][y] = 2;
}

Value calculate_score(const Value board[BOARD_SIZE][BOARD_SIZE]) {
  Value score = 0;
  for (int i = 0; i < BOARD_SIZE; ++i) {
    for (int j = 0; j < BOARD_SIZE; ++j) {
      score += board[i][j];
    }
  }
  return score;
}

void copy_board(const Value source[BOARD_SIZE][BOARD_SIZE],
                Value dest[BOARD_SIZE][BOARD_SIZE]) {
  memcpy(dest, source, sizeof(Value) * BOARD_SIZE * BOARD_SIZE);
}

bool is_board_equal(const Value b1[BOARD_SIZE][BOARD_SIZE],
                    const Value b2[BOARD_SIZE][BOARD_SIZE]) {
  return memcmp(b1, b2, sizeof(Value) * BOARD_SIZE * BOARD_SIZE) == 0;
}

bool is_board_full(const Value board[BOARD_SIZE][BOARD_SIZE]) {
  for (int i = 0; i < BOARD_SIZE; ++i) {
    for (int j = 0; j < BOARD_SIZE; ++j) {
      if (board[i][j] == 0) {
        return false;
      }
    }
  }
  return true;
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

bool is_game_over(const Value board[BOARD_SIZE][BOARD_SIZE]) {
  // check if game is over by trying to move left, right, up, down.
  // if we can not move then the game is over
  Value tmp[BOARD_SIZE][BOARD_SIZE];
  const PushFn fns[] = {push_left, push_right, push_up, push_down};
  const int len = sizeof(fns) / sizeof(PushFn);

  for (int i = 0; i < len; ++i) {
    copy_board(board, tmp);
    fns[i](tmp);
    if (!is_board_equal(board, tmp)) {
      return false;
    }
  }

  return true;
}

void update_board(Value board[BOARD_SIZE][BOARD_SIZE]) {
  PushFn push_fn = NULL;
  int c = GetCharPressed();
  if (c == 'a') {
    push_fn = push_left;
  }
  if (c == 'd') {
    push_fn = push_right;
  }
  if (c == 's') {
    push_fn = push_up;
  }
  if (c == 'w') {
    push_fn = push_down;
  }
  if (push_fn == NULL) {
    return;
  }

  Value tmp[BOARD_SIZE][BOARD_SIZE];
  copy_board(board, tmp);
  push_fn(tmp);

  if (is_board_equal(board, tmp)) {
    return;
  }

  if (!is_board_full(tmp)) {
    generate_new_title(tmp);
  }

  if (is_game_over(tmp)) {
    printf("Game over\n");
  }

  copy_board(tmp, board);
}

void draw_game_board(const Value board[BOARD_SIZE][BOARD_SIZE]) {
  const int padding = 2;
  const int font_padding = 5;
  char text[20];

  const Value score = calculate_score(board);
  sprintf(text, "Score: %lld", score);
  DrawText(text, GAME_PADDING, GAME_HEIGHT + GAME_PADDING, FONT_SIZE, WHITE);

  for (int i = 0; i < BOARD_SIZE; ++i) {
    for (int j = 0; j < BOARD_SIZE; ++j) {
      const int top_left_x = j * (BLOCK_WIDTH + padding);
      const int top_left_y = i * (BLOCK_WIDTH + padding);
      const int value = board[i][j];
      const int color_idx = get_bit_num(value);

      DrawRectangle(top_left_x, top_left_y, BLOCK_WIDTH, BLOCK_WIDTH,
                    BLOCK_COLORS[color_idx]);
      if (value != 0) {
        sprintf(text, "%d", value);
        DrawText(text, top_left_x + font_padding, top_left_y + font_padding,
                 FONT_SIZE, WHITE);
      }
    }
  }
}

int main(void) {
  SetRandomSeed(124);

  Value board[BOARD_SIZE][BOARD_SIZE] = {0};

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
