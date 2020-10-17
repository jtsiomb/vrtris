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
#include "sndfx.h"
#include "audio.h"
#include "logger.h"

#define MAX_SOUNDS	8

static int findchan(void);

struct audio_sample *samp_plonk;
struct audio_sample *samp_woosh;
struct audio_sample *samp_drip;
struct audio_source *src[MAX_SOUNDS];
static float vol = 1.0f;

int init_sndfx(void)
{
	int i;

	if(!(samp_plonk = au_create_sample()) || au_load_sample(samp_plonk, "data/plonk.ogg") == -1) {
		error_log("failed to create plonk sound effect\n");
		return -1;
	}
	if(!(samp_woosh = au_create_sample()) || au_load_sample(samp_woosh, "data/woosh.ogg") == -1) {
		error_log("failed to create woosh sound effect\n");
		return -1;
	}
	if(!(samp_drip = au_create_sample()) || au_load_sample(samp_drip, "data/drip.ogg") == -1) {
		error_log("failed to create drip sound effect\n");
		return -1;
	}

	for(i=0; i<MAX_SOUNDS; i++) {
		if(!(src[i] = au_create_source())) {
			error_log("failed to create source\n");
			return -1;
		}
	}
	return 0;
}

void destroy_sndfx(void)
{
	int i;
	for(i=0; i<MAX_SOUNDS; i++) {
		au_free_source(src[i]);
	}
	au_free_sample(samp_plonk);
	au_free_sample(samp_woosh);
	au_free_sample(samp_drip);
}

void set_snd_volume(float v)
{
	int i;
	vol = v;
	for(i=0; i<MAX_SOUNDS; i++) {
		au_set_source_volume(src[i], v);
	}
}

float get_snd_volume(void)
{
	return vol;
}

void snd_stick(void)
{
	int idx = findchan();
	au_set_source_sample(src[idx], samp_plonk);
	au_play_source(src[idx]);
}

void snd_rot(void)
{
	int idx = findchan();
	au_set_source_sample(src[idx], samp_drip);
	au_play_source(src[idx]);
}

void snd_shift(void)
{
	int idx = findchan();
	au_set_source_sample(src[idx], samp_woosh);
	au_play_source(src[idx]);
}

static int findchan(void)
{
	int i;
	for(i=0; i<MAX_SOUNDS; i++) {
		if(!au_is_source_playing(src[i])) {
			return i;
		}
	}
	return 0;
}
