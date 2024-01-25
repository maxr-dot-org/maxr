/***************************************************************************
 *      Mechanized Assault and Exploration Reloaded Projectfile            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "unifonts.h"

#include "SDLutility/drawing.h"
#include "SDLutility/uniquesurface.h"
#include "resources/pcx.h"
#include "settings.h"
#include "utility/color.h"
#include "utility/log.h"
#include "utility/position.h"
#include "utility/string/trim.h"
#include "utility/string/utf-8.h"

#include <filesystem>

#if 1
/*
 * The following stuff is part of the GNU LIBICONV Library.
 * Copyright (C) 1999-2001 Free Software Foundation, Inc.
 *
 * The GNU LIBICONV Library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * The pages to assign the iso glyphes to their unicode matches have
 * been taken from the "libiconv" project.
 * The author is "Bruno Haible" <bruno@clisp.org>
 * To get more information about libiconv vist
 * http://www.gnu.org/software/libiconv/
 */

static const unsigned short iso8859_2_2uni[96] =
{
	/* 0xa0 */
	0x00a0, 0x0104, 0x02d8, 0x0141, 0x00a4, 0x013d, 0x015a, 0x00a7,
	0x00a8, 0x0160, 0x015e, 0x0164, 0x0179, 0x00ad, 0x017d, 0x017b,
	/* 0xb0 */
	0x00b0, 0x0105, 0x02db, 0x0142, 0x00b4, 0x013e, 0x015b, 0x02c7,
	0x00b8, 0x0161, 0x015f, 0x0165, 0x017a, 0x02dd, 0x017e, 0x017c,
	/* 0xc0 */
	0x0154, 0x00c1, 0x00c2, 0x0102, 0x00c4, 0x0139, 0x0106, 0x00c7,
	0x010c, 0x00c9, 0x0118, 0x00cb, 0x011a, 0x00cd, 0x00ce, 0x010e,
	/* 0xd0 */
	0x0110, 0x0143, 0x0147, 0x00d3, 0x00d4, 0x0150, 0x00d6, 0x00d7,
	0x0158, 0x016e, 0x00da, 0x0170, 0x00dc, 0x00dd, 0x0162, 0x00df,
	/* 0xe0 */
	0x0155, 0x00e1, 0x00e2, 0x0103, 0x00e4, 0x013a, 0x0107, 0x00e7,
	0x010d, 0x00e9, 0x0119, 0x00eb, 0x011b, 0x00ed, 0x00ee, 0x010f,
	/* 0xf0 */
	0x0111, 0x0144, 0x0148, 0x00f3, 0x00f4, 0x0151, 0x00f6, 0x00f7,
	0x0159, 0x016f, 0x00fa, 0x0171, 0x00fc, 0x00fd, 0x0163, 0x02d9,
};

static const unsigned short iso8859_3_2uni[96] =
{
	/* 0xa0 */
	0x00a0, 0x0126, 0x02d8, 0x00a3, 0x00a4, 0xfffd, 0x0124, 0x00a7,
	0x00a8, 0x0130, 0x015e, 0x011e, 0x0134, 0x00ad, 0xfffd, 0x017b,
	/* 0xb0 */
	0x00b0, 0x0127, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x0125, 0x00b7,
	0x00b8, 0x0131, 0x015f, 0x011f, 0x0135, 0x00bd, 0xfffd, 0x017c,
	/* 0xc0 */
	0x00c0, 0x00c1, 0x00c2, 0xfffd, 0x00c4, 0x010a, 0x0108, 0x00c7,
	0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf,
	/* 0xd0 */
	0xfffd, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x0120, 0x00d6, 0x00d7,
	0x011c, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x016c, 0x015c, 0x00df,
	/* 0xe0 */
	0x00e0, 0x00e1, 0x00e2, 0xfffd, 0x00e4, 0x010b, 0x0109, 0x00e7,
	0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef,
	/* 0xf0 */
	0xfffd, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x0121, 0x00f6, 0x00f7,
	0x011d, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x016d, 0x015d, 0x02d9,
};

static const unsigned short iso8859_4_2uni[96] =
{
	/* 0xa0 */
	0x00a0, 0x0104, 0x0138, 0x0156, 0x00a4, 0x0128, 0x013b, 0x00a7,
	0x00a8, 0x0160, 0x0112, 0x0122, 0x0166, 0x00ad, 0x017d, 0x00af,
	/* 0xb0 */
	0x00b0, 0x0105, 0x02db, 0x0157, 0x00b4, 0x0129, 0x013c, 0x02c7,
	0x00b8, 0x0161, 0x0113, 0x0123, 0x0167, 0x014a, 0x017e, 0x014b,
	/* 0xc0 */
	0x0100, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x012e,
	0x010c, 0x00c9, 0x0118, 0x00cb, 0x0116, 0x00cd, 0x00ce, 0x012a,
	/* 0xd0 */
	0x0110, 0x0145, 0x014c, 0x0136, 0x00d4, 0x00d5, 0x00d6, 0x00d7,
	0x00d8, 0x0172, 0x00da, 0x00db, 0x00dc, 0x0168, 0x016a, 0x00df,
	/* 0xe0 */
	0x0101, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x012f,
	0x010d, 0x00e9, 0x0119, 0x00eb, 0x0117, 0x00ed, 0x00ee, 0x012b,
	/* 0xf0 */
	0x0111, 0x0146, 0x014d, 0x0137, 0x00f4, 0x00f5, 0x00f6, 0x00f7,
	0x00f8, 0x0173, 0x00fa, 0x00fb, 0x00fc, 0x0169, 0x016b, 0x02d9,
};

