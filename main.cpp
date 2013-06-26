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

#include <math.h>
#include <iostream>
#include <stdio.h>
#include <sstream>

#include <SDL.h>
#include <SDL_thread.h>
#include <SDL_net.h>
#include <SDL_mixer.h>
#include <SDL_getenv.h>

#define __main__
#include "main.h"

#include "autosurface.h"
#include "base.h"
#include "buildings.h"
#include "clans.h"
#include "dedicatedserver.h"
#include "events.h"
#include "files.h"
#include "input.h"
#include "keys.h"
#include "loaddata.h"
#include "log.h"
#include "map.h"
#include "menus.h"
#include "mouse.h"
#include "mveplayer.h"
#include "network.h"
#include "pcx.h"
#include "player.h"
#include "savegame.h"
#include "settings.h"
#include "sound.h"
#include "unifonts.h"
#include "vehicles.h"
#include "video.h"

using namespace std;

static int initNet();
static int initSDL();
static int initSound();

int main (int argc, char* argv[])
{
	if (!cSettings::getInstance().isInitialized())
	{
		Quit();
		return -1;
	}
	//stop on error during init of SDL basics. WARNINGS will be ignored!
	if (initSDL() == -1) return -1;
	{
		string sVersion = PACKAGE_NAME; sVersion += " ";
		sVersion += PACKAGE_VERSION; sVersion += " ";
		sVersion += PACKAGE_REV; sVersion += " ";
		Log.write (sVersion, cLog::eLOG_TYPE_INFO);
		string sBuild = "Build: "; sBuild += MAX_BUILD_DATE;
		Log.write (sBuild, cLog::eLOG_TYPE_INFO);
#if HAVE_AUTOVERSION_H
		string sBuildVerbose = "On: ";
		sBuildVerbose += BUILD_UNAME_S;
		sBuildVerbose += " ";
		sBuildVerbose += BUILD_UNAME_R;
		Log.write (sBuildVerbose, cLog::eLOG_TYPE_INFO);

		sBuildVerbose = "From: ";
		sBuildVerbose += BUILD_USER;
		sBuildVerbose += " at ";
		sBuildVerbose += BUILD_UNAME_N;
		Log.write (sBuildVerbose, cLog::eLOG_TYPE_INFO);
#endif
		Log.mark();
		Log.write (sVersion, cLog::eLOG_TYPE_NET_DEBUG);
		Log.write (sBuild, cLog::eLOG_TYPE_NET_DEBUG);
	}

	srand ( (unsigned) time (NULL)); // start random number generator

	if (!DEDICATED_SERVER)
	{
		// detect some video modes for us
		Video.doDetection();

		Video.initSplash(); // show splashscreen
		initSound(); // now config is loaded and we can init sound and net
	}
	initNet();

	// load files
	int loadingState = LOAD_GOING;
	SDL_Thread* dataThread = SDL_CreateThread (LoadData, &loadingState);

	SDL_Event event;
	while (loadingState != LOAD_FINISHED)
	{
		if (loadingState == LOAD_ERROR)
		{
			Log.write ("Error while loading data!", cLog::eLOG_TYPE_ERROR);
			SDL_WaitThread (dataThread, NULL);
			Quit();
		}
		while (SDL_PollEvent (&event))
		{
			if (event.type == SDL_ACTIVEEVENT)
			{
				if (!DEDICATED_SERVER)
					SDL_UpdateRect (screen, 0, 0, 0, 0);
			}
		}
		SDL_Delay (100);
	}

	if (!DEDICATED_SERVER)
	{
		// play intro if we're supposed to and the file exists
		if (cSettings::getInstance().shouldShowIntro())
		{
			if (FileExists ( (cSettings::getInstance().getMvePath() + PATH_DELIMITER + "MAXINT.MVE").c_str()))
			{
				// Close maxr sound for intro movie
				CloseSound();

				char mvereturn;
				Log.write ("Starting movie " + cSettings::getInstance().getMvePath() + PATH_DELIMITER + "MAXINT.MVE", cLog::eLOG_TYPE_DEBUG);
				mvereturn = MVEPlayer ( (cSettings::getInstance().getMvePath() + PATH_DELIMITER + "MAXINT.MVE").c_str(), Video.getResolutionX(), Video.getResolutionY(), !Video.getWindowMode(), !cSettings::getInstance().isSoundMute());
				Log.write ("MVEPlayer returned " + iToStr (mvereturn), cLog::eLOG_TYPE_DEBUG);
				//FIXME: make this case sensitive - my mve is e.g. completly lower cases -- beko

				// reinit maxr sound
				if (cSettings::getInstance().isSoundEnabled() && !InitSound (cSettings::getInstance().getFrequency(), cSettings::getInstance().getChunkSize()))
				{
					Log.write ("Can't reinit sound after playing intro" + iToStr (mvereturn), cLog::eLOG_TYPE_DEBUG);
				}
			}
			else
			{
				Log.write ("Couldn't find movie " + cSettings::getInstance().getMvePath() + PATH_DELIMITER + "MAXINT.MVE", cLog::eLOG_TYPE_WARNING);
			}
		}
		else
		{
			Log.write ("Skipped intro movie due settings", cLog::eLOG_TYPE_DEBUG);
		}
	}

	SDL_WaitThread (dataThread, NULL);

	if (!DEDICATED_SERVER)
	{
		Video.setResolution (Video.getResolutionX(), Video.getResolutionY(), true);
		SDL_ShowCursor (0);
		Video.clearBuffer();

		mouse = new cMouse;
		InputHandler = new cInput;
		cStartMenu mainMenu;
		mainMenu.show (NULL);
	}
	else
	{
		cDedicatedServer::instance().run();
	}

	Quit();
	return 0;
}


