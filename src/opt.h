#ifndef OPT_H_
#define OPT_H_

enum {
	OPT_VR			= 1,
	OPT_FULLSCREEN	= 2
};

struct options {
	int width, height;
	unsigned int flags;
} opt, def_opt;

int init_options(int argc, char **argv, const char *cfgfile);

#endif	/* OPT_H_ */
