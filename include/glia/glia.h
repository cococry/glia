#pragma once

#include <stdint.h>
#include <stdbool.h>

#include <GLFW/glfw3.h>

typedef struct {
    int32_t keycode;
    bool happened, pressed;
} IaKeyEvent;

typedef struct {
    int32_t button_code;
    bool happened, pressed;
} IaMouseButtonEvent;

typedef struct {
    int32_t x, y;
    bool happened;
} IaCursorPosEvent;

typedef struct {
    int32_t xoffset, yoffset;
    bool happened;
} IaScrollEvent;

typedef struct {
    int32_t charcode;
    bool happened;
} IaCharEvent;

void ia_init(GLFWwindow* window);

void ia_update();

void ia_add_key_callback(void* cb);

void ia_add_mouse_button_callback(void* cb);

void ia_add_scroll_callback(void* cb);

void ia_add_cursor_pos_callback(void* cb);

bool ia_key_went_down(uint32_t key);

bool ia_key_is_down(uint32_t key);

bool ia_key_is_released(uint32_t key);

bool ia_key_changed(uint32_t key);

bool ia_mouse_button_went_down(uint32_t button);

bool ia_mouse_button_is_down(uint32_t button);

bool ia_mouse_button_is_released(uint32_t button);

bool ia_mouse_button_changed(uint32_t button);

IaCursorPosEvent ia_mouse_move_event();

IaMouseButtonEvent ia_mouse_button_event();

IaScrollEvent ia_mouse_scroll_event();

IaKeyEvent ia_key_event();

IaCharEvent ia_char_event();
