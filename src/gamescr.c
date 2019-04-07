#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <assert.h>
#include <imago2.h>
#ifdef BUILD_VR
#include <goatvr.h>
#endif
#include <drawtext.h>
#include "opengl.h"
#include "game.h"
#include "screen.h"
#include "cmesh.h"
#include "blocks.h"
#include "logger.h"
#include "gameinp.h"
#include "color.h"
#include "sndfx.h"
#include "scoredb.h"

#define FONTSZ	54

int init_starfield(void);
void update_starfield(float dt);
void draw_starfield(void);

static int init(void);
static void cleanup(void);
static void start(void);
static void stop(void);
static void update(float dt);
static void draw(void);
static void draw_block(int block, const int *pos, int rot, float sat, float alpha);
static void drawpf(void);
static void game_input(unsigned int inp);
static void reshape(int x, int y);
static void keyboard(int key, int pressed);
static void mouse(int bn, int pressed, int x, int y);
static void motion(int x, int y);
static void wheel(int dir);

static void update_cur_block(void);
static void addscore(int nlines);
static int spawn(void);
static int collision(int block, const int *pos);
static void stick(int block, const int *pos);
static void erase_completed(void);

struct game_screen game_screen = {
	"game",
	1,	/* opaque */
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

static struct cmesh *blkmesh, *wellmesh;
static unsigned int tex_well;

static struct dtx_font *scorefont;

static float cam_theta, cam_phi, cam_dist;
static float cam_height;
static unsigned int bnstate;
static int prev_mx, prev_my;

static long tick_interval;

/* dimensions of the playfield */
#define PF_ROWS		18
#define PF_COLS		10

#define PF_FULL		0x100
#define PF_VIS		0x200
#define PF_VIS_SHIFT	9

static unsigned int pfield[PF_ROWS * PF_COLS];

static int pos[2], next_pos[2];
static int cur_block, next_block, prev_block;
static int cur_rot, prev_rot;
static int complines[4];
static int num_complines;
static int gameover;
static int pause;
static int score, level, lines;
static int just_spawned;

#ifdef BUILD_VR
static int vrbn_a = 0, vrbn_b = 1, vrbn_x = 4;
static float vrscale = 40.0f;
#endif

#define NUM_LEVELS	21
static const long level_speed[NUM_LEVELS] = {
	887, 820, 753, 686, 619, 552, 469, 368, 285, 184,
	167, 151, 134, 117, 107, 98, 88, 79, 69, 60, 50
};

static const float blkcolor[][4] = {
	{1.0, 0.65, 0.0, 1},
	{0.16, 1.0, 0.4, 1},
	{0.65, 0.65, 1.0, 1},
	{1.0, 0.9, 0.1, 1},
	{0.0, 1.0, 1.0, 1},
	{1.0, 0.5, 1.0, 1},
	{1.0, 0.35, 0.2, 1},
	{0.5, 0.5, 0.5, 1}
};

#define GAMEOVER_FILL_RATE	50


static int init(void)
{
	if(!(scorefont = dtx_open_font("data/score.font", 0))) {
		error_log("failed to open score font\n");
		return -1;
	}
	dtx_prepare_range(scorefont, FONTSZ, 32, 127);

	if(init_starfield() == -1) {
		return -1;
	}

	if(!(blkmesh = cmesh_alloc()) || cmesh_load(blkmesh, "data/noisecube.obj") == -1) {
		error_log("failed to load block mesh\n");
		return -1;
	}

	if(!(wellmesh = cmesh_alloc()) || cmesh_load(wellmesh, "data/well.obj") == -1) {
		error_log("failed to load well mesh\n");
		return -1;
	}

	if(!(tex_well = img_gltexture_load("data/grid.png"))) {
		error_log("failed to load well texture\n");
		return -1;
	}

	return 0;
}

static void cleanup(void)
{
	cmesh_free(blkmesh);
	cmesh_free(wellmesh);
	glDeleteTextures(1, &tex_well);
	dtx_close_font(scorefont);
}

static void start(void)
{
	srand(time(0));

	glClearColor(0.12, 0.12, 0.18, 1);

	pause = 0;
	gameover = 0;
	num_complines = 0;
	score = level = lines = 0;
	tick_interval = level_speed[0];
	cur_block = -1;
	prev_block = 0;
	next_block = rand() % NUM_BLOCKS;

	memset(pfield, 0, PF_COLS * PF_ROWS * sizeof *pfield);

	ginp_repeat(350, 75, GINP_LEFT | GINP_RIGHT | GINP_DOWN);

	cam_theta = 0;
	cam_phi = 0;
	cam_dist = 30;
	cam_height = 0;

#ifdef BUILD_VR
	if(goatvr_invr()) {
		int bn = goatvr_lookup_button("A");
		if(bn >= 0) vrbn_a = bn;
		bn = goatvr_lookup_button("B");
		if(bn >= 0) vrbn_b = bn;
		bn = goatvr_lookup_button("X");
		if(bn >= 0) vrbn_x = bn;

		/* switch to VR-optimized camera parameters */
		cam_theta = 0;
		cam_phi = -2.5;
		cam_dist = 20;
		cam_height = 3.5;
		vrscale = 40.0f;

		goatvr_set_units_scale(vrscale);
	}

	goatvr_recenter();
#endif
}

static void stop(void)
{
	if(score) {
		save_score(score, lines, level);
	}
#ifdef BUILD_VR
	goatvr_set_units_scale(1.0f);
#endif
}

#define JTHRES	0.6

#define CHECK_BUTTON(idx, gbn) \
	if(joy_bnstate & (1 << idx)) { \
		ginp_bnstate |= gbn; \
	}

static void update_input(float dtsec)
{
	unsigned int bnmask;
#ifdef BUILD_VR
	int num_vr_sticks;
	float orig_joy_axis[3];
	unsigned int orig_joy_bnstate = UINT_MAX;

	if(goatvr_invr() && (num_vr_sticks = goatvr_num_sticks()) > 0) {
		float p[2];

		memcpy(orig_joy_axis, joy_axis, sizeof orig_joy_axis);
		orig_joy_bnstate = joy_bnstate;

		goatvr_stick_pos(0, p);
		p[1] *= 0.65;	/* drops harder to trigger accidentally */

		if(fabs(p[0]) > fabs(joy_axis[GPAD_LSTICK_X])) {
			joy_axis[GPAD_LSTICK_X] = p[0];
		}
		if(fabs(p[1]) > fabs(joy_axis[GPAD_LSTICK_Y])) {
			joy_axis[GPAD_LSTICK_Y] = -p[1];
		}

		if(goatvr_button_state(vrbn_a)) {
			joy_bnstate |= 1 << GPAD_A;
		}
		if(goatvr_button_state(vrbn_b)) {
			joy_bnstate |= 1 << GPAD_B;
		}
		if(goatvr_button_state(vrbn_x)) {
			joy_bnstate |= 1 << GPAD_START;
		}
		if(goatvr_action(0, GOATVR_ACTION_TRIGGER) || goatvr_action(1, GOATVR_ACTION_TRIGGER)) {
			joy_bnstate |= 1 << GPAD_UP;
		}
	}
#endif	/* BUILD_VR */

	ginp_bnstate = 0;

	/* update repeating keys */
	if(keystate['a'] || keystate[KEY_LEFT]) {
		ginp_bnstate |= GINP_LEFT;
	}
	if(keystate['d'] || keystate[KEY_RIGHT]) {
		ginp_bnstate |= GINP_RIGHT;
	}
	if(keystate['s'] || keystate[KEY_DOWN]) {
		ginp_bnstate |= GINP_DOWN;
	}


	/* joystick axis */
	if(joy_axis[GPAD_LSTICK_X] >= JTHRES) {
		ginp_bnstate |= GINP_RIGHT;
	} else if(joy_axis[GPAD_LSTICK_X] <= -JTHRES) {
		ginp_bnstate |= GINP_LEFT;
	}

	if(joy_axis[GPAD_LSTICK_Y] >= JTHRES) {
		ginp_bnstate |= GINP_DOWN;
	} else if(joy_axis[GPAD_LSTICK_Y] <= -JTHRES) {
		ginp_bnstate |= GINP_UP;
	}

	CHECK_BUTTON(GPAD_LEFT, GINP_LEFT);
	CHECK_BUTTON(GPAD_RIGHT, GINP_RIGHT);
	CHECK_BUTTON(GPAD_UP, GINP_UP);
	CHECK_BUTTON(GPAD_DOWN, GINP_DOWN);
	CHECK_BUTTON(GPAD_A, GINP_ROTATE);
	CHECK_BUTTON(GPAD_B, GINP_ROTATE);
	CHECK_BUTTON(GPAD_START, GINP_PAUSE);

	update_ginp();

	bnmask = 1;
	while(bnmask < GINP_ALL) {
		if(GINP_PRESS(bnmask)) {
			game_input(bnmask);
		}
		bnmask <<= 1;
	}

#ifdef BUILD_VR
	if(orig_joy_bnstate != UINT_MAX) {
		memcpy(joy_axis, orig_joy_axis, sizeof joy_axis);
		joy_bnstate = orig_joy_bnstate;
	}
#endif
}

static void update(float dtsec)
{
	static long prev_tick;
	long dt;

	update_input(dtsec);

	if(pause || screen != &game_screen) {
		prev_tick = time_msec;
		return;
	}
	dt = time_msec - prev_tick;

	update_starfield(dtsec);

	if(gameover) {
		int i, row = PF_ROWS - gameover;
		unsigned int *ptr;

		if(dt < GAMEOVER_FILL_RATE) {
			return;
		}

		if(row >= 0) {
			ptr = pfield + row * PF_COLS;
			for(i=0; i<PF_COLS; i++) {
				*ptr++ = PF_VIS | PF_FULL | 7;
			}

			gameover++;
			prev_tick = time_msec;
		}
		return;
	}

	if(num_complines) {
		/* lines where completed, we're in blinking mode */
		int i, j, blink = dt >> 8;

		if(blink > 6) {
			erase_completed();
			num_complines = 0;
			return;
		}

		for(i=0; i<num_complines; i++) {
			unsigned int *ptr = pfield + complines[i] * PF_COLS;
			for(j=0; j<PF_COLS; j++) {
				*ptr = (*ptr & ~PF_VIS) | ((blink & 1) << PF_VIS_SHIFT);
				ptr++;
			}
		}
		return;
	}

	/* fall */
	while(dt >= tick_interval) {
		if(cur_block >= 0) {
			just_spawned = 0;
			next_pos[0] = pos[0] + 1;
			if(collision(cur_block, next_pos)) {
				next_pos[0] = pos[0];
				stick(cur_block, next_pos);
				return;
			}
		} else {
			/* respawn */
			if(spawn() == -1) {
				gameover = 1;
				return;
			}
		}

		dt -= tick_interval;
		prev_tick = time_msec;
	}

	update_cur_block();
}

static void draw(void)
{
	static const int nextblk_pos[] = {0, 0};
	static const float lpos[] = {-1, 1, 6, 1};
	float t;

	glTranslatef(0, 0, -cam_dist);
	glRotatef(cam_phi, 1, 0, 0);
	glRotatef(cam_theta, 0, 1, 0);
	glTranslatef(0, -cam_height, 0);

	glLightfv(GL_LIGHT0, GL_POSITION, lpos);

	draw_starfield();

	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_COLOR_MATERIAL);
	glBindTexture(GL_TEXTURE_2D, tex_well);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glColor3f(1, 1, 1);
	cmesh_draw(wellmesh);
	glPopAttrib();

