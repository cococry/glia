#include "include/glia/glia.h"

#include <string.h>

#define IA_MAX_KEYS GLFW_KEY_LAST
#define IA_MAX_MOUSE_BUTTONS GLFW_MOUSE_BUTTON_LAST
#define IA_KEY_CALLBACK_t GLFWkeyfun
#define IA_MOUSE_BUTTON_CALLBACK_t GLFWmousebuttonfun
#define IA_SCROLL_CALLBACK_t GLFWscrollfun
#define IA_CURSOR_CALLBACK_t GLFWcursorposfun
#define IA_MAX_KEY_CALLBACKS 4
#define IA_MAX_MOUSE_BTTUON_CALLBACKS 4
#define IA_MAX_SCROLL_CALLBACKS 4
#define IA_MAX_CURSOR_POS_CALLBACKS 4

typedef struct {
  bool keys[IA_MAX_KEYS];
  bool keys_changed[IA_MAX_KEYS];
} IaKeyboard;

typedef struct {
  bool buttons_current[IA_MAX_MOUSE_BUTTONS];
  bool buttons_last[IA_MAX_MOUSE_BUTTONS];

  double xpos, ypos, xpos_last, ypos_last, xpos_delta, ypos_delta;
  bool first_mouse_press; 
  double xscroll_delta, yscroll_delta;
} IaMouse;

// State of input 
typedef struct {
  IaKeyboard keyboard;
  IaMouse mouse;

  // List of callbacks (user defined)
  IA_KEY_CALLBACK_t key_cbs[IA_MAX_KEY_CALLBACKS];
  IA_MOUSE_BUTTON_CALLBACK_t mouse_button_cbs[IA_MAX_MOUSE_BTTUON_CALLBACKS];
  IA_SCROLL_CALLBACK_t scroll_cbs[IA_MAX_SCROLL_CALLBACKS];
  IA_CURSOR_CALLBACK_t cursor_pos_cbs[IA_MAX_CURSOR_POS_CALLBACKS];

  uint32_t key_cb_count, mouse_button_cb_count, scroll_cb_count, cursor_pos_cb_count;

  IaKeyEvent key_ev;
  IaMouseButtonEvent mb_ev;
  IaCursorPosEvent cp_ev;
  IaScrollEvent scr_ev;
  IaCharEvent ch_ev;
} IaInputState;

static IaInputState state;


static void glfw_key_callback(
  GLFWwindow* window, 
  int32_t key, 
  int32_t scancode, 
  int32_t action,
  int32_t mods);

static void glfw_mouse_button_callback(
  GLFWwindow* window, 
  int32_t button, 
  int32_t action, 
  int32_t mods); 
static void glfw_scroll_callback(
  GLFWwindow* window, 
  double xoffset, 
  double yoffset);

static void glfw_cursor_callback(
  GLFWwindow* window, 
  double xpos, 
  double ypos);

static void glfw_char_callback(
  GLFWwindow* window, 
  uint32_t charcode);


static void update_input();
static void clear_events();

void 
glfw_key_callback(
  GLFWwindow* window, 
  int32_t key, 
  int32_t scancode, 
  int32_t action,
  int32_t mods) {
  (void)window;
  (void)mods;
  (void)scancode;
  // Changing the the keys array to resamble the state of the keyboard 
  if(action != GLFW_RELEASE) {
    if(!state.keyboard.keys[key]) 
      state.keyboard.keys[key] = true;
  }  else {
    state.keyboard.keys[key] = false;
  }
  state.keyboard.keys_changed[key] = (action != GLFW_REPEAT);

  // Calling user defined callbacks
  for(uint32_t i = 0; i < state.key_cb_count; i++) {
    state.key_cbs[i](window, key, scancode, action, mods);
  }

  // Populating the key event
  state.key_ev.happened = true;
  state.key_ev.pressed = action != GLFW_RELEASE;
  state.key_ev.keycode = key;
}

void 
glfw_mouse_button_callback(
  GLFWwindow* window, 
  int32_t button, 
  int32_t action, 
  int32_t mods) {
  (void)window;
  (void)mods;
  // Changing the buttons array to resamble the state of the mouse
  if(action != GLFW_RELEASE)  {
    if(!state.mouse.buttons_current[button])
      state.mouse.buttons_current[button] = true;
  } else {
    state.mouse.buttons_current[button] = false;
  }
  // Calling user defined callbacks
  for(uint32_t i = 0; i < state.mouse_button_cb_count; i++) {
    state.mouse_button_cbs[i](window, button, action, mods);
  }
  // Populating the mouse button event
  state.mb_ev.happened = true;
  state.mb_ev.pressed = action != GLFW_RELEASE;
  state.mb_ev.button_code = button;
}

