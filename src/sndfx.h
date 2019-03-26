#ifndef SNDFX_H_
#define SNDFX_H_

int init_sndfx(void);
void destroy_sndfx(void);

void set_snd_volume(float vol);
float get_snd_volume(void);

void snd_stick(void);
void snd_shift(void);
void snd_rot(void);

#endif	/* SNDFX_H_ */
