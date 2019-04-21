#ifndef VR_H_
#define VR_H_

#ifdef BUILD_VR
#include <goatvr.h>

int should_swap;
int vrbn_a, vrbn_b, vrbn_x, vrbn_y;
#endif

int init_vr(void);
void shutdown_vr(void);
void update_vr_input(void);

#endif	/* VR_H_ */
