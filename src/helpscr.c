#include <math.h>
#include <imago2.h>
#include "opengl.h"
#include "game.h"
#include "screen.h"
#include "cmesh.h"
#include "logger.h"


#define ZOOM_SPEED	17.0f
#define MAX_ZOOM	8.0f
#define OUT_DUR		0.6f

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

struct keymesh {
	struct cmesh *mesh;
	int active;
	const char *name;
};

struct cmesh *kbmesh;	/* whole mesh */
static struct keymesh kmesh[] = {
	{0, 0, "unusedkeys"},
	{0, 1, "key_up"},
	{0, 1, "key_down"},
	{0, 1, "key_left"},
	{0, 1, "key_right"},
	{0, 1, "key_p"},
	{0, 1, "key_ret"},
	{0, 1, "key_bs"}
};
static const int num_kmesh = sizeof kmesh / sizeof *kmesh;


static float zoom_lin;
static float stopping;

static unsigned int tex_sph, tex_glyphs;


static int init(void)
{
	int i, num_subs, idx;

	if(!(kbmesh = cmesh_alloc()) || cmesh_load(kbmesh, "data/keyb.obj") == -1) {
		error_log("failed to load keyboard layout mesh for the help screen\n");
		return -1;
	}
	if((num_subs = cmesh_submesh_count(kbmesh)) < num_kmesh) {
		error_log("keyb.obj doesn't have all the submeshes expected\n");
		return -1;
	}

	for(i=0; i<num_kmesh; i++) {
		if((idx = cmesh_find_submesh(kbmesh, kmesh[i].name)) == -1) {
			error_log("keyb.obj doesn't have submesh \"%s\"\n", kmesh[i].name);
			return -1;
		}
		if(!(kmesh[i].mesh = cmesh_alloc()) || cmesh_clone_submesh(kmesh[i].mesh, kbmesh, idx) == -1) {
			error_log("failed to create submesh for keyb.obj:%s\n", kmesh[i].name);
			return -1;
		}
	}

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
	int i;
	cmesh_free(kbmesh);
	for(i=0; i<num_kmesh; i++) {
		cmesh_free(kmesh[i].mesh);
	}
	glDeleteTextures(1, &tex_sph);
	glDeleteTextures(1, &tex_glyphs);
}

static void start(void)
{
	zoom_lin = 0;
	stopping = 0;
}

static void stop(void)
{
}

static void update(float dt)
{
	if(screen->next) {
		screen->next->update(dt);
	}

	if(stopping > 0.0f) {
		stopping -= dt;
		if(stopping <= 0.0f) {
			pop_screen();
		}
		zoom_lin = stopping / OUT_DUR * MAX_ZOOM;
	} else {
		zoom_lin += dt * ZOOM_SPEED;
	}
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
	glTranslatef((anim - 1.0) * 16.0f, 0, -23);
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

	cmesh_draw(kbmesh);

	glBindTexture(GL_TEXTURE_2D, tex_glyphs);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glDepthFunc(GL_LEQUAL);
	glDisable(GL_TEXTURE_GEN_S);
	glDisable(GL_TEXTURE_GEN_T);

	for(i=0; i<num_kmesh; i++) {
		if(kmesh[i].active) {
			glColor3f(1, 0.5, 0.2);
		} else {
			glColor3f(0.5, 0.5, 0.5);
		}

		cmesh_draw(kmesh[i].mesh);
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
		stopping = OUT_DUR;
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
