#ifndef FLTBOOL_H_
#define FLTBOOL_H_

#define VRTK_BOOL_DEF_SPEED	2.0

typedef struct vrtk_bool {
	int value;
	float fval;
	int dsign;	/* 0: not changing, -1: decreasing, 1: increasing */
	float speed;
} vrtk_bool;

void vrtk_bool_init(struct vrtk_bool *b, int value);
void vrtk_bool_set(struct vrtk_bool *b, int value);
void vrtk_bool_change(struct vrtk_bool *b, int value);

/* returns 0 if the value has stabilized, or 1 if it's still animating
 * and needs to be updated again soon
 */
int vrtk_bool_update(struct vrtk_bool *b, float dt);

#endif	/* FLTBOOL_H_ */
