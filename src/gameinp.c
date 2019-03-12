#include "gameinp.h"
#include "game.h"

#define NUM_BN	6

static int rep_start, rep_rep;
static long first_press[16], last_press[16];
static unsigned int repmask;


void ginp_repeat(int start, int rep, unsigned int mask)
{
	rep_start = start;
	rep_rep = rep;
	repmask = mask;
}

void update_ginp(void)
{
	static unsigned int prevstate;
	int i;

	ginp_bndelta = ginp_bnstate ^ prevstate;
	prevstate = ginp_bnstate;

	for(i=0; i<NUM_BN; i++) {
		unsigned int bit = 1 << i;
		if(!(bit & repmask)) {
			continue;
		}

		if(ginp_bnstate & bit) {
			if(ginp_bndelta & bit) {
				first_press[i] = time_msec;
			} else {
				if(time_msec - first_press[i] >= rep_start && time_msec - last_press[i] >= rep_rep) {
					ginp_bndelta |= bit;
					last_press[i] = time_msec;
				}
			}
		}
	}
}
