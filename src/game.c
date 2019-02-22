#include <assert.h>
#include "opengl.h"
#include "game.h"
#include "opt.h"

int game_init(int argc, char **argv)
{
	if(init_opengl() == -1) {
		return -1;
	}

	if(init_options(argc, argv, "vrtris.conf") == -1) {
		return -1;
	}

	return 0;
}

void game_cleanup()
{
}

void game_display()
{
	glClear(GL_COLOR_BUFFER_BIT);

	game_swap_buffers();
	assert(glGetError() == GL_NO_ERROR);
}

void game_reshape(int x, int y)
{
}

void game_keyboard(int key, int pressed)
{
}

void game_mouse_button(int bn, int pressed, int x, int y)
{
}

void game_mouse_motion(int x, int y)
{
}

void game_mouse_delta(int dx, int dy)
{
}

void game_mouse_wheel(int dir)
{
}


void game_gamepad_axis(int axis, float val)
{
}

void game_gamepad_button(int bn, int pressed)
{
}
