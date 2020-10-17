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
#ifndef OPENGL_H_
#define OPENGL_H_

#include <GL/glew.h>

struct GLCaps {
	int debug;	/* ARB_debug_output */
};

extern struct GLCaps glcaps;

int init_opengl(void);

void dump_gl_texture(unsigned int tex, const char *fname);

#endif	/* OPENGL_H_ */
