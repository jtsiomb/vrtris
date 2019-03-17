#include "openal.h"

/* --- audio sources --- */
struct audio_source *au_create_source(void);
void au_destroy_source(struct audio_source *asrc);

void au_set_source_sample(struct audio_source *asrc, struct audio_sample *as);
struct audio_sample *au_get_source_sample(struct audio_source *asrc);

void au_set_source_pos(struct audio_source *asrc, float x, float y, float z);
void au_get_source_pos(struct audio_source *asrc, float *x, float *y, float *z);

void au_set_source_refdist(struct audio_source *asrc, float refdist);

int au_is_source_playing(struct audio_source *asrc);
void au_play_source(struct audio_source *asrc);
void au_stop_source(struct audio_source *asrc);
