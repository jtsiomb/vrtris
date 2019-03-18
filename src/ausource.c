#include <stdlib.h>
#include "openal.h"
#include "audio.h"
#include "logger.h"

struct audio_source {
	unsigned int alsrc;
	struct audio_sample *sample;
};

struct audio_source *au_create_source(void)
{
	struct audio_source *asrc;

	if(!(asrc = malloc(sizeof *asrc))) {
		error_log("failed to allocate audio source\n");
		return 0;
	}
	asrc->sample = 0;

	alGenSources(1, &asrc->alsrc);
	alSourcei(asrc->alsrc, AL_LOOPING, AL_TRUE);

	return asrc;
}

void au_free_source(struct audio_source *asrc)
{
	if(asrc) {
		if(au_is_source_playing(asrc)) {
			au_stop_source(asrc);
		}
		alDeleteSources(1, &asrc->alsrc);
		free(asrc);
	}
}

void au_set_source_sample(struct audio_source *asrc, struct audio_sample *as)
{
	au_stop_source(asrc);

	if(as) {
		if(!as->albuf) {
			error_log("trying to attach null buffer\n");
			return;
		}
		alSourcei(asrc->alsrc, AL_BUFFER, as->albuf);
	}
	asrc->sample = as;
}

struct audio_sample *au_get_source_sample(struct audio_source *asrc)
{
	return asrc->sample;
}

void au_set_source_pos(struct audio_source *asrc, float x, float y, float z)
{
	alSourcei(asrc->alsrc, AL_SOURCE_RELATIVE, AL_FALSE);
	alSource3f(asrc->alsrc, AL_POSITION, x, y, z);
}

void au_set_source_pos_rel(struct audio_source *asrc, float x, float y, float z)
{
	alSourcei(asrc->alsrc, AL_SOURCE_RELATIVE, AL_TRUE);
	alSource3f(asrc->alsrc, AL_POSITION, x, y, z);
}

void au_get_source_pos(struct audio_source *asrc, float *x, float *y, float *z)
{
	float pos[3];
	alGetSourcefv(asrc->alsrc, AL_POSITION, pos);
	*x = pos[0];
	*y = pos[1];
	*z = pos[2];
}

void au_set_source_volume(struct audio_source *asrc, float vol)
{
	alSourcef(asrc->alsrc, AL_GAIN, vol);
}

float au_get_source_volume(struct audio_source *asrc)
{
	float vol;
	alGetSourcef(asrc->alsrc, AL_GAIN, &vol);
	return vol;
}

void au_set_source_refdist(struct audio_source *asrc, float refdist)
{
	alSourcef(asrc->alsrc, AL_REFERENCE_DISTANCE, refdist);
}

int au_is_source_playing(struct audio_source *asrc)
{
	int state;
	alGetSourcei(asrc->alsrc, AL_SOURCE_STATE, &state);
	return state == AL_PLAYING;
}

void au_play_source(struct audio_source *asrc)
{
	if(asrc->sample) {
		alSourcei(asrc->alsrc, AL_LOOPING, AL_FALSE);
		alSourcePlay(asrc->alsrc);
	}
}

void au_play_source_loop(struct audio_source *asrc)
{
	if(asrc->sample) {
		alSourcei(asrc->alsrc, AL_LOOPING, AL_TRUE);
		alSourcePlay(asrc->alsrc);
	}
}

void au_stop_source(struct audio_source *asrc)
{
	if(asrc->sample) {
		alSourceStop(asrc->alsrc);
	}
}
