/*
vrtris - tetris in virtual reality
Copyright (C) 2019-2020  John Tsiombikas <nuclear@member.fsf.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#ifndef GAME_H_
#define GAME_H_

extern long time_msec;
extern int win_width, win_height;
extern int vp_width, vp_height;	/* viewport size differs from win size during VR eye rendering */
extern float win_aspect;
extern int fb_srgb;

extern unsigned char keystate[256];
extern float joy_axis[3];
extern unsigned int joy_bnstate;

extern float view_matrix[16], proj_matrix[16];

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
	GPAD_RIGHT
};

/* XXX make sure these match with SDL_GameControllerAxis */
enum {
	GPAD_LSTICK_X,
	GPAD_LSTICK_Y,
	GPAD_RSTICK_X,
	GPAD_RSTICK_Y,
	GPAD_LTRIG,
	GPAD_RTRIG
};

int game_init(int argc, char **argv);
void game_cleanup(void);

void game_display(void);
void game_reshape(int x, int y);

void game_keyboard(int key, int pressed);
void game_mouse_button(int bn, int pressed, int x, int y);
void game_mouse_motion(int x, int y);
void game_mouse_wheel(int dir);

void game_gamepad_axis(int axis, float val);
void game_gamepad_button(int bn, int pressed);

/* the following functions are implemented by the backend (main.c) */
void game_quit(void);
void game_swap_buffers(void);
unsigned int game_get_modifiers(void);

void game_resize(int x, int y);
void game_fullscreen(int fs);
void game_toggle_fullscreen(void);
int game_is_fullscreen(void);

int game_num_joy_axes(void);
int game_num_joy_buttons(void);

unsigned long get_msec(void);

#endif	// GAME_H_