static const unsigned short iso8859_5_2uni[96] =
{
	/* 0xa0 */
	0x00a0, 0x0401, 0x0402, 0x0403, 0x0404, 0x0405, 0x0406, 0x0407,
	0x0408, 0x0409, 0x040a, 0x040b, 0x040c, 0x00ad, 0x040e, 0x040f,
	/* 0xb0 */
	0x0410, 0x0411, 0x0412, 0x0413, 0x0414, 0x0415, 0x0416, 0x0417,
	0x0418, 0x0419, 0x041a, 0x041b, 0x041c, 0x041d, 0x041e, 0x041f,
	/* 0xc0 */
	0x0420, 0x0421, 0x0422, 0x0423, 0x0424, 0x0425, 0x0426, 0x0427,
	0x0428, 0x0429, 0x042a, 0x042b, 0x042c, 0x042d, 0x042e, 0x042f,
	/* 0xd0 */
	0x0430, 0x0431, 0x0432, 0x0433, 0x0434, 0x0435, 0x0436, 0x0437,
	0x0438, 0x0439, 0x043a, 0x043b, 0x043c, 0x043d, 0x043e, 0x043f,
	/* 0xe0 */
	0x0440, 0x0441, 0x0442, 0x0443, 0x0444, 0x0445, 0x0446, 0x0447,
	0x0448, 0x0449, 0x044a, 0x044b, 0x044c, 0x044d, 0x044e, 0x044f,
	/* 0xf0 */
	0x2116, 0x0451, 0x0452, 0x0453, 0x0454, 0x0455, 0x0456, 0x0457,
	0x0458, 0x0459, 0x045a, 0x045b, 0x045c, 0x00a7, 0x045e, 0x045f,
};

static const unsigned short iso8859_6_2uni[96] =
{
	/* 0xa0 */
	0x00a0, 0xfffd, 0xfffd, 0xfffd, 0x00a4, 0xfffd, 0xfffd, 0xfffd,
	0xfffd, 0xfffd, 0xfffd, 0xfffd, 0x060c, 0x00ad, 0xfffd, 0xfffd,
	/* 0xb0 */
	0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd,
	0xfffd, 0xfffd, 0xfffd, 0x061b, 0xfffd, 0xfffd, 0xfffd, 0x061f,
	/* 0xc0 */
	0xfffd, 0x0621, 0x0622, 0x0623, 0x0624, 0x0625, 0x0626, 0x0627,
	0x0628, 0x0629, 0x062a, 0x062b, 0x062c, 0x062d, 0x062e, 0x062f,
	/* 0xd0 */
	0x0630, 0x0631, 0x0632, 0x0633, 0x0634, 0x0635, 0x0636, 0x0637,
	0x0638, 0x0639, 0x063a, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd,
	/* 0xe0 */
	0x0640, 0x0641, 0x0642, 0x0643, 0x0644, 0x0645, 0x0646, 0x0647,
	0x0648, 0x0649, 0x064a, 0x064b, 0x064c, 0x064d, 0x064e, 0x064f,
	/* 0xf0 */
	0x0650, 0x0651, 0x0652, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd,
	0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd,
};

static const unsigned short iso8859_7_2uni[96] =
{
	/* 0xa0 */
	0x00a0, 0x2018, 0x2019, 0x00a3, 0x20ac, 0x20af, 0x00a6, 0x00a7,
	0x00a8, 0x00a9, 0x037a, 0x00ab, 0x00ac, 0x00ad, 0xfffd, 0x2015,
	/* 0xb0 */
	0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x0384, 0x0385, 0x0386, 0x00b7,
	0x0388, 0x0389, 0x038a, 0x00bb, 0x038c, 0x00bd, 0x038e, 0x038f,
	/* 0xc0 */
	0x0390, 0x0391, 0x0392, 0x0393, 0x0394, 0x0395, 0x0396, 0x0397,
	0x0398, 0x0399, 0x039a, 0x039b, 0x039c, 0x039d, 0x039e, 0x039f,
	/* 0xd0 */
	0x03a0, 0x03a1, 0xfffd, 0x03a3, 0x03a4, 0x03a5, 0x03a6, 0x03a7,
	0x03a8, 0x03a9, 0x03aa, 0x03ab, 0x03ac, 0x03ad, 0x03ae, 0x03af,
	/* 0xe0 */
	0x03b0, 0x03b1, 0x03b2, 0x03b3, 0x03b4, 0x03b5, 0x03b6, 0x03b7,
	0x03b8, 0x03b9, 0x03ba, 0x03bb, 0x03bc, 0x03bd, 0x03be, 0x03bf,
	/* 0xf0 */
	0x03c0, 0x03c1, 0x03c2, 0x03c3, 0x03c4, 0x03c5, 0x03c6, 0x03c7,
	0x03c8, 0x03c9, 0x03ca, 0x03cb, 0x03cc, 0x03cd, 0x03ce, 0xfffd,
};

