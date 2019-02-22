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
