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
	SDL_Rect src_rect, dst_rect;
	char szNum[13];
	char szNum1[13];
	char szNum2[13];
	SDL_Surface *surface, *output;
	
	cout << "Vehicle graphics\n";

	//air_transport
	cout << "air_transport\n";
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
	cout << "alien_assault\n";
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
	cout << "alien_plane\n";
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
	cout << "alien_ship\n";
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
	cout << "alien_tank\n";
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
	cout << "apc\n";
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
	cout << "assault\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "assault" + PATH_DELIMITER;
	for (int i = 0; i < 8; i++)
	{
		sprintf( szNum, "%d", i);
		surface = getImage("ARTILLRY", i);
		removePlayerColor( surface );
		resizeSurface ( surface, 23, 24, 64, 64);
		savePCX(surface, path + "img" + szNum + ".pcx");
		SDL_FreeSurface( surface );
		
		copyFileFromRes("S_ARTILL", path + "shw" + szNum + ".pcx", i );
	}
	copyFileFromRes("A_ARTY", path + "store.pcx");
	copyFileFromRes("P_ARTY", path + "info.pcx");

	//awac
	cout << "awac\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "awac" + PATH_DELIMITER;
	for ( int i = 0; i < 8; i++)
	{
		sprintf( szNum, "%d", i);
		copyFileFromRes_rpc("AWAC", path + "img" + szNum + ".pcx", i);
		copyFileFromRes("S_AWAC", path + "shw" + szNum + ".pcx", i);
	}
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

	copyFileFromRes("A_AWAC", path + "store.pcx");
	copyFileFromRes("P_AWAC", path + "info.pcx");

	//bomber
	cout << "bomber\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "bomber" + PATH_DELIMITER;
	for ( int i = 0; i < 8; i++)
	{
		sprintf(szNum, "%d", i);
		surface = getImage("BOMBER", i);
		removePlayerColor( surface );
		resizeSurface( surface, 14, 18, 64, 64);
		savePCX(surface, path + "img" + szNum + ".pcx");
		SDL_FreeSurface( surface );

		copyFileFromRes("S_BOMBER", path + "shw" + szNum + ".pcx", i);
	}
	copyFileFromRes("A_BOMBER", path + "store.pcx");
	copyFileFromRes("P_BOMBER", path + "info.pcx");

	//bulldozer
	cout << "bulldozer\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "bulldozer" + PATH_DELIMITER;
	for ( int i = 0; i < 8; i++ )
	{
		sprintf( szNum, "%d", i);
		surface = getImage("BULLDOZR", i);
		removePlayerColor( surface );
		resizeSurface( surface, 4, 4, 64, 64);
		savePCX( surface, path + "img" + szNum + ".pcx");
		SDL_FreeSurface( surface );
		
		surface = getImage("S_BULLDO", i);
		resizeSurface( surface, 4, 4, 68, 68);
		savePCX( surface, path + "shw" + szNum + ".pcx");
		SDL_FreeSurface( surface );
	}
	copyFileFromRes("A_BULLDZ", path + "store.pcx");
	copyFileFromRes("P_BULLDZ", path + "info.pcx");
	
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

	output = getImage("S_LRGCON");
	resizeSurface( output, 6, 6, 128, 128 );
	surface = getImage("S_BULLDO");
	dst_rect.x = 38;
	dst_rect.y = 37;
	SDL_BlitSurface( surface, 0, output, &dst_rect );
	SDL_FreeSurface( surface );
	savePCX( output, path + "clear_big_shw.pcx");
	SDL_FreeSurface( output );
	
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

	output = getImage("S_SMLCON");
	resizeSurface( output, 6, 6, 64, 66 );
	surface = getImage("S_BULLDO");
	dst_rect.x = 6;
	dst_rect.y = 5;
	SDL_BlitSurface( surface, 0, output, &dst_rect );
	SDL_FreeSurface( surface );

	savePCX( output, path + "clear_small_shw.pcx");
	SDL_FreeSurface( output );

	//cargoship
	cout << "cargoship\n";
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
	cout << "cluster\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "cluster" + PATH_DELIMITER;
	for ( int i = 0; i < 8; i++)
	{
		sprintf(szNum, "%d", i);
		surface = getImage("ROCKTLCH", i);
		removePlayerColor( surface );
		resizeSurface( surface, 15, 15, 64, 64);
		savePCX(surface, path + "img" + szNum + ".pcx");
		SDL_FreeSurface( surface );

		copyFileFromRes("S_ROCKTL", path + "shw" + szNum + ".pcx", i);
	}
	copyFileFromRes("A_ROCKET", path + "store.pcx");
	copyFileFromRes("P_ROCKET", path + "info.pcx");

	//commando
	cout << "commando\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "commando" + PATH_DELIMITER;
	for (int i = 0; i < 8; i++)
	{
		sprintf( szNum1, "%d", i);
		for (int n = 0; n < 13; n++)
		{
			sprintf( szNum2, "%.2d", n);
			surface = getImage("COMMANDO", n * 8 + i );
			removePlayerColor( surface );
			resizeSurface( surface, 73, 73, 64, 64);
			savePCX(surface, path + "img" + szNum + ".pcx");
			SDL_FreeSurface( surface );
		}
	}
	copyFileFromRes("A_COMMAN", path + "store.pcx");
	copyFileFromRes("P_COMMAN", path + "info.pcx");

	//corvet
	cout << "corvet\n";
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
	cout << "escort\n";
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
	cout << "fighter\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "fighter" + PATH_DELIMITER;
	for ( int i = 0; i < 8; i++)
	{
		sprintf(szNum, "%d", i);
		surface = getImage("FIGHTER", i);
		removePlayerColor( surface );
		resizeSurface( surface, 17, 16, 64, 64);
		savePCX(surface, path + "img" + szNum + ".pcx");
		SDL_FreeSurface( surface );

		copyFileFromRes("S_FIGHTE", path + "shw" + szNum + ".pcx", i);
	}
	copyFileFromRes("A_FIGHTR", path + "store.pcx");
	copyFileFromRes("P_FIGHTR", path + "info.pcx");
	
	//gunboat
	cout << "gunboat\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "gunboat" + PATH_DELIMITER;
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

	for (int i = 0; i < 8; i++ )
	{
		sprintf(szNum, "%d", i);
		copyFileFromRes("S_BATTLS", path + "shw" + szNum + ".pcx", i);
	}
	copyFileFromRes("A_GUNBT", path + "store.pcx");
	copyFileFromRes("P_GUNBT", path + "info.pcx");

	//infantery
	cout << "infantery\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "infantery" + PATH_DELIMITER;
	for (int i = 0; i < 8; i++)
	{
		sprintf( szNum1, "%d", i);
		for (int n = 0; n < 13; n++)
		{
			sprintf( szNum2, "%.2d", n);
			surface = getImage("INFANTRY", n * 8 + i );
			removePlayerColor( surface );
			resizeSurface( surface, 73, 73, 64, 64);
			savePCX(surface, path + "img" + szNum1 + "_" + szNum2 + ".pcx");
			SDL_FreeSurface( surface );
		}
	}
	copyFileFromRes("A_INFANT", path + "store.pcx");
	copyFileFromRes("P_INFANT", path + "info.pcx");
	
	//konstrukt
	cout << "konstrukt\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "konstrukt" + PATH_DELIMITER;
	for ( int i = 0; i < 8; i++ )
	{
		sprintf( szNum, "%d", i);
		copyFileFromRes_rpc("CONSTRCT", path + "img" + szNum + ".pcx", i);
		copyFileFromRes("S_CONSTR", path + "shw" + szNum + ".pcx", i);
	}
	copyFileFromRes("A_CONTRC", path + "store.pcx");
	copyFileFromRes("P_CONTRC", path + "info.pcx");

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

	output = getImage("S_LRGCON");
	resizeSurface( output, 6, 6, 128, 128 );
	surface = getImage("S_CONSTR");
	dst_rect.x = 38;
	dst_rect.y = 37;
	SDL_BlitSurface( surface, 0, output, &dst_rect );
	SDL_FreeSurface( surface );
	savePCX( output, path + "build_shw.pcx");
	SDL_FreeSurface( output );


	//minelayer
	cout << "minelayer\n";
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
	cout << "missel\n";
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
	cout << "missel_ship\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "missel_ship" + PATH_DELIMITER;
	for ( int i = 0; i < 8; i++)
	{
		sprintf(szNum, "%d", i);
		surface = getImage("MSSLBOAT", i);
		removePlayerColor( surface );
		resizeSurface( surface, 16, 16, 64, 64);
		savePCX(surface, path + "img" + szNum + ".pcx");
		SDL_FreeSurface( surface );
		copyFileFromRes("S_MSSLBO", path + "shw" + szNum + ".pcx", i);
	}
	copyFileFromRes("A_MSLCR", path + "store.pcx");
	copyFileFromRes("P_MSLCR", path + "info.pcx");
	
	//mobile_aa
	cout << "mobile_aa\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "mobile_aa" + PATH_DELIMITER;
	src_rect.h = 33; 
	src_rect.w = 35;
	src_rect.x = 17;
	src_rect.y = 16;

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

	for (int i = 0; i < 8; i++ )
	{
		sprintf( szNum, "%d", i);
		copyFileFromRes("S_FLAK", path + "shw" + szNum + ".pcx", i);
	}
	copyFileFromRes("A_AA", path + "store.pcx");
	copyFileFromRes("P_AA", path + "info.pcx");

	//pionier
	cout << "pionier\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "pionier" + PATH_DELIMITER;
	for (int i = 0; i < 8; i++ )
	{
		sprintf( szNum, "%d", i);
		surface = getImage("ENGINEER", i);
		removePlayerColor( surface );
		resizeSurface( surface, 4, 4, 64, 64);
		savePCX(surface, path + "img" + szNum + ".pcx");
		SDL_FreeSurface( surface );
		
		surface = getImage("S_ENGINE", i);
		resizeSurface( surface, 4, 4, 67, 66);
		savePCX( surface, path + "shw" + szNum + ".pcx");
		SDL_FreeSurface( surface );
	}
	copyFileFromRes("A_ENGINR", path + "store.pcx");
	copyFileFromRes("P_ENGINR", path + "info.pcx");

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

	output = getImage("S_SMLCON");
	resizeSurface( output, 6, 6, 64, 66 );
	surface = getImage("S_ENGINE");
	dst_rect.x = 6;
	dst_rect.y = 5;
	SDL_BlitSurface( surface, 0, output, &dst_rect );
	SDL_FreeSurface( surface );
	savePCX( output, path + "build_shw.pcx");
	SDL_FreeSurface( output );


	//repair
	cout << "repair\n";
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
	cout << "scanner\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "scanner" + PATH_DELIMITER;
	for ( int i = 0; i < 8; i++)
	{
		sprintf( szNum, "%d", i);
		surface = getImage("SCANNER", i);
		removePlayerColor( surface );
		resizeSurface( surface, 8, 8, 64, 64);
		savePCX( surface, path + "img" + szNum + ".pcx");
		SDL_FreeSurface( surface );
		surface = getImage("S_SCANNE", i);
		resizeSurface( surface, 8, 8, 61, 60);
		savePCX( surface, path + "shw" + szNum + ".pcx");
		SDL_FreeSurface( surface );
	}
	copyFileFromRes("A_SCANNR", path + "store.pcx");
	copyFileFromRes("P_SCANNR", path + "info.pcx");

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

	//scout
	cout << "scout\n";
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
	cout << "sea_minelayer\n";
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
	cout << "sea_transport\n";
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
	cout << "sub\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "sub" + PATH_DELIMITER;
	for ( int i = 0; i < 8; i++)
	{
		sprintf( szNum, "%d", i);
		copyFileFromRes_rpc("SUBMARNE", path + "img" + szNum + ".pcx", i + 8);
	}
	copyFileFromRes("A_SUB", path + "store.pcx");
	copyFileFromRes("P_SUB", path + "info.pcx");

	//surveyor
	cout << "surveyor\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "surveyor" + PATH_DELIMITER;
	for ( int i = 0; i < 8; i++)
	{
		sprintf( szNum, "%d", i);
		surface = getImage("SURVEYOR", i);
		removePlayerColor( surface );
		resizeSurface( surface, 2, 2, 64, 64);
		savePCX( surface, path + "img" + szNum + ".pcx");
		SDL_FreeSurface( surface );
		surface = getImage("S_SURVEY", i);
		resizeSurface( surface, 2, 2, 69, 72);
		savePCX( surface, path + "shw" + szNum + ".pcx");
		SDL_FreeSurface( surface );
	}
	copyFileFromRes("A_SURVEY", path + "store.pcx");
	copyFileFromRes("P_SURVEY", path + "info.pcx");
	
	//tank
	cout << "tank\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "tank" + PATH_DELIMITER;
	for ( int i = 0; i < 8; i++)
	{
		sprintf( szNum, "%d", i);
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
		surface = getImage("S_TANK", i);
		resizeSurface( surface, 2, 2, 69, 72);
		savePCX( surface, path + "shw" + szNum + ".pcx");
		SDL_FreeSurface ( surface );
	}
	copyFileFromRes("A_TANK", path + "store.pcx");
	copyFileFromRes("P_TANK", path + "info.pcx");

	//trans_gold
	cout << "trans_gold\n";
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
	cout << "trans_metal\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "trans_metal" + PATH_DELIMITER;
	for (int i = 0; i < 8; i++)
	{
		sprintf( szNum, "%d", i);
		surface = getImage("SPLYTRCK", i);
		removePlayerColor( surface );
		resizeSurface( surface, 4, 4, 64, 64);
		savePCX( surface, path + "img" + szNum + ".pcx");
		SDL_FreeSurface ( surface );
		surface = getImage("S_SPLYTR", i);
		resizeSurface( surface, 4, 4, 64, 64);
		savePCX( surface, path + "shw" + szNum + ".pcx");
		SDL_FreeSurface ( surface );
	}
	copyFileFromRes("A_SPLYTR", path + "store.pcx");
	copyFileFromRes("P_SPLYTR", path + "info.pcx");
	
	//trans_oil
	cout << "trans_oil\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "trans_oil" + PATH_DELIMITER;
	for (int i = 0; i < 8; i++)
	{
		sprintf( szNum, "%d", i);
		surface = getImage("FUELTRCK", i);
		removePlayerColor( surface );
		resizeSurface( surface, 3, 3, 64, 64);
		savePCX( surface, path + "img" + szNum + ".pcx");
		SDL_FreeSurface ( surface );
		surface = getImage("S_FUELTR", i);
		resizeSurface( surface, 3, 3, 66, 66);
		savePCX( surface, path + "shw" + szNum + ".pcx");
		SDL_FreeSurface ( surface );
	}
	copyFileFromRes("A_FUELTR", path + "store.pcx");
	copyFileFromRes("P_FUELTR", path + "info.pcx");

	return 1;
}

