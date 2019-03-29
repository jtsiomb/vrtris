#ifndef VRTK_H_
#define VRTK_H_

#include <cgmath/cgmath.h>
#include "fltbool.h"

struct vrtk;
struct vrtk_widget;

enum vrtk_action {
	VRTK_ACT_PRIMARY,
	VRTK_ACT_SECONDARY,
	VRTK_ACT_GRAB,
	VRTK_ACT_RELEASE,
	VRTK_ACT_UP,
	VRTK_ACT_DOWN,
	VRTK_ACT_LEFT,
	VRTK_ACT_RIGHT
};

enum vrtk_event_type {
	VRTK_EVCLICK,
	VRTK_EVMODIFY,

	VRTK_NUM_EVENT_TYPES
};

struct vrtk_event {
	int type;
};

typedef void (*vrtk_callback_func)(struct vrtk_widget*, struct vrtk_event *ev, void *cls);

/* bounding box extends from center - vec to center + vec in each axis */
struct vrtk_box {
	cgm_vec3 c;
	cgm_vec3 u, v, w;
};

struct vrtk *vrtk_init(void);
void vrtk_shutdown(struct vrtk *tk);

/* vrtk_update returns non-zero if there are widgets in motion, which means
 * that vrtk_update needs to be called again soon. Returns 0 if everything is
 * stable, and as long as there is no relevant input, the UI doesn't need
 * updating or redrawing again.
 *
 * This facilitates event-driven non-busy-looping use, if that's desirable, or
 * you can call update every time around the main loop in programs that redraw
 * continuously anyway.
 */
int vrtk_update(struct vrtk *tk, float dt);
void vrtk_draw(struct vrtk *tk);

/* returns the oriented bounding box of the whole UI */
void vrtk_bounds(struct vrtk *tk, struct vrtk_box *bbox);

/* input */
void vrtk_input_pointer(struct vrtk *tk, cgm_ray *ray);
void vrtk_input_action(struct vrtk *tk, enum vrtk_action act);
void vrtk_input_key(struct vrtk *tk, int key, int pressed);


/* widget */
struct vrtk_widget *vrtk_create_widget(struct vrtk *tk);
void vrtk_destroy_widget(struct vrtk_widget *w);

void vrtk_attach_widget(struct vrtk_widget *w, struct vrtk_widget *parent);
void vrtk_detach_widget(struct vrtk_widget *w);

/* returns the oriented bounding box of the widget in world space */
void vrtk_widget_bounds(struct vrtk_widget *w, struct vrtk_box *bbox);
/* returns the oriented bounding box of thr widget in its own local system */
void vrtk_widget_local_bounds(struct vrtk_widget *w, struct vrtk_box *bbox);

void vrtk_set_widget_pos(struct vrtk_widget *w, cgm_vec3 *pos);
void vrtk_set_widget_pos3f(struct vrtk_widget *w, float x, float y, float z);
void vrtk_set_widget_rotation(struct vrtk_widget *w, cgm_quat *rot);
void vrtk_set_widget_size(struct vrtk_widget *w, float s);

/* register an event callback */
void vrtk_callback(struct vrtk_widget *w, int evtype, vrtk_callback_func func, void *cls);

void vrtk_widget_update(struct vrtk_widget *w, float dt);
void vrtk_widget_draw(struct vrtk_widget *w);

/* button */
struct vrtk_widget *vrtk_create_button(struct vrtk *tk, 

/* returns the plane equation defining a plane of the box
 * plane is a pointer to 4 floats (plane equation: ax + by + cz + d = 0)
 */
void vrtk_bbox_planes(struct vrtk_box *b, float *plane);

#endif	/* VRTK_H_ */
