#include "openal.h"
#include "audio.h"
#include "logger.h"

static ALCdevice *dev;
static ALCcontext *ctx;

int au_init(void)
{
	if(dev) return 0;

	if(!(dev = alcOpenDevice(0))) {
		error_log("failed to open OpenAL device\n");
		return -1;
	}
	ctx = alcCreateContext(dev, 0);
	alcMakeContextCurrent(ctx);

	alGetError();	/* clear error code */
	return 0;
}

void au_destroy(void)
{
	alcMakeContextCurrent(0);
	if(ctx) {
		alcDestroyContext(ctx);
	}
	if(dev) {
		alcCloseDevice(dev);
	}
}

void au_set_listener(float *xform)
{
	float orient[6];

	alListenerfv(AL_POSITION, xform + 12);

	orient[0] = xform[8];
	orient[1] = xform[9];
	orient[2] = -xform[10];

	orient[3] = xform[4];
	orient[4] = xform[5];
	orient[5] = xform[6];

	alListenerfv(AL_ORIENTATION, orient);
}
