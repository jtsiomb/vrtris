#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <errno.h>
#include <assert.h>
#include "vrtkpriv.h"

struct vrtk *vrtk_init(void)
{
	struct vrtk *tk;

	if(!(tk = malloc(sizeof *tk))) {
		perror("failed to allocate vrtk context");
		return 0;
	}
	memset(tk, 0, sizeof *tk);

	return tk;
}

void vrtk_shutdown(struct vrtk *tk)
{
	if(!tk) return;

	while(tk->wlist) {
		struct vrtk_widget *w = tk->wlist;
		tk->wlist = tk->wlist->next;
		vrtk_destroy_widget(w);
	}

	free(tk);
}

int vrtk_update(struct vrtk *tk, float dt)
{
	int reupd = 0;
	struct vrtk_widget *w = tk->wlist;

	while(w) {
		int res = vrtk_update_widget(w, dt);
		if(res) reupd = 1;
		w = w->next;
	}
	return reupd;
}

void vrtk_draw(struct vrtk *tk)
{
	struct vrtk_widget *w = tk->wlist;

	while(w) {
		vrtk_draw_widget(w);
		w = w->next;
	}
}

struct vrtk_widget *vrtk_contains(struct vrtk *tk, cgm_vec3 *pt, float rad)
{
	struct vrtk_widget *w = tk->wlist;

	while(w) {
		struct vrtk_widget *res = vrtk_widget_contains(w, pt, rad);
		if(res) return res;
		w = w->next;
	}
	return 0;
}

struct vrtk_widget *vrtk_rayhit(struct vrtk *tk, cgm_ray *ray, float *hit)
{
	float t, best_t = FLT_MAX;
	struct vrtk_widget *w, *res, *best = 0;

	w = tk->wlist;
	while(w) {
		if((res = vrtk_widget_rayhit(w, ray, &t)) && t >= 0.0f && t < best_t) {
			best = res;
			best_t = t;
		}
		w = w->next;
	}

	return best;
}

void vrtk_add_widget(struct vrtk *tk, struct vrtk_widget *w)
{
	w->ui = tk;
	w->parent = 0;
	w->next = tk->wlist;
	w->prev = 0;
	if(tk->wlist) {
		tk->wlist->prev = w;
	}
	tk->wlist = w;
}

void vrtk_remove_widget(struct vrtk *tk, struct vrtk_widget *w)
{
	if(w->ui != tk) return;

	if(w->prev) {
		w->prev->next = w->next;
	} else {
		assert(tk->wlist == w);
		tk->wlist = w->next;
	}
	if(w->next) {
		w->next->prev = w->prev;
	}

	w->ui = 0;
	w->parent = w->prev = w->next = 0;
}


/* input handling */
/* defer ray and containment tests until next update */

void vrtk_input_pointer(struct vrtk *tk, cgm_ray *ray)
{
	tk->inp_ray = *ray;
	tk->inp_pending |= INP_RAY_PENDING;
}

void vrtk_input_position(struct vrtk *tk, cgm_vec3 *pos)
{
	tk->inp_pos = *pos;
	tk->inp_pending |= INP_POS_PENDING;
}

void vrtk_input_rotation(struct vrtk *tk, cgm_quat *rot)
{
	tk->inp_rot = *rot;
	tk->inp_pending |= INP_ROT_PENDING;
}

void vrtk_input_action(struct vrtk *tk, enum vrtk_action act)
{
	if(tk->hover) {
		vrtk_send_action_event(tk->hover, act, &tk->inp_pos);
	}
}

void vrtk_input_key(struct vrtk *tk, int key, int pressed)
{
	if(tk->focus) {
		vrtk_send_key_event(tk->focus, key, pressed);
	}
}

int vrtk_send_event(struct vrtk_widget *w, struct vrtk_event *ev)
{
	return w->op->handle_event(w, ev);
}

int vrtk_send_action_event(struct vrtk_widget *w, enum vrtk_action act, cgm_vec3 *pos)
{
	struct vrtk_event ev;

	ev.type = VRTK_EV_ACTION;
	ev.data.act.action = act;
	ev.data.act.pos = *pos;
	return vrtk_send_event(w, &ev);
}

int vrtk_send_modify_event(struct vrtk_widget *w)
{
	struct vrtk_event ev;

	ev.type = VRTK_EV_MODIFY;
	return vrtk_send_event(w, &ev);
}

int vrtk_send_key_event(struct vrtk_widget *w, int key, int pressed)
{
	struct vrtk_event ev;

	ev.type = VRTK_EV_KEY;
	ev.data.key.key = key;
	ev.data.key.pressed = pressed;
	return vrtk_send_event(w, &ev);
}