static const unsigned short iso8859_8_2uni[96] =
{
	/* 0xa0 */
	0x00a0, 0xfffd, 0x00a2, 0x00a3, 0x00a4, 0x00a5, 0x00a6, 0x00a7,
	0x00a8, 0x00a9, 0x00d7, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00af,
	/* 0xb0 */
	0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x00b4, 0x00b5, 0x00b6, 0x00b7,
	0x00b8, 0x00b9, 0x00f7, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0xfffd,
	/* 0xc0 */
	0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd,
	0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd,
	/* 0xd0 */
	0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd,
	0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0xfffd, 0x2017,
	/* 0xe0 */
	0x05d0, 0x05d1, 0x05d2, 0x05d3, 0x05d4, 0x05d5, 0x05d6, 0x05d7,
	0x05d8, 0x05d9, 0x05da, 0x05db, 0x05dc, 0x05dd, 0x05de, 0x05df,
	/* 0xf0 */
	0x05e0, 0x05e1, 0x05e2, 0x05e3, 0x05e4, 0x05e5, 0x05e6, 0x05e7,
	0x05e8, 0x05e9, 0x05ea, 0xfffd, 0xfffd, 0x200e, 0x200f, 0xfffd,
};

static const unsigned short iso8859_9_2uni[48] =
{
	/* 0xd0 */
	0x011e, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x00d7,
	0x00d8, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x0130, 0x015e, 0x00df,
	/* 0xe0 */
	0x00e0, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x00e7,
	0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef,
	/* 0xf0 */
	0x011f, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x00f7,
	0x00f8, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x0131, 0x015f, 0x00ff,
};

static const unsigned short iso8859_10_2uni[96] =
{
	/* 0xa0 */
	0x00a0, 0x0104, 0x0112, 0x0122, 0x012a, 0x0128, 0x0136, 0x00a7,
	0x013b, 0x0110, 0x0160, 0x0166, 0x017d, 0x00ad, 0x016a, 0x014a,
	/* 0xb0 */
	0x00b0, 0x0105, 0x0113, 0x0123, 0x012b, 0x0129, 0x0137, 0x00b7,
	0x013c, 0x0111, 0x0161, 0x0167, 0x017e, 0x2015, 0x016b, 0x014b,
	/* 0xc0 */
	0x0100, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x012e,
	0x010c, 0x00c9, 0x0118, 0x00cb, 0x0116, 0x00cd, 0x00ce, 0x00cf,
	/* 0xd0 */
	0x00d0, 0x0145, 0x014c, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x0168,
	0x00d8, 0x0172, 0x00da, 0x00db, 0x00dc, 0x00dd, 0x00de, 0x00df,
	/* 0xe0 */
	0x0101, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x012f,
	0x010d, 0x00e9, 0x0119, 0x00eb, 0x0117, 0x00ed, 0x00ee, 0x00ef,
	/* 0xf0 */
	0x00f0, 0x0146, 0x014d, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x0169,
	0x00f8, 0x0173, 0x00fa, 0x00fb, 0x00fc, 0x00fd, 0x00fe, 0x0138,
};

// ISO-8859-11 isn't supported yet

// ISO-8859-12 doesn't exists

static const unsigned short iso8859_13_2uni[96] =
{
	/* 0xa0 */
	0x00a0, 0x201d, 0x00a2, 0x00a3, 0x00a4, 0x201e, 0x00a6, 0x00a7,
	0x00d8, 0x00a9, 0x0156, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00c6,
	/* 0xb0 */
	0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x201c, 0x00b5, 0x00b6, 0x00b7,
	0x00f8, 0x00b9, 0x0157, 0x00bb, 0x00bc, 0x00bd, 0x00be, 0x00e6,
	/* 0xc0 */
	0x0104, 0x012e, 0x0100, 0x0106, 0x00c4, 0x00c5, 0x0118, 0x0112,
	0x010c, 0x00c9, 0x0179, 0x0116, 0x0122, 0x0136, 0x012a, 0x013b,
	/* 0xd0 */
	0x0160, 0x0143, 0x0145, 0x00d3, 0x014c, 0x00d5, 0x00d6, 0x00d7,
	0x0172, 0x0141, 0x015a, 0x016a, 0x00dc, 0x017b, 0x017d, 0x00df,
	/* 0xe0 */
	0x0105, 0x012f, 0x0101, 0x0107, 0x00e4, 0x00e5, 0x0119, 0x0113,
	0x010d, 0x00e9, 0x017a, 0x0117, 0x0123, 0x0137, 0x012b, 0x013c,
	/* 0xf0 */
	0x0161, 0x0144, 0x0146, 0x00f3, 0x014d, 0x00f5, 0x00f6, 0x00f7,
	0x0173, 0x0142, 0x015b, 0x016b, 0x00fc, 0x017c, 0x017e, 0x2019,
};

static const unsigned short iso8859_14_2uni[96] =
{
	/* 0xa0 */
	0x00a0, 0x1e02, 0x1e03, 0x00a3, 0x010a, 0x010b, 0x1e0a, 0x00a7,
	0x1e80, 0x00a9, 0x1e82, 0x1e0b, 0x1ef2, 0x00ad, 0x00ae, 0x0178,
	/* 0xb0 */
	0x1e1e, 0x1e1f, 0x0120, 0x0121, 0x1e40, 0x1e41, 0x00b6, 0x1e56,
	0x1e81, 0x1e57, 0x1e83, 0x1e60, 0x1ef3, 0x1e84, 0x1e85, 0x1e61,
	/* 0xc0 */
	0x00c0, 0x00c1, 0x00c2, 0x00c3, 0x00c4, 0x00c5, 0x00c6, 0x00c7,
	0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf,
	/* 0xd0 */
	0x0174, 0x00d1, 0x00d2, 0x00d3, 0x00d4, 0x00d5, 0x00d6, 0x1e6a,
	0x00d8, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x00dd, 0x0176, 0x00df,
	/* 0xe0 */
	0x00e0, 0x00e1, 0x00e2, 0x00e3, 0x00e4, 0x00e5, 0x00e6, 0x00e7,
	0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef,
	/* 0xf0 */
	0x0175, 0x00f1, 0x00f2, 0x00f3, 0x00f4, 0x00f5, 0x00f6, 0x1e6b,
	0x00f8, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x00fd, 0x0177, 0x00ff,
};