int installBuildingGraphics()
{
	string path;
	SDL_Surface* surface;
	SDL_Surface* output;
	SDL_Rect src_rect, dst_rect;

	cout << "Building graphics\n";

	//Barracks
	cout << "Barracks\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "barracks" + PATH_DELIMITER;
	copyFileFromRes_rpc("BARRACKS", path + "img.pcx", 1 );
	copyFileFromRes("P_BARRCK", path + "info.pcx");
	copyFileFromRes("S_BARRAC", path + "shw.pcx");
	copyImageFromFLC( sMAXPath + "BARX_ISO.FLC", path + "video.pcx");

	//block
	cout << "block\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "block" + PATH_DELIMITER;
	copyFileFromRes("BLOCK", path + "img.pcx");
	copyFileFromRes("P_BLOCK", path + "info.pcx");
	copyFileFromRes("S_BLOCK", path + "shw.pcx");
	copyImageFromFLC( sMAXPath + "BLOK128.FLC", path + "video.pcx");

	//bridge
	cout << "bridge\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "bridge" + PATH_DELIMITER;
	copyFileFromRes_rpc("BRIDGE", path + "img.pcx");
	copyFileFromRes("P_BRIDGE", path + "info.pcx");
	copyFileFromRes("S_BRIDGE", path + "shw.pcx");
	copyImageFromFLC( sMAXPath + "BRIDGE.FLC", path + "video.pcx");

	//connector
	cout << "connector\n";
	SDL_Surface* output_s, *surface_s;
	path = sOutputPath + "buildings" + PATH_DELIMITER + "connector" + PATH_DELIMITER;
	surface = getImage("CNCT_4W", 0);
	surface_s = getImage("S_CNCT4W", 0);
	output = SDL_CreateRGBSurface(SDL_SWSURFACE, 1024, 64,8,0,0,0,0);
	output_s = SDL_CreateRGBSurface(SDL_SWSURFACE, 1024, 64,8,0,0,0,0);
	SDL_SetColors(output, surface->format->palette->colors, 0, 256);
	SDL_SetColors(output_s, surface_s->format->palette->colors, 0, 256);
	SDL_FillRect( output, 0, SDL_MapRGB( output->format, 255, 0, 255));
	SDL_FillRect( output_s, 0, SDL_MapRGB( output_s->format, 255, 0, 255));
	dst_rect.y = 0;
	for ( int i = 0; i < 1024; i+=64 )
	{
		dst_rect.x = i;
		SDL_BlitSurface(surface, 0, output, &dst_rect);
		SDL_BlitSurface(surface_s, 0, output_s, &dst_rect);

	}
	SDL_FreeSurface( surface );
	SDL_FreeSurface( surface_s );

	surface = getImage("CNCT_4W", 2 );
	surface_s = getImage("S_CNCT4W", 2);
	dst_rect.x = 64;
	SDL_BlitSurface(surface, 0, output, &dst_rect);
	SDL_BlitSurface(surface_s, 0, output_s, &dst_rect);
	dst_rect.x = 320;
	SDL_BlitSurface(surface, 0, output, &dst_rect);
	SDL_BlitSurface(surface_s, 0, output_s, &dst_rect);
	dst_rect.x = 448;
	SDL_BlitSurface(surface, 0, output, &dst_rect);
	SDL_BlitSurface(surface_s, 0, output_s, &dst_rect);
	dst_rect.x = 512;
	SDL_BlitSurface(surface, 0, output, &dst_rect);
	SDL_BlitSurface(surface_s, 0, output_s, &dst_rect);
	dst_rect.x = 704;
	SDL_BlitSurface(surface, 0, output, &dst_rect);
	SDL_BlitSurface(surface_s, 0, output_s, &dst_rect);
	dst_rect.x = 768;
	SDL_BlitSurface(surface, 0, output, &dst_rect);
	SDL_BlitSurface(surface_s, 0, output_s, &dst_rect);
	dst_rect.x = 832;
	SDL_BlitSurface(surface, 0, output, &dst_rect);
	SDL_BlitSurface(surface_s, 0, output_s, &dst_rect);
	dst_rect.x = 960;
	SDL_BlitSurface(surface, 0, output, &dst_rect);
	SDL_BlitSurface(surface_s, 0, output_s, &dst_rect);
	SDL_FreeSurface( surface );
	SDL_FreeSurface( surface_s );

	surface = getImage("CNCT_4W", 3 );
	surface_s = getImage("S_CNCT4W", 3);
	dst_rect.x = 128;
	SDL_BlitSurface(surface, 0, output, &dst_rect);
	SDL_BlitSurface(surface_s, 0, output_s, &dst_rect);
	dst_rect.x = 384;
	SDL_BlitSurface(surface, 0, output, &dst_rect);
	SDL_BlitSurface(surface_s, 0, output_s, &dst_rect);
	dst_rect.x = 512;
	SDL_BlitSurface(surface, 0, output, &dst_rect);
	SDL_BlitSurface(surface_s, 0, output_s, &dst_rect);
	dst_rect.x = 576;
	SDL_BlitSurface(surface, 0, output, &dst_rect);
	SDL_BlitSurface(surface_s, 0, output_s, &dst_rect);
	dst_rect.x = 768;
	SDL_BlitSurface(surface, 0, output, &dst_rect);
	SDL_BlitSurface(surface_s, 0, output_s, &dst_rect);
	dst_rect.x = 832;
	SDL_BlitSurface(surface, 0, output, &dst_rect);
	SDL_BlitSurface(surface_s, 0, output_s, &dst_rect);
	dst_rect.x = 896;
	SDL_BlitSurface(surface, 0, output, &dst_rect);
	SDL_BlitSurface(surface_s, 0, output_s, &dst_rect);
	dst_rect.x = 960;
	SDL_BlitSurface(surface, 0, output, &dst_rect);
	SDL_BlitSurface(surface_s, 0, output_s, &dst_rect);
	SDL_FreeSurface( surface );
	SDL_FreeSurface( surface_s );

	surface = getImage("CNCT_4W", 4 );
	surface_s = getImage("S_CNCT4W", 4);
	dst_rect.x = 192;
	SDL_BlitSurface(surface, 0, output, &dst_rect);
	SDL_BlitSurface(surface_s, 0, output_s, &dst_rect);
	dst_rect.x = 320;
	SDL_BlitSurface(surface, 0, output, &dst_rect);
	SDL_BlitSurface(surface_s, 0, output_s, &dst_rect);
	dst_rect.x = 576;
	SDL_BlitSurface(surface, 0, output, &dst_rect);
	SDL_BlitSurface(surface_s, 0, output_s, &dst_rect);
	dst_rect.x = 640;
	SDL_BlitSurface(surface, 0, output, &dst_rect);
	SDL_BlitSurface(surface_s, 0, output_s, &dst_rect);
	dst_rect.x = 704;
	SDL_BlitSurface(surface, 0, output, &dst_rect);
	SDL_BlitSurface(surface_s, 0, output_s, &dst_rect);
	dst_rect.x = 832;
	SDL_BlitSurface(surface, 0, output, &dst_rect);
	SDL_BlitSurface(surface_s, 0, output_s, &dst_rect);
	dst_rect.x = 896;
	SDL_BlitSurface(surface, 0, output, &dst_rect);
	SDL_BlitSurface(surface_s, 0, output_s, &dst_rect);
	dst_rect.x = 960;
	SDL_BlitSurface(surface, 0, output, &dst_rect);
	SDL_BlitSurface(surface_s, 0, output_s, &dst_rect);
	SDL_FreeSurface( surface );
	SDL_FreeSurface( surface_s );

	surface = getImage("CNCT_4W", 5 );
	surface_s = getImage("S_CNCT4W", 5);
	dst_rect.x = 256;
	SDL_BlitSurface(surface, 0, output, &dst_rect);
	SDL_BlitSurface(surface_s, 0, output_s, &dst_rect);
	dst_rect.x = 384;
	SDL_BlitSurface(surface, 0, output, &dst_rect);
	SDL_BlitSurface(surface_s, 0, output_s, &dst_rect);
	dst_rect.x = 448;
	SDL_BlitSurface(surface, 0, output, &dst_rect);
	SDL_BlitSurface(surface_s, 0, output_s, &dst_rect);
	dst_rect.x = 640;
	SDL_BlitSurface(surface, 0, output, &dst_rect);
	SDL_BlitSurface(surface_s, 0, output_s, &dst_rect);
	dst_rect.x = 704;
	SDL_BlitSurface(surface, 0, output, &dst_rect);
	SDL_BlitSurface(surface_s, 0, output_s, &dst_rect);
	dst_rect.x = 768;
	SDL_BlitSurface(surface, 0, output, &dst_rect);
	SDL_BlitSurface(surface_s, 0, output_s, &dst_rect);
	dst_rect.x = 896;
	SDL_BlitSurface(surface, 0, output, &dst_rect);
	SDL_BlitSurface(surface_s, 0, output_s, &dst_rect);
	dst_rect.x = 960;
	SDL_BlitSurface(surface, 0, output, &dst_rect);
	SDL_BlitSurface(surface_s, 0, output_s, &dst_rect);
	SDL_FreeSurface( surface );
	SDL_FreeSurface( surface_s );

	savePCX( output, path + "img.pcx");
	SDL_FreeSurface( output );
	savePCX( output_s, path + "shw.pcx");
	SDL_FreeSurface( output_s );

	copyFileFromRes("P_CONNEC", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "CROSS.FLC", path + "video.pcx");

	//depot
	cout << "depot\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "depot" + PATH_DELIMITER;
	copyFileFromRes_rpc("DEPOT", path + "img.pcx", 1);
	copyFileFromRes("P_DEPOT", path + "info.pcx");
	copyFileFromRes("S_DEPOT", path + "shw.pcx", 1);
	copyImageFromFLC( sMAXPath + "DPBG_S.FLC", path + "video.pcx");



	//dock
	cout << "dock\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "dock" + PATH_DELIMITER;
	copyFileFromRes_rpc("DOCK", path + "img.pcx");
	copyFileFromRes("S_DOCK", path + "shw.pcx");
	copyFileFromRes("P_DOCK", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "DOCK.FLC", path + "video.pcx");

	//energy big
	cout << "energy big\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "energy_big" + PATH_DELIMITER;
	copyFileFromRes_rpc("POWERSTN", path + "img.pcx");
	copyFileFromRes("S_POWERS", path + "shw.pcx");
	copyFileFromRes("P_POWSTN", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "PWRGEN.FLC", path + "video.pcx");

	//energy small
	cout << "energy small\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "energy_small" + PATH_DELIMITER;
	copyFileFromRes_rpc("POWGEN", path + "img.pcx");
	copyFileFromRes("S_POWGEN", path + "shw.pcx");
	copyFileFromRes("P_POWGEN", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "SPWR_ISO.FLC", path + "video.pcx");

	//fac air
	cout << "fac air\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "fac_air" + PATH_DELIMITER;
	copyFileFromRes_rpc("AIRPLT", path + "img.pcx");
	copyFileFromRes("S_AIRPLT", path + "shw.pcx");
	copyFileFromRes("P_AIRPLT", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "AIRPLNT.FLC", path + "video.pcx");

	//fac alien
	cout << "fac alien\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "fac_alien" + PATH_DELIMITER;
	copyFileFromRes_rpc("RECCENTR", path + "img.pcx");
	copyFileFromRes("S_RECCEN", path + "shw.pcx");
	copyFileFromRes("P_RECCTR", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "RECNTR.FLC", path + "video.pcx");

	//fac big
	cout << "fac big\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "fac_big" + PATH_DELIMITER;
	copyFileFromRes_rpc("LANDPLT", path + "img.pcx");
	copyFileFromRes("S_LANDPL", path + "shw.pcx");
	copyFileFromRes("P_HVYPLT", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "FVP.FLC", path + "video.pcx");

	//fac ship
	cout << "fac ship\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "fac_ship" + PATH_DELIMITER;
	copyFileFromRes_rpc("SHIPYARD", path + "img.pcx");
	copyFileFromRes("S_SHIPYA", path + "shw.pcx");
	copyFileFromRes("P_SHIPYD", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "SHPYRD.FLC", path + "video.pcx");

	//fac small
	cout << "fac small\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "fac_small" + PATH_DELIMITER;
	copyFileFromRes_rpc("LIGHTPLT", path + "img.pcx");
	copyFileFromRes("S_LIGHTP", path + "shw.pcx");
	copyFileFromRes("P_LGHTPL", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "LVP_ISO.FLC", path + "video.pcx");

	//goldraff
	cout << "goldraff\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "goldraff" + PATH_DELIMITER;
	copyFileFromRes_rpc("COMMTWR", path + "img.pcx");
	copyFileFromRes("S_COMMTW", path + "shw.pcx");
	copyFileFromRes("P_TRANSP", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "COMMTWR.FLC", path + "video.pcx");

	//gun aa
	cout << "gun aa\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "gun_aa" + PATH_DELIMITER;
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
	
	copyFileFromRes("S_ANTIAI", path + "shw.pcx", 8); 
	copyFileFromRes("P_FXAA", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "AA_ISO.FLC", path + "video.pcx");

	//gun ari
	cout << "gun ari\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "gun_ari" + PATH_DELIMITER;
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

	copyFileFromRes("S_ARTYTR", path + "shw.pcx", 8);
	copyFileFromRes("P_ARTYTR", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "FXDGUN.FLC", path + "video.pcx");

	//gun missile
	cout << "gun missile\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "gun_missel" + PATH_DELIMITER;
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
	copyFileFromRes("S_ANTIMS", path + "shw.pcx", 1);
	copyFileFromRes("P_FXROCK", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "ANTIMSL.FLC", path + "video.pcx");

	//gun turret
	cout << "gun turret\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "gun_turret" + PATH_DELIMITER;
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

	copyFileFromRes("S_GUNTUR", path + "shw.pcx", 1);
	copyFileFromRes("P_GUNTUR", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "FXAGUN.FLC", path + "video.pcx");

	//habitat
	cout << "habitat\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "habitat" + PATH_DELIMITER;
	copyFileFromRes_rpc("HABITAT", path + "img.pcx", 1);
	copyFileFromRes("S_HABITA", path + "shw.pcx", 1);
	copyFileFromRes("P_HABITA", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "DORM.FLC", path + "video.pcx");

	//hangar
	cout << "hangar\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "hangar" + PATH_DELIMITER;
	copyFileFromRes_rpc("HANGAR", path + "img.pcx", 1);
	copyFileFromRes("S_HANGAR", path + "shw.pcx", 1);
	copyFileFromRes("P_HANGAR", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "HANGR.FLC", path + "video.pcx");

	//landmine
	cout << "landmine\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "landmine" + PATH_DELIMITER;
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

	copyFileFromRes("P_LANDMN", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "LMINE01.FLC", path + "video.pcx");

	//mine
	cout << "mine; Graphic is temporary, until the clans will be implemented\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "mine" + PATH_DELIMITER;
	copyFileFromRes_rpc("MININGST", path + "img.pcx");	//this is temorary!
														//until the clans will be implemented
	copyFileFromRes("S_MINING", path + "shw.pcx");
	copyFileFromRes("P_MINING", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "MSTRSTAT.FLC", path + "video.pcx");

	//mine deep
	cout << "mine deep\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "mine_deep" + PATH_DELIMITER;
	copyFileFromRes_rpc("SUPRTPLT", path + "img.pcx");
	copyFileFromRes("S_SUPRTP", path + "shw.pcx");
	copyFileFromRes("P_LIFESP", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "SVP.FLC", path + "video.pcx");

	//pad
	cout << "pad\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "pad" + PATH_DELIMITER;
	copyFileFromRes_rpc("LANDPAD", path + "img.pcx");
	copyFileFromRes("S_LANDPA", path + "shw.pcx");
	copyFileFromRes("P_LANDPD", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "LP_ISO.FLC", path + "video.pcx");

	//platform
	cout << "platform\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "platform" + PATH_DELIMITER;
	copyFileFromRes_rpc("WTRPLTFM", path + "img.pcx");
	copyFileFromRes("S_WTRPLT", path + "shw.pcx");
	copyFileFromRes("P_WATER", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "WP.FLC", path + "video.pcx");
	
	//radar
	cout << "radar\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "radar" + PATH_DELIMITER;
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
	
	copyFileFromRes("S_RADAR", path + "shw.pcx", 14);
	copyFileFromRes("P_RADAR", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "RADAR.FLC", path + "video.pcx");

	//road
	cout << "road\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "road" + PATH_DELIMITER;
	copyFileFromRes("ROAD", path + "img.pcx");
	copyFileFromRes("S_ROAD", path + "shw.pcx");
	copyFileFromRes("P_ROAD", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "ROAD.FLC", path + "video.pcx");

	//seamine
	cout << "seamine\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "seamine" + PATH_DELIMITER;
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

	//seamines don't have a shadow in the original. So creating a dummy file here
	surface = getImage("SEAMINE");
	output = SDL_CreateRGBSurface(SDL_SWSURFACE, 42,41,8,0,0,0,0);
	output->pitch = output->w;					//workaround for an SDL-Bug
	SDL_SetColors(output, surface->format->palette->colors, 0, 256);
	SDL_FillRect( output, 0, SDL_MapRGB( output->format, 255, 0, 255));
	SDL_FreeSurface( surface );
	savePCX(output, path + "shw.pcx");
	SDL_FreeSurface( output );
	
	copyFileFromRes("P_SEAMIN", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "SMINE01.FLC", path + "video.pcx");

	//shield
	cout << "shield\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "shield" + PATH_DELIMITER;
	copyFileFromRes_rpc("SHIELDGN", path + "img.pcx");
	copyFileFromRes("S_SHIELD", path + "shw.pcx");
	copyFileFromRes("P_SHIELD", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "SHLDGEN.FLC", path + "video.pcx");

	//storage gold
	cout << "storage gold\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "storage_gold" + PATH_DELIMITER;
	copyFileFromRes_rpc("GOLDSM", path + "img.pcx");
	copyFileFromRes("S_GOLDSM", path + "shw.pcx");
	copyFileFromRes("P_SMVLT", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "GOLDSM.FLC", path + "video.pcx");

	//storage metal
	cout << "storage metal\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "storage_metal" + PATH_DELIMITER;
	copyFileFromRes_rpc("ADUMP", path + "img.pcx");
	copyFileFromRes("S_ADUMP", path + "shw.pcx");
	copyFileFromRes("P_SMSTOR", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "SS_ISO.FLC", path + "video.pcx");

	//storage oil
	cout << "storage oil\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "storage_oil" + PATH_DELIMITER;
	copyFileFromRes_rpc("FDUMP", path + "img.pcx");
	copyFileFromRes("S_FDUMP", path + "shw.pcx");
	copyFileFromRes("P_SMFUEL", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "SF_ISO.FLC", path + "video.pcx");

	//training
	cout << "training\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "training" + PATH_DELIMITER;
	copyFileFromRes_rpc("TRAINHAL", path + "img.pcx");
	copyFileFromRes("S_TRAINH", path + "shw.pcx");
	copyFileFromRes("P_TRNHLL", path + "info.pcx");
	copyImageFromFLC( sMAXPath + "THALL.FLC", path + "video.pcx");

	//rubble
	cout << "rubble\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER;
	copyFileFromRes("LRGRUBLE", path + "dirt_big.pcx",  0);
	copyFileFromRes("LRGRUBLE", path + "dirt_big1.pcx", 1);
	copyFileFromRes("SMLRUBLE", path + "dirt_small.pcx",  0);
	copyFileFromRes("SMLRUBLE", path + "dirt_small1.pcx", 1);
	copyFileFromRes("SMLRUBLE", path + "dirt_small2.pcx", 2);
	copyFileFromRes("SMLRUBLE", path + "dirt_small3.pcx", 3);
	copyFileFromRes("SMLRUBLE", path + "dirt_small4.pcx", 4);

	copyFileFromRes("S_LRGRBL", path + "dirt_big_shw.pcx",  1);
	copyFileFromRes("S_LRGRBL", path + "dirt_big_shw1.pcx", 1);
	copyFileFromRes("S_SMLRBL", path + "dirt_small_shw.pcx",  0);
	copyFileFromRes("S_SMLRBL", path + "dirt_small_shw1.pcx", 1);
	copyFileFromRes("S_SMLRBL", path + "dirt_small_shw2.pcx", 2);
	copyFileFromRes("S_SMLRBL", path + "dirt_small_shw3.pcx", 3);
	copyFileFromRes("S_SMLRBL", path + "dirt_small_shw4.pcx", 4);


	return 1;
}

