#ifndef VRTK_THEME_H_
#define VRTK_THEME_H_

#include "vrtk.h"

struct vrtk_theme {
	char *name;
	void *data;

	void (*draw)(struct vrtk_widget*);
	int (*bounds)(struct vrtk_widget*, struct vrtk_box*);
	int (*contains)(struct vrtk_widget*, cgm_vec3*);
	float (*rayhit)(struct vrtk_widget*, cgm_ray*);
	int (*boxhit)(struct vrtk_widget*, struct vrtk_box*);
};

#endif	/* VRTK_THEME_H_ */
