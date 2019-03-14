#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <cgmath/cgmath.h>
#include <imago2.h>
#include "opengl.h"
#include "game.h"
#include "logger.h"

static unsigned int tex_bolt, tex_star;
static float star_speed = 50.0f;
static float star_depth = 1000.0f;
static float star_size = 0.35f;

#define STAR_ZOFFS	100.0f
#define STAR_COUNT	4096
static cgm_vec3 star[STAR_COUNT];
static float star_lenxy[STAR_COUNT];

int init_starfield(void)
{
	int i;
	float width, excl_rad;

	if(!(tex_star = img_gltexture_load("data/pimg.png"))) {
		error_log("failed to load star texture\n");
		return -1;
	}
	if(!(tex_bolt = img_gltexture_load("data/bolt.png"))) {
		error_log("failed to load star tail texture\n");
		return -1;
	}

	width = star_depth / 4.0f;
	excl_rad = 10.0f / width;
	for(i=0; i<STAR_COUNT; i++) {
		float x, y, z;

		do {
			x = (2.0f * rand() / RAND_MAX) - 1.0;
			y = (2.0f * rand() / RAND_MAX) - 1.0;
		} while(x * x + y * y < excl_rad * excl_rad);
		z = (float)rand() / RAND_MAX;

		cgm_vcons(star + i, x * width, y * width, z * star_depth);
		star_lenxy[i] = sqrt(star[i].x * star[i].x + star[i].y * star[i].y);
	}
	return 0;
}

void draw_starfield(void)
{
	int i;
	cgm_vec3 pos;
	float x, y, z, t;
	float x0, y0, x1, y1;
	float theta, sz;

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex_bolt);

	glBegin(GL_QUADS);
	for(i=0; i<STAR_COUNT; i++) {
		pos = star[i];
		z = fmod(pos.z + time_msec * star_speed / 1000.0f, star_depth);
		t = z / star_depth;
		pos.z = z - star_depth + STAR_ZOFFS;

		theta = atan2(pos.y / star_lenxy[i], pos.x / star_lenxy[i]);

		y = -star_size;
		x = 0;

		x0 = x * cos(theta) - y * sin(theta);
		y0 = x * sin(theta) + y * cos(theta);

		y = star_size;

		x1 = x * cos(theta) - y * sin(theta);
		y1 = x * sin(theta) + y * cos(theta);

		x0 += pos.x;
		x1 += pos.x;
		y0 += pos.y;
		y1 += pos.y;

		glColor4f(1, 1, 1, t);
		glTexCoord2f(0, 1);
		glVertex3f(x0, y0, pos.z);
		glTexCoord2f(1, 1);
		glVertex3f(x1, y1, pos.z);
		glTexCoord2f(1, 0);
		glVertex3f(x1, y1, pos.z - star_size * 16.0);
		glTexCoord2f(0, 0);
		glVertex3f(x0, y0, pos.z - star_size * 16.0);
	}
	glEnd();

	glBindTexture(GL_TEXTURE_2D, tex_star);
	sz = star_size * 4.0f;
	glBegin(GL_QUADS);
	for(i=0; i<STAR_COUNT; i++) {
		pos = star[i];
		z = fmod(pos.z + time_msec * star_speed / 1000.0f, star_depth);
		t = z / star_depth;
		pos.z = z - star_depth + STAR_ZOFFS;

		glColor4f(1, 1, 1, t);
		glTexCoord2f(0, 0);
		glVertex3f(pos.x - sz, pos.y - sz, pos.z);
		glTexCoord2f(1, 0);
		glVertex3f(pos.x + sz, pos.y - sz, pos.z);
		glTexCoord2f(1, 1);
		glVertex3f(pos.x + sz, pos.y + sz, pos.z);
		glTexCoord2f(0, 1);
		glVertex3f(pos.x - sz, pos.y + sz, pos.z);
	}
	glEnd();

	glPopAttrib();
}
