#include "sndfx.h"
#include "audio.h"
#include "logger.h"

#define MAX_SOUNDS	8

struct audio_sample *samp_plonk;
struct audio_source *src[MAX_SOUNDS];

int init_sndfx(void)
{
	int i;

	if(!(samp_plonk = au_create_sample()) || au_load_sample(samp_plonk, "data/plonk.ogg") == -1) {
		error_log("failed to create plonk sound effect\n");
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
}

void snd_stick(void)
{
	au_set_source_sample(src[0], samp_plonk);
	au_play_source(src[0]);
}
