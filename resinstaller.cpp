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
 
#define __resinstaller__
#include <SDL.h>
#include <string>
#include "defines.h"
#include "resinstaller.h"
#include "converter.h"
#include "pcx.h"
#include "file.h"
#include "wave.h"
#include "ogg_encode.h"


int installVehicleGraphics()
{
	string path;

	iTotalFiles = 808;
	iErrors = 0;
	iInstalledFiles = 0;

	SDL_Rect src_rect, dst_rect;
	char szNum[13];
	char szNum1[13];
	char szNum2[13];
	SDL_Surface *surface, *output;
	cout << "========================================================================\n";
	cout << "Vehicle graphics\n";

	//air_transport
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "air_transport" + PATH_DELIMITER;
	for (int i = 0; i < 8; i++)
	{
		sprintf( szNum, "%d", i);
		copyFileFromRes_rpc("AIRTRANS", path + "img" + szNum + ".pcx", i);
		copyFileFromRes("S_AIRTRA", path + "shw" + szNum + ".pcx", i);
	}
	copyFileFromRes("A_AIRTRN", path + "store.pcx");
	copyFileFromRes("P_AIRTRN", path + "info.pcx");

	//alien_assault
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "alien_assault" + PATH_DELIMITER;
	for (int i = 0; i < 8; i++)
	{
		sprintf( szNum, "%d", i);
		copyFileFromRes_rpc("ALNASGUN", path + "img" + szNum + ".pcx", i);
		copyFileFromRes("S_ALNASG", path + "shw" + szNum + ".pcx", i);
	}
	copyFileFromRes("A_ALNASG", path + "store.pcx");
	copyFileFromRes("P_ALNASG", path + "info.pcx");

	//alien_plane
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "alien_plane" + PATH_DELIMITER;
	for (int i = 0; i < 8; i++)
	{
		sprintf( szNum, "%d", i);
		copyFileFromRes_rpc("ALNPLANE", path + "img" + szNum + ".pcx", i);
		copyFileFromRes("S_ALNPLA", path + "shw" + szNum + ".pcx", i);
	}
	copyFileFromRes("A_ALNPLA", path + "store.pcx");
	copyFileFromRes("P_ALNPLA", path + "info.pcx");

	//alien_ship
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "alien_ship" + PATH_DELIMITER;
	for (int i = 0; i < 8; i++)
	{
		sprintf( szNum, "%d", i);
		copyFileFromRes_rpc("JUGGRNT", path + "img" + szNum + ".pcx", i);
		copyFileFromRes("S_JUGGRN", path + "shw" + szNum + ".pcx", i);
	}
	copyFileFromRes("A_JUGGER", path + "store.pcx");
	copyFileFromRes("P_JUGGER", path + "info.pcx");

	//alien_tank
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "alien_tank" + PATH_DELIMITER;
	for (int i = 0; i < 8; i++)
	{
		sprintf( szNum, "%d", i);
		copyFileFromRes_rpc("ALNTANK", path + "img" + szNum + ".pcx", i);
		copyFileFromRes("S_ALNTAN", path + "shw" + szNum + ".pcx", i);
	}
	copyFileFromRes("A_ALNTAN", path + "store.pcx");
	copyFileFromRes("P_ALNTAN", path + "info.pcx");

	//apc
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "apc" + PATH_DELIMITER;
	for (int i = 0; i < 8; i++)
	{
		sprintf( szNum, "%d", i);
		copyFileFromRes_rpc("CLNTRANS", path + "img" + szNum + ".pcx", i + 8);
		copyFileFromRes("S_CLNTRA", path + "shw" + szNum + ".pcx", i + 8);
	}
	copyFileFromRes("A_COLNST", path + "store.pcx");
	copyFileFromRes("P_COLNST", path + "info.pcx");

	//assault
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "assault" + PATH_DELIMITER;
	for (int i = 0; i < 8; i++)
	{
		sprintf( szNum, "%d", i);
		try
		{
			surface = getImageFromRes("ARTILLRY", i);
			removePlayerColor( surface );
			resizeSurface ( surface, 23, 24, 64, 64);
			savePCX(surface, path + "img" + szNum + ".pcx");
			SDL_FreeSurface( surface );
		}
		END_INSTALL_FILE( path + "img" + szNum + ".pcx" );
		
		copyFileFromRes("S_ARTILL", path + "shw" + szNum + ".pcx", i );
	}
	copyFileFromRes("A_ARTY", path + "store.pcx");
	copyFileFromRes("P_ARTY", path + "info.pcx");

	//awac
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "awac" + PATH_DELIMITER;
	for ( int i = 0; i < 8; i++)
	{
		sprintf( szNum, "%d", i);
		copyFileFromRes_rpc("AWAC", path + "img" + szNum + ".pcx", i);
		copyFileFromRes("S_AWAC", path + "shw" + szNum + ".pcx", i);
	}

	try
	{
		output = getImageFromRes("AWAC", 8);
		resizeSurface( output,16 ,14 , 32, 32);
		resizeSurface( output,0 ,0 ,256, 32);
		dst_rect.y = 0;
		dst_rect.x = 32;
		src_rect.x = 15;
		src_rect.y = 15;
		src_rect.h = 32;
		src_rect.w = 32;
		for ( int i = 1; i < 8; i++)
		{
			surface = getImageFromRes("AWAC", i*4 +8);
			SDL_BlitSurface( surface, &src_rect, output, &dst_rect);
			SDL_FreeSurface( surface );
			dst_rect.x += 32;
		}
		
		savePCX( output, path + "overlay.pcx"); 
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "overlay.pcx" )
	
	copyFileFromRes("A_AWAC", path + "store.pcx");
	copyFileFromRes("P_AWAC", path + "info.pcx");

	//bomber
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "bomber" + PATH_DELIMITER;
	for ( int i = 0; i < 8; i++)
	{
		sprintf(szNum, "%d", i);
		try
		{
			surface = getImageFromRes("BOMBER", i);
			removePlayerColor( surface );
			resizeSurface( surface, 14, 18, 64, 64);
			savePCX(surface, path + "img" + szNum + ".pcx");
			SDL_FreeSurface( surface );
		}
		END_INSTALL_FILE( path + "img" + szNum + ".pcx" )

		copyFileFromRes("S_BOMBER", path + "shw" + szNum + ".pcx", i);
	}
	copyFileFromRes("A_BOMBER", path + "store.pcx");
	copyFileFromRes("P_BOMBER", path + "info.pcx");

	//bulldozer
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "bulldozer" + PATH_DELIMITER;
	for ( int i = 0; i < 8; i++ )
	{
		sprintf( szNum, "%d", i);
		try
		{
			surface = getImageFromRes("BULLDOZR", i);
			removePlayerColor( surface );
			resizeSurface( surface, 4, 4, 64, 64);
			savePCX( surface, path + "img" + szNum + ".pcx");
			SDL_FreeSurface( surface );
		}
		END_INSTALL_FILE( path + "img" + szNum + ".pcx")
		
		try
		{
			surface = getImageFromRes("S_BULLDO", i);
			resizeSurface( surface, 4, 4, 68, 68);
			savePCX( surface, path + "shw" + szNum + ".pcx");
			SDL_FreeSurface( surface );
		}
		END_INSTALL_FILE( path + "shw" + szNum + ".pcx")
	}
	copyFileFromRes("A_BULLDZ", path + "store.pcx");
	copyFileFromRes("P_BULLDZ", path + "info.pcx");
	
	try
	{
		surface = getImageFromRes( "LRGTAPE", 0);
		output = SDL_CreateRGBSurface(SDL_SWSURFACE, 512, 128, 8,0,0,0,0);
		SDL_SetColors(output, surface->format->palette->colors, 0, 256);
		SDL_FillRect( output, 0, SDL_MapRGB( output->format, 255, 0, 255));
		output->pitch = output->w;		//this seems to be an SDL-Bug...
										//sometimes the pitch of a surface has an wrong value
		dst_rect.x = 0;
		dst_rect.y = 0;
		for ( int i = 0; i < 4; i++ )
		{
			SDL_BlitSurface( surface, 0, output, &dst_rect );
			dst_rect.x += 128;
		}
		SDL_FreeSurface( surface );

		surface = getImageFromRes("LRGCONES");
		dst_rect.y = 5;
		dst_rect.x = 5;
		for ( int i = 0; i < 4; i++ )
		{
			SDL_BlitSurface( surface, 0, output, &dst_rect );
			dst_rect.x += 128;
		}
		SDL_FreeSurface( surface );

		surface = getImageFromRes("BULLDOZR", 8);
		removePlayerColor( surface );
		dst_rect.x = 36;
		dst_rect.y = 36;
		SDL_BlitSurface( surface, 0, output, &dst_rect );

		generateAnimationFrame( surface, 1);
		dst_rect.x = 164;
		SDL_BlitSurface( surface, 0, output, &dst_rect );

		generateAnimationFrame( surface, 2);
		dst_rect.x = 292;
		SDL_BlitSurface( surface, 0, output, &dst_rect );

		generateAnimationFrame( surface, 3);
		dst_rect.x = 420;
		SDL_BlitSurface( surface, 0, output, &dst_rect );
		SDL_FreeSurface( surface );
		savePCX( output, path + "clear_big.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "clear_big.pcx" )

	try
	{
		output = getImageFromRes("S_LRGCON");
		resizeSurface( output, 6, 6, 128, 128 ); 
		surface = getImageFromRes("S_BULLDO");
		dst_rect.x = 38;
		dst_rect.y = 37;
		SDL_BlitSurface( surface, 0, output, &dst_rect );
		SDL_FreeSurface( surface );
		savePCX( output, path + "clear_big_shw.pcx"); 
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "clear_big_shw.pcx")
	
	try
	{
		surface = getImageFromRes("SMLTAPE");
		output = SDL_CreateRGBSurface(SDL_SWSURFACE, 256, 64, 8,0,0,0,0);
		SDL_SetColors(output, surface->format->palette->colors, 0, 256);
		SDL_FillRect( output, 0, SDL_MapRGB( output->format, 255, 0, 255));
		output->pitch = output->w;		//this seems to be an SDL-Bug...
										//sometimes the pitch of a surface has an wrong value
		dst_rect.x = 0;
		dst_rect.y = 0;
		for ( int i = 0; i < 4; i++ )
		{
			SDL_BlitSurface( surface, 0, output, &dst_rect );
			dst_rect.x += 64;
		}
		SDL_FreeSurface( surface );

		surface = getImageFromRes("SMLCONES");
		dst_rect.y = 5;
		dst_rect.x = 5;
		for ( int i = 0; i < 4; i++ )
		{
			SDL_BlitSurface( surface, 0, output, &dst_rect );
			dst_rect.x += 64;
		}
		SDL_FreeSurface( surface );

		surface = getImageFromRes("BULLDOZR", 8);
		removePlayerColor( surface );
		dst_rect.x = 4;
		dst_rect.y = 4;
		SDL_BlitSurface( surface, 0, output, &dst_rect );

		generateAnimationFrame( surface, 1);
		dst_rect.x += 64;
		SDL_BlitSurface( surface, 0, output, &dst_rect );

		generateAnimationFrame( surface, 2);
		dst_rect.x += 64;
		SDL_BlitSurface( surface, 0, output, &dst_rect );

		generateAnimationFrame( surface, 3);
		dst_rect.x += 64;
		SDL_BlitSurface( surface, 0, output, &dst_rect );
		SDL_FreeSurface( surface );
		savePCX( output, path + "clear_small.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "clear_small.pcx")

	try
	{
		output = getImageFromRes("S_SMLCON");
		resizeSurface( output, 6, 6, 64, 66 );
		surface = getImageFromRes("S_BULLDO");
		dst_rect.x = 6;
		dst_rect.y = 5;
		SDL_BlitSurface( surface, 0, output, &dst_rect );
		SDL_FreeSurface( surface );

		savePCX( output, path + "clear_small_shw.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "clear_small_shw.pcx")

	//cargoship
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "cargoship" + PATH_DELIMITER;
	for ( int i = 0; i < 8; i++ )
	{
		sprintf( szNum, "%d", i);
		copyFileFromRes_rpc("CARGOSHP", path + "img" + szNum + ".pcx", i);
		copyFileFromRes("S_CARGOS", path + "shw" + szNum + ".pcx", i);
	}
	copyFileFromRes("A_CARGOS", path + "store.pcx");
	copyFileFromRes("P_CARGOS", path + "info.pcx");

	//cluster
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "cluster" + PATH_DELIMITER;
	for ( int i = 0; i < 8; i++)
	{
		sprintf(szNum, "%d", i);
		try
		{
			surface = getImageFromRes("ROCKTLCH", i);
			removePlayerColor( surface );
			resizeSurface( surface, 15, 15, 64, 64);
			savePCX(surface, path + "img" + szNum + ".pcx");
			SDL_FreeSurface( surface );
		}
		END_INSTALL_FILE( path + "img" + szNum + ".pcx" )

		copyFileFromRes("S_ROCKTL", path + "shw" + szNum + ".pcx", i);
	}
	copyFileFromRes("A_ROCKET", path + "store.pcx");
	copyFileFromRes("P_ROCKET", path + "info.pcx");

	//commando
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "commando" + PATH_DELIMITER;
	for (int i = 0; i < 8; i++)
	{
		sprintf( szNum1, "%d", i);
		for (int n = 0; n < 13; n++)
		{
			sprintf( szNum2, "%.2d", n);
			try
			{
				surface = getImageFromRes("COMMANDO", n * 8 + i );
				removePlayerColor( surface );
				resizeSurface( surface, 73, 73, 64, 64);
				savePCX(surface, path + "img" + szNum1 + "_" + szNum2 + ".pcx");
				SDL_FreeSurface( surface );
			}
			END_INSTALL_FILE( path + "img" + szNum1 + "_" + szNum2 + ".pcx")
		}
	}
	copyFileFromRes("A_COMMAN", path + "store.pcx");
	copyFileFromRes("P_COMMAN", path + "info.pcx");

	//corvet
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "corvet" + PATH_DELIMITER;
	for ( int i = 0; i < 8; i++ )
	{
		sprintf( szNum, "%d", i);
		copyFileFromRes_rpc("CORVETTE", path + "img" + szNum + ".pcx", i);
		copyFileFromRes("S_CORVET", path + "shw" + szNum + ".pcx", i);
	}
	copyFileFromRes("A_CORVET", path + "store.pcx");
	copyFileFromRes("P_CORVET", path + "info.pcx");

	//escort
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "escort" + PATH_DELIMITER;
	for ( int i = 0; i < 8; i++ )
	{
		sprintf( szNum, "%d", i);
		copyFileFromRes_rpc("FASTBOAT", path + "img" + szNum + ".pcx", i);
		copyFileFromRes("S_FASTBO", path + "shw" + szNum + ".pcx", i);
	}
	copyFileFromRes("A_ESCORT", path + "store.pcx");
	copyFileFromRes("P_ESCORT", path + "info.pcx");

	//fighter
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "fighter" + PATH_DELIMITER;
	for ( int i = 0; i < 8; i++)
	{
		sprintf(szNum, "%d", i);
		try
		{
			surface = getImageFromRes("FIGHTER", i);
			removePlayerColor( surface );
			resizeSurface( surface, 17, 16, 64, 64);
			savePCX(surface, path + "img" + szNum + ".pcx");
			SDL_FreeSurface( surface );
		}
		END_INSTALL_FILE( path + "img" + szNum + ".pcx")

		copyFileFromRes("S_FIGHTE", path + "shw" + szNum + ".pcx", i);
	}
	copyFileFromRes("A_FIGHTR", path + "store.pcx");
	copyFileFromRes("P_FIGHTR", path + "info.pcx");
	
	//gunboat
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "gunboat" + PATH_DELIMITER;
	try
	{
		src_rect.h = 40; 
		src_rect.w = 40;
		src_rect.x = 18;
		src_rect.y = 23;
		output = getImageFromRes("BATTLSHP", 0);
		removePlayerColor( output );
		resizeSurface( output, 4, 11, 64, 64);
		dst_rect.y = 3;
		dst_rect.x = 14; 
		surface = getImageFromRes("BATTLSHP", 8);
		SDL_BlitSurface(surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface( surface );
		savePCX(output, path + "img0.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "img0.pcx")

	try
	{
		output = getImageFromRes("BATTLSHP", 1);
		removePlayerColor( output );
		resizeSurface( output, 4, 11, 64, 64);
		dst_rect.y = 4;
		dst_rect.x = 22;
		surface = getImageFromRes("BATTLSHP", 9);
		SDL_BlitSurface(surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface( surface );
		savePCX(output, path + "img1.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "img1.pcx")

	try
	{
		output = getImageFromRes("BATTLSHP", 2);
		removePlayerColor( output );
		resizeSurface( output, 4, 11, 64, 64);
		dst_rect.y = 12;
		dst_rect.x = 23;
		surface = getImageFromRes("BATTLSHP", 10);
		SDL_BlitSurface(surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface( surface );
		savePCX(output, path + "img2.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "img2.pcx")

	try
	{
		output = getImageFromRes("BATTLSHP", 3);
		removePlayerColor( output );
		resizeSurface( output, 4, 11, 64, 64);
		dst_rect.y = 20;
		dst_rect.x = 22;
		surface = getImageFromRes("BATTLSHP", 11);
		SDL_BlitSurface(surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface( surface );
		savePCX(output, path + "img3.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "img3.pcx")

	try
	{
		output = getImageFromRes("BATTLSHP", 4);
		removePlayerColor( output );
		resizeSurface( output, 4, 11, 64, 64);
		dst_rect.y = 21;
		dst_rect.x = 14;
		surface = getImageFromRes("BATTLSHP", 12);
		SDL_BlitSurface(surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface( surface );
		savePCX(output, path + "img4.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "img4.pcx")

	try
	{
		output = getImageFromRes("BATTLSHP", 5);
		removePlayerColor( output );
		resizeSurface( output, 4, 11, 64, 64);
		dst_rect.y = 20;
		dst_rect.x = 6;
		surface = getImageFromRes("BATTLSHP", 13);
		SDL_BlitSurface(surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface( surface );
		savePCX(output, path + "img5.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "img5.pcx")

	try
	{
		output = getImageFromRes("BATTLSHP", 6);
		removePlayerColor( output );
		resizeSurface( output, 4, 11, 64, 64);
		dst_rect.y = 12;
		dst_rect.x = 5;
		surface = getImageFromRes("BATTLSHP", 14);
		SDL_BlitSurface(surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface( surface );
		savePCX(output, path + "img6.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "img6.pcx")

	try
	{
		output = getImageFromRes("BATTLSHP", 7);
		removePlayerColor( output );
		resizeSurface( output, 4, 11, 64, 64);
		dst_rect.y = 4;
		dst_rect.x = 6;
		surface = getImageFromRes("BATTLSHP", 15);
		SDL_BlitSurface(surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface( surface );
		savePCX(output, path + "img7.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "img7.pcx")

	for (int i = 0; i < 8; i++ )
	{
		sprintf(szNum, "%d", i);
		copyFileFromRes("S_BATTLS", path + "shw" + szNum + ".pcx", i);
	}
	copyFileFromRes("A_GUNBT", path + "store.pcx");
	copyFileFromRes("P_GUNBT", path + "info.pcx");

	//infantery
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "infantery" + PATH_DELIMITER;
	for (int i = 0; i < 8; i++)
	{
		sprintf( szNum1, "%d", i);
		for (int n = 0; n < 13; n++)
		{
			sprintf( szNum2, "%.2d", n);
			try
			{
				surface = getImageFromRes("INFANTRY", n * 8 + i );
				removePlayerColor( surface );
				resizeSurface( surface, 73, 73, 64, 64);
				savePCX(surface, path + "img" + szNum1 + "_" + szNum2 + ".pcx");
				SDL_FreeSurface( surface );
			}
			END_INSTALL_FILE( path + "img" + szNum1 + "_" + szNum2 + ".pcx")
		}
	}
	copyFileFromRes("A_INFANT", path + "store.pcx");
	copyFileFromRes("P_INFANT", path + "info.pcx");
	
	//konstrukt
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "konstrukt" + PATH_DELIMITER;
	for ( int i = 0; i < 8; i++ )
	{
		sprintf( szNum, "%d", i);
		copyFileFromRes_rpc("CONSTRCT", path + "img" + szNum + ".pcx", i);
		copyFileFromRes("S_CONSTR", path + "shw" + szNum + ".pcx", i);
	}
	copyFileFromRes("A_CONTRC", path + "store.pcx");
	copyFileFromRes("P_CONTRC", path + "info.pcx");

	try
	{
		surface = getImageFromRes( "LRGTAPE", 0);
		output = SDL_CreateRGBSurface(SDL_SWSURFACE, 512, 128, 8,0,0,0,0);
		SDL_SetColors(output, surface->format->palette->colors, 0, 256);
		SDL_FillRect( output, 0, SDL_MapRGB( output->format, 255, 0, 255));
		output->pitch = output->w;		//this seems to be an SDL-Bug...
										//sometimes the pitch of a surface has an wrong value
		dst_rect.x = 0;
		dst_rect.y = 0;
		for ( int i = 0; i < 4; i++ )
		{
			SDL_BlitSurface( surface, 0, output, &dst_rect );
			dst_rect.x += 128;
		}
		SDL_FreeSurface( surface );

		surface = getImageFromRes("LRGCONES");
		dst_rect.y = 5;
		dst_rect.x = 5;
		for ( int i = 0; i < 4; i++ )
		{
			SDL_BlitSurface( surface, 0, output, &dst_rect );
			dst_rect.x += 128;
		}
		SDL_FreeSurface( surface );

		surface = getImageFromRes("CONSTRCT", 16);
		removePlayerColor( surface );
		dst_rect.x = 33;
		dst_rect.y = 36;
		SDL_BlitSurface( surface, 0, output, &dst_rect );
		SDL_FreeSurface( surface );

		surface = getImageFromRes("CONSTRCT", 24);
		removePlayerColor( surface );
		generateAnimationFrame( surface, 1);
		dst_rect.x += 128;
		SDL_BlitSurface( surface, 0, output, &dst_rect );
		SDL_FreeSurface( surface );

		surface = getImageFromRes("CONSTRCT", 16);
		removePlayerColor( surface );
		generateAnimationFrame( surface, 2);
		dst_rect.x += 128;
		SDL_BlitSurface( surface, 0, output, &dst_rect );
		SDL_FreeSurface( surface );
		
		surface = getImageFromRes("CONSTRCT", 32);
		removePlayerColor( surface );
		generateAnimationFrame( surface, 3);
		dst_rect.x += 128;
		SDL_BlitSurface( surface, 0, output, &dst_rect );
		SDL_FreeSurface( surface );
		savePCX( output, path + "build.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "build.pcx")

	try
	{
		output = getImageFromRes("S_LRGCON");
		resizeSurface( output, 6, 6, 128, 128 );
		surface = getImageFromRes("S_CONSTR");
		dst_rect.x = 38;
		dst_rect.y = 37;
		SDL_BlitSurface( surface, 0, output, &dst_rect );
		SDL_FreeSurface( surface );
		savePCX( output, path + "build_shw.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "build_shw.pcx")


	//minelayer
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "minelayer" + PATH_DELIMITER;
	for ( int i = 0; i < 8; i++ )
	{
		sprintf( szNum, "%d", i);
		copyFileFromRes_rpc("MINELAYR", path + "img" + szNum + ".pcx", i);
		copyFileFromRes("S_MINELA", path + "shw" + szNum + ".pcx", i);
	}
	copyFileFromRes("A_MNELAY", path + "store.pcx");
	copyFileFromRes("P_MNELAY", path + "info.pcx");

	//missel
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "missel" + PATH_DELIMITER;
	for ( int i = 0; i < 8; i++ )
	{
		try
		{
			sprintf( szNum, "%d", i);
			surface = getImageFromRes("MISSLLCH", i);
			removePlayerColor( surface );
			resizeSurface( surface, 16, 15, 64, 64);
			savePCX(surface, path + "img" + szNum + ".pcx");
			SDL_FreeSurface( surface );
		}
		END_INSTALL_FILE( path + "img" + szNum + ".pcx" );

		try
		{
			surface = getImageFromRes("S_MISSLL", i);
			resizeSurface( surface, 16, 16, 64, 64);
			savePCX( surface, path + "shw" + szNum + ".pcx" );
			SDL_FreeSurface( surface );
		}
		END_INSTALL_FILE( path + "shw" + szNum + ".pcx" );

	}
	copyFileFromRes("A_MISSIL", path + "store.pcx");
	copyFileFromRes("P_MISSIL", path + "info.pcx");

	//missel_ship
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "missel_ship" + PATH_DELIMITER;
	for ( int i = 0; i < 8; i++)
	{
		sprintf(szNum, "%d", i);
		try
		{
			surface = getImageFromRes("MSSLBOAT", i);
			removePlayerColor( surface );
			resizeSurface( surface, 16, 16, 64, 64);
			savePCX(surface, path + "img" + szNum + ".pcx");
			SDL_FreeSurface( surface );
		}
		END_INSTALL_FILE( path + "img" + szNum + ".pcx")

		copyFileFromRes("S_MSSLBO", path + "shw" + szNum + ".pcx", i);
	}
	copyFileFromRes("A_MSLCR", path + "store.pcx");
	copyFileFromRes("P_MSLCR", path + "info.pcx");
	
	//mobile_aa
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "mobile_aa" + PATH_DELIMITER;
	src_rect.h = 33; 
	src_rect.w = 35;
	src_rect.x = 17;
	src_rect.y = 16;

	try
	{
		output = getImageFromRes("SP_FLAK", 0);
		removePlayerColor( output );
		resizeSurface( output, 3, 0, 64, 64);
		dst_rect.y = 3;
		dst_rect.x = 14; 
		surface = getImageFromRes("SP_FLAK", 8);
		SDL_BlitSurface(surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface( surface );
		savePCX(output, path + "img0.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "img0.pcx")

	try
	{
		output = getImageFromRes("SP_FLAK", 1);
		removePlayerColor( output );
		resizeSurface( output, 3, 0, 64, 64);
		dst_rect.y = 4;
		dst_rect.x = 24;
		surface = getImageFromRes("SP_FLAK", 9);
		SDL_BlitSurface(surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface( surface );
		savePCX(output, path + "img1.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "img1.pcx")

	try
	{
		output = getImageFromRes("SP_FLAK", 2);
		removePlayerColor( output );
		resizeSurface( output, 3, 0, 64, 64);
		dst_rect.y = 15;
		dst_rect.x = 24;
		surface = getImageFromRes("SP_FLAK", 10);
		SDL_BlitSurface(surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface( surface );
		savePCX(output, path + "img2.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "img2.pcx")

	try
	{
		output = getImageFromRes("SP_FLAK", 3);
		removePlayerColor( output );
		resizeSurface( output, 3, 0, 64, 64);
		dst_rect.y = 23;
		dst_rect.x = 22;
		surface = getImageFromRes("SP_FLAK", 11);
		SDL_BlitSurface(surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface( surface );
		savePCX(output, path + "img3.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "img3.pcx")

	try
	{
		output = getImageFromRes("SP_FLAK", 4);
		removePlayerColor( output );
		resizeSurface( output, 3, 0, 64, 64);
		dst_rect.y = 27;
		dst_rect.x = 14;
		surface = getImageFromRes("SP_FLAK", 12);
		SDL_BlitSurface(surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface( surface );
		savePCX(output, path + "img4.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "img4.pcx")

	try
	{
		output = getImageFromRes("SP_FLAK", 5);
		removePlayerColor( output );
		resizeSurface( output, 3, 0, 64, 64);
		dst_rect.y = 22;
		dst_rect.x = 6;
		surface = getImageFromRes("SP_FLAK", 13);
		SDL_BlitSurface(surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface( surface );
		savePCX(output, path + "img5.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "img5.pcx")

	try
	{
		output = getImageFromRes("SP_FLAK", 6);
		removePlayerColor( output );
		resizeSurface( output, 3, 0, 64, 64);
		dst_rect.y = 16;
		dst_rect.x = 4;
		surface = getImageFromRes("SP_FLAK", 14);
		SDL_BlitSurface(surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface( surface );
		savePCX(output, path + "img6.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "img6.pcx")

	try
	{
		output = getImageFromRes("SP_FLAK", 7);
		removePlayerColor( output );
		resizeSurface( output, 3, 0, 64, 64);
		dst_rect.y = 8;
		dst_rect.x = 5;
		surface = getImageFromRes("SP_FLAK", 15);
		SDL_BlitSurface(surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface( surface );
		savePCX(output, path + "img7.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "img7.pcx")

	for (int i = 0; i < 8; i++ )
	{
		sprintf( szNum, "%d", i);
		copyFileFromRes("S_FLAK", path + "shw" + szNum + ".pcx", i);
	}
	copyFileFromRes("A_AA", path + "store.pcx");
	copyFileFromRes("P_AA", path + "info.pcx");

	//pionier
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "pionier" + PATH_DELIMITER;
	for (int i = 0; i < 8; i++ )
	{
		sprintf( szNum, "%d", i);
		try
		{
			surface = getImageFromRes("ENGINEER", i);
			removePlayerColor( surface );
			resizeSurface( surface, 4, 4, 64, 64);
			savePCX(surface, path + "img" + szNum + ".pcx");
			SDL_FreeSurface( surface );
		}
		END_INSTALL_FILE( path + "img" + szNum + ".pcx")
		
		try
		{
			surface = getImageFromRes("S_ENGINE", i);
			resizeSurface( surface, 4, 4, 67, 66);
			savePCX( surface, path + "shw" + szNum + ".pcx");
			SDL_FreeSurface( surface );
		}
		END_INSTALL_FILE( path + "shw" + szNum + ".pcx")
	}
	copyFileFromRes("A_ENGINR", path + "store.pcx");
	copyFileFromRes("P_ENGINR", path + "info.pcx");

	try
	{
		surface = getImageFromRes("SMLTAPE");
		output = SDL_CreateRGBSurface(SDL_SWSURFACE, 256, 64, 8,0,0,0,0);
		SDL_SetColors(output, surface->format->palette->colors, 0, 256);
		SDL_FillRect( output, 0, SDL_MapRGB( output->format, 255, 0, 255));
		output->pitch = output->w;		//this seems to be an SDL-Bug...
										//sometimes the pitch of a surface has an wrong value
		dst_rect.x = 0;
		dst_rect.y = 0;
		for ( int i = 0; i < 4; i++ )
		{
			SDL_BlitSurface( surface, 0, output, &dst_rect );
			dst_rect.x += 64;
		}
		SDL_FreeSurface( surface );

		surface = getImageFromRes("SMLCONES");
		dst_rect.y = 5;
		dst_rect.x = 5;
		for ( int i = 0; i < 4; i++ )
		{
			SDL_BlitSurface( surface, 0, output, &dst_rect );
			dst_rect.x += 64;
		}
		SDL_FreeSurface( surface );

		surface = getImageFromRes("ENGINEER", 16);
		removePlayerColor( surface );
		dst_rect.x = 4;
		dst_rect.y = 4;
		SDL_BlitSurface( surface, 0, output, &dst_rect );

		generateAnimationFrame( surface, 1);
		dst_rect.x += 64;
		SDL_BlitSurface( surface, 0, output, &dst_rect );

		generateAnimationFrame( surface, 2);
		dst_rect.x += 64;
		SDL_BlitSurface( surface, 0, output, &dst_rect );

		generateAnimationFrame( surface, 3);
		dst_rect.x += 64;
		SDL_BlitSurface( surface, 0, output, &dst_rect );
		SDL_FreeSurface( surface );
		savePCX( output, path + "build.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "build.pcx")

	try
	{
		output = getImageFromRes("S_SMLCON");
		resizeSurface( output, 6, 6, 64, 66 );
		surface = getImageFromRes("S_ENGINE");
		dst_rect.x = 6;
		dst_rect.y = 5;
		SDL_BlitSurface( surface, 0, output, &dst_rect );
		SDL_FreeSurface( surface );
		savePCX( output, path + "build_shw.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "build_shw.pcx")


	//repair
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "repair" + PATH_DELIMITER;
	for (int i = 0; i < 8; i++)
	{
		sprintf( szNum, "%d", i);
		copyFileFromRes_rpc("REPAIR", path + "img" + szNum + ".pcx", i);
		copyFileFromRes("S_REPAIR", path + "shw" + szNum + ".pcx", i);
	}
	copyFileFromRes("A_REPAIR", path + "store.pcx");
	copyFileFromRes("P_REPAIR", path + "info.pcx");

	//scanner
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "scanner" + PATH_DELIMITER;
	for ( int i = 0; i < 8; i++)
	{
		sprintf( szNum, "%d", i);
		try
		{
			surface = getImageFromRes("SCANNER", i);
			removePlayerColor( surface );
			resizeSurface( surface, 8, 8, 64, 64);
			savePCX( surface, path + "img" + szNum + ".pcx");
			SDL_FreeSurface( surface );
		}
		END_INSTALL_FILE( path + "img" + szNum + ".pcx")

		try
		{
			surface = getImageFromRes("S_SCANNE", i);
			resizeSurface( surface, 8, 8, 61, 60);
			savePCX( surface, path + "shw" + szNum + ".pcx");
			SDL_FreeSurface( surface );
		}
		END_INSTALL_FILE( path + "shw" + szNum + ".pcx")
	}
	copyFileFromRes("A_SCANNR", path + "store.pcx");
	copyFileFromRes("P_SCANNR", path + "info.pcx");

	try
	{
		surface = getImageFromRes("SCANNER", 8);
		output = SDL_CreateRGBSurface(SDL_SWSURFACE, 360, 45, 8,0,0,0,0);
		SDL_SetColors(output, surface->format->palette->colors, 0, 256);
		SDL_FillRect( output, 0, SDL_MapRGB( output->format, 255, 0, 255));
		output->pitch = output->w;		//this seems to be an SDL-Bug...
										//sometimes the pitch of a surface has an wrong value
		resizeSurface( surface, 2, 1, 45, 45);
		dst_rect.x = 0;
		dst_rect.y = 0;
		SDL_BlitSurface( surface, 0, output, &dst_rect);
		SDL_FreeSurface( surface );
		dst_rect.x = 45;
		for ( int i = 1; i < 8; i++)
		{
			surface = getImageFromRes("SCANNER", i*2 + 8);
			resizeSurface( surface, 2, 1, 45, 45);
			SDL_BlitSurface( surface, 0, output, &dst_rect);
			SDL_FreeSurface( surface );
			dst_rect.x += 45;
		}
		savePCX( output, path + "overlay.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "overlay.pcx")

	//scout
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "scout" + PATH_DELIMITER;
	for ( int i = 0; i < 8; i++)
	{
		sprintf( szNum, "%d", i);
		copyFileFromRes_rpc("SCOUT", path + "img" + szNum + ".pcx", i);
		copyFileFromRes("S_SCOUT", path + "shw" + szNum + ".pcx", i);
	}
	copyFileFromRes("A_SCOUT", path + "store.pcx");
	copyFileFromRes("P_SCOUT", path + "info.pcx");

	//sea_minelayer
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "sea_minelayer" + PATH_DELIMITER;
	for ( int i = 0; i < 8; i++)
	{
		sprintf( szNum, "%d", i);
		copyFileFromRes_rpc("SEAMNLYR", path + "img" + szNum + ".pcx", i);
		copyFileFromRes("S_SEAMNL", path + "shw" + szNum + ".pcx", i);
	}
	copyFileFromRes("A_SEAMNL", path + "store.pcx");
	copyFileFromRes("P_SEAMNL", path + "info.pcx");

	//sea_transport
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "sea_transport" + PATH_DELIMITER;
	for ( int i = 0; i < 8; i++)
	{
		sprintf( szNum, "%d", i);
		copyFileFromRes_rpc("SEATRANS", path + "img" + szNum + ".pcx", i);
		copyFileFromRes("S_SEATRA", path + "shw" + szNum + ".pcx", i);
	}
	copyFileFromRes("A_SEATRN", path + "store.pcx");
	copyFileFromRes("P_SEATRN", path + "info.pcx");

	//sub
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "sub" + PATH_DELIMITER;
	for ( int i = 0; i < 8; i++)
	{
		sprintf( szNum, "%d", i);
		copyFileFromRes_rpc("SUBMARNE", path + "img" + szNum + ".pcx", i + 8);
	}
	copyFileFromRes("A_SUB", path + "store.pcx");
	copyFileFromRes("P_SUB", path + "info.pcx");

	//surveyor
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "surveyor" + PATH_DELIMITER;
	for ( int i = 0; i < 8; i++)
	{
		sprintf( szNum, "%d", i);
		try
		{
			surface = getImageFromRes("SURVEYOR", i);
			removePlayerColor( surface );
			resizeSurface( surface, 2, 2, 64, 64);
			savePCX( surface, path + "img" + szNum + ".pcx");
			SDL_FreeSurface( surface );
		}
		END_INSTALL_FILE( path + "img" + szNum + ".pcx")

		try
		{
			surface = getImageFromRes("S_SURVEY", i);
			resizeSurface( surface, 2, 2, 69, 72);
			savePCX( surface, path + "shw" + szNum + ".pcx");
			SDL_FreeSurface( surface );
		}
		END_INSTALL_FILE( path + "shw" + szNum + ".pcx")
	}
	copyFileFromRes("A_SURVEY", path + "store.pcx");
	copyFileFromRes("P_SURVEY", path + "info.pcx");
	
	//tank
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "tank" + PATH_DELIMITER;
	for ( int i = 0; i < 8; i++)
	{
		sprintf( szNum, "%d", i);
		try
		{
			output = getImageFromRes("TANK", i);
			removePlayerColor( output );
			resizeSurface( output, 15, 15, 64, 64);
			surface = getImageFromRes("TANK", i + 8);
			src_rect.x = 15;
			src_rect.y = 15;
			src_rect.h = 64;
			src_rect.w = 64;
			SDL_BlitSurface( surface, &src_rect, output, 0);
			SDL_FreeSurface( surface );
			savePCX( output, path + "img" + szNum + ".pcx");
			SDL_FreeSurface( output );
		}
		END_INSTALL_FILE( path + "img" + szNum + ".pcx")

		try
		{
			surface = getImageFromRes("S_TANK", i);
			resizeSurface( surface, 2, 2, 69, 72);
			savePCX( surface, path + "shw" + szNum + ".pcx");
			SDL_FreeSurface ( surface );
		}
		END_INSTALL_FILE( path + "shw" + szNum + ".pcx")
	}
	copyFileFromRes("A_TANK", path + "store.pcx");
	copyFileFromRes("P_TANK", path + "info.pcx");

	//trans_gold
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "trans_gold" + PATH_DELIMITER;
	for ( int i = 0; i < 8; i++)
	{
		sprintf( szNum, "%d", i);
		copyFileFromRes_rpc("GOLDTRCK", path + "img" + szNum + ".pcx", i);
		copyFileFromRes("S_GOLDTR", path + "shw" + szNum + ".pcx", i);
	}
	copyFileFromRes("A_GOLDTR", path + "store.pcx");
	copyFileFromRes("P_GOLDTR", path + "info.pcx");
	
	//trans_metal
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "trans_metal" + PATH_DELIMITER;
	for (int i = 0; i < 8; i++)
	{
		sprintf( szNum, "%d", i);
		try
		{
			surface = getImageFromRes("SPLYTRCK", i);
			removePlayerColor( surface );
			resizeSurface( surface, 4, 4, 64, 64);
			savePCX( surface, path + "img" + szNum + ".pcx");
			SDL_FreeSurface ( surface );
		}
		END_INSTALL_FILE( path + "img" + szNum + ".pcx")

		try
		{
			surface = getImageFromRes("S_SPLYTR", i);
			resizeSurface( surface, 4, 4, 64, 64);
			savePCX( surface, path + "shw" + szNum + ".pcx");
			SDL_FreeSurface ( surface );
		}
		END_INSTALL_FILE( path + "shw" + szNum + ".pcx")
	}
	copyFileFromRes("A_SPLYTR", path + "store.pcx");
	copyFileFromRes("P_SPLYTR", path + "info.pcx");
	
	//trans_oil
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "trans_oil" + PATH_DELIMITER;
	for (int i = 0; i < 8; i++)
	{
		sprintf( szNum, "%d", i);
		try
		{
			surface = getImageFromRes("FUELTRCK", i);
			removePlayerColor( surface );
			resizeSurface( surface, 3, 3, 64, 64);
			savePCX( surface, path + "img" + szNum + ".pcx");
			SDL_FreeSurface ( surface );
		}
		END_INSTALL_FILE( path + "img" + szNum + ".pcx")

		try
		{
			surface = getImageFromRes("S_FUELTR", i);
			resizeSurface( surface, 3, 3, 66, 66);
			savePCX( surface, path + "shw" + szNum + ".pcx");
			SDL_FreeSurface ( surface );
		}
		END_INSTALL_FILE( path + "shw" + szNum + ".pcx")
	}
	copyFileFromRes("A_FUELTR", path + "store.pcx");
	copyFileFromRes("P_FUELTR", path + "info.pcx");

	if ( logFile != NULL )
	{
		writeLog( string("Vehicle graphics") + TEXT_FILE_LF);
		writeLog( iToStr( iErrors) + " errors" + TEXT_FILE_LF);
		writeLog( string("========================================================================") + TEXT_FILE_LF);
	}

	cout << "\n";
	cout << iToStr( iErrors) << " errors\n";
	return 1;
}

int installBuildingGraphics()
{
	string path;
	iTotalFiles = 140;
	iErrors = 0;
	iInstalledFiles = 0;
	SDL_Surface* surface;
	SDL_Surface* output;
	SDL_Rect src_rect, dst_rect;

	cout << "========================================================================\n";
	cout << "Building graphics\n";

	//Barracks
	path = sOutputPath + "buildings" + PATH_DELIMITER + "barracks" + PATH_DELIMITER;
	copyFileFromRes_rpc("BARRACKS", path + "img.pcx", 1 );
	copyFileFromRes("P_BARRCK", path + "info.pcx");
	copyFileFromRes("S_BARRAC", path + "shw.pcx");
	copyImageFromFLC( sMAXPath + "BARX_ISO.FLC", path + "video.pcx");
	
	//block
	path = sOutputPath + "buildings" + PATH_DELIMITER + "block" + PATH_DELIMITER;
	copyFileFromRes("BLOCK", path + "img.pcx");
	copyFileFromRes("P_BLOCK", path + "info.pcx");
	copyFileFromRes("S_BLOCK", path + "shw.pcx");
	copyImageFromFLC( sMAXPath + "BLOK128.FLC", path + "video.pcx");

	//bridge
	path = sOutputPath + "buildings" + PATH_DELIMITER + "bridge" + PATH_DELIMITER;
	copyFileFromRes_rpc("BRIDGE", path + "img.pcx");
	copyFileFromRes("P_BRIDGE", path + "info.pcx");
	copyFileFromRes("S_BRIDGE", path + "shw.pcx");
	copyImageFromFLC( sMAXPath + "BRIDGE.FLC", path + "video.pcx");

	//connector
	path = sOutputPath + "buildings" + PATH_DELIMITER + "connector" + PATH_DELIMITER;
	try
	{
		surface = getImageFromRes("CNCT_4W", 0);
		output = SDL_CreateRGBSurface(SDL_SWSURFACE, 1024, 64,8,0,0,0,0);
		SDL_SetColors(output, surface->format->palette->colors, 0, 256);
		SDL_FillRect( output, 0, SDL_MapRGB( output->format, 255, 0, 255));
		dst_rect.y = 0;
		for ( int i = 0; i < 1024; i+=64 )
		{
			dst_rect.x = i;
			SDL_BlitSurface(surface, 0, output, &dst_rect);
		}
		SDL_FreeSurface( surface );

		surface = getImageFromRes("CNCT_4W", 2 );
		dst_rect.x = 64;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 320;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 448;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 512;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 704;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 768;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 832;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 960;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		SDL_FreeSurface( surface );

		surface = getImageFromRes("CNCT_4W", 3 );
		dst_rect.x = 128;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 384;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 512;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 576;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 768;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 832;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 896;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 960;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		SDL_FreeSurface( surface );

		surface = getImageFromRes("CNCT_4W", 4 );
		dst_rect.x = 192;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 320;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 576;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 640;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 704;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 832;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 896;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 960;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		SDL_FreeSurface( surface );

		surface = getImageFromRes("CNCT_4W", 5 );
		dst_rect.x = 256;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 384;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 448;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 640;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 704;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 768;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 896;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 960;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		SDL_FreeSurface( surface );

		savePCX( output, path + "img.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "img.pcx" )

	try
	{
		surface = getImageFromRes("S_CNCT4W", 0);
		output = SDL_CreateRGBSurface(SDL_SWSURFACE, 1024, 64,8,0,0,0,0);
		SDL_SetColors(output, surface->format->palette->colors, 0, 256);
		SDL_FillRect( output, 0, SDL_MapRGB( output->format, 255, 0, 255));
		dst_rect.y = 0;
		for ( int i = 0; i < 1024; i+=64 )
		{
			dst_rect.x = i;
			SDL_BlitSurface(surface, 0, output, &dst_rect);
		}
		SDL_FreeSurface( surface );

		surface = getImageFromRes("S_CNCT4W", 2);
		dst_rect.x = 64;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 320;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 448;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 512;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 704;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 768;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 832;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 960;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		SDL_FreeSurface( surface );

		surface = getImageFromRes("S_CNCT4W", 3);
		dst_rect.x = 128;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 384;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 512;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 576;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 768;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 832;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 896;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 960;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		SDL_FreeSurface( surface );

		surface = getImageFromRes("S_CNCT4W", 4);
		dst_rect.x = 192;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 320;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 576;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 640;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 704;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 832;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 896;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 960;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		SDL_FreeSurface( surface );

		surface = getImageFromRes("S_CNCT4W", 5);
		dst_rect.x = 256;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 384;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 448;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 640;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 704;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 768;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 896;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		dst_rect.x = 960;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		SDL_FreeSurface( surface );

		savePCX( output, path + "shw.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "shw.pcx" )

	copyFileFromRes("P_CONNEC", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "CROSS.FLC", path + "video.pcx");

	//depot
	path = sOutputPath + "buildings" + PATH_DELIMITER + "depot" + PATH_DELIMITER;
	copyFileFromRes_rpc("DEPOT", path + "img.pcx", 1);
	copyFileFromRes("P_DEPOT", path + "info.pcx");
	copyFileFromRes("S_DEPOT", path + "shw.pcx", 1);
	copyImageFromFLC( sMAXPath + "DPBG_S.FLC", path + "video.pcx");



	//dock
	path = sOutputPath + "buildings" + PATH_DELIMITER + "dock" + PATH_DELIMITER;
	copyFileFromRes_rpc("DOCK", path + "img.pcx");
	copyFileFromRes("S_DOCK", path + "shw.pcx");
	copyFileFromRes("P_DOCK", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "DOCK.FLC", path + "video.pcx");

	//energy big
	path = sOutputPath + "buildings" + PATH_DELIMITER + "energy_big" + PATH_DELIMITER;
	copyFileFromRes_rpc("POWERSTN", path + "img.pcx");
	copyFileFromRes("S_POWERS", path + "shw.pcx");
	copyFileFromRes("P_POWSTN", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "PWRGEN.FLC", path + "video.pcx");

	//energy small
	path = sOutputPath + "buildings" + PATH_DELIMITER + "energy_small" + PATH_DELIMITER;
	copyFileFromRes_rpc("POWGEN", path + "img.pcx");
	copyFileFromRes("S_POWGEN", path + "shw.pcx");
	copyFileFromRes("P_POWGEN", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "SPWR_ISO.FLC", path + "video.pcx");

	//fac air
	path = sOutputPath + "buildings" + PATH_DELIMITER + "fac_air" + PATH_DELIMITER;
	copyFileFromRes_rpc("AIRPLT", path + "img.pcx");
	copyFileFromRes("S_AIRPLT", path + "shw.pcx");
	copyFileFromRes("P_AIRPLT", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "AIRPLNT.FLC", path + "video.pcx");

	//fac alien
	path = sOutputPath + "buildings" + PATH_DELIMITER + "fac_alien" + PATH_DELIMITER;
	copyFileFromRes_rpc("RECCENTR", path + "img.pcx");
	copyFileFromRes("S_RECCEN", path + "shw.pcx");
	copyFileFromRes("P_RECCTR", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "RECNTR.FLC", path + "video.pcx");

	//fac big
	path = sOutputPath + "buildings" + PATH_DELIMITER + "fac_big" + PATH_DELIMITER;
	copyFileFromRes_rpc("LANDPLT", path + "img.pcx");
	copyFileFromRes("S_LANDPL", path + "shw.pcx");
	copyFileFromRes("P_HVYPLT", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "FVP.FLC", path + "video.pcx");

	//fac ship
	path = sOutputPath + "buildings" + PATH_DELIMITER + "fac_ship" + PATH_DELIMITER;
	copyFileFromRes_rpc("SHIPYARD", path + "img.pcx");
	copyFileFromRes("S_SHIPYA", path + "shw.pcx");
	copyFileFromRes("P_SHIPYD", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "SHPYRD.FLC", path + "video.pcx");

	//fac small
	path = sOutputPath + "buildings" + PATH_DELIMITER + "fac_small" + PATH_DELIMITER;
	copyFileFromRes_rpc("LIGHTPLT", path + "img.pcx");
	copyFileFromRes("S_LIGHTP", path + "shw.pcx");
	copyFileFromRes("P_LGHTPL", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "LVP_ISO.FLC", path + "video.pcx");

	//goldraff
	path = sOutputPath + "buildings" + PATH_DELIMITER + "goldraff" + PATH_DELIMITER;
	copyFileFromRes_rpc("COMMTWR", path + "img.pcx");
	copyFileFromRes("S_COMMTW", path + "shw.pcx");
	copyFileFromRes("P_TRANSP", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "COMMTWR.FLC", path + "video.pcx");

	//gun aa
	path = sOutputPath + "buildings" + PATH_DELIMITER + "gun_aa" + PATH_DELIMITER;
	try
	{
		surface = getImageFromRes("ANTIAIR");
		removePlayerColor( surface );
		output = SDL_CreateRGBSurface(SDL_SWSURFACE, 512, 64,8,0,0,0,0);
		SDL_SetColors(output, surface->format->palette->colors, 0, 256);
		SDL_FillRect( output, 0, SDL_MapRGB( output->format, 255, 0, 255));
		dst_rect.y = 0;
		dst_rect.x = 0;
		src_rect.x = 16;
		src_rect.y = 13;
		src_rect.h = 64;
		src_rect.w = 64;
		for ( int i = 0; i < 512; i+=64 )
		{
			dst_rect.x = i;
			SDL_BlitSurface(surface, &src_rect, output, &dst_rect);
		}
		SDL_FreeSurface( surface );


		for ( int i = 1; i < 9; i++ )
		{
			surface = getImageFromRes("ANTIAIR", i);
			dst_rect.x = (i - 1)*64;
			SDL_BlitSurface(surface, &src_rect, output, &dst_rect);
			SDL_FreeSurface( surface );
		}
		
		savePCX( output, path + "img.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "img.pcx")
	
	copyFileFromRes("S_ANTIAI", path + "shw.pcx", 8); 
	copyFileFromRes("P_FXAA", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "AA_ISO.FLC", path + "video.pcx");

	//gun ari
	path = sOutputPath + "buildings" + PATH_DELIMITER + "gun_ari" + PATH_DELIMITER;
	try
		{
		surface = getImageFromRes("ARTYTRRT");
		removePlayerColor( surface );
		output = SDL_CreateRGBSurface(SDL_SWSURFACE, 512, 64,8,0,0,0,0);
		SDL_SetColors(output, surface->format->palette->colors, 0, 256);
		SDL_FillRect( output, 0, SDL_MapRGB( output->format, 255, 0, 255));
		dst_rect.y = 0;
		dst_rect.x = 0;
		src_rect.x = 16;
		src_rect.y = 16;
		src_rect.h = 64;
		src_rect.w = 64;
		for ( int i = 0; i < 512; i+=64 )
		{
			dst_rect.x = i;
			SDL_BlitSurface(surface, &src_rect, output, &dst_rect);
		}
		SDL_FreeSurface( surface );

		for ( int i = 1; i < 9; i++ )
		{
			surface = getImageFromRes("ARTYTRRT", i);
			dst_rect.x = (i - 1)*64;
			SDL_BlitSurface(surface, &src_rect, output, &dst_rect);
			SDL_FreeSurface( surface );
		}
		
		savePCX( output, path + "img.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "img.pcx");

	copyFileFromRes("S_ARTYTR", path + "shw.pcx", 8);
	copyFileFromRes("P_ARTYTR", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "FXDGUN.FLC", path + "video.pcx");

	//gun missile
	path = sOutputPath + "buildings" + PATH_DELIMITER + "gun_missel" + PATH_DELIMITER;
	try
	{
		surface = getImageFromRes("ANTIMSSL", 0);
		removePlayerColor( surface );
		output = SDL_CreateRGBSurface(SDL_SWSURFACE, 512, 64,8,0,0,0,0);
		SDL_SetColors(output, surface->format->palette->colors, 0, 256);
		SDL_FillRect( output, 0, SDL_MapRGB( output->format, 255, 0, 255));
		dst_rect.y = 0;
		dst_rect.x = 0;
		src_rect.x = 15;
		src_rect.y = 16;
		src_rect.h = 64;
		src_rect.w = 64;
		for ( int i = 0; i < 512; i+=64 )
		{
			dst_rect.x = i;
			SDL_BlitSurface(surface, &src_rect, output, &dst_rect);
		}
		SDL_FreeSurface( surface );

		for ( int i = 1; i < 9; i++ )
		{
			surface = getImageFromRes("ANTIMSSL", i);
			dst_rect.x = (i - 1)*64;
			SDL_BlitSurface(surface, &src_rect, output, &dst_rect);
			SDL_FreeSurface( surface );
		}
		savePCX( output, path + "img.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "img.pcx")

	copyFileFromRes("S_ANTIMS", path + "shw.pcx", 1);
	copyFileFromRes("P_FXROCK", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "ANTIMSL.FLC", path + "video.pcx");

	//gun turret
	path = sOutputPath + "buildings" + PATH_DELIMITER + "gun_turret" + PATH_DELIMITER;
	try
	{
		surface = getImageFromRes("GUNTURRT", 0);
		removePlayerColor( surface );
		output = SDL_CreateRGBSurface(SDL_SWSURFACE, 512, 64,8,0,0,0,0);
		SDL_SetColors(output, surface->format->palette->colors, 0, 256);
		SDL_FillRect( output, 0, SDL_MapRGB( output->format, 255, 0, 255));
		dst_rect.y = 0;
		dst_rect.x = 0;
		src_rect.x = 13;
		src_rect.y = 15;
		src_rect.h = 64;
		src_rect.w = 64;
		for ( int i = 0; i < 512; i+=64 )
		{
			dst_rect.x = i;
			SDL_BlitSurface(surface, &src_rect, output, &dst_rect);
		}
		SDL_FreeSurface( surface );

		for ( int i = 1; i < 9; i++ )
		{
			surface = getImageFromRes("GUNTURRT", i);
			dst_rect.x = (i - 1)*64;
			SDL_BlitSurface(surface, &src_rect, output, &dst_rect);
			SDL_FreeSurface( surface );
		}
		
		savePCX( output, path + "img.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "img.pcx")

	copyFileFromRes("S_GUNTUR", path + "shw.pcx", 1);
	copyFileFromRes("P_GUNTUR", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "FXAGUN.FLC", path + "video.pcx");

	//habitat
	path = sOutputPath + "buildings" + PATH_DELIMITER + "habitat" + PATH_DELIMITER;
	copyFileFromRes_rpc("HABITAT", path + "img.pcx", 1);
	copyFileFromRes("S_HABITA", path + "shw.pcx", 1);
	copyFileFromRes("P_HABITA", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "DORM.FLC", path + "video.pcx");

	//hangar
	path = sOutputPath + "buildings" + PATH_DELIMITER + "hangar" + PATH_DELIMITER;
	copyFileFromRes_rpc("HANGAR", path + "img.pcx", 1);
	copyFileFromRes("S_HANGAR", path + "shw.pcx", 1);
	copyFileFromRes("P_HANGAR", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "HANGR.FLC", path + "video.pcx");

	//landmine
	path = sOutputPath + "buildings" + PATH_DELIMITER + "landmine" + PATH_DELIMITER;
	try
	{
		surface = getImageFromRes("LANDMINE");
		output = SDL_CreateRGBSurface(SDL_SWSURFACE, 64,64,8,0,0,0,0);
		SDL_SetColors(output, surface->format->palette->colors, 0, 256);
		SDL_FillRect( output, 0, SDL_MapRGB( output->format, 255, 0, 255));
		dst_rect.x = 22;
		dst_rect.y = 22;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		SDL_FreeSurface( surface );
		savePCX(output, path + "img.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "img.pcx")

	try
	{
		surface = getImageFromRes("S_LANDMI");
		output = SDL_CreateRGBSurface(SDL_SWSURFACE, 42,41,8,0,0,0,0);
		output->pitch = output->w;					//workaround for an SDL-Bug
		SDL_SetColors(output, surface->format->palette->colors, 0, 256);
		SDL_FillRect( output, 0, SDL_MapRGB( output->format, 255, 0, 255));
		dst_rect.x = 22;
		dst_rect.y = 22;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		SDL_FreeSurface( surface );
		savePCX(output, path + "shw.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "shw.pcx")

	copyFileFromRes("P_LANDMN", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "LMINE01.FLC", path + "video.pcx");

	//mine
	path = sOutputPath + "buildings" + PATH_DELIMITER + "mine" + PATH_DELIMITER;
	copyFileFromRes_rpc("MININGST", path + "img.pcx");	//this is temorary!
														//until the clans will be implemented
	copyFileFromRes("S_MINING", path + "shw.pcx");
	copyFileFromRes("P_MINING", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "MSTRSTAT.FLC", path + "video.pcx");

	//mine deep
	path = sOutputPath + "buildings" + PATH_DELIMITER + "mine_deep" + PATH_DELIMITER;
	copyFileFromRes_rpc("SUPRTPLT", path + "img.pcx");
	copyFileFromRes("S_SUPRTP", path + "shw.pcx");
	copyFileFromRes("P_LIFESP", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "SVP.FLC", path + "video.pcx");

	//pad
	path = sOutputPath + "buildings" + PATH_DELIMITER + "pad" + PATH_DELIMITER;
	copyFileFromRes_rpc("LANDPAD", path + "img.pcx");
	copyFileFromRes("S_LANDPA", path + "shw.pcx");
	copyFileFromRes("P_LANDPD", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "LP_ISO.FLC", path + "video.pcx");

	//platform
	path = sOutputPath + "buildings" + PATH_DELIMITER + "platform" + PATH_DELIMITER;
	copyFileFromRes_rpc("WTRPLTFM", path + "img.pcx");
	copyFileFromRes("S_WTRPLT", path + "shw.pcx");
	copyFileFromRes("P_WATER", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "WP.FLC", path + "video.pcx");
	
	//radar
	path = sOutputPath + "buildings" + PATH_DELIMITER + "radar" + PATH_DELIMITER;
	try
	{
		surface = getImageFromRes("RADAR", 0);
		removePlayerColor( surface );
		output = SDL_CreateRGBSurface(SDL_SWSURFACE, 1024,64,8,0,0,0,0);
		SDL_SetColors(output, surface->format->palette->colors, 0, 256);
		SDL_FillRect( output, 0, SDL_MapRGB( output->format, 255, 0, 255));
		dst_rect.x = 0;
		dst_rect.y = 0;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		SDL_FreeSurface ( surface );
		for ( int i = 1; i < 16; i++)
		{
			surface = getImageFromRes("RADAR", i);
			removePlayerColor( surface );
			dst_rect.x = i*64;
			SDL_BlitSurface(surface, 0, output, &dst_rect);
			SDL_FreeSurface( surface );
		}
		savePCX(output, path + "img.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "img.pcx")
	
	copyFileFromRes("S_RADAR", path + "shw.pcx", 14);
	copyFileFromRes("P_RADAR", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "RADAR.FLC", path + "video.pcx");

	//research
	path = sOutputPath + "buildings" + PATH_DELIMITER + "research" + PATH_DELIMITER;
	copyFileFromRes("RESEARCH", path + "img.pcx");
	copyFileFromRes("S_RESEAR", path + "shw.pcx");
	copyFileFromRes("P_RESEAR", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "RESEARCH.FLC", path + "video.pcx");

	//road
	path = sOutputPath + "buildings" + PATH_DELIMITER + "road" + PATH_DELIMITER;
	copyFileFromRes("ROAD", path + "img.pcx");
	copyFileFromRes("S_ROAD", path + "shw.pcx");
	copyFileFromRes("P_ROAD", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "ROAD.FLC", path + "video.pcx");

	//seamine
	path = sOutputPath + "buildings" + PATH_DELIMITER + "seamine" + PATH_DELIMITER;
	try
	{
		surface = getImageFromRes("SEAMINE");
		output = SDL_CreateRGBSurface(SDL_SWSURFACE, 42,41,8,0,0,0,0);
		output->pitch = output->w;					//workaround for an SDL-Bug
		SDL_SetColors(output, surface->format->palette->colors, 0, 256);
		SDL_FillRect( output, 0, SDL_MapRGB( output->format, 255, 0, 255));
		dst_rect.x = 23;
		dst_rect.y = 23;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		SDL_FreeSurface( surface );
		savePCX(output, path + "img.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "img.pcx")

	//seamines don't have a shadow in the original. So creating a dummy file here
	try
	{
		surface = getImageFromRes("SEAMINE");
		output = SDL_CreateRGBSurface(SDL_SWSURFACE, 42,41,8,0,0,0,0);
		output->pitch = output->w;					//workaround for an SDL-Bug
		SDL_SetColors(output, surface->format->palette->colors, 0, 256);
		SDL_FillRect( output, 0, SDL_MapRGB( output->format, 255, 0, 255));
		SDL_FreeSurface( surface );
		savePCX(output, path + "shw.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "shw.pcx")
	
	copyFileFromRes("P_SEAMIN", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "SMINE01.FLC", path + "video.pcx");

	//shield
	path = sOutputPath + "buildings" + PATH_DELIMITER + "shield" + PATH_DELIMITER;
	copyFileFromRes_rpc("SHIELDGN", path + "img.pcx");
	copyFileFromRes("S_SHIELD", path + "shw.pcx");
	copyFileFromRes("P_SHIELD", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "SHLDGEN.FLC", path + "video.pcx");

	//storage gold
	path = sOutputPath + "buildings" + PATH_DELIMITER + "storage_gold" + PATH_DELIMITER;
	copyFileFromRes_rpc("GOLDSM", path + "img.pcx");
	copyFileFromRes("S_GOLDSM", path + "shw.pcx");
	copyFileFromRes("P_SMVLT", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "GOLDSM.FLC", path + "video.pcx");

	//storage metal
	path = sOutputPath + "buildings" + PATH_DELIMITER + "storage_metal" + PATH_DELIMITER;
	copyFileFromRes_rpc("ADUMP", path + "img.pcx");
	copyFileFromRes("S_ADUMP", path + "shw.pcx");
	copyFileFromRes("P_SMSTOR", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "SS_ISO.FLC", path + "video.pcx");

	//storage oil
	path = sOutputPath + "buildings" + PATH_DELIMITER + "storage_oil" + PATH_DELIMITER;
	copyFileFromRes_rpc("FDUMP", path + "img.pcx");
	copyFileFromRes("S_FDUMP", path + "shw.pcx");
	copyFileFromRes("P_SMFUEL", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "SF_ISO.FLC", path + "video.pcx");

	//training
	path = sOutputPath + "buildings" + PATH_DELIMITER + "training" + PATH_DELIMITER;
	copyFileFromRes_rpc("TRAINHAL", path + "img.pcx");
	copyFileFromRes("S_TRAINH", path + "shw.pcx");
	copyFileFromRes("P_TRNHLL", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "THALL.FLC", path + "video.pcx");

	//rubble
	path = sOutputPath + "buildings" + PATH_DELIMITER;
	try
	{
		output = getImageFromRes( "LRGRUBLE", 0);
		resizeSurface( output, 0, 0, 256, 128 );
		surface = getImageFromRes( "LRGRUBLE", 1);
		SDL_Rect dst_rect = { 128, 0, 0, 0 };
		SDL_BlitSurface ( surface, NULL, output, &dst_rect );
		SDL_FreeSurface ( surface );
		savePCX( output, path + "dirt_big.pcx");
		SDL_FreeSurface ( output );
	}
	END_INSTALL_FILE( path + "dirt_big.pcx");

	try
	{
		output = getImageFromRes( "SMLRUBLE", 0);
		resizeSurface( output, 0, 0, 320, 64 );
		SDL_Rect dst_rect = { 64, 0, 0, 0 };
		for ( int i = 1; i < 5 ; i++ )
		{
			surface = getImageFromRes( "SMLRUBLE", i );
			SDL_BlitSurface( surface, NULL, output, &dst_rect);
			SDL_FreeSurface( surface );
			dst_rect.x += 64;
		}
		savePCX( output, path + "dirt_small.pcx");
	}
	END_INSTALL_FILE(path + "dirt_small.pcx");

	try
	{
		output = getImageFromRes( "S_LRGRBL", 0);
		resizeSurface( output, 0, 0, 256, 128 );
		surface = getImageFromRes( "S_LRGRBL", 1);
		SDL_Rect dst_rect = { 128, 0, 0, 0 };
		SDL_BlitSurface ( surface, NULL, output, &dst_rect );
		SDL_FreeSurface ( surface );
		savePCX( output, path + "dirt_big_shw.pcx");
		SDL_FreeSurface ( output );
	}
	END_INSTALL_FILE( path + "dirt_big_shw.pcx");

	try
	{
		output = getImageFromRes( "S_SMLRBL", 0);
		resizeSurface( output, 0, 0, 320, 64 );
		SDL_Rect dst_rect = { 64, 0, 0, 0 };
		for ( int i = 1; i < 5 ; i++ )
		{
			surface = getImageFromRes( "S_SMLRBL", i );
			SDL_BlitSurface( surface, NULL, output, &dst_rect);
			SDL_FreeSurface( surface );
			dst_rect.x += 64;
		}
		savePCX( output, path + "dirt_small_shw.pcx");
	}
	END_INSTALL_FILE(path + "dirt_small_shw.pcx");


	cout << "\n";
	cout << iToStr( iErrors) << " errors\n";

	if ( logFile != NULL )
	{
		writeLog( string("Building graphics") + TEXT_FILE_LF);
		writeLog( iToStr( iErrors) + " errors" + TEXT_FILE_LF);
		writeLog( string("========================================================================") + TEXT_FILE_LF);
	}

	return 1;
}

int installVehicleVideos()
{
	string path;
	iTotalFiles = 35;
	iErrors = 0;
	iInstalledFiles = 0;



	cout << "========================================================================\n";
	cout << "Vehicle videos\n";

	//air_transport
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "air_transport" + PATH_DELIMITER;
	copyFile( sMAXPath + "ATRANS02.FLC", path + "video.flc" );

	//alien_assault
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "alien_assault" + PATH_DELIMITER;
	copyFile( sMAXPath + "ALNASG.FLC", path + "video.flc" );
	
	//alien_plane
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "alien_plane" + PATH_DELIMITER;
	copyFile( sMAXPath + "ALNPLANE.FLC", path + "video.flc" );
	
	//alien_ship
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "alien_ship" + PATH_DELIMITER;
	copyFile( sMAXPath + "JUGGERN.FLC", path + "video.flc" );
	
	//alien_tank
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "alien_tank" + PATH_DELIMITER;
	copyFile( sMAXPath + "ALNTANK.FLC", path + "video.flc" );
	
	//apc
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "apc" + PATH_DELIMITER;
	copyFile( sMAXPath + "APC_TR01.FLC", path + "video.flc" );
	
	//assault
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "assault" + PATH_DELIMITER;
	copyFile( sMAXPath + "E_ART2.FLC", path + "video.flc" );
	
	//awac
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "awac" + PATH_DELIMITER;
	copyFile( sMAXPath + "AWACS03.FLC", path + "video.flc" );
	
	//bomber
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "bomber" + PATH_DELIMITER;
	copyFile( sMAXPath + "BOMBER03.FLC", path + "video.flc" );
	
	//bulldozer
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "bulldozer" + PATH_DELIMITER;
	copyFile( sMAXPath + "DOZER01.FLC", path + "video.flc" );
	
	//cargoship
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "cargoship" + PATH_DELIMITER;
	copyFile( sMAXPath + "SCARGO02.FLC", path + "video.flc" );
	
	//cluster
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "cluster" + PATH_DELIMITER;
	copyFile( sMAXPath + "MML.FLC", path + "video.flc" );
	
	//commando
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "commando" + PATH_DELIMITER;
	copyFile( sMAXPath + "AGT.FLC", path + "video.flc" );
	
	//corvet
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "corvet" + PATH_DELIMITER;
	copyFile( sMAXPath + "CORVETTE.FLC", path + "video.flc" );
	
	//escort
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "escort" + PATH_DELIMITER;
	copyFile( sMAXPath + "ESCORT.FLC", path + "video.flc" );
	
	//fighter
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "fighter" + PATH_DELIMITER;
	copyFile( sMAXPath + "FIGHTER.FLC", path + "video.flc" );
	
	//gunboat
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "gunboat" + PATH_DELIMITER;
	copyFile( sMAXPath + "HG1.FLC", path + "video.flc" );
	
	//infantery
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "infantery" + PATH_DELIMITER;
	copyFile( sMAXPath + "INFANTRY.FLC", path + "video.flc" );
		
	//konstrukt
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "konstrukt" + PATH_DELIMITER;
	copyFile( sMAXPath + "CONSTRCT.FLC", path + "video.flc" );
	
	//minelayer
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "minelayer" + PATH_DELIMITER;
	copyFile( sMAXPath + "MINELAY.FLC", path + "video.flc" );
	
	//missel
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "missel" + PATH_DELIMITER;
	copyFile( sMAXPath + "MISSLE_L.FLC", path + "video.flc" );
	
	//missel_ship
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "missel_ship" + PATH_DELIMITER;
	copyFile( sMAXPath + "MB5.FLC", path + "video.flc" );
		
	//mobile_aa
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "mobile_aa" + PATH_DELIMITER;
	copyFile( sMAXPath + "MAA.FLC", path + "video.flc" );
	
	//pionier
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "pionier" + PATH_DELIMITER;
	copyFile( sMAXPath + "ENGINEER.FLC", path + "video.flc" );
	
	//repair
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "repair" + PATH_DELIMITER;
	copyFile( sMAXPath + "REPAIR02.FLC", path + "video.flc" );
	
	//scanner
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "scanner" + PATH_DELIMITER;
	copyFile( sMAXPath + "SCANR1.FLC", path + "video.flc" );
	
	//scout
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "scout" + PATH_DELIMITER;
	copyFile( sMAXPath + "SCOUT.FLC", path + "video.flc" );
	
	//sea_minelayer
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "sea_minelayer" + PATH_DELIMITER;
	copyFile( sMAXPath + "SML.FLC", path + "video.flc" );
	
	//sea_transport
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "sea_transport" + PATH_DELIMITER;
	copyFile( sMAXPath + "SEATRANS.FLC", path + "video.flc" );
	
	//sub
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "sub" + PATH_DELIMITER;
	copyFile( sMAXPath + "SUB.FLC", path + "video.flc" );
	
	//surveyor
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "surveyor" + PATH_DELIMITER;
	copyFile( sMAXPath + "SURVEYOR.FLC", path + "video.flc" );
	
	//tank
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "tank" + PATH_DELIMITER;
	copyFile( sMAXPath + "TANK03.FLC", path + "video.flc" );
	
	//trans_gold
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "trans_gold" + PATH_DELIMITER;
	copyFile( sMAXPath + "MCHARGE.FLC", path + "video.flc" );
	
	//trans_metal
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "trans_metal" + PATH_DELIMITER;
	copyFile( sMAXPath + "TRUCK.FLC", path + "video.flc" );
	
	//trans_oil
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "trans_oil" + PATH_DELIMITER;
	copyFile( sMAXPath + "MGENR.FLC", path + "video.flc" );
	
	cout << "\n";
	cout << iToStr( iErrors) << " errors\n";
	
	if ( logFile != NULL )
	{
		writeLog( string("Vehicle videos") + TEXT_FILE_LF);
		writeLog( iToStr( iErrors) + " errors" + TEXT_FILE_LF);
		writeLog( string("========================================================================") + TEXT_FILE_LF);
	}

	return 1;
}

int installFX()
{
	string path;
	SDL_Surface *surface, *output;
	iTotalFiles = 8;
	iErrors = 0;
	iInstalledFiles = 0;

	cout << "========================================================================\n";
	cout << "Fx\n";

	path = sOutputPath + "fx" + PATH_DELIMITER;

	//waldo
	try
	{
		surface = getImageFromRes("WALDO");
		resizeSurface( surface, 8, 7, 64, 64);
		savePCX( surface, path + "corpse.pcx");
		SDL_FreeSurface( surface );
	}
	END_INSTALL_FILE( path + "corpse.pcx" )

	//hit
	try
	{
		output = getImageFromRes("HITEXPLD", 0);
		resizeSurface( output, 13, 15, 320, 64);
		
		SDL_Rect dst_rect = { 64, 15, 0, 0 };
		for ( int i = 1; i < 5; i++)
		{
			surface = getImageFromRes("HITEXPLD", i );
			SDL_BlitSurface( surface, NULL, output, &dst_rect);
			SDL_FreeSurface( surface );
			dst_rect.x += 64;
		}
		
		savePCX( output, path + "hit.pcx"); 
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "hit.pcx" )

	//explo_air
	try
	{
		output = getImageFromRes("AIREXPLD", 0);
		resizeSurface( output, 0, 0, 1918, 121 );

		SDL_Rect dst_rect = { 137, 0, 0, 0 };
		for ( int i = 1; i < 14; i++)
		{
			surface = getImageFromRes("AIREXPLD", i );
			SDL_BlitSurface( surface, NULL, output, &dst_rect);
			SDL_FreeSurface( surface );
			dst_rect.x += 137;
		}
		
		savePCX( output, path + "explo_air.pcx"); 
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "explo_air.pcx" )

	//explo_big
	try
	{
		output = getImageFromRes("BLDEXPLD", 0);
		resizeSurface( output, 0, 0, 8596, 194 );

		SDL_Rect dst_rect = { 307, 0, 0, 0 };
		for ( int i = 1; i < 28; i++)
		{
			surface = getImageFromRes("BLDEXPLD", i );
			SDL_BlitSurface( surface, NULL, output, &dst_rect);
			SDL_FreeSurface( surface );
			dst_rect.x += 307;
		}
		
		savePCX( output, path + "explo_big.pcx"); 
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "explo_big.pcx" )

	//explo_small
	try
	{
		output = getImageFromRes("LNDEXPLD", 0);
		resizeSurface( output, 0, 0, 1596, 108 );

		SDL_Rect dst_rect = { 114, 0, 0, 0 };
		for ( int i = 1; i < 14; i++)
		{
			surface = getImageFromRes("LNDEXPLD", i );
			SDL_BlitSurface( surface, NULL, output, &dst_rect);
			SDL_FreeSurface( surface );
			dst_rect.x += 114;
		}
		
		savePCX( output, path + "explo_small.pcx"); 
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "explo_small.pcx" )

	//explo_water
	try
	{
		output = getImageFromRes("SEAEXPLD", 0);
		resizeSurface( output, 0, 0, 1596, 108 );

		SDL_Rect dst_rect = { 114, 0, 0, 0 };
		for ( int i = 1; i < 14; i++)
		{
			surface = getImageFromRes("SEAEXPLD", i );
			SDL_BlitSurface( surface, NULL, output, &dst_rect);
			SDL_FreeSurface( surface );
			dst_rect.x += 114;
		}
		
		savePCX( output, path + "explo_water.pcx"); 
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "explo_water.pcx" )

	//rocket
	try
	{
		output = getImageFromRes( "ROCKET", 0);
		resizeSurface(output, 0, 0, 224, 28);
		SDL_Rect dst_rect = { 28, 0, 0, 0 };
		for ( int i = 1; i < 8; i++ )
		{
			surface = getImageFromRes("ROCKET", 2*i );
			SDL_BlitSurface( surface, NULL, output, &dst_rect);
			SDL_FreeSurface( surface );
			dst_rect.x += 28;
		}
		
		savePCX( output, path + "rocket.pcx"); 
		SDL_FreeSurface( output );

	}
	END_INSTALL_FILE( path + "rocket.pcx" )

	//torpedo???

	//saveload.flc
	copyFile( sMAXPath + "SAVELOAD.FLC", path + "saveload.flc" );



	if ( logFile != NULL )
	{
		writeLog( string("Fx") + TEXT_FILE_LF);
		writeLog( iToStr( iErrors) + " errors" + TEXT_FILE_LF);
		writeLog( string("========================================================================") + TEXT_FILE_LF);
	}

	cout << "\n";
	cout << iToStr( iErrors) << " errors\n";

	return 1;
}

int installGfx()
{
	string path;
	SDL_Surface *surface, *output;
	iTotalFiles = 32;
	iErrors = 0;
	iInstalledFiles = 0;

	cout << "========================================================================\n";
	cout << "Gfx\n";

	path = sOutputPath + "gfx" + PATH_DELIMITER;

	//activate
	try
	{
		output = getImageFromRes("ACTVTPTR");
		setColor( output, 0, 255, 0, 255 );
		savePCX( output, path + "activate.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "activate.pcx" );

	//attack
	try
	{
		output = getImageFromRes("ENMY_PTR");
		setColor( output, 77, 255, 0, 255 );
		savePCX( output, path + "attack.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "attack.pcx" );

	try
	{
		output = getImageFromRes("BLDMRK1");
		setColor( output, 0, 255, 0, 255 );
		resizeSurface( output, 1, 1, 320, 64 );

		surface = getImageFromRes("BLDMRK2");
		setColor( surface, 0, 255, 0, 255 );
		SDL_Rect dst_rect = { 65, 1, 0, 0 };
		SDL_BlitSurface( surface, NULL, output, &dst_rect );
		SDL_FreeSurface( surface );

		surface = getImageFromRes("BLDMRK3");
		setColor( surface, 0, 255, 0, 255 );
		dst_rect.x += 64;
		SDL_BlitSurface( surface, NULL, output, &dst_rect );
		SDL_FreeSurface( surface );

		surface = getImageFromRes("BLDMRK4");
		setColor( surface, 0, 255, 0, 255 );
		dst_rect.x += 64;
		SDL_BlitSurface( surface, NULL, output, &dst_rect );
		SDL_FreeSurface( surface );

		surface = getImageFromRes("BLDMRK5");
		setColor( surface, 0, 255, 0, 255 );
		dst_rect.x += 64;
		SDL_BlitSurface( surface, NULL, output, &dst_rect );
		SDL_FreeSurface( surface );

		savePCX( output, path + "activate_field.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "activate_field.pcx" );

	//band_big
	copyFileFromRes("LRGTAPE", path + "band_big.pcx", 0);

	//band_big_water
	//copyFileFromRes("LRGTAPE", path + "band_big_water.pcx", 1);

	//band_small
	copyFileFromRes("SMLTAPE", path + "band_small.pcx", 0);

	//band_small_water
	//copyFileFromRes("SMLTAPE", path + "band_big_water.pcx", 1);

	//band_cur
	try
	{
		output = getImageFromRes("MAP_PTR");
		setColor( output, 0, 255, 0, 255 );
		savePCX( output, path + "band_cur.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "band_cur.pcx" );

	//big_beton
	try
	{
		output = getImageFromRes("LRGSLAB");
		resizeSurface(output, 0, 0, 128, 128);
		savePCX( output, path + "big_beton.pcx");
	}
	END_INSTALL_FILE( path + "big_beton.pcx");
	
	//disable
	try
	{
		output = getImageFromRes("DISBLPTR");
		setColor( output, 77, 255, 0, 255 );
		savePCX( output, path + "disable.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "disable.pcx" );

	//edock
	copyFileFromRes("E_DOCK", path + "edock.pcx");

	//edepot
	copyFileFromRes("E_DEPOT", path + "edepot.pcx");

	//ehangar
	copyFileFromRes("E_HANGAR", path + "ehangar.pcx");

	//hand
	try
	{
		output = getImageFromRes("HANDPTR");
		setColor( output, 0, 255, 0, 255 );
		savePCX( output, path + "hand.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "hand.pcx" );

	//help
	try
	{
		output = getImageFromRes("PTR_HELP");
		setColor( output, 0, 255, 0, 255 );
		savePCX( output, path + "help.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "help.pcx" );

	//load
	try
	{
		output = getImageFromRes("FRND_LOD");
		setColor( output, 0, 255, 0, 255 );
		savePCX( output, path + "load.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "load.pcx" );

	//move
	try
	{
		output = getImageFromRes("UNIT_GO");
		setColor( output, 0, 255, 0, 255 );
		savePCX( output, path + "move.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "move.pcx" );

	//muni
	try
	{
		output = getImageFromRes("PTR_RLD");
		setColor( output, 0, 255, 0, 255 );
		savePCX( output, path + "muni.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "muni.pcx" );

	//no
	try
	{
		output = getImageFromRes("UNIT_NGO");
		setColor( output, 0, 255, 0, 255 );
		savePCX( output, path + "no.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "no.pcx" );

	//object_manu
	try
	{
		output = getImageFromRes("UNTBTN_U");
		setColor( output, 0, 255, 0, 255 );
		resizeSurface( output, 0, 0, 42, 42 );

		surface = getImageFromRes("UNTBTN_D");
		setColor( surface, 0, 255, 0, 255 );
		SDL_Rect dst_rect = { 0, 21, 0, 0};
		SDL_BlitSurface( surface, NULL, output, &dst_rect );
		SDL_FreeSurface( surface );

		savePCX( output, path + "object_menu2.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "object_menu2.pcx" );

	//pf_x
	try
	{
		output = getImageFromRes("ARROW_SW");
		setColor( output, 1, 255, 0, 255 );
		savePCX( output, path + "pf_1.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "pf_1.pcx" );

	try
	{
		output = getImageFromRes("ARROW_S");
		setColor( output, 1, 255, 0, 255 );
		savePCX( output, path + "pf_2.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "pf_2.pcx" );
	try
	{
		output = getImageFromRes("ARROW_SE");
		setColor( output, 1, 255, 0, 255 );
		savePCX( output, path + "pf_3.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "pf_3.pcx" );
	try
	{
		output = getImageFromRes("ARROW_E");
		setColor( output, 1, 255, 0, 255 );
		savePCX( output, path + "pf_6.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "pf_6.pcx" );
	try
	{
		output = getImageFromRes("ARROW_NE");
		setColor( output, 1, 255, 0, 255 );
		savePCX( output, path + "pf_9.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "pf_9.pcx" );
	try
	{
		output = getImageFromRes("ARROW_N");
		setColor( output, 1, 255, 0, 255 );
		savePCX( output, path + "pf_8.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "pf_8.pcx" );
	try
	{
		output = getImageFromRes("ARROW_NW");
		setColor( output, 1, 255, 0, 255 );
		savePCX( output, path + "pf_7.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "pf_7.pcx" );
	try
	{
		output = getImageFromRes("ARROW_W");
		setColor( output, 1, 255, 0, 255 );
		savePCX( output, path + "pf_4.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "pf_4.pcx" );

	//transfer
	try
	{
		output = getImageFromRes("FRND_XFR");
		setColor( output, 0, 255, 0, 255 );
		savePCX( output, path + "transf.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "transf.pcx" );

	//repair
	try
	{
		output = getImageFromRes("FRND_FIX");
		setColor( output, 0, 255, 0, 255 );
		savePCX( output, path + "repair.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "repair.pcx" );

	//steal
	try
	{
		output = getImageFromRes("STEALPTR");
		setColor( output, 77, 255, 0, 255 );
		savePCX( output, path + "steal.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "steal.pcx" )

	//select
	try
	{
		output = getImageFromRes("FRND_PTR");
		setColor( output, 0, 255, 0, 255 );
		savePCX( output, path + "select.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "select.pcx" )

	//res
	try
	{
		output = getImageFromRes("RAWMSK0");
		setColor( output, 255, 255, 0, 255 );
		resizeSurface( output, 13, 13, 1088, 64 );

		for ( int i = 1; i < 17; i++)
		{
			surface = getImageFromRes("RAWMSK" + iToStr(i) );
			setColor( surface, 48, 255, 0, 255 );
			SDL_Rect dst_rect = { 1 + i*64, 1, 0, 0};
			SDL_BlitSurface( surface, NULL, output, &dst_rect);
			SDL_FreeSurface( surface );
		}

		savePCX( output, path + "res.pcx" );
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "res.pcx" )
	
	/*
	//fuel
	try
	{
		output = getImageFromRes("FUELMK0");
		setColor( output, 255, 255, 0, 255 );
		resizeSurface( output, 13, 13, 1088, 64 );

		for ( int i = 1; i < 17; i++)
		{
			surface = getImageFromRes("FUELMK" + iToStr(i) );
			setColor( surface, 48, 255, 0, 255 );
			SDL_Rect dst_rect = { 1 + i*64, 1, 0, 0};
			SDL_BlitSurface( surface, NULL, output, &dst_rect);
			SDL_FreeSurface( surface );
		}

		savePCX( output, path + "fuel.pcx" );
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "fuel.pcx" )

	//gold
	try
	{
		output = getImageFromRes("GOLDMK0");
		setColor( output, 255, 255, 0, 255 );
		resizeSurface( output, 13, 13, 1088, 64 );

		for ( int i = 1; i < 17; i++)
		{
			surface = getImageFromRes("GOLDMK" + iToStr(i) );
			setColor( surface, 48, 255, 0, 255 );
			SDL_Rect dst_rect = { 1 + i*64, 1, 0, 0};
			SDL_BlitSurface( surface, NULL, output, &dst_rect);
			SDL_FreeSurface( surface );
		}

		savePCX( output, path + "gold.pcx" );
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "gold.pcx" )
	*/

	//and now the ugly hud_stuff.pcx :|
#define COPY_GRAPHIC(name, _x, _y) \
	surface = getImageFromRes( name ); \
	setColor( surface, backgroundIndex, 255, 0, 255 ); \
	dst_rect.x = (_x); \
	dst_rect.y = (_y); \
	SDL_BlitSurface( surface, NULL, output, &dst_rect ); \
	SDL_FreeSurface( surface );

	try
	{
		output = loadPCX( path + "hud_stuff.pcx");
		output->pitch = output->w;
		SDL_Rect dst_rect = {0, 0, 0, 0};

		int backgroundIndex = 17;
		COPY_GRAPHIC("I_HITS",   11, 109);
		backgroundIndex = 0;
		COPY_GRAPHIC("EI_AMMO",  55, 98);
		COPY_GRAPHIC("EI_SHOTS", 96, 98);
		COPY_GRAPHIC("EI_POWER", 81, 98);
		COPY_GRAPHIC("EI_SPEED",  7, 98);
		COPY_GRAPHIC("SI_POWER", 74, 98);
		COPY_GRAPHIC("SI_SPEED",  0, 98);
		COPY_GRAPHIC("EI_HITSB", 20, 98);
		COPY_GRAPHIC("EI_HITSR", 44, 98);
		COPY_GRAPHIC("EI_HITSY", 32, 98);
		COPY_GRAPHIC("SI_HITSB", 14, 98);
		COPY_GRAPHIC("SI_HITSR", 38, 98);
		COPY_GRAPHIC("SI_HITSY", 26, 98);
		COPY_GRAPHIC("EI_FUEL", 112, 98);
		COPY_GRAPHIC("EI_WORK", 178, 98);
		COPY_GRAPHIC("SI_FUEL", 104, 98);
		COPY_GRAPHIC("SI_WORK", 170, 98);
		COPY_GRAPHIC("EI_GOLD", 129, 98);
		COPY_GRAPHIC("SI_GOLD", 120, 98);
		COPY_GRAPHIC("EI_RAW",   67, 98);
		COPY_GRAPHIC("SI_RAW",   60, 98);
		COPY_GRAPHIC("SI_AMMO",  50, 98);
		COPY_GRAPHIC("I_SHOTS",  37, 109);
		COPY_GRAPHIC("EI_LAND", 154, 98);
		COPY_GRAPHIC("SI_LAND", 138, 98);
		COPY_GRAPHIC("I_AMMO",	 18, 109);
		COPY_GRAPHIC("I_GOLD",  112, 109);
		COPY_GRAPHIC("I_FUEL",  101, 109);
		COPY_GRAPHIC("I_SPEED",   0, 109);
		COPY_GRAPHIC("I_HRDATK", 27, 109);
		COPY_GRAPHIC("I_ARMOR",  65, 109);
		COPY_GRAPHIC("EI_AIR",  207, 98);
		COPY_GRAPHIC("SI_AIR",  186, 98);
		COPY_GRAPHIC("I_RANGE",  52, 109);
		COPY_GRAPHIC("I_SCAN",   76, 109);
		COPY_GRAPHIC("I_RAW",    89, 109);
		COPY_GRAPHIC("I_LIFE",  138, 109);
		COPY_GRAPHIC("I_POWER", 125, 109);
		COPY_GRAPHIC("BARTAPE", 156, 307);

		surface = getImageFromRes("SI_SHOTS");
		setColor( surface, 0, 255, 0, 255 );
		setColor( surface, 9, 255, 0, 255 );
		dst_rect.x = 88;
		dst_rect.y = 98;
		SDL_BlitSurface( surface, NULL, output, &dst_rect );
		SDL_FreeSurface( surface );
		
		SDL_Rect src_rect;

		surface = getImageFromRes("SMBRFUEL");
		setColor( surface, 0, 255, 0, 255 );
		src_rect.x = 119;
		src_rect.y = 0;
		src_rect.h = 16;
		src_rect.w = 223;
		dst_rect.x = 156;
		dst_rect.y = 273;
		SDL_BlitSurface( surface, &src_rect, output, &dst_rect );
		SDL_FreeSurface( surface );

		surface = getImageFromRes("SMBRGOLD");
		setColor( surface, 0, 255, 0, 255 );
		src_rect.x = 119;
		src_rect.y = 0;
		src_rect.h = 16;
		src_rect.w = 223;
		dst_rect.x = 156;
		dst_rect.y = 290;
		SDL_BlitSurface( surface, &src_rect, output, &dst_rect );
		SDL_FreeSurface( surface );

		surface = getImageFromRes("SMBRRAW");
		setColor( surface, 0, 255, 0, 255 );
		src_rect.x = 62;
		src_rect.y = 0;
		src_rect.h = 16;
		src_rect.w = 223;
		dst_rect.x = 156;
		dst_rect.y = 256;
		SDL_BlitSurface( surface, &src_rect, output, &dst_rect );
		SDL_FreeSurface( surface );

		surface = getImageFromRes("BARRAW");
		setColor( surface, 17, 255, 0, 255 );
		src_rect.x = 1;
		src_rect.y = 0;
		src_rect.h = 31;
		src_rect.w = 1000; //?
		dst_rect.x = 156;
		dst_rect.y = 338;
		SDL_BlitSurface( surface, &src_rect, output, &dst_rect );
		SDL_FreeSurface( surface );

		surface = getImageFromRes("BARFUEL");
		setColor( surface, 0, 255, 0, 255 );
		src_rect.x = 33;
		src_rect.y = 0;
		src_rect.h = 30; 
		src_rect.w = 1000; //?
		dst_rect.x = 156;
		dst_rect.y = 369;
		SDL_BlitSurface( surface, &src_rect, output, &dst_rect );
		SDL_FreeSurface( surface );

		surface = getImageFromRes("BARGOLD");
		setColor( surface, 0, 255, 0, 255 );
		src_rect.x = 1;
		src_rect.y = 0;
		src_rect.h = 30; 
		src_rect.w = 1000; //?
		dst_rect.x = 156;
		dst_rect.y = 400;
		SDL_BlitSurface( surface, &src_rect, output, &dst_rect );
		SDL_FreeSurface( surface );

		surface = getImageFromRes("VERTRAW");
		setColor( surface, 0, 255, 0, 255 );
		src_rect.x = 0;
		src_rect.y = 0;
		src_rect.h = 115;
		src_rect.w = 20;
		dst_rect.x = 135;
		dst_rect.y = 336;
		SDL_BlitSurface( surface, &src_rect, output, &dst_rect );
		SDL_FreeSurface( surface );

		surface = getImageFromRes("VERTGOLD");
		setColor( surface, 0, 255, 0, 255 );
		src_rect.w = 20;
		src_rect.h = 115;
		src_rect.x = 0;
		src_rect.y = 0;
		dst_rect.x = 114;
		dst_rect.y = 336;
		SDL_BlitSurface( surface, &src_rect, output, &dst_rect );
		SDL_FreeSurface( surface );


		savePCX( output, path + "hud_stuff.pcx");
	}
	END_INSTALL_FILE(path + "hud_stuff.pcx");

	if ( logFile != NULL )
	{
		writeLog( string("Gfx") + TEXT_FILE_LF);
		writeLog( iToStr( iErrors) + " errors" + TEXT_FILE_LF);
		writeLog( string("========================================================================") + TEXT_FILE_LF);
	}

	cout << "\n";
	cout << iToStr( iErrors) << " errors\n";

	return 1;

}

int installBuildingSounds()
{
	string path;
	iTotalFiles = 42;
	iErrors = 0;
	iInstalledFiles = 0;

	oggEncode = 0;

	cout << "========================================================================\n";
	cout << "Building sounds\n";

	//energy big
	path = sOutputPath + "buildings" + PATH_DELIMITER + "energy_big" + PATH_DELIMITER;
	copyPartOfWAV( sMAXPath + "POWGN17.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "POWGN17.WAV", path + "running.wav", 1);
	copyWAV( sMAXPath + "POWGN18.WAV", path + "stop.wav");
	
	//energy small
	path = sOutputPath + "buildings" + PATH_DELIMITER + "energy_small" + PATH_DELIMITER;
	copyPartOfWAV( sMAXPath + "POWGN17.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "POWGN17.WAV", path + "running.wav", 1);
	copyWAV( sMAXPath + "POWGN18.WAV", path + "stop.wav");
		
	//fac air
	path = sOutputPath + "buildings" + PATH_DELIMITER + "fac_air" + PATH_DELIMITER;
	copyPartOfWAV( sMAXPath + "AUNIT17.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "AUNIT17.WAV", path + "running.wav", 1);
	copyWAV( sMAXPath + "AUNIT18.WAV", path + "stop.wav");
	
	//fac alien
	path = sOutputPath + "buildings" + PATH_DELIMITER + "fac_alien" + PATH_DELIMITER;
	copyPartOfWAV( sMAXPath + "LVP17.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "LVP17.WAV", path + "running.wav", 1);
	copyWAV( sMAXPath + "LVP18.WAV", path + "stop.wav");
	
	//fac big
	path = sOutputPath + "buildings" + PATH_DELIMITER + "fac_big" + PATH_DELIMITER;
	copyPartOfWAV( sMAXPath + "HVP17.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "HVP17.WAV", path + "running.wav", 1);
	copyWAV( sMAXPath + "HVP18.WAV", path + "stop.wav");
	
	//fac ship
	path = sOutputPath + "buildings" + PATH_DELIMITER + "fac_ship" + PATH_DELIMITER;
	copyPartOfWAV( sMAXPath + "HVP17.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "HVP17.WAV", path + "running.wav", 1);
	copyWAV( sMAXPath + "HVP18.WAV", path + "stop.wav");
	
	//fac small
	path = sOutputPath + "buildings" + PATH_DELIMITER + "fac_small" + PATH_DELIMITER;
	copyPartOfWAV( sMAXPath + "LVP17.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "LVP17.WAV", path + "running.wav", 1);
	copyWAV( sMAXPath + "LVP18.WAV", path + "stop.wav");
	
	//goldraff
	path = sOutputPath + "buildings" + PATH_DELIMITER + "goldraff" + PATH_DELIMITER;
	copyPartOfWAV( sMAXPath + "MONOP17.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "MONOP17.WAV", path + "running.wav", 1);
	copyWAV( sMAXPath + "MONOP18.WAV", path + "stop.wav");
	
	//gun aa
	path = sOutputPath + "buildings" + PATH_DELIMITER + "gun_aa" + PATH_DELIMITER;
	copyWAV( sMAXPath + "FANTI14.WAV", path + "attack.wav");
	
	//gun ari
	path = sOutputPath + "buildings" + PATH_DELIMITER + "gun_ari" + PATH_DELIMITER;
	copyWAV( sMAXPath + "FARTY14.WAV", path + "attack.wav");
	
	//gun missile
	path = sOutputPath + "buildings" + PATH_DELIMITER + "gun_missel" + PATH_DELIMITER;
	copyWAV( sMAXPath + "MISLFIRE.WAV", path + "attack.wav");

	//gun turret
	path = sOutputPath + "buildings" + PATH_DELIMITER + "gun_turret" + PATH_DELIMITER;
	copyWAV( sMAXPath + "CANFIRE.WAV", path + "attack.wav");
	
	//landmine
	path = sOutputPath + "buildings" + PATH_DELIMITER + "landmine" + PATH_DELIMITER;
	copyWAV( sMAXPath + "EXPSDIRT.WAV", path + "attack.wav");
	
	//mine
	path = sOutputPath + "buildings" + PATH_DELIMITER + "mine" + PATH_DELIMITER;
	copyPartOfWAV( sMAXPath + "HVP17.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "HVP17.WAV", path + "running.wav", 1);
	copyWAV( sMAXPath + "MSTAT18.WAV", path + "stop.wav");

	//mine deep
	path = sOutputPath + "buildings" + PATH_DELIMITER + "mine_deep" + PATH_DELIMITER;
	copyPartOfWAV( sMAXPath + "HVP17.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "HVP17.WAV", path + "running.wav", 1);
	copyWAV( sMAXPath + "MSTAT18.WAV", path + "stop.wav");
	
	//radar
	//path = sOutputPath + "buildings" + PATH_DELIMITER + "radar" + PATH_DELIMITER;
	//copyWAV( sMAXPath + "RADAR13.WAV", path + "running.wav");
	
	//research
	path = sOutputPath + "buildings" + PATH_DELIMITER + "research" + PATH_DELIMITER;
	copyPartOfWAV( sMAXPath + "RESEAR17.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "RESEAR17.WAV", path + "running.wav", 1);
	copyWAV( sMAXPath + "RESEAR18.WAV", path + "stop.wav");

	//seamine
	path = sOutputPath + "buildings" + PATH_DELIMITER + "seamine" + PATH_DELIMITER;
	copyWAV( sMAXPath + "EPLOWET1.WAV", path + "attack.wav");
	
	//shield
	//path = sOutputPath + "buildings" + PATH_DELIMITER + "shield" + PATH_DELIMITER;

	//training
	path = sOutputPath + "buildings" + PATH_DELIMITER + "training" + PATH_DELIMITER;
	copyPartOfWAV( sMAXPath + "LVP17.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "LVP17.WAV", path + "running.wav", 1);
	copyWAV( sMAXPath + "LVP18.WAV", path + "stop.wav");

	if ( logFile != NULL )
	{
		writeLog( string("Building sounds") + TEXT_FILE_LF);
		writeLog( iToStr( iErrors) + " errors" + TEXT_FILE_LF);
		writeLog( string("========================================================================") + TEXT_FILE_LF);
	}

	cout << "\n";
	cout << iToStr( iErrors) << " errors\n";
	return 1;
}

int installVehicleSounds()
{
	string path;
	iTotalFiles = 173;
	iErrors = 0;
	iInstalledFiles = 0;

	oggEncode = 0;

	cout << "========================================================================\n";
	cout << "Vehicle Sounds\n";

	//air_transport
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "air_transport" + PATH_DELIMITER;
	copyWAV( sMAXPath + "ATRANS1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "ATRANS5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "ATRANS5.WAV", path + "drive.wav", 1);
	copyWAV( sMAXPath + "ATRANS7.WAV", path + "stop.wav");

	//alien_assault
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "alien_assault" + PATH_DELIMITER;
	copyWAV( sMAXPath + "ALNTK1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "ALNTK5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "ALNTK5.WAV", path + "drive.wav", 1);
	copyWAV( sMAXPath + "ALNTK7.WAV", path + "stop.wav");
	copyWAV( sMAXPath + "ASGUN14.WAV", path + "attack.wav");
		
	//alien_plane
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "alien_plane" + PATH_DELIMITER;
	copyWAV( sMAXPath + "ATTACK1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "ATTACK5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "ATTACK5.WAV", path + "drive.wav", 1);
	copyWAV( sMAXPath + "ATTACK7.WAV", path + "stop.wav");
	copyWAV( sMAXPath + "MISLFIRE.WAV", path + "attack.wav");
		
	//alien_ship
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "alien_ship" + PATH_DELIMITER;
	copyWAV( sMAXPath + "JUGGR1.WAV", path + "wait_water.wav");
	copyPartOfWAV( sMAXPath + "JUGGR5.WAV", path + "start_water.wav", 0);
	copyPartOfWAV( sMAXPath + "JUGGR5.WAV", path + "drive_water.wav", 1);
	copyWAV( sMAXPath + "JUGGR7.WAV", path + "stop_water.wav");
	copyWAV( sMAXPath + "FARTY14.WAV", path + "attack.wav");
		
	//alien_tank
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "alien_tank" + PATH_DELIMITER;
	copyWAV( sMAXPath + "ALNTK1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "ALNTK5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "ALNTK5.WAV", path + "drive.wav", 1);
	copyWAV( sMAXPath + "ALNTK7.WAV", path + "stop.wav");
	copyWAV( sMAXPath + "CANFIRE.WAV", path + "attack.wav");
		
	//apc
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "apc" + PATH_DELIMITER;
	copyWAV( sMAXPath + "APC1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "APC5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "APC5.WAV", path + "drive.wav", 1);
	copyWAV( sMAXPath + "APC7.WAV", path + "stop.wav");
	copyWAV( sMAXPath + "SUB2.WAV", path + "wait_water.wav");
	copyPartOfWAV( sMAXPath + "SUB6.WAV", path + "start_water.wav", 0);
	copyPartOfWAV( sMAXPath + "SUB6.WAV", path + "drive_water.wav", 1);
	copyWAV( sMAXPath + "SUB8.WAV", path + "stop_water.wav");
		
	//assault
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "assault" + PATH_DELIMITER;
	copyWAV( sMAXPath + "TANKA_1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "TANKA_5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "TANKA_5.WAV", path + "drive.wav", 1);
	copyWAV( sMAXPath + "TANKA_7.WAV", path + "stop.wav");
	copyWAV( sMAXPath + "ASGUN14.WAV", path + "attack.wav");
		
	//awac
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "awac" + PATH_DELIMITER;
	copyWAV( sMAXPath + "AWAC1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "AWAC5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "AWAC5.WAV", path + "drive.wav", 1);
	copyWAV( sMAXPath + "AWAC7.WAV", path + "stop.wav");
		
	//bomber
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "bomber" + PATH_DELIMITER;
	copyWAV( sMAXPath + "ATTACK1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "ATTACK5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "ATTACK5.WAV", path + "drive.wav", 1);
	copyWAV( sMAXPath + "ATTACK7.WAV", path + "stop.wav");
	copyWAV( sMAXPath + "MISLFIRE.WAV", path + "attack.wav");
		
	//bulldozer
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "bulldozer" + PATH_DELIMITER;
	copyWAV( sMAXPath + "BULL1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "BULL5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "BULL5.WAV", path + "drive.wav", 1);
	copyWAV( sMAXPath + "BULL7.WAV", path + "stop.wav");
	
	//cargoship
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "cargoship" + PATH_DELIMITER;
	copyWAV( sMAXPath + "MBOATIDL.WAV", path + "wait_water.wav");
	copyPartOfWAV( sMAXPath + "MBOATMVE.WAV", path + "start_water.wav", 0);
	copyPartOfWAV( sMAXPath + "MBOATMVE.WAV", path + "drive_water.wav", 1);
	copyWAV( sMAXPath + "MBOATSTP.WAV", path + "stop_water.wav");
		
	//cluster
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "cluster" + PATH_DELIMITER;
	copyWAV( sMAXPath + "TANKC_1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "TANKC_5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "TANKC_5.WAV", path + "drive.wav", 1);
	copyWAV( sMAXPath + "TANKC_7.WAV", path + "stop.wav");
	copyWAV( sMAXPath + "MISLFIRE.WAV", path + "attack.wav");

	//commando
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "commando" + PATH_DELIMITER;
	copyWAV( sMAXPath + "MANMOVE.WAV", path + "drive.wav");
	copyWAV( sMAXPath + "INFIL14.WAV", path + "attack.wav");

	//corvet
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "corvet" + PATH_DELIMITER;
	copyWAV( sMAXPath + "SBOATIDL.WAV", path + "wait_water.wav");
	copyPartOfWAV( sMAXPath + "SBOATMVE.WAV", path + "start_water.wav", 0);
	copyPartOfWAV( sMAXPath + "SBOATMVE.WAV", path + "drive_water.wav", 1);
	copyWAV( sMAXPath + "SBOATSTP.WAV", path + "stop_water.wav");
	copyWAV( sMAXPath + "CORVT14.WAV", path + "attack.wav");
		
	//escort
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "escort" + PATH_DELIMITER;
	copyWAV( sMAXPath + "SBOATIDL.WAV", path + "wait_water.wav");
	copyPartOfWAV( sMAXPath + "SBOATMVE.WAV", path + "start_water.wav", 0);
	copyPartOfWAV( sMAXPath + "SBOATMVE.WAV", path + "drive_water.wav", 1);
	copyWAV( sMAXPath + "SBOATSTP.WAV", path + "stop_water.wav");
	copyWAV( sMAXPath + "CORVT14.WAV", path + "attack.wav");
		
	//fighter
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "fighter" + PATH_DELIMITER;
	copyWAV( sMAXPath + "FIGHT1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "FIGHT5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "FIGHT5.WAV", path + "drive.wav", 1);
	copyWAV( sMAXPath + "FIGHT7.WAV", path + "stop.wav");
	copyWAV( sMAXPath + "SCOUT14.WAV", path + "attack.wav");
		
	//gunboat
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "gunboat" + PATH_DELIMITER;
	copyWAV( sMAXPath + "GBOATIDL.WAV", path + "wait_water.wav");
	copyPartOfWAV( sMAXPath + "GBOATMVE.WAV", path + "start_water.wav", 0);
	copyPartOfWAV( sMAXPath + "GBOATMVE.WAV", path + "drive_water.wav", 1);
	copyWAV( sMAXPath + "GBOATSTP.WAV", path + "stop_water.wav");
	copyWAV( sMAXPath + "FARTY14.WAV", path + "attack.wav");

	//infantery
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "infantery" + PATH_DELIMITER;
	copyWAV( sMAXPath + "MANMOVE.WAV", path + "drive.wav");
	copyWAV( sMAXPath + "INFAN14.WAV", path + "attack.wav");
			
	//konstrukt
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "konstrukt" + PATH_DELIMITER;
	copyWAV( sMAXPath + "CONST2.WAV", path + "wait_water.wav");
	copyPartOfWAV( sMAXPath + "CONST6.WAV", path + "start_water.wav", 0);
	copyPartOfWAV( sMAXPath + "CONST6.WAV", path + "drive_water.wav", 1);
	copyWAV( sMAXPath + "CONST8.WAV", path + "stop_water.wav");
	copyWAV( sMAXPath + "CONST1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "CONST5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "CONST5.WAV", path + "drive.wav", 1);
	copyWAV( sMAXPath + "CONST7.WAV", path + "stop.wav");
		
	//minelayer
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "minelayer" + PATH_DELIMITER;
	copyWAV( sMAXPath + "TANKC_1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "TANKC_5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "TANKC_5.WAV", path + "drive.wav", 1);
	copyWAV( sMAXPath + "TANKC_7.WAV", path + "stop.wav");
		
	//missel
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "missel" + PATH_DELIMITER;
	copyWAV( sMAXPath + "TANKC_1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "TANKC_5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "TANKC_5.WAV", path + "drive.wav", 1);
	copyWAV( sMAXPath + "TANKC_7.WAV", path + "stop.wav");
	copyWAV( sMAXPath + "MISLFIRE.WAV", path + "attack.wav");
		
	//missel_ship
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "missel_ship" + PATH_DELIMITER;
	copyWAV( sMAXPath + "MBOATIDL.WAV", path + "wait_water.wav");
	copyPartOfWAV( sMAXPath + "MBOATMVE.WAV", path + "start_water.wav", 0);
	copyPartOfWAV( sMAXPath + "MBOATMVE.WAV", path + "drive_water.wav", 1);
	copyWAV( sMAXPath + "MBOATSTP.WAV", path + "stop_water.wav");
	copyWAV( sMAXPath + "MISLFIRE.WAV", path + "attack.wav");
	
	//mobile_aa
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "mobile_aa" + PATH_DELIMITER;
	copyWAV( sMAXPath + "APC1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "APC5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "APC5.WAV", path + "drive.wav", 1);
	copyWAV( sMAXPath + "APC7.WAV", path + "stop.wav");
	copyWAV( sMAXPath + "MANTI14.WAV", path + "attack.wav");
		
	//pionier
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "pionier" + PATH_DELIMITER;
	copyWAV( sMAXPath + "ENGIN2.WAV", path + "wait_water.wav");
	copyPartOfWAV( sMAXPath + "ENGIN6.WAV", path + "start_water.wav", 0);
	copyPartOfWAV( sMAXPath + "ENGIN6.WAV", path + "drive_water.wav", 1);
	copyWAV( sMAXPath + "ENGIN8.WAV", path + "stop_water.wav");
	copyWAV( sMAXPath + "ENGIN1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "ENGIN5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "ENGIN5.WAV", path + "drive.wav", 1);
	copyWAV( sMAXPath + "ENGIN7.WAV", path + "stop.wav");
		
	//repair
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "repair" + PATH_DELIMITER;
	copyWAV( sMAXPath + "REPAIR1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "REPAIR5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "REPAIR5.WAV", path + "drive.wav", 1);
	copyWAV( sMAXPath + "REPAIR7.WAV", path + "stop.wav");
		
	//scanner
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "scanner" + PATH_DELIMITER;
	copyWAV( sMAXPath + "SCAN1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "SCAN5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "SCAN5.WAV", path + "drive.wav", 1);
	copyWAV( sMAXPath + "SCAN7.WAV", path + "stop.wav");
		
	//scout
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "scout" + PATH_DELIMITER;
	copyWAV( sMAXPath + "SCOUT2.WAV", path + "wait_water.wav");
	copyPartOfWAV( sMAXPath + "SCOUT6.WAV", path + "start_water.wav", 0);
	copyPartOfWAV( sMAXPath + "SCOUT6.WAV", path + "drive_water.wav", 1);
	copyWAV( sMAXPath + "SCOUT8.WAV", path + "stop_water.wav");
	copyWAV( sMAXPath + "SCOUT1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "SCOUT5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "SCOUT5.WAV", path + "drive.wav", 1);
	copyWAV( sMAXPath + "SCOUT7.WAV", path + "stop.wav");
	copyWAV( sMAXPath + "SCOUT14.WAV", path + "attack.wav");
		
	//sea_minelayer
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "sea_minelayer" + PATH_DELIMITER;
	copyWAV( sMAXPath + "GBOATIDL.WAV", path + "wait_water.wav");
	copyPartOfWAV( sMAXPath + "GBOATMVE.WAV", path + "start_water.wav", 0);
	copyPartOfWAV( sMAXPath + "GBOATMVE.WAV", path + "drive_water.wav", 1);
	copyWAV( sMAXPath + "GBOATSTP.WAV", path + "stop_water.wav");
		
	//sea_transport
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "sea_transport" + PATH_DELIMITER;
	copyWAV( sMAXPath + "GBOATIDL.WAV", path + "wait_water.wav");
	copyPartOfWAV( sMAXPath + "GBOATMVE.WAV", path + "start_water.wav", 0);
	copyPartOfWAV( sMAXPath + "GBOATMVE.WAV", path + "drive_water.wav", 1);
	copyWAV( sMAXPath + "GBOATSTP.WAV", path + "stop_water.wav");
		
	//sub
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "sub" + PATH_DELIMITER;
	copyWAV( sMAXPath + "SUB2.WAV", path + "wait_water.wav");
	copyPartOfWAV( sMAXPath + "SUB6.WAV", path + "start_water.wav", 0);
	copyPartOfWAV( sMAXPath + "SUB6.WAV", path + "drive_water.wav", 1);
	copyWAV( sMAXPath + "SUB8.WAV", path + "stop_water.wav");
	copyWAV( sMAXPath + "SUB14.WAV", path + "attack.wav");
		
	//surveyor
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "surveyor" + PATH_DELIMITER;
	copyWAV( sMAXPath + "SURVY2.WAV", path + "wait_water.wav");
	copyPartOfWAV( sMAXPath + "SURVY6.WAV", path + "start_water.wav", 0);
	copyPartOfWAV( sMAXPath + "SURVY6.WAV", path + "drive_water.wav", 1);
	copyWAV( sMAXPath + "SURVY8.WAV", path + "stop_water.wav");
	copyWAV( sMAXPath + "SURVY1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "SURVY5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "SURVY5.WAV", path + "drive.wav", 1);
	copyWAV( sMAXPath + "SURVY7.WAV", path + "stop.wav");
		
	//tank
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "tank" + PATH_DELIMITER;
	copyWAV( sMAXPath + "TANK1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "TANK5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "TANK5.WAV", path + "drive.wav", 1);
	copyWAV( sMAXPath + "TANK7.WAV", path + "stop.wav");
	copyWAV( sMAXPath + "CANFIRE.WAV", path + "attack.wav");
		
	//trans_gold
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "trans_gold" + PATH_DELIMITER;
	copyWAV( sMAXPath + "TRUCKIDL.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "TRUCKMVE.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "TRUCKMVE.WAV", path + "drive.wav", 1);
	copyWAV( sMAXPath + "TRUCKSTP.WAV", path + "stop.wav");
		
	//trans_metal
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "trans_metal" + PATH_DELIMITER;
	copyWAV( sMAXPath + "TRUCKIDL.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "TRUCKMVE.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "TRUCKMVE.WAV", path + "drive.wav", 1);
	copyWAV( sMAXPath + "TRUCKSTP.WAV", path + "stop.wav");
		
	//trans_oil
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "trans_oil" + PATH_DELIMITER;
	copyWAV( sMAXPath + "TRUCKIDL.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "TRUCKMVE.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "TRUCKMVE.WAV", path + "drive.wav", 1);
	copyWAV( sMAXPath + "TRUCKSTP.WAV", path + "stop.wav");
	
	if ( logFile != NULL )
	{
		writeLog( string("Vehicle sounds") + TEXT_FILE_LF);
		writeLog( iToStr( iErrors) + " errors" + TEXT_FILE_LF);
		writeLog( string("========================================================================") + TEXT_FILE_LF);
	}

	cout << "\n";
	cout << iToStr( iErrors) << " errors\n";
	return 1;
}

void installVoices()
{
	string path;
	iTotalFiles = 32;
	iErrors = 0;
	iInstalledFiles = 0;

	oggEncode = 0;

	cout << "========================================================================\n";
	cout << "Voices\n";

	path = sOutputPath + "voices" + PATH_DELIMITER;
	copyWAV(sVoicePath + "F001.WAV", path + "ok1.wav");
	copyWAV(sVoicePath + "F004.WAV", path + "ok2.wav");
	copyWAV(sVoicePath + "F006.WAV", path + "ok3.wav");
	copyWAV(sVoicePath + "F012.WAV", path + "commando_detected.wav");
	copyWAV(sVoicePath + "F013.WAV", path + "saved.wav");
	copyWAV(sVoicePath + "F053.WAV", path + "start_none.wav");
	copyWAV(sVoicePath + "F070.WAV", path + "detected1.wav");
	copyWAV(sVoicePath + "F071.WAV", path + "detected2.wav");
	copyWAV(sVoicePath + "F085.WAV", path + "loaded.wav");
	copyWAV(sVoicePath + "F093.WAV", path + "research_complete.wav");
	copyWAV(sVoicePath + "F094.WAV", path + "no_path1.wav");
	copyWAV(sVoicePath + "F095.WAV", path + "no_path2.wav");
	copyWAV(sVoicePath + "F138.WAV", path + "low_ammo2.wav");
	copyWAV(sVoicePath + "F142.WAV", path + "low_ammo1.wav");
	copyWAV(sVoicePath + "F145.WAV", path + "no_speed.wav");
	copyWAV(sVoicePath + "F150.WAV", path + "status_yellow.wav");
	copyWAV(sVoicePath + "F154.WAV", path + "status_red.wav");
	copyWAV(sVoicePath + "F158.WAV", path + "wachposten.wav");
	copyWAV(sVoicePath + "F162.WAV", path + "build_done1.wav");
	copyWAV(sVoicePath + "F165.WAV", path + "build_done2.wav");
	copyWAV(sVoicePath + "F166.WAV", path + "start_one.wav");
	copyWAV(sVoicePath + "F171.WAV", path + "clearing.wav");
	copyWAV(sVoicePath + "F181.WAV", path + "laying_mines.wav");
	copyWAV(sVoicePath + "F187.WAV", path + "clearing_mines.wav");
	//copyWAV(sVoicePath + "F191.WAV", path + "surveying.wav");
	copyWAV(sVoicePath + "F206.WAV", path + "start_more.wav");
	copyWAV(sVoicePath + "F220.WAV", path + "repaired.wav");
	copyWAV(sVoicePath + "F224.WAV", path + "transfer_done.wav");
	copyWAV(sVoicePath + "F232.WAV", path + "attacking_us.wav");
	copyWAV(sVoicePath + "F234.WAV", path + "destroyed_us.wav");
	copyWAV(sVoicePath + "F239.WAV", path + "unit_stolen.wav");
	copyWAV(sVoicePath + "F244.WAV", path + "unit_disabled.wav");
	copyWAV(sVoicePath + "F249.WAV", path + "disabled.wav");


	if ( logFile != NULL )
	{
		writeLog( string("Voices") + TEXT_FILE_LF);
		writeLog( iToStr( iErrors) + " errors" + TEXT_FILE_LF);
		writeLog( string("========================================================================") + TEXT_FILE_LF);
	}

	cout << "\n";
	cout << iToStr( iErrors) << " errors\n";

}

void installMaps()
{
	string path;
	iTotalFiles = 24;
	iErrors = 0;
	iInstalledFiles = 0;

	cout << "========================================================================\n";
	cout << "Maps\n";

	path = sOutputPath + "maps" + PATH_DELIMITER;
	copyFile(sMAXPath + "CRATER_1.WRL", path + "Iron Cross.wrl");
	copyFile(sMAXPath + "CRATER_2.WRL", path + "Splatterscape.wrl");
	copyFile(sMAXPath + "CRATER_3.WRL", path + "Peak-a-boo.wrl");
	copyFile(sMAXPath + "CRATER_4.WRL", path + "Valentine's Planet.wrl");
	copyFile(sMAXPath + "CRATER_5.WRL", path + "Three Rings.wrl");
	copyFile(sMAXPath + "CRATER_6.WRL", path + "Great divide.wrl");
	copyFile(sMAXPath + "DESERT_1.WRL", path + "Freckles.wrl");
	copyFile(sMAXPath + "DESERT_2.WRL", path + "Sandspit.wrl");
	copyFile(sMAXPath + "DESERT_3.WRL", path + "Great Circle.wrl");
	copyFile(sMAXPath + "DESERT_4.WRL", path + "Long Passage.wrl");
	copyFile(sMAXPath + "DESERT_5.WRL", path + "Flash Point.wrl");
	copyFile(sMAXPath + "DESERT_6.WRL", path + "Bottleneck.wrl");
	copyFile(sMAXPath + "GREEN_1.WRL", path + "New Luzon.wrl");
	copyFile(sMAXPath + "GREEN_2.WRL", path + "Middle Sea.wrl");
	copyFile(sMAXPath + "GREEN_3.WRL", path + "High Impact.wrl");
	copyFile(sMAXPath + "GREEN_4.WRL", path + "Sanctuary.wrl");
	copyFile(sMAXPath + "GREEN_5.WRL", path + "Islandia.wrl");
	copyFile(sMAXPath + "GREEN_6.WRL", path + "Hammerhead.wrl");
	copyFile(sMAXPath + "SNOW_1.WRL", path + "Snowcrab.wrl");
	copyFile(sMAXPath + "SNOW_2.WRL", path + "Frigia.wrl");
	copyFile(sMAXPath + "SNOW_3.WRL", path + "Ice Berg.wrl");
	copyFile(sMAXPath + "SNOW_4.WRL", path + "The Cooler.wrl");
	copyFile(sMAXPath + "SNOW_5.WRL", path + "Ultima Thule.wrl");
	copyFile(sMAXPath + "SNOW_6.WRL", path + "Long Floes.wrl");
	
	if ( logFile != NULL )
	{
		writeLog( string("Maps") + TEXT_FILE_LF);
		writeLog( iToStr( iErrors) + " errors" + TEXT_FILE_LF);
		writeLog( string("========================================================================") + TEXT_FILE_LF);
	}

	cout << "\n";
	cout << iToStr( iErrors) << " errors\n";
}

void installSounds()
{
	string path;
	iTotalFiles = 29;
	iErrors = 0;
	iInstalledFiles = 0;

	oggEncode = 0;

	cout << "========================================================================\n";
	cout << "Sounds\n";

	path = sOutputPath + "sounds" + PATH_DELIMITER;
	copyWAV( sMAXPath + "ACTIVATE.WAV", path + "activate.wav");
	copyWAV( sMAXPath + "MASTR17.WAV", path + "building.wav");
	copyWAV( sMAXPath + "BULL17.WAV", path + "clearing.wav");
	copyWAV( sMAXPath + "MENGENS4.WAV", path + "HudButton.wav");
	copyWAV( sMAXPath + "IHITS0.WAV", path + "HudSwitch.wav");
	copyWAV( sMAXPath + "MLAYER18.WAV", path + "land_mine_clear.wav");
	copyWAV( sMAXPath + "MLAYER17.WAV", path + "land_mine_place.wav");
	copyWAV( sMAXPath + "LOAD.WAV", path + "load.wav");
	copyWAV( sMAXPath + "MENU38.WAV", path + "MenuButton.wav");
	copyWAV( sMAXPath + "KBUY0.WAV", path + "ObjectMenu.wav");
	copyWAV( sMAXPath + "ICLOS0L.WAV", path + "panel_close.wav");
	copyWAV( sMAXPath + "IOPEN0.WAV", path + "panel_open.wav");
	copyWAV( sMAXPath + "FQUIT.WAV", path + "quitsch.wav");
	copyWAV( sMAXPath + "REPAIR17.WAV", path + "repair.wav");
	copyWAV( sMAXPath + "SMINE17.WAV", path + "sea_mine_place.wav");
	copyWAV( sMAXPath + "SMINE18.WAV", path + "sea_mine_clear.wav");
	copyWAV( sMAXPath + "FTRUCK17.WAV", path + "reload.wav");
	
	copyWAV( sMAXPath + "BOATEXP1.WAV", path + "exp_small_wet0.wav");
	copyWAV( sMAXPath + "EPLOWET1.WAV", path + "exp_small_wet1.wav");
	copyWAV( sMAXPath + "EPLOWET2.WAV", path + "exp_small_wet2.wav");
	copyWAV( sMAXPath + "EXPLLRGE.WAV", path + "exp_small0.wav");
	copyWAV( sMAXPath + "EXPLMED.WAV", path + "exp_small1.wav");
	copyWAV( sMAXPath + "EXPLSMAL.WAV", path + "exp_small2.wav");
	copyWAV( sMAXPath + "CBLDEXP1.WAV", path + "exp_big_wet0.wav");
	copyWAV( sMAXPath + "CBLDEXP2.WAV", path + "exp_big_wet1.wav");
	copyWAV( sMAXPath + "BLDEXPLG.WAV", path + "exp_big0.wav");
	copyWAV( sMAXPath + "EXPBULD6.WAV", path + "exp_big1.wav");
	copyWAV( sMAXPath + "EXPLBLD1.WAV", path + "exp_big2.wav");
	copyWAV( sMAXPath + "EXPLBLD2.WAV", path + "exp_big3.wav");
	
	if ( logFile != NULL )
	{
		writeLog( string("Sounds") + TEXT_FILE_LF);
		writeLog( iToStr( iErrors) + " errors" + TEXT_FILE_LF);
		writeLog( string("========================================================================") + TEXT_FILE_LF);
	}

	cout << "\n";
	cout << iToStr( iErrors) << " errors\n";
}

void installMusic()
{
	string path;
	iTotalFiles = 13;
	iErrors = 0;
	iInstalledFiles = 0;

	oggEncode = 1;

	cout << "========================================================================\n";
	cout << "Music (May take a while)\n";

	path = sOutputPath + "music" + PATH_DELIMITER;
	copyWAV( sMAXPath + "MAIN_MSC.MSC", path + "main.wav");
	copyWAV( sMAXPath + "BKG1_MSC.MSC", path + "bkg1.wav");
	copyWAV( sMAXPath + "BKG2_MSC.MSC", path + "bkg2.wav");
	copyWAV( sMAXPath + "BKG3_MSC.MSC", path + "bkg3.wav");
	copyWAV( sMAXPath + "BKG4_MSC.MSC", path + "bkg4.wav");
	copyWAV( sMAXPath + "BKG5_MSC.MSC", path + "bkg5.wav");
	copyWAV( sMAXPath + "BKG6_MSC.MSC", path + "bkg6.wav");
	copyWAV( sMAXPath + "CRTR_MSC.MSC", path + "crtr.wav");
	copyWAV( sMAXPath + "DSRT_MSC.MSC", path + "dsrt.wav");
	copyWAV( sMAXPath + "GREN_MSC.MSC", path + "gren.wav");
	copyWAV( sMAXPath + "LOSE_MSC.MSC", path + "lose.wav");
	copyWAV( sMAXPath + "SNOW_MSC.MSC", path + "snow.wav");
	copyWAV( sMAXPath + "WINR_MSC.MSC", path + "winr.wav");
	
	if ( logFile != NULL )
	{
		writeLog( string("Music") + TEXT_FILE_LF);
		writeLog( iToStr( iErrors) + " errors" + TEXT_FILE_LF);
		writeLog( string("========================================================================") + TEXT_FILE_LF);
	}

	cout << "\n";
	cout << iToStr( iErrors) << " errors\n";
}

// this is needed to prevent a linker error with MinGW
#ifdef main
#undef main
#endif

int main ( int argc, char* argv[] )
{
    // at startup SDL_Init should be called before all other SDL functions
    if (SDL_Init(SDL_INIT_VIDEO) == -1)
	{
		printf("Can't init SDL:  %s\n", SDL_GetError());
		exit (-1);
	}

    /*
    Uncommented since you do break any terminal I/O for users at least
    under linux with that too. You may write some WIN32 DEFINED code 
    to prevend stdout.txt to be created on windows -- beko
    */
    // added code to prevent writing to stdout.txt and stderr.txt
    //freopen( "CON", "w", stdout );

    atexit(SDL_Quit);	

	cout << "Resinstaller - installs graphics and sounds from Interplay's M.A.X. to\n\
M.A.X.R. for original game look and feel. For this you need an existing\n\
M.A.X. installation or an original M.A.X. CD available.\n\n";
	
	cout << "\
This program is free software; you can redistribute it and/or modify\n\
it under the terms of the GNU General Public License as published by\n\
the Free Software Foundation; either version 2 of the License, or\n\
(at your option) any later version.\n\
\n\
This program is distributed in the hope that it will be useful,\n\
but WITHOUT ANY WARRANTY; without even the implied warranty of\n\
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the\n\
GNU General Public License for more details.\n\n";

	// look for paths in argv[]
	if ( argc > 1 ) sMAXPath = argv[1];
	if ( argc > 2 ) sOutputPath = argv[2];

	//create log file
	logFile = SDL_RWFromFile("resinstaller.log", "w" );
	if ( logFile == NULL )
	{
		cout << "Warning: Couldn't create log file. Writing to stdout instead.\n";
	}
	else
	{
		freopen( "resinstaller.log", "a", stderr );	//write errors to log instead stdout(.txt)
	}

	writeLog(string("resinstaller version ") + VERSION + TEXT_FILE_LF);

	wasError = 0;
		
	while ( 1 )
	{
		cout << "Please enter full path to existing M.A.X. installation or mounted cd:\n";
#ifdef EIKO
		sMAXPath = "C:\\Dokumente und Einstellungen\\Eiko\\Desktop\\MAX-Develop\\MAX\\";
#else
		if ( argc < 2 || sMAXPath.size() == 0 )
		{
			char temp[1024];
			temp[1023] = '\0';
			cin.getline( temp, sizeof(temp) - 1 ); //don't overwrite the last char in temp to be sure its a \0
			cin.seekg(0,ios::end);
			cin.clear();
			sMAXPath = temp;
		}
		else
		{
			writeLog( "sMAXPath from command line: " + sMAXPath );
			cout << sMAXPath << "\n";
		}
#endif

		//now testing different input variations
		try
		{
			res = openFile ( (sMAXPath + "MAX.RES").c_str(), "rb" );
			break;
		}
		catch ( InstallException ) {}	//ignore exceptions

		try
		{
			res = openFile ( (sMAXPath + PATH_DELIMITER + "MAX.RES").c_str(), "rb" );
			sMAXPath += PATH_DELIMITER;
			break;
		}
		catch ( InstallException ) {}

		try
		{
			res = openFile ( (sMAXPath + "MAX" + PATH_DELIMITER + "MAX.RES").c_str(), "rb" );
			sMAXPath = sMAXPath + "MAX" + PATH_DELIMITER;
			break;
		}
		catch ( InstallException ) {}
		
		try
		{
			res = openFile ( (sMAXPath + PATH_DELIMITER + "MAX" + PATH_DELIMITER + "MAX.RES").c_str(), "rb" );
			sMAXPath = sMAXPath + PATH_DELIMITER + "MAX" + PATH_DELIMITER;
			break;
		}
		catch ( InstallException ) {}

		try
		{
			res = openFile ( (sMAXPath + "max" + PATH_DELIMITER + "MAX.RES").c_str(), "rb" );
			sMAXPath = sMAXPath + "max" + PATH_DELIMITER;
			break;
		}
		catch ( InstallException ) {}
		
		try
		{
			res = openFile ( (sMAXPath + PATH_DELIMITER + "max" + PATH_DELIMITER + "MAX.RES").c_str(), "rb" );
			sMAXPath = sMAXPath + PATH_DELIMITER + "max" + PATH_DELIMITER;
			break;
		}
		catch ( InstallException ) {}
		
		cout << "Couldn't find valid M.A.X. installation in given folder:\n";
		cout << "No max.res found.\n";
		sMAXPath.clear();
	}

	while (1)
	{
		cout << "\nPlease enter full path to M.A.X.R. installation for extracted files:\n";
	
#ifdef EIKO
		cout << "\n";
		//sOutputPath = "C:\\Dokumente und Einstellungen\\Eiko\\Desktop\\MAX-Develop\\MAXR Install\\output - install skript\\";
		sOutputPath = "C:\\Dokumente und Einstellungen\\Eiko\\Desktop\\MAX-Develop\\MAX Reloaded\\debug\\";
#else
		if ( argc < 3 || sOutputPath.size() == 0 )
		{
			char temp[1024];
			temp[1023] = '\0';
			cin.getline( temp, sizeof(temp) - 1 ); //don't overwrite the last char in temp to be sure its a \0
			cin.seekg(0,ios::end);
			cin.clear();
			sOutputPath = temp;
		}
		else
		{
			writeLog( "sOutputPath from command line: " + sOutputPath );
			cout << sOutputPath << "\n";
		}
#endif
		//test for valid output folder
		string testFileName = "max.xml";
		SDL_RWops* testFile;
		try
		{
			testFile = openFile( sOutputPath + testFileName, "r");
			SDL_RWclose( testFile );
			break;
		}
		catch ( InstallException ) {}

		try
		{
			testFile = openFile( sOutputPath + PATH_DELIMITER + testFileName, "r");
			sOutputPath += PATH_DELIMITER;
			SDL_RWclose( testFile );
			break;
		}
		catch ( InstallException ) {}

		cout << "Couldn't find valid M.A.X.R. installation in given folder.\n";
		sOutputPath.clear();
	}

	//check for available languages for voices
	string testFileNameLowerCase = "f001.wav";
	string testFileNameUpperCase = "F001.WAV";

	bool german = false, italian = false, french = false;
	bool uppercase;
	int iLanguages = 0;
	SDL_RWops* testFile;
	try
	{
		testFile = openFile( sMAXPath + "german" + PATH_DELIMITER + testFileNameLowerCase, "r" );
		german = true;
		iLanguages++;
		uppercase = false;
		SDL_RWclose( testFile );
	}
	catch ( InstallException ) {}

	try
	{
		testFile = openFile( sMAXPath + "GERMAN" + PATH_DELIMITER + testFileNameUpperCase, "r" );
		if ( german == false ) iLanguages++;
		german = true;
		uppercase = true;
		SDL_RWclose( testFile );
	}
	catch ( InstallException ) {}

	try
	{
		testFile = openFile( sMAXPath + "italian" + PATH_DELIMITER + testFileNameLowerCase, "r" );
		italian = true;
		iLanguages++;
		uppercase = false;
		SDL_RWclose( testFile );
	}
	catch ( InstallException ) {}

	try
	{
		testFile = openFile( sMAXPath + "ITALIAN" + PATH_DELIMITER + testFileNameUpperCase, "r" );
		if ( italian == false ) iLanguages++;
		italian = true;
		uppercase = true;
		SDL_RWclose( testFile );
	}
	catch ( InstallException ) {}

	try
	{
		testFile = openFile( sMAXPath + "french" + PATH_DELIMITER + testFileNameLowerCase, "r" );
		french = true;
		iLanguages++;
		uppercase = false;
		SDL_RWclose( testFile );
	}
	catch ( InstallException ) {}

	try
	{
		testFile = openFile( sMAXPath + "FRENCH" + PATH_DELIMITER + testFileNameUpperCase, "r" );
		if ( french == false ) iLanguages++;
		french = true;
		uppercase = true;
		SDL_RWclose( testFile );
	}
	catch ( InstallException ) {}

	if ( iLanguages == 0 )
	{
		//we are not installing from CD
		sVoicePath = sMAXPath;
	}
	else
	{
		//make menu
		cout << "\nThe following voice samples are available from your install source:\n";
		cout << "- english\n";
		if ( german == true ) cout << "- german\n";
		if ( italian == true ) cout << "- italian\n";
		if ( french == true ) cout << "- french\n";

		string input;
		while ( 1 )
		{
			cout << "\nEnter your preferred language: ";

			char temp[1024];
			temp[1023] = '\0';
			cin.getline( temp, sizeof(temp) - 1 ); //don't overwrite the last char in temp to be sure its a \0
			cin.seekg(0,ios::end);
			cin.clear();
			input = temp;

			if ( input.compare("english") == 0 )
			{
				sVoicePath = sMAXPath;
				break;
			}

			if ( german && input.compare("german") == 0 )
			{
				if ( uppercase )
				{
					sVoicePath = sMAXPath + "GERMAN" + PATH_DELIMITER;
				}
				else
				{
					sVoicePath = sMAXPath + "german" + PATH_DELIMITER;
				}
				break;
			}

			if ( italian && input.compare("italian") == 0 )
			{
				if ( uppercase )
				{
					sVoicePath = sMAXPath + "ITALIAN" + PATH_DELIMITER;
				}
				else
				{
					sVoicePath = sMAXPath + "italian" + PATH_DELIMITER;
				}
				break;
			}

			if ( french && input.compare("french") == 0 )
			{
				if ( uppercase )
				{
					sVoicePath = sMAXPath + "FRENCH" + PATH_DELIMITER;
				}
				else
				{
					sVoicePath = sMAXPath + "french" + PATH_DELIMITER;
				}
				break;
			}

			cout << "Language not recognized\n";
			cout << "Hint: Do not type the leading dashes/blanks!\n";
		}
	}

	//init res converter
	SDL_RWseek( res, 0, SEEK_END );
	lEndOfFile = SDL_RWtell (res);

	
	lPosBegin = 15000000;		//the '[EOD]' should be after this position 
								//for all versions of max.res, I think 

	//a little state maschine for searching the string "[EOD]" in max.res
	unsigned char temp, state = 0;
	SDL_RWseek( res, lPosBegin, SEEK_SET);

	while ( lPosBegin < lEndOfFile )
	{
		SDL_RWread( res, &temp, sizeof ( char ), 1 );
		lPosBegin++;

		switch ( state )
		{
			case 1:
				if ( temp == 'E' )
					state = 2;
				else
					state = 0;
				break;
			case 2:
				if ( temp == 'O' )
					state = 3;
				else
					state = 0;
				break;	
			case 3:
				if ( temp == 'D' )
					state = 4;
				else
					state = 0;
				break;
			case 4:
				if ( temp == ']' )
					state = 5;
				else
					state = 0;
				break;
		}

		if ( temp == '[' ) state = 1;

		if ( state == 5 ) break;
	}

	if ( lPosBegin == lEndOfFile )
	{
		cout << "Error: [EOD] not found in resource file. Please contact the developer!";
		exit (-1);
	}

	installBuildingSounds();
	installVehicleSounds();
	installFX();
	installGfx();
	installVehicleVideos();
	installVehicleGraphics();
	installBuildingGraphics();
	installVoices();
	installMaps();
	installSounds();
	installMusic();
	
	if ( wasError )
	{
		cout << "There were errors during install. See 'resinstaller.log' for details.\n";
	}
	else
	{
		cout << "Finished\n";
	}
	
	SDL_RWclose( res );
	SDL_RWclose( logFile );
	
#ifdef WIN32
	//wait for key press
	FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));
	getch();
#endif

	return 0;
}