	/* center playfield */
	glPushMatrix();
	glTranslatef(-PF_COLS / 2 + 0.5, PF_ROWS / 2 - 0.5, 0);

	drawpf();
	if(cur_block >= 0) {
		draw_block(cur_block, pos, cur_rot, 1.0f, 1.0f);
	}
	glPopMatrix();

	glPushAttrib(GL_ENABLE_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glPushMatrix();
	t = (float)time_msec / 1000.0f;
	glTranslatef(-PF_COLS / 2 + 0.5 + PF_COLS + 3, PF_ROWS / 2 - 0.5, 0);
	glTranslatef(1.5, -1, 0);
	glRotatef(cos(t) * 8.0f, 1, 0, 0);
	glRotatef(sin(t * 1.2f) * 10.0f, 0, 1, 0);
	glTranslatef(-1.5, 1, 0);
	draw_block(next_block, nextblk_pos, 0, 0.4f, 0.75f);
	glPopMatrix();
	glPopAttrib();

	dtx_use_font(scorefont, FONTSZ);
	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);

	glPushMatrix();
	glTranslatef(-11, 6, 0);
	glScalef(0.013, 0.013, 0.013);

	glColor3f(1, 1, 1);
	dtx_string("Score");
	glTranslatef(0, -dtx_line_height() * 1.5, 0);
	glPushMatrix();
	glScalef(1.5, 1.5, 1.5);
	dtx_printf("%d", score);
	glPopMatrix();

