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

#include <iostream>
#include <string>
#include "resinstaller.h"

int copyFile( string source, string dest )
{
	long int size;
	unsigned char* buffer;
	FILE *sourceFile, *destFile;

	sourceFile = fopen ( source.c_str(), "rb" );
	if ( sourceFile == NULL )
	{
		return 0;
	}

	destFile = fopen ( dest.c_str(), "wb" );
	if ( destFile == NULL )
	{
		return 0;
	}

	fseek( sourceFile, 0, SEEK_END );
	size = ftell( sourceFile );

	buffer = (unsigned char*) malloc( size );
	if ( buffer == NULL )
	{
		return 0;
	}

	fseek( sourceFile, 0, SEEK_SET);
	fread( buffer, 1, size, sourceFile );

	fwrite( buffer, 1, size, destFile );

	free ( buffer );
	fclose( sourceFile );
	fclose( destFile );

	return 1;
}
