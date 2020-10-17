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
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <drawtext.h>
#include "cgmath/cgmath.h"
#include "opengl.h"
#include "osd.h"
#include "game.h"

#define FONTSZ	16

static int init(void);

struct message {
	long start_time, show_until;
	char *str;
	cgm_vec3 color;
	struct message *next;
};
static struct message *msglist;

struct text {
	char *str;
	cgm_vec2 pos;
	cgm_vec3 color;
	struct text *next;
};
static struct text *txlist;

static long timeout = 2000;
static long trans_time = 250;

int ui_font_size = FONTSZ;

void set_message_timeout(long tm)
{
	timeout = tm;
}

void osd_printf(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	show_messagev(timeout, 1, 1, 1, fmt, ap);
	va_end(ap);
}

void show_message(long timeout, float r, float g, float b, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	show_messagev(timeout, r, g, b, fmt, ap);
	va_end(ap);
}

void show_messagev(long timeout, float r, float g, float b, const char *fmt, va_list ap)
{
	char buf[512];
	struct message *msg;
	struct message dummy;
	int len;

	init();

	vsnprintf(buf, sizeof buf, fmt, ap);

	if(!(msg = malloc(sizeof *msg))) {
		perror("failed to allocate memory");
		abort();
	}
	len = strlen(buf);
	if(!(msg->str = malloc(len + 1))) {
		perror("failed to allocate memory");
		abort();
	}
	memcpy(msg->str, buf, len + 1);
	msg->start_time = time_msec;
	msg->show_until = time_msec + timeout;
	msg->color.x = r;
	msg->color.y = g;
	msg->color.z = b;

	dummy.next = msglist;
	struct message *prev = &dummy;
	while(prev->next && prev->next->show_until < msg->show_until) {
		prev = prev->next;
	}
	msg->next = prev->next;
	prev->next = msg;
	msglist = dummy.next;
}

void print_text(float x, float y, float r, float g, float b, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	print_textv(x, y, r, g, b, fmt, ap);
	va_end(ap);
}

void print_textv(float x, float y, float r, float g, float b, const char *fmt, va_list ap)
{
	char buf[512];
	int len;
	struct text *tx;

	init();

	vsnprintf(buf, sizeof buf, fmt, ap);

	if(!(tx = malloc(sizeof *tx))) {
		perror("failed to allocate memory");
		abort();
	}
	len = strlen(buf);
	if(!(tx->str = malloc(len + 1))) {
		perror("failed to allocate memory");
		abort();
	}
	memcpy(tx->str, buf, len + 1);
	tx->color.x = r;
	tx->color.y = g;
	tx->color.z = b;
	tx->pos.x = x;
	tx->pos.y = -y;

	tx->next = txlist;
	txlist = tx;
}

void draw_osd(void)
{
	if(!ui_font) return;

	while(msglist && msglist->show_until <= time_msec) {
		struct message *msg = msglist;
		msglist = msg->next;
		free(msg->str);
		free(msg);
	}

	dtx_use_font(ui_font, ui_font_size);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, win_width, -win_height, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glPushAttrib(GL_ENABLE_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glUseProgram(0);

	struct message *msg = msglist;
	while(msg) {
		long t = time_msec - msg->start_time;
		long dur = msg->show_until - msg->start_time;
		float alpha = cgm_smoothstep(0, trans_time, t) *
			(1.0 - cgm_smoothstep(dur - trans_time, dur, t));
		glColor4f(msg->color.x, msg->color.y, msg->color.z, alpha);
		glTranslatef(0, -dtx_line_height(), 0);
		dtx_string(msg->str);
		msg = msg->next;
	}

	while(txlist) {
		struct text *tx = txlist;
		txlist = txlist->next;

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glTranslatef(tx->pos.x, tx->pos.y, 0);

		glColor3f(tx->color.x, tx->color.y, tx->color.z);
		dtx_string(tx->str);

		free(tx->str);
		free(tx);
	}

	glPopAttrib();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

static int init(void)
{
	static int done_init;
	if(done_init) return 0;

	done_init = 1;

	if(!(ui_font = dtx_open_font("data/ui.font", 0))) {
		fprintf(stderr, "failed to open font: data/ui.font\n");
		return -1;
	}
	dtx_prepare_range(ui_font, ui_font_size, 32, 127);
	return 0;
}