	glTranslatef(0, -dtx_line_height() * 2, 0);
	dtx_string("Level");
	glTranslatef(0, -dtx_line_height() * 1.5, 0);
	glPushMatrix();
	glScalef(1.5, 1.5, 1.5);
	dtx_printf("%d", level);
	glPopMatrix();

	glTranslatef(0, -dtx_line_height() * 2, 0);
	dtx_string("Lines");
	glTranslatef(0, -dtx_line_height() * 1.5, 0);
	glPushMatrix();
	glScalef(1.5, 1.5, 1.5);
	dtx_printf("%d", lines);
	glPopMatrix();

	glPopMatrix();
	glPopAttrib();
}

static const float blkspec[] = {0.85, 0.85, 0.85, 1};

static void draw_block(int block, const int *pos, int rot, float sat, float alpha)
{
	int i;
	unsigned char *p = blocks[block][rot];
	float col[4], hsv[3];

	rgb_to_hsv(blkcolor[block][0], blkcolor[block][1], blkcolor[block][2],
			hsv, hsv + 1, hsv + 2);
	hsv_to_rgb(hsv[0], hsv[1] * sat, hsv[2], col, col + 1, col + 2);
	col[3] = alpha;

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, col);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, blkspec);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 50.0f);

	for(i=0; i<4; i++) {
		int x = pos[1] + BLKX(*p);
		int y = pos[0] + BLKY(*p);
		p++;

		glPushMatrix();
		glTranslatef(x, -y, 0);
		cmesh_draw(blkmesh);
		glPopMatrix();
	}
}