/**
 *Inits SDL
 *@author beko
 *@return -1 on error<br>0 on success<br>1 with warnings
 */
static int initSDL()
{
	int sdlInitResult = -1;
	if (DEDICATED_SERVER)
		sdlInitResult = SDL_Init (SDL_INIT_TIMER | SDL_INIT_NOPARACHUTE);  // start SDL basics without video
	else
		sdlInitResult = SDL_Init (SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_NOPARACHUTE);  // start SDL basics
	if (sdlInitResult == -1)
	{
		Log.write ("Could not init SDL", cLog::eLOG_TYPE_ERROR);
		Log.write (SDL_GetError(), cLog::eLOG_TYPE_ERROR);
		return -1;
	}
	else
	{
		Log.write ("Initalized SDL basics - looks good!", cLog::eLOG_TYPE_INFO);
		Log.mark();
		//made it - enough to start game
		return 0;
	}
}

/**
 *Inits SDL_sound
 *@author beko
 *@return -1 on error<br>0 on success<br>1 with warnings
 */
static int initSound()
{
	if (!cSettings::getInstance().isSoundEnabled())
	{
		Log.write ("Sound disabled due configuration", cLog::eLOG_TYPE_INFO);
		return 1;
	}

	if (SDL_Init (SDL_INIT_AUDIO) < 0)     //start sound
	{
		Log.write ("Could not init SDL_INIT_AUDIO", cLog::eLOG_TYPE_WARNING);
		Log.write ("Sound won't be available!", cLog::eLOG_TYPE_WARNING);
		Log.write (SDL_GetError(), cLog::eLOG_TYPE_WARNING);
		cSettings::getInstance().setSoundEnabled (false, false);
		return -1;
	}

	if (!InitSound (cSettings::getInstance().getFrequency(), cSettings::getInstance().getChunkSize()))
	{
		Log.write ("Could not access mixer", cLog::eLOG_TYPE_WARNING);
		Log.write ("Sound won't be available!", cLog::eLOG_TYPE_WARNING);
		cSettings::getInstance().setSoundEnabled (false, false);
		return -1;
	}
	Log.write ("Sound started", cLog::eLOG_TYPE_INFO);
	return 0;
}

/**
 *Inits SDL_net
 *@author beko
 *@return -1 on error<br>0 on success<br>1 with warnings
 */
static int initNet()
{
	if (SDLNet_Init() == -1)   // start SDL_net
	{
		Log.write ("Could not init SDLNet_Init\nNetwork games won' be available! ", cLog::eLOG_TYPE_WARNING);
		Log.write (SDL_GetError(), cLog::eLOG_TYPE_WARNING);
		return -1;
	}
	Log.write ("Net started", cLog::eLOG_TYPE_INFO);
	return 0;
}

/**
 *Terminates app
 *@author beko
 */
void Quit()
{
	delete mouse;
	delete font;
	delete InputHandler;

	UnitsData.vehicle.clear();
	UnitsData.building.clear();

	//unload files here
	CloseSound();
	SDLNet_Quit();
	SDL_FreeSurface (buffer);
	SDL_FreeSurface (screen);
	SDL_Quit();
	Log.write ("EOF");
	exit (0);
}

