#include "fltbool.h"

void vrtk_bool_init(struct vrtk_bool *b, int value)
{
	b->value = value;
	b->fval = value;
	b->dsign = 0;
	b->speed = VRTK_BOOL_DEF_SPEED;
}

void vrtk_bool_set(struct vrtk_bool *b, int value)
{
	b->value = value;
	b->fval = value;
	b->dsign = 0;
}

void vrtk_bool_change(struct vrtk_bool *b, int value)
{
	b->dsign = value ? 1 : -1;
}

void vrtk_bool_update(struct vrtk_bool *b, float dt)
{
	if(b->dsign == 0) return;

	if(b->dsign > 0) {
		b->fval += dt * b->speed;
		b->value = 1;
	} else {
		b->fval -= dt * b->speed;
		b->value = 0;
	}

	if(b->fval <= 0.0f) {
		b->fval = 0.0f;
		b->dsign = 0;
	} else if(b->fval >= 1.0f) {
		b->fval = 1.0f;
		b->dsign = 0;
	}
}
