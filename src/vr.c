#include <math.h>
#include "vr.h"
#include "game.h"
#include "gameinp.h"
#include "opt.h"

#ifdef BUILD_VR
static int vr_initialized;

int init_vr(void)
{
	if(opt.flags & OPT_VR) {
		if(goatvr_init() == -1) {
			return -1;
		}
		goatvr_set_origin_mode(GOATVR_HEAD);

		goatvr_startvr();
		should_swap = goatvr_should_swap();

		if((vrbn_a = goatvr_lookup_button("A")) < 0) {
			vrbn_a = 0;
		}
		if((vrbn_b = goatvr_lookup_button("B")) < 0) {
			vrbn_b = 1;
		}
		if((vrbn_x = goatvr_lookup_button("X")) < 0) {
			vrbn_x = 4;
		}
		if((vrbn_y = goatvr_lookup_button("Y")) < 0) {
			vrbn_y = 5;
		}

		vr_initialized = 1;
	}

	return 0;
}

void shutdown_vr(void)
{
	if(vr_initialized) {
		goatvr_shutdown();
	}
}

void update_vr_input(void)
{
	int num_vr_sticks;

	if(goatvr_invr() && (num_vr_sticks = goatvr_num_sticks()) > 0) {
		float p[2];

		goatvr_stick_pos(0, p);

		if(fabs(p[0]) > fabs(joy_axis[GPAD_LSTICK_X])) {
			joy_axis[GPAD_LSTICK_X] = p[0];
		}
		if(fabs(p[1]) > fabs(joy_axis[GPAD_LSTICK_Y])) {
			joy_axis[GPAD_LSTICK_Y] = -p[1];
		}

		if(goatvr_button_state(vrbn_a)) {
			joy_bnstate |= 1 << GPAD_A;
		}
		if(goatvr_button_state(vrbn_b)) {
			joy_bnstate |= 1 << GPAD_B;
		}
		if(goatvr_button_state(vrbn_x)) {
			joy_bnstate |= 1 << GPAD_X;
		}
		if(goatvr_button_state(vrbn_y)) {
			joy_bnstate |= 1 << GPAD_Y;
		}
		if(goatvr_action(0, GOATVR_ACTION_TRIGGER)) {
			joy_bnstate |= 1 << GPAD_L;
		}
		if(goatvr_action(1, GOATVR_ACTION_TRIGGER)) {
			joy_bnstate |= 1 << GPAD_R;
		}
	}
}

#else
int init_vr(void) { return 0; }
void shutdown_vr(void) {}
void update_vr_input(void) {}
#endif	/* BUILD_VR */