/**
 * draws one line of the source surface scaled to the destination surface.
 * @author alzi alias DoctorDeath
 * @param srcPixelData pointer to the first byte of the line in the pixeldate
 *        of the source surface.
 * @param srcWidth width of the line in the sourcesurface.
 * @param destPixelData pointer to the first byte where in the pixeldate
 *        of the source surface the line should be drawn.
 * @param destWidth Directory width of the line how it should be drawn
 *        to the destination surface.
 */
template<typename Type> static void drawStetchedLine (Type* srcPixelData, int srcWidth, Type* destPixelData, int destWidth)
{
	int i = 0;
	int width = destWidth;
	Type pixel = 0;
	Type* srcEnd = srcPixelData + srcWidth;
	// go trough all pixel in this line
	while (srcPixelData < srcEnd)
	{
		pixel = *srcPixelData;
drawpixel:
		// copy the pixel
		*destPixelData++ = pixel;
		width--;
		i += srcWidth;
		if (!width) break;
		// draw pixel once more if necessary
		if (i < destWidth) goto drawpixel;
		// skip pixels when necessary
		do
		{
			i -= destWidth;
			srcPixelData++;
		}
		while (i >= destWidth);
	};
}

SDL_Surface* scaleSurface (SDL_Surface* scr, SDL_Surface* dest, int width, int height)
{
	if (width <= 0 || height <= 0 || !scr) return NULL;
	SDL_Surface* surface;

	//can not enlage an existing surface
	if (width > scr->w && dest) width = scr->w;
	if (height > scr->h && dest) height = scr->h;

	// generate new surface if necessary
	if (dest == NULL) surface = SDL_CreateRGBSurface (scr->flags, width, height, scr->format->BitsPerPixel, scr->format->Rmask, scr->format->Gmask, scr->format->Bmask, scr->format->Amask);
	else
	{
		// else set the size of the old one
		surface = dest;
		surface->w = width;
		surface->h = height;
	}

	if (SDL_MUSTLOCK (scr)) SDL_LockSurface (scr);
	if (SDL_MUSTLOCK (surface)) SDL_LockSurface (surface);

#if 0
	// just blit the surface when the new size is identic to the old one
	if (scr->w == width && scr->h == height)
	{
		SDL_BlitSurface (scr, NULL, surface, NULL);
		return surface;
	}
#endif
	// copy palette when necessary
	if (scr->format->BitsPerPixel == 8 && !dest)
	{
		for (int i = 0; i < 256; i++)
		{
			surface->format->palette->colors[i].r = scr->format->palette->colors[i].r;
			surface->format->palette->colors[i].g = scr->format->palette->colors[i].g;
			surface->format->palette->colors[i].b = scr->format->palette->colors[i].b;
		}
	}

	int srcRow = 0;
	int destRow = 0;
	int i = 0;
	// go trough all rows
	while (srcRow < scr->h)
	{
		Uint8* srcPixelData = static_cast<Uint8*> (scr->pixels) + (srcRow * scr->pitch);
		// draw the complete line
drawline:
		Uint8* destPixelData = static_cast<Uint8*> (surface->pixels) + (destRow * surface->pitch);

		// pay attention to diffrent surface formats
		switch (scr->format->BytesPerPixel)
		{
			case 1:
				drawStetchedLine<Uint8> (srcPixelData, scr->w, destPixelData, surface->w);
				break;
			case 2:
				drawStetchedLine<Uint16> (reinterpret_cast<Uint16*> (srcPixelData), scr->w, (Uint16*) destPixelData, surface->w);
				break;
			case 3:
				// not yet supported
				break;
			case 4:
				drawStetchedLine<Uint32> (reinterpret_cast<Uint32*> (srcPixelData), scr->w, (Uint32*) destPixelData, surface->w);
				break;
		}
		destRow++;
		i += scr->h;
		// break when we have already finished
		if (destRow == surface->h) break;
		// draw the line once more when the destiniation surface has
		// a bigger height than the source surface
		if (i < surface->h) goto drawline;
		// skip lines in the source surface when the destination surface has
		// a smaller height than the source surface
		do
		{
			i -= surface->h;
			srcRow++;
		}
		while (i >= surface->h);
	}

	if (SDL_MUSTLOCK (scr)) SDL_UnlockSurface (scr);
	if (SDL_MUSTLOCK (surface)) SDL_UnlockSurface (surface);

	return surface;
}

