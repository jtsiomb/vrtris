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

/* widget events */
enum vrtk_event_type {
	VRTK_EV_ACTION,
	VRTK_EV_MODIFY,
	VRTK_EV_KEY,
	VRTK_EV_HOVER,

	VRTK_NUM_EVENT_TYPES
};

struct vrtk_event_action {
	enum vrtk_action action;
	cgm_vec3 pos;
};

struct vrtk_event_key {
	int key;
	int pressed;
};

struct vrtk_event_hover {
	int hover;
	cgm_vec3 pos;
	cgm_ray ray;
};

struct vrtk_event {
	int type;

	union {
		struct vrtk_event_action act;
		struct vrtk_event_key key;
		struct vrtk_event_hover hover;
	} data;
};

typedef void (*vrtk_callback_func)(struct vrtk_widget*, struct vrtk_event *ev, void *cls);


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

/* vrtk_contains tests if a sphere is contained in any of the widgets, and
 * returns the "most appropriate" containing widget, or 0 if no containing
 * widget was found. rad can be 0 for point containment tests
 */
struct vrtk_widget *vrtk_contains(struct vrtk *tk, cgm_vec3 *pt, float rad);

/* vrtk_rayhit tests if a ray hits any of the widgets, and returns the nearest
 * one, or 0 if no intersections were found. The parametric distance of the
 * intersection point is written through the hit pointer. hit can be 0 if the
 * intersection distance is not needed
 */
struct vrtk_widget *vrtk_rayhit(struct vrtk *tk, cgm_ray *ray, float *hit);

/* vrtk takes ownership of all widgets added, and relinquishes ownership of
 * any widgets removed. vrtk_remove_widget does not destroy the widget
 */
void vrtk_add_widget(struct vrtk *tk, struct vrtk_widget *w);
void vrtk_remove_widget(struct vrtk *tk, struct vrtk_widget *w);

/* input events */
void vrtk_input_pointer(struct vrtk *tk, cgm_ray *ray);
void vrtk_input_position(struct vrtk *tk, cgm_vec3 *pos);
void vrtk_input_rotation(struct vrtk *tk, cgm_quat *rot);
void vrtk_input_action(struct vrtk *tk, enum vrtk_action act);
void vrtk_input_key(struct vrtk *tk, int key, int pressed);


/* widget */
struct vrtk_widget *vrtk_create_widget(struct vrtk *tk);
void vrtk_destroy_widget(struct vrtk_widget *w);

void vrtk_set_widget_text(struct vrtk_widget *w, const char *text);
const char *vrtk_get_widget_text(struct vrtk_widget *w);

int vrtk_widget_visible(struct vrtk_widget *w);
int vrtk_widget_active(struct vrtk_widget *w);
int vrtk_widget_hovered(struct vrtk_widget *w);
int vrtk_widget_focused(struct vrtk_widget *w);
int vrtk_widget_grabbed(struct vrtk_widget *w);

/* see above */
struct vrtk_widget *vrtk_widget_contains(struct vrtk_widget *w, cgm_vec3 *pt, float rad);
struct vrtk_widget *vrtk_widget_rayhit(struct vrtk_widget *w, cgm_ray *ray, float *hit);

void vrtk_set_widget_pos(struct vrtk_widget *w, cgm_vec3 *pos);
void vrtk_set_widget_pos3f(struct vrtk_widget *w, float x, float y, float z);
void vrtk_set_widget_rotation(struct vrtk_widget *w, cgm_quat *rot);
void vrtk_set_widget_size(struct vrtk_widget *w, float s);

/* register an event callback */
void vrtk_callback(struct vrtk_widget *w, int evtype, vrtk_callback_func func, void *cls);

int vrtk_update_widget(struct vrtk_widget *w, float dt);
void vrtk_draw_widget(struct vrtk_widget *w);

/* button */

#endif	/* VRTK_H_ */
