#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "optcfg.h"
#include "opt.h"


struct options def_opt = { 1024, 768, 0, "game" };

enum {
	OPTCFG_SIZE,
	OPTCFG_VR,
	OPTCFG_FULLSCREEN,
	OPTCFG_WINDOWED,
	OPTCFG_SCREEN,
	OPTCFG_HELP
};

static struct optcfg_option options[] = {
	// short, long, id, desc
	{'s', "size", OPTCFG_SIZE, "window size (WxH)"},
	{0, "vr", OPTCFG_VR, "enable VR mode"},
	{'f', "fullscreen", OPTCFG_FULLSCREEN, "run in fullscreen mode"},
	{'w', "windowed", OPTCFG_WINDOWED, "run in windowed mode"},
	{0, "screen", OPTCFG_SCREEN, "select starting screen"},
	{'h', "help", OPTCFG_HELP, "print usage and exit"},
	OPTCFG_OPTIONS_END
};

static int opt_handler(struct optcfg *oc, int opt, void *cls);
static int arg_handler(struct optcfg *oc, const char *arg, void *cls);

static char *argv0;

int init_options(int argc, char **argv, const char *cfgfile)
{
	struct optcfg *oc;

	/* default options */
	opt = def_opt;
	if(!(opt.start_scr = malloc(strlen(def_opt.start_scr) + 1))) {
		perror("failed to allocate memory");
		return -1;
	}
	strcpy(opt.start_scr, def_opt.start_scr);

	argv0 = argv[0];

	oc = optcfg_init(options);
	optcfg_set_opt_callback(oc, opt_handler, 0);
	optcfg_set_arg_callback(oc, arg_handler, 0);

	if(cfgfile) {
		optcfg_parse_config_file(oc, cfgfile);
	}

	if(argv && optcfg_parse_args(oc, argc, argv) == -1) {
		fprintf(stderr, "invalid option\n");
		optcfg_destroy(oc);
		return -1;
	}

	optcfg_destroy(oc);
	return 0;
}

static int is_enabled(struct optcfg *oc)
{
	int res;
	optcfg_enabled_value(oc, &res);
	return res != 0;
}

static int opt_handler(struct optcfg *oc, int optid, void *cls)
{
	char *valstr;

	switch(optid) {
	case OPTCFG_SIZE:
		valstr = optcfg_next_value(oc);
		if(!valstr || sscanf(valstr, "%dx%d", &opt.width, &opt.height) != 2) {
			fprintf(stderr, "size must be of the form: WIDTHxHEIGHT\n");
			return -1;
		}
		break;

	case OPTCFG_VR:
		if(is_enabled(oc)) {
			opt.flags |= OPT_VR;
		} else {
			opt.flags &= ~OPT_VR;
		}
		break;

	case OPTCFG_FULLSCREEN:
		if(is_enabled(oc)) {
			opt.flags |= OPT_FULLSCREEN;
		} else {
			opt.flags &= ~OPT_FULLSCREEN;
		}
		break;

	case OPTCFG_WINDOWED:
		if(is_enabled(oc)) {
			opt.flags &= ~OPT_FULLSCREEN;
		} else {
			opt.flags |= OPT_FULLSCREEN;
		}
		break;

	case OPTCFG_SCREEN:
		if(!(valstr = optcfg_next_value(oc))) {
			fprintf(stderr, "screen name missing\n");
			return -1;
		}
		free(opt.start_scr);
		if(!(opt.start_scr = malloc(strlen(valstr) + 1))) {
			perror("failed to allocate memory");
			return -1;
		}
		strcpy(opt.start_scr, valstr);
		break;

	case OPTCFG_HELP:
		printf("Usage: %s [options]\nOptions:\n", argv0);
		optcfg_print_options(oc);
		exit(0);
	}
	return 0;
}

static int arg_handler(struct optcfg *oc, const char *arg, void *cls)
{
	fprintf(stderr, "unexpected argument: %s\n", arg);
	return -1;
}
