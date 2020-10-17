/*
vrtris - tetris in virtual reality
Copyright (C) 2019-2020  John Tsiombikas <nuclear@member.fsf.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "optcfg.h"
#include "opt.h"


struct options def_opt = { 1024, 768, 0, "game" };

enum {
	OPTCFG_SIZE,
#ifdef BUILD_VR
	OPTCFG_VR,
#endif
	OPTCFG_FULLSCREEN,
	OPTCFG_VSYNC,
	OPTCFG_WINDOWED,
	OPTCFG_SCREEN,
	OPTCFG_HELP
};

static struct optcfg_option options[] = {
	// short, long, id, desc
	{'s', "size", OPTCFG_SIZE, "window size (WxH)"},
#ifdef BUILD_VR
	{0, "vr", OPTCFG_VR, "enable VR mode"},
#endif
	{'f', "fullscreen", OPTCFG_FULLSCREEN, "run in fullscreen mode"},
	{0, "vsync", OPTCFG_VSYNC, "vsync"},
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

#ifdef BUILD_VR
	case OPTCFG_VR:
		if(is_enabled(oc)) {
			opt.flags |= OPT_VR;
		} else {
			opt.flags &= ~OPT_VR;
		}
		break;
#endif

	case OPTCFG_FULLSCREEN:
		if(is_enabled(oc)) {
			opt.flags |= OPT_FULLSCREEN;
		} else {
			opt.flags &= ~OPT_FULLSCREEN;
		}
		break;

	case OPTCFG_VSYNC:
		if(is_enabled(oc)) {
			opt.flags |= OPT_VSYNC;
		} else {
			opt.flags &= ~OPT_VSYNC;
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
