#ifndef GAME_H_
#define GAME_H_

long time_msec;
int win_width, win_height;
int vp_width, vp_height;	/* viewport size differs from win size during VR eye rendering */
float win_aspect;
int fb_srgb;

enum {
	MOD_SHIFT	= 1,
	MOD_ALT		= 2,
	MOD_CTRL	= 4
};

/* special keys */
enum {
	KEY_DEL = 127,
	KEY_LEFT,
	KEY_RIGHT,
	KEY_UP,
	KEY_DOWN,
	KEY_PGUP,
	KEY_PGDOWN,
	KEY_HOME,
	KEY_END,
	KEY_F1,
	KEY_F2,
	KEY_F3,
	KEY_F4,
	KEY_F5,
	KEY_F6,
	KEY_F7,
	KEY_F8,
	KEY_F9,
	KEY_F10,
	KEY_F11,
	KEY_F12
};

/* XXX make sure these match with SDL_GameControllerButton */
enum {
	GPAD_A,
	GPAD_B,
	GPAD_X,
	GPAD_Y,
	GPAD_BACK,
	GPAD_GUIDE,
	GPAD_START,
	GPAD_LSTICK,
	GPAD_RSTICK,
	GPAD_L,
	GPAD_R,
	GPAD_UP,
	GPAD_DOWN,
	GPAD_LEFT,
	GPAD_RIGHT,
};

int game_init(int argc, char **argv);
void game_cleanup();

void game_display();
void game_reshape(int x, int y);

void game_keyboard(int key, int pressed);
void game_mouse_button(int bn, int pressed, int x, int y);
void game_mouse_motion(int x, int y);
void game_mouse_delta(int dx, int dy);
void game_mouse_wheel(int dir);

void game_gamepad_axis(int axis, float val);
void game_gamepad_button(int bn, int pressed);

/* the following functions are implemented by the backend (main.c) */
void game_quit();
void game_swap_buffers();
unsigned int game_get_modifiers();

void game_resize(int x, int y);
void game_fullscreen(int fs);
void game_toggle_fullscreen();
int game_is_fullscreen();

#endif	// GAME_H_
