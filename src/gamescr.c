#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include "opengl.h"
#include "game.h"
#include "screen.h"
#include "cmesh.h"
#include "blocks.h"

static int init(void);
static void cleanup(void);
static void start(void);
static void stop(void);
static void update(float dt);
static void draw(void);
static void draw_block(int block, const int *pos, int rot);
static void drawpf(void);
static void reshape(int x, int y);
static void keyboard(int key, int pressed);
static void mouse(int bn, int pressed, int x, int y);
static void motion(int x, int y);
static void wheel(int dir);

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

static struct cmesh *blkmesh;
static float cam_theta, cam_phi, cam_dist = 30;
static int bnstate[16];
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

#define NUM_LEVELS	21
static const long level_speed[NUM_LEVELS] = {
	887, 820, 753, 686, 619, 552, 469, 368, 285, 184,
	167, 151, 134, 117, 107, 98, 88, 79, 69, 60, 50
};


static int init(void)
{
	if(!(blkmesh = cmesh_alloc())) {
		return -1;
	}
	if(cmesh_load(blkmesh, "data/noisecube.obj") == -1) {
		fprintf(stderr, "failed to load block model\n");
		return -1;
	}
	return 0;
}

static void cleanup(void)
{
	cmesh_free(blkmesh);
}

static void start(void)
{
	srand(time(0));

	pause = 0;
	gameover = 0;
	num_complines = 0;
	score = level = lines = 0;
	tick_interval = level_speed[0];
	cur_block = -1;
	prev_block = 0;
	next_block = rand() % NUM_BLOCKS;

	memset(pfield, 0, PF_COLS * PF_ROWS * sizeof *pfield);
}

static void stop(void)
{
}

static void update(float dtsec)
{
	static long prev_tick;
	long dt;

	if(pause) {
		prev_tick = time_msec;
		return;
	}
	dt = time_msec - prev_tick;

	/*
	if(gameover) {
		int i, row = PF_ROWS - gameover;
		int *ptr;

		if(dt < GAMEOVER_FILL_RATE) {
			return;
		}

		if(row >= 0) {
			ptr = pfield + row * PF_COLS;
			for(i=0; i<PF_COLS; i++) {
				*ptr++ = TILE_GAMEOVER;
			}

			gameover++;
			prev_tick = msec;
		}
		return;
	}
	*/

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
}

static void draw(void)
{
	glTranslatef(0, 0, -cam_dist);
	glRotatef(cam_phi, 1, 0, 0);
	glRotatef(cam_theta, 0, 1, 0);

	/* center playfield */
	glPushMatrix();
	glTranslatef(-PF_COLS / 2, PF_ROWS / 2, 0);

	drawpf();
	if(cur_block >= 0) {
		draw_block(cur_block, pos, cur_rot);
	}

	glPopMatrix();
}

static void draw_block(int block, const int *pos, int rot)
{
	int i, pal;
	unsigned char *p = blocks[block][rot];

	/*pal = FIRST_BLOCK_PAL + block;*/

	for(i=0; i<4; i++) {
		int x = pos[1] + BLKX(*p);
		int y = pos[0] + BLKY(*p);
		p++;

		if(y < 0) continue;

		glPushMatrix();
		glTranslatef(x, y, 0);
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
			if(val & PF_FULL) {
				glPushMatrix();
				glTranslatef(j, i, 0);
				cmesh_draw(blkmesh);
				glPopMatrix();
			}
		}
	}
}


static void reshape(int x, int y)
{
}

static void keyboard(int key, int pressed)
{
	/*char *name = 0;*/

	if(!pressed) return;

	switch(key) {
	case 'a':
		if(!pause) {
			next_pos[1] = pos[1] - 1;
			if(collision(cur_block, next_pos)) {
				next_pos[1] = pos[1];
			} else {
				/*snd_shift();*/
			}
		}
		break;

	case 'd':
		if(!pause) {
			next_pos[1] = pos[1] + 1;
			if(collision(cur_block, next_pos)) {
				next_pos[1] = pos[1];
			} else {
				/*snd_shift();*/
			}
		}
		break;

	case 'w':
		if(!pause) {
			prev_rot = cur_rot;
			cur_rot = (cur_rot + 1) & 3;
			if(collision(cur_block, next_pos)) {
				cur_rot = prev_rot;
			} else {
				/*snd_rot();*/
			}
		}
		break;

	case 's':
		/* ignore drops until the first update after a spawn */
		if(cur_block >= 0 && !just_spawned && !pause) {
			next_pos[0] = pos[0] + 1;
			if(collision(cur_block, next_pos)) {
				next_pos[0] = pos[0];
				stick(cur_block, next_pos);	/* stick immediately */
			}
		}
		break;

	case '\n':
	case '\t':
		if(!pause && cur_block >= 0) {
			next_pos[0] = pos[0] + 1;
			while(!collision(cur_block, next_pos)) {
				next_pos[0]++;
			}
			next_pos[0]--;
			stick(cur_block, next_pos);	/* stick immediately */
		}
		break;

	case 'p':
		if(gameover) {
			/*
			if(score && is_highscore(score)) {
				name = name_screen(score);
			}
			save_score(name, score, lines, level);
			*/
			/* TODO: pop screen */
		} else {
			pause ^= 1;
		}
		break;

	case '\b':
		/*
		if(score && is_highscore(score)) {
			name = name_screen(score);
		}
		save_score(name, score, lines, level);
		*/
		/* TODO: pop screen */
		break;

	default:
		break;
	}
}

static void mouse(int bn, int pressed, int x, int y)
{
	bnstate[bn] = pressed;
	prev_mx = x;
	prev_my = y;
}

static void motion(int x, int y)
{
	float dx = x - prev_mx;
	float dy = y - prev_my;
	prev_mx = x;
	prev_my = y;

	if(bnstate[0]) {
		cam_theta += dx * 0.5;
		cam_phi += dy * 0.5;

		if(cam_phi < -90) cam_phi = -90;
		if(cam_phi > 90) cam_phi = 90;
	}
	if(bnstate[2]) {
		cam_dist += dy * 0.1;
		if(cam_dist < 0) cam_dist = 0;
	}
}

static void wheel(int dir)
{
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

	/*snd_stick();*/

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

