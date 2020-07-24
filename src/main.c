#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <SDL2/SDL.h>
#include "game.h"
#include "opt.h"

static void process_event(SDL_Event *ev);
static void proc_modkeys();
static int translate_keysym(SDL_Keycode sym);

static SDL_Window *win;
static SDL_GLContext ctx;
static int fullscreen;
static int quit;

static unsigned long start_time;
static unsigned int modkeys;

SDL_GameController *gamepad;

static int scale_factor = 1;

#define DEFPOS	SDL_WINDOWPOS_UNDEFINED

int main(int argc, char **argv)
{
	int i;
	unsigned int sdlflags;

	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0) {
		fprintf(stderr, "failed to initialize SDL\n");
		return 1;
	}

	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 8);
	SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 1);
#ifndef NDEBUG
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

	sdlflags = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;

	if(!(win = SDL_CreateWindow("vrtris", DEFPOS, DEFPOS, 1024, 768, sdlflags))) {
		/* try again without sRGB capability */
		SDL_GL_SetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, 0);
		if(!(win = SDL_CreateWindow("vrtris", DEFPOS, DEFPOS, 1024, 768, sdlflags))) {
			fprintf(stderr, "failed to create window\n");
			SDL_Quit();
			return 1;
		}
		fprintf(stderr, "failed to get an sRGB framebuffer\n");
	}
	SDL_GL_GetAttribute(SDL_GL_FRAMEBUFFER_SRGB_CAPABLE, &fb_srgb);

	if(!(ctx = SDL_GL_CreateContext(win))) {
		fprintf(stderr, "failed to create OpenGL context\n");
		SDL_Quit();
		return 1;
	}
	SDL_GL_GetDrawableSize(win, &win_width, &win_height);
	win_aspect = (float)win_width / (float)win_height;

	printf("detected %d joysticks\n", SDL_NumJoysticks());
	for(i=0; i<SDL_NumJoysticks(); i++) {
		if(SDL_IsGameController(i)) {
			if(!(gamepad = SDL_GameControllerOpen(i))) {
				fprintf(stderr, "failed to open game controller %i: %s\n", i, SDL_GetError());
				continue;
			}
			printf("Using gamepad: %s\n", SDL_GameControllerNameForIndex(i));
		}
	}

	if(game_init(argc, argv) == -1) {
		SDL_Quit();
		return 1;
	}

	game_reshape(win_width, win_height);
	SDL_RaiseWindow(win);

	SDL_GL_SetSwapInterval(opt.flags & OPT_VSYNC ? 1 : 0);

	start_time = SDL_GetTicks();

	while(!quit) {
		SDL_Event ev;

		time_msec = get_msec();
		while(SDL_PollEvent(&ev)) {
			process_event(&ev);
			if(quit) goto break_evloop;
		}

		game_display();
	}
break_evloop:

	game_cleanup();
	SDL_Quit();
	return 0;
}

void game_swap_buffers()
{
	SDL_GL_SwapWindow(win);
}

void game_quit()
{
	quit = 1;
}

unsigned int game_get_modifiers()
{
	return modkeys;
}

void game_resize(int x, int y)
{
	SDL_SetWindowSize(win, x, y);
}

void game_fullscreen(int fs)
{
	SDL_SetWindowFullscreen(win, fs ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
	fullscreen = fs;
}

void game_toggle_fullscreen()
{
	game_fullscreen(!fullscreen);
}

int game_is_fullscreen()
{
	return fullscreen;
}

unsigned long get_msec(void)
{
	return SDL_GetTicks() - start_time;
}

static void process_event(SDL_Event *ev)
{
	int key;

	switch(ev->type) {
	case SDL_QUIT:
		quit = 1;
		break;

	case SDL_KEYDOWN:
	case SDL_KEYUP:
		proc_modkeys();
		if((key = translate_keysym(ev->key.keysym.sym)) != -1) {
			game_keyboard(key, ev->key.state == SDL_PRESSED);
		}
		break;

	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
		proc_modkeys();
		game_mouse_button(ev->button.button - SDL_BUTTON_LEFT, ev->button.state == SDL_PRESSED,
				ev->button.x * scale_factor, ev->button.y * scale_factor);
		break;

	case SDL_MOUSEMOTION:
		game_mouse_motion(ev->motion.x * scale_factor, ev->motion.y * scale_factor);
		break;

	case SDL_MOUSEWHEEL:
		game_mouse_wheel(ev->wheel.y);
		break;

	case SDL_WINDOWEVENT:
		if(ev->window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
			SDL_GL_GetDrawableSize(win, &win_width, &win_height);
			win_aspect = (float)win_width / (float)win_height;
			scale_factor = win_width / ev->window.data1;
			game_reshape(win_width, win_height);
		}
		break;

	case SDL_CONTROLLERAXISMOTION:
		game_gamepad_axis(ev->caxis.axis, ev->caxis.value / 32768.0f);
		break;

	case SDL_CONTROLLERBUTTONDOWN:
	case SDL_CONTROLLERBUTTONUP:
		game_gamepad_button(ev->cbutton.button, ev->type == SDL_CONTROLLERBUTTONDOWN);
		break;
	}
}

static void proc_modkeys()
{
	modkeys = 0;
	SDL_Keymod sdlmod = SDL_GetModState();
	if(sdlmod & KMOD_SHIFT) {
		modkeys |= MOD_SHIFT;
	}
	if(sdlmod & KMOD_ALT) {
		modkeys |= MOD_ALT;
	}
	if(sdlmod & KMOD_CTRL) {
		modkeys |= MOD_CTRL;
	}
}

static int translate_keysym(SDL_Keycode sym)
{
	switch(sym) {
	case SDLK_RETURN:
		return '\n';
	case SDLK_DELETE:
		return KEY_DEL;
	case SDLK_LEFT:
		return KEY_LEFT;
	case SDLK_RIGHT:
		return KEY_RIGHT;
	case SDLK_UP:
		return KEY_UP;
	case SDLK_DOWN:
		return KEY_DOWN;
	case SDLK_PAGEUP:
		return KEY_PGUP;
	case SDLK_PAGEDOWN:
		return KEY_PGDOWN;
	case SDLK_HOME:
		return KEY_HOME;
	case SDLK_END:
		return KEY_END;
	default:
		break;
	}

	if(sym < 127) {
		return sym;
	}
	if(sym >= SDLK_F1 && sym <= SDLK_F12) {
		return KEY_F1 + sym - SDLK_F1;
	}
	return -1;
}
