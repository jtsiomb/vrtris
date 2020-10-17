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
#ifndef GAMEINP_H_
#define GAMEINP_H_

enum {
	GINP_LEFT	= 0x0001,
	GINP_RIGHT	= 0x0002,
	GINP_DROP	= 0x0004,
	GINP_DOWN	= 0x0008,
	GINP_ROTATE	= 0x0010,
	GINP_PAUSE	= 0x0020,
	GINP_CENTER = 0x0040,
	GINP_HELP	= 0x0080
};
#define GINP_ALL \
	(GINP_LEFT | GINP_RIGHT | GINP_DROP | GINP_DOWN | GINP_ROTATE | GINP_PAUSE | GINP_HELP)

#define GINP_PRESS(bn)		((ginp_bnstate & (bn)) && (ginp_bndelta & (bn)))
#define GINP_RELEASE(bn)	((ginp_bnstate & (bn)) == 0 && (ginp_bndelta & (bn)))

unsigned int ginp_bnstate, ginp_bndelta;

void ginp_repeat(int start, int rep, unsigned int mask);

void update_ginp(void);


#endif	/* GAMEINP_H_ */