// CreatePfeil ////////////////////////////////////////////////////////////////
// Erzeigt ein Pfeil-Surface:
SDL_Surface* CreatePfeil (int p1x, int p1y, int p2x, int p2y, int p3x, int p3y, unsigned int color, int size)
{
	SDL_Surface* sf;
	sf = SDL_CreateRGBSurface (Video.getSurfaceType() | SDL_SRCCOLORKEY, size, size, Video.getColDepth(), 0, 0, 0, 0);
	SDL_SetColorKey (sf, SDL_SRCCOLORKEY, 0xFF00FF);
	SDL_FillRect (sf, NULL, 0xFF00FF);
	SDL_LockSurface (sf);

	const float fak = size / 64.0f;
	p1x = (int) Round (p1x * fak);
	p1y = (int) Round (p1y * fak);
	p2x = (int) Round (p2x * fak);
	p2y = (int) Round (p2y * fak);
	p3x = (int) Round (p3x * fak);
	p3y = (int) Round (p3y * fak);
	line (p1x, p1y, p2x, p2y, color, sf);
	line (p2x, p2y, p3x, p3y, color, sf);
	line (p3x, p3y, p1x, p1y, color, sf);

	SDL_UnlockSurface (sf);
	return sf;
}


void line (int x1, int y1, int x2, int y2, unsigned int color, SDL_Surface* sf)
{
	if (x2 < x1)
	{
		std::swap (x1, x2);
		std::swap (y1, y2);
	}
	int dx = x2 - x1;
	int dy = y2 - y1;
	int dir = 1;
	if (dy < 0) {dy = -dy; dir = -1;}
	int error = 0;
	Uint32* ptr = static_cast<Uint32*> (sf->pixels);
	if (dx > dy)
	{
		for (; x1 != x2; x1++, error += dy)
		{
			if (error > dx) {error -= dx; y1 += dir;}
			if (x1 < sf->w && x1 >= 0 && y1 >= 0 && y1 < sf->h)
				ptr[x1 + y1 * sf->w] = color;
		}
		return;
	}
	for (; y1 != y2; y1 += dir, error += dx)
	{
		if (error > dy) {error -= dy; x1++;}
		if (x1 < sf->w && x1 >= 0 && y1 >= 0 && y1 < sf->h)
			ptr[x1 + y1 * sf->w] = color;
	}
}
void drawCircle (int iX, int iY, int iRadius, int iColor, SDL_Surface* surface)
{
	if (iX + iRadius < 0 || iX - iRadius > Video.getResolutionX() ||
		iY + iRadius < 0 || iY - iRadius > Video.getResolutionY()) return;
	SDL_LockSurface (surface);

	int d = 0;
	int xx = 0;
	int yy = iRadius;
	int bry = (int) Round (0.70710678 * iRadius, 0);
	while (yy > bry)
	{
		int da = d + (xx << 1) + 1;
		int db = da - (yy << 1) + 1;
		if (abs (da) < abs (db))
		{
			d = da;
			xx++;
		}
		else
		{
			d = db;
			xx++;
			yy--;
		}
		setPixel (surface, iX + xx, iY + yy, iColor);
		setPixel (surface, iX + yy, iY + xx, iColor);
		setPixel (surface, iX + yy, iY - xx, iColor);
		setPixel (surface, iX + xx, iY - yy, iColor);
		setPixel (surface, iX - xx, iY + yy, iColor);
		setPixel (surface, iX - yy, iY + xx, iColor);
		setPixel (surface, iX - yy, iY - xx, iColor);
		setPixel (surface, iX - xx, iY - yy, iColor);
	}
	SDL_UnlockSurface (surface);
}

void setPixel (SDL_Surface* surface, int x, int y, int iColor)
{
	//check the surface size
	if (x < 0 || x >= surface->w || y < 0 || y >= surface->h) return;
	//check the clip rect
	if (x < surface->clip_rect.x || x >= surface->clip_rect.x + surface->clip_rect.w ||
		y < surface->clip_rect.y || y >= surface->clip_rect.y + surface->clip_rect.h) return;

	static_cast<Uint32*> (surface->pixels) [x + y * surface->w] = iColor;
}

