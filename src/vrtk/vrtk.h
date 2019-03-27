#ifndef VRTK_H_
#define VRTK_H_

#include <cgmath/cgmath.h>

struct vrtk;
struct vrtk_widget;

/* bounding box extends from center - vec to center + vec in each axis */
struct vrtk_box {
	cgm_vec3 center;
	cgm_vec3 uvec, vvec, wvec;
};

struct vrtk *vrtk_init(void);
void vrtk_shutdown(struct vrtk *tk);

void vrtk_update(struct vrtk *tk, float dt);
void vrtk_draw(struct vrtk *tk);

struct vrtk_widget *vrtk_create_widget(struct vrtk *tk);
void vrtk_destroy_widget(struct vrtk_widget *w);

void vrtk_set_widget_xform(struct vrtk_widget *w, float *matrix);
void vrtk_get_widget_xform(struct vrtk_widget *w, float *matrix);


/* returns the plane equation defining a plane of the box
 * plane is a pointer to 4 floats (plane equation: ax + by + cz + d = 0)
 */
void vrtk_bbox_planes(struct vrtk_box *b, float *plane);

#endif	/* VRTK_H_ */