static void drawpf(void)
{
	int i, j;
	unsigned int *sptr = pfield;

	for(i=0; i<PF_ROWS; i++) {
		for(j=0; j<PF_COLS; j++) {
			unsigned int val = *sptr++;

			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, blkcolor[val & 7]);
			glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, blkspec);
			glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 50.0f);

			if((val & (PF_FULL | PF_VIS)) == (PF_FULL | PF_VIS)) {
				glPushMatrix();
				glTranslatef(j, -i, 0);
				cmesh_draw(blkmesh);
				glPopMatrix();
			}
		}
	}
}

static void game_input(unsigned int inp)
{
	switch(inp) {
	case GINP_LEFT:
		if(!pause) {
			next_pos[1] = pos[1] - 1;
			if(collision(cur_block, next_pos)) {
				next_pos[1] = pos[1];
			} else {
				snd_shift();
			}
		}
		break;

	case GINP_RIGHT:
		if(!pause) {
			next_pos[1] = pos[1] + 1;
			if(collision(cur_block, next_pos)) {
				next_pos[1] = pos[1];
			} else {
				snd_shift();
			}
		}
		break;

	case GINP_DOWN:
		/* ignore drops until the first update after a spawn */
		if(cur_block >= 0 && !just_spawned && !pause) {
			next_pos[0] = pos[0] + 1;
			if(collision(cur_block, next_pos)) {
				next_pos[0] = pos[0];
				update_cur_block();
				stick(cur_block, next_pos);	/* stick immediately */
			}
		}
		break;

	case GINP_ROTATE:
		if(!pause) {
			prev_rot = cur_rot;
			cur_rot = (cur_rot + 1) & 3;
			if(collision(cur_block, next_pos)) {
				cur_rot = prev_rot;
			} else {
				snd_rot();
			}
		}
		break;

	case GINP_UP:
		if(!pause && cur_block >= 0) {
			next_pos[0] = pos[0] + 1;
			while(!collision(cur_block, next_pos)) {
				next_pos[0]++;
			}
			next_pos[0]--;
			update_cur_block();
			stick(cur_block, next_pos);	/* stick immediately */
		}
		break;

	case GINP_PAUSE:
		if(gameover) {
			if(score) {
				save_score(score, lines, level);
			}
			/* TODO: pop screen */
			start();
		} else {
			pause ^= 1;
		}
		break;

	default:
		break;
	}
}

static void reshape(int x, int y)
{
}

/* in the keyboard event handler we ignore all the repeating keys
 * which are handled in game_input instead, called from update_input
 */
static void keyboard(int key, int pressed)
{
	/*char *name = 0;*/

	if(!pressed) return;

	switch(key) {
	case 'w':
	case KEY_UP:
	case ' ':
		game_input(GINP_ROTATE);
		break;


	case '\n':
	case '\t':
		game_input(GINP_UP);
		break;

	case 'p':
		game_input(GINP_PAUSE);
		break;

	case '\b':
		if(score) {
			save_score(score, lines, level);
		}
		/* TODO: pop screen */
		break;

	case KEY_F1:
		push_screen(find_screen("help"));
		break;

	default:
		break;
	}
}

static void mouse(int bn, int pressed, int x, int y)
{
	if(pressed) {
		bnstate |= 1 << bn;
	} else {
		bnstate &= ~(1 << bn);
	}
	prev_mx = x;
	prev_my = y;
}