static const unsigned short iso8859_15_2uni[32] =
{
	/* 0xa0 */
	0x00a0, 0x00a1, 0x00a2, 0x00a3, 0x20ac, 0x00a5, 0x0160, 0x00a7,
	0x0161, 0x00a9, 0x00aa, 0x00ab, 0x00ac, 0x00ad, 0x00ae, 0x00af,
	/* 0xb0 */
	0x00b0, 0x00b1, 0x00b2, 0x00b3, 0x017d, 0x00b5, 0x00b6, 0x00b7,
	0x017e, 0x00b9, 0x00ba, 0x00bb, 0x0152, 0x0153, 0x0178, 0x00bf,
};

static const unsigned short iso8859_16_2uni[96] =
{
	/* 0xa0 */
	0x00a0, 0x0104, 0x0105, 0x0141, 0x20ac, 0x201e, 0x0160, 0x00a7,
	0x0161, 0x00a9, 0x0218, 0x00ab, 0x0179, 0x00ad, 0x017a, 0x017b,
	/* 0xb0 */
	0x00b0, 0x00b1, 0x010c, 0x0142, 0x017d, 0x201d, 0x00b6, 0x00b7,
	0x017e, 0x010d, 0x0219, 0x00bb, 0x0152, 0x0153, 0x0178, 0x017c,
	/* 0xc0 */
	0x00c0, 0x00c1, 0x00c2, 0x0102, 0x00c4, 0x0106, 0x00c6, 0x00c7,
	0x00c8, 0x00c9, 0x00ca, 0x00cb, 0x00cc, 0x00cd, 0x00ce, 0x00cf,
	/* 0xd0 */
	0x0110, 0x0143, 0x00d2, 0x00d3, 0x00d4, 0x0150, 0x00d6, 0x015a,
	0x0170, 0x00d9, 0x00da, 0x00db, 0x00dc, 0x0118, 0x021a, 0x00df,
	/* 0xe0 */
	0x00e0, 0x00e1, 0x00e2, 0x0103, 0x00e4, 0x0107, 0x00e6, 0x00e7,
	0x00e8, 0x00e9, 0x00ea, 0x00eb, 0x00ec, 0x00ed, 0x00ee, 0x00ef,
	/* 0xf0 */
	0x0111, 0x0144, 0x00f2, 0x00f3, 0x00f4, 0x0151, 0x00f6, 0x015b,
	0x0171, 0x00f9, 0x00fa, 0x00fb, 0x00fc, 0x0119, 0x021b, 0x00ff,
};

#endif

//------------------------------------------------------------------------------
cUnicodeFont::cUnicodeFont()
{
	// load all existing fonts.
	// If there will be added some, they have also to be added here!
	loadChars (eUnicodeFontCharset::Iso8559_ALL, eUnicodeFontType::LatinNormal);
	loadChars (eUnicodeFontCharset::Iso8559_1, eUnicodeFontType::LatinNormal);
	loadChars (eUnicodeFontCharset::Iso8559_2, eUnicodeFontType::LatinNormal);
	loadChars (eUnicodeFontCharset::Iso8559_5, eUnicodeFontType::LatinNormal);

	loadChars (eUnicodeFontCharset::Iso8559_ALL, eUnicodeFontType::LatinNormalRed);
	loadChars (eUnicodeFontCharset::Iso8559_1, eUnicodeFontType::LatinNormalRed);
	loadChars (eUnicodeFontCharset::Iso8559_2, eUnicodeFontType::LatinNormalRed);
	loadChars (eUnicodeFontCharset::Iso8559_5, eUnicodeFontType::LatinNormalRed);

	loadChars (eUnicodeFontCharset::Iso8559_ALL, eUnicodeFontType::LatinBig);
	loadChars (eUnicodeFontCharset::Iso8559_1, eUnicodeFontType::LatinBig);
	loadChars (eUnicodeFontCharset::Iso8559_2, eUnicodeFontType::LatinBig);
	loadChars (eUnicodeFontCharset::Iso8559_5, eUnicodeFontType::LatinBig);

	loadChars (eUnicodeFontCharset::Iso8559_ALL, eUnicodeFontType::LatinBigGold);
	loadChars (eUnicodeFontCharset::Iso8559_1, eUnicodeFontType::LatinBigGold);
	loadChars (eUnicodeFontCharset::Iso8559_2, eUnicodeFontType::LatinBigGold);

	loadChars (eUnicodeFontCharset::Iso8559_ALL, eUnicodeFontType::LatinSmallWhite);
	loadChars (eUnicodeFontCharset::Iso8559_1, eUnicodeFontType::LatinSmallWhite);
	loadChars (eUnicodeFontCharset::Iso8559_2, eUnicodeFontType::LatinSmallWhite);
	loadChars (eUnicodeFontCharset::Iso8559_5, eUnicodeFontType::LatinSmallWhite);

	loadChars (eUnicodeFontCharset::Iso8559_ALL, eUnicodeFontType::LatinSmallRed);
	loadChars (eUnicodeFontCharset::Iso8559_1, eUnicodeFontType::LatinSmallRed);
	loadChars (eUnicodeFontCharset::Iso8559_2, eUnicodeFontType::LatinSmallRed);
	loadChars (eUnicodeFontCharset::Iso8559_5, eUnicodeFontType::LatinSmallRed);

	loadChars (eUnicodeFontCharset::Iso8559_ALL, eUnicodeFontType::LatinSmallGreen);
	loadChars (eUnicodeFontCharset::Iso8559_1, eUnicodeFontType::LatinSmallGreen);
	loadChars (eUnicodeFontCharset::Iso8559_2, eUnicodeFontType::LatinSmallGreen);
	loadChars (eUnicodeFontCharset::Iso8559_5, eUnicodeFontType::LatinSmallGreen);

	loadChars (eUnicodeFontCharset::Iso8559_ALL, eUnicodeFontType::LatinSmallYellow);
	loadChars (eUnicodeFontCharset::Iso8559_1, eUnicodeFontType::LatinSmallYellow);
	loadChars (eUnicodeFontCharset::Iso8559_2, eUnicodeFontType::LatinSmallYellow);
	loadChars (eUnicodeFontCharset::Iso8559_5, eUnicodeFontType::LatinSmallYellow);
}