int installVehicleVideos()
{
	string path;

	cout << "Vehicle videos\n";

	//air_transport
	cout << "air_transport\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "air_transport" + PATH_DELIMITER;
	copyFile( sMAXPath + "ATRANS02.FLC", path + "video.flc" );

	//alien_assault
	cout << "alien_assault\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "alien_assault" + PATH_DELIMITER;
	copyFile( sMAXPath + "ALNASG.FLC", path + "video.flc" );
	
	//alien_plane
	cout << "alien_plane\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "alien_plane" + PATH_DELIMITER;
	copyFile( sMAXPath + "ALNPLANE.FLC", path + "video.flc" );
	
	//alien_ship
	cout << "alien_ship\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "alien_ship" + PATH_DELIMITER;
	copyFile( sMAXPath + "JUGGERN.FLC", path + "video.flc" );
	
	//alien_tank
	cout << "alien_tank\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "alien_tank" + PATH_DELIMITER;
	copyFile( sMAXPath + "ALNTANK.FLC", path + "video.flc" );
	
	//apc
	cout << "apc\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "apc" + PATH_DELIMITER;
	copyFile( sMAXPath + "APC_TR01.FLC", path + "video.flc" );
	
	//assault
	cout << "assault\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "assault" + PATH_DELIMITER;
	copyFile( sMAXPath + "E_ART2.FLC", path + "video.flc" );
	
	//awac
	cout << "awac\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "awac" + PATH_DELIMITER;
	copyFile( sMAXPath + "AWACS03.FLC", path + "video.flc" );
	
	//bomber
	cout << "bomber\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "bomber" + PATH_DELIMITER;
	copyFile( sMAXPath + "BOMBER03.FLC", path + "video.flc" );
	
	//bulldozer
	cout << "bulldozer\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "bulldozer" + PATH_DELIMITER;
	copyFile( sMAXPath + "DOZER01.FLC", path + "video.flc" );
	
	//cargoship
	cout << "cargoship\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "cargoship" + PATH_DELIMITER;
	copyFile( sMAXPath + "SCARGO02.FLC", path + "video.flc" );
	
	//cluster
	cout << "cluster\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "cluster" + PATH_DELIMITER;
	copyFile( sMAXPath + "MML.FLC", path + "video.flc" );
	
	//commando
	cout << "commando\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "commando" + PATH_DELIMITER;
	copyFile( sMAXPath + "AGT.FLC", path + "video.flc" );
	
	//corvet
	cout << "corvet\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "corvet" + PATH_DELIMITER;
	copyFile( sMAXPath + "CORVETTE.FLC", path + "video.flc" );
	
	//escort
	cout << "escort\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "escort" + PATH_DELIMITER;
	copyFile( sMAXPath + "ESCORT.FLC", path + "video.flc" );
	
	//fighter
	cout << "fighter\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "fighter" + PATH_DELIMITER;
	copyFile( sMAXPath + "FIGHTER.FLC", path + "video.flc" );
	
	//gunboat
	cout << "gunboat\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "gunboat" + PATH_DELIMITER;
	copyFile( sMAXPath + "HG1.FLC", path + "video.flc" );
	
	//infantery
	cout << "infantery\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "infantery" + PATH_DELIMITER;
	copyFile( sMAXPath + "INFANTRY.FLC", path + "video.flc" );
		
	//konstrukt
	cout << "konstrukt\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "konstrukt" + PATH_DELIMITER;
	copyFile( sMAXPath + "CONSTRCT.FLC", path + "video.flc" );
	
	//minelayer
	cout << "minelayer\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "minelayer" + PATH_DELIMITER;
	copyFile( sMAXPath + "MINELAY.FLC", path + "video.flc" );
	
	//missel
	cout << "missel\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "missel" + PATH_DELIMITER;
	copyFile( sMAXPath + "MISSLE_L.FLC", path + "video.flc" );
	
	//missel_ship
	cout << "missel_ship\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "missel_ship" + PATH_DELIMITER;
	copyFile( sMAXPath + "MB5.FLC", path + "video.flc" );
		
	//mobile_aa
	cout << "mobile_aa\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "mobile_aa" + PATH_DELIMITER;
	copyFile( sMAXPath + "MAA.FLC", path + "video.flc" );
	
	//pionier
	cout << "pionier\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "pionier" + PATH_DELIMITER;
	copyFile( sMAXPath + "ENGINEER.FLC", path + "video.flc" );
	
	//repair
	cout << "repair\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "repair" + PATH_DELIMITER;
	copyFile( sMAXPath + "REPAIR02.FLC", path + "video.flc" );
	
	//scanner
	cout << "scanner\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "scanner" + PATH_DELIMITER;
	copyFile( sMAXPath + "SCANR1.FLC", path + "video.flc" );
	
	//scout
	cout << "scout\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "scout" + PATH_DELIMITER;
	copyFile( sMAXPath + "SCOUT.FLC", path + "video.flc" );
	
	//sea_minelayer
	cout << "sea_minelayer\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "sea_minelayer" + PATH_DELIMITER;
	copyFile( sMAXPath + "SML.FLC", path + "video.flc" );
	
	//sea_transport
	cout << "sea_transport\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "sea_transport" + PATH_DELIMITER;
	copyFile( sMAXPath + "SEATRANS.FLC", path + "video.flc" );
	
	//sub
	cout << "sub\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "sub" + PATH_DELIMITER;
	copyFile( sMAXPath + "SUB.FLC", path + "video.flc" );
	
	//surveyor
	cout << "surveyor\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "surveyor" + PATH_DELIMITER;
	copyFile( sMAXPath + "SURVEYOR.FLC", path + "video.flc" );
	
	//tank
	cout << "tank\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "tank" + PATH_DELIMITER;
	copyFile( sMAXPath + "TANK03.FLC", path + "video.flc" );
	
	//trans_gold
	cout << "trans_gold\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "trans_gold" + PATH_DELIMITER;
	copyFile( sMAXPath + "MCHARGE.FLC", path + "video.flc" );
	
	//trans_metal
	cout << "trans_metal\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "trans_metal" + PATH_DELIMITER;
	copyFile( sMAXPath + "TRUCK.FLC", path + "video.flc" );
	
	//trans_oil
	cout << "trans_oil\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "trans_oil" + PATH_DELIMITER;
	copyFile( sMAXPath + "MGENR.FLC", path + "video.flc" );
	
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
	cout << "Building sounds\n";

	//energy big
	cout << "energy big\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "energy_big" + PATH_DELIMITER;
	copyPartOfWAV( sMAXPath + "POWGN17.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "POWGN17.WAV", path + "running.wav", 1);
	copyFile( sMAXPath + "POWGN18.WAV", path + "stop.wav");
	
	//energy small
	cout << "energy small\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "energy_small" + PATH_DELIMITER;
	copyPartOfWAV( sMAXPath + "POWGN17.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "POWGN17.WAV", path + "running.wav", 1);
	copyFile( sMAXPath + "POWGN18.WAV", path + "stop.wav");
		
	//fac air
	cout << "fac air\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "fac_air" + PATH_DELIMITER;
	copyPartOfWAV( sMAXPath + "AUNIT17.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "AUNIT17.WAV", path + "running.wav", 1);
	copyFile( sMAXPath + "AUNIT18.WAV", path + "stop.wav");
	
	//fac alien
	cout << "fac alien\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "fac_alien" + PATH_DELIMITER;
	copyPartOfWAV( sMAXPath + "LVP17.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "LVP17.WAV", path + "running.wav", 1);
	copyFile( sMAXPath + "LVP18.WAV", path + "stop.wav");
	
	//fac big
	cout << "fac big\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "fac_big" + PATH_DELIMITER;
	copyPartOfWAV( sMAXPath + "HVP17.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "HVP17.WAV", path + "running.wav", 1);
	copyFile( sMAXPath + "HVP18.WAV", path + "stop.wav");
	
	//fac ship
	cout << "fac ship\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "fac_ship" + PATH_DELIMITER;
	copyPartOfWAV( sMAXPath + "HVP17.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "HVP17.WAV", path + "running.wav", 1);
	copyFile( sMAXPath + "HVP18.WAV", path + "stop.wav");
	
	//fac small
	cout << "fac small\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "fac_small" + PATH_DELIMITER;
	copyPartOfWAV( sMAXPath + "LVP17.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "LVP17.WAV", path + "running.wav", 1);
	copyFile( sMAXPath + "LVP18.WAV", path + "stop.wav");
	
	//goldraff
	cout << "goldraff\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "goldraff" + PATH_DELIMITER;
	copyPartOfWAV( sMAXPath + "MONOP17.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "MONOP17.WAV", path + "running.wav", 1);
	copyFile( sMAXPath + "MONOP18.WAV", path + "stop.wav");
	
	//gun aa
	cout << "gun aa\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "gun_aa" + PATH_DELIMITER;
	copyFile( sMAXPath + "FANTY14.WAV", path + "attack.wav");
	
	//gun ari
	cout << "gun ari\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "gun_ari" + PATH_DELIMITER;
	copyFile( sMAXPath + "FARTY14", path + "attack.wav");
	
	//gun missile
	cout << "gun missile\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "gun_missel" + PATH_DELIMITER;
	copyFile( sMAXPath + "MISLFIRE.WAV", path + "attack.wav");

	//gun turret
	cout << "gun turret\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "gun_turret" + PATH_DELIMITER;
	copyFile( sMAXPath + "CANFIRE.WAV", path + "attack.wav");
	
	//landmine
	cout << "landmine\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "landmine" + PATH_DELIMITER;
	copyFile( sMAXPath + "EXPSDIRT.WAV", path + "attack.wav");
	
	//mine
	cout << "mine;\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "mine" + PATH_DELIMITER;
	copyPartOfWAV( sMAXPath + "HVP17.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "HVP17.WAV", path + "running.wav", 1);
	copyFile( sMAXPath + "MSTAT18.WAV", path + "stop.wav");

	//mine deep
	cout << "mine deep\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "mine_deep" + PATH_DELIMITER;
	copyPartOfWAV( sMAXPath + "HVP17.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "HVP17.WAV", path + "running.wav", 1);
	copyFile( sMAXPath + "MSTAT18.WAV", path + "stop.wav");
	
	//radar
	//cout << "radar\n";
	//path = sOutputPath + "buildings" + PATH_DELIMITER + "radar" + PATH_DELIMITER;
	//copyFile( sMAXPath + "RADAR13.WAV", path + "running.wav");
	
	//research
	cout << "research\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "research" + PATH_DELIMITER;
	copyPartOfWAV( sMAXPath + "RESEAR17.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "RESEAR17.WAV", path + "running.wav", 1);
	copyFile( sMAXPath + "RESEAR18.WAV", path + "stop.wav");

	//seamine
	cout << "seamine\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "seamine" + PATH_DELIMITER;
	copyFile( sMAXPath + "EPLOWET1.WAV", path + "attack.wav");
	
	//shield
	//cout << "shield\n";
	//path = sOutputPath + "buildings" + PATH_DELIMITER + "shield" + PATH_DELIMITER;

	//training
	cout << "training\n";
	path = sOutputPath + "buildings" + PATH_DELIMITER + "training" + PATH_DELIMITER;
	copyPartOfWAV( sMAXPath + "LVP17.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "LVP17.WAV", path + "running.wav", 1);
	copyFile( sMAXPath + "LVP18.WAV", path + "stop.wav");


	return 1;
}

int installVehicleSounds()
{
	string path;

	cout << "Vehicle Sounds\n";

	//air_transport
	cout << "air_transport\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "air_transport" + PATH_DELIMITER;
	copyFile( sMAXPath + "ATRANS1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "ATRANS5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "ATRANS5.WAV", path + "drive.wav", 1);
	copyFile( sMAXPath + "ATRANS7.WAV", path + "stop.wav");

	//alien_assault
	cout << "alien_assault\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "alien_assault" + PATH_DELIMITER;
	copyFile( sMAXPath + "ALNTK1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "ALNTK5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "ALNTK5.WAV", path + "drive.wav", 1);
	copyFile( sMAXPath + "ALNTK7.WAV", path + "stop.wav");
	copyFile( sMAXPath + "ASGUN14.WAV", path + "attack.wav");
		
	//alien_plane
	cout << "alien_plane\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "alien_plane" + PATH_DELIMITER;
	copyFile( sMAXPath + "ATTACK1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "ATTACK5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "ATTACK5.WAV", path + "drive.wav", 1);
	copyFile( sMAXPath + "ATTACK7.WAV", path + "stop.wav");
	copyFile( sMAXPath + "MISLFIRE.WAV", path + "attack.wav");
		
	//alien_ship
	cout << "alien_ship\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "alien_ship" + PATH_DELIMITER;
	copyFile( sMAXPath + "JUGGR1.WAV", path + "wait_water.wav");
	copyPartOfWAV( sMAXPath + "JUGGR5.WAV", path + "start_water.wav", 0);
	copyPartOfWAV( sMAXPath + "JUGGR5.WAV", path + "drive_water.wav", 1);
	copyFile( sMAXPath + "JUGGR7.WAV", path + "stop_water.wav");
	copyFile( sMAXPath + "FARTY14.WAV", path + "attack.wav");
		
	//alien_tank
	cout << "alien_tank\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "alien_tank" + PATH_DELIMITER;
	copyFile( sMAXPath + "ALNTK1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "ALNTK5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "ALNTK5.WAV", path + "drive.wav", 1);
	copyFile( sMAXPath + "ALNTK7.WAV", path + "stop.wav");
	copyFile( sMAXPath + "CANFIRE.WAV", path + "attack.wav");
		
	//apc
	cout << "apc\n";
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
	cout << "assault\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "assault" + PATH_DELIMITER;
	copyFile( sMAXPath + "TANKA_1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "TANKA_5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "TANKA_5.WAV", path + "drive.wav", 1);
	copyFile( sMAXPath + "TANKA_7.WAV", path + "stop.wav");
	copyFile( sMAXPath + "ASGUN14.WAV", path + "attack.wav");
		
	//awac
	cout << "awac\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "awac" + PATH_DELIMITER;
	copyFile( sMAXPath + "AWAC1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "AWAC5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "AWAC5.WAV", path + "drive.wav", 1);
	copyFile( sMAXPath + "AWAC7.WAV", path + "stop.wav");
		
	//bomber
	cout << "bomber\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "bomber" + PATH_DELIMITER;
	copyFile( sMAXPath + "ATTACK1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "ATTACK5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "ATTACK5.WAV", path + "drive.wav", 1);
	copyFile( sMAXPath + "ATTACK7.WAV", path + "stop.wav");
	copyFile( sMAXPath + "MISLFIRE.WAV", path + "attack.wav");
		
	//bulldozer
	cout << "bulldozer\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "bulldozer" + PATH_DELIMITER;
	copyFile( sMAXPath + "BULL1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "BULL5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "BULL5.WAV", path + "drive.wav", 1);
	copyFile( sMAXPath + "BULL7.WAV", path + "stop.wav");
	
	//cargoship
	cout << "cargoship\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "cargoship" + PATH_DELIMITER;
	copyFile( sMAXPath + "MBAOTIDL.WAV", path + "wait_water.wav");
	copyPartOfWAV( sMAXPath + "MBOATMVE.WAV", path + "start_water.wav", 0);
	copyPartOfWAV( sMAXPath + "MBOATMVE.WAV", path + "drive_water.wav", 1);
	copyFile( sMAXPath + "MBOATSTP.WAV", path + "stop_water.wav");
		
	//cluster
	cout << "cluster\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "cluster" + PATH_DELIMITER;
	copyFile( sMAXPath + "TANKC_1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "TANKC_5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "TANKC_5.WAV", path + "drive.wav", 1);
	copyFile( sMAXPath + "TANKC_7.WAV", path + "stop.wav");
	copyFile( sMAXPath + "MISLFIRE.WAV", path + "attack.wav");

	//commando
	cout << "commando\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "commando" + PATH_DELIMITER;
	copyFile( sMAXPath + "MANMOVE.WAV", path + "drive.wav");
	copyFile( sMAXPath + "INFIL14.WAV", path + "attack.wav");

	//corvet
	cout << "corvet\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "corvet" + PATH_DELIMITER;
	copyFile( sMAXPath + "SBAOTIDL.WAV", path + "wait_water.wav");
	copyPartOfWAV( sMAXPath + "SBOATMVE.WAV", path + "start_water.wav", 0);
	copyPartOfWAV( sMAXPath + "SBOATMVE.WAV", path + "drive_water.wav", 1);
	copyFile( sMAXPath + "SBOATSTP.WAV", path + "stop_water.wav");
	copyFile( sMAXPath + "CORVT14.WAV", path + "attack.wav");
		
	//escort
	cout << "escort\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "escort" + PATH_DELIMITER;
	copyFile( sMAXPath + "SBAOTIDL.WAV", path + "wait_water.wav");
	copyPartOfWAV( sMAXPath + "SBOATMVE.WAV", path + "start_water.wav", 0);
	copyPartOfWAV( sMAXPath + "SBOATMVE.WAV", path + "drive_water.wav", 1);
	copyFile( sMAXPath + "SBOATSTP.WAV", path + "stop_water.wav");
	copyFile( sMAXPath + "CORVT14.WAV", path + "attack.wav");
		
	//fighter
	cout << "fighter\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "fighter" + PATH_DELIMITER;
	copyFile( sMAXPath + "FIGHT1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "FIGHT5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "FIGHT5.WAV", path + "drive.wav", 1);
	copyFile( sMAXPath + "FIGHT7.WAV", path + "stop.wav");
	copyFile( sMAXPath + "SCOUT14.WAV", path + "attack.wav");
		
	//gunboat
	cout << "gunboat\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "gunboat" + PATH_DELIMITER;
	copyFile( sMAXPath + "GBAOTIDL.WAV", path + "wait_water.wav");
	copyPartOfWAV( sMAXPath + "GBOATMVE.WAV", path + "start_water.wav", 0);
	copyPartOfWAV( sMAXPath + "GBOATMVE.WAV", path + "drive_water.wav", 1);
	copyFile( sMAXPath + "GBOATSTP.WAV", path + "stop_water.wav");
	copyFile( sMAXPath + "FARTY14.WAV", path + "attack.wav");

	//infantery
	cout << "infantery\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "infantery" + PATH_DELIMITER;
	copyFile( sMAXPath + "MANMOVE.WAV", path + "drive.wav");
	copyFile( sMAXPath + "INFAN14.WAV", path + "attack.wav");
			
	//konstrukt
	cout << "konstrukt\n";
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
	cout << "minelayer\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "minelayer" + PATH_DELIMITER;
	copyFile( sMAXPath + "TANKC_1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "TANKC_5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "TANKC_5.WAV", path + "drive.wav", 1);
	copyFile( sMAXPath + "TANKC_7.WAV", path + "stop.wav");
		
	//missel
	cout << "missel\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "missel" + PATH_DELIMITER;
	copyFile( sMAXPath + "TANKC_1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "TANKC_5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "TANKC_5.WAV", path + "drive.wav", 1);
	copyFile( sMAXPath + "TANKC_7.WAV", path + "stop.wav");
	copyFile( sMAXPath + "MISLFIRE.WAV", path + "attack.wav");
		
	//missel_ship
	cout << "missel_ship\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "missel_ship" + PATH_DELIMITER;
	copyFile( sMAXPath + "MBAOTIDL.WAV", path + "wait_water.wav");
	copyPartOfWAV( sMAXPath + "MBOATMVE.WAV", path + "start_water.wav", 0);
	copyPartOfWAV( sMAXPath + "MBOATMVE.WAV", path + "drive_water.wav", 1);
	copyFile( sMAXPath + "MBOATSTP.WAV", path + "stop_water.wav");
	copyFile( sMAXPath + "MISLFIRE.WAV", path + "attack.wav");
	
	//mobile_aa
	cout << "mobile_aa\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "mobile_aa" + PATH_DELIMITER;
	copyFile( sMAXPath + "APC1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "APC5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "APC5.WAV", path + "drive.wav", 1);
	copyFile( sMAXPath + "APC7.WAV", path + "stop.wav");
	copyFile( sMAXPath + "MANTI14", path + "attack.wav");
		
	//pionier
	cout << "pionier\n";
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
	cout << "repair\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "repair" + PATH_DELIMITER;
	copyFile( sMAXPath + "REPAIR1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "REPAIR5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "REPAIR5.WAV", path + "drive.wav", 1);
	copyFile( sMAXPath + "REPAIR7.WAV", path + "stop.wav");
		
	//scanner
	cout << "scanner\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "scanner" + PATH_DELIMITER;
	copyFile( sMAXPath + "SCAN1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "SCAN5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "SCAN5.WAV", path + "drive.wav", 1);
	copyFile( sMAXPath + "SCAN7.WAV", path + "stop.wav");
		
	//scout
	cout << "scout\n";
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
	cout << "sea_minelayer\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "sea_minelayer" + PATH_DELIMITER;
	copyFile( sMAXPath + "GBAOTIDL.WAV", path + "wait_water.wav");
	copyPartOfWAV( sMAXPath + "GBOATMVE.WAV", path + "start_water.wav", 0);
	copyPartOfWAV( sMAXPath + "GBOATMVE.WAV", path + "drive_water.wav", 1);
	copyFile( sMAXPath + "GBOATSTP.WAV", path + "stop_water.wav");
		
	//sea_transport
	cout << "sea_transport\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "sea_transport" + PATH_DELIMITER;
	copyFile( sMAXPath + "GBAOTIDL.WAV", path + "wait_water.wav");
	copyPartOfWAV( sMAXPath + "GBOATMVE.WAV", path + "start_water.wav", 0);
	copyPartOfWAV( sMAXPath + "GBOATMVE.WAV", path + "drive_water.wav", 1);
	copyFile( sMAXPath + "GBOATSTP.WAV", path + "stop_water.wav");
		
	//sub
	cout << "sub\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "sub" + PATH_DELIMITER;
	copyFile( sMAXPath + "SUB2.WAV", path + "wait_water.wav");
	copyPartOfWAV( sMAXPath + "SUB6.WAV", path + "start_water.wav", 0);
	copyPartOfWAV( sMAXPath + "SUB6.WAV", path + "drive_water.wav", 1);
	copyFile( sMAXPath + "SUB8.WAV", path + "stop_water.wav");
	copyFile( sMAXPath + "SUB14.WAV", path + "attack.wav");
		
	//surveyor
	cout << "surveyor\n";
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
	cout << "tank\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "tank" + PATH_DELIMITER;
	copyFile( sMAXPath + "TANK1.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "TANK5.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "TANK5.WAV", path + "drive.wav", 1);
	copyFile( sMAXPath + "TANK7.WAV", path + "stop.wav");
	copyFile( sMAXPath + "CANFIRE.WAV", path + "attack.wav");
		
	//trans_gold
	cout << "trans_gold\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "trans_gold" + PATH_DELIMITER;
	copyFile( sMAXPath + "TRUCKIDL.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "TRUCKMVE.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "TRUCKMVE.WAV", path + "drive.wav", 1);
	copyFile( sMAXPath + "TRUCKSTP.WAV", path + "stop.wav");
		
	//trans_metal
	cout << "trans_metal\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "trans_metal" + PATH_DELIMITER;
	copyFile( sMAXPath + "TRUCKIDL.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "TRUCKMVE.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "TRUCKMVE.WAV", path + "drive.wav", 1);
	copyFile( sMAXPath + "TRUCKSTP.WAV", path + "stop.wav");
		
	//trans_oil
	cout << "trans_oil\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "trans_oil" + PATH_DELIMITER;
	copyFile( sMAXPath + "TRUCKIDL.WAV", path + "wait.wav");
	copyPartOfWAV( sMAXPath + "TRUCKMVE.WAV", path + "start.wav", 0);
	copyPartOfWAV( sMAXPath + "TRUCKMVE.WAV", path + "drive.wav", 1);
	copyFile( sMAXPath + "TRUCKSTP.WAV", path + "stop.wav");
		
	return 1;
}


int main ( int argc, char* argv[] )
{
	while ( 1 )
	{
		cout << "char: " << sizeof( char ) << "\n";
		cout << "short: " << sizeof( short ) << "\n";
		cout << "int: " << sizeof( int ) << "\n";
		cout << "long: " << sizeof( long ) << "\n";
		cout << "Please enter path to MAX-Installation: ";
		cin >> sMAXPath;
		//sMAXPath = "C:\\Dokumente und Einstellungen\\Eiko\\Desktop\\MAX-Develop\\MAX\\"; //temp

		res = SDL_RWFromFile ( (sMAXPath + "MAX.RES").c_str(), "rb" );
		if( !res )
		{
			cout << "Could not open resourcefile\n";
			//exit(1);
		}
		else
		{
			break;
		}
	}

	FILE *pal;
	while ( 1 )
	{
		cout << "\nPlease enter path to palette-file: ";
		cin >> sPalettePath;
		//sPalettePath = "C:\\Dokumente und Einstellungen\\Eiko\\Desktop\\MAX-Develop\\MAXR Install\\Source\\palette.pal";

		
		if( ( pal = fopen ( sPalettePath.c_str(), "rb" ) ) == NULL )
		{
			cout << "Could not open palette\n";
		}
		else
		{
			break;
		}
	}

	fseek( pal, 0, SEEK_SET );
	orig_palette = ( unsigned char* ) malloc ( sizeof( unsigned char ) * 768 );
	fread ( orig_palette, sizeof( unsigned char ), 786, pal );
	fclose ( pal );


	cout << "\nPlease enter path to ouputfolder: \n";
	cin >> sOutputPath;
	//sOutputPath = "C:\\Dokumente und Einstellungen\\Eiko\\Desktop\\MAX-Develop\\MAXR Install\\output - install skript\\";



	SDL_RWseek( res, 0, SEEK_END );
	lEndOfFile = SDL_RWtell (res);

	
	lPosBegin = 15000000;		//the '[EOD]' should be after this position 
								//for all versions of max.res, I think 
								//--eiko

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
	

	free (orig_palette);
	SDL_RWclose(res);
	return 0;
}
