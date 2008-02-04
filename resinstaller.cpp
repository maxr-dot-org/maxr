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
			surface = getImage("ARTILLRY", i);
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
		output = getImage("AWAC", 8);
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
			surface = getImage("AWAC", i*4 +8);
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
			surface = getImage("BOMBER", i);
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
			surface = getImage("BULLDOZR", i);
			removePlayerColor( surface );
			resizeSurface( surface, 4, 4, 64, 64);
			savePCX( surface, path + "img" + szNum + ".pcx");
			SDL_FreeSurface( surface );
		}
		END_INSTALL_FILE( path + "img" + szNum + ".pcx")
		
		try
		{
			surface = getImage("S_BULLDO", i);
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
		surface = getImage( "LRGTAPE", 0);
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

		surface = getImage("LRGCONES");
		dst_rect.y = 5;
		dst_rect.x = 5;
		for ( int i = 0; i < 4; i++ )
		{
			SDL_BlitSurface( surface, 0, output, &dst_rect );
			dst_rect.x += 128;
		}
		SDL_FreeSurface( surface );

		surface = getImage("BULLDOZR", 8);
		removePlayerColor( surface );
		dst_rect.x = 36;
		dst_rect.y = 36;
		SDL_BlitSurface( surface, 0, output, &dst_rect );
		SDL_FreeSurface( surface );

		//can someone tell me, why I have to reload the surface? :-|
		//otherwise changeing the palette after the first blit has no effect anymore
		surface = getImage("BULLDOZR", 8);
		removePlayerColor( surface );
		generateAnimationFrame( surface, 1);
		dst_rect.x = 164;
		SDL_BlitSurface( surface, 0, output, &dst_rect );
		SDL_FreeSurface( surface );

		surface = getImage("BULLDOZR", 8);
		removePlayerColor( surface );
		generateAnimationFrame( surface, 2);
		dst_rect.x = 292;
		SDL_BlitSurface( surface, 0, output, &dst_rect );
		SDL_FreeSurface( surface );
		
		surface = getImage("BULLDOZR", 8);
		removePlayerColor( surface );
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
		output = getImage("S_LRGCON");
		resizeSurface( output, 6, 6, 128, 128 ); 
		surface = getImage("S_BULLDO");
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
		surface = getImage("SMLTAPE");
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

		surface = getImage("SMLCONES");
		dst_rect.y = 5;
		dst_rect.x = 5;
		for ( int i = 0; i < 4; i++ )
		{
			SDL_BlitSurface( surface, 0, output, &dst_rect );
			dst_rect.x += 64;
		}
		SDL_FreeSurface( surface );

		surface = getImage("BULLDOZR", 8);
		removePlayerColor( surface );
		dst_rect.x = 4;
		dst_rect.y = 4;
		SDL_BlitSurface( surface, 0, output, &dst_rect );
		SDL_FreeSurface( surface );

		surface = getImage("BULLDOZR", 8);
		removePlayerColor( surface );
		generateAnimationFrame( surface, 1);
		dst_rect.x += 64;
		SDL_BlitSurface( surface, 0, output, &dst_rect );
		SDL_FreeSurface( surface );

		surface = getImage("BULLDOZR", 8);
		removePlayerColor( surface );
		generateAnimationFrame( surface, 2);
		dst_rect.x += 64;
		SDL_BlitSurface( surface, 0, output, &dst_rect );
		SDL_FreeSurface( surface );
		
		surface = getImage("BULLDOZR", 8);
		removePlayerColor( surface );
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
		output = getImage("S_SMLCON");
		resizeSurface( output, 6, 6, 64, 66 );
		surface = getImage("S_BULLDO");
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
			surface = getImage("ROCKTLCH", i);
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
				surface = getImage("COMMANDO", n * 8 + i );
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
			surface = getImage("FIGHTER", i);
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
		output = getImage("BATTLSHP", 0);
		removePlayerColor( output );
		resizeSurface( output, 4, 11, 64, 64);
		dst_rect.y = 3;
		dst_rect.x = 14; 
		surface = getImage("BATTLSHP", 8);
		SDL_BlitSurface(surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface( surface );
		savePCX(output, path + "img0.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "img0.pcx")

	try
	{
		output = getImage("BATTLSHP", 1);
		removePlayerColor( output );
		resizeSurface( output, 4, 11, 64, 64);
		dst_rect.y = 4;
		dst_rect.x = 22;
		surface = getImage("BATTLSHP", 9);
		SDL_BlitSurface(surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface( surface );
		savePCX(output, path + "img1.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "img1.pcx")

	try
	{
		output = getImage("BATTLSHP", 2);
		removePlayerColor( output );
		resizeSurface( output, 4, 11, 64, 64);
		dst_rect.y = 12;
		dst_rect.x = 23;
		surface = getImage("BATTLSHP", 10);
		SDL_BlitSurface(surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface( surface );
		savePCX(output, path + "img2.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "img2.pcx")

	try
	{
		output = getImage("BATTLSHP", 3);
		removePlayerColor( output );
		resizeSurface( output, 4, 11, 64, 64);
		dst_rect.y = 20;
		dst_rect.x = 22;
		surface = getImage("BATTLSHP", 11);
		SDL_BlitSurface(surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface( surface );
		savePCX(output, path + "img3.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "img3.pcx")

	try
	{
		output = getImage("BATTLSHP", 4);
		removePlayerColor( output );
		resizeSurface( output, 4, 11, 64, 64);
		dst_rect.y = 21;
		dst_rect.x = 14;
		surface = getImage("BATTLSHP", 12);
		SDL_BlitSurface(surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface( surface );
		savePCX(output, path + "img4.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "img4.pcx")

	try
	{
		output = getImage("BATTLSHP", 5);
		removePlayerColor( output );
		resizeSurface( output, 4, 11, 64, 64);
		dst_rect.y = 20;
		dst_rect.x = 6;
		surface = getImage("BATTLSHP", 13);
		SDL_BlitSurface(surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface( surface );
		savePCX(output, path + "img5.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "img5.pcx")

	try
	{
		output = getImage("BATTLSHP", 6);
		removePlayerColor( output );
		resizeSurface( output, 4, 11, 64, 64);
		dst_rect.y = 12;
		dst_rect.x = 5;
		surface = getImage("BATTLSHP", 14);
		SDL_BlitSurface(surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface( surface );
		savePCX(output, path + "img6.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "img6.pcx")

	try
	{
		output = getImage("BATTLSHP", 7);
		removePlayerColor( output );
		resizeSurface( output, 4, 11, 64, 64);
		dst_rect.y = 4;
		dst_rect.x = 6;
		surface = getImage("BATTLSHP", 15);
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
				surface = getImage("INFANTRY", n * 8 + i );
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
		surface = getImage( "LRGTAPE", 0);
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

		surface = getImage("LRGCONES");
		dst_rect.y = 5;
		dst_rect.x = 5;
		for ( int i = 0; i < 4; i++ )
		{
			SDL_BlitSurface( surface, 0, output, &dst_rect );
			dst_rect.x += 128;
		}
		SDL_FreeSurface( surface );

		surface = getImage("CONSTRCT", 16);
		removePlayerColor( surface );
		dst_rect.x = 33;
		dst_rect.y = 36;
		SDL_BlitSurface( surface, 0, output, &dst_rect );
		SDL_FreeSurface( surface );

		surface = getImage("CONSTRCT", 24);
		removePlayerColor( surface );
		generateAnimationFrame( surface, 1);
		dst_rect.x += 128;
		SDL_BlitSurface( surface, 0, output, &dst_rect );
		SDL_FreeSurface( surface );

		surface = getImage("CONSTRCT", 16);
		removePlayerColor( surface );
		generateAnimationFrame( surface, 2);
		dst_rect.x += 128;
		SDL_BlitSurface( surface, 0, output, &dst_rect );
		SDL_FreeSurface( surface );
		
		surface = getImage("CONSTRCT", 32);
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
		output = getImage("S_LRGCON");
		resizeSurface( output, 6, 6, 128, 128 );
		surface = getImage("S_CONSTR");
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
		sprintf( szNum, "%d", i);
		copyFileFromRes_rpc("MISSLLCH", path + "img" + szNum + ".pcx", i);
		copyFileFromRes("S_MISSLL", path + "shw" + szNum + ".pcx", i);
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
			surface = getImage("MSSLBOAT", i);
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
		output = getImage("SP_FLAK", 0);
		removePlayerColor( output );
		resizeSurface( output, 3, 0, 64, 64);
		dst_rect.y = 3;
		dst_rect.x = 14; 
		surface = getImage("SP_FLAK", 8);
		SDL_BlitSurface(surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface( surface );
		savePCX(output, path + "img0.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "img0.pcx")

	try
	{
		output = getImage("SP_FLAK", 1);
		removePlayerColor( output );
		resizeSurface( output, 3, 0, 64, 64);
		dst_rect.y = 4;
		dst_rect.x = 24;
		surface = getImage("SP_FLAK", 9);
		SDL_BlitSurface(surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface( surface );
		savePCX(output, path + "img1.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "img1.pcx")

	try
	{
		output = getImage("SP_FLAK", 2);
		removePlayerColor( output );
		resizeSurface( output, 3, 0, 64, 64);
		dst_rect.y = 15;
		dst_rect.x = 24;
		surface = getImage("SP_FLAK", 10);
		SDL_BlitSurface(surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface( surface );
		savePCX(output, path + "img2.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "img2.pcx")

	try
	{
		output = getImage("SP_FLAK", 3);
		removePlayerColor( output );
		resizeSurface( output, 3, 0, 64, 64);
		dst_rect.y = 23;
		dst_rect.x = 22;
		surface = getImage("SP_FLAK", 11);
		SDL_BlitSurface(surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface( surface );
		savePCX(output, path + "img3.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "img3.pcx")

	try
	{
		output = getImage("SP_FLAK", 4);
		removePlayerColor( output );
		resizeSurface( output, 3, 0, 64, 64);
		dst_rect.y = 27;
		dst_rect.x = 14;
		surface = getImage("SP_FLAK", 12);
		SDL_BlitSurface(surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface( surface );
		savePCX(output, path + "img4.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "img4.pcx")

	try
	{
		output = getImage("SP_FLAK", 5);
		removePlayerColor( output );
		resizeSurface( output, 3, 0, 64, 64);
		dst_rect.y = 22;
		dst_rect.x = 6;
		surface = getImage("SP_FLAK", 13);
		SDL_BlitSurface(surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface( surface );
		savePCX(output, path + "img5.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "img5.pcx")

	try
	{
		output = getImage("SP_FLAK", 6);
		removePlayerColor( output );
		resizeSurface( output, 3, 0, 64, 64);
		dst_rect.y = 16;
		dst_rect.x = 4;
		surface = getImage("SP_FLAK", 14);
		SDL_BlitSurface(surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface( surface );
		savePCX(output, path + "img6.pcx");
		SDL_FreeSurface( output );
	}
	END_INSTALL_FILE( path + "img6.pcx")

	try
	{
		output = getImage("SP_FLAK", 7);
		removePlayerColor( output );
		resizeSurface( output, 3, 0, 64, 64);
		dst_rect.y = 8;
		dst_rect.x = 5;
		surface = getImage("SP_FLAK", 15);
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
			surface = getImage("ENGINEER", i);
			removePlayerColor( surface );
			resizeSurface( surface, 4, 4, 64, 64);
			savePCX(surface, path + "img" + szNum + ".pcx");
			SDL_FreeSurface( surface );
		}
		END_INSTALL_FILE( path + "img" + szNum + ".pcx")
		
		try
		{
			surface = getImage("S_ENGINE", i);
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
		surface = getImage("SMLTAPE");
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

		surface = getImage("SMLCONES");
		dst_rect.y = 5;
		dst_rect.x = 5;
		for ( int i = 0; i < 4; i++ )
		{
			SDL_BlitSurface( surface, 0, output, &dst_rect );
			dst_rect.x += 64;
		}
		SDL_FreeSurface( surface );

		surface = getImage("ENGINEER", 16);
		removePlayerColor( surface );
		dst_rect.x = 4;
		dst_rect.y = 4;
		SDL_BlitSurface( surface, 0, output, &dst_rect );
		SDL_FreeSurface( surface );

		surface = getImage("ENGINEER", 16);
		removePlayerColor( surface );
		generateAnimationFrame( surface, 1);
		dst_rect.x += 64;
		SDL_BlitSurface( surface, 0, output, &dst_rect );
		SDL_FreeSurface( surface );

		surface = getImage("ENGINEER", 16);
		removePlayerColor( surface );
		generateAnimationFrame( surface, 2);
		dst_rect.x += 64;
		SDL_BlitSurface( surface, 0, output, &dst_rect );
		SDL_FreeSurface( surface );
		
		surface = getImage("ENGINEER", 16);
		removePlayerColor( surface );
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
		output = getImage("S_SMLCON");
		resizeSurface( output, 6, 6, 64, 66 );
		surface = getImage("S_ENGINE");
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
			surface = getImage("SCANNER", i);
			removePlayerColor( surface );
			resizeSurface( surface, 8, 8, 64, 64);
			savePCX( surface, path + "img" + szNum + ".pcx");
			SDL_FreeSurface( surface );
		}
		END_INSTALL_FILE( path + "img" + szNum + ".pcx")

		try
		{
			surface = getImage("S_SCANNE", i);
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
		surface = getImage("SCANNER", 8);
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
			surface = getImage("SCANNER", i*2 + 8);
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
			surface = getImage("SURVEYOR", i);
			removePlayerColor( surface );
			resizeSurface( surface, 2, 2, 64, 64);
			savePCX( surface, path + "img" + szNum + ".pcx");
			SDL_FreeSurface( surface );
		}
		END_INSTALL_FILE( path + "img" + szNum + ".pcx")

		try
		{
			surface = getImage("S_SURVEY", i);
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
			output = getImage("TANK", i);
			removePlayerColor( output );
			resizeSurface( output, 15, 15, 64, 64);
			surface = getImage("TANK", i + 8);
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
			surface = getImage("S_TANK", i);
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
			surface = getImage("SPLYTRCK", i);
			removePlayerColor( surface );
			resizeSurface( surface, 4, 4, 64, 64);
			savePCX( surface, path + "img" + szNum + ".pcx");
			SDL_FreeSurface ( surface );
		}
		END_INSTALL_FILE( path + "img" + szNum + ".pcx")

		try
		{
			surface = getImage("S_SPLYTR", i);
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
			surface = getImage("FUELTRCK", i);
			removePlayerColor( surface );
			resizeSurface( surface, 3, 3, 64, 64);
			savePCX( surface, path + "img" + szNum + ".pcx");
			SDL_FreeSurface ( surface );
		}
		END_INSTALL_FILE( path + "img" + szNum + ".pcx")

		try
		{
			surface = getImage("S_FUELTR", i);
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
	iTotalFiles = 136;
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
		surface = getImage("CNCT_4W", 0);
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

		surface = getImage("CNCT_4W", 2 );
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

		surface = getImage("CNCT_4W", 3 );
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

		surface = getImage("CNCT_4W", 4 );
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

		surface = getImage("CNCT_4W", 5 );
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
		surface = getImage("S_CNCT4W", 0);
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

		surface = getImage("S_CNCT4W", 2);
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

		surface = getImage("S_CNCT4W", 3);
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

		surface = getImage("S_CNCT4W", 4);
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

		surface = getImage("S_CNCT4W", 5);
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
		surface = getImage("ANTIAIR");
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
			surface = getImage("ANTIAIR", i);
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
		surface = getImage("ARTYTRRT");
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
			surface = getImage("ARTYTRRT", i);
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
		surface = getImage("ANTIMSSL", 0);
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
			surface = getImage("ANTIMSSL", i);
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
		surface = getImage("GUNTURRT", 0);
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
			surface = getImage("GUNTURRT", i);
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
		surface = getImage("LANDMINE");
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
		surface = getImage("S_LANDMI");
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
		surface = getImage("RADAR", 0);
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
			surface = getImage("RADAR", i);
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
		surface = getImage("SEAMINE");
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
		surface = getImage("SEAMINE");
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
	copyFileFromRes("LRGRUBLE", path + "dirt_big.pcx",  0);
	//copyFileFromRes("LRGRUBLE", path + "dirt_big1.pcx", 1);
	copyFileFromRes("SMLRUBLE", path + "dirt_small.pcx",  0);
	//copyFileFromRes("SMLRUBLE", path + "dirt_small1.pcx", 1);
	//copyFileFromRes("SMLRUBLE", path + "dirt_small2.pcx", 2);
	//copyFileFromRes("SMLRUBLE", path + "dirt_small3.pcx", 3);
	//copyFileFromRes("SMLRUBLE", path + "dirt_small4.pcx", 4);

	copyFileFromRes("S_LRGRBL", path + "dirt_big_shw.pcx",  1);
	//copyFileFromRes("S_LRGRBL", path + "dirt_big_shw1.pcx", 1);
	copyFileFromRes("S_SMLRBL", path + "dirt_small_shw.pcx",  0);
	//copyFileFromRes("S_SMLRBL", path + "dirt_small_shw1.pcx", 1);
	//copyFileFromRes("S_SMLRBL", path + "dirt_small_shw2.pcx", 2);
	//copyFileFromRes("S_SMLRBL", path + "dirt_small_shw3.pcx", 3);
	//copyFileFromRes("S_SMLRBL", path + "dirt_small_shw4.pcx", 4);
	

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
	SDL_Surface *surface;
	string path = sOutputPath + "fx" + PATH_DELIMITER;

	surface = getImage("WALDO");
	resizeSurface( surface, 8, 7, 64, 64);
	savePCX( surface, path + "corpse.pcx");

	return 0;

}

int installBuildingSounds()
{
	string path;
	iTotalFiles = 42;
	iErrors = 0;
	iInstalledFiles = 0;

	cout << "========================================================================\n";
	cout << "Building sounds\n";

	//energy big
	path = sOutputPath + "buildings" + PATH_DELIMITER + "energy_big" + PATH_DELIMITER;
	copyPartOfWAV( sMAXPath + "POWGN17.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "POWGN17.WAV", path + "running.wav", 1);
	copyFile( sMAXPath + "POWGN18.WAV", path + "stop.wav");
	
	//energy small
	path = sOutputPath + "buildings" + PATH_DELIMITER + "energy_small" + PATH_DELIMITER;
	copyPartOfWAV( sMAXPath + "POWGN17.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "POWGN17.WAV", path + "running.wav", 1);
	copyFile( sMAXPath + "POWGN18.WAV", path + "stop.wav");
		
	//fac air
	path = sOutputPath + "buildings" + PATH_DELIMITER + "fac_air" + PATH_DELIMITER;
	copyPartOfWAV( sMAXPath + "AUNIT17.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "AUNIT17.WAV", path + "running.wav", 1);
	copyFile( sMAXPath + "AUNIT18.WAV", path + "stop.wav");
	
	//fac alien
	path = sOutputPath + "buildings" + PATH_DELIMITER + "fac_alien" + PATH_DELIMITER;
	copyPartOfWAV( sMAXPath + "LVP17.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "LVP17.WAV", path + "running.wav", 1);
	copyFile( sMAXPath + "LVP18.WAV", path + "stop.wav");
	
	//fac big
	path = sOutputPath + "buildings" + PATH_DELIMITER + "fac_big" + PATH_DELIMITER;
	copyPartOfWAV( sMAXPath + "HVP17.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "HVP17.WAV", path + "running.wav", 1);
	copyFile( sMAXPath + "HVP18.WAV", path + "stop.wav");
	
	//fac ship
	path = sOutputPath + "buildings" + PATH_DELIMITER + "fac_ship" + PATH_DELIMITER;
	copyPartOfWAV( sMAXPath + "HVP17.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "HVP17.WAV", path + "running.wav", 1);
	copyFile( sMAXPath + "HVP18.WAV", path + "stop.wav");
	
	//fac small
	path = sOutputPath + "buildings" + PATH_DELIMITER + "fac_small" + PATH_DELIMITER;
	copyPartOfWAV( sMAXPath + "LVP17.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "LVP17.WAV", path + "running.wav", 1);
	copyFile( sMAXPath + "LVP18.WAV", path + "stop.wav");
	
	//goldraff
	path = sOutputPath + "buildings" + PATH_DELIMITER + "goldraff" + PATH_DELIMITER;
	copyPartOfWAV( sMAXPath + "MONOP17.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "MONOP17.WAV", path + "running.wav", 1);
	copyFile( sMAXPath + "MONOP18.WAV", path + "stop.wav");
	
	//gun aa
	path = sOutputPath + "buildings" + PATH_DELIMITER + "gun_aa" + PATH_DELIMITER;
	copyFile( sMAXPath + "FANTI14.WAV", path + "attack.wav");
	
	//gun ari
	path = sOutputPath + "buildings" + PATH_DELIMITER + "gun_ari" + PATH_DELIMITER;
	copyFile( sMAXPath + "FARTY14.WAV", path + "attack.wav");
	
	//gun missile
	path = sOutputPath + "buildings" + PATH_DELIMITER + "gun_missel" + PATH_DELIMITER;
	copyFile( sMAXPath + "MISLFIRE.WAV", path + "attack.wav");

	//gun turret
	path = sOutputPath + "buildings" + PATH_DELIMITER + "gun_turret" + PATH_DELIMITER;
	copyFile( sMAXPath + "CANFIRE.WAV", path + "attack.wav");
	
	//landmine
	path = sOutputPath + "buildings" + PATH_DELIMITER + "landmine" + PATH_DELIMITER;
	copyFile( sMAXPath + "EXPSDIRT.WAV", path + "attack.wav");
	
	//mine
	path = sOutputPath + "buildings" + PATH_DELIMITER + "mine" + PATH_DELIMITER;
	copyPartOfWAV( sMAXPath + "HVP17.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "HVP17.WAV", path + "running.wav", 1);
	copyFile( sMAXPath + "MSTAT18.WAV", path + "stop.wav");

	//mine deep
	path = sOutputPath + "buildings" + PATH_DELIMITER + "mine_deep" + PATH_DELIMITER;
	copyPartOfWAV( sMAXPath + "HVP17.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "HVP17.WAV", path + "running.wav", 1);
	copyFile( sMAXPath + "MSTAT18.WAV", path + "stop.wav");
	
	//radar
	//path = sOutputPath + "buildings" + PATH_DELIMITER + "radar" + PATH_DELIMITER;
	//copyFile( sMAXPath + "RADAR13.WAV", path + "running.wav");
	
	//research
	path = sOutputPath + "buildings" + PATH_DELIMITER + "research" + PATH_DELIMITER;
	copyPartOfWAV( sMAXPath + "RESEAR17.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "RESEAR17.WAV", path + "running.wav", 1);
	copyFile( sMAXPath + "RESEAR18.WAV", path + "stop.wav");

	//seamine
	path = sOutputPath + "buildings" + PATH_DELIMITER + "seamine" + PATH_DELIMITER;
	copyFile( sMAXPath + "EPLOWET1.WAV", path + "attack.wav");
	
	//shield
	//path = sOutputPath + "buildings" + PATH_DELIMITER + "shield" + PATH_DELIMITER;

	//training
	path = sOutputPath + "buildings" + PATH_DELIMITER + "training" + PATH_DELIMITER;
	copyPartOfWAV( sMAXPath + "LVP17.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "LVP17.WAV", path + "running.wav", 1);
	copyFile( sMAXPath + "LVP18.WAV", path + "stop.wav");

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

	cout << "========================================================================\n";
	cout << "Vehicle Sounds\n";

	//air_transport
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "air_transport" + PATH_DELIMITER;
	copyFile( sMAXPath + "ATRANS1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "ATRANS5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "ATRANS5.WAV", path + "drive.wav", 1);
	copyFile( sMAXPath + "ATRANS7.WAV", path + "stop.wav");

	//alien_assault
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "alien_assault" + PATH_DELIMITER;
	copyFile( sMAXPath + "ALNTK1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "ALNTK5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "ALNTK5.WAV", path + "drive.wav", 1);
	copyFile( sMAXPath + "ALNTK7.WAV", path + "stop.wav");
	copyFile( sMAXPath + "ASGUN14.WAV", path + "attack.wav");
		
	//alien_plane
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "alien_plane" + PATH_DELIMITER;
	copyFile( sMAXPath + "ATTACK1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "ATTACK5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "ATTACK5.WAV", path + "drive.wav", 1);
	copyFile( sMAXPath + "ATTACK7.WAV", path + "stop.wav");
	copyFile( sMAXPath + "MISLFIRE.WAV", path + "attack.wav");
		
	//alien_ship
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "alien_ship" + PATH_DELIMITER;
	copyFile( sMAXPath + "JUGGR1.WAV", path + "wait_water.wav");
	copyPartOfWAV( sMAXPath + "JUGGR5.WAV", path + "start_water.wav", 0);
	copyPartOfWAV( sMAXPath + "JUGGR5.WAV", path + "drive_water.wav", 1);
	copyFile( sMAXPath + "JUGGR7.WAV", path + "stop_water.wav");
	copyFile( sMAXPath + "FARTY14.WAV", path + "attack.wav");
		
	//alien_tank
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "alien_tank" + PATH_DELIMITER;
	copyFile( sMAXPath + "ALNTK1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "ALNTK5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "ALNTK5.WAV", path + "drive.wav", 1);
	copyFile( sMAXPath + "ALNTK7.WAV", path + "stop.wav");
	copyFile( sMAXPath + "CANFIRE.WAV", path + "attack.wav");
		
	//apc
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "apc" + PATH_DELIMITER;
	copyFile( sMAXPath + "APC1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "APC5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "APC5.WAV", path + "drive.wav", 1);
	copyFile( sMAXPath + "APC7.WAV", path + "stop.wav");
	copyFile( sMAXPath + "SUB2.WAV", path + "wait_water.wav");
	copyPartOfWAV( sMAXPath + "SUB6.WAV", path + "start_water.wav", 0);
	copyPartOfWAV( sMAXPath + "SUB6.WAV", path + "drive_water.wav", 1);
	copyFile( sMAXPath + "SUB8.WAV", path + "stop_water.wav");
		
	//assault
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "assault" + PATH_DELIMITER;
	copyFile( sMAXPath + "TANKA_1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "TANKA_5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "TANKA_5.WAV", path + "drive.wav", 1);
	copyFile( sMAXPath + "TANKA_7.WAV", path + "stop.wav");
	copyFile( sMAXPath + "ASGUN14.WAV", path + "attack.wav");
		
	//awac
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "awac" + PATH_DELIMITER;
	copyFile( sMAXPath + "AWAC1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "AWAC5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "AWAC5.WAV", path + "drive.wav", 1);
	copyFile( sMAXPath + "AWAC7.WAV", path + "stop.wav");
		
	//bomber
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "bomber" + PATH_DELIMITER;
	copyFile( sMAXPath + "ATTACK1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "ATTACK5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "ATTACK5.WAV", path + "drive.wav", 1);
	copyFile( sMAXPath + "ATTACK7.WAV", path + "stop.wav");
	copyFile( sMAXPath + "MISLFIRE.WAV", path + "attack.wav");
		
	//bulldozer
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "bulldozer" + PATH_DELIMITER;
	copyFile( sMAXPath + "BULL1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "BULL5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "BULL5.WAV", path + "drive.wav", 1);
	copyFile( sMAXPath + "BULL7.WAV", path + "stop.wav");
	
	//cargoship
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "cargoship" + PATH_DELIMITER;
	copyFile( sMAXPath + "MBOATIDL.WAV", path + "wait_water.wav");
	copyPartOfWAV( sMAXPath + "MBOATMVE.WAV", path + "start_water.wav", 0);
	copyPartOfWAV( sMAXPath + "MBOATMVE.WAV", path + "drive_water.wav", 1);
	copyFile( sMAXPath + "MBOATSTP.WAV", path + "stop_water.wav");
		
	//cluster
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "cluster" + PATH_DELIMITER;
	copyFile( sMAXPath + "TANKC_1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "TANKC_5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "TANKC_5.WAV", path + "drive.wav", 1);
	copyFile( sMAXPath + "TANKC_7.WAV", path + "stop.wav");
	copyFile( sMAXPath + "MISLFIRE.WAV", path + "attack.wav");

	//commando
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "commando" + PATH_DELIMITER;
	copyFile( sMAXPath + "MANMOVE.WAV", path + "drive.wav");
	copyFile( sMAXPath + "INFIL14.WAV", path + "attack.wav");

	//corvet
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "corvet" + PATH_DELIMITER;
	copyFile( sMAXPath + "SBOATIDL.WAV", path + "wait_water.wav");
	copyPartOfWAV( sMAXPath + "SBOATMVE.WAV", path + "start_water.wav", 0);
	copyPartOfWAV( sMAXPath + "SBOATMVE.WAV", path + "drive_water.wav", 1);
	copyFile( sMAXPath + "SBOATSTP.WAV", path + "stop_water.wav");
	copyFile( sMAXPath + "CORVT14.WAV", path + "attack.wav");
		
	//escort
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "escort" + PATH_DELIMITER;
	copyFile( sMAXPath + "SBOATIDL.WAV", path + "wait_water.wav");
	copyPartOfWAV( sMAXPath + "SBOATMVE.WAV", path + "start_water.wav", 0);
	copyPartOfWAV( sMAXPath + "SBOATMVE.WAV", path + "drive_water.wav", 1);
	copyFile( sMAXPath + "SBOATSTP.WAV", path + "stop_water.wav");
	copyFile( sMAXPath + "CORVT14.WAV", path + "attack.wav");
		
	//fighter
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "fighter" + PATH_DELIMITER;
	copyFile( sMAXPath + "FIGHT1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "FIGHT5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "FIGHT5.WAV", path + "drive.wav", 1);
	copyFile( sMAXPath + "FIGHT7.WAV", path + "stop.wav");
	copyFile( sMAXPath + "SCOUT14.WAV", path + "attack.wav");
		
	//gunboat
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "gunboat" + PATH_DELIMITER;
	copyFile( sMAXPath + "GBOATIDL.WAV", path + "wait_water.wav");
	copyPartOfWAV( sMAXPath + "GBOATMVE.WAV", path + "start_water.wav", 0);
	copyPartOfWAV( sMAXPath + "GBOATMVE.WAV", path + "drive_water.wav", 1);
	copyFile( sMAXPath + "GBOATSTP.WAV", path + "stop_water.wav");
	copyFile( sMAXPath + "FARTY14.WAV", path + "attack.wav");

	//infantery
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "infantery" + PATH_DELIMITER;
	copyFile( sMAXPath + "MANMOVE.WAV", path + "drive.wav");
	copyFile( sMAXPath + "INFAN14.WAV", path + "attack.wav");
			
	//konstrukt
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "konstrukt" + PATH_DELIMITER;
	copyFile( sMAXPath + "CONST2.WAV", path + "wait_water.wav");
	copyPartOfWAV( sMAXPath + "CONST6.WAV", path + "start_water.wav", 0);
	copyPartOfWAV( sMAXPath + "CONST6.WAV", path + "drive_water.wav", 1);
	copyFile( sMAXPath + "CONST8.WAV", path + "stop_water.wav");
	copyFile( sMAXPath + "CONST1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "CONST5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "CONST5.WAV", path + "drive.wav", 1);
	copyFile( sMAXPath + "CONST7.WAV", path + "stop.wav");
		
	//minelayer
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "minelayer" + PATH_DELIMITER;
	copyFile( sMAXPath + "TANKC_1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "TANKC_5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "TANKC_5.WAV", path + "drive.wav", 1);
	copyFile( sMAXPath + "TANKC_7.WAV", path + "stop.wav");
		
	//missel
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "missel" + PATH_DELIMITER;
	copyFile( sMAXPath + "TANKC_1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "TANKC_5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "TANKC_5.WAV", path + "drive.wav", 1);
	copyFile( sMAXPath + "TANKC_7.WAV", path + "stop.wav");
	copyFile( sMAXPath + "MISLFIRE.WAV", path + "attack.wav");
		
	//missel_ship
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "missel_ship" + PATH_DELIMITER;
	copyFile( sMAXPath + "MBOATIDL.WAV", path + "wait_water.wav");
	copyPartOfWAV( sMAXPath + "MBOATMVE.WAV", path + "start_water.wav", 0);
	copyPartOfWAV( sMAXPath + "MBOATMVE.WAV", path + "drive_water.wav", 1);
	copyFile( sMAXPath + "MBOATSTP.WAV", path + "stop_water.wav");
	copyFile( sMAXPath + "MISLFIRE.WAV", path + "attack.wav");
	
	//mobile_aa
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "mobile_aa" + PATH_DELIMITER;
	copyFile( sMAXPath + "APC1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "APC5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "APC5.WAV", path + "drive.wav", 1);
	copyFile( sMAXPath + "APC7.WAV", path + "stop.wav");
	copyFile( sMAXPath + "MANTI14.WAV", path + "attack.wav");
		
	//pionier
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "pionier" + PATH_DELIMITER;
	copyFile( sMAXPath + "ENGIN2.WAV", path + "wait_water.wav");
	copyPartOfWAV( sMAXPath + "ENGIN6.WAV", path + "start_water.wav", 0);
	copyPartOfWAV( sMAXPath + "ENGIN6.WAV", path + "drive_water.wav", 1);
	copyFile( sMAXPath + "ENGIN8.WAV", path + "stop_water.wav");
	copyFile( sMAXPath + "ENGIN1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "ENGIN5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "ENGIN5.WAV", path + "drive.wav", 1);
	copyFile( sMAXPath + "ENGIN7.WAV", path + "stop.wav");
		
	//repair
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "repair" + PATH_DELIMITER;
	copyFile( sMAXPath + "REPAIR1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "REPAIR5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "REPAIR5.WAV", path + "drive.wav", 1);
	copyFile( sMAXPath + "REPAIR7.WAV", path + "stop.wav");
		
	//scanner
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "scanner" + PATH_DELIMITER;
	copyFile( sMAXPath + "SCAN1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "SCAN5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "SCAN5.WAV", path + "drive.wav", 1);
	copyFile( sMAXPath + "SCAN7.WAV", path + "stop.wav");
		
	//scout
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "scout" + PATH_DELIMITER;
	copyFile( sMAXPath + "SCOUT2.WAV", path + "wait_water.wav");
	copyPartOfWAV( sMAXPath + "SCOUT6.WAV", path + "start_water.wav", 0);
	copyPartOfWAV( sMAXPath + "SCOUT6.WAV", path + "drive_water.wav", 1);
	copyFile( sMAXPath + "SCOUT8.WAV", path + "stop_water.wav");
	copyFile( sMAXPath + "SCOUT1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "SCOUT5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "SCOUT5.WAV", path + "drive.wav", 1);
	copyFile( sMAXPath + "SCOUT7.WAV", path + "stop.wav");
	copyFile( sMAXPath + "SCOUT14.WAV", path + "attack.wav");
		
	//sea_minelayer
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "sea_minelayer" + PATH_DELIMITER;
	copyFile( sMAXPath + "GBOATIDL.WAV", path + "wait_water.wav");
	copyPartOfWAV( sMAXPath + "GBOATMVE.WAV", path + "start_water.wav", 0);
	copyPartOfWAV( sMAXPath + "GBOATMVE.WAV", path + "drive_water.wav", 1);
	copyFile( sMAXPath + "GBOATSTP.WAV", path + "stop_water.wav");
		
	//sea_transport
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "sea_transport" + PATH_DELIMITER;
	copyFile( sMAXPath + "GBOATIDL.WAV", path + "wait_water.wav");
	copyPartOfWAV( sMAXPath + "GBOATMVE.WAV", path + "start_water.wav", 0);
	copyPartOfWAV( sMAXPath + "GBOATMVE.WAV", path + "drive_water.wav", 1);
	copyFile( sMAXPath + "GBOATSTP.WAV", path + "stop_water.wav");
		
	//sub
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "sub" + PATH_DELIMITER;
	copyFile( sMAXPath + "SUB2.WAV", path + "wait_water.wav");
	copyPartOfWAV( sMAXPath + "SUB6.WAV", path + "start_water.wav", 0);
	copyPartOfWAV( sMAXPath + "SUB6.WAV", path + "drive_water.wav", 1);
	copyFile( sMAXPath + "SUB8.WAV", path + "stop_water.wav");
	copyFile( sMAXPath + "SUB14.WAV", path + "attack.wav");
		
	//surveyor
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "surveyor" + PATH_DELIMITER;
	copyFile( sMAXPath + "SURVY2.WAV", path + "wait_water.wav");
	copyPartOfWAV( sMAXPath + "SURVY6.WAV", path + "start_water.wav", 0);
	copyPartOfWAV( sMAXPath + "SURVY6.WAV", path + "drive_water.wav", 1);
	copyFile( sMAXPath + "SURVY8.WAV", path + "stop_water.wav");
	copyFile( sMAXPath + "SURVY1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "SURVY5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "SURVY5.WAV", path + "drive.wav", 1);
	copyFile( sMAXPath + "SURVY7.WAV", path + "stop.wav");
		
	//tank
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "tank" + PATH_DELIMITER;
	copyFile( sMAXPath + "TANK1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "TANK5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "TANK5.WAV", path + "drive.wav", 1);
	copyFile( sMAXPath + "TANK7.WAV", path + "stop.wav");
	copyFile( sMAXPath + "CANFIRE.WAV", path + "attack.wav");
		
	//trans_gold
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "trans_gold" + PATH_DELIMITER;
	copyFile( sMAXPath + "TRUCKIDL.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "TRUCKMVE.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "TRUCKMVE.WAV", path + "drive.wav", 1);
	copyFile( sMAXPath + "TRUCKSTP.WAV", path + "stop.wav");
		
	//trans_metal
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "trans_metal" + PATH_DELIMITER;
	copyFile( sMAXPath + "TRUCKIDL.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "TRUCKMVE.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "TRUCKMVE.WAV", path + "drive.wav", 1);
	copyFile( sMAXPath + "TRUCKSTP.WAV", path + "stop.wav");
		
	//trans_oil
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "trans_oil" + PATH_DELIMITER;
	copyFile( sMAXPath + "TRUCKIDL.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "TRUCKMVE.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "TRUCKMVE.WAV", path + "drive.wav", 1);
	copyFile( sMAXPath + "TRUCKSTP.WAV", path + "stop.wav");
	
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


int main ( int argc, char* argv[] )
{
	while ( 1 )
	{
		//fixme: path's with space characters don't work
		cout << "Please enter path to MAX-Installation or MAX-CD: ";
		cin >> sMAXPath;
		//sMAXPath = "C:\\Dokumente und Einstellungen\\Eiko\\Desktop\\MAX-Develop\\MAX\\"; //temp

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
		
		cout << "Could not open resourcefile\n";

	}

	while (1)
	{
		//fixme: path's with space characters don't work
		cout << "\nPlease enter path to outputfolder: ";
		cin >> sOutputPath;
		//sOutputPath = "C:\\Dokumente und Einstellungen\\Eiko\\Desktop\\MAX-Develop\\MAXR Install\\output - install skript\\";
		
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

		cout << "MAX Reloaded installation not found in the given folder.\n";
	}
	
	//create log file
	logFile = SDL_RWFromFile("resinstaller.log", "w" );
	if ( logFile == NULL )
	{
		cout << "Warning: Couldn't create log file. writing to stdout.\n";
	}

	wasError = 0;
		
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
		cout << "Error:  [EOD] not found in res-File.";
		exit (-1);
	}

	installBuildingSounds();
	installVehicleSounds();
	installVehicleVideos();
	installVehicleGraphics();
	installBuildingGraphics();

	if ( wasError )
	{
		cout << "There were errors while installing. See 'resinstaller.log' for details.\n";
	}
	else
	{
		cout << "Finished\n";
	}
	
	SDL_RWclose( res );
	SDL_RWclose( logFile );
	
	//while (1);
	return 0;
}
