#include <math.h>
#include <imago2.h>
#include "opengl.h"
#include "game.h"
#include "screen.h"
#include "cmesh.h"
#include "logger.h"

static int init(void);
static void cleanup(void);
static void start(void);
static void stop(void);
static void update(float dt);
static void draw(void);
static void reshape(int x, int y);
static void keyboard(int key, int pressed);
static void mouse(int bn, int pressed, int x, int y);
static void motion(int x, int y);
static void wheel(int dir);

struct game_screen help_screen = {
	"help",
	0,	/* opaque */
	0,	/* next */
	init,
	cleanup,
	start,
	stop,
	update,
	draw,
	reshape,
	keyboard,
	mouse,
	motion,
	wheel
};

static struct cmesh *keybmesh;
static int num_keyb_submeshes;
static int smidx;
static float zoom_lin;

static unsigned int tex_sph, tex_glyphs;


static int init(void)
{
	if(!(keybmesh = cmesh_alloc()) || cmesh_load(keybmesh, "data/keyb.obj") == -1) {
		error_log("failed to load keyboard layout mesh for the help screen\n");
		return -1;
	}
	num_keyb_submeshes = cmesh_submesh_count(keybmesh);

	if(!(tex_sph = img_gltexture_load("data/sphmap1.jpg"))) {
		error_log("failed to load data/sphmap1.jpg\n");
		return -1;
	}

	if(!(tex_glyphs = img_gltexture_load("data/keytex.png"))) {
		error_log("failed to load data/keytex.png\n");
		return -1;
	}
	return 0;
}

static void cleanup(void)
{
	cmesh_free(keybmesh);
	glDeleteTextures(1, &tex_sph);
	glDeleteTextures(1, &tex_glyphs);
}

static void start(void)
{
	smidx = -1;
	zoom_lin = 0;
}

static void stop(void)
{
}

#define ZOOM_SPEED	20.0f
#define MAX_ZOOM	8.0f

static void update(float dt)
{
	if(screen->next) {
		screen->next->update(dt);
	}

	zoom_lin += dt * ZOOM_SPEED;
}

static void draw(void)
{
	static const float lpos[] = {-1, 6, 4, 1};
	int i;
	float t = (float)time_msec / 1000.0f;

	float anim = cgm_smoothstep(0.0f, MAX_ZOOM, zoom_lin);

	if(screen->next) {
		glPushMatrix();
		glTranslatef(0, 0, -anim * MAX_ZOOM);
		screen->next->draw();
		glPopMatrix();
	}

	/* darken the backdrop */
	glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_LIGHTING);
	glDepthMask(0);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	glBegin(GL_QUADS);
	glColor4f(0, 0, 0, 0.5);
	glVertex2f(-1, -1);
	glVertex2f(1, -1);
	glVertex2f(1, 1);
	glVertex2f(-1, 1);
	glEnd();

	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glDepthMask(1);


	/* draw the controls */
	glTranslatef((anim - 1.0) * 10.0f, 0, -23);
	glRotatef(40, 1, 0, 0);

	glLightfv(GL_LIGHT0, GL_POSITION, lpos);

	glPushMatrix();
	glRotatef(cos(t * 0.5) * 2.0f, 1, 0, 0);
	glRotatef(sin(t * 0.4) * 8.0f, 0, 1, 0);
	glScalef(0.5, 0.5, 0.5);

	glEnable(GL_NORMALIZE);
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);

	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);

	glBindTexture(GL_TEXTURE_2D, tex_sph);

	glColor3f(1, 1, 1);

	if(smidx >= 0) {
		cmesh_draw_submesh(keybmesh, smidx);
	} else {
		cmesh_draw(keybmesh);
	}

	glBindTexture(GL_TEXTURE_2D, tex_glyphs);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glDepthFunc(GL_LEQUAL);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);

	glColor3f(1, 0.5, 0.2);
	for(i=0; i<num_keyb_submeshes; i++) {
		if(i >= num_keyb_submeshes - 1) {
			glColor3f(0.5, 0.5, 0.5);
		}
		cmesh_draw_submesh(keybmesh, i);
	}
	glPopAttrib();
	glPopMatrix();
}

static void reshape(int x, int y)
{
}

static void keyboard(int key, int pressed)
{
	if(!pressed) return;

	switch(key) {
	case KEY_F1:
	case 27:
		pop_screen();
		break;

	case ' ':
		smidx = (smidx + 1) % cmesh_submesh_count(keybmesh);
		printf("drawing submesh: %d\n", smidx);
		break;

	default:
		break;
	}
}

static void mouse(int bn, int pressed, int x, int y)
{
}

static void motion(int x, int y)
{
}

static void wheel(int dir)
{
}
