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

#include "resinstaller.h"

#include "converter.h"
#include "defines.h"
#include "file.h"
#include "ogg_encode.h"
#include "pcx.h"
#include "wave.h"

#include <SDL.h>
#include <algorithm> // for transform toupper
#include <filesystem>
#include <iostream>
#include <string>
#include <vector> // for vectorLanguages

#if MAC
# include "mac/sources/resinstallerGUI.h"
#elif WIN32
# include <Shlobj.h>
# include <conio.h> // for getch
#endif

static std::filesystem::path sMAXPath;
static std::filesystem::path sVoicePath;
std::filesystem::path sOutputPath;
static std::string sLanguage;
static std::string sResChoice;
static std::string waveExtension;
SDL_RWops* res = nullptr;
SDL_RWops* logFile = nullptr;

Uint32 lPosBegin = 0;
Uint32 lEndOfFile = 0;

int iErrors = 0;
int iInstalledFiles = 0;
int iTotalFiles = 0;
bool wasError = false;

bool oggEncode = false;

//-------------------------------------------------------------
int installMVEs()
{
	iTotalFiles = 3;
	iErrors = 0;
	iInstalledFiles = 0;

#if MAC
	updateProgressWindow ("Movie files", iTotalFiles, iInstalledFiles);
#endif

	std::cout << "========================================================================\n";
	std::cout << "MVE files\n";

	const auto path = sOutputPath / "mve";
	copyFile (sVoicePath / "MAXINT.MVE", path / "MAXINT.MVE");
	copyFile (sMAXPath / "MAXMVE1.MVE", path / "MAXMVE1.MVE");
	copyFile (sMAXPath / "MAXMVE2.MVE", path / "MAXMVE2.MVE");

	if (logFile != nullptr)
	{
		writeLog (std::string ("MVEs") + TEXT_FILE_LF);
		writeLog (std::to_string (iErrors) + " errors" + TEXT_FILE_LF);
		writeLog (std::string ("========================================================================") + TEXT_FILE_LF);
	}

	std::cout << "\n";
	std::cout << std::to_string (iErrors) << " errors\n";
	return 1;
}