//------------------------------------------------------------------------------
void cUnicodeFont::loadChars (eUnicodeFontCharset charset, eUnicodeFontType fonttype)
{
	UniqueSurface surface (loadCharsetSurface (charset, fonttype));
	if (!surface)
	{
		// LOG: error while loading font
		return;
	}
	UniqueSurface (*pchars)[0xFFFF] = getFontTypeSurfaces (fonttype);
	if (!pchars)
	{
		// LOG: error while loading font
		return;
	}
	UniqueSurface (&chars)[0xFFFF] = *pchars;
	const unsigned short* iso8859_to_uni = getIsoPage (charset);

	int highcount;
	if (charset == eUnicodeFontCharset::Iso8559_ALL)
		highcount = 16;
	else
		highcount = 6;

	int cellW = surface->w / 16;
	int cellH = surface->h / highcount;
	int currentChar = 0;
	int pX = 0;
	int pY = 0;
	const auto limitColor = SDL_MapRGB (surface->format, 0xFF, 0, 0xFF);

	for (int rows = 0; rows < highcount; rows++)
	{
		//go through the cols
		for (int cols = 0; cols < 16; cols++)
		{
			// write each cell position and size into array
			SDL_Rect Rect;
			Rect.x = cellW * cols;
			Rect.y = cellH * rows;
			Rect.h = cellH;
			Rect.w = cellW;

			// go through pixels to find offset x
			for (int pCol = 0; pCol < cellH; pCol++)
			{
				for (int pRow = 0; pRow < cellH; pRow++)
				{
					pX = (cellW * cols) + pCol;
					pY = (cellH * rows) + pRow;

					if (getPixel (*surface, cPosition (pX, pY)) != limitColor)
					{
						// offset
						Rect.x = pX;
						pCol = cellW; // break loop
						pRow = cellH;
					}
				}
			}
			// go through pixel to find offset w
			for (int pCol_w = cellW - 1; pCol_w >= 0; pCol_w--)
			{
				for (int pRow_w = 0; pRow_w < cellH; pRow_w++)
				{
					pX = (cellW * cols) + pCol_w;
					pY = (cellH * rows) + pRow_w;

					if (getPixel (*surface, cPosition (pX, pY)) != limitColor)
					{
						Rect.w = (pX - Rect.x) + 1;
						pCol_w = -1; // break loop
						pRow_w = cellH;
					}
				}
			}

			// get the unicode place of the character
			int unicodeplace = 0;
			if (iso8859_to_uni == nullptr)
			{
				if (charset == eUnicodeFontCharset::Iso8559_ALL)
					unicodeplace = currentChar;
				else if (charset == eUnicodeFontCharset::Iso8559_1)
					unicodeplace = currentChar + 128 + 2 * 16;
			}
			else
				unicodeplace = iso8859_to_uni[currentChar];
			chars[unicodeplace] = UniqueSurface (SDL_CreateRGBSurface (0, Rect.w, Rect.h, 32, 0, 0, 0, 0));

			SDL_FillRect (chars[unicodeplace].get(), nullptr, 0xFF00FF);
			SDL_BlitSurface (surface.get(), &Rect, chars[unicodeplace].get(), nullptr);
			SDL_SetColorKey (chars[unicodeplace].get(), SDL_TRUE, 0xFF00FF);

			// change color for some fonts
			switch (fonttype)
			{
				case eUnicodeFontType::LatinNormalRed:
					replaceColor (*chars[unicodeplace], cRgbColor (214, 189, 148), cRgbColor (250, 0, 0));
					replaceColor (*chars[unicodeplace], cRgbColor (140, 132, 132), cRgbColor (163, 0, 0));
					break;
				case eUnicodeFontType::LatinSmallRed:
					replaceColor (*chars[unicodeplace], cRgbColor (240, 216, 184), cRgbColor (230, 0, 0));
					break;
				case eUnicodeFontType::LatinSmallGreen:
					replaceColor (*chars[unicodeplace], cRgbColor (240, 216, 184), cRgbColor (4, 174, 4));
					break;
				case eUnicodeFontType::LatinSmallYellow:
					replaceColor (*chars[unicodeplace], cRgbColor (240, 216, 184), cRgbColor (219, 222, 0));
					break;
				case eUnicodeFontType::LatinNormal:
				case eUnicodeFontType::LatinBig:
				case eUnicodeFontType::LatinBigGold:
				case eUnicodeFontType::LatinSmallWhite:
					break;
			}

			// goto next character
			currentChar++;
		}
	}
}