int random (int const x)
{
	return (int) ( (double) rand() / RAND_MAX * x);
}

string iToStr (int x)
{
	stringstream strStream;
	strStream << x;
	return strStream.str();
}

string iToHex (unsigned int x)
{
	stringstream strStream;
	strStream << std::hex << x;
	return strStream.str();
}

string dToStr (double x)
{
	stringstream strStream;
	strStream << x;
	return strStream.str();
}

std::string pToStr (void* x)
{
	stringstream strStream;
	strStream << x;
	return "0x" + strStream.str();
}

// Round //////////////////////////////////////////////////////////////////////
// Rounds a Number to 'iDecimalPlace' digits after the comma:
double Round (double dValueToRound, unsigned int iDecimalPlace)
{
	dValueToRound *= pow (10., (int) iDecimalPlace);
	if (dValueToRound >= 0)
		dValueToRound = floor (dValueToRound + 0.5);
	else
		dValueToRound = ceil (dValueToRound - 0.5);
	dValueToRound /= pow (10., (int) iDecimalPlace);
	return dValueToRound;
}

int Round (double dValueToRound)
{
	return (int) Round (dValueToRound, 0);
}


//------------------------------------------------------------------------------
// ----------- sID Implementation ----------------------------------------------
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
string sID::getText() const
{
	char tmp[6];
	snprintf (tmp, sizeof (tmp), "%.2d %.2d", iFirstPart, iSecondPart);
	return tmp;
}

//------------------------------------------------------------------------------
void sID::generate (const string& text)
{
	iFirstPart = atoi (text.substr (0, text.find (" ", 0)).c_str());
	iSecondPart = atoi (text.substr (text.find (" ", 0), text.length()).c_str());
}

//------------------------------------------------------------------------------
sUnitData* sID::getUnitDataOriginalVersion (cPlayer* Owner) const
{
	switch (iFirstPart)
	{
		case 0:
			if (!getVehicle (Owner)) return NULL;
			return &getVehicle (Owner)->data;
		case 1:
			if (!getBuilding (Owner)) return NULL;
			return &getBuilding (Owner)->data;
		default:
			return NULL;
	}
	return NULL;
}

//------------------------------------------------------------------------------
sVehicle* sID::getVehicle (cPlayer* Owner) const
{
	if (iFirstPart != 0) return NULL;
	for (unsigned int i = 0; i < UnitsData.getNrVehicles(); i++)
	{
		sVehicle* result = Owner ? &UnitsData.getVehicle (i, Owner->getClan()) : &UnitsData.getVehicle (i);
		if (result->data.ID == *this) return result;
	}
	return NULL;
}

//------------------------------------------------------------------------------
sBuilding* sID::getBuilding (cPlayer* Owner) const
{
	if (iFirstPart != 1) return NULL;
	for (unsigned int i = 0; i < UnitsData.getNrBuildings(); i++)
	{
		sBuilding* result = Owner ? &UnitsData.getBuilding (i, Owner->getClan()) : &UnitsData.getBuilding (i);
		if (result->data.ID == *this) return result;
	}
	return NULL;
}

//------------------------------------------------------------------------------
bool sID::operator == (const sID& ID) const
{
	if (iFirstPart == ID.iFirstPart && iSecondPart == ID.iSecondPart) return true;
	return false;
}

//------------------------------------------------------------------------------
cUnitsData::cUnitsData() :
	initializedClanUnitData (false)
{
}

//------------------------------------------------------------------------------
sVehicle& cUnitsData::getVehicle (int nr, int clan)
{
	if (!initializedClanUnitData)
		initializeClanUnitData();

	if (clan < 0 || clan > (int) clanUnitDataVehicles.size())
	{
		return vehicle[nr];
	}
	return clanUnitDataVehicles.at (clan).at (nr);   //[clan][nr];
}

//------------------------------------------------------------------------------
sBuilding& cUnitsData::getBuilding (int nr, int clan)
{
	if (!initializedClanUnitData)
		initializeClanUnitData();

	if (clan < 0 || clan > (int) clanUnitDataBuildings.size())
	{
		return building[nr];
	}
	return clanUnitDataBuildings.at (clan).at (nr);   //[clan][nr];
}

unsigned int cUnitsData::getNrVehicles() const
{
	return (int) vehicle.size();
}

unsigned int cUnitsData::getNrBuildings() const
{
	return (int) building.size();
}

