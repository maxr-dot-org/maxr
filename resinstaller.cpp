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


int installVehicleGraphics()
{
	string path;
	SDL_Rect src_rect;
	char szNum[13];
	SDL_Surface *surface, *output;

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

	//assault
	cout << "assault\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "assault" + PATH_DELIMITER;
	src_rect.h = 64;
	src_rect.w = 64;
	src_rect.x = 23;
	src_rect.y = 24;
	for (int i = 0; i < 8; i++)
	{
		sprintf( szNum, "%d", i);
		surface = getImage("ARTILLRY", i);
		removePlayerColor( surface );
		output = SDL_CreateRGBSurface(SDL_SWSURFACE, 64,64,8,0,0,0,0);
		SDL_SetColors(output, surface->format->palette->colors, 0, 256);
		SDL_FillRect( output, 0, SDL_MapRGB( output->format, 255, 0, 255));
		SDL_BlitSurface(surface, &src_rect, output, 0);
		SDL_FreeSurface( surface );
		save_PCX(output, path + "img" + szNum + ".pcx");
		SDL_FreeSurface( output );
		
		copyFileFromRes("S_ARTILL", path + "shw" + szNum + ".pcx", i );
	}
	copyFileFromRes("A_ARTY", path + "store.pcx");

	//awac
	cout << "awac\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "awac" + PATH_DELIMITER;
	copyFileFromRes("A_AWAC", path + "store.pcx");

	//bomber
	cout << "bomber\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "bomber" + PATH_DELIMITER;
	copyFileFromRes("A_BOMBER", path + "store.pcx");

	//bulldozer
	cout << "bulldozer\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "bulldozer" + PATH_DELIMITER;
	copyFileFromRes("A_BULLDZ", path + "store.pcx");

	//cargoship
	cout << "cargoship\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "cargoship" + PATH_DELIMITER;
	copyFileFromRes("A_CARGOS", path + "store.pcx");

	//cluster
	cout << "cluster\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "cluster" + PATH_DELIMITER;
	copyFileFromRes("A_ROCKET", path + "store.pcx");

	//commando
	cout << "commando\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "commando" + PATH_DELIMITER;
	copyFileFromRes("A_COMMAN", path + "store.pcx");

	//corvet
	cout << "corvet\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "corvet" + PATH_DELIMITER;
	copyFileFromRes("A_CORVET", path + "store.pcx");

	//escort
	cout << "escort\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "escort" + PATH_DELIMITER;
	copyFileFromRes("A_ESCORT", path + "store.pcx");

	//fighter
	cout << "fighter\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "fighter" + PATH_DELIMITER;
	copyFileFromRes("A_FIGHTR", path + "store.pcx");

	//gunboat
	cout << "gunboat\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "gunboat" + PATH_DELIMITER;
	copyFileFromRes("A_GUNBT", path + "store.pcx");

	//infantery
	cout << "infantery\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "infantery" + PATH_DELIMITER;
	copyFileFromRes("A_INFANT", path + "store.pcx");

	//konstrukt
	cout << "konstrukt\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "konstrukt" + PATH_DELIMITER;
	copyFileFromRes("A_CONTRC", path + "store.pcx");

	//minelayer
	cout << "minelayer\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "minelayer" + PATH_DELIMITER;
	copyFileFromRes("A_MNELAY", path + "store.pcx");

	//missel
	cout << "missel\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "missel" + PATH_DELIMITER;
	copyFileFromRes("A_MISSIL", path + "store.pcx");

	//missel_ship
	cout << "missel_ship\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "missel_ship" + PATH_DELIMITER;
	copyFileFromRes("A_MSLCR", path + "store.pcx");

	//mobile_aa
	cout << "mobile_aa\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "mobile_aa" + PATH_DELIMITER;
	copyFileFromRes("A_AA", path + "store.pcx");

	//pionier
	cout << "pionier\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "pionier" + PATH_DELIMITER;
	copyFileFromRes("A_ENGINR", path + "store.pcx");

	//repair
	cout << "repair\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "repair" + PATH_DELIMITER;
	copyFileFromRes("A_REPAIR", path + "store.pcx");

	//scanner
	cout << "scanner\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "scanner" + PATH_DELIMITER;
	copyFileFromRes("A_SCANNR", path + "store.pcx");

	//scout
	cout << "scout\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "scout" + PATH_DELIMITER;
	copyFileFromRes("A_SCOUT", path + "store.pcx");

	//sea_minelayer
	cout << "sea_minelayer\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "sea_minelayer" + PATH_DELIMITER;
	copyFileFromRes("A_SEAMNL", path + "store.pcx");

	//sea_transport
	cout << "sea_transport\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "sea_transport" + PATH_DELIMITER;
	copyFileFromRes("A_SEATRN", path + "store.pcx");

	//sub
	cout << "sub\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "sub" + PATH_DELIMITER;
	copyFileFromRes("A_SUB", path + "store.pcx");

	//surveyor
	cout << "surveyor\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "surveyor" + PATH_DELIMITER;
	copyFileFromRes("A_SURVEY", path + "store.pcx");

	//tank
	cout << "tank\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "tank" + PATH_DELIMITER;
	copyFileFromRes("A_TANK", path + "store.pcx");

	//trans_gold
	cout << "trans_gold\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "trans_gold" + PATH_DELIMITER;
	copyFileFromRes("A_GOLDTR", path + "store.pcx");
	
	//trans_metal
	cout << "trans_metal\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "trans_metal" + PATH_DELIMITER;
	copyFileFromRes("A_SPLYTR", path + "store.pcx");
	
	//trans_oil
	cout << "trans_oil\n";
	path = sOutputPath + "vehicles" + PATH_DELIMITER + "trans_oil" + PATH_DELIMITER;
	copyFileFromRes("A_FUELTR", path + "store.pcx");

	return 1;
}

