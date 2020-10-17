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
#ifndef LOGGER_H_
#define LOGGER_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

void set_log_file(const char *fname);

void info_log(const char *fmt, ...);
void warning_log(const char *fmt, ...);
void error_log(const char *fmt, ...);
void fatal_log(const char *fmt, ...);
void debug_log(const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#endif	// LOGGER_H_
