#include <stdlib.h>
#include "opengl.h"
#include "logger.h"


static void GLAPIENTRY gldebug_logger(GLenum src, GLenum type, GLuint id, GLenum severity,
		GLsizei len, const char *msg, const void *cls);

static const char *gldebug_srcstr(unsigned int src);
static const char *gldebug_typestr(unsigned int type);

struct GLCaps glcaps;

int init_opengl(void)
{
	glewInit();

	glcaps.debug = GLEW_ARB_debug_output;

#ifndef NDEBUG
	if(glcaps.debug) {
		info_log("Installing OpenGL debug callback\n");
		glDebugMessageCallbackARB(gldebug_logger, 0);
	}
#endif

	return 0;
}

void dump_gl_texture(unsigned int tex, const char *fname)
{
	FILE *fp;
	int i, width, height;
	unsigned char *pixels;

	glBindTexture(GL_TEXTURE_2D, tex);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);

	if(!(pixels = malloc(width * height * 3))) {
		return;
	}
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	if(!(fp = fopen(fname, "wb"))) {
		free(pixels);
		return;
	}
	fprintf(fp, "P6\n%d %d\n255\n", width, height);
	for(i=0; i<width * height * 3; i++) {
		fputc(pixels[i], fp);
	}
	fclose(fp);
	free(pixels);
}


static void GLAPIENTRY gldebug_logger(GLenum src, GLenum type, GLuint id, GLenum severity,
		GLsizei len, const char *msg, const void *cls)
{
	static const char *fmt = "[GLDEBUG] (%s) %s: %s\n";
	switch(type) {
	case GL_DEBUG_TYPE_ERROR:
		error_log(fmt, gldebug_srcstr(src), gldebug_typestr(type), msg);
		break;

	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
	case GL_DEBUG_TYPE_PORTABILITY:
	case GL_DEBUG_TYPE_PERFORMANCE:
		debug_log(fmt, gldebug_srcstr(src), gldebug_typestr(type), msg);
		break;

	default:
		debug_log(fmt, gldebug_srcstr(src), gldebug_typestr(type), msg);
	}
}

static const char *gldebug_srcstr(unsigned int src)
{
	switch(src) {
	case GL_DEBUG_SOURCE_API:
		return "api";
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		return "wsys";
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		return "sdrc";
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		return "3rdparty";
	case GL_DEBUG_SOURCE_APPLICATION:
		return "app";
	case GL_DEBUG_SOURCE_OTHER:
		return "other";
	default:
		break;
	}
	return "unknown";
}

static const char *gldebug_typestr(unsigned int type)
{
	switch(type) {
	case GL_DEBUG_TYPE_ERROR:
		return "error";
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		return "deprecated";
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		return "undefined behavior";
	case GL_DEBUG_TYPE_PORTABILITY:
		return "portability warning";
	case GL_DEBUG_TYPE_PERFORMANCE:
		return "performance warning";
	case GL_DEBUG_TYPE_OTHER:
		return "other";
	default:
		break;
	}
	return "unknown";
}
