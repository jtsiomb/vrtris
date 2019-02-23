#include <assert.h>
#include <goatvr.h>
#include <cgmath/cgmath.h>
#include "opengl.h"
#include "game.h"
#include "screen.h"
#include "osd.h"
#include "opt.h"

static void calc_framerate(void);
static void print_framerate(void);

static float view_matrix[16], proj_matrix[16];
static int should_swap;
static unsigned long framerate;

int game_init(int argc, char **argv)
{
	if(init_opengl() == -1) {
		return -1;
	}

	if(init_options(argc, argv, "vrtris.conf") == -1) {
		return -1;
	}

	if(init_screens() == -1) {
		return -1;
	}

	if(opt.flags & OPT_VR) {
		if(goatvr_init() == -1) {
			return -1;
		}
		goatvr_set_origin_mode(GOATVR_HEAD);

		goatvr_startvr();
		should_swap = goatvr_should_swap();
	}
	return 0;
}

void game_cleanup()
{
	if(opt.flags & OPT_VR) {
		goatvr_shutdown();
	}
	cleanup_screens();
}

static void update(float dt)
{
	int num_vr_sticks;

	if((num_vr_sticks = goatvr_num_sticks()) > 0) {
		float p[2];
		goatvr_stick_pos(0, p);
		/* TODO */
	}

	screen->update(dt);
}

void game_display(void)
{
	static long prev_msec;
	int i;
	float dt = (float)(time_msec - prev_msec) / 1000.0f;
	prev_msec = time_msec;

	update(dt);

	if(opt.flags & OPT_VR) {
		goatvr_draw_start();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		for(i=0; i<2; i++) {
			/* for each eye */
			goatvr_draw_eye(i);

			cgm_mcopy(proj_matrix, goatvr_projection_matrix(i, 0.5, 500.0));
			glMatrixMode(GL_PROJECTION);
			glLoadMatrixf(proj_matrix);

			cgm_mcopy(view_matrix, goatvr_view_matrix(i));
			glMatrixMode(GL_MODELVIEW);
			glLoadMatrixf(view_matrix);

			screen->draw();
			print_framerate();
			draw_osd();
		}

		goatvr_draw_done();

		if(should_swap) {
			game_swap_buffers();
		}

	} else {
		/* non-VR mode */
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		cgm_mperspective(proj_matrix, cgm_deg_to_rad(50.0), win_aspect, 0.5, 500.0);
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(proj_matrix);

		cgm_midentity(view_matrix);
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(view_matrix);

		screen->draw();
		print_framerate();
		draw_osd();

		game_swap_buffers();
	}

	assert(glGetError() == GL_NO_ERROR);

	calc_framerate();
}

void game_reshape(int x, int y)
{
	glViewport(0, 0, x, y);
	goatvr_set_fb_size(x, y, 1.0f);

	reshape_screens(x, y);
}

void game_keyboard(int key, int pressed)
{
	unsigned int mod = game_get_modifiers();

	if(pressed) {
		switch(key) {
		case 27:
			game_quit();
			return;

		case '\n':
		case '\r':
			if(mod & MOD_ALT) {
				game_toggle_fullscreen();
				return;
			}
			break;

		default:
			break;
		}
	}

	screen->keyboard(key, pressed);
}

void game_mouse_button(int bn, int pressed, int x, int y)
{
	screen->mouse(bn, pressed, x, y);
}

void game_mouse_motion(int x, int y)
{
	screen->motion(x, y);
}

void game_mouse_wheel(int dir)
{
	screen->wheel(dir);
}


void game_gamepad_axis(int axis, float val)
{
}

void game_gamepad_button(int bn, int pressed)
{
}

static void calc_framerate(void)
{
	static unsigned long nframes;
	static long prev_upd;

	long elapsed = time_msec - prev_upd;
	if(elapsed >= 1000) {
		framerate = nframes * 10000 / elapsed;
		nframes = 1;
		prev_upd = time_msec;
	} else {
		++nframes;
	}
}

static void print_framerate(void)
{
	print_text(9 * win_width / 10, 20, 1, 1, 0, "fps: %d.%d", framerate / 10, framerate % 10);
}