static int getConstructorIndex()
{
	for (unsigned int i = 0; i != UnitsData.getNrVehicles (); ++i)
	{
		const sVehicle& vehicle = UnitsData.vehicle[i];

		if (vehicle.data.canBuild.compare ("BigBuilding") == 0)
		{
			return i;
		}
	}
	return -1;
}

static int getEngineerIndex()
{
	for (unsigned int i = 0; i != UnitsData.getNrVehicles(); ++i)
	{
		const sVehicle& vehicle = UnitsData.vehicle[i];

		if (vehicle.data.canBuild.compare ("SmallBuilding") == 0)
		{
			return i;
		}
	}
	return -1;
}

static int getSurveyorIndex()
{
	for (unsigned int i = 0; i != UnitsData.getNrVehicles (); ++i)
	{
		const sVehicle& vehicle = UnitsData.vehicle[i];

		if (vehicle.data.canSurvey)
		{
			return i;
		}
	}
	return -1;
}

void cUnitsData::initializeIDData()
{
	int constructorIndex = getConstructorIndex();
	int engineerIndex = getEngineerIndex();
	int surveyorIndex = getSurveyorIndex();

	assert (constructorIndex != -1);
	assert (engineerIndex != -1);
	assert (surveyorIndex != -1);

	constructorID = vehicle[constructorIndex].data.ID;
	engineerID = vehicle[engineerIndex].data.ID;
	surveyorID = vehicle[surveyorIndex].data.ID;
}

//------------------------------------------------------------------------------
void cUnitsData::initializeClanUnitData()
{
	if (initializedClanUnitData == true) return;

	cClanData& clanData = cClanData::instance();
	for (int clanIdx = 0; clanIdx < clanData.getNrClans(); clanIdx++)
	{
		cClan* clan = clanData.getClan (clanIdx);
		if (clan == 0)
			continue;

		clanUnitDataVehicles.push_back (std::vector<sVehicle>());
		vector<sVehicle>& clanListVehicles = clanUnitDataVehicles.back();
		for (unsigned int vehicleIdx = 0; vehicleIdx < vehicle.size(); vehicleIdx++)
		{
			// make a copy of the vehicle's stats
			const sVehicle& curVehicle = vehicle[vehicleIdx];
			clanListVehicles.push_back (curVehicle);

			cClanUnitStat* changedStat = clan->getUnitStat (curVehicle.data.ID.iFirstPart, curVehicle.data.ID.iSecondPart);
			if (changedStat == NULL) continue;

			sVehicle& clanVehicle = clanListVehicles.back();
			if (changedStat->hasModification ("Damage"))
				clanVehicle.data.damage = changedStat->getModificationValue ("Damage");
			if (changedStat->hasModification ("Range"))
				clanVehicle.data.range = changedStat->getModificationValue ("Range");
			if (changedStat->hasModification ("Armor"))
				clanVehicle.data.armor = changedStat->getModificationValue ("Armor");
			if (changedStat->hasModification ("Hitpoints"))
				clanVehicle.data.hitpointsMax = changedStat->getModificationValue ("Hitpoints");
			if (changedStat->hasModification ("Scan"))
				clanVehicle.data.scan = changedStat->getModificationValue ("Scan");
			if (changedStat->hasModification ("Speed"))
				clanVehicle.data.speedMax = changedStat->getModificationValue ("Speed") * 4;
			if (changedStat->hasModification ("Built_Costs"))
				clanVehicle.data.buildCosts = changedStat->getModificationValue ("Built_Costs");
		}

		clanUnitDataBuildings.push_back (std::vector<sBuilding>());
		vector<sBuilding>& clanListBuildings = clanUnitDataBuildings.back();
		for (unsigned int buildingIdx = 0; buildingIdx < building.size(); buildingIdx++)
		{
			// make a copy of the building's stats
			const sBuilding& curBuilding = building[buildingIdx];
			clanListBuildings.push_back (curBuilding);

			cClanUnitStat* changedStat = clan->getUnitStat (curBuilding.data.ID.iFirstPart, curBuilding.data.ID.iSecondPart);
			if (changedStat == NULL) continue;
			sBuilding& clanBuilding = clanListBuildings.back();
			if (changedStat->hasModification ("Damage"))
				clanBuilding.data.damage = changedStat->getModificationValue ("Damage");
			if (changedStat->hasModification ("Range"))
				clanBuilding.data.range = changedStat->getModificationValue ("Range");
			if (changedStat->hasModification ("Armor"))
				clanBuilding.data.armor = changedStat->getModificationValue ("Armor");
			if (changedStat->hasModification ("Hitpoints"))
				clanBuilding.data.hitpointsMax = changedStat->getModificationValue ("Hitpoints");
			if (changedStat->hasModification ("Scan"))
				clanBuilding.data.scan = changedStat->getModificationValue ("Scan");
			if (changedStat->hasModification ("Speed"))
				clanBuilding.data.speedMax = changedStat->getModificationValue ("Speed") * 4;
			if (changedStat->hasModification ("Built_Costs"))
				clanBuilding.data.buildCosts = changedStat->getModificationValue ("Built_Costs");
		}
	}

	initializedClanUnitData = true;
}

