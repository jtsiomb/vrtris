#ifndef GAMEINP_H_
#define GAMEINP_H_

enum {
	GINP_LEFT	= 1,
	GINP_RIGHT	= 2,
	GINP_UP		= 4,
	GINP_DOWN	= 8,
	GINP_ROTATE	= 16,
	GINP_PAUSE	= 32
};
#define GINP_ALL \
	(GINP_LEFT | GINP_RIGHT | GINP_UP | GINP_DOWN | GINP_ROTATE | GINP_PAUSE)

#define GINP_PRESS(bn)		((ginp_bnstate & (bn)) && (ginp_bndelta & (bn)))
#define GINP_RELEASE(bn)	((ginp_bnstate & (bn)) == 0 && (ginp_bndelta & (bn)))

unsigned int ginp_bnstate, ginp_bndelta;

void ginp_repeat(int start, int rep, unsigned int mask);

void update_ginp(void);


#endif	/* GAMEINP_H_ */
