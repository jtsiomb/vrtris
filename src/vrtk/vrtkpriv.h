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
#ifndef VRTKPRIV_H_
#define VRTKPRIV_H_

#include "vrtk.h"
#include "fltbool.h"
#include "theme.h"

enum {
	INP_RAY_PENDING		= 0x0001,
	INP_POS_PENDING		= 0x0002,
	INP_ROT_PENDING		= 0x0004
};

struct vrtk {
	struct vrtk_widget *wlist;

	struct vrtk_widget *hover;
	struct vrtk_widget *grabbed; /* XXX do we need both grab and active? */
	struct vrtk_widget *active;
	struct vrtk_widget *focus;

	struct vrtk_theme *theme;

	/* defered input handling state */
	cgm_ray inp_ray;
	cgm_vec3 inp_pos;
	cgm_quat inp_rot;
	unsigned int inp_pending;
};

struct widget_ops {
	int (*handle_event)(struct vrtk_widget*, struct vrtk_event*);
};

struct vrtk_widget {
	int type;

	struct vrtk *ui;	/* parent gui instance */
	struct vrtk_widget *parent;
	struct vrtk_widget *next, *prev;

	char *text;

	cgm_vec3 pos;
	cgm_quat rot;
	float size;

	float xform[4];
	int xform_valid;

	int can_focus;

	vrtk_bool visible, active, grabbed, hover, focus;

	vrtk_callback_func cbfunc[VRTK_NUM_EVENT_TYPES];
	void *cbcls[VRTK_NUM_EVENT_TYPES];

	struct widget_ops *op;
};

void vrtk_init_widget(struct vrtk_widget *w);

int vrtk_send_action_event(struct vrtk_widget *w, enum vrtk_action act, cgm_vec3 *pos);
int vrtk_send_modify_event(struct vrtk_widget *w);
int vrtk_send_key_event(struct vrtk_widget *w, int key, int pressed);

#endif	/* VRTKPRIV_H_ */