static void motion(int x, int y)
{
	float dx = x - prev_mx;
	float dy = y - prev_my;
	prev_mx = x;
	prev_my = y;

	if(bnstate & 1) {
		cam_theta += dx * 0.5;
		cam_phi += dy * 0.5;

		if(cam_phi < -90) cam_phi = -90;
		if(cam_phi > 90) cam_phi = 90;
	}
	if(bnstate & 2) {
		cam_height += dy * 0.1;
	}
	if(bnstate & 4) {
		cam_dist += dy * 0.1;
		if(cam_dist < 0) cam_dist = 0;
	}

#ifdef DBG_PRINT_VIEW
	if(bnstate) {
		debug_log("Camera params\n");
		debug_log("  theta: %g  phi: %g  dist: %g  height: %g\n", cam_theta,
				cam_phi, cam_dist, cam_height);
	}
#endif
}

static void wheel(int dir)
{
	/* debug code, used to figure out the best scales */
	/*
	vrscale += dir * 0.01;
	if(vrscale < 0.01) vrscale = 0.01;
	goatvr_set_units_scale(vrscale);
	debug_log("VR scale: %g\n", vrscale);
	*/
}

static void update_cur_block(void)
{
	if(cur_block < 0) return;

	memcpy(pos, next_pos, sizeof pos);
	prev_rot = cur_rot;
}

static void addscore(int nlines)
{
	static const int stab[] = {40, 100, 300, 1200};	/* bonus per line completed */

	assert(nlines < 5);

	score += stab[nlines - 1] * (level + 1);
	lines += nlines;

	level = lines / 10;
	if(level > NUM_LEVELS - 1) level = NUM_LEVELS - 1;

	tick_interval = level_speed[level];
}

static int spawn(void)
{
	int r, tries = 2;

	do {
		r = rand() % NUM_BLOCKS;
	} while(tries-- > 0 && (r | prev_block | next_block) == prev_block);

	cur_block = next_block;
	next_block = r;

	prev_rot = cur_rot = 0;
	pos[0] = block_spawnpos[cur_block][0];
	next_pos[0] = pos[0] + 1;
	pos[1] = next_pos[1] = PF_COLS / 2 + block_spawnpos[cur_block][1];

	if(collision(cur_block, next_pos)) {
		return -1;
	}

	just_spawned = 1;
	return 0;
}

static int collision(int block, const int *pos)
{
	int i;
	unsigned char *p = blocks[block][cur_rot];

	for(i=0; i<4; i++) {
		int x = pos[1] + BLKX(*p);
		int y = pos[0] + BLKY(*p);
		p++;

		if(y < 0) continue;

		if(x < 0 || x >= PF_COLS || y >= PF_ROWS) return 1;
		if(pfield[y * PF_COLS + x] & PF_FULL) return 1;
	}

	return 0;
}

static void stick(int block, const int *pos)
{
	int i, j, nblank;
	unsigned int *pfline;
	unsigned char *p = blocks[block][cur_rot];

	num_complines = 0;
	prev_block = cur_block;	/* used by the spawn routine */
	cur_block = -1;

	for(i=0; i<4; i++) {
		int x = pos[1] + BLKX(*p);
		int y = pos[0] + BLKY(*p);
		p++;

		pfline = pfield + y * PF_COLS;
		pfline[x] = PF_FULL | PF_VIS | block;

		nblank = 0;
		for(j=0; j<PF_COLS; j++) {
			if(!(pfline[j] & PF_FULL)) {
				nblank++;
			}
		}

		if(nblank == 0) {
			complines[num_complines++] = y;
		}
	}

	snd_stick();

	if(num_complines) {
		addscore(num_complines);
	}
}

static void erase_completed(void)
{
	int i, j, srow, drow;
	unsigned int *pfstart = pfield;
	unsigned int *dptr;

	/* sort completed lines from highest to lowest row number */
	for(i=0; i<num_complines-1; i++) {
		for(j=i+1; j<num_complines; j++) {
			if(complines[j] > complines[i]) {
				int tmp = complines[j];
				complines[j] = complines[i];
				complines[i] = tmp;
			}
		}
	}

	srow = drow = PF_ROWS - 1;
	dptr = pfstart + drow * PF_COLS;

	for(i=0; i<PF_ROWS; i++) {
		for(j=0; j<num_complines; j++) {
			if(complines[j] == srow) {
				srow--;
			}
		}

		if(srow < 0) {
			for(j=0; j<PF_COLS; j++) {
				dptr[j] &= ~PF_FULL;
			}

		} else if(srow != drow) {
			unsigned int *sptr = pfstart + srow * PF_COLS;
			memcpy(dptr, sptr, PF_COLS * sizeof *dptr);
		}

		srow--;
		drow--;
		dptr -= PF_COLS;
	}
}