int installBuildingGraphics()
{
	string path;
	SDL_Surface* surface;
	SDL_Surface* output;
	SDL_Rect src_rect, dst_rect;

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

	save_PCX( output, path + "img.pcx");
	SDL_FreeSurface( output );
	save_PCX( output_s, path + "shw.pcx");
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
	
	save_PCX( output, path + "img.pcx");
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
	
	save_PCX( output, path + "img.pcx");
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
	
	save_PCX( output, path + "img.pcx");
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
	
	save_PCX( output, path + "img.pcx");
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
	save_PCX(output, path + "img.pcx");
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
	save_PCX(output, path + "shw.pcx");
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
	save_PCX(output, path + "img.pcx");
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
	save_PCX(output, path + "img.pcx");
	SDL_FreeSurface( output );

	//seamines don't have a shadow in the original. So creating a dummy file here
	surface = getImage("SEAMINE");
	output = SDL_CreateRGBSurface(SDL_SWSURFACE, 42,41,8,0,0,0,0);
	output->pitch = output->w;					//workaround for an SDL-Bug
	SDL_SetColors(output, surface->format->palette->colors, 0, 256);
	SDL_FillRect( output, 0, SDL_MapRGB( output->format, 255, 0, 255));
	SDL_FreeSurface( surface );
	save_PCX(output, path + "shw.pcx");
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


	return 1;
}

int main ( int argc, char* argv[] )
{
	while ( 1 )
	{
		cout << "Please enter path to MAX-Installation: ";
		cin >> sMAXPath;
		//sMAXPath = "C:\\Dokumente und Einstellungen\\Eiko\\Desktop\\MAX-Develop\\MAX" + PATH_DELIMITER; //temp

		res = fopen ( (sMAXPath + "max.res").c_str(), "rb" );
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
		//sPalettePath = "C:\\Dokumente und Einstellungen\\Eiko\\Desktop\\MAX-Develop\\MAXR Install\\palette.pal";

		
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
	
	//sOutputPath += PATH_DELIMITER;
	//sOutputPath = "C:\\Dokumente und Einstellungen\\Eiko\\Desktop\\MAX-Develop\\MAXR Install\\output - install skript" + PATH_DELIMITER;



	fseek( res, 0, SEEK_END );
	lEndOfFile = ftell (res);

	
	lPosBegin = 15000000;		//the '[EOD]' should be after this position 
								//for all versions of max.res, I think 
								//--eiko

	//a little state maschine for searching the string "[EOD]" in max.res
	unsigned char temp, state = 0;
	fseek( res, lPosBegin, SEEK_SET);

	while ( lPosBegin < lEndOfFile )
	{
		fread( &temp, 1, 1, res );
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
		return -1;
	}
	
	installVehicleGraphics();
	installBuildingGraphics();

	
	free (orig_palette);
	fclose(res);
	return 0;
}
