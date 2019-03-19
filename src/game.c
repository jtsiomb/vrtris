#include <assert.h>
#ifdef BUILD_VR
#include <goatvr.h>
#endif
#include "cgmath/cgmath.h"
#include "opengl.h"
#include "game.h"
#include "screen.h"
#include "osd.h"
#include "opt.h"
#include "logger.h"
#include "audio.h"
#include "sndfx.h"

#define DEFSCR	"game"

static void calc_framerate(void);
static void print_framerate(void);

#ifdef BUILD_VR
static int should_swap;
#endif
static unsigned long framerate;


int game_init(int argc, char **argv)
{
	if(init_opengl() == -1) {
		return -1;
	}
	if(au_init() == -1) {
		return -1;
	}

	if(init_options(argc, argv, "vrtris.conf") == -1) {
		return -1;
	}

	if(init_screens() == -1) {
		return -1;
	}

	if(init_sndfx() == -1) {
		return -1;
	}

#ifdef BUILD_VR
	if(opt.flags & OPT_VR) {
		if(goatvr_init() == -1) {
			return -1;
		}
		goatvr_set_origin_mode(GOATVR_HEAD);

		goatvr_startvr();
		should_swap = goatvr_should_swap();
	}
#endif	/* BUILD_VR */

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	screen->start();
	return 0;
}

void game_cleanup()
{
#ifdef BUILD_VR
	if(opt.flags & OPT_VR) {
		goatvr_shutdown();
	}
#endif
	cleanup_screens();
	destroy_sndfx();
	au_destroy();
}

static void update(float dt)
{
	screen->update(dt);
}

void game_display(void)
{
	static long prev_msec;
	float dt = (float)(time_msec - prev_msec) / 1000.0f;
	prev_msec = time_msec;

#ifdef BUILD_VR
	if(opt.flags & OPT_VR) {
		int i;
		goatvr_draw_start();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		update(dt);

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

	} else
#endif	/* BUILD_VR */
	{
		/* non-VR mode */
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		update(dt);

		cgm_mperspective(proj_matrix, cgm_deg_to_rad(40.0), win_aspect, 0.5, 500.0);
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
#ifdef BUILD_VR
	goatvr_set_fb_size(x, y, 1.0f);
#endif

	reshape_screens(x, y);
}

void game_keyboard(int key, int pressed)
{
	unsigned int mod = game_get_modifiers();

	if(key < 256) {
		keystate[key] = pressed;
	}

	if(pressed) {
		switch(key) {
		case 27:
			game_quit();
			return;

		case KEY_F11:
			if(mod & MOD_ALT) {
				game_toggle_fullscreen();
				return;
			}
			break;

		case KEY_HOME:
#ifdef BUILD_VR
			if(goatvr_invr()) {
				goatvr_recenter();
				return;
			}
#endif
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
	joy_axis[axis] = val;
}

void game_gamepad_button(int bn, int pressed)
{
	if(pressed) {
		joy_bnstate |= (1 << bn);
	} else {
		joy_bnstate &= ~(1 << bn);
	}

#ifdef BUILD_VR
	if(goatvr_invr()) {
		if(bn == GPAD_BACK && pressed) {
			goatvr_recenter();
		}
	}
#endif
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