//------------------------------------------------------------------------------
const cUnicodeFont::FontTypeSurfaces*
cUnicodeFont::getFontTypeSurfaces (eUnicodeFontType const fonttype) const
{
	switch (fonttype)
	{
		case eUnicodeFontType::LatinNormal: return &charsNormal;
		case eUnicodeFontType::LatinNormalRed: return &charsNormalRed;
		case eUnicodeFontType::LatinBig: return &charsBig;
		case eUnicodeFontType::LatinBigGold: return &charsBigGold;
		case eUnicodeFontType::LatinSmallWhite: return &charsSmallWhite;
		case eUnicodeFontType::LatinSmallRed: return &charsSmallRed;
		case eUnicodeFontType::LatinSmallGreen: return &charsSmallGreen;
		case eUnicodeFontType::LatinSmallYellow: return &charsSmallYellow;
	}
	return nullptr;
}

//------------------------------------------------------------------------------
cUnicodeFont::FontTypeSurfaces*
cUnicodeFont::getFontTypeSurfaces (eUnicodeFontType fonttype)
{
	return const_cast<FontTypeSurfaces*> (const_cast<const cUnicodeFont*> (this)->getFontTypeSurfaces (fonttype));
}

//------------------------------------------------------------------------------
UniqueSurface cUnicodeFont::loadCharsetSurface (eUnicodeFontCharset charset,
                                              eUnicodeFontType fonttype)
{
	// build the filename from the information
	std::string filename = "latin_";
	switch (fonttype)
	{
		case eUnicodeFontType::LatinNormal:
		case eUnicodeFontType::LatinNormalRed:
			filename += "normal";
			break;
		case eUnicodeFontType::LatinBig:
			filename += "big";
			break;
		case eUnicodeFontType::LatinBigGold:
			filename += "big_gold";
			break;
		case eUnicodeFontType::LatinSmallWhite:
		case eUnicodeFontType::LatinSmallRed:
		case eUnicodeFontType::LatinSmallGreen:
		case eUnicodeFontType::LatinSmallYellow:
			filename += "small";
			break;
	}
	if (charset != eUnicodeFontCharset::Iso8559_ALL)
	{
		filename += "_iso-8559-";
		// it's important that the enum-numbers are the same
		// as their iso-numbers!
		filename += std::to_string (static_cast<int> (charset));
	}
	filename += ".pcx";

	auto path = cSettings::getInstance().getFontPath() / filename;
	// load the bitmap
	if (std::filesystem::exists (path))
		return LoadPCX (path);
	else
		return nullptr;
}

//------------------------------------------------------------------------------
const unsigned short* cUnicodeFont::getIsoPage (eUnicodeFontCharset charset) const
{
	switch (charset)
	{
		case eUnicodeFontCharset::Iso8559_ALL: return nullptr;
		case eUnicodeFontCharset::Iso8559_1: return nullptr;
		case eUnicodeFontCharset::Iso8559_2: return iso8859_2_2uni;
		case eUnicodeFontCharset::Iso8559_3: return iso8859_3_2uni;
		case eUnicodeFontCharset::Iso8559_4: return iso8859_4_2uni;
		case eUnicodeFontCharset::Iso8559_5: return iso8859_5_2uni;
		case eUnicodeFontCharset::Iso8559_6: return iso8859_6_2uni;
		case eUnicodeFontCharset::Iso8559_7: return iso8859_7_2uni;
		case eUnicodeFontCharset::Iso8559_8: return iso8859_8_2uni;
		case eUnicodeFontCharset::Iso8559_9: return iso8859_9_2uni;
		case eUnicodeFontCharset::Iso8559_10: return iso8859_10_2uni;
		case eUnicodeFontCharset::Iso8559_11: return nullptr;
		case eUnicodeFontCharset::Iso8559_13: return iso8859_13_2uni;
		case eUnicodeFontCharset::Iso8559_14: return iso8859_14_2uni;
		case eUnicodeFontCharset::Iso8559_15: return iso8859_15_2uni;
		case eUnicodeFontCharset::Iso8559_16: return iso8859_16_2uni;
		default:
			//LOG: unknown iso format
			break;
	}
	return nullptr;
}

//------------------------------------------------------------------------------
void cUnicodeFont::showText (int x, int y, const std::string& text, eUnicodeFontType fonttype)
{
	std::string sText (text);
	int offX = x;
	int offY = y;
	int iSpace = 0;
	const UniqueSurface (&chars)[0xFFFF] = *getFontTypeSurfaces (fonttype);

	// make sure only upper characters are read for the small fonts
	// since we don't support lower chars on the small fonts
	switch (fonttype)
	{
		case eUnicodeFontType::LatinSmallGreen:
		case eUnicodeFontType::LatinSmallRed:
		case eUnicodeFontType::LatinSmallWhite:
		case eUnicodeFontType::LatinSmallYellow:
			for (char& c : sText)
				c = toupper (c);
			iSpace = 1;
			break;
		case eUnicodeFontType::LatinNormal:
		case eUnicodeFontType::LatinNormalRed:
		case eUnicodeFontType::LatinBig:
		case eUnicodeFontType::LatinBigGold:
			break;
	}

	// decode the UTF-8 String:
	utf8::for_each (sText, [&] (std::uint32_t c) {
		switch (c)
		{
			case ' ': // is space?
			{
				if (chars['a'].get()) offX += chars['a']->w;
				break;
			}
			case '\n': // is new line?
			{
				offY += getFontHeight (fonttype);
				offX = x;
				break;
			}
			case '\r': // ignore - is breakline in file
			{
				break;
			}
			default:
			{
				if (std::size (chars) <= c)
				{
					Log.warn ("No display character to display : '" + utf8::to_utf8 (c) + "'");
				}
				else if (chars[c] != nullptr)
				{
					SDL_Rect rTmp = {Sint16 (offX), Sint16 (offY), 16, 16};
					SDL_BlitSurface (chars[c].get(), nullptr, surface, &rTmp);

					// move one px forward for space between signs
					offX += chars[c]->w + iSpace;
				}
			}
		}
	});
	if (cSettings::getInstance().isDebug() && surface->w < offX)
	{
		Log.warn ("Cannot display entirely: '" + text + "'");
		Log.debug ("surface weight: " + std::to_string (surface->w) + " < offX = " + std::to_string (offX));
	}
}

