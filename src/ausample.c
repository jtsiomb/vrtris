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
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <vorbis/vorbisfile.h>
#include "openal.h"
#include "logger.h"

struct audio_sample {
	unsigned int albuf;
};

/* --- audio samples --- */
struct audio_sample *au_create_sample(void)
{
	struct audio_sample *as;

	if(!(as = malloc(sizeof *as))) {
		error_log("failed to allocate audio_sample structure\n");
		return 0;
	}
	alGenBuffers(1, &as->albuf);
	return as;
}

void au_free_sample(struct audio_sample *as)
{
	if(as && as->albuf) {
		alDeleteBuffers(1, &as->albuf);
		free(as);
	}
}

int au_set_sample_data(struct audio_sample *as, int rate, int nchan, int nsamples, void *data)
{
	unsigned int err;
	ALenum alfmt = nchan == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;

	alBufferData(as->albuf, alfmt, data, nsamples * 2, rate);
	if((err = alGetError())) {
		error_log("failed to set sample data to an OpenAL buffer (err: %x)\n", err);
		return -1;
	}
	return 0;
}

int au_load_sample(struct audio_sample *as, const char *fname)
{
	OggVorbis_File vf;
	vorbis_info *vinf;
	long num_samples, bufsz, total_read;
	int16_t *samples, *sptr;

	if(ov_fopen(fname, &vf) != 0) {
		error_log("failed to open ogg/vorbis file: %s\n", fname);
		return -1;
	}
	vinf = ov_info(&vf, -1);

	info_log("loading sample: %s: %ld samples/s, %s (%d chan)\n", fname, vinf->rate,
			vinf->channels == 1 ? "mono" : "stereo", vinf->channels);

	num_samples = ov_pcm_total(&vf, -1) * vinf->channels;
	bufsz = num_samples * sizeof *samples;

	if(!(samples = malloc(bufsz))) {
		error_log("failed to allocate samples buffer (%ld bytes)\n", bufsz);
		ov_clear(&vf);
		return -1;
	}

	total_read = 0;
	sptr = samples;
	while(total_read < bufsz) {
		int bitstream;
		long rd = ov_read(&vf, (char*)sptr, bufsz, 0, 2, 1, &bitstream);
		if(!rd) {
			bufsz = total_read;
			error_log("unexpected eof while reading %s\n", fname);
			ov_clear(&vf);
			free(samples);
			return -1;
		}
		total_read += rd;
		sptr += rd / 2;
	}

	if(au_set_sample_data(as, vinf->rate, vinf->channels, num_samples, samples) == -1) {
		error_log("failed to set sample data\n");
		ov_clear(&vf);
		free(samples);
		return -1;
	}

	free(samples);
	ov_clear(&vf);
	return 0;
}

void au_play_sample(struct audio_sample *as)
{
	/* TODO */
}
