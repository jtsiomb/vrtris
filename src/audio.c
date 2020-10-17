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
