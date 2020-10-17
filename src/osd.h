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
#ifndef OSD_H_
#define OSD_H_

#include <stdarg.h>

struct dtx_font;

struct dtx_font *ui_font;
int ui_font_size;

void set_message_timeout(long timeout);
void osd_printf(const char *fmt, ...);
void show_message(long timeout, float r, float g, float b, const char *fmt, ...);
void show_messagev(long timeout, float r, float g, float b, const char *fmt, va_list ap);

void print_text(float x, float y, float r, float g, float b, const char *fmt, ...);
void print_textv(float x, float y, float r, float g, float b, const char *fmt, va_list ap);

void draw_osd(void);

#endif	/* OSD_H_ */
