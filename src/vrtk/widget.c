#include <stdio.h>
#include <stdlib.h>
#include "vrtkpriv.h"


struct vrtk_widget *vrtk_create_widget(struct vrtk *tk)
{
	struct vrtk_widget *w;

	if(!(w = malloc(sizeof *w))) {
		perror("failed to allocate widget");
		return 0;
	}
	vrtk_init_widget(w);

	if(tk) {
		vrtk_add_widget(tk, w);
	}
	return w;
}

void vrtk_init_widget(struct vrtk_widget *w)
{
	memset(w, 0, sizeof *w);

	cgm_qcons(&w->rot, 0, 0, 0, 1);
	w->size = 1.0f;

	vrtk_bool_init(&w->visible, 1);
	vrtk_bool_init(&w->active, 1);
	vrtk_bool_init(&w->grabbed, 0);
	vrtk_bool_init(&w->hover, 0);
	vrtk_bool_init(&w->focus, 0);
}

void vrtk_destroy_widget(struct vrtk_widget *w)
{
	if(!w) return;

	if(w->ui) {
		vrtk_remove_widget(w->ui, w);
	}

	free(w->text);
	free(w);
}

struct vrtk_widget *vrtk_widget_contains(struct vrtk_widget *w, cgm_vec3 *pt, float rad)
{
	return 0;	/* TODO */
}

struct vrtk_widget *vrtk_widget_rayhit(struct vrtk_widget *w, cgm_ray *ray, float *hit)
{
	return 0;	/* TODO */
}

int vrtk_update_widget(struct vrtk_widget *w, float dt)
{
	return 0;	/* TODO */
}

void vrtk_draw_widget(struct vrtk_widget *w)
{
}
