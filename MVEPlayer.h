/*
    MVEPlayer - Interplay MVE multimedia file player
    Copyright (C) 2008 Jared Livesey

	This file is part of MVEPlayer.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Jared Livesey
    jaredlivesey at yahoo dot com
*/

/*	MVEPlay.h - include file for MVEPlayer function.
	Author: jarli
*/
#ifdef __cplusplus
extern "C"
{
#endif

#ifndef _MVEPLAYER_H
#define _MVEPLAYER_H

/*	define return states */
#define SUCCESS						0x00
#define UNABLE_TO_OPEN_FILE			0x01
#define FILE_TOO_SHORT				0x02
#define NOT_MVE_FILE				0x03
#define CHUNK_READ_FAILED			0x04
#define OPCODE_READ_FAILED			0x05
#define MVE_CORRUPT					0x06
#define SDL_INIT_FAILURE			0x07
#define COULD_NOT_OPEN_AUDIO		0x08
#define AUDIO_FRAME_READ_FAILURE	0x09
#define CANNOT_PROCESS_AUDIO		0x0A
#define READ_FAILURE_IN_COMPAUDIO	0x0B
#define SIXTEEN_BIT_MVE				0X0C

/* player function prototype */
int MVEPlayer(const char *MVEfilename, int dwidth, int dheight, int fullscreen);

#endif /* _MVEPLAYER_H */
#ifdef __cplusplus
}
#endif