//------------------------------------------------------------------------------
void cUnicodeFont::showText (const cPosition& position, const std::string& text, eUnicodeFontType fonttype)
{
	showText (position.x(), position.y(), text, fonttype);
}

//------------------------------------------------------------------------------
int cUnicodeFont::drawWithBreakLines (SDL_Rect rDest, const std::string& text, eUnicodeFontType fonttype)
{
	std::string sText (text);
	std::string drawString = "";

	while (getTextWide (sText, fonttype) > rDest.w)
	{
		// get the position of the end of as many words
		// from the text as fit in rDest.w
		size_t pos = 0, lastPos = 0;
		do
		{
			lastPos = pos;
			pos = sText.find (" ", pos + 1);
		} while (getTextWide (sText.substr (0, pos), fonttype) < rDest.w && pos != std::string::npos);

		// get the words.
		// If there was no " " in the text we get the whole text string
		if (lastPos != 0)
			drawString = sText.substr (0, lastPos);
		else
			drawString = sText;

		// if there is only one word in the string it is possible
		// that this word is to long.
		// we will check this, and cut it if necessary
		while (getTextWide (drawString, fonttype) > rDest.w)
		{
			std::string stringPart = drawString;

			// delete as many chars as it is needed to fit into the line
			while (getTextWide (stringPart, fonttype) + getTextWide ("-", fonttype) > rDest.w)
				utf8::pop_back (stringPart);
			stringPart += "-";

			// show the part of the word
			showText (rDest.x, rDest.y, stringPart, fonttype);
			rDest.y += getFontHeight (fonttype);

			// erase the part from the line and from the hole text
			drawString.erase (0, stringPart.length() - 1);
			sText.erase (0, stringPart.length() - 1);
		}

		// draw the rest of the line
		showText (rDest.x, rDest.y, drawString, fonttype);
		rDest.y += getFontHeight (fonttype);

		sText.erase (0, drawString.length());
		if (lastPos != 0) sText.erase (0, 1);
	}

	// draw the rest of the text
	showText (rDest.x, rDest.y, sText, fonttype);
	rDest.y += getFontHeight (fonttype);

	return rDest.y;
}

//------------------------------------------------------------------------------
int cUnicodeFont::showTextAsBlock (SDL_Rect rDest, const std::string& text, eUnicodeFontType fonttype)
{
	std::string sText (text);
	size_t k;

	do
	{
		// erase all invalid formatted breaklines
		// like we may get them from translation
		k = sText.find ("\\n");

		if (k != std::string::npos)
		{
			sText.replace (k, 2, "\n");
		}
	} while (k != std::string::npos);

	do
	{
		// erase all blanks > 2
		k = sText.find ("  ");
		// IMPORTANT: _two_ blanks!
		// don't change this or this will become an endless loop

		if (k != std::string::npos)
		{
			sText.erase (k, 1);
		}
	} while (k != std::string::npos);

	// support of linebreaks: snip text at linebreaks,
	// do the auto linebreak for first part and proceed with second part
	do
	{
		// search and replace \n since we want a blocktext
		// - no manual breaklines allowed
		k = sText.find ("\n");

		if (k != std::string::npos)
		{
			std::string sTmp = sText;

			// delete everything before and including linebreak \n
			sText.erase (0, k + 1);
			// delete everything after \n
			sTmp.erase (k, sTmp.size());

			// draw first part of text and proceed searching for breaklines
			rDest.y = drawWithBreakLines (rDest, sTmp, fonttype);
			// += getFontHeight (eBitmapFontType); //add newline for each breakline
		}
	} while (k != std::string::npos);

	// draw rest of text
	return drawWithBreakLines (rDest, sText, fonttype);
}

//------------------------------------------------------------------------------
void cUnicodeFont::showTextCentered (int x, int y, const std::string& sText, eUnicodeFontType fonttype)
{
	SDL_Rect rTmp = getTextSize (sText, fonttype);
	showText (x - rTmp.w / 2, y, sText, fonttype);
}

//------------------------------------------------------------------------------
void cUnicodeFont::showTextCentered (const cPosition& position, const std::string& sText, eUnicodeFontType fonttype)
{
	showTextCentered (position.x(), position.y(), sText, fonttype);
}

//------------------------------------------------------------------------------
int cUnicodeFont::getTextWide (const std::string& sText, eUnicodeFontType fonttype) const
{
	SDL_Rect rTmp = getTextSize (sText, fonttype);
	return rTmp.w;
}