void 
glfw_scroll_callback(
  GLFWwindow* window, 
  double xoffset, 
  double yoffset) {
  (void)window;
  // Setting the scroll values
  state.mouse.xscroll_delta = xoffset;
  state.mouse.yscroll_delta = yoffset;

  // Calling user defined callbacks
  for(uint32_t i = 0; i< state.scroll_cb_count; i++) {
    state.scroll_cbs[i](window, xoffset, yoffset);
  }
  // Populating the scroll event
  state.scr_ev.happened = true;
  state.scr_ev.xoffset = xoffset;
  state.scr_ev.yoffset = yoffset;
}

void 
glfw_cursor_callback(
  GLFWwindow* window, 
  double xpos, 
  double ypos) {
  (void)window;
  IaMouse* mouse = &state.mouse;
  // Setting the position values 
  mouse->xpos = xpos;
  mouse->ypos = ypos;

  if(mouse->first_mouse_press) {
    mouse->xpos_last = xpos;
    mouse->ypos_last = ypos;
    mouse->first_mouse_press = false;
  }
  // Delta mouse positions 
  mouse->xpos_delta = mouse->xpos - mouse->xpos_last;
  mouse->ypos_delta = mouse->ypos - mouse->ypos_last;
  mouse->xpos_last = xpos;
  mouse->ypos_last = ypos;

  // Calling User defined callbacks
  for(uint32_t i = 0; i < state.cursor_pos_cb_count; i++) {
    state.cursor_pos_cbs[i](window, xpos, ypos);
  }

  // Populating the cursor event
  state.cp_ev.happened = true;
  state.cp_ev.x = xpos;
  state.cp_ev.y = ypos;
}

void 
glfw_char_callback(
  GLFWwindow* window, 
  uint32_t charcode) {
  (void)window;
  state.ch_ev.charcode = charcode;
  state.ch_ev.happened = true;
}


void 
update_input() {
  memcpy(
    state.mouse.buttons_last, 
    state.mouse.buttons_current, 
    sizeof(bool) * IA_MAX_MOUSE_BUTTONS);
}
void 
clear_events() {
  state.key_ev.happened = false;
  state.mb_ev.happened = false;
  state.cp_ev.happened = false;
  state.scr_ev.happened = false;
  state.ch_ev.happened = false;
  state.mouse.xpos_delta = 0;
  state.mouse.ypos_delta = 0;
}

void 
ia_init(GLFWwindow* window) {
  state.mouse.first_mouse_press = true;

  glfwSetKeyCallback(window, glfw_key_callback);
  glfwSetMouseButtonCallback(window, glfw_mouse_button_callback);
  glfwSetScrollCallback(window, glfw_scroll_callback);
  glfwSetCursorPosCallback(window, glfw_cursor_callback);
  glfwSetCharCallback(window, glfw_char_callback);
}

void 
ia_update() {
  update_input();
  clear_events();
}


void
lf_add_key_callback(void* cb) {
  state.key_cbs[state.key_cb_count++] = (IA_KEY_CALLBACK_t)cb;
}
void
lf_add_mouse_button_callback(void* cb) {
  state.mouse_button_cbs[state.mouse_button_cb_count++] = (IA_MOUSE_BUTTON_CALLBACK_t)cb;
}

void
lf_add_scroll_callback(void* cb) {
  state.scroll_cbs[state.scroll_cb_count++] = (IA_SCROLL_CALLBACK_t)cb;
}

void
lf_add_cursor_pos_callback(void* cb) {
  state.cursor_pos_cbs[state.cursor_pos_cb_count++] = (IA_CURSOR_CALLBACK_t)cb;
}

bool
ia_key_went_down(uint32_t key) {
  return ia_key_changed(key) && state.keyboard.keys[key];
}

bool
ia_key_is_down(uint32_t key) {
  return state.keyboard.keys[key];
}

bool ia_key_is_released(uint32_t key) {
  return ia_key_changed(key) && !state.keyboard.keys[key];
}

bool
ia_key_changed(uint32_t key) {
  bool ret = state.keyboard.keys_changed[key];
  state.keyboard.keys_changed[key] = false;
  return ret;
}

bool
ia_mouse_button_went_down(uint32_t button) {
  return ia_mouse_button_changed(button) && state.mouse.buttons_current[button];
}

bool
ia_mouse_button_is_down(uint32_t button) {
  return state.mouse.buttons_current[button];
}

bool
ia_mouse_button_is_released(uint32_t button) {
  return ia_mouse_button_changed(button) && !state.mouse.buttons_current[button];
}

bool
ia_mouse_button_changed(uint32_t button) {
  return state.mouse.buttons_current[button]  != state.mouse.buttons_last[button];
}

IaCursorPosEvent lf_mouse_move_event() {
  return state.cp_ev;
}

IaMouseButtonEvent lf_mouse_button_event() {
  return state.mb_ev;
}

IaScrollEvent lf_mouse_scroll_event() {
  return state.scr_ev;
}

IaKeyEvent lf_key_event() {
  return state.key_ev;
}

IaCharEvent lf_char_event() { 
  return state.ch_ev;
}
