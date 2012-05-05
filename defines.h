/***************************************************************************
 *              Resinstaller - installs missing GFX for MAXR               *
 *              This file is part of the resinstaller project              *
 *   Copyright (C) 2007, 2008 Eiko Oltmanns                                *
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

#ifndef DefinesH
#define DefinesH

#define VERSION "0.8"

#ifdef __resinstaller__
#define EX
#define ZERO =0
#define ONE =1
#else
#define EX extern
#define ZERO
#define ONE 
#endif

#ifdef WIN32
	#ifndef PATH_DELIMITER
		#define PATH_DELIMITER "\\"
	#endif
	#ifndef TEXT_FILE_LF
		#define TEXT_FILE_LF "\r\n"
	#endif
#else
	#ifndef PATH_DELIMITER
		#define PATH_DELIMITER "/"
	#endif
	#ifndef TEXT_FILE_LF
		#define TEXT_FILE_LF "\n"
	#endif
#endif

#endif // DefinesH