//-------------------------------------------------------------
int installVehicleGraphics()
{
	iTotalFiles = 824;
	iErrors = 0;
	iInstalledFiles = 0;

	SDL_Rect src_rect, dst_rect;
	SDL_Surface *surface, *output;

#if MAC
	updateProgressWindow ("Vehicle graphics", iTotalFiles, iInstalledFiles);
#endif

	std::cout << "========================================================================\n";
	std::cout << "Vehicle graphics\n";

	// air_transport
	auto path = sOutputPath / "vehicles" / "air_transport";
	for (int i = 0; i < 8; i++)
	{
		auto szNum = std::to_string (i);
		copyFileFromRes_rpc ("AIRTRANS", path / ("img" + szNum + ".pcx"), i);
		copyFileFromRes ("S_AIRTRA", path / ("shw" + szNum + ".pcx"), i);
	}
	copyFileFromRes ("A_AIRTRN", path / "store.pcx");
	copyFileFromRes ("P_AIRTRN", path / "info.pcx");

	// alien_assault
	path = sOutputPath / "vehicles" / "alien_assault";
	for (int i = 0; i < 8; i++)
	{
		auto szNum = std::to_string (i);
		copyFileFromRes_rpc ("ALNASGUN", path / ("img" + szNum + ".pcx"), i);
		copyFileFromRes ("S_ALNASG", path / ("shw" + szNum + ".pcx"), i);
	}
	copyFileFromRes ("A_ALNASG", path / "store.pcx");
	copyFileFromRes ("P_ALNASG", path / "info.pcx");

	// alien_plane
	path = sOutputPath / "vehicles" / "alien_plane";
	for (int i = 0; i < 8; i++)
	{
		auto szNum = std::to_string (i);
		copyFileFromRes_rpc ("ALNPLANE", path / ("img" + szNum + ".pcx"), i);
		copyFileFromRes ("S_ALNPLA", path / ("shw" + szNum + ".pcx"), i);
	}
	copyFileFromRes ("A_ALNPLA", path / "store.pcx");
	copyFileFromRes ("P_ALNPLA", path / "info.pcx");

	// alien_ship
	path = sOutputPath / "vehicles" / "alien_ship";
	for (int i = 0; i < 8; i++)
	{
		auto szNum = std::to_string (i);
		copyFileFromRes_rpc ("JUGGRNT", path / ("img" + szNum + ".pcx"), i);
		copyFileFromRes ("S_JUGGRN", path / ("shw" + szNum + ".pcx"), i);
	}
	copyFileFromRes ("A_JUGGER", path / "store.pcx");
	copyFileFromRes ("P_JUGGER", path / "info.pcx");

	// alien_tank
	path = sOutputPath / "vehicles" / "alien_tank";
	for (int i = 0; i < 8; i++)
	{
		auto szNum = std::to_string (i);
		copyFileFromRes_rpc ("ALNTANK", path / ("img" + szNum + ".pcx"), i);
		copyFileFromRes ("S_ALNTAN", path / ("shw" + szNum + ".pcx"), i);
	}
	copyFileFromRes ("A_ALNTAN", path / "store.pcx");
	copyFileFromRes ("P_ALNTAN", path / "info.pcx");

	// apc
	path = sOutputPath / "vehicles" / "apc";
	for (int i = 0; i < 8; i++)
	{
		auto szNum = std::to_string (i);
		copyFileFromRes_rpc ("CLNTRANS", path / ("img" + szNum + ".pcx"), i + 8);
		try
		{
			surface = getImageFromRes ("S_CLNTRA", i + 8);
			resizeSurface (surface, 5, 5, 65, 75);
			savePCX (surface, path / ("shw" + szNum + ".pcx"));
			SDL_FreeSurface (surface);
		}
		END_INSTALL_FILE (path / ("shw" + szNum + ".pcx"))
	}
	for (int i = 0; i < 8; i++)
	{
		auto szNum = std::to_string (i);
		copyFileFromRes_rpc ("CLNTRANS", path / ("img_stealth" + szNum + ".pcx"), i);
	}
	copyFileFromRes ("A_COLNST", path / "store.pcx");
	copyFileFromRes ("P_COLNST", path / "info.pcx");

	// assault
	path = sOutputPath / "vehicles" / "assault";
	for (int i = 0; i < 8; i++)
	{
		auto szNum = std::to_string (i);
		try
		{
			surface = getImageFromRes ("ARTILLRY", i);
			removePlayerColor (surface);
			resizeSurface (surface, 23, 24, 64, 64);
			savePCX (surface, path / ("img" + szNum + ".pcx"));
			SDL_FreeSurface (surface);
		}
		END_INSTALL_FILE (path / ("img" + szNum + ".pcx"));

		copyFileFromRes ("S_ARTILL", path / ("shw" + szNum + ".pcx"), i);
	}
	copyFileFromRes ("A_ARTY", path / "store.pcx");
	copyFileFromRes ("P_ARTY", path / "info.pcx");

	// awac
	path = sOutputPath / "vehicles" / "awac";
	for (int i = 0; i < 8; i++)
	{
		auto szNum = std::to_string (i);
		copyFileFromRes_rpc ("AWAC", path / ("img" + szNum + ".pcx"), i);
		copyFileFromRes ("S_AWAC", path / ("shw" + szNum + ".pcx"), i);
	}

	try
	{
		output = getImageFromRes ("AWAC", 8);
		resizeSurface (output, 16, 14, 32, 32);
		resizeSurface (output, 0, 0, 256, 32);
		dst_rect.y = 0;
		dst_rect.x = 32;
		src_rect.x = 15;
		src_rect.y = 15;
		src_rect.h = 32;
		src_rect.w = 32;
		for (int i = 1; i < 8; i++)
		{
			surface = getImageFromRes ("AWAC", i * 4 + 8);
			SDL_BlitSurface (surface, &src_rect, output, &dst_rect);
			SDL_FreeSurface (surface);
			dst_rect.x += 32;
		}

		savePCX (output, path / "overlay.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "overlay.pcx")

	copyFileFromRes ("A_AWAC", path / "store.pcx");
	copyFileFromRes ("P_AWAC", path / "info.pcx");

	// bomber
	path = sOutputPath / "vehicles" / "bomber";
	for (int i = 0; i < 8; i++)
	{
		auto szNum = std::to_string (i);
		try
		{
			surface = getImageFromRes ("BOMBER", i);
			removePlayerColor (surface);
			resizeSurface (surface, 14, 14, 64, 64);
			savePCX (surface, path / ("img" + szNum + ".pcx"));
			SDL_FreeSurface (surface);
		}
		END_INSTALL_FILE (path / ("img" + szNum + ".pcx"))

		copyFileFromRes ("S_BOMBER", path / ("shw" + szNum + ".pcx"), i);
	}
	copyFileFromRes ("A_BOMBER", path / "store.pcx");
	copyFileFromRes ("P_BOMBER", path / "info.pcx");

	// bulldozer
	path = sOutputPath / "vehicles" / "bulldozer";
	for (int i = 0; i < 8; i++)
	{
		auto szNum = std::to_string (i);
		try
		{
			surface = getImageFromRes ("BULLDOZR", i);
			removePlayerColor (surface);
			resizeSurface (surface, 4, 4, 64, 64);
			savePCX (surface, path / ("img" + szNum + ".pcx"));
			SDL_FreeSurface (surface);
		}
		END_INSTALL_FILE (path / ("img" + szNum + ".pcx"))

		try
		{
			surface = getImageFromRes ("S_BULLDO", i);
			resizeSurface (surface, 4, 4, 68, 68);
			savePCX (surface, path / ("shw" + szNum + ".pcx"));
			SDL_FreeSurface (surface);
		}
		END_INSTALL_FILE (path / ("shw" + szNum + ".pcx"))
	}
	copyFileFromRes ("A_BULLDZ", path / "store.pcx");
	copyFileFromRes ("P_BULLDZ", path / "info.pcx");

	try
	{
		surface = getImageFromRes ("LRGTAPE", 0);
		output = SDL_CreateRGBSurface (SDL_SWSURFACE, 512, 128, 8, 0, 0, 0, 0);
		SDL_SetPaletteColors (output->format->palette, surface->format->palette->colors, 0, 256);
		SDL_FillRect (output, nullptr, SDL_MapRGB (output->format, 255, 0, 255));

		dst_rect.x = 0;
		dst_rect.y = 0;
		for (int i = 0; i < 4; i++)
		{
			SDL_BlitSurface (surface, 0, output, &dst_rect);
			dst_rect.x += 128;
		}
		SDL_FreeSurface (surface);

		surface = getImageFromRes ("LRGCONES");
		dst_rect.y = 5;
		dst_rect.x = 5;
		for (int i = 0; i < 4; i++)
		{
			SDL_BlitSurface (surface, 0, output, &dst_rect);
			dst_rect.x += 128;
		}
		SDL_FreeSurface (surface);

		surface = getImageFromRes ("BULLDOZR", 8);
		removePlayerColor (surface);
		dst_rect.x = 36;
		dst_rect.y = 36;
		SDL_BlitSurface (surface, 0, output, &dst_rect);

		generateAnimationFrame (surface, 1);
		dst_rect.x = 164;
		SDL_BlitSurface (surface, 0, output, &dst_rect);

		generateAnimationFrame (surface, 2);
		dst_rect.x = 292;
		SDL_BlitSurface (surface, 0, output, &dst_rect);

		generateAnimationFrame (surface, 3);
		dst_rect.x = 420;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		SDL_FreeSurface (surface);
		savePCX (output, path / "clear_big.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "clear_big.pcx")

	try
	{
		output = getImageFromRes ("S_LRGCON");
		resizeSurface (output, 6, 6, 128, 128);
		surface = getImageFromRes ("S_BULLDO");
		dst_rect.x = 38;
		dst_rect.y = 37;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		SDL_FreeSurface (surface);
		savePCX (output, path / "clear_big_shw.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "clear_big_shw.pcx")

	try
	{
		surface = getImageFromRes ("SMLTAPE");
		output = SDL_CreateRGBSurface (SDL_SWSURFACE, 256, 64, 8, 0, 0, 0, 0);
		SDL_SetPaletteColors (output->format->palette, surface->format->palette->colors, 0, 256);
		SDL_FillRect (output, nullptr, SDL_MapRGB (output->format, 255, 0, 255));

		dst_rect.x = 0;
		dst_rect.y = 0;
		for (int i = 0; i < 4; i++)
		{
			SDL_BlitSurface (surface, 0, output, &dst_rect);
			dst_rect.x += 64;
		}
		SDL_FreeSurface (surface);

		surface = getImageFromRes ("SMLCONES");
		dst_rect.y = 5;
		dst_rect.x = 5;
		for (int i = 0; i < 4; i++)
		{
			SDL_BlitSurface (surface, 0, output, &dst_rect);
			dst_rect.x += 64;
		}
		SDL_FreeSurface (surface);

		surface = getImageFromRes ("BULLDOZR", 8);
		removePlayerColor (surface);
		dst_rect.x = 4;
		dst_rect.y = 4;
		SDL_BlitSurface (surface, 0, output, &dst_rect);

		generateAnimationFrame (surface, 1);
		dst_rect.x += 64;
		SDL_BlitSurface (surface, 0, output, &dst_rect);

		generateAnimationFrame (surface, 2);
		dst_rect.x += 64;
		SDL_BlitSurface (surface, 0, output, &dst_rect);

		generateAnimationFrame (surface, 3);
		dst_rect.x += 64;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		SDL_FreeSurface (surface);
		savePCX (output, path / "clear_small.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "clear_small.pcx")

	try
	{
		output = getImageFromRes ("S_SMLCON");
		resizeSurface (output, 6, 6, 64, 66);
		surface = getImageFromRes ("S_BULLDO");
		dst_rect.x = 6;
		dst_rect.y = 5;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		SDL_FreeSurface (surface);

		savePCX (output, path / "clear_small_shw.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "clear_small_shw.pcx")

	// cargoship
	path = sOutputPath / "vehicles" / "cargoship";
	for (int i = 0; i < 8; i++)
	{
		auto szNum = std::to_string (i);
		copyFileFromRes_rpc ("CARGOSHP", path / ("img" + szNum + ".pcx"), i);
		copyFileFromRes ("S_CARGOS", path / ("shw" + szNum + ".pcx"), i);
	}
	copyFileFromRes ("A_CARGOS", path / "store.pcx");
	copyFileFromRes ("P_CARGOS", path / "info.pcx");

	// cluster
	path = sOutputPath / "vehicles" / "cluster";
	for (int i = 0; i < 8; i++)
	{
		auto szNum = std::to_string (i);
		try
		{
			surface = getImageFromRes ("ROCKTLCH", i);
			removePlayerColor (surface);
			resizeSurface (surface, 15, 15, 64, 64);
			savePCX (surface, path / ("img" + szNum + ".pcx"));
			SDL_FreeSurface (surface);
		}
		END_INSTALL_FILE (path / ("img" + szNum + ".pcx"))

		copyFileFromRes ("S_ROCKTL", path / ("shw" + szNum + ".pcx"), i);
	}
	copyFileFromRes ("A_ROCKET", path / "store.pcx");
	copyFileFromRes ("P_ROCKET", path / "info.pcx");

	// commando
	path = sOutputPath / "vehicles" / "commando";
	for (int i = 0; i < 8; i++)
	{
		auto szNum1 = std::to_string (i);
		for (int n = 0; n < 13; n++)
		{
			char szNum2[13];
			sprintf (szNum2, "%.2d", n);
			try
			{
				surface = getImageFromRes ("COMMANDO", n * 8 + i);
				removePlayerColor (surface);
				resizeSurface (surface, 73, 73, 64, 64);
				savePCX (surface, path / ("img" + szNum1 + "_" + szNum2 + ".pcx"));
				SDL_FreeSurface (surface);
			}
			END_INSTALL_FILE (path / ("img" + szNum1 + "_" + szNum2 + ".pcx"))
		}
	}
	for (int dir = 0; dir < 8; dir++)
	{
		auto szNum1 = std::to_string (dir);
		try
		{
			output = SDL_CreateRGBSurface (SDL_SWSURFACE, 256, 64, 32, 0, 0, 0, 0);
			//SDL_SetPaletteColors(output->format->palette, surface->format->palette->colors, 0, 256);
			SDL_FillRect (output, nullptr, SDL_MapRGB (output->format, 255, 0, 255));

			SDL_Rect dest = {0, 0, 0, 0};
			for (int frame = 0; frame < 4; frame++)
			{
				surface = getImageFromRes ("COMMANDO", dir + 200);
				for (int i = 0; i < 256; i++)
				{
					if (i < 7 || i > 24)
					{
						// remove all non animated colors
						setColor (surface, i, 255, 0, 255);
					}
				}
				generateAnimationFrame (surface, frame);
				resizeSurface (surface, 73, 73, 64, 64);

				SDL_BlitSurface (surface, 0, output, &dest);
				SDL_FreeSurface (surface);
				dest.x += 64;
			}

			savePCX (output, path / ("overlay_" + szNum1 + ".pcx"));
			SDL_FreeSurface (output);
		}
		END_INSTALL_FILE (path / ("overlay_" + szNum1 + ".pcx"))
	}
	copyFileFromRes ("A_COMMAN", path / "store.pcx");
	copyFileFromRes ("P_COMMAN", path / "info.pcx");

	// corvet
	path = sOutputPath / "vehicles" / "corvet";
	for (int i = 0; i < 8; i++)
	{
		auto szNum = std::to_string (i);
		copyFileFromRes_rpc ("CORVETTE", path / ("img" + szNum + ".pcx"), i);
		try
		{
			surface = getImageFromRes ("S_CORVET", i);
			resizeSurface (surface, 5, 5, 64, 64);
			savePCX (surface, path / ("shw" + szNum + ".pcx"));
			SDL_FreeSurface (surface);
		}
		END_INSTALL_FILE (path / ("shw" + szNum + ".pcx"));
	}
	copyFileFromRes ("A_CORVET", path / "store.pcx");
	copyFileFromRes ("P_CORVET", path / "info.pcx");

	// escort
	path = sOutputPath / "vehicles" / "escort";
	for (int i = 0; i < 8; i++)
	{
		auto szNum = std::to_string (i);
		copyFileFromRes_rpc ("FASTBOAT", path / ("img" + szNum + ".pcx"), i);
		try
		{
			surface = getImageFromRes ("S_FASTBO", i);
			resizeSurface (surface, 3, 6, 64, 64);
			savePCX (surface, path / ("shw" + szNum + ".pcx"));
			SDL_FreeSurface (surface);
		}
		END_INSTALL_FILE (path / ("shw" + szNum + ".pcx"));
	}
	copyFileFromRes ("A_ESCORT", path / "store.pcx");
	copyFileFromRes ("P_ESCORT", path / "info.pcx");

	// fighter
	path = sOutputPath / "vehicles" / "fighter";
	for (int i = 0; i < 8; i++)
	{
		auto szNum = std::to_string (i);
		try
		{
			surface = getImageFromRes ("FIGHTER", i);
			removePlayerColor (surface);
			resizeSurface (surface, 17, 16, 64, 64);
			savePCX (surface, path / ("img" + szNum + ".pcx"));
			SDL_FreeSurface (surface);
		}
		END_INSTALL_FILE (path / ("img" + szNum + ".pcx"))

		copyFileFromRes ("S_FIGHTE", path / ("shw" + szNum + ".pcx"), i);
	}
	copyFileFromRes ("A_FIGHTR", path / "store.pcx");
	copyFileFromRes ("P_FIGHTR", path / "info.pcx");

	// gunboat
	path = sOutputPath / "vehicles" / "gunboat";
	try
	{
		src_rect.h = 40;
		src_rect.w = 40;
		src_rect.x = 18;
		src_rect.y = 23;
		output = getImageFromRes ("BATTLSHP", 0);
		removePlayerColor (output);
		resizeSurface (output, 4, 11, 64, 64);
		dst_rect.y = 3;
		dst_rect.x = 14;
		surface = getImageFromRes ("BATTLSHP", 8);
		SDL_BlitSurface (surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface (surface);
		savePCX (output, path / "img0.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "img0.pcx")

	try
	{
		output = getImageFromRes ("BATTLSHP", 1);
		removePlayerColor (output);
		resizeSurface (output, 4, 11, 64, 64);
		dst_rect.y = 4;
		dst_rect.x = 22;
		surface = getImageFromRes ("BATTLSHP", 9);
		SDL_BlitSurface (surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface (surface);
		savePCX (output, path / "img1.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "img1.pcx")

	try
	{
		output = getImageFromRes ("BATTLSHP", 2);
		removePlayerColor (output);
		resizeSurface (output, 4, 11, 64, 64);
		dst_rect.y = 12;
		dst_rect.x = 23;
		surface = getImageFromRes ("BATTLSHP", 10);
		SDL_BlitSurface (surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface (surface);
		savePCX (output, path / "img2.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "img2.pcx")

	try
	{
		output = getImageFromRes ("BATTLSHP", 3);
		removePlayerColor (output);
		resizeSurface (output, 4, 11, 64, 64);
		dst_rect.y = 20;
		dst_rect.x = 22;
		surface = getImageFromRes ("BATTLSHP", 11);
		SDL_BlitSurface (surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface (surface);
		savePCX (output, path / "img3.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "img3.pcx")

	try
	{
		output = getImageFromRes ("BATTLSHP", 4);
		removePlayerColor (output);
		resizeSurface (output, 4, 11, 64, 64);
		dst_rect.y = 21;
		dst_rect.x = 14;
		surface = getImageFromRes ("BATTLSHP", 12);
		SDL_BlitSurface (surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface (surface);
		savePCX (output, path / "img4.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "img4.pcx")

	try
	{
		output = getImageFromRes ("BATTLSHP", 5);
		removePlayerColor (output);
		resizeSurface (output, 4, 11, 64, 64);
		dst_rect.y = 20;
		dst_rect.x = 6;
		surface = getImageFromRes ("BATTLSHP", 13);
		SDL_BlitSurface (surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface (surface);
		savePCX (output, path / "img5.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "img5.pcx")

	try
	{
		output = getImageFromRes ("BATTLSHP", 6);
		removePlayerColor (output);
		resizeSurface (output, 4, 11, 64, 64);
		dst_rect.y = 12;
		dst_rect.x = 5;
		surface = getImageFromRes ("BATTLSHP", 14);
		SDL_BlitSurface (surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface (surface);
		savePCX (output, path / "img6.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "img6.pcx")

	try
	{
		output = getImageFromRes ("BATTLSHP", 7);
		removePlayerColor (output);
		resizeSurface (output, 4, 11, 64, 64);
		dst_rect.y = 4;
		dst_rect.x = 6;
		surface = getImageFromRes ("BATTLSHP", 15);
		SDL_BlitSurface (surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface (surface);
		savePCX (output, path / "img7.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "img7.pcx")

	for (int i = 0; i < 8; i++)
	{
		auto szNum = std::to_string (i);
		copyFileFromRes ("S_BATTLS", path / ("shw" + szNum + ".pcx"), i);
	}
	copyFileFromRes ("A_GUNBT", path / "store.pcx");
	copyFileFromRes ("P_GUNBT", path / "info.pcx");

	// infantery
	path = sOutputPath / "vehicles" / "infantery";
	for (int i = 0; i < 8; i++)
	{
		const auto szNum1 = std::to_string (i);
		for (int n = 0; n < 13; n++)
		{
			char szNum2[13];
			sprintf (szNum2, "%.2d", n);
			try
			{
				surface = getImageFromRes ("INFANTRY", n * 8 + i);
				removePlayerColor (surface);
				resizeSurface (surface, 73, 73, 64, 64);
				savePCX (surface, path / ("img" + szNum1 + "_" + szNum2 + ".pcx"));
				SDL_FreeSurface (surface);
			}
			END_INSTALL_FILE (path / ("img" + szNum1 + "_" + szNum2 + ".pcx"))
		}
	}
	copyFileFromRes ("A_INFANT", path / "store.pcx");
	copyFileFromRes ("P_INFANT", path / "info.pcx");

	// konstrukt
	path = sOutputPath / "vehicles" / "konstrukt";
	for (int i = 0; i < 8; i++)
	{
		auto szNum = std::to_string (i);
		copyFileFromRes_rpc ("CONSTRCT", path / ("img" + szNum + ".pcx"), i);
		try
		{
			surface = getImageFromRes ("S_CONSTR", i);
			resizeSurface (surface, 3, 3, 67, 73);
			savePCX (surface, path / ("shw" + szNum + ".pcx"));
			SDL_FreeSurface (surface);
		}
		END_INSTALL_FILE (path / ("shw" + szNum + ".pcx"))
		// copyFileFromRes("S_CONSTR", path / ("shw" + szNum + ".pcx"), i);
	}
	copyFileFromRes ("A_CONTRC", path / "store.pcx");
	copyFileFromRes ("P_CONTRC", path / "info.pcx");

	try
	{
		surface = getImageFromRes ("LRGTAPE", 0);
		output = SDL_CreateRGBSurface (SDL_SWSURFACE, 512, 128, 8, 0, 0, 0, 0);
		SDL_SetPaletteColors (output->format->palette, surface->format->palette->colors, 0, 256);
		SDL_FillRect (output, nullptr, SDL_MapRGB (output->format, 255, 0, 255));

		dst_rect.x = 0;
		dst_rect.y = 0;
		for (int i = 0; i < 4; i++)
		{
			SDL_BlitSurface (surface, 0, output, &dst_rect);
			dst_rect.x += 128;
		}
		SDL_FreeSurface (surface);

		surface = getImageFromRes ("LRGCONES");
		dst_rect.y = 5;
		dst_rect.x = 5;
		for (int i = 0; i < 4; i++)
		{
			SDL_BlitSurface (surface, 0, output, &dst_rect);
			dst_rect.x += 128;
		}
		SDL_FreeSurface (surface);

		surface = getImageFromRes ("CONSTRCT", 16);
		removePlayerColor (surface);
		dst_rect.x = 33;
		dst_rect.y = 36;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		SDL_FreeSurface (surface);

		surface = getImageFromRes ("CONSTRCT", 24);
		removePlayerColor (surface);
		generateAnimationFrame (surface, 1);
		dst_rect.x += 128;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		SDL_FreeSurface (surface);

		surface = getImageFromRes ("CONSTRCT", 16);
		removePlayerColor (surface);
		generateAnimationFrame (surface, 2);
		dst_rect.x += 128;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		SDL_FreeSurface (surface);

		surface = getImageFromRes ("CONSTRCT", 32);
		removePlayerColor (surface);
		generateAnimationFrame (surface, 3);
		dst_rect.x += 128;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		SDL_FreeSurface (surface);
		savePCX (output, path / "build.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "build.pcx")

	try
	{
		output = getImageFromRes ("S_LRGCON");
		resizeSurface (output, 6, 6, 128, 128);
		surface = getImageFromRes ("S_CONSTR");
		dst_rect.x = 38;
		dst_rect.y = 37;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		SDL_FreeSurface (surface);
		savePCX (output, path / "build_shw.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "build_shw.pcx")

	// minelayer
	path = sOutputPath / "vehicles" / "minelayer";
	for (int i = 0; i < 8; i++)
	{
		auto szNum = std::to_string (i);
		copyFileFromRes_rpc ("MINELAYR", path / ("img" + szNum + ".pcx"), i);
		copyFileFromRes ("S_MINELA", path / ("shw" + szNum + ".pcx"), i);
	}
	copyFileFromRes ("A_MNELAY", path / "store.pcx");
	copyFileFromRes ("P_MNELAY", path / "info.pcx");

	// missel
	path = sOutputPath / "vehicles" / "missel";
	for (int i = 0; i < 8; i++)
	{
		auto szNum = std::to_string (i);
		try
		{
			surface = getImageFromRes ("MISSLLCH", i);
			removePlayerColor (surface);
			resizeSurface (surface, 16, 15, 64, 64);
			savePCX (surface, path / ("img" + szNum + ".pcx"));
			SDL_FreeSurface (surface);
		}
		END_INSTALL_FILE (path / ("img" + szNum + ".pcx"));

		try
		{
			surface = getImageFromRes ("S_MISSLL", i);
			resizeSurface (surface, 16, 16, 64, 64);
			savePCX (surface, path / ("shw" + szNum + ".pcx"));
			SDL_FreeSurface (surface);
		}
		END_INSTALL_FILE (path / ("shw" + szNum + ".pcx"));
	}
	copyFileFromRes ("A_MISSIL", path / "store.pcx");
	copyFileFromRes ("P_MISSIL", path / "info.pcx");

	// missel_ship
	path = sOutputPath / "vehicles" / "missel_ship";
	for (int i = 0; i < 8; i++)
	{
		auto szNum = std::to_string (i);
		try
		{
			surface = getImageFromRes ("MSSLBOAT", i);
			removePlayerColor (surface);
			resizeSurface (surface, 16, 16, 64, 64);
			savePCX (surface, path / ("img" + szNum + ".pcx"));
			SDL_FreeSurface (surface);
		}
		END_INSTALL_FILE (path / ("img" + szNum + ".pcx"))
		try
		{
			surface = getImageFromRes ("S_MSSLBO", i);
			resizeSurface (surface, 8, 8, 66, 66);
			savePCX (surface, path / ("shw" + szNum + ".pcx"));
			SDL_FreeSurface (surface);
		}
		END_INSTALL_FILE (path / ("shw" + szNum + ".pcx"));
	}
	copyFileFromRes ("A_MSLCR", path / "store.pcx");
	copyFileFromRes ("P_MSLCR", path / "info.pcx");

	// mobile_aa
	path = sOutputPath / "vehicles" / "mobile_aa";
	src_rect.h = 33;
	src_rect.w = 35;
	src_rect.x = 17;
	src_rect.y = 16;

	try
	{
		output = getImageFromRes ("SP_FLAK", 0);
		removePlayerColor (output);
		resizeSurface (output, 3, 0, 64, 64);
		dst_rect.y = 3;
		dst_rect.x = 14;
		surface = getImageFromRes ("SP_FLAK", 8);
		SDL_BlitSurface (surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface (surface);
		savePCX (output, path / "img0.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "img0.pcx")

	try
	{
		output = getImageFromRes ("SP_FLAK", 1);
		removePlayerColor (output);
		resizeSurface (output, 3, 0, 64, 64);
		dst_rect.y = 4;
		dst_rect.x = 24;
		surface = getImageFromRes ("SP_FLAK", 9);
		SDL_BlitSurface (surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface (surface);
		savePCX (output, path / "img1.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "img1.pcx")

	try
	{
		output = getImageFromRes ("SP_FLAK", 2);
		removePlayerColor (output);
		resizeSurface (output, 3, 0, 64, 64);
		dst_rect.y = 15;
		dst_rect.x = 24;
		surface = getImageFromRes ("SP_FLAK", 10);
		SDL_BlitSurface (surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface (surface);
		savePCX (output, path / "img2.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "img2.pcx")

	try
	{
		output = getImageFromRes ("SP_FLAK", 3);
		removePlayerColor (output);
		resizeSurface (output, 3, 0, 64, 64);
		dst_rect.y = 23;
		dst_rect.x = 22;
		surface = getImageFromRes ("SP_FLAK", 11);
		SDL_BlitSurface (surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface (surface);
		savePCX (output, path / "img3.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "img3.pcx")

	try
	{
		output = getImageFromRes ("SP_FLAK", 4);
		removePlayerColor (output);
		resizeSurface (output, 3, 0, 64, 64);
		dst_rect.y = 27;
		dst_rect.x = 14;
		surface = getImageFromRes ("SP_FLAK", 12);
		SDL_BlitSurface (surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface (surface);
		savePCX (output, path / "img4.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "img4.pcx")

	try
	{
		output = getImageFromRes ("SP_FLAK", 5);
		removePlayerColor (output);
		resizeSurface (output, 3, 0, 64, 64);
		dst_rect.y = 22;
		dst_rect.x = 6;
		surface = getImageFromRes ("SP_FLAK", 13);
		SDL_BlitSurface (surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface (surface);
		savePCX (output, path / "img5.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "img5.pcx")

	try
	{
		output = getImageFromRes ("SP_FLAK", 6);
		removePlayerColor (output);
		resizeSurface (output, 3, 0, 64, 64);
		dst_rect.y = 16;
		dst_rect.x = 4;
		surface = getImageFromRes ("SP_FLAK", 14);
		SDL_BlitSurface (surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface (surface);
		savePCX (output, path / "img6.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "img6.pcx")

	try
	{
		output = getImageFromRes ("SP_FLAK", 7);
		removePlayerColor (output);
		resizeSurface (output, 3, 0, 64, 64);
		dst_rect.y = 8;
		dst_rect.x = 5;
		surface = getImageFromRes ("SP_FLAK", 15);
		SDL_BlitSurface (surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface (surface);
		savePCX (output, path / "img7.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "img7.pcx")

	for (int i = 0; i < 8; i++)
	{
		auto szNum = std::to_string (i);
		copyFileFromRes ("S_FLAK", path / ("shw" + szNum + ".pcx"), i);
	}
	copyFileFromRes ("A_AA", path / "store.pcx");
	copyFileFromRes ("P_AA", path / "info.pcx");

	// pionier
	path = sOutputPath / "vehicles" / "pionier";
	for (int i = 0; i < 8; i++)
	{
		auto szNum = std::to_string (i);
		try
		{
			surface = getImageFromRes ("ENGINEER", i);
			removePlayerColor (surface);
			resizeSurface (surface, 4, 4, 64, 64);
			savePCX (surface, path / ("img" + szNum + ".pcx"));
			SDL_FreeSurface (surface);
		}
		END_INSTALL_FILE (path / ("img" + szNum + ".pcx"))

		try
		{
			surface = getImageFromRes ("S_ENGINE", i);
			resizeSurface (surface, 4, 4, 67, 66);
			savePCX (surface, path / ("shw" + szNum + ".pcx"));
			SDL_FreeSurface (surface);
		}
		END_INSTALL_FILE (path / ("shw" + szNum + ".pcx"))
	}
	copyFileFromRes ("A_ENGINR", path / "store.pcx");
	copyFileFromRes ("P_ENGINR", path / "info.pcx");

	try
	{
		surface = getImageFromRes ("SMLTAPE");
		output = SDL_CreateRGBSurface (SDL_SWSURFACE, 256, 64, 8, 0, 0, 0, 0);
		SDL_SetPaletteColors (output->format->palette, surface->format->palette->colors, 0, 256);
		SDL_FillRect (output, nullptr, SDL_MapRGB (output->format, 255, 0, 255));

		dst_rect.x = 0;
		dst_rect.y = 0;
		for (int i = 0; i < 4; i++)
		{
			SDL_BlitSurface (surface, 0, output, &dst_rect);
			dst_rect.x += 64;
		}
		SDL_FreeSurface (surface);

		surface = getImageFromRes ("SMLCONES");
		dst_rect.y = 5;
		dst_rect.x = 5;
		for (int i = 0; i < 4; i++)
		{
			SDL_BlitSurface (surface, 0, output, &dst_rect);
			dst_rect.x += 64;
		}
		SDL_FreeSurface (surface);

		surface = getImageFromRes ("ENGINEER", 16);
		removePlayerColor (surface);
		dst_rect.x = 4;
		dst_rect.y = 4;
		SDL_BlitSurface (surface, 0, output, &dst_rect);

		generateAnimationFrame (surface, 1);
		dst_rect.x += 64;
		SDL_BlitSurface (surface, 0, output, &dst_rect);

		generateAnimationFrame (surface, 2);
		dst_rect.x += 64;
		SDL_BlitSurface (surface, 0, output, &dst_rect);

		generateAnimationFrame (surface, 3);
		dst_rect.x += 64;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		SDL_FreeSurface (surface);
		savePCX (output, path / "build.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "build.pcx")

	try
	{
		output = getImageFromRes ("S_SMLCON");
		resizeSurface (output, 6, 6, 64, 66);
		surface = getImageFromRes ("S_ENGINE");
		dst_rect.x = 6;
		dst_rect.y = 5;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		SDL_FreeSurface (surface);
		savePCX (output, path / "build_shw.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "build_shw.pcx")

	// repair
	path = sOutputPath / "vehicles" / "repair";
	for (int i = 0; i < 8; i++)
	{
		auto szNum = std::to_string (i);
		copyFileFromRes_rpc ("REPAIR", path / ("img" + szNum + ".pcx"), i);
		copyFileFromRes ("S_REPAIR", path / ("shw" + szNum + ".pcx"), i);
	}
	copyFileFromRes ("A_REPAIR", path / "store.pcx");
	copyFileFromRes ("P_REPAIR", path / "info.pcx");

	// scanner
	path = sOutputPath / "vehicles" / "scanner";
	for (int i = 0; i < 8; i++)
	{
		auto szNum = std::to_string (i);
		try
		{
			surface = getImageFromRes ("SCANNER", i);
			removePlayerColor (surface);
			resizeSurface (surface, 8, 8, 64, 64);
			savePCX (surface, path / ("img" + szNum + ".pcx"));
			SDL_FreeSurface (surface);
		}
		END_INSTALL_FILE (path / ("img" + szNum + ".pcx"))

		try
		{
			surface = getImageFromRes ("S_SCANNE", i);
			resizeSurface (surface, 8, 8, 61, 60);
			savePCX (surface, path / ("shw" + szNum + ".pcx"));
			SDL_FreeSurface (surface);
		}
		END_INSTALL_FILE (path / ("shw" + szNum + ".pcx"))
	}
	copyFileFromRes ("A_SCANNR", path / "store.pcx");
	copyFileFromRes ("P_SCANNR", path / "info.pcx");

	try
	{
		surface = getImageFromRes ("SCANNER", 8);
		output = SDL_CreateRGBSurface (SDL_SWSURFACE, 360, 45, 8, 0, 0, 0, 0);
		SDL_SetPaletteColors (output->format->palette, surface->format->palette->colors, 0, 256);
		SDL_FillRect (output, nullptr, SDL_MapRGB (output->format, 255, 0, 255));

		resizeSurface (surface, 2, 1, 45, 45);
		dst_rect.x = 0;
		dst_rect.y = 0;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		SDL_FreeSurface (surface);
		dst_rect.x = 45;
		for (int i = 1; i < 8; i++)
		{
			surface = getImageFromRes ("SCANNER", i * 2 + 8);
			resizeSurface (surface, 2, 1, 45, 45);
			SDL_BlitSurface (surface, 0, output, &dst_rect);
			SDL_FreeSurface (surface);
			dst_rect.x += 45;
		}
		savePCX (output, path / "overlay.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "overlay.pcx")

	// scout
	path = sOutputPath / "vehicles" / "scout";
	for (int i = 0; i < 8; i++)
	{
		auto szNum = std::to_string (i);
		copyFileFromRes_rpc ("SCOUT", path / ("img" + szNum + ".pcx"), i);
		try
		{
			surface = getImageFromRes ("S_SCOUT", i);
			resizeSurface (surface, 11, 10, 61, 59);
			savePCX (surface, path / ("shw" + szNum + ".pcx"));
			SDL_FreeSurface (surface);
		}
		END_INSTALL_FILE (path / ("shw" + szNum + ".pcx"))
	}
	copyFileFromRes ("A_SCOUT", path / "store.pcx");
	copyFileFromRes ("P_SCOUT", path / "info.pcx");

	// sea_minelayer
	path = sOutputPath / "vehicles" / "sea_minelayer";
	for (int i = 0; i < 8; i++)
	{
		auto szNum = std::to_string (i);
		copyFileFromRes_rpc ("SEAMNLYR", path / ("img" + szNum + ".pcx"), i);
		copyFileFromRes ("S_SEAMNL", path / ("shw" + szNum + ".pcx"), i);
	}
	copyFileFromRes ("A_SEAMNL", path / "store.pcx");
	copyFileFromRes ("P_SEAMNL", path / "info.pcx");

	// sea_transport
	path = sOutputPath / "vehicles" / "sea_transport";
	for (int i = 0; i < 8; i++)
	{
		auto szNum = std::to_string (i);
		copyFileFromRes_rpc ("SEATRANS", path / ("img" + szNum + ".pcx"), i);
		try
		{
			surface = getImageFromRes ("S_SEATRA", i);
			resizeSurface (surface, 10, 10, 68, 68);
			savePCX (surface, path / ("shw" + szNum + ".pcx"));
			SDL_FreeSurface (surface);
		}
		END_INSTALL_FILE (path / ("shw" + szNum + ".pcx"));
	}
	copyFileFromRes ("A_SEATRN", path / "store.pcx");
	copyFileFromRes ("P_SEATRN", path / "info.pcx");

	// sub
	path = sOutputPath / "vehicles" / "sub";
	for (int i = 0; i < 8; i++)
	{
		auto szNum = std::to_string (i);
		copyFileFromRes_rpc ("SUBMARNE", path / ("img" + szNum + ".pcx"), i + 8);
	}
	for (int i = 0; i < 8; i++)
	{
		auto szNum = std::to_string (i);
		copyFileFromRes_rpc ("SUBMARNE", path / ("img_stealth" + szNum + ".pcx"), i);
	}
	copyFileFromRes ("A_SUB", path / "store.pcx");
	copyFileFromRes ("P_SUB", path / "info.pcx");

	// surveyor
	path = sOutputPath / "vehicles" / "surveyor";
	for (int i = 0; i < 8; i++)
	{
		auto szNum = std::to_string (i);
		try
		{
			surface = getImageFromRes ("SURVEYOR", i);
			removePlayerColor (surface);
			resizeSurface (surface, 2, 2, 64, 64);
			savePCX (surface, path / ("img" + szNum + ".pcx"));
			SDL_FreeSurface (surface);
		}
		END_INSTALL_FILE (path / ("img" + szNum + ".pcx"))

		try
		{
			surface = getImageFromRes ("S_SURVEY", i);
			resizeSurface (surface, 2, 2, 69, 72);
			savePCX (surface, path / ("shw" + szNum + ".pcx"));
			SDL_FreeSurface (surface);
		}
		END_INSTALL_FILE (path / ("shw" + szNum + ".pcx"))
	}
	copyFileFromRes ("A_SURVEY", path / "store.pcx");
	copyFileFromRes ("P_SURVEY", path / "info.pcx");

	// tank
	path = sOutputPath / "vehicles" / "tank";
	for (int i = 0; i < 8; i++)
	{
		auto szNum = std::to_string (i);
		try
		{
			output = getImageFromRes ("TANK", i);
			removePlayerColor (output);
			resizeSurface (output, 15, 15, 64, 64);
			surface = getImageFromRes ("TANK", i + 8);
			src_rect.x = 15;
			src_rect.y = 15;
			src_rect.h = 64;
			src_rect.w = 64;
			SDL_BlitSurface (surface, &src_rect, output, 0);
			SDL_FreeSurface (surface);
			savePCX (output, path / ("img" + szNum + ".pcx"));
			SDL_FreeSurface (output);
		}
		END_INSTALL_FILE (path / ("img" + szNum + ".pcx"))

		try
		{
			surface = getImageFromRes ("S_TANK", i);
			resizeSurface (surface, 2, 2, 69, 72);
			savePCX (surface, path / ("shw" + szNum + ".pcx"));
			SDL_FreeSurface (surface);
		}
		END_INSTALL_FILE (path / ("shw" + szNum + ".pcx"))
	}
	copyFileFromRes ("A_TANK", path / "store.pcx");
	copyFileFromRes ("P_TANK", path / "info.pcx");

	// trans_gold
	path = sOutputPath / "vehicles" / "trans_gold";
	for (int i = 0; i < 8; i++)
	{
		auto szNum = std::to_string (i);
		copyFileFromRes_rpc ("GOLDTRCK", path / ("img" + szNum + ".pcx"), i);
		try
		{
			surface = getImageFromRes ("S_GOLDTR", i);
			resizeSurface (surface, 6, 6, 76, 74);
			savePCX (surface, path / ("shw" + szNum + ".pcx"));
			SDL_FreeSurface (surface);
		}
		END_INSTALL_FILE (path / ("shw" + szNum + ".pcx"))
	}
	copyFileFromRes ("A_GOLDTR", path / "store.pcx");
	copyFileFromRes ("P_GOLDTR", path / "info.pcx");

	// trans_metal
	path = sOutputPath / "vehicles" / "trans_metal";
	for (int i = 0; i < 8; i++)
	{
		auto szNum = std::to_string (i);
		try
		{
			surface = getImageFromRes ("SPLYTRCK", i);
			removePlayerColor (surface);
			resizeSurface (surface, 4, 4, 64, 64);
			savePCX (surface, path / ("img" + szNum + ".pcx"));
			SDL_FreeSurface (surface);
		}
		END_INSTALL_FILE (path / ("img" + szNum + ".pcx"))

		try
		{
			surface = getImageFromRes ("S_SPLYTR", i);
			resizeSurface (surface, 8, 8, 64, 64);
			savePCX (surface, path / ("shw" + szNum + ".pcx"));
			SDL_FreeSurface (surface);
		}
		END_INSTALL_FILE (path / ("shw" + szNum + ".pcx"))
	}
	copyFileFromRes ("A_SPLYTR", path / "store.pcx");
	copyFileFromRes ("P_SPLYTR", path / "info.pcx");

	// trans_oil
	path = sOutputPath / "vehicles" / "trans_oil";
	for (int i = 0; i < 8; i++)
	{
		auto szNum = std::to_string (i);
		try
		{
			surface = getImageFromRes ("FUELTRCK", i);
			removePlayerColor (surface);
			resizeSurface (surface, 3, 3, 64, 64);
			savePCX (surface, path / ("img" + szNum + ".pcx"));
			SDL_FreeSurface (surface);
		}
		END_INSTALL_FILE (path / ("img" + szNum + ".pcx"))

		try
		{
			surface = getImageFromRes ("S_FUELTR", i);
			resizeSurface (surface, 3, 3, 66, 66);
			savePCX (surface, path / ("shw" + szNum + ".pcx"));
			SDL_FreeSurface (surface);
		}
		END_INSTALL_FILE (path / ("shw" + szNum + ".pcx"))
	}
	copyFileFromRes ("A_FUELTR", path / "store.pcx");
	copyFileFromRes ("P_FUELTR", path / "info.pcx");

	if (logFile != nullptr)
	{
		writeLog (std::string ("Vehicle graphics") + TEXT_FILE_LF);
		writeLog (std::to_string (iErrors) + " errors" + TEXT_FILE_LF);
		writeLog (std::string ("========================================================================") + TEXT_FILE_LF);
	}

	std::cout << "\n";
	std::cout << std::to_string (iErrors) << " errors\n";
	return 1;
}

//-------------------------------------------------------------
int installBuildingGraphics()
{
	iTotalFiles = 161;
	iErrors = 0;
	iInstalledFiles = 0;
	SDL_Surface* surface;
	SDL_Surface* output;
	SDL_Rect src_rect, dst_rect;

#if MAC
	updateProgressWindow ("Building graphics", iTotalFiles, iInstalledFiles);
#endif

	std::cout << "========================================================================\n";
	std::cout << "Building graphics\n";

	// Barracks
	auto path = sOutputPath / "buildings" / "barracks";
	copyFileFromRes_rpc ("BARRACKS", path / "img.pcx", 1);
	copyFileFromRes ("P_BARRCK", path / "info.pcx");
	copyFileFromRes ("S_BARRAC", path / "shw.pcx");
	copyImageFromFLC (sMAXPath / "BARX_ISO.FLC", path / "video.pcx");

	// block
	path = sOutputPath / "buildings" / "block";
	copyFileFromRes ("BLOCK", path / "img.pcx");
	copyFileFromRes ("P_BLOCK", path / "info.pcx");
	copyFileFromRes ("S_BLOCK", path / "shw.pcx");
	copyImageFromFLC (sMAXPath / "BLOK128.FLC", path / "video.pcx");

	// bridge
	path = sOutputPath / "buildings" / "bridge";
	copyFileFromRes_rpc ("BRIDGE", path / "img.pcx");
	copyFileFromRes ("P_BRIDGE", path / "info.pcx");
	copyFileFromRes ("S_BRIDGE", path / "shw.pcx");
	copyImageFromFLC (sMAXPath / "BRIDGE.FLC", path / "video.pcx");

	// connector
	path = sOutputPath / "buildings" / "connector";
	try
	{
		surface = getImageFromRes ("CNCT_4W", 0);
		output = SDL_CreateRGBSurface (SDL_SWSURFACE, 1024, 64, 8, 0, 0, 0, 0);
		SDL_SetPaletteColors (output->format->palette, surface->format->palette->colors, 0, 256);
		SDL_FillRect (output, nullptr, SDL_MapRGB (output->format, 255, 0, 255));
		dst_rect.y = 0;
		for (int i = 0; i < 1024; i += 64)
		{
			dst_rect.x = i;
			SDL_BlitSurface (surface, 0, output, &dst_rect);
		}
		SDL_FreeSurface (surface);

		surface = getImageFromRes ("CNCT_4W", 2);
		dst_rect.x = 64;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 320;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 448;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 512;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 704;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 768;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 832;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 960;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		SDL_FreeSurface (surface);

		surface = getImageFromRes ("CNCT_4W", 3);
		dst_rect.x = 128;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 384;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 512;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 576;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 768;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 832;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 896;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 960;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		SDL_FreeSurface (surface);

		surface = getImageFromRes ("CNCT_4W", 4);
		dst_rect.x = 192;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 320;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 576;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 640;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 704;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 832;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 896;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 960;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		SDL_FreeSurface (surface);

		surface = getImageFromRes ("CNCT_4W", 5);
		dst_rect.x = 256;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 384;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 448;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 640;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 704;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 768;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 896;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 960;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		SDL_FreeSurface (surface);

		savePCX (output, path / "img.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "img.pcx")

	try
	{
		surface = getImageFromRes ("S_CNCT4W", 0);
		output = SDL_CreateRGBSurface (SDL_SWSURFACE, 1024, 64, 8, 0, 0, 0, 0);
		SDL_SetPaletteColors (output->format->palette, surface->format->palette->colors, 0, 256);
		SDL_FillRect (output, nullptr, SDL_MapRGB (output->format, 255, 0, 255));
		dst_rect.y = 0;
		for (int i = 0; i < 1024; i += 64)
		{
			dst_rect.x = i;
			SDL_BlitSurface (surface, 0, output, &dst_rect);
		}
		SDL_FreeSurface (surface);

		surface = getImageFromRes ("S_CNCT4W", 2);
		dst_rect.x = 64;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 320;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 448;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 512;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 704;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 768;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 832;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 960;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		SDL_FreeSurface (surface);

		surface = getImageFromRes ("S_CNCT4W", 3);
		dst_rect.x = 128;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 384;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 512;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 576;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 768;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 832;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 896;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 960;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		SDL_FreeSurface (surface);

		surface = getImageFromRes ("S_CNCT4W", 4);
		dst_rect.x = 192;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 320;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 576;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 640;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 704;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 832;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 896;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 960;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		SDL_FreeSurface (surface);

		surface = getImageFromRes ("S_CNCT4W", 5);
		dst_rect.x = 256;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 384;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 448;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 640;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 704;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 768;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 896;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		dst_rect.x = 960;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		SDL_FreeSurface (surface);

		savePCX (output, path / "shw.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "shw.pcx")

	copyFileFromRes ("P_CONNEC", path / "info.pcx");
	copyImageFromFLC (sMAXPath / "CROSS.FLC", path / "video.pcx");

	// depot
	path = sOutputPath / "buildings" / "depot";
	copyFileFromRes_rpc ("DEPOT", path / "img.pcx", 1);
	copyFileFromRes ("P_DEPOT", path / "info.pcx");
	copyFileFromRes ("S_DEPOT", path / "shw.pcx", 1);
	copyImageFromFLC (sMAXPath / "DPBG_S.FLC", path / "video.pcx");

	// dock
	path = sOutputPath / "buildings" / "dock";
	copyFileFromRes_rpc ("DOCK", path / "img.pcx");
	copyFileFromRes ("S_DOCK", path / "shw.pcx");
	copyFileFromRes ("P_DOCK", path / "info.pcx");
	copyImageFromFLC (sMAXPath / "DOCK.FLC", path / "video.pcx");

	// eco-sphere
	path = sOutputPath / "buildings" / "ecosphere";
	copyFileFromRes_rpc ("GREENHSE", path / "img.pcx");
	copyFileFromRes ("S_GREENH", path / "shw.pcx");
	copyFileFromRes ("P_GREENH", path / "info.pcx");
	copyImageFromFLC (sMAXPath / "GRNHOUSE.FLC", path / "video.pcx");
	copyFile (path / "effect_org.pcx", path / "effect.pcx");

	// energy big
	path = sOutputPath / "buildings" / "energy_big";
	copyFileFromRes_rpc ("POWERSTN", path / "img.pcx");
	copyFileFromRes ("S_POWERS", path / "shw.pcx");
	copyFileFromRes ("P_POWSTN", path / "info.pcx");
	copyImageFromFLC (sMAXPath / "PWRGEN.FLC", path / "video.pcx");
	copyFile (path / "effect_org.pcx", path / "effect.pcx");

	// energy small
	path = sOutputPath / "buildings" / "energy_small";
	copyFileFromRes_rpc ("POWGEN", path / "img.pcx");
	copyFileFromRes ("S_POWGEN", path / "shw.pcx");
	copyFileFromRes ("P_POWGEN", path / "info.pcx");
	copyImageFromFLC (sMAXPath / "SPWR_ISO.FLC", path / "video.pcx");
	copyFile (path / "effect_org.pcx", path / "effect.pcx");

	// fac air
	path = sOutputPath / "buildings" / "fac_air";
	copyFileFromRes_rpc ("AIRPLT", path / "img.pcx");
	copyFileFromRes ("S_AIRPLT", path / "shw.pcx");
	copyFileFromRes ("P_AIRPLT", path / "info.pcx");
	copyImageFromFLC (sMAXPath / "AIRPLNT.FLC", path / "video.pcx");
	copyFile (path / "effect_org.pcx", path / "effect.pcx");

	// fac alien
	path = sOutputPath / "buildings" / "fac_alien";
	copyFileFromRes_rpc ("RECCENTR", path / "img.pcx");
	copyFileFromRes ("S_RECCEN", path / "shw.pcx");
	copyFileFromRes ("P_RECCTR", path / "info.pcx");
	copyImageFromFLC (sMAXPath / "RECNTR.FLC", path / "video.pcx");
	copyFile (path / "effect_org.pcx", path / "effect.pcx");

	// fac big
	path = sOutputPath / "buildings" / "fac_big";
	copyFileFromRes_rpc ("LANDPLT", path / "img.pcx");
	copyFileFromRes ("S_LANDPL", path / "shw.pcx");
	copyFileFromRes ("P_HVYPLT", path / "info.pcx");
	copyImageFromFLC (sMAXPath / "FVP.FLC", path / "video.pcx");
	copyFile (path / "effect_org.pcx", path / "effect.pcx");

	// fac ship
	path = sOutputPath / "buildings" / "fac_ship";
	copyFileFromRes_rpc ("SHIPYARD", path / "img.pcx");
	copyFileFromRes ("S_SHIPYA", path / "shw.pcx");
	copyFileFromRes ("P_SHIPYD", path / "info.pcx");
	copyImageFromFLC (sMAXPath / "SHPYRD.FLC", path / "video.pcx");
	copyFile (path / "effect_org.pcx", path / "effect.pcx");

	// fac small
	path = sOutputPath / "buildings" / "fac_small";
	copyFileFromRes_rpc ("LIGHTPLT", path / "img.pcx");
	copyFileFromRes ("S_LIGHTP", path / "shw.pcx");
	copyFileFromRes ("P_LGHTPL", path / "info.pcx");
	copyImageFromFLC (sMAXPath / "LVP_ISO.FLC", path / "video.pcx");
	copyFile (path / "effect_org.pcx", path / "effect.pcx");

	// goldraff
	path = sOutputPath / "buildings" / "goldraff";
	copyFileFromRes_rpc ("COMMTWR", path / "img.pcx");
	copyFileFromRes ("S_COMMTW", path / "shw.pcx");
	copyFileFromRes ("P_TRANSP", path / "info.pcx");
	copyImageFromFLC (sMAXPath / "COMMTWR.FLC", path / "video.pcx");
	copyFile (path / "effect_org.pcx", path / "effect.pcx");

	// gun aa
	path = sOutputPath / "buildings" / "gun_aa";
	try
	{
		surface = getImageFromRes ("ANTIAIR");
		removePlayerColor (surface);
		output = SDL_CreateRGBSurface (SDL_SWSURFACE, 512, 64, 8, 0, 0, 0, 0);
		SDL_SetPaletteColors (output->format->palette, surface->format->palette->colors, 0, 256);
		SDL_FillRect (output, nullptr, SDL_MapRGB (output->format, 255, 0, 255));
		dst_rect.y = 0;
		dst_rect.x = 0;
		src_rect.x = 16;
		src_rect.y = 13;
		src_rect.h = 64;
		src_rect.w = 64;
		for (int i = 0; i < 512; i += 64)
		{
			dst_rect.x = i;
			SDL_BlitSurface (surface, &src_rect, output, &dst_rect);
		}
		SDL_FreeSurface (surface);

		for (int i = 1; i < 9; i++)
		{
			surface = getImageFromRes ("ANTIAIR", i);
			dst_rect.x = (i - 1) * 64;
			SDL_BlitSurface (surface, &src_rect, output, &dst_rect);
			SDL_FreeSurface (surface);
		}

		savePCX (output, path / "img.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "img.pcx")

	copyFileFromRes ("S_ANTIAI", path / "shw.pcx", 8);
	copyFileFromRes ("P_FXAA", path / "info.pcx");
	copyImageFromFLC (sMAXPath / "AA_ISO.FLC", path / "video.pcx");

	// gun ari
	path = sOutputPath / "buildings" / "gun_ari";
	try
	{
		surface = getImageFromRes ("ARTYTRRT");
		removePlayerColor (surface);
		output = SDL_CreateRGBSurface (SDL_SWSURFACE, 512, 64, 8, 0, 0, 0, 0);
		SDL_SetPaletteColors (output->format->palette, surface->format->palette->colors, 0, 256);
		SDL_FillRect (output, nullptr, SDL_MapRGB (output->format, 255, 0, 255));
		dst_rect.y = 0;
		dst_rect.x = 0;
		src_rect.x = 16;
		src_rect.y = 16;
		src_rect.h = 64;
		src_rect.w = 64;
		for (int i = 0; i < 512; i += 64)
		{
			dst_rect.x = i;
			SDL_BlitSurface (surface, &src_rect, output, &dst_rect);
		}
		SDL_FreeSurface (surface);

		for (int i = 1; i < 9; i++)
		{
			surface = getImageFromRes ("ARTYTRRT", i);
			dst_rect.x = (i - 1) * 64;
			SDL_BlitSurface (surface, &src_rect, output, &dst_rect);
			SDL_FreeSurface (surface);
		}

		savePCX (output, path / "img.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "img.pcx");

	copyFileFromRes ("S_ARTYTR", path / "shw.pcx", 8);
	copyFileFromRes ("P_ARTYTR", path / "info.pcx");
	copyImageFromFLC (sMAXPath / "FXDGUN.FLC", path / "video.pcx");

	// gun missile
	path = sOutputPath / "buildings" / "gun_missel";
	try
	{
		surface = getImageFromRes ("ANTIMSSL", 0);
		removePlayerColor (surface);
		output = SDL_CreateRGBSurface (SDL_SWSURFACE, 512, 64, 8, 0, 0, 0, 0);
		SDL_SetPaletteColors (output->format->palette, surface->format->palette->colors, 0, 256);
		SDL_FillRect (output, nullptr, SDL_MapRGB (output->format, 255, 0, 255));
		dst_rect.y = 0;
		dst_rect.x = 0;
		src_rect.x = 15;
		src_rect.y = 16;
		src_rect.h = 64;
		src_rect.w = 64;
		for (int i = 0; i < 512; i += 64)
		{
			dst_rect.x = i;
			SDL_BlitSurface (surface, &src_rect, output, &dst_rect);
		}
		SDL_FreeSurface (surface);

		for (int i = 1; i < 9; i++)
		{
			surface = getImageFromRes ("ANTIMSSL", i);
			dst_rect.x = (i - 1) * 64;
			SDL_BlitSurface (surface, &src_rect, output, &dst_rect);
			SDL_FreeSurface (surface);
		}
		savePCX (output, path / "img.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "img.pcx")

	copyFileFromRes ("S_ANTIMS", path / "shw.pcx", 1);
	copyFileFromRes ("P_FXROCK", path / "info.pcx");
	copyImageFromFLC (sMAXPath / "ANTIMSL.FLC", path / "video.pcx");

	// gun turret
	path = sOutputPath / "buildings" / "gun_turret";
	try
	{
		surface = getImageFromRes ("GUNTURRT", 0);
		removePlayerColor (surface);
		output = SDL_CreateRGBSurface (SDL_SWSURFACE, 512, 64, 8, 0, 0, 0, 0);
		SDL_SetPaletteColors (output->format->palette, surface->format->palette->colors, 0, 256);
		SDL_FillRect (output, nullptr, SDL_MapRGB (output->format, 255, 0, 255));
		dst_rect.y = 0;
		dst_rect.x = 0;
		src_rect.x = 13;
		src_rect.y = 15;
		src_rect.h = 64;
		src_rect.w = 64;
		for (int i = 0; i < 512; i += 64)
		{
			dst_rect.x = i;
			SDL_BlitSurface (surface, &src_rect, output, &dst_rect);
		}
		SDL_FreeSurface (surface);

		for (int i = 1; i < 9; i++)
		{
			surface = getImageFromRes ("GUNTURRT", i);
			dst_rect.x = (i - 1) * 64;
			SDL_BlitSurface (surface, &src_rect, output, &dst_rect);
			SDL_FreeSurface (surface);
		}

		savePCX (output, path / "img.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "img.pcx")

	copyFileFromRes ("S_GUNTUR", path / "shw.pcx", 1);
	copyFileFromRes ("P_GUNTUR", path / "info.pcx");
	copyImageFromFLC (sMAXPath / "FXAGUN.FLC", path / "video.pcx");

	// habitat
	path = sOutputPath / "buildings" / "habitat";
	copyFileFromRes_rpc ("HABITAT", path / "img.pcx", 1);
	copyFileFromRes ("S_HABITA", path / "shw.pcx", 1);
	copyFileFromRes ("P_HABITA", path / "info.pcx");
	copyImageFromFLC (sMAXPath / "DORM.FLC", path / "video.pcx");
	copyFile (path / "effect_org.pcx", path / "effect.pcx");

	// hangar
	path = sOutputPath / "buildings" / "hangar";
	copyFileFromRes_rpc ("HANGAR", path / "img.pcx", 1);
	copyFileFromRes ("S_HANGAR", path / "shw.pcx", 1);
	copyFileFromRes ("P_HANGAR", path / "info.pcx");
	copyImageFromFLC (sMAXPath / "HANGR.FLC", path / "video.pcx");
	copyFile (path / "effect_org.pcx", path / "effect.pcx");

	// landmine
	path = sOutputPath / "buildings" / "landmine";
	try
	{
		surface = getImageFromRes ("LANDMINE");
		output = SDL_CreateRGBSurface (SDL_SWSURFACE, 64, 64, 8, 0, 0, 0, 0);
		SDL_SetPaletteColors (output->format->palette, surface->format->palette->colors, 0, 256);
		SDL_FillRect (output, nullptr, SDL_MapRGB (output->format, 255, 0, 255));
		dst_rect.x = 22;
		dst_rect.y = 22;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		SDL_FreeSurface (surface);
		savePCX (output, path / "img.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "img.pcx")

	try
	{
		surface = getImageFromRes ("S_LANDMI");
		output = SDL_CreateRGBSurface (SDL_SWSURFACE, 42, 41, 8, 0, 0, 0, 0);
		SDL_SetPaletteColors (output->format->palette, surface->format->palette->colors, 0, 256);
		SDL_FillRect (output, nullptr, SDL_MapRGB (output->format, 255, 0, 255));
		dst_rect.x = 22;
		dst_rect.y = 22;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		SDL_FreeSurface (surface);
		savePCX (output, path / "shw.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "shw.pcx")

	copyFileFromRes ("P_LANDMN", path / "info.pcx");
	copyImageFromFLC (sMAXPath / "LMINE01.FLC", path / "video.pcx");

	// mine
	path = sOutputPath / "buildings" / "mine";

	try
	{
		output = getImageFromRes ("MININGST", 0);
		removePlayerColor (output);
		resizeSurface (output, 0, 0, 128 * 9, 128);

		for (int i = 0; i < 8; i++)
		{
			surface = getImageFromRes ("MININGST", i * 2);
			removePlayerColor (surface);
			dst_rect.x = i * 128 + 128;
			dst_rect.y = 0;
			SDL_BlitSurface (surface, nullptr, output, &dst_rect);
			SDL_FreeSurface (surface);
		}

		// remove the clan logo from the first image
		surface = loadPCX (path / "roof.pcx");
		dst_rect.x = 32;
		dst_rect.y = 40;
		SDL_BlitSurface (surface, nullptr, output, &dst_rect);

		savePCX (output, path / "img.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "img.pcx");

	copyFileFromRes ("S_MINING", path / "shw.pcx");
	copyFileFromRes ("P_MINING", path / "info.pcx");
	copyImageFromFLC (sMAXPath / "MSTRSTAT.FLC", path / "video.pcx");
	copyFile (path / "effect_org.pcx", path / "effect.pcx");

	// mine deep
	path = sOutputPath / "buildings" / "mine_deep";
	copyFileFromRes_rpc ("SUPRTPLT", path / "img.pcx");
	copyFileFromRes ("S_SUPRTP", path / "shw.pcx");
	copyFileFromRes ("P_LIFESP", path / "info.pcx");
	copyImageFromFLC (sMAXPath / "SVP.FLC", path / "video.pcx");
	copyFile (path / "effect_org.pcx", path / "effect.pcx");

	// pad
	path = sOutputPath / "buildings" / "pad";
	copyFileFromRes_rpc ("LANDPAD", path / "img.pcx");
	copyFileFromRes ("S_LANDPA", path / "shw.pcx");
	copyFileFromRes ("P_LANDPD", path / "info.pcx");
	copyImageFromFLC (sMAXPath / "LP_ISO.FLC", path / "video.pcx");
	copyFile (path / "effect_org.pcx", path / "effect.pcx");

	// platform
	path = sOutputPath / "buildings" / "platform";
	copyFileFromRes_rpc ("WTRPLTFM", path / "img.pcx");
	copyFileFromRes ("S_WTRPLT", path / "shw.pcx");
	copyFileFromRes ("P_WATER", path / "info.pcx");
	copyImageFromFLC (sMAXPath / "WP.FLC", path / "video.pcx");

	// radar
	path = sOutputPath / "buildings" / "radar";
	try
	{
		surface = getImageFromRes ("RADAR", 0);
		removePlayerColor (surface);
		output = SDL_CreateRGBSurface (SDL_SWSURFACE, 1024, 64, 8, 0, 0, 0, 0);
		SDL_SetPaletteColors (output->format->palette, surface->format->palette->colors, 0, 256);
		SDL_FillRect (output, nullptr, SDL_MapRGB (output->format, 255, 0, 255));
		dst_rect.x = 0;
		dst_rect.y = 0;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		SDL_FreeSurface (surface);
		for (int i = 1; i < 16; i++)
		{
			surface = getImageFromRes ("RADAR", i);
			removePlayerColor (surface);
			dst_rect.x = i * 64;
			SDL_BlitSurface (surface, 0, output, &dst_rect);
			SDL_FreeSurface (surface);
		}
		savePCX (output, path / "img.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "img.pcx")

	copyFileFromRes ("S_RADAR", path / "shw.pcx", 14);
	copyFileFromRes ("P_RADAR", path / "info.pcx");
	copyImageFromFLC (sMAXPath / "RADAR.FLC", path / "video.pcx");

	// research
	path = sOutputPath / "buildings" / "research";
	copyFileFromRes_rpc ("RESEARCH", path / "img.pcx");
	copyFileFromRes ("S_RESEAR", path / "shw.pcx");
	copyFileFromRes ("P_RESEAR", path / "info.pcx");
	copyImageFromFLC (sMAXPath / "RESEARCH.FLC", path / "video.pcx");
	copyFile (path / "effect_org.pcx", path / "effect.pcx");

	// road
	path = sOutputPath / "buildings" / "road";
	copyFileFromRes ("ROAD", path / "img.pcx");
	copyFileFromRes ("S_ROAD", path / "shw.pcx");
	copyFileFromRes ("P_ROAD", path / "info.pcx");
	copyImageFromFLC (sMAXPath / "ROAD.FLC", path / "video.pcx");

	// seamine
	path = sOutputPath / "buildings" / "seamine";
	try
	{
		surface = getImageFromRes ("SEAMINE");
		output = SDL_CreateRGBSurface (SDL_SWSURFACE, 42, 41, 8, 0, 0, 0, 0);
		SDL_SetPaletteColors (output->format->palette, surface->format->palette->colors, 0, 256);
		SDL_FillRect (output, nullptr, SDL_MapRGB (output->format, 255, 0, 255));
		dst_rect.x = 23;
		dst_rect.y = 23;
		SDL_BlitSurface (surface, 0, output, &dst_rect);
		SDL_FreeSurface (surface);
		savePCX (output, path / "img.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "img.pcx")

	// seamines don't have a shadow in the original.
	// So creating a dummy file here
	try
	{
		surface = getImageFromRes ("SEAMINE");
		output = SDL_CreateRGBSurface (SDL_SWSURFACE, 42, 41, 8, 0, 0, 0, 0);
		SDL_SetPaletteColors (output->format->palette, surface->format->palette->colors, 0, 256);
		SDL_FillRect (output, nullptr, SDL_MapRGB (output->format, 255, 0, 255));
		SDL_FreeSurface (surface);
		savePCX (output, path / "shw.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "shw.pcx")

	copyFileFromRes ("P_SEAMIN", path / "info.pcx");
	copyImageFromFLC (sMAXPath / "SMINE01.FLC", path / "video.pcx");

	// shield
	path = sOutputPath / "buildings" / "shield";
	copyFileFromRes_rpc ("SHIELDGN", path / "img.pcx");
	copyFileFromRes ("S_SHIELD", path / "shw.pcx");
	copyFileFromRes ("P_SHIELD", path / "info.pcx");
	copyImageFromFLC (sMAXPath / "SHLDGEN.FLC", path / "video.pcx");
	copyFile (path / "effect_org.pcx", path / "effect.pcx");

	// storage gold
	path = sOutputPath / "buildings" / "storage_gold";
	copyFileFromRes_rpc ("GOLDSM", path / "img.pcx");
	copyFileFromRes ("S_GOLDSM", path / "shw.pcx");
	copyFileFromRes ("P_SMVLT", path / "info.pcx");
	copyImageFromFLC (sMAXPath / "GOLDSM.FLC", path / "video.pcx");

	// storage metal
	path = sOutputPath / "buildings" / "storage_metal";
	copyFileFromRes_rpc ("ADUMP", path / "img.pcx");
	copyFileFromRes ("S_ADUMP", path / "shw.pcx");
	copyFileFromRes ("P_SMSTOR", path / "info.pcx");
	copyImageFromFLC (sMAXPath / "SS_ISO.FLC", path / "video.pcx");

	// storage oil
	path = sOutputPath / "buildings" / "storage_oil";
	copyFileFromRes_rpc ("FDUMP", path / "img.pcx");
	copyFileFromRes ("S_FDUMP", path / "shw.pcx");
	copyFileFromRes ("P_SMFUEL", path / "info.pcx");
	copyImageFromFLC (sMAXPath / "SF_ISO.FLC", path / "video.pcx");

	// training
	path = sOutputPath / "buildings" / "training";
	copyFileFromRes_rpc ("TRAINHAL", path / "img.pcx");
	copyFileFromRes ("S_TRAINH", path / "shw.pcx");
	copyFileFromRes ("P_TRNHLL", path / "info.pcx");
	copyImageFromFLC (sMAXPath / "THALL.FLC", path / "video.pcx");
	copyFile (path / "effect_org.pcx", path / "effect.pcx");

	// rubble
	path = sOutputPath / "buildings";
	try
	{
		output = getImageFromRes ("LRGRUBLE", 0);
		resizeSurface (output, 0, 0, 256, 128);
		surface = getImageFromRes ("LRGRUBLE", 1);
		SDL_Rect dst_rect = {128, 0, 0, 0};
		SDL_BlitSurface (surface, nullptr, output, &dst_rect);
		SDL_FreeSurface (surface);
		savePCX (output, path / "dirt_big.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "dirt_big.pcx");

	try
	{
		output = getImageFromRes ("SMLRUBLE", 0);
		resizeSurface (output, 0, 0, 320, 64);
		SDL_Rect dst_rect = {64, 0, 0, 0};
		for (int i = 1; i < 5; i++)
		{
			surface = getImageFromRes ("SMLRUBLE", i);
			SDL_BlitSurface (surface, nullptr, output, &dst_rect);
			SDL_FreeSurface (surface);
			dst_rect.x += 64;
		}
		savePCX (output, path / "dirt_small.pcx");
	}
	END_INSTALL_FILE (path / "dirt_small.pcx");

	try
	{
		output = getImageFromRes ("S_LRGRBL", 0);
		resizeSurface (output, 0, 0, 256, 128);
		surface = getImageFromRes ("S_LRGRBL", 1);
		SDL_Rect dst_rect = {128, 0, 0, 0};
		SDL_BlitSurface (surface, nullptr, output, &dst_rect);
		SDL_FreeSurface (surface);
		savePCX (output, path / "dirt_big_shw.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "dirt_big_shw.pcx");

	try
	{
		output = getImageFromRes ("S_SMLRBL", 0);
		resizeSurface (output, 0, 0, 320, 64);
		SDL_Rect dst_rect = {64, 0, 0, 0};
		for (int i = 1; i < 5; i++)
		{
			surface = getImageFromRes ("S_SMLRBL", i);
			SDL_BlitSurface (surface, nullptr, output, &dst_rect);
			SDL_FreeSurface (surface);
			dst_rect.x += 64;
		}
		savePCX (output, path / "dirt_small_shw.pcx");
	}
	END_INSTALL_FILE (path / "dirt_small_shw.pcx");

	std::cout << "\n";
	std::cout << std::to_string (iErrors) << " errors\n";

	if (logFile != nullptr)
	{
		writeLog (std::string ("Building graphics") + TEXT_FILE_LF);
		writeLog (std::to_string (iErrors) + " errors" + TEXT_FILE_LF);
		writeLog (std::string ("========================================================================") + TEXT_FILE_LF);
	}

	return 1;
}

//-------------------------------------------------------------
int installVehicleVideos()
{
	iTotalFiles = 35;
	iErrors = 0;
	iInstalledFiles = 0;

#if MAC
	updateProgressWindow ("Vehicle videos", iTotalFiles, iInstalledFiles);
#endif

	std::cout << "========================================================================\n";
	std::cout << "Vehicle videos\n";

	const std::filesystem::path path = std::filesystem::path (sOutputPath) / "vehicles";

	copyFile (sMAXPath / "ATRANS02.FLC", path / "air_transport" / "video.flc"); // air_transport
	copyFile (sMAXPath / "ALNASG.FLC", path / "alien_assault" / "video.flc"); // alien_assault
	copyFile (sMAXPath / "ALNPLANE.FLC", path / "alien_plane" / "video.flc"); // alien_plane
	copyFile (sMAXPath / "JUGGERN.FLC", path / "alien_ship" / "video.flc"); // alien_ship
	copyFile (sMAXPath / "ALNTANK.FLC", path / "alien_tank" / "video.flc"); // alien_tank
	copyFile (sMAXPath / "APC_TR01.FLC", path / "apc" / "video.flc"); // apc
	copyFile (sMAXPath / "E_ART2.FLC", path / "assault" / "video.flc"); // assault
	copyFile (sMAXPath / "AWACS03.FLC", path / "awac" / "video.flc"); // awac
	copyFile (sMAXPath / "BOMBER03.FLC", path / "bomber" / "video.flc"); // bomber
	copyFile (sMAXPath / "DOZER01.FLC", path / "bulldozer" / "video.flc"); // bulldozer
	copyFile (sMAXPath / "SCARGO02.FLC", path / "cargoship" / "video.flc"); // cargoship
	copyFile (sMAXPath / "MML.FLC", path / "cluster" / "video.flc"); // cluster
	copyFile (sMAXPath / "AGT.FLC", path / "commando" / "video.flc"); // commando
	copyFile (sMAXPath / "CORVETTE.FLC", path / "corvet" / "video.flc"); // corvet
	copyFile (sMAXPath / "ESCORT.FLC", path / "escort" / "video.flc"); // escort
	copyFile (sMAXPath / "FIGHTER.FLC", path / "fighter" / "video.flc"); // fighter
	copyFile (sMAXPath / "HG1.FLC", path / "gunboat" / "video.flc"); // gunboat
	copyFile (sMAXPath / "INFANTRY.FLC", path / "infantery" / "video.flc"); // infantery
	copyFile (sMAXPath / "CONSTRCT.FLC", path / "konstrukt" / "video.flc"); // konstrukt
	copyFile (sMAXPath / "MINELAY.FLC", path / "minelayer" / "video.flc"); // minelayer
	copyFile (sMAXPath / "MISSLE_L.FLC", path / "missel" / "video.flc"); // missel
	copyFile (sMAXPath / "MB5.FLC", path / "missel_ship" / "video.flc"); // missel_ship
	copyFile (sMAXPath / "MAA.FLC", path / "mobile_aa" / "video.flc"); // mobile_aa
	copyFile (sMAXPath / "ENGINEER.FLC", path / "pionier" / "video.flc"); // pionier
	copyFile (sMAXPath / "REPAIR02.FLC", path / "repair" / "video.flc"); // repair
	copyFile (sMAXPath / "SCANR1.FLC", path / "scanner" / "video.flc"); // scanner
	copyFile (sMAXPath / "SCOUT.FLC", path / "scout" / "video.flc"); // scout
	copyFile (sMAXPath / "SML.FLC", path / "sea_minelayer" / "video.flc"); // sea_minelayer
	copyFile (sMAXPath / "SEATRANS.FLC", path / "sea_transport" / "video.flc"); // sea_transport
	copyFile (sMAXPath / "SUB.FLC", path / "sub" / "video.flc"); // sub
	copyFile (sMAXPath / "SURVEYOR.FLC", path / "surveyor" / "video.flc"); // surveyor
	copyFile (sMAXPath / "TANK03.FLC", path / "tank" / "video.flc"); // tank
	copyFile (sMAXPath / "MCHARGE.FLC", path / "trans_gold" / "video.flc"); // trans_gold
	copyFile (sMAXPath / "TRUCK.FLC", path / "trans_metal" / "video.flc"); // trans_metal
	copyFile (sMAXPath / "MGENR.FLC", path / "trans_oil" / "video.flc"); // trans_oil

	std::cout << "\n";
	std::cout << std::to_string (iErrors) << " errors\n";

	if (logFile != nullptr)
	{
		writeLog (std::string ("Vehicle videos") + TEXT_FILE_LF);
		writeLog (std::to_string (iErrors) + " errors" + TEXT_FILE_LF);
		writeLog (std::string ("========================================================================") + TEXT_FILE_LF);
	}

	return 1;
}

//-------------------------------------------------------------
int installFX()
{
	SDL_Surface *surface, *output;
	iTotalFiles = 9;
	iErrors = 0;
	iInstalledFiles = 0;

#if MAC
	updateProgressWindow ("Fx", iTotalFiles, iInstalledFiles);
#endif

	std::cout << "========================================================================\n";
	std::cout << "Fx\n";

	const auto path = sOutputPath / "fx";

	// waldo
	try
	{
		surface = getImageFromRes ("WALDO");
		resizeSurface (surface, 8, 7, 64, 64);
		savePCX (surface, path / "corpse.pcx");
		SDL_FreeSurface (surface);
	}
	END_INSTALL_FILE (path / "corpse.pcx")

	// hit
	try
	{
		output = getImageFromRes ("HITEXPLD", 0);
		resizeSurface (output, 13, 15, 320, 64);

		SDL_Rect dst_rect = {77, 15, 0, 0};
		for (int i = 1; i < 5; i++)
		{
			surface = getImageFromRes ("HITEXPLD", i);
			SDL_BlitSurface (surface, nullptr, output, &dst_rect);
			SDL_FreeSurface (surface);
			dst_rect.x += 64;
		}

		savePCX (output, path / "hit.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "hit.pcx")

	// explo_air
	try
	{
		output = getImageFromRes ("AIREXPLD", 0);
		resizeSurface (output, 0, 0, 1918, 121);

		SDL_Rect dst_rect = {137, 0, 0, 0};
		for (int i = 1; i < 14; i++)
		{
			surface = getImageFromRes ("AIREXPLD", i);
			SDL_BlitSurface (surface, nullptr, output, &dst_rect);
			SDL_FreeSurface (surface);
			dst_rect.x += 137;
		}

		savePCX (output, path / "explo_air.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "explo_air.pcx")

	// explo_big
	try
	{
		output = getImageFromRes ("BLDEXPLD", 0);
		resizeSurface (output, 0, 0, 8596, 194);

		SDL_Rect dst_rect = {307, 0, 0, 0};
		for (int i = 1; i < 28; i++)
		{
			surface = getImageFromRes ("BLDEXPLD", i);
			SDL_BlitSurface (surface, nullptr, output, &dst_rect);
			SDL_FreeSurface (surface);
			dst_rect.x += 307;
		}

		savePCX (output, path / "explo_big.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "explo_big.pcx")

	// explo_small
	try
	{
		output = getImageFromRes ("LNDEXPLD", 0);
		resizeSurface (output, 0, 0, 1596, 108);

		SDL_Rect dst_rect = {114, 0, 0, 0};
		for (int i = 1; i < 14; i++)
		{
			surface = getImageFromRes ("LNDEXPLD", i);
			SDL_BlitSurface (surface, nullptr, output, &dst_rect);
			SDL_FreeSurface (surface);
			dst_rect.x += 114;
		}

		savePCX (output, path / "explo_small.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "explo_small.pcx")

	// explo_water
	try
	{
		output = getImageFromRes ("SEAEXPLD", 0);
		resizeSurface (output, 0, 0, 1596, 108);

		SDL_Rect dst_rect = {114, 0, 0, 0};
		for (int i = 1; i < 14; i++)
		{
			surface = getImageFromRes ("SEAEXPLD", i);
			SDL_BlitSurface (surface, nullptr, output, &dst_rect);
			SDL_FreeSurface (surface);
			dst_rect.x += 114;
		}

		savePCX (output, path / "explo_water.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "explo_water.pcx")
	// rocket
	try
	{
		output = getImageFromRes ("ROCKET", 0);
		resizeSurface (output, 0, 0, 224, 28);
		SDL_Rect dst_rect = {28, 0, 0, 0};
		for (int i = 1; i < 8; i++)
		{
			surface = getImageFromRes ("ROCKET", 2 * i);
			SDL_BlitSurface (surface, nullptr, output, &dst_rect);
			SDL_FreeSurface (surface);
			dst_rect.x += 28;
		}

		savePCX (output, path / "rocket.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "rocket.pcx")

	// torpedo (for the sup)
	try
	{
		output = getImageFromRes ("TORPEDO", 0);
		resizeSurface (output, 0, 0, 224, 28);
		SDL_Rect dst_rect = {28, 0, 0, 0};
		for (int i = 1; i < 8; i++)
		{
			surface = getImageFromRes ("TORPEDO", 2 * i);
			SDL_BlitSurface (surface, nullptr, output, &dst_rect);
			SDL_FreeSurface (surface);
			dst_rect.x += 28;
		}

		savePCX (output, path / "torpedo.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "torpedo.pcx")

#if 0
// start iMuzzleTypes for alien units
	// alien tank plasma ball
	try
	{
		output = getImageFromRes( "ALIEN TANK PLASMA BALL", 0);
		resizeSurface(output, 0, 0, 224, 28);
		SDL_Rect dst_rect = { 28, 0, 0, 0 };
		for ( int i = 1; i < 8; i++ )
		{
			surface = getImageFromRes("ALIEN TANK PLASMA BALL", 2*i );
			SDL_BlitSurface( surface, nullptr, output, &dst_rect);
			SDL_FreeSurface( surface );
			dst_rect.x += 28;
		}

		savePCX( output, path / "alien_tank_plasma_ball.pcx");
		SDL_FreeSurface( output );

	}
	END_INSTALL_FILE( path / "alien_tank_plasma_ball.pcx" )
	// alien ari plasma ball
	try
	{
		output = getImageFromRes( "ALIEN ARTILLERY PLASMA BALL", 0);
		resizeSurface(output, 0, 0, 224, 28);
		SDL_Rect dst_rect = { 28, 0, 0, 0 };
		for ( int i = 1; i < 8; i++ )
		{
			surface = getImageFromRes("ALIEN ARTILLERY PLASMA BALL", 2*i );
			SDL_BlitSurface( surface, nullptr, output, &dst_rect);
			SDL_FreeSurface( surface );
			dst_rect.x += 28;
		}

		savePCX( output, path / "alien_ari_plasma_ball.pcx");
		SDL_FreeSurface( output );

	}
	END_INSTALL_FILE( path / "alien_ari_plasma_ball.pcx" )
	// alien missle (for alien attack plane)
	try
	{
		output = getImageFromRes( "ALIEN MISSLE", 0);
		resizeSurface(output, 0, 0, 224, 28);
		SDL_Rect dst_rect = { 28, 0, 0, 0 };
		for ( int i = 1; i < 8; i++ )
		{
			surface = getImageFromRes("ALIEN MISSLE", 2*i );
			SDL_BlitSurface( surface, nullptr, output, &dst_rect);
			SDL_FreeSurface( surface );
			dst_rect.x += 28;
		}

		savePCX( output, path / "alien_missle.pcx");
		SDL_FreeSurface( output );

	}
	END_INSTALL_FILE( path / "alien_missle.pcx" )
// end iMuzzleTypes for alien units
#endif
	// saveload.flc
	copyFile (sMAXPath / "SAVELOAD.FLC", path / "saveload.flc");

	if (logFile != nullptr)
	{
		writeLog (std::string ("Fx") + TEXT_FILE_LF);
		writeLog (std::to_string (iErrors) + " errors" + TEXT_FILE_LF);
		writeLog (std::string ("========================================================================") + TEXT_FILE_LF);
	}

	std::cout << "\n";
	std::cout << std::to_string (iErrors) << " errors\n";

	return 1;
}

//-------------------------------------------------------------
int installGfx()
{
	SDL_Surface *surface, *output;
	iTotalFiles = 44;
	iErrors = 0;
	iInstalledFiles = 0;

#if MAC
	updateProgressWindow ("Gfx", iTotalFiles, iInstalledFiles);
#endif

	std::cout << "========================================================================\n";
	std::cout << "Gfx\n";

	const auto path = sOutputPath / "gfx";

	// clan logos
	for (int i = 1; i <= 8; i++)
	{
		const auto szNum = std::to_string (i);
		try
		{
			output = getImageFromRes (std::string ("CLN") + szNum + "LOGO");
			setColor (output, 0, 255, 0, 255);
			savePCX (output, path / ("clanlogo" + szNum + ".pcx"));
			SDL_FreeSurface (output);
		}
		END_INSTALL_FILE (path / ("clanlogo" + szNum + ".pcx"));
	}

	// End game
	for (int i = 1; i <= 9; i++)
	{
		const auto szNum = std::to_string (i);
		try
		{
			output = getImageFromRes ("ENDGAME" + szNum);
			setColor (output, 0, 255, 0, 255);
			savePCX (output, path / "endgame" / ("endgame" + szNum + ".pcx"));
			SDL_FreeSurface (output);
		}
		END_INSTALL_FILE (path / "endgame" / ("endgame" + szNum + ".pcx"));
	}

	// activate
	try
	{
		output = getImageFromRes ("ACTVTPTR");
		setColor (output, 0, 255, 0, 255);
		savePCX (output, path / "activate.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "activate.pcx");

	// attack
	try
	{
		output = getImageFromRes ("ENMY_PTR");
		setColor (output, 77, 255, 0, 255);
		savePCX (output, path / "attack.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "attack.pcx");
	// attack out-of-range
	try
	{
		output = getImageFromRes ("PTR_FTRG");
		setColor (output, 77, 255, 0, 255);
		savePCX (output, path / "attack_oor.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "attack_oor.pcx");

	try
	{
		output = getImageFromRes ("BLDMRK1");
		setColor (output, 0, 255, 0, 255);
		resizeSurface (output, 1, 1, 320, 64);

		surface = getImageFromRes ("BLDMRK2");
		setColor (surface, 0, 255, 0, 255);
		SDL_Rect dst_rect = {65, 1, 0, 0};
		SDL_BlitSurface (surface, nullptr, output, &dst_rect);
		SDL_FreeSurface (surface);

		surface = getImageFromRes ("BLDMRK3");
		setColor (surface, 0, 255, 0, 255);
		dst_rect.x += 64;
		SDL_BlitSurface (surface, nullptr, output, &dst_rect);
		SDL_FreeSurface (surface);

		surface = getImageFromRes ("BLDMRK4");
		setColor (surface, 0, 255, 0, 255);
		dst_rect.x += 64;
		SDL_BlitSurface (surface, nullptr, output, &dst_rect);
		SDL_FreeSurface (surface);

		surface = getImageFromRes ("BLDMRK5");
		setColor (surface, 0, 255, 0, 255);
		dst_rect.x += 64;
		SDL_BlitSurface (surface, nullptr, output, &dst_rect);
		SDL_FreeSurface (surface);

		savePCX (output, path / "activate_field.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "activate_field.pcx");

	// band_big
	try
	{
		surface = getImageFromRes ("LRGTAPE");
		resizeSurface (surface, 0, 0, 128, 128);
		savePCX (surface, path / "band_big.pcx");
		SDL_FreeSurface (surface);
	}
	END_INSTALL_FILE (path / "band_big.pcx");

	// band_big_water
	// copyFileFromRes("LRGTAPE", path / "band_big_water.pcx", 1);

	// band_small
	copyFileFromRes ("SMLTAPE", path / "band_small.pcx", 0);

	// band_small_water
	// copyFileFromRes("SMLTAPE", path / "band_big_water.pcx", 1);

	// band_cur
	try
	{
		output = getImageFromRes ("MAP_PTR");
		setColor (output, 0, 255, 0, 255);
		savePCX (output, path / "band_cur.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "band_cur.pcx");

	// big_beton
	try
	{
		output = getImageFromRes ("LRGSLAB");
		resizeSurface (output, 0, 0, 128, 128);
		savePCX (output, path / "big_beton.pcx");
	}
	END_INSTALL_FILE (path / "big_beton.pcx");

	// disable
	try
	{
		output = getImageFromRes ("DISBLPTR");
		setColor (output, 77, 255, 0, 255);
		savePCX (output, path / "disable.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "disable.pcx");

	// edock
	copyFileFromRes ("E_DOCK", path / "edock.pcx");

	// edepot
	copyFileFromRes ("E_DEPOT", path / "edepot.pcx");

	// ehangar
	copyFileFromRes ("E_HANGAR", path / "ehangar.pcx");

	// hand
	try
	{
		output = getImageFromRes ("HANDPTR");
		setColor (output, 0, 255, 0, 255);
		savePCX (output, path / "hand.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "hand.pcx");

	// help
	try
	{
		output = getImageFromRes ("PTR_HELP");
		setColor (output, 0, 255, 0, 255);
		savePCX (output, path / "help.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "help.pcx");

	// load
	try
	{
		output = getImageFromRes ("FRND_LOD");
		setColor (output, 0, 255, 0, 255);
		savePCX (output, path / "load.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "load.pcx");

	// move
	try
	{
		output = getImageFromRes ("UNIT_GO");
		setColor (output, 0, 255, 0, 255);
		savePCX (output, path / "move.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "move.pcx");
	// move draft
	try
	{
		output = getImageFromRes ("WAY_PTR");
		setColor (output, 0, 255, 0, 255);
		savePCX (output, path / "move_draft.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "move_draft.pcx");

	// muni
	try
	{
		output = getImageFromRes ("PTR_RLD");
		setColor (output, 0, 255, 0, 255);
		savePCX (output, path / "muni.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "muni.pcx");

	// no
	try
	{
		output = getImageFromRes ("UNIT_NGO");
		setColor (output, 0, 255, 0, 255);
		savePCX (output, path / "no.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "no.pcx");

	// object_manu
	try
	{
		output = getImageFromRes ("UNTBTN_U");
		setColor (output, 0, 255, 0, 255);
		resizeSurface (output, 0, 0, 42, 42);

		surface = getImageFromRes ("UNTBTN_D");
		setColor (surface, 0, 255, 0, 255);
		SDL_Rect dst_rect = {0, 21, 0, 0};
		SDL_BlitSurface (surface, nullptr, output, &dst_rect);
		SDL_FreeSurface (surface);

		savePCX (output, path / "object_menu2.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "object_menu2.pcx");

	// pf_x
	try
	{
		output = getImageFromRes ("ARROW_SW");
		setColor (output, 1, 255, 0, 255);
		savePCX (output, path / "pf_1.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "pf_1.pcx");

	try
	{
		output = getImageFromRes ("ARROW_S");
		setColor (output, 1, 255, 0, 255);
		savePCX (output, path / "pf_2.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "pf_2.pcx");
	try
	{
		output = getImageFromRes ("ARROW_SE");
		setColor (output, 1, 255, 0, 255);
		savePCX (output, path / "pf_3.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "pf_3.pcx");
	try
	{
		output = getImageFromRes ("ARROW_E");
		setColor (output, 1, 255, 0, 255);
		savePCX (output, path / "pf_6.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "pf_6.pcx");
	try
	{
		output = getImageFromRes ("ARROW_NE");
		setColor (output, 1, 255, 0, 255);
		savePCX (output, path / "pf_9.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "pf_9.pcx");
	try
	{
		output = getImageFromRes ("ARROW_N");
		setColor (output, 1, 255, 0, 255);
		savePCX (output, path / "pf_8.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "pf_8.pcx");
	try
	{
		output = getImageFromRes ("ARROW_NW");
		setColor (output, 1, 255, 0, 255);
		savePCX (output, path / "pf_7.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "pf_7.pcx");
	try
	{
		output = getImageFromRes ("ARROW_W");
		setColor (output, 1, 255, 0, 255);
		savePCX (output, path / "pf_4.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "pf_4.pcx");

	// transfer
	try
	{
		output = getImageFromRes ("FRND_XFR");
		setColor (output, 0, 255, 0, 255);
		savePCX (output, path / "transf.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "transf.pcx");

	// repair
	try
	{
		output = getImageFromRes ("FRND_FIX");
		setColor (output, 0, 255, 0, 255);
		savePCX (output, path / "repair.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "repair.pcx");

	// steal
	try
	{
		output = getImageFromRes ("STEALPTR");
		setColor (output, 77, 255, 0, 255);
		savePCX (output, path / "steal.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "steal.pcx")

	// select
	try
	{
		output = getImageFromRes ("FRND_PTR");
		setColor (output, 0, 255, 0, 255);
		savePCX (output, path / "select.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "select.pcx")

	// res
	try
	{
		output = getImageFromRes ("RAWMSK0");
		setColor (output, 255, 255, 0, 255);
		resizeSurface (output, 13, 13, 1088, 64);

		for (int i = 1; i < 17; i++)
		{
			surface = getImageFromRes ("RAWMSK" + std::to_string (i));
			setColor (surface, 48, 255, 0, 255);
			SDL_Rect dst_rect = {1 + i * 64, 1, 0, 0};
			SDL_BlitSurface (surface, nullptr, output, &dst_rect);
			SDL_FreeSurface (surface);
		}

		savePCX (output, path / "res.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "res.pcx")

	// fuel
	try
	{
		output = getImageFromRes ("FUELMK0");
		setColor (output, 255, 255, 0, 255);
		resizeSurface (output, 13, 13, 1088, 64);

		for (int i = 1; i < 17; i++)
		{
			surface = getImageFromRes ("FUELMK" + std::to_string (i));
			setColor (surface, 255, 255, 0, 255);
			SDL_Rect dst_rect = {1 + i * 64, 1, 0, 0};
			SDL_BlitSurface (surface, nullptr, output, &dst_rect);
			SDL_FreeSurface (surface);
		}

		savePCX (output, path / "fuel.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "fuel.pcx")

	// gold
	try
	{
		output = getImageFromRes ("GOLDMK0");
		setColor (output, 255, 255, 0, 255);
		resizeSurface (output, 13, 13, 1088, 64);

		for (int i = 1; i < 17; i++)
		{
			surface = getImageFromRes ("GOLDMK" + std::to_string (i));
			setColor (surface, 255, 255, 0, 255);
			SDL_Rect dst_rect = {1 + i * 64, 1, 0, 0};
			SDL_BlitSurface (surface, nullptr, output, &dst_rect);
			SDL_FreeSurface (surface);
		}

		savePCX (output, path / "gold.pcx");
		SDL_FreeSurface (output);
	}
	END_INSTALL_FILE (path / "gold.pcx")

//some unused pointer from original res-file
#if 0
		// blizzard-pointer...
		// maybe useful if teammode available,
		// => mouse over a friends unit -> ident. friend or foe (IFF)
		try
		{
			output = getImageFromRes("FRND_FUE");
			setColor( output, 0, 255, 0, 255 );
			savePCX( output, path / "blizzard.pcx");
			SDL_FreeSurface( output );
		}
		END_INSTALL_FILE( path / "blizzard.pcx" );

		// this could be used for what it is labled for ;)
		try
		{
			output = getImageFromRes("GROUPPTR");
			setColor( output, 0, 255, 0, 255 );
			savePCX( output, path / "select_groupe.pcx");
			SDL_FreeSurface( output );
		}
		END_INSTALL_FILE( path / "select_groupe.pcx" );

		// another funny ptr... maybe usefull for #776
		try
		{
			output = getImageFromRes("PTR_PATH");
			setColor( output, 0, 255, 0, 255 );
			savePCX( output, path / "maze.pcx");
			SDL_FreeSurface( output );
		}
		END_INSTALL_FILE( path / "maze.pcx" );
#endif

	// and now the ugly hud_stuff.pcx :|
#define COPY_GRAPHIC(name, _x, _y) \
 surface = getImageFromRes (name); \
 setColor (surface, backgroundIndex, 255, 0, 255); \
 dst_rect.x = (_x); \
 dst_rect.y = (_y); \
 SDL_BlitSurface (surface, nullptr, output, &dst_rect); \
 SDL_FreeSurface (surface);

	try
	{
		output = loadPCX (path / "hud_stuff.pcx");
		SDL_Rect dst_rect = {0, 0, 0, 0};

		int backgroundIndex = 17;
		COPY_GRAPHIC ("I_HITS", 11, 109);
		backgroundIndex = 0;
		COPY_GRAPHIC ("EI_AMMO", 55, 98);
		COPY_GRAPHIC ("EI_SHOTS", 96, 98);
		COPY_GRAPHIC ("EI_POWER", 81, 98);
		COPY_GRAPHIC ("EI_SPEED", 7, 98);
		COPY_GRAPHIC ("SI_POWER", 74, 98);
		COPY_GRAPHIC ("SI_SPEED", 0, 98);
		COPY_GRAPHIC ("EI_HITSB", 20, 98);
		COPY_GRAPHIC ("EI_HITSR", 44, 98);
		COPY_GRAPHIC ("EI_HITSY", 32, 98);
		COPY_GRAPHIC ("SI_HITSB", 14, 98);
		COPY_GRAPHIC ("SI_HITSR", 38, 98);
		COPY_GRAPHIC ("SI_HITSY", 26, 98);
		COPY_GRAPHIC ("EI_FUEL", 112, 98);
		COPY_GRAPHIC ("EI_WORK", 178, 98);
		COPY_GRAPHIC ("SI_FUEL", 104, 98);
		COPY_GRAPHIC ("SI_WORK", 170, 98);
		COPY_GRAPHIC ("EI_GOLD", 129, 98);
		COPY_GRAPHIC ("SI_GOLD", 120, 98);
		COPY_GRAPHIC ("EI_RAW", 67, 98);
		COPY_GRAPHIC ("SI_RAW", 60, 98);
		COPY_GRAPHIC ("SI_AMMO", 50, 98);
		COPY_GRAPHIC ("I_SHOTS", 37, 109);
		COPY_GRAPHIC ("EI_LAND", 154, 98);
		COPY_GRAPHIC ("SI_LAND", 138, 98);
		COPY_GRAPHIC ("I_AMMO", 18, 109);
		COPY_GRAPHIC ("I_GOLD", 112, 109);
		COPY_GRAPHIC ("I_FUEL", 101, 109);
		COPY_GRAPHIC ("I_SPEED", 0, 109);
		COPY_GRAPHIC ("I_HRDATK", 27, 109);
		COPY_GRAPHIC ("I_ARMOR", 65, 109);
		COPY_GRAPHIC ("EI_AIR", 207, 98);
		COPY_GRAPHIC ("SI_AIR", 186, 98);
		COPY_GRAPHIC ("I_RANGE", 52, 109);
		COPY_GRAPHIC ("I_SCAN", 76, 109);
		COPY_GRAPHIC ("I_RAW", 89, 109);
		COPY_GRAPHIC ("I_RAWE", 175, 109); // big emty raw ("cost")
		COPY_GRAPHIC ("I_LIFE", 138, 109);
		COPY_GRAPHIC ("I_POWER", 125, 109);
		COPY_GRAPHIC ("BARTAPE", 156, 307);
		COPY_GRAPHIC ("IL_SPEED", 244, 98);
		COPY_GRAPHIC ("IL_SHOTS", 254, 98);
		COPY_GRAPHIC ("IL_DSBLD", 150, 109);

		surface = getImageFromRes ("SI_SHOTS");
		setColor (surface, 0, 255, 0, 255);
		setColor (surface, 9, 255, 0, 255);
		dst_rect.x = 88;
		dst_rect.y = 98;
		SDL_BlitSurface (surface, nullptr, output, &dst_rect);
		SDL_FreeSurface (surface);

		SDL_Rect src_rect;

		surface = getImageFromRes ("SMBRFUEL");
		setColor (surface, 0, 255, 0, 255);
		src_rect.x = 119;
		src_rect.y = 0;
		src_rect.h = 16;
		src_rect.w = 223;
		dst_rect.x = 156;
		dst_rect.y = 273;
		SDL_BlitSurface (surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface (surface);

		surface = getImageFromRes ("SMBRGOLD");
		setColor (surface, 0, 255, 0, 255);
		src_rect.x = 119;
		src_rect.y = 0;
		src_rect.h = 16;
		src_rect.w = 223;
		dst_rect.x = 156;
		dst_rect.y = 290;
		SDL_BlitSurface (surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface (surface);

		surface = getImageFromRes ("SMBRRAW");
		setColor (surface, 0, 255, 0, 255);
		src_rect.x = 62;
		src_rect.y = 0;
		src_rect.h = 16;
		src_rect.w = 223;
		dst_rect.x = 156;
		dst_rect.y = 256;
		SDL_BlitSurface (surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface (surface);

		surface = getImageFromRes ("BARRAW");
		setColor (surface, 17, 255, 0, 255);
		src_rect.x = 1;
		src_rect.y = 0;
		src_rect.h = 31;
		src_rect.w = 1000; // ?
		dst_rect.x = 156;
		dst_rect.y = 338;
		SDL_BlitSurface (surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface (surface);

		surface = getImageFromRes ("BARFUEL");
		setColor (surface, 0, 255, 0, 255);
		src_rect.x = 33;
		src_rect.y = 0;
		src_rect.h = 30;
		src_rect.w = 1000; // ?
		dst_rect.x = 156;
		dst_rect.y = 369;
		SDL_BlitSurface (surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface (surface);

		surface = getImageFromRes ("BARGOLD");
		setColor (surface, 0, 255, 0, 255);
		src_rect.x = 1;
		src_rect.y = 0;
		src_rect.h = 30;
		src_rect.w = 1000; // ?
		dst_rect.x = 156;
		dst_rect.y = 400;
		SDL_BlitSurface (surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface (surface);

		surface = getImageFromRes ("VERTRAW");
		setColor (surface, 0, 255, 0, 255);
		src_rect.x = 0;
		src_rect.y = 0;
		src_rect.h = 115;
		src_rect.w = 20;
		dst_rect.x = 135;
		dst_rect.y = 336;
		SDL_BlitSurface (surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface (surface);

		surface = getImageFromRes ("VERTGOLD");
		setColor (surface, 0, 255, 0, 255);
		src_rect.w = 20;
		src_rect.h = 115;
		src_rect.x = 0;
		src_rect.y = 0;
		dst_rect.x = 114;
		dst_rect.y = 336;
		SDL_BlitSurface (surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface (surface);

		surface = getImageFromRes ("VERTFUEL");
		setColor (surface, 0, 255, 0, 255);
		src_rect.w = 20;
		src_rect.h = 115;
		src_rect.x = 0;
		src_rect.y = 0;
		dst_rect.x = 400;
		dst_rect.y = 348;
		SDL_BlitSurface (surface, &src_rect, output, &dst_rect);
		SDL_FreeSurface (surface);

		savePCX (output, path / "hud_stuff.pcx");
	}
	END_INSTALL_FILE (path / "hud_stuff.pcx");

	if (logFile != nullptr)
	{
		writeLog (std::string ("Gfx") + TEXT_FILE_LF);
		writeLog (std::to_string (iErrors) + " errors" + TEXT_FILE_LF);
		writeLog (std::string ("========================================================================") + TEXT_FILE_LF);
	}

	std::cout << "\n";
	std::cout << std::to_string (iErrors) << " errors\n";

	return 1;
}

//-------------------------------------------------------------
int installBuildingSounds()
{
	iTotalFiles = 46;
	iErrors = 0;
	iInstalledFiles = 0;
	oggEncode = 1;

#if MAC
	updateProgressWindow ("Building sounds", iTotalFiles, iInstalledFiles);
#endif

	std::cout << "========================================================================\n";
	std::cout << "Building sounds\n";

	// energy big
	auto path = sOutputPath / "buildings" / "energy_big";
	copyPartOfWAV (sMAXPath / ("POWGN17" + waveExtension), path / "start.wav", 0);
	copyPartOfWAV (sMAXPath / ("POWGN17" + waveExtension), path / "running.wav", 1);
	copyWAV (sMAXPath / ("POWGN18" + waveExtension), path / "stop.wav");

	// energy small
	path = sOutputPath / "buildings" / "energy_small";
	copyPartOfWAV (sMAXPath / ("POWGN17" + waveExtension), path / "start.wav", 0);
	copyPartOfWAV (sMAXPath / ("POWGN17" + waveExtension), path / "running.wav", 1);
	copyWAV (sMAXPath / ("POWGN18" + waveExtension), path / "stop.wav");

	// fac air
	path = sOutputPath / "buildings" / "fac_air";
	copyPartOfWAV (sMAXPath / ("AUNIT17" + waveExtension), path / "start.wav", 0);
	copyPartOfWAV (sMAXPath / ("AUNIT17" + waveExtension), path / "running.wav", 1);
	copyWAV (sMAXPath / ("AUNIT18" + waveExtension), path / "stop.wav");

	// fac alien
	path = sOutputPath / "buildings" / "fac_alien";
	copyPartOfWAV (sMAXPath / ("LVP17" + waveExtension), path / "start.wav", 0);
	copyPartOfWAV (sMAXPath / ("LVP17" + waveExtension), path / "running.wav", 1);
	copyWAV (sMAXPath / ("LVP18" + waveExtension), path / "stop.wav");

	// fac big
	path = sOutputPath / "buildings" / "fac_big";
	copyPartOfWAV (sMAXPath / ("HVP17" + waveExtension), path / "start.wav", 0);
	copyPartOfWAV (sMAXPath / ("HVP17" + waveExtension), path / "running.wav", 1);
	copyWAV (sMAXPath / ("HVP18" + waveExtension), path / "stop.wav");

	// fac ship
	path = sOutputPath / "buildings" / "fac_ship";
	copyPartOfWAV (sMAXPath / ("HVP17" + waveExtension), path / "start.wav", 0);
	copyPartOfWAV (sMAXPath / ("HVP17" + waveExtension), path / "running.wav", 1);
	copyWAV (sMAXPath / ("HVP18" + waveExtension), path / "stop.wav");

	// fac small
	path = sOutputPath / "buildings" / "fac_small";
	copyPartOfWAV (sMAXPath / ("LVP17" + waveExtension), path / "start.wav", 0);
	copyPartOfWAV (sMAXPath / ("LVP17" + waveExtension), path / "running.wav", 1);
	copyWAV (sMAXPath / ("LVP18" + waveExtension), path / "stop.wav");

	// goldraff
	// goldraff is using the monopole-mine sound in original game - nonsinn
	path = sOutputPath / "buildings" / "goldraff";
	copyPartOfWAV (sMAXPath / ("MONOP17" + waveExtension), path / "start.wav", 0);
	copyPartOfWAV (sMAXPath / ("MONOP17" + waveExtension), path / "running.wav", 1);
	copyWAV (sMAXPath / ("MONOP18" + waveExtension), path / "stop.wav");

	// gun aa
	path = sOutputPath / "buildings" / "gun_aa";
	copyWAV (sMAXPath / ("FANTI14" + waveExtension), path / "attack.wav");

	// gun ari
	path = sOutputPath / "buildings" / "gun_ari";
	copyWAV (sMAXPath / ("FARTY14" + waveExtension), path / "attack.wav");

	// gun missile
	path = sOutputPath / "buildings" / "gun_missel";
	copyWAV (sMAXPath / ("MISLFIRE" + waveExtension), path / "attack.wav");

	// gun turret
	path = sOutputPath / "buildings" / "gun_turret";
	copyWAV (sMAXPath / ("FGUN14" + waveExtension), path / "attack.wav");

	// landmine
	path = sOutputPath / "buildings" / "landmine";
	copyWAV (sMAXPath / ("EXPSDIRT" + waveExtension), path / "attack.wav");

	// mine
	path = sOutputPath / "buildings" / "mine";
	copyPartOfWAV (sMAXPath / ("HVP17" + waveExtension), path / "start.wav", 0);
	copyPartOfWAV (sMAXPath / ("HVP17" + waveExtension), path / "running.wav", 1);
	copyWAV (sMAXPath / ("MSTAT18" + waveExtension), path / "stop.wav");

	// ecosphere
	path = sOutputPath / "buildings" / "ecosphere";
	copyPartOfWAV (sMAXPath / ("HVP17" + waveExtension), path / "start.wav", 0);
	copyPartOfWAV (sMAXPath / ("HVP17" + waveExtension), path / "running.wav", 1);
	copyWAV (sMAXPath / ("MSTAT18" + waveExtension), path / "stop.wav");

	// mine deep (monopol-mine)
	// while monopole sound is used for goldraff,
	// we can use the goldraff sound for monopol mine
	//  => both have a different sound - nonsinn
	path = sOutputPath / "buildings" / "mine_deep";
	copyPartOfWAV (sMAXPath / ("GOLDR17" + waveExtension), path / "start.wav", 0);
	copyPartOfWAV (sMAXPath / ("GOLDR17" + waveExtension), path / "running.wav", 1);
	copyWAV (sMAXPath / ("GOLDR18" + waveExtension), path / "stop.wav");

	// radar
	path = sOutputPath / "buildings" / "radar";
	copyWAV (sMAXPath / ("RADAR13" + waveExtension), path / "wait.wav");

	// research
	path = sOutputPath / "buildings" / "research";
	copyPartOfWAV (sMAXPath / ("RESEAR17" + waveExtension), path / "start.wav", 0);
	copyPartOfWAV (sMAXPath / ("RESEAR17" + waveExtension), path / "running.wav", 1);
	copyWAV (sMAXPath / ("RESEAR18" + waveExtension), path / "stop.wav");

	// seamine
	path = sOutputPath / "buildings" / "seamine";
	copyWAV (sMAXPath / ("EPLOWET1" + waveExtension), path / "attack.wav");

	// shield
	// path = sOutputPath / "buildings" / "shield";

	// training
	path = sOutputPath / "buildings" / "training";
	copyPartOfWAV (sMAXPath / ("LVP17" + waveExtension), path / "start.wav", 0);
	copyPartOfWAV (sMAXPath / ("LVP17" + waveExtension), path / "running.wav", 1);
	copyWAV (sMAXPath / ("LVP18" + waveExtension), path / "stop.wav");

	if (logFile != nullptr)
	{
		writeLog (std::string ("Building sounds") + TEXT_FILE_LF);
		writeLog (std::to_string (iErrors) + " errors" + TEXT_FILE_LF);
		writeLog (std::string ("========================================================================") + TEXT_FILE_LF);
	}

	std::cout << "\n";
	std::cout << std::to_string (iErrors) << " errors\n";
	return 1;
}

//-------------------------------------------------------------
int installVehicleSounds()
{
	iTotalFiles = 178;
	iErrors = 0;
	iInstalledFiles = 0;
	oggEncode = 1;

#if MAC
	updateProgressWindow ("Vehicle sounds", iTotalFiles, iInstalledFiles);
#endif

	std::cout << "========================================================================\n";
	std::cout << "Vehicle Sounds\n";

	// air_transport
	auto path = sOutputPath / "vehicles" / "air_transport";
	copyWAV (sMAXPath / ("ATRANS1" + waveExtension), path / "wait.wav");
	copyPartOfWAV (sMAXPath / ("ATRANS5" + waveExtension), path / "start.wav", 0);
	copyPartOfWAV (sMAXPath / ("ATRANS5" + waveExtension), path / "drive.wav", 1);
	copyWAV (sMAXPath / ("ATRANS7" + waveExtension), path / "stop.wav");

	// alien_assault
	path = sOutputPath / "vehicles" / "alien_assault";
	copyWAV (sMAXPath / ("ALNTK1" + waveExtension), path / "wait.wav");
	copyPartOfWAV (sMAXPath / ("ALNTK5" + waveExtension), path / "start.wav", 0);
	copyPartOfWAV (sMAXPath / ("ALNTK5" + waveExtension), path / "drive.wav", 1);
	copyWAV (sMAXPath / ("ALNTK7" + waveExtension), path / "stop.wav");
	copyWAV (sMAXPath / ("ASGUN14" + waveExtension), path / "attack.wav");

	// alien_plane
	path = sOutputPath / "vehicles" / "alien_plane";
	copyWAV (sMAXPath / ("ATTACK1" + waveExtension), path / "wait.wav");
	copyPartOfWAV (sMAXPath / ("ATTACK5" + waveExtension), path / "start.wav", 0);
	copyPartOfWAV (sMAXPath / ("ATTACK5" + waveExtension), path / "drive.wav", 1);
	copyWAV (sMAXPath / ("ATTACK7" + waveExtension), path / "stop.wav");
	copyWAV (sMAXPath / ("ASGUN14" + waveExtension), path / "attack.wav");

	// alien_ship
	path = sOutputPath / "vehicles" / "alien_ship";
	copyWAV (sMAXPath / ("JUGGR1" + waveExtension), path / "wait_water.wav");
	copyPartOfWAV (sMAXPath / ("JUGGR5" + waveExtension), path / "start_water.wav", 0);
	copyPartOfWAV (sMAXPath / ("JUGGR5" + waveExtension), path / "drive_water.wav", 1);
	copyWAV (sMAXPath / ("JUGGR7" + waveExtension), path / "stop_water.wav");
	copyWAV (sMAXPath / ("ASGUN14" + waveExtension), path / "attack.wav");

	// alien_tank
	path = sOutputPath / "vehicles" / "alien_tank";
	copyWAV (sMAXPath / ("ALNTK1" + waveExtension), path / "wait.wav");
	copyPartOfWAV (sMAXPath / ("ALNTK5" + waveExtension), path / "start.wav", 0);
	copyPartOfWAV (sMAXPath / ("ALNTK5" + waveExtension), path / "drive.wav", 1);
	copyWAV (sMAXPath / ("ALNTK7" + waveExtension), path / "stop.wav");
	copyWAV (sMAXPath / ("SCOUT14" + waveExtension), path / "attack.wav");

	// apc
	path = sOutputPath / "vehicles" / "apc";
	copyWAV (sMAXPath / ("APC1" + waveExtension), path / "wait.wav");
	copyPartOfWAV (sMAXPath / ("APC5" + waveExtension), path / "start.wav", 0);
	copyPartOfWAV (sMAXPath / ("APC5" + waveExtension), path / "drive.wav", 1);
	copyWAV (sMAXPath / ("APC7" + waveExtension), path / "stop.wav");
	copyWAV (sMAXPath / ("APC1" + waveExtension), path / "wait_water.wav");
	copyPartOfWAV (sMAXPath / ("APC5" + waveExtension), path / "start_water.wav", 0);
	copyPartOfWAV (sMAXPath / ("APC5" + waveExtension), path / "drive_water.wav", 1);
	copyWAV (sMAXPath / ("APC7" + waveExtension), path / "stop_water.wav");

	// assault
	path = sOutputPath / "vehicles" / "assault";
	copyWAV (sMAXPath / ("TANKA_1" + waveExtension), path / "wait.wav");
	copyPartOfWAV (sMAXPath / ("TANKA_5" + waveExtension), path / "start.wav", 0);
	copyPartOfWAV (sMAXPath / ("TANKA_5" + waveExtension), path / "drive.wav", 1);
	copyWAV (sMAXPath / ("TANKA_7" + waveExtension), path / "stop.wav");
	copyWAV (sMAXPath / ("ASGUN14" + waveExtension), path / "attack.wav");

	// awac
	path = sOutputPath / "vehicles" / "awac";
	copyWAV (sMAXPath / ("AWAC1" + waveExtension), path / "wait.wav");
	copyPartOfWAV (sMAXPath / ("AWAC5" + waveExtension), path / "start.wav", 0);
	copyPartOfWAV (sMAXPath / ("AWAC5" + waveExtension), path / "drive.wav", 1);
	copyWAV (sMAXPath / ("AWAC7" + waveExtension), path / "stop.wav");

	// bomber
	path = sOutputPath / "vehicles" / "bomber";
	copyWAV (sMAXPath / ("ATTACK1" + waveExtension), path / "wait.wav");
	copyPartOfWAV (sMAXPath / ("ATTACK5" + waveExtension), path / "start.wav", 0);
	copyPartOfWAV (sMAXPath / ("ATTACK5" + waveExtension), path / "drive.wav", 1);
	copyWAV (sMAXPath / ("ATTACK7" + waveExtension), path / "stop.wav");
	copyWAV (sMAXPath / ("SCOUT14" + waveExtension), path / "attack.wav");

	// bulldozer
	path = sOutputPath / "vehicles" / "bulldozer";
	copyWAV (sMAXPath / ("BULL1" + waveExtension), path / "wait.wav");
	copyPartOfWAV (sMAXPath / ("BULL5" + waveExtension), path / "start.wav", 0);
	copyPartOfWAV (sMAXPath / ("BULL5" + waveExtension), path / "drive.wav", 1);
	copyWAV (sMAXPath / ("BULL7" + waveExtension), path / "stop.wav");

	// cargoship
	path = sOutputPath / "vehicles" / "cargoship";
	copyWAV (sMAXPath / ("MBOATIDL" + waveExtension), path / "wait_water.wav");
	copyPartOfWAV (sMAXPath / ("MBOATMVE" + waveExtension), path / "start_water.wav", 0);
	copyPartOfWAV (sMAXPath / ("MBOATMVE" + waveExtension), path / "drive_water.wav", 1);
	copyWAV (sMAXPath / ("MBOATSTP" + waveExtension), path / "stop_water.wav");

	// cluster
	path = sOutputPath / "vehicles" / "cluster";
	copyWAV (sMAXPath / ("TANKC_1" + waveExtension), path / "wait.wav");
	copyPartOfWAV (sMAXPath / ("TANKC_5" + waveExtension), path / "start.wav", 0);
	copyPartOfWAV (sMAXPath / ("TANKC_5" + waveExtension), path / "drive.wav", 1);
	copyWAV (sMAXPath / ("TANKC_7" + waveExtension), path / "stop.wav");
	copyWAV (sMAXPath / ("MISLFIRE" + waveExtension), path / "attack.wav");

	// commando
	path = sOutputPath / "vehicles" / "commando";
	copyWAV (sMAXPath / ("MANMOVE" + waveExtension), path / "drive.wav");
	copyWAV (sMAXPath / ("INFIL14" + waveExtension), path / "attack.wav");
	copyWAV (sMAXPath / ("INFIL15" + waveExtension), path / "death1.wav");
	copyWAV (sMAXPath / ("INFIL16" + waveExtension), path / "death2.wav");

	// corvet
	path = sOutputPath / "vehicles" / "corvet";
	copyWAV (sMAXPath / ("SBOATIDL" + waveExtension), path / "wait_water.wav");
	copyPartOfWAV (sMAXPath / ("SBOATMVE" + waveExtension), path / "start_water.wav", 0);
	copyPartOfWAV (sMAXPath / ("SBOATMVE" + waveExtension), path / "drive_water.wav", 1);
	copyWAV (sMAXPath / ("SBOATSTP" + waveExtension), path / "stop_water.wav");
	copyWAV (sMAXPath / ("CORVT14" + waveExtension), path / "attack.wav");

	// escort
	path = sOutputPath / "vehicles" / "escort";
	copyWAV (sMAXPath / ("SBOATIDL" + waveExtension), path / "wait_water.wav");
	copyPartOfWAV (sMAXPath / ("SBOATMVE" + waveExtension), path / "start_water.wav", 0);
	copyPartOfWAV (sMAXPath / ("SBOATMVE" + waveExtension), path / "drive_water.wav", 1);
	copyWAV (sMAXPath / ("SBOATSTP" + waveExtension), path / "stop_water.wav");
	copyWAV (sMAXPath / ("MANTI14" + waveExtension), path / "attack.wav");

	// fighter
	path = sOutputPath / "vehicles" / "fighter";
	copyWAV (sMAXPath / ("FIGHT1" + waveExtension), path / "wait.wav");
	copyPartOfWAV (sMAXPath / ("FIGHT5" + waveExtension), path / "start.wav", 0);
	copyPartOfWAV (sMAXPath / ("FIGHT5" + waveExtension), path / "drive.wav", 1);
	copyWAV (sMAXPath / ("FIGHT7" + waveExtension), path / "stop.wav");
	copyWAV (sMAXPath / ("SCOUT14" + waveExtension), path / "attack.wav");

	// gunboat
	path = sOutputPath / "vehicles" / "gunboat";
	copyWAV (sMAXPath / ("GBOATIDL" + waveExtension), path / "wait_water.wav");
	copyPartOfWAV (sMAXPath / ("GBOATMVE" + waveExtension), path / "start_water.wav", 0);
	copyPartOfWAV (sMAXPath / ("GBOATMVE" + waveExtension), path / "drive_water.wav", 1);
	copyWAV (sMAXPath / ("GBOATSTP" + waveExtension), path / "stop_water.wav");
	copyWAV (sMAXPath / ("CANFIRE" + waveExtension), path / "attack.wav");

	// infantery
	path = sOutputPath / "vehicles" / "infantery";
	copyWAV (sMAXPath / ("MANMOVE" + waveExtension), path / "drive.wav");
	copyWAV (sMAXPath / ("INFAN14" + waveExtension), path / "attack.wav");
	copyWAV (sMAXPath / ("INFAN15" + waveExtension), path / "death1.wav");
	copyWAV (sMAXPath / ("INFAN16" + waveExtension), path / "death2.wav");

	// konstrukt
	path = sOutputPath / "vehicles" / "konstrukt";
	copyWAV (sMAXPath / ("CONST1" + waveExtension), path / "wait.wav");
	copyPartOfWAV (sMAXPath / ("CONST5" + waveExtension), path / "start.wav", 0);
	copyPartOfWAV (sMAXPath / ("CONST5" + waveExtension), path / "drive.wav", 1);
	copyWAV (sMAXPath / ("CONST7" + waveExtension), path / "stop.wav");
	copyWAV (sMAXPath / ("CONST2" + waveExtension), path / "wait_water.wav");
	copyPartOfWAV (sMAXPath / ("CONST6" + waveExtension), path / "start_water.wav", 0);
	copyPartOfWAV (sMAXPath / ("CONST6" + waveExtension), path / "drive_water.wav", 1);
	copyWAV (sMAXPath / ("CONST8" + waveExtension), path / "stop_water.wav");

	// minelayer
	path = sOutputPath / "vehicles" / "minelayer";
	copyWAV (sMAXPath / ("TANKC_1" + waveExtension), path / "wait.wav");
	copyPartOfWAV (sMAXPath / ("TANKC_5" + waveExtension), path / "start.wav", 0);
	copyPartOfWAV (sMAXPath / ("TANKC_5" + waveExtension), path / "drive.wav", 1);
	copyWAV (sMAXPath / ("TANKC_7" + waveExtension), path / "stop.wav");

	// missel
	path = sOutputPath / "vehicles" / "missel";
	copyWAV (sMAXPath / ("TANKC_1" + waveExtension), path / "wait.wav");
	copyPartOfWAV (sMAXPath / ("TANKC_5" + waveExtension), path / "start.wav", 0);
	copyPartOfWAV (sMAXPath / ("TANKC_5" + waveExtension), path / "drive.wav", 1);
	copyWAV (sMAXPath / ("TANKC_7" + waveExtension), path / "stop.wav");
	copyWAV (sMAXPath / ("MISLFIRE" + waveExtension), path / "attack.wav");

	// missel_ship
	path = sOutputPath / "vehicles" / "missel_ship";
	copyWAV (sMAXPath / ("MBOATIDL" + waveExtension), path / "wait_water.wav");
	copyPartOfWAV (sMAXPath / ("MBOATMVE" + waveExtension), path / "start_water.wav", 0);
	copyPartOfWAV (sMAXPath / ("MBOATMVE" + waveExtension), path / "drive_water.wav", 1);
	copyWAV (sMAXPath / ("MBOATSTP" + waveExtension), path / "stop_water.wav");
	copyWAV (sMAXPath / ("MSLCR14" + waveExtension), path / "attack.wav");

	// mobile_aa
	path = sOutputPath / "vehicles" / "mobile_aa";
	copyWAV (sMAXPath / ("APC1" + waveExtension), path / "wait.wav");
	copyPartOfWAV (sMAXPath / ("APC5" + waveExtension), path / "start.wav", 0);
	copyPartOfWAV (sMAXPath / ("APC5" + waveExtension), path / "drive.wav", 1);
	copyWAV (sMAXPath / ("APC7" + waveExtension), path / "stop.wav");
	copyWAV (sMAXPath / ("MANTI14" + waveExtension), path / "attack.wav");

	// pionier
	path = sOutputPath / "vehicles" / "pionier";
	copyWAV (sMAXPath / ("ENGIN1" + waveExtension), path / "wait.wav");
	copyPartOfWAV (sMAXPath / ("ENGIN5" + waveExtension), path / "start.wav", 0);
	copyPartOfWAV (sMAXPath / ("ENGIN5" + waveExtension), path / "drive.wav", 1);
	copyWAV (sMAXPath / ("ENGIN7" + waveExtension), path / "stop.wav");
	copyWAV (sMAXPath / ("ENGIN2" + waveExtension), path / "wait_water.wav");
	copyPartOfWAV (sMAXPath / ("ENGIN6" + waveExtension), path / "start_water.wav", 0);
	copyPartOfWAV (sMAXPath / ("ENGIN6" + waveExtension), path / "drive_water.wav", 1);
	copyWAV (sMAXPath / ("ENGIN8" + waveExtension), path / "stop_water.wav");

	// repair
	path = sOutputPath / "vehicles" / "repair";
	copyWAV (sMAXPath / ("REPAIR1" + waveExtension), path / "wait.wav");
	copyPartOfWAV (sMAXPath / ("REPAIR5" + waveExtension), path / "start.wav", 0);
	copyPartOfWAV (sMAXPath / ("REPAIR5" + waveExtension), path / "drive.wav", 1);
	copyWAV (sMAXPath / ("REPAIR7" + waveExtension), path / "stop.wav");

	// scanner
	path = sOutputPath / "vehicles" / "scanner";
	copyWAV (sMAXPath / ("SCAN1" + waveExtension), path / "wait.wav");
	copyPartOfWAV (sMAXPath / ("SCAN5" + waveExtension), path / "start.wav", 0);
	copyPartOfWAV (sMAXPath / ("SCAN5" + waveExtension), path / "drive.wav", 1);
	copyWAV (sMAXPath / ("SCAN7" + waveExtension), path / "stop.wav");

	// scout
	path = sOutputPath / "vehicles" / "scout";
	copyWAV (sMAXPath / ("SCOUT1" + waveExtension), path / "wait.wav");
	copyPartOfWAV (sMAXPath / ("SCOUT5" + waveExtension), path / "start.wav", 0);
	copyPartOfWAV (sMAXPath / ("SCOUT5" + waveExtension), path / "drive.wav", 1);
	copyWAV (sMAXPath / ("SCOUT7" + waveExtension), path / "stop.wav");
	copyWAV (sMAXPath / ("SCOUT2" + waveExtension), path / "wait_water.wav");
	copyPartOfWAV (sMAXPath / ("SCOUT6" + waveExtension), path / "start_water.wav", 0);
	copyPartOfWAV (sMAXPath / ("SCOUT6" + waveExtension), path / "drive_water.wav", 1);
	copyWAV (sMAXPath / ("SCOUT8" + waveExtension), path / "stop_water.wav");
	copyWAV (sMAXPath / ("SCOUT14" + waveExtension), path / "attack.wav");

	// sea_minelayer
	path = sOutputPath / "vehicles" / "sea_minelayer";
	copyWAV (sMAXPath / ("GBOATIDL" + waveExtension), path / "wait_water.wav");
	copyPartOfWAV (sMAXPath / ("GBOATMVE" + waveExtension), path / "start_water.wav", 0);
	copyPartOfWAV (sMAXPath / ("GBOATMVE" + waveExtension), path / "drive_water.wav", 1);
	copyWAV (sMAXPath / ("GBOATSTP" + waveExtension), path / "stop_water.wav");

	// sea_transport
	path = sOutputPath / "vehicles" / "sea_transport";
	copyWAV (sMAXPath / ("GBOATIDL" + waveExtension), path / "wait_water.wav");
	copyPartOfWAV (sMAXPath / ("GBOATMVE" + waveExtension), path / "start_water.wav", 0);
	copyPartOfWAV (sMAXPath / ("GBOATMVE" + waveExtension), path / "drive_water.wav", 1);
	copyWAV (sMAXPath / ("GBOATSTP" + waveExtension), path / "stop_water.wav");

	// sub
	path = sOutputPath / "vehicles" / "sub";
	copyWAV (sMAXPath / ("SUB2" + waveExtension), path / "wait_water.wav");
	copyPartOfWAV (sMAXPath / ("SUB6" + waveExtension), path / "start_water.wav", 0);
	copyPartOfWAV (sMAXPath / ("SUB6" + waveExtension), path / "drive_water.wav", 1);
	copyWAV (sMAXPath / ("SUB8" + waveExtension), path / "stop_water.wav");
	copyWAV (sMAXPath / ("SUB14" + waveExtension), path / "attack.wav");
	copyWAV (sMAXPath / ("SUB16" + waveExtension), path / "death1.wav");

	// surveyor
	path = sOutputPath / "vehicles" / "surveyor";
	copyWAV (sMAXPath / ("SURVY1" + waveExtension), path / "wait.wav");
	copyPartOfWAV (sMAXPath / ("SURVY5" + waveExtension), path / "start.wav", 0);
	copyPartOfWAV (sMAXPath / ("SURVY5" + waveExtension), path / "drive.wav", 1);
	copyWAV (sMAXPath / ("SURVY7" + waveExtension), path / "stop.wav");
	copyWAV (sMAXPath / ("SURVY2" + waveExtension), path / "wait_water.wav");
	copyPartOfWAV (sMAXPath / ("SURVY6" + waveExtension), path / "start_water.wav", 0);
	copyPartOfWAV (sMAXPath / ("SURVY6" + waveExtension), path / "drive_water.wav", 1);
	copyWAV (sMAXPath / ("SURVY8" + waveExtension), path / "stop_water.wav");

	// tank
	path = sOutputPath / "vehicles" / "tank";
	copyWAV (sMAXPath / ("TANK1" + waveExtension), path / "wait.wav");
	copyPartOfWAV (sMAXPath / ("TANK5" + waveExtension), path / "start.wav", 0);
	copyPartOfWAV (sMAXPath / ("TANK5" + waveExtension), path / "drive.wav", 1);
	copyWAV (sMAXPath / ("TANK7" + waveExtension), path / "stop.wav");
	copyWAV (sMAXPath / ("CANFIRE" + waveExtension), path / "attack.wav");

	// trans_gold
	path = sOutputPath / "vehicles" / "trans_gold";
	copyWAV (sMAXPath / ("TRUCKIDL" + waveExtension), path / "wait.wav");
	copyPartOfWAV (sMAXPath / ("TRUCKMVE" + waveExtension), path / "start.wav", 0);
	copyPartOfWAV (sMAXPath / ("TRUCKMVE" + waveExtension), path / "drive.wav", 1);
	copyWAV (sMAXPath / ("TRUCKSTP" + waveExtension), path / "stop.wav");

	// trans_metal
	path = sOutputPath / "vehicles" / "trans_metal";
	copyWAV (sMAXPath / ("TRUCKIDL" + waveExtension), path / "wait.wav");
	copyPartOfWAV (sMAXPath / ("TRUCKMVE" + waveExtension), path / "start.wav", 0);
	copyPartOfWAV (sMAXPath / ("TRUCKMVE" + waveExtension), path / "drive.wav", 1);
	copyWAV (sMAXPath / ("TRUCKSTP" + waveExtension), path / "stop.wav");

	// trans_oil
	path = sOutputPath / "vehicles" / "trans_oil";
	copyWAV (sMAXPath / ("TRUCKIDL" + waveExtension), path / "wait.wav");
	copyPartOfWAV (sMAXPath / ("TRUCKMVE" + waveExtension), path / "start.wav", 0);
	copyPartOfWAV (sMAXPath / ("TRUCKMVE" + waveExtension), path / "drive.wav", 1);
	copyWAV (sMAXPath / ("TRUCKSTP" + waveExtension), path / "stop.wav");

	if (logFile != nullptr)
	{
		writeLog (std::string ("Vehicle sounds") + TEXT_FILE_LF);
		writeLog (std::to_string (iErrors) + " errors" + TEXT_FILE_LF);
		writeLog (std::string ("========================================================================") + TEXT_FILE_LF);
	}

	std::cout << "\n";
	std::cout << std::to_string (iErrors) << " errors\n";
	return 1;
}

//-------------------------------------------------------------
void installVoices()
{
	iTotalFiles = 65;
	iErrors = 0;
	iInstalledFiles = 0;
	oggEncode = 1;

#if MAC
	updateProgressWindow ("Voices", iTotalFiles, iInstalledFiles);
#endif

	std::cout << "========================================================================\n";
	std::cout << "Voices\n";

	auto path = sOutputPath / "voices";
	std::string waveExt = ".wav";
	copyWAV (sVoicePath / ("F001" + waveExtension), path / ("ok1" + waveExt));
	copyWAV (sVoicePath / ("F004" + waveExtension), path / ("ok2" + waveExt));
	copyWAV (sVoicePath / ("F006" + waveExtension), path / ("ok3" + waveExt));
	copyWAV (sVoicePath / ("F005" + waveExtension), path / ("ok4" + waveExt));
	copyWAV (sVoicePath / ("F007" + waveExtension), path / ("commando_failed1" + waveExt));
	copyWAV (sVoicePath / ("F010" + waveExtension), path / ("commando_failed2" + waveExt));
	copyWAV (sVoicePath / ("F012" + waveExtension), path / ("commando_failed3" + waveExt));
	copyWAV (sVoicePath / ("F013" + waveExtension), path / ("saved" + waveExt));
	copyWAV (sVoicePath / ("F053" + waveExtension), path / ("start_none" + waveExt));
	copyWAV (sVoicePath / ("F070" + waveExtension), path / ("detected1" + waveExt));
	copyWAV (sVoicePath / ("F071" + waveExtension), path / ("detected2" + waveExt));
	copyWAV (sVoicePath / ("F085" + waveExtension), path / ("reammo" + waveExt));
	copyWAV (sVoicePath / ("F089" + waveExtension), path / ("reammo_all" + waveExt));
	copyWAV (sVoicePath / ("F093" + waveExtension), path / ("research_complete" + waveExt));
	copyWAV (sVoicePath / ("F094" + waveExtension), path / ("no_path1" + waveExt));
	copyWAV (sVoicePath / ("F095" + waveExtension), path / ("no_path2" + waveExt));
	copyWAV (sVoicePath / ("F145" + waveExtension), path / ("no_speed" + waveExt));
	copyWAV (sVoicePath / ("F150" + waveExtension), path / ("status_yellow1" + waveExt));
	copyWAV (sVoicePath / ("F151" + waveExtension), path / ("status_yellow2" + waveExt));
	copyWAV (sVoicePath / ("F154" + waveExtension), path / ("status_red1" + waveExt));
	copyWAV (sVoicePath / ("F155" + waveExtension), path / ("status_red2" + waveExt));
	copyWAV (sVoicePath / ("F158" + waveExtension), path / ("sentry" + waveExt));
	copyWAV (sVoicePath / ("F162" + waveExtension), path / ("build_done1" + waveExt)); // for pio + constr
	copyWAV (sVoicePath / ("F165" + waveExtension), path / ("build_done2" + waveExt)); // for pio + constr
	copyWAV (sVoicePath / ("F166" + waveExtension), path / ("start_one" + waveExt)); // unit completed (in fac.)
	copyWAV (sVoicePath / ("F169" + waveExtension), path / ("build_done3" + waveExt)); // for factories
	copyWAV (sVoicePath / ("F216" + waveExtension), path / ("build_done4" + waveExt)); // for factories
	copyWAV (sVoicePath / ("F171" + waveExtension), path / ("clearing" + waveExt));
	copyWAV (sVoicePath / ("F181" + waveExtension), path / ("laying_mines" + waveExt));
	copyWAV (sVoicePath / ("F186" + waveExtension), path / ("clearing_mines2" + waveExt));
	copyWAV (sVoicePath / ("F187" + waveExtension), path / ("clearing_mines" + waveExt));
	copyWAV (sVoicePath / ("F191" + waveExtension), path / ("surveying" + waveExt));
	copyWAV (sVoicePath / ("F192" + waveExtension), path / ("surveying2" + waveExt));
	copyWAV (sVoicePath / ("F196" + waveExtension), path / ("attacking1" + waveExt));
	copyWAV (sVoicePath / ("F198" + waveExtension), path / ("attacking2" + waveExt));
	copyWAV (sVoicePath / ("F201" + waveExtension), path / ("sub_detected" + waveExt));
	copyWAV (sVoicePath / ("F206" + waveExtension), path / ("start_more" + waveExt));
	copyWAV (sVoicePath / ("F210" + waveExtension), path / ("repaired_all1" + waveExt));
	copyWAV (sVoicePath / ("F211" + waveExtension), path / ("repaired_all2" + waveExt));
	copyWAV (sVoicePath / ("F219" + waveExtension), path / ("repaired2" + waveExt));
	copyWAV (sVoicePath / ("F220" + waveExtension), path / ("repaired" + waveExt));
	copyWAV (sVoicePath / ("F224" + waveExtension), path / ("transfer_done" + waveExt));
	copyWAV (sVoicePath / ("F229" + waveExtension), path / ("attacking_us2" + waveExt));
	copyWAV (sVoicePath / ("F230" + waveExtension), path / ("attacking_us3" + waveExt));
	copyWAV (sVoicePath / ("F232" + waveExtension), path / ("attacking_us" + waveExt));
	copyWAV (sVoicePath / ("F234" + waveExtension), path / ("destroyed_us1" + waveExt));
	copyWAV (sVoicePath / ("F234" + waveExtension), path / ("destroyed_us2" + waveExt));
	copyWAV (sVoicePath / ("F239" + waveExtension), path / ("unit_stolen1" + waveExt));
	copyWAV (sVoicePath / ("F242" + waveExtension), path / ("unit_stolen2" + waveExt));
	copyWAV (sVoicePath / ("F243" + waveExtension), path / ("unit_stolen_by_enemy" + waveExt));
	copyWAV (sVoicePath / ("F244" + waveExtension), path / ("unit_disabled" + waveExt));
	copyWAV (sVoicePath / ("F247" + waveExtension), path / ("unit_disabled_by_enemy1" + waveExt));
	copyWAV (sVoicePath / ("F249" + waveExtension), path / ("unit_disabled_by_enemy2" + waveExt));
	copyWAV (sVoicePath / ("F250" + waveExtension), path / ("attacking_enemy1" + waveExt));
	copyWAV (sVoicePath / ("F251" + waveExtension), path / ("attacking_enemy2" + waveExt));

	//-------------------------------------------------------------
	// fix differences between eng and ger original sound-files - nonsinn
	// FIMXE / TODO : French-soundfile-check
	//-------------------------------------------------------------
	// landing screen
	copyWAV (sVoicePath / ("F176" + waveExtension), path / ("landing1" + waveExt));
	if (sLanguage != "german")
	{
		copyWAV (sVoicePath / ("F177" + waveExtension), path / ("landing2" + waveExt));
		copyWAV (sVoicePath / ("F278" + waveExtension), path / ("landing3" + waveExt));
	}
	else
	{ /* install german lang */
		copyWAV (sVoicePath / ("F275" + waveExtension), path / ("landing2" + waveExt));
		copyWAV (sVoicePath / ("F276" + waveExtension), path / ("landing3" + waveExt));
	}

	// differenc of F270 (ammo low vs. empty)
	copyWAV (sVoicePath / ("F138" + waveExtension), path / ("ammo_low1" + waveExt));
	copyWAV (sVoicePath / ("F142" + waveExtension), path / ("ammo_empty1" + waveExt));
	if (sLanguage != "german")
	{
		copyWAV (sVoicePath / ("F271" + waveExtension), path / ("ammo_low2" + waveExt));
		copyWAV (sVoicePath / ("F142" + waveExtension), path / ("ammo_empty2" + waveExt));
	}
	else
	{ /* install german lang */
		copyWAV (sVoicePath / ("F138" + waveExtension), path / ("ammo_low2" + waveExt));
		copyWAV (sVoicePath / ("F270" + waveExtension), path / ("ammo_empty2" + waveExt));
	}

	// 20 sec left
	if (sLanguage != "german")
	{
		copyWAV (sVoicePath / ("F272" + waveExtension), path / ("turn_end_20_sec1" + waveExt)); // not used yet
		copyWAV (sVoicePath / ("F273" + waveExtension), path / ("turn_end_20_sec2" + waveExt)); // not used yet
		copyWAV (sVoicePath / ("F275" + waveExtension), path / ("turn_end_20_sec3" + waveExt)); // not used yet
	}
	else
	{ /* install german lang */
		copyWAV (sVoicePath / ("F271" + waveExtension), path / ("turn_end_20_sec1" + waveExt)); // not used yet
		copyWAV (sVoicePath / ("F272" + waveExtension), path / ("turn_end_20_sec2" + waveExt)); // not used yet
		copyWAV (sVoicePath / ("F273" + waveExtension), path / ("turn_end_20_sec3" + waveExt)); // not used yet
	}
	//-------------------------------------------------------------

	if (logFile != nullptr)
	{
		writeLog (std::string ("Voices") + TEXT_FILE_LF);
		writeLog (std::to_string (iErrors) + " errors" + TEXT_FILE_LF);
		writeLog (std::string ("========================================================================") + TEXT_FILE_LF);
	}

	std::cout << "\n";
	std::cout << std::to_string (iErrors) << " errors\n";
}

//-------------------------------------------------------------
void installMaps()
{
	iTotalFiles = 24;
	iErrors = 0;
	iInstalledFiles = 0;

#if MAC
	updateProgressWindow ("Maps", iTotalFiles, iInstalledFiles);
#endif

	std::cout << "========================================================================\n";
	std::cout << "Maps\n";

	const std::filesystem::path path = std::filesystem::path (sOutputPath) / "maps";
	copyFile (sMAXPath / "CRATER_1.WRL", path / "Iron Cross.wrl");
	copyFile (sMAXPath / "CRATER_2.WRL", path / "Splatterscape.wrl");
	copyFile (sMAXPath / "CRATER_3.WRL", path / "Peak-a-boo.wrl");
	copyFile (sMAXPath / "CRATER_4.WRL", path / "Valentine's Planet.wrl");
	copyFile (sMAXPath / "CRATER_5.WRL", path / "Three Rings.wrl");
	copyFile (sMAXPath / "CRATER_6.WRL", path / "Great divide.wrl");
	copyFile (sMAXPath / "DESERT_1.WRL", path / "Freckles.wrl");
	copyFile (sMAXPath / "DESERT_2.WRL", path / "Sandspit.wrl");
	copyFile (sMAXPath / "DESERT_3.WRL", path / "Great Circle.wrl");
	copyFile (sMAXPath / "DESERT_4.WRL", path / "Long Passage.wrl");
	copyFile (sMAXPath / "DESERT_5.WRL", path / "Flash Point.wrl");
	copyFile (sMAXPath / "DESERT_6.WRL", path / "Bottleneck.wrl");
	copyFile (sMAXPath / "GREEN_1.WRL", path / "New Luzon.wrl");
	copyFile (sMAXPath / "GREEN_2.WRL", path / "Middle Sea.wrl");
	copyFile (sMAXPath / "GREEN_3.WRL", path / "High Impact.wrl");
	copyFile (sMAXPath / "GREEN_4.WRL", path / "Sanctuary.wrl");
	copyFile (sMAXPath / "GREEN_5.WRL", path / "Islandia.wrl");
	copyFile (sMAXPath / "GREEN_6.WRL", path / "Hammerhead.wrl");
	copyFile (sMAXPath / "SNOW_1.WRL", path / "Snowcrab.wrl");
	copyFile (sMAXPath / "SNOW_2.WRL", path / "Frigia.wrl");
	copyFile (sMAXPath / "SNOW_3.WRL", path / "Ice Berg.wrl");
	copyFile (sMAXPath / "SNOW_4.WRL", path / "The Cooler.wrl");
	copyFile (sMAXPath / "SNOW_5.WRL", path / "Ultima Thule.wrl");
	copyFile (sMAXPath / "SNOW_6.WRL", path / "Long Floes.wrl");

	if (logFile != nullptr)
	{
		writeLog (std::string ("Maps") + TEXT_FILE_LF);
		writeLog (std::to_string (iErrors) + " errors" + TEXT_FILE_LF);
		writeLog (std::string ("========================================================================") + TEXT_FILE_LF);
	}

	std::cout << "\n";
	std::cout << std::to_string (iErrors) << " errors\n";
}

//-------------------------------------------------------------
void installSounds()
{
	iTotalFiles = 31;
	iErrors = 0;
	iInstalledFiles = 0;
	oggEncode = 1;

#if MAC
	updateProgressWindow ("Sounds", iTotalFiles, iInstalledFiles);
#endif

	std::cout << "========================================================================\n";
	std::cout << "Sounds\n";

	std::filesystem::path path = std::filesystem::path (sOutputPath) / "sounds";
	copyWAV (sMAXPath / ("ACTIVATE" + waveExtension), path / "activate.wav");
	copyWAV (sMAXPath / ("MASTR17" + waveExtension), path / "building.wav");
	copyWAV (sMAXPath / ("BULL17" + waveExtension), path / "clearing.wav");
	copyWAV (sMAXPath / ("MENGENS4" + waveExtension), path / "HudButton.wav");
	copyWAV (sMAXPath / ("IHITS0" + waveExtension), path / "HudSwitch.wav");
	copyWAV (sMAXPath / ("MLAYER18" + waveExtension), path / "land_mine_clear.wav");
	copyWAV (sMAXPath / ("MLAYER17" + waveExtension), path / "land_mine_place.wav");
	copyWAV (sMAXPath / ("LOAD" + waveExtension), path / "load.wav");
	copyWAV (sMAXPath / ("MENU38" + waveExtension), path / "MenuButton.wav");
	copyWAV (sMAXPath / ("KBUY0" + waveExtension), path / "ObjectMenu.wav");
	copyWAV (sMAXPath / ("ICLOS0L" + waveExtension), path / "panel_close.wav");
	copyWAV (sMAXPath / ("IOPEN0" + waveExtension), path / "panel_open.wav");
	copyWAV (sMAXPath / ("FQUIT" + waveExtension), path / "quitsch.wav");
	copyWAV (sMAXPath / ("REPAIR17" + waveExtension), path / "repair.wav");
	copyWAV (sMAXPath / ("SMINE17" + waveExtension), path / "sea_mine_place.wav");
	copyWAV (sMAXPath / ("SMINE18" + waveExtension), path / "sea_mine_clear.wav");
	copyWAV (sMAXPath / ("FTRUCK17" + waveExtension), path / "reload.wav");
	copyWAV (sMAXPath / ("PLANLAND" + waveExtension), path / "plane_land.wav");
	copyWAV (sMAXPath / ("PLANOFF" + waveExtension), path / "plane_takeoff.wav");

	copyWAV (sMAXPath / ("BOATEXP1" + waveExtension), path / "exp_small_wet0.wav");
	copyWAV (sMAXPath / ("EPLOWET1" + waveExtension), path / "exp_small_wet1.wav");
	copyWAV (sMAXPath / ("EPLOWET2" + waveExtension), path / "exp_small_wet2.wav");
	copyWAV (sMAXPath / ("EXPLLRGE" + waveExtension), path / "exp_small0.wav");
	copyWAV (sMAXPath / ("EXPLMED" + waveExtension), path / "exp_small1.wav");
	copyWAV (sMAXPath / ("EXPLSMAL" + waveExtension), path / "exp_small2.wav");
	copyWAV (sMAXPath / ("CBLDEXP1" + waveExtension), path / "exp_big_wet0.wav");
	copyWAV (sMAXPath / ("CBLDEXP2" + waveExtension), path / "exp_big_wet1.wav");
	copyWAV (sMAXPath / ("BLDEXPLG" + waveExtension), path / "exp_big0.wav");
	copyWAV (sMAXPath / ("EXPBULD6" + waveExtension), path / "exp_big1.wav");
	copyWAV (sMAXPath / ("EXPLBLD1" + waveExtension), path / "exp_big2.wav");
	copyWAV (sMAXPath / ("EXPLBLD2" + waveExtension), path / "exp_big3.wav");

	if (logFile != nullptr)
	{
		writeLog (std::string ("Sounds") + TEXT_FILE_LF);
		writeLog (std::to_string (iErrors) + " errors" + TEXT_FILE_LF);
		writeLog (std::string ("========================================================================") + TEXT_FILE_LF);
	}

	std::cout << "\n";
	std::cout << std::to_string (iErrors) << " errors\n";
}

//-------------------------------------------------------------
void installMusic()
{
	iTotalFiles = 13;
	iErrors = 0;
	iInstalledFiles = 0;

	oggEncode = 1;

#if MAC
	updateProgressWindow ("Music (May take a while)", iTotalFiles, iInstalledFiles);
#endif

	std::cout << "========================================================================\n";
	std::cout << "Music (May take a while)\n";

	const std::filesystem::path path = std::filesystem::path (sOutputPath) / "music";
	copyWAV (sMAXPath / "MAIN_MSC.MSC", path / "main.wav");
	copyWAV (sMAXPath / "BKG1_MSC.MSC", path / "bkg1.wav");
	copyWAV (sMAXPath / "BKG2_MSC.MSC", path / "bkg2.wav");
	copyWAV (sMAXPath / "BKG3_MSC.MSC", path / "bkg3.wav");
	copyWAV (sMAXPath / "BKG4_MSC.MSC", path / "bkg4.wav");
	copyWAV (sMAXPath / "BKG5_MSC.MSC", path / "bkg5.wav");
	copyWAV (sMAXPath / "BKG6_MSC.MSC", path / "bkg6.wav");
	copyWAV (sMAXPath / "CRTR_MSC.MSC", path / "crtr.wav");
	copyWAV (sMAXPath / "DSRT_MSC.MSC", path / "dsrt.wav");
	copyWAV (sMAXPath / "GREN_MSC.MSC", path / "gren.wav");
	copyWAV (sMAXPath / "LOSE_MSC.MSC", path / "lose.wav");
	copyWAV (sMAXPath / "SNOW_MSC.MSC", path / "snow.wav");
	copyWAV (sMAXPath / "WINR_MSC.MSC", path / "winr.wav");

	if (logFile != nullptr)
	{
		writeLog (std::string ("Music") + TEXT_FILE_LF);
		writeLog (std::to_string (iErrors) + " errors" + TEXT_FILE_LF);
		writeLog (std::string ("========================================================================") + TEXT_FILE_LF);
	}

	std::cout << "\n";
	std::cout << std::to_string (iErrors) << " errors\n";
}

//-------------------------------------------------------------
void initialize()
{
	// at startup SDL_Init should be called before all other SDL functions
	if (SDL_Init (SDL_INIT_VIDEO) == -1)
	{
		printf ("Can't init SDL:  %s\n", SDL_GetError());
		exit (-1);
	}

	/*
	 Uncommented since you do break any terminal I/O for users at least
	 under linux with that too. You may write some WIN32 DEFINED code
	 to prevend stdout.txt to be created on windows -- beko
	 */
	// added code to prevent writing to stdout.txt and stderr.txt
	// freopen( "CON", "w", stdout );

	atexit (SDL_Quit);
}

//-------------------------------------------------------------
void showIntroduction()
{
	std::string strAbout1 = "Resinstaller - installs graphics and sounds from Interplay's M.A.X. to ";
	std::string strAbout2 = "M.A.X.R. for original game look and feel. For this you need an existing ";
	std::string strAbout3 = "M.A.X. installation or an original M.A.X. CD available.";

	std::string strGPL1 = "This program is free software; you can redistribute it and/or modify ";
	std::string strGPL2 = "it under the terms of the GNU General Public License as published by ";
	std::string strGPL3 = "the Free Software Foundation; either version 2 of the License, or ";
	std::string strGPL4 = "(at your option) any later version.";

	std::string strGPL5 = "This program is distributed in the hope that it will be useful, ";
	std::string strGPL6 = "but WITHOUT ANY WARRANTY; without even the implied warranty of ";
	std::string strGPL7 = "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the ";
	std::string strGPL8 = "GNU General Public License for more details.";

#if MAC
	std::string strText = strAbout1 + strAbout2 + strAbout3 + "\n\n\n"
	                    + strGPL1 + strGPL2 + strGPL3 + strGPL4 + "\n\n"
	                    + strGPL5 + strGPL6 + strGPL7 + strGPL8;
	showIntroductionMAC (strText);
#else
	std::cout << strAbout1 << std::endl;
	std::cout << strAbout2 << std::endl;
	std::cout << strAbout3 << std::endl
			  << std::endl;
	std::cout << strGPL1 << std::endl;
	std::cout << strGPL2 << std::endl;
	std::cout << strGPL3 << std::endl;
	std::cout << strGPL4 << std::endl
			  << std::endl;
	std::cout << strGPL5 << std::endl;
	std::cout << strGPL6 << std::endl;
	std::cout << strGPL7 << std::endl;
	std::cout << strGPL8 << std::endl
			  << std::endl;
#endif
}

#ifdef WIN32
std::filesystem::path getHomeDir()
{
	char szPath[MAX_PATH];
	if (SHGetFolderPathA (nullptr, CSIDL_PERSONAL, nullptr, 0, szPath) == S_OK)
	{
		return szPath;
	}
	return "";
}
#endif

//-------------------------------------------------------------
void createLogFile (const std::filesystem::path& dataDir)
{
	std::filesystem::path path;
	if (!dataDir.empty() && DirExists (dataDir / "portable"))
	{
		path = dataDir / "portable";
	}
	else
	{
#ifdef WIN32
		path = getHomeDir();
		if (path.empty())
		{
			std::cout << "Warning: Couldn't determine home directory. Writing log to current directory instead.\n";
		}
		else
		{
			path /= "maxr";
			if (!DirExists (path))
			{
				if (!std::filesystem::create_directories (path))
				{
					path = "";
					std::cout << "Warning: Couldn't write in home directory. Writing log to current directory instead.\n";
				}
			}
		}
#endif
	}
	auto fileName = path / "resinstaller.log";
	logFile = SDL_RWFromFile (fileName.u8string().c_str(), "a");
	if (logFile == nullptr)
	{
		std::cout << "Warning: Couldn't create log file. Writing to stdout instead.\n";
	}
	else
	{
		std::cout << "write log in " << fileName << std::endl;
		freopen (fileName.string().c_str(), "a", stderr); // write errors to log instead stdout(.txt)
	}

	writeLog (std::string ("resinstaller version ") + VERSION + TEXT_FILE_LF);
}

void checkWritePermissions (const std::string& appName, bool bDoNotElevate)
{
#ifdef WIN32
	// create test file
	auto testFileName = sOutputPath / "writeTest.txt";
	SDL_RWops* testFile = SDL_RWFromFile (testFileName.u8string().c_str(), "w");

	if (testFile == nullptr)
	{
		writeLog (std::string ("Couldn't write to output directory") + TEXT_FILE_LF);
		if (!bDoNotElevate)
		{
			writeLog (std::string ("Retrying with admin rights...") + TEXT_FILE_LF);
			SDL_RWclose (logFile);
			std::string parameter = "\"" + sMAXPath.string() + "\"\" \"" + sOutputPath.string() + "\"\" " + sLanguage + " " + sResChoice + " /donotelevate";

			HINSTANCE result = ShellExecuteA (
				nullptr,
				"runas", // request elevated rights
				appName.c_str(),
				parameter.c_str(),
				nullptr, // working directory
				SW_SHOW);

			if ((std::uintptr_t) result > 32)
				exit (0); // success
		}

		std::cout << "Failed. Please restart the application with admin rights." << std::endl;

		// wait for key press
		FlushConsoleInputBuffer (GetStdHandle (STD_INPUT_HANDLE));
		getch();

		exit (-1);
	}

	SDL_RWclose (testFile);
	std::filesystem::remove (testFileName);

#endif
}

//-------------------------------------------------------------
bool validateMAXPath (std::filesystem::path& maxPath)
{
	const std::filesystem::path dirs[] = {
		maxPath,
		maxPath / "MAX",
		maxPath / "max",
	};

	// now testing different input variations
	for (const auto& dir : dirs)
	{
		try
		{
			res = openFile (dir / "MAX.RES", "rb");
			maxPath = dir;
			return true;
		}
		catch (InstallException)
		{} // ignore exceptions
	}
	return false;
}

//-------------------------------------------------------------
std::filesystem::path getMAXPathFromUser (std::string cmdLineMaxPath)
{
#if MAC
	// pass the validateMAXPath-Method as function pointer, so that the askForCDPath-Method can determine, if the path is valid.
	return askForCDPath (validateMAXPath);
#else

	std::cout << "Path to an existing M.A.X. installation or mounted cd." << std::endl;
	// test, if a path was given from the command line
	if (cmdLineMaxPath.size() != 0)
	{
		trimQuotes (cmdLineMaxPath);
		writeLog ("sMAXPath from command line: " + cmdLineMaxPath + TEXT_FILE_LF);
		std::cout << "Path was given as command line argument: " << cmdLineMaxPath << std::endl;
		std::filesystem::path maxPath = cmdLineMaxPath;
		if (validateMAXPath (maxPath))
			return maxPath;
		else
			std::cout << "Path is not a valid full path to an existing M.A.X. installation or mounted cd." << std::endl;
	}

	while (true)
	{
		std::cout << "Please enter full path to existing M.A.X. installation or mounted cd:" << std::endl;

		// read path from cin
		std::string pathFromUser = "";
		std::getline (std::cin, pathFromUser, '\n');
		trimSpaces (pathFromUser);
		trimQuotes (pathFromUser);

		std::filesystem::path maxPath = pathFromUser;
		if (validateMAXPath (maxPath))
		{
			std::cout << std::endl;
			return maxPath;
		}

		std::cout << "Couldn't find valid M.A.X. installation in given folder:\n";
		std::cout << "No max.res found.\n";
	}
#endif
}

//-------------------------------------------------------------
bool validateOutputPath (const std::string& outputPath)
{
	return std::filesystem::exists (std::filesystem::path (outputPath) / "init.pcx");
}

//-------------------------------------------------------------
std::string validateResources (std::string zChoices)
{
	if (zChoices.find ("all") != std::string::npos)
	{
		zChoices = "0123456789ab";
	}
	if (zChoices.find ("z") != std::string::npos)
	{
		zChoices = "012345b";
	}
	return zChoices;
}

//-------------------------------------------------------------
void getResChoiceFromUser()
{
	std::string sChoiceFromUser = "";
	// what kind of resources should the resinstaller import into M.A.X. Reloaded
	std::cout << "What kind of resources should the resinstaller import into your M.A.X. Reloaded installation?" << std::endl;
	std::cout << "========================================================================" << std::endl;
	std::cout << "All Resources        (all)" << std::endl;
	std::cout << "==========================" << std::endl;
	std::cout << "Building Sounds        (0)" << std::endl;
	std::cout << "Vehicle Sounds         (1)" << std::endl;
	std::cout << "Voices                 (2)" << std::endl;
	std::cout << "Sounds                 (3)" << std::endl;
	std::cout << "Music                  (4)" << std::endl;
	std::cout << "Movies                 (5)" << std::endl;
	std::cout << "FX                     (6)" << std::endl;
	std::cout << "GFX                    (7)" << std::endl;
	std::cout << "Vehicle Videos         (8)" << std::endl;
	std::cout << "Vehicle Graphics       (9)" << std::endl;
	std::cout << "Building Graphics      (a)" << std::endl;
	std::cout << "Maps                   (b)" << std::endl;
	std::cout << "==========================" << std::endl;
	std::cout << "All but no old Graphic (z)" << std::endl;
	std::cout << "==========================" << std::endl;
	std::cout << "Example: Type \"0123458b\", \"all\" or \"z\" without the \"" << std::endl;

	std::getline (std::cin, sChoiceFromUser); // get user input
	for (unsigned int i = 0; i < (sChoiceFromUser.length()); i++)
	{
		// convert string into lowercase
		sChoiceFromUser[i] = tolower (sChoiceFromUser[i]);
	}
	sResChoice = validateResources (sChoiceFromUser); // call validate function
}

//-------------------------------------------------------------
std::filesystem::path getOutputPathFromUser (std::string cmdLineOutputPath)
{
#if MAC
	// pass the validateOutputPath-Method as function pointer, so that the askForOutputPath-Method can determine, if the path is valid.
	return askForOutputPath (validateOutputPath);

#else

	std::cout << "Path to M.A.X.R. for extracted files." << std::endl;
	// test, if a path was given from the command line
	if (cmdLineOutputPath.size() != 0)
	{
		trimQuotes (cmdLineOutputPath);
		writeLog ("sOutputPath from command line: " + cmdLineOutputPath + TEXT_FILE_LF);
		std::cout << "Path was given as command line argument: " << cmdLineOutputPath << std::endl;
		if (validateOutputPath (cmdLineOutputPath))
			return cmdLineOutputPath;
		else
			std::cout << "Can't extract the resources to the path, because it doesn't point to a M.A.X.R installation." << std::endl;
	}

	while (true)
	{
		std::string pathFromUser = "";
		std::cout << "Please enter full path to M.A.X.R. installation for extracted files:" << std::endl;

		// read the path from cin
		std::getline (std::cin, pathFromUser);
		trimSpaces (pathFromUser);
		trimQuotes (pathFromUser);

		if (validateOutputPath (pathFromUser))
		{
			std::cout << std::endl;
			return pathFromUser;
		}

		std::cout << "Couldn't find valid M.A.X.R. installation in given folder." << std::endl;
		pathFromUser.clear();
	}
#endif
}

//-------------------------------------------------------------
int checkForAvailableLanguages (std::string testFileName, bool& bGerman, bool& bItalian, bool& bFrench, bool& bUppercase)
{
	int iLanguages = 0;
	if (std::filesystem::exists (sMAXPath / "german" / (testFileName + waveExtension)))
	{
		iLanguages++;
		bGerman = true;
		bUppercase = false;
	}
	if (std::filesystem::exists (sMAXPath / "GERMAN" / (testFileName + waveExtension)))
	{
		if (bGerman == false) iLanguages++;
		bGerman = true;
		bUppercase = true;
	}
	if (std::filesystem::exists (sMAXPath / "italian" / (testFileName + waveExtension)))
	{
		iLanguages++;
		bItalian = true;
		bUppercase = false;
	}
	if (std::filesystem::exists (sMAXPath / "ITALIAN" / (testFileName + waveExtension)))
	{
		if (bItalian == false) iLanguages++;
		bItalian = true;
		bUppercase = true;
	}
	if (std::filesystem::exists (sMAXPath / "french" / (testFileName + waveExtension)))
	{
		iLanguages++;
		bFrench = true;
		bUppercase = false;
	}
	if (std::filesystem::exists (sMAXPath / "FRENCH" / (testFileName + waveExtension)))
	{
		if (bFrench == false) iLanguages++;
		bFrench = true;
		bUppercase = true;
	}
	return iLanguages;
}

bool gFinishedInstalling = false; // MAC: needed as flag, for closing the progess bar window, when the installation is finished.

//-------------------------------------------------------------
int installEverything (void*)
{
	gFinishedInstalling = false;

#if 0
	saveAllFiles();
#endif

	if (sResChoice.find ("0") != std::string::npos)
	{
		installBuildingSounds();
	}
	else
	{
		std::cout << "Skip Building Sounds" << std::endl;
		writeLog (std::string ("Skip Building Sounds") + TEXT_FILE_LF);
		writeLog (std::string ("========================================================================") + TEXT_FILE_LF);
	}

	if (sResChoice.find ("1") != std::string::npos)
	{
		installVehicleSounds();
	}
	else
	{
		std::cout << "Skip Vehicle Sounds" << std::endl;
		writeLog (std::string ("Skip Vehicle Sounds") + TEXT_FILE_LF);
		writeLog (std::string ("========================================================================") + TEXT_FILE_LF);
	}

	if (sResChoice.find ("2") != std::string::npos)
	{
		installVoices();
	}
	else
	{
		std::cout << "Skip Voices" << std::endl;
		writeLog (std::string ("Skip Voices") + TEXT_FILE_LF);
		writeLog (std::string ("========================================================================") + TEXT_FILE_LF);
	}

	if (sResChoice.find ("3") != std::string::npos)
	{
		installSounds();
	}
	else
	{
		std::cout << "Skip Sounds" << std::endl;
		writeLog (std::string ("Skip Sounds") + TEXT_FILE_LF);
		writeLog (std::string ("========================================================================") + TEXT_FILE_LF);
	}

	if (sResChoice.find ("4") != std::string::npos)
	{
		installMusic();
	}
	else
	{
		std::cout << "Skip Music" << std::endl;
		writeLog (std::string ("Skip Music") + TEXT_FILE_LF);
		writeLog (std::string ("========================================================================") + TEXT_FILE_LF);
	}

	if (sResChoice.find ("5") != std::string::npos)
	{
		installMVEs();
	}
	else
	{
		std::cout << "Skip Movies" << std::endl;
		writeLog (std::string ("Skip Movies") + TEXT_FILE_LF);
		writeLog (std::string ("========================================================================") + TEXT_FILE_LF);
	}

	if (sResChoice.find ("6") != std::string::npos)
	{
		installFX();
	}
	else
	{
		std::cout << "Skip FX" << std::endl;
		writeLog (std::string ("Skip FX") + TEXT_FILE_LF);
		writeLog (std::string ("========================================================================") + TEXT_FILE_LF);
	}

	if (sResChoice.find ("7") != std::string::npos)
	{
		installGfx();
	}
	else
	{
		std::cout << "Skip GFX" << std::endl;
		writeLog (std::string ("Skip GFX") + TEXT_FILE_LF);
		writeLog (std::string ("========================================================================") + TEXT_FILE_LF);
	}

	if (sResChoice.find ("8") != std::string::npos)
	{
		installVehicleVideos();
	}
	else
	{
		std::cout << "Skip Vehicle Videos" << std::endl;
		writeLog (std::string ("Skip Vehicle Videos") + TEXT_FILE_LF);
		writeLog (std::string ("========================================================================") + TEXT_FILE_LF);
	}

	if (sResChoice.find ("9") != std::string::npos)
	{
		installVehicleGraphics();
	}
	else
	{
		std::cout << "Skip Vehicle Graphics" << std::endl;
		writeLog (std::string ("Skip Vehicle Graphics") + TEXT_FILE_LF);
		writeLog (std::string ("========================================================================") + TEXT_FILE_LF);
	}

	if (sResChoice.find ("a") != std::string::npos)
	{
		installBuildingGraphics();
	}
	else
	{
		std::cout << "Skip Building Graphics" << std::endl;
		writeLog (std::string ("Skip Building Graphics") + TEXT_FILE_LF);
		writeLog (std::string ("========================================================================") + TEXT_FILE_LF);
	}

	if (sResChoice.find ("b") != std::string::npos)
	{
		installMaps();
	}
	else
	{
		std::cout << "Skip Maps" << std::endl;
		writeLog (std::string ("Skip Maps") + TEXT_FILE_LF);
		writeLog (std::string ("========================================================================") + TEXT_FILE_LF);
	}

	gFinishedInstalling = true;

	return 0;
}

void trimSpaces (std::string& str, const std::locale& loc)
{
	// trim spaces at the beginning
	std::string::size_type pos = 0;
	while (pos < str.size() && isspace (str[pos], loc))
		pos++;
	str.erase (0, pos);
	// trim spaces at the end
	pos = str.size();
	while (pos > 0 && isspace (str[pos - 1], loc))
		pos--;
	str.erase (pos);
}

void trimQuotes (std::string& str)
{
	std::string::size_type pos = 0;
	while (pos < str.size() && str[pos] == '"')
		pos++;
	str.erase (0, pos);
	pos = str.size();
	while (pos > 0 && str[pos - 1] == '"')
		pos--;
	str.erase (pos);
}

//-------------------------------------------------------------
int main (int argc, char* argv[])
{
	initialize();
	showIntroduction();

	const std::string appName = argv[0];
	// original M.A.X. Path (CD or installation)
	sMAXPath = getMAXPathFromUser ((argc > 1) ? argv[1] : ""); // if the user canceled, sMAXPath will be empty (at least on MAC)

	if (sMAXPath.empty())
		exit (-1);

	// M.A.X. Reloaded Path
	sOutputPath = getOutputPathFromUser ((argc > 2) ? argv[2] : ""); // if the user canceled, sOutputPath will be empty (at least on MAC)
	if (sOutputPath.empty())
		exit (-1);

	// language german, english, french, italian
	sLanguage = (argc > 3) ? argv[3] : "";

	// Parameter for gNewGrafic
	sResChoice = (argc > 4) ? argv[4] : "";

	createLogFile (sOutputPath);
	wasError = false;

	sResChoice = validateResources (sResChoice); // validate the parameter for importing the resources

	const bool bDoNotElevate = (std::string (argv[argc - 1]) == "/donotelevate");

	if (sResChoice.empty())
	{
		// what resources should the resinstaller import into M.A.X. Reloaded? Call user input!
		getResChoiceFromUser();
	}

	// test with a sound file, which languages are available at the install source
	std::string testFileName = "F001";
	// check for North American CD source
	waveExtension = ".WAV";
	try
	{
		SDL_RWops* testFile = openFile (sMAXPath / (testFileName + ".SPW"), "r");
		waveExtension = ".SPW";
		SDL_RWclose (testFile);
	}
	catch (InstallException)
	{}

	bool bGerman = false, bItalian = false, bFrench = false;
	bool bUppercase;
	int iLanguages = checkForAvailableLanguages (testFileName, bGerman, bItalian, bFrench, bUppercase);

	// choose the language to install
	if (iLanguages == 0)
	{
		// we are not installing from CD
		sVoicePath = sMAXPath;
	}
	// we got the language parameter from commandline
	else if (!sLanguage.empty())
	{
		writeLog ("Language argument from command line: " + sLanguage + TEXT_FILE_LF);
		if (sLanguage == "english" || sLanguage == "ENG")
		{
			sVoicePath = sMAXPath;
		}
		else if ((sLanguage == "german" || sLanguage == "GER") && bGerman)
		{
			if (bUppercase)
				sVoicePath = sMAXPath / "GERMAN";
			else
				sVoicePath = sMAXPath / "german";
		}
		else if ((sLanguage == "french" || sLanguage == "FRE") && bFrench)
		{
			if (bUppercase)
				sVoicePath = sMAXPath / "FRENCH";
			else
				sVoicePath = sMAXPath / "french";
		}
		else if ((sLanguage == "italian" || sLanguage == "ITA") && bItalian)
		{
			if (bUppercase)
				sVoicePath = sMAXPath / "ITALIAN";
			else
				sVoicePath = sMAXPath / "italian";
		}
		else
		{
			sVoicePath = sMAXPath;
			writeLog ("Language is not available");
		}
		writeLog ("Voice path: " + sVoicePath.u8string() + TEXT_FILE_LF);
	}
	// ask the user, which language to install
	else
	{
#if MAC
		int languageChosen = askForLanguage (bGerman, bItalian, bFrench);
		if (languageChosen == 0) // english
			sVoicePath = sMAXPath;
		else if (languageChosen == 1 && bUppercase)
			sVoicePath = sMAXPath / "GERMAN";
		else if (languageChosen == 1 && !bUppercase)
			sVoicePath = sMAXPath / "german";
		else if (languageChosen == 2 && bUppercase)
			sVoicePath = sMAXPath / "ITALIAN";
		else if (languageChosen == 2 && !bUppercase)
			sVoicePath = sMAXPath / "italian";
		else if (languageChosen == 3 && bUppercase)
			sVoicePath = sMAXPath / "FRENCH";
		else if (languageChosen == 3 && !bUppercase)
			sVoicePath = sMAXPath / "french";
		else
			sVoicePath = sMAXPath; // default - but should not happen
#else

		// make menu values
		std::vector<std::string> vectorLanguages{"english"}; // initialize empty vector of strings

		if (bGerman)
		{
			vectorLanguages.push_back ("german");
		}
		if (bItalian)
		{
			vectorLanguages.push_back ("italian");
		}
		if (bFrench)
		{
			vectorLanguages.push_back ("french");
		}

		do
		{
			// make menu output
			std::cout << "\nThe following voice samples are available from your install source:\n";
			std::cout << "\n No. | as word\n";
			std::cout << " ------------- " << std::endl;
			for (unsigned ii = 0; ii < vectorLanguages.size(); ii++)
			{
				std::cout << "  " << ii + 1 << "  | " << vectorLanguages[ii] << std::endl; // output languages from vector with increased number to start menu with 1 instead of 0
				std::cout << " ------------- " << std::endl;
			}

			std::cout << "\nPlease enter your preferred language (as number or word): ";

			// read lang from cin
			std::string input;
			std::getline (std::cin, input);
			trimSpaces (input);
			transform (input.begin(), input.end(), input.begin(), ::tolower);

			long int value = strtol (input.c_str(), nullptr, 10); // If no valid conversion could be performed, a zero value is returned
			if (value > 0 && value <= (long) vectorLanguages.size())
			{
				input = vectorLanguages[value - 1];
			}
			sLanguage = input;
			std::string errormsg;
			if (value < 0 || value > (long) vectorLanguages.size())
			{
				errormsg = "you inserted an invalid number";
			}
			else // no number entered. Search language by string
			{
				for (unsigned ii = 0; ii < vectorLanguages.size(); ii++)
				{
					if (input == "english")
					{
						sVoicePath = sMAXPath;
					}
					else if (!bUppercase && input == vectorLanguages[ii])
					{
						sVoicePath = sMAXPath / input;
					}
					else if (bUppercase && input == vectorLanguages[ii])
					{
						transform (input.begin(), input.end(), input.begin(), ::toupper);
						sVoicePath = sMAXPath / input;
					}
				}
			}

			if (errormsg.empty() && sVoicePath.empty())
			{
				errormsg = "you inserted an invalid word as language";
			}
			if (sVoicePath.empty())
			{
				std::cout << "\nSo sorry, but " << errormsg << std::endl;
				std::cout << " Please try it again...\n";
			}
		} while (sVoicePath.empty());

#endif
	}

	// check if we need admin rights for the selected output directory
	checkWritePermissions (appName, bDoNotElevate);

	// init res converter
	SDL_RWseek (res, 0, SEEK_END);
	lEndOfFile = SDL_RWtell (res);

	lPosBegin = 15000000; // the '[EOD]' should be after this position
		// for all versions of max.res, I think

	// a little state machine for searching the string "[EOD]" in max.res
	unsigned char temp, state = 0;
	SDL_RWseek (res, lPosBegin, SEEK_SET);

	while (lPosBegin < lEndOfFile)
	{
		SDL_RWread (res, &temp, sizeof (char), 1);
		lPosBegin++;

		switch (state)
		{
			case 1:
				if (temp == 'E')
					state = 2;
				else
					state = 0;
				break;
			case 2:
				if (temp == 'O')
					state = 3;
				else
					state = 0;
				break;
			case 3:
				if (temp == 'D')
					state = 4;
				else
					state = 0;
				break;
			case 4:
				if (temp == ']')
					state = 5;
				else
					state = 0;
				break;
		}

		if (temp == '[') state = 1;

		if (state == 5) break;
	}

	if (lPosBegin == lEndOfFile)
	{
		std::cout << "Error: [EOD] not found in resource file. Please contact the developer!";
		exit (-1);
	}

	// Do the work: Install the resources
#if MAC
	// on MAC the installation has to happen in a separate thread to allow displaying and updating a progress window
	SDL_Thread* installThread = nullptr;
	gFinishedInstalling = false;
	installThread = SDL_CreateThread (installEverything, nullptr);
	if (installThread != 0)
	{
		displayProgressWindow (gFinishedInstalling);
		SDL_WaitThread (installThread, nullptr);
	}
#else
	installEverything (nullptr);
#endif

	if (wasError)
	{
		std::cout << "There were errors during install. See 'resinstaller.log' for details.\n";
#if MAC
		showMessage ("Installation Finished", "There were errors during install. See 'resinstaller.log' for details.");
#endif
	}
	else
	{
		std::cout << "Finished\n";
#if MAC
		showMessage ("Installation Finished", "The installation finished without errors. Have fun playing M.A.X.R.!");
#endif
	}

	SDL_RWclose (res);
	SDL_RWclose (logFile);

#if defined(WIN32) && !defined(NDEBUG)
	// wait for key press
	FlushConsoleInputBuffer (GetStdHandle (STD_INPUT_HANDLE));
	getch();
#endif

	return 0;
}
