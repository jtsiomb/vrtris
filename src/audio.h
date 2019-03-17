#ifndef AUDIO_H_
#define AUDIO_H_

struct audio_sample;
struct audio_source;
/*struct audio_stream; TODO */

int au_init(void);
void au_destroy(void);

void au_set_listener(float *xform);

/* --- audio samples --- */
struct audio_sample *au_create_sample(void);
void au_free_sample(struct audio_sample *as);

/* data can be 0, in which case only the buffer gets allocated */
int au_set_sample_data(struct audio_sample *as, int rate, int nchan, int nsamples, void *data);
int au_load_sample(struct audio_sample *as, const char *fname);

/* non-positional sample playback, choose an idle source and play */
void au_play_sample(struct audio_sample *as);

/* --- audio sources --- */
struct audio_source *au_create_source(void);
void au_free_source(struct audio_source *asrc);

void au_set_source_sample(struct audio_source *asrc, struct audio_sample *as);
struct audio_sample *au_get_source_sample(struct audio_source *asrc);

void au_set_source_pos(struct audio_source *asrc, float x, float y, float z);
void au_get_source_pos(struct audio_source *asrc, float *x, float *y, float *z);

void au_set_source_refdist(struct audio_source *asrc, float refdist);

int au_is_source_playing(struct audio_source *asrc);
void au_play_source(struct audio_source *asrc);
void au_stop_source(struct audio_source *asrc);

#endif	/* AUDIO_H_ */