//------------------------------------------------------------------------------
sUnitData::sUnitData()
{
	version = 0;
	muzzleType = MUZZLE_TYPE_NONE;

	ammoMax = 0;
	ammoCur = 0;
	shotsMax = 0;
	shotsCur = 0;
	range = 0;
	damage = 0;
	canAttack = 0;
	canDriveAndFire = false;

	buildCosts = 0;
	maxBuildFactor = 0;

	canBuildPath = false;
	canBuildRepeat = false;
	buildIntern = false;

	// Movement
	speedMax = 0;
	speedCur = 0;

	factorGround = 0.0f;
	factorSea = 0.0f;
	factorAir = 0.0f;
	factorCoast = 0.0f;

	// Abilities
	isBig = false;
	connectsToBase = false;
	armor = 0;
	hitpointsMax = 0;
	hitpointsCur = 0;
	scan = 0;
	modifiesSpeed = 0.0f;
	canClearArea = false;
	canBeCaptured = false;
	canBeDisabled = false;
	canCapture = false;
	canDisable = false;
	canRepair = false;
	canRearm = false;
	canResearch = false;
	canPlaceMines = false;
	canSurvey = false;
	doesSelfRepair = false;
	convertsGold = 0;
	canSelfDestroy = false;
	canScore = false;

	canMineMaxRes = 0;
	needsMetal = 0;
	needsOil = 0;
	needsEnergy = 0;
	needsHumans = 0;
	produceEnergy = 0;
	produceHumans = 0;

	isStealthOn = 0;
	canDetectStealthOn = 0;

	surfacePosition = SURFACE_POS_BENEATH_SEA;
	canBeOverbuild = OVERBUILD_TYPE_NO;

	canBeLandedOn = false;
	canWork = false;
	explodesOnContact = false;
	isHuman = false;

	// Storage
	storageResMax = 0;
	storageResCur = 0;
	storeResType = STORE_RES_NONE;
	storageUnitsMax = 0;
	storageUnitsCur = 0;
	storeUnitsImageType = STORE_UNIT_IMG_NONE;

	// Graphic
	hasClanLogos = false;
	hasCorpse = false;
	hasDamageEffect = false;
	hasBetonUnderground = false;
	hasPlayerColor = false;
	hasOverlay = false;

	buildUpGraphic = false;
	animationMovement = false;
	powerOnGraphic = false;
	isAnimated = false;
	makeTracks = false;

	isConnectorGraphic = false;
	hasFrames = 0;
}