//------------------------------------------------------------------------------
SDL_Rect cUnicodeFont::getTextSize (const std::string& text, eUnicodeFontType fonttype) const
{
	std::string sText (text);
	int iSpace = 0;
	const UniqueSurface (&chars)[0xFFFF] = *getFontTypeSurfaces (fonttype);
	SDL_Rect rTmp = {0, 0, 0, 0};

	// make sure only upper characters are read for the small fonts
	// since we don't support lower chars on the small fonts
	switch (fonttype)
	{
		case eUnicodeFontType::LatinSmallGreen:
		case eUnicodeFontType::LatinSmallRed:
		case eUnicodeFontType::LatinSmallWhite:
		case eUnicodeFontType::LatinSmallYellow:
			for (char& c : sText)
				c = toupper (c);
			iSpace = 1;
			break;
		case eUnicodeFontType::LatinNormal:
		case eUnicodeFontType::LatinNormalRed:
		case eUnicodeFontType::LatinBig:
		case eUnicodeFontType::LatinBigGold:
			break;
	}

	// decode the UTF-8 String:
	utf8::for_each (sText, [&] (std::uint32_t c) {
		switch (c)
		{
			case ' ': // is space?
			{
				// we will use the wight of the 'a' for spaces
				if (chars['a'].get()) rTmp.w += chars['a']->w;
				break;
			}
			case '\n': // is new line?
			{
				rTmp.h += getFontHeight (fonttype);
				break;
			}
			case '\r': //ignore - is breakline in file
			{
				break;
			}
			default:
			{
				if (chars[c] != nullptr)
				{
					rTmp.w += chars[c]->w + iSpace;
					rTmp.h = chars[c]->h;
				}
				break;
			}
		}
	});
	return rTmp;
}

//------------------------------------------------------------------------------
int cUnicodeFont::getFontHeight (eUnicodeFontType fonttype) const
{
	const UniqueSurface (&chars)[0xFFFF] = *getFontTypeSurfaces (fonttype);
	// we will return the height of the first character in the list
	for (const auto& surface : chars)
	{
		if (surface != nullptr) return surface->h;
	}
	return 0;
}

//------------------------------------------------------------------------------
/* static */ eUnicodeFontSize cUnicodeFont::getFontSize (eUnicodeFontType fonttype)
{
	switch (fonttype)
	{
		default:
		case eUnicodeFontType::LatinNormal:
		case eUnicodeFontType::LatinNormalRed:
			return eUnicodeFontSize::Normal;
		case eUnicodeFontType::LatinBig:
		case eUnicodeFontType::LatinBigGold:
			return eUnicodeFontSize::Big;
		case eUnicodeFontType::LatinSmallWhite:
		case eUnicodeFontType::LatinSmallRed:
		case eUnicodeFontType::LatinSmallGreen:
		case eUnicodeFontType::LatinSmallYellow:
			return eUnicodeFontSize::Small;
	}
}

//------------------------------------------------------------------------------
std::string cUnicodeFont::shortenStringToSize (const std::string& str, int size, eUnicodeFontType fonttype) const
{
	std::string res (str);

	if (getTextWide (res, fonttype) > size)
	{
		while (getTextWide (res + ".", fonttype) > size)
		{
			utf8::pop_back (res);
		}
		res += ".";
		if (cSettings::getInstance().isDebug())
		{
			Log.warn ("shorten string : '" + str + "' to '" + res + "'");
		}
	}
	return res;
}

//------------------------------------------------------------------------------
int cUnicodeFont::getUnicodeCharacterWidth (Uint16 unicodeCharacter, eUnicodeFontType fonttype) const
{
	const UniqueSurface (&chars)[0xFFFF] = *getFontTypeSurfaces (fonttype);

	// make sure only upper characters are read for the small fonts
	// since we don't support lower chars on the small fonts
	int space;
	switch (fonttype)
	{
		case eUnicodeFontType::LatinSmallGreen:
		case eUnicodeFontType::LatinSmallRed:
		case eUnicodeFontType::LatinSmallWhite:
		case eUnicodeFontType::LatinSmallYellow:
			unicodeCharacter = toupper (unicodeCharacter);
			space = 1;
			break;
		default:
		case eUnicodeFontType::LatinNormal:
		case eUnicodeFontType::LatinNormalRed:
		case eUnicodeFontType::LatinBig:
		case eUnicodeFontType::LatinBigGold:
			space = 0;
			break;
	}

	if (unicodeCharacter == ' ') unicodeCharacter = 'a'; // we use the length of 'a' for the length of a space

	if (chars[unicodeCharacter] != nullptr)
	{
		return chars[unicodeCharacter]->w + space;
	}
	return 0;
}

//------------------------------------------------------------------------------
std::vector<std::string> cUnicodeFont::breakText (const std::string& text, int maximalWidth, eUnicodeFontType fontType) const
{
	const auto isSpace = [] (char c) {
		return c == ' ' || c == '\f' || c == '\r' || c == '\t' || c == '\v';
	};
	std::vector<std::string> lines;
	auto it = text.begin();

	while (it != text.end())
	{
		const auto nextLine = std::find (it, text.end(), '\n');
		const auto firstWord = std::find_if_not (it, nextLine, isSpace);
		auto next = std::find_if (it, nextLine, isSpace);

		while (next != nextLine)
		{
			auto candidate = std::find_if (next + 1, nextLine, isSpace);
			auto size = getTextWide ({it, candidate}, fontType);
			if (size > maximalWidth)
			{
				break;
			}
			next = candidate;
		}
		if (firstWord <= next) // handle too long leading space sequence
		{
			lines.emplace_back (it, next);
			trim_right (lines.back());
		}
		it = std::find_if_not (next, nextLine, isSpace);
		if (it == nextLine && it != text.end())
		{
			++it;
		}
	}
	if (!text.empty() && text.back() == '\n') { lines.emplace_back(); }
	return lines;
}

std::unique_ptr<cUnicodeFont> cUnicodeFont::font;
