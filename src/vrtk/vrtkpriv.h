#ifndef VRTKPRIV_H_
#define VRTKPRIV_H_

#include "vrtk.h"
#include "fltbool.h"
#include "theme.h"

struct vrtk {
	struct vrtk_widget *wlist;

	struct vrtk_box bbox;
	int bbox_valid;

	struct vrtk_widget *hover;
	struct vrtk_widget *grabbed;
	struct vrtk_widget *active;
	struct vrtk_widget *focus;

	struct vrtk_theme *theme;
};

struct widget_ops {
};

struct vrtk_widget {
	int type;

	struct vrtk *ui;	/* parent gui instance */
	struct vrtk_widget *parent;
	struct vrtk_widget *next;

	char *name;

	cgm_vec3 pos;
	cgm_quat rot;
	float size;

	float xform[4];
	int xform_valid;

	struct vrtk_box bbox;
	int bbox_valid;

	vrtk_bool active, grabbed, hover, focus;

	vrtk_callback_func cbfunc[VRTK_NUM_EVENT_TYPES];
	void *cbcls[VRTK_NUM_EVENT_TYPES];

	struct widget_ops *op;
};

#endif	/* VRTKPRIV_H_ */