//------------------------------------------------------------------------------
void blittPerSurfaceAlphaToAlphaChannel (SDL_Surface* src, SDL_Rect* srcrect, SDL_Surface* dst, SDL_Rect* dstrect)
{
	SDL_Rect temp1, temp2;

	if (!dst || !src) return;

	//check surface formats
	if (!dst->format->Amask) return;
	if (src->format->Amask || ! (src->flags & SDL_SRCALPHA)) return;

	if (srcrect == NULL)
	{
		srcrect = &temp1;
		srcrect->x = 0;
		srcrect->y = 0;
		srcrect->h = src->h;
		srcrect->w = src->w;
	}

	if (dstrect == NULL)
	{
		dstrect = &temp2;
		dstrect->x = 0;
		dstrect->y = 0;
	}

	int width  = srcrect->w;
	int height = srcrect->h;

	//clip source rect to the source surface
	if (srcrect->x < 0)
	{
		width += srcrect->x;
		srcrect->x = 0;
	}
	if (srcrect->y < 0)
	{
		height += srcrect->y;
		srcrect->y = 0;
	}
	width  = std::min (src->w - srcrect->x, width);
	height = std::min (src->h - srcrect->y, height);

	//clip the dest rect to the destination clip rect
	if (dstrect->x < dst->clip_rect.x)
	{
		width -= dst->clip_rect.x - dstrect->x;
		srcrect->x += dst->clip_rect.x - dstrect->x;
		dstrect->x = dst->clip_rect.x;
	}
	if (dstrect->y < dst->clip_rect.y)
	{
		height -= dst->clip_rect.y - dstrect->y;
		srcrect->y += dst->clip_rect.y - dstrect->y;
		dstrect->y = dst->clip_rect.y;
	}
	width  = std::min (dst->clip_rect.x + dst->clip_rect.w - dstrect->x, width);
	height = std::min (dst->clip_rect.y + dst->clip_rect.h - dstrect->y, height);

	if (width <= 0 || height <= 0)
	{
		dstrect->w = 0;
		dstrect->h = 0;
		return;
	}

	if (SDL_MUSTLOCK (src)) SDL_LockSurface (src);
	if (SDL_MUSTLOCK (dst)) SDL_LockSurface (dst);

	//setup needed variables
	const Uint32 srcAlpha = src->format->alpha;
	const int srmask = src->format->Rmask;
	const int sgmask = src->format->Gmask;
	const int sbmask = src->format->Bmask;
	const int damask = dst->format->Amask;
	const int drmask = dst->format->Rmask;
	const int dgmask = dst->format->Gmask;
	const int dbmask = dst->format->Bmask;
	const int rshift = src->format->Rshift - dst->format->Rshift;
	const int gshift = src->format->Gshift - dst->format->Gshift;
	const int bshift = src->format->Bshift - dst->format->Bshift;
	const int ashift = dst->format->Ashift;
	const Uint32 colorKey = src->format->colorkey;
	const bool useColorKey = (src->flags & SDL_SRCCOLORKEY) != 0;

	Uint32* dstPixel = static_cast<Uint32*> (dst->pixels) + dstrect->x + dstrect->y * dst->w;
	Uint32* srcPixel = static_cast<Uint32*> (src->pixels) + srcrect->x + srcrect->y * src->w;

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			const Uint32 dcolor = *dstPixel;
			const Uint32 scolor = *srcPixel;

			if (useColorKey && scolor == colorKey)
			{
				dstPixel++;
				srcPixel++;
				continue;
			}

			Uint32 r = (scolor & srmask) >> rshift;
			Uint32 g = (scolor & sgmask) >> gshift;
			Uint32 b = (scolor & sbmask) >> bshift;
			Uint32 dalpha = (dcolor & damask) >> ashift;

			r = ( ( (dcolor & drmask) >> 8) * (255 - srcAlpha) * dalpha) + r * srcAlpha;
			g = ( ( (dcolor & dgmask) * (255 - srcAlpha) * dalpha) >> 8) + g * srcAlpha;
			b = ( ( (dcolor & dbmask) * (255 - srcAlpha) * dalpha) >> 8) + b * srcAlpha;

			const Uint8 a = srcAlpha + dalpha - (srcAlpha * dalpha) / 255;

			if (a > 0)
			{
				r /= a;
				g /= a;
				b /= a;
			}

			r = r & drmask;
			g = g & dgmask;
			b = b & dbmask;

			*dstPixel = r | g | b | a << ashift;

			dstPixel++;
			srcPixel++;
		}

		dstPixel += dst->pitch / 4 - width;
		srcPixel += src->pitch / 4 - width;
	}

	if (SDL_MUSTLOCK (src)) SDL_UnlockSurface (src);
	if (SDL_MUSTLOCK (dst)) SDL_UnlockSurface (dst);
}

void blittAlphaSurface (SDL_Surface* src, SDL_Rect* srcrect, SDL_Surface* dst, SDL_Rect* dstrect)
{
	if (dst->format->Amask && (src->flags & SDL_SRCALPHA))
		blittPerSurfaceAlphaToAlphaChannel (src, srcrect, dst, dstrect);
	else
		SDL_BlitSurface (src, srcrect, dst, dstrect);
}

sFreezeModes::sFreezeModes() :
	waitForOthers (false),
	waitForServer (false),
	waitForReconnect (false),
	waitForTurnEnd (false),
	pause (false),
	waitForPlayer (false),
	playerNumber (-1)
{}
