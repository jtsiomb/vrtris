#ifndef COLOR_H_
#define COLOR_H_

void rgb_to_hsv(float r, float g, float b, float *h, float *s, float *v);
void hsv_to_rgb(float h, float s, float v, float *r, float *g, float *b);

#endif	/* COLOR_H_ */
