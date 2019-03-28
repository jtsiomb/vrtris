#ifndef FLTBOOL_H_
#define FLTBOOL_H_

#define VRTK_BOOL_DEF_SPEED	2.0

struct vrtk_bool {
	int value;
	float fval;
	int dsign;	/* 0: not changing, -1: decreasing, 1: increasing */
	float speed;
};

void vrtk_bool_init(struct vrtk_bool *b, int value);
void vrtk_bool_set(struct vrtk_bool *b, int value);
void vrtk_bool_change(struct vrtk_bool *b, int value);

void vrtk_bool_update(struct vrtk_bool *b, float dt);

#endif	/* FLTBOOL_H_ */
