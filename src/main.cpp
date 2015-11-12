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

#include <ctime>
#include <cmath>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <algorithm>

#include <SDL.h>
#include <SDL_thread.h>
#include <SDL_net.h>
#include <SDL_mixer.h>

#define __main__
#include "main.h"

#include "game/data/base/base.h"
#include "game/data/units/building.h"
#include "game/data/player/clans.h"
#include "dedicatedserver.h"
#include "utility/files.h"
#include "keys.h"
#include "loaddata.h"
#include "utility/log.h"
#include "game/data/map/map.h"
#include "mveplayer.h"
#include "network.h"
#include "pcx.h"
#include "game/data/player/player.h"
#include "settings.h"
#include "sound.h"
#include "unifonts.h"
#include "game/data/units/vehicle.h"
#include "video.h"
#include "maxrversion.h"
#include "input/mouse/mouse.h"
#include "input/keyboard/keyboard.h"

#include "output/sound/sounddevice.h"

#include "ui/graphical/application.h"
#include "ui/graphical/menu/windows/windowstart.h"

using namespace std;

static int initNet();
static int initSDL();
static int initSound();
static void logMAXRVersion();
static void showIntro();

int main (int argc, char* argv[])
{
	if (!cSettings::getInstance().isInitialized())
	{
		Quit();
		return -1;
	}
	// stop on error during init of SDL basics. WARNINGS will be ignored!
	if (initSDL() == -1) return -1;

	// call it once to initialize
	is_main_thread();

	logMAXRVersion();

	if (!DEDICATED_SERVER)
	{
		Video.init();
		Video.showSplashScreen(); // show splashscreen
		initSound(); // now config is loaded and we can init sound and net
	}
	initNet();

	// load files
	volatile int loadingState = LOAD_GOING;
	SDL_Thread* dataThread = SDL_CreateThread (LoadData, "loadingData", const_cast<int*> (&loadingState));

	SDL_Event event;
	while (loadingState != LOAD_FINISHED)
	{
		if (loadingState == LOAD_ERROR)
		{
			Log.write ("Error while loading data!", cLog::eLOG_TYPE_ERROR);
			SDL_WaitThread (dataThread, nullptr);
			Quit();
		}
		while (SDL_PollEvent (&event))
		{
			if (!DEDICATED_SERVER
				&& event.type == SDL_WINDOWEVENT
				&& event.window.event == SDL_WINDOWEVENT_EXPOSED)
			{
				Video.draw();
			}
		}
		SDL_Delay (100);
		if (!DEDICATED_SERVER)
		{
			// The draw may be conditionned when screen has changed.
			Video.draw();
		}
	}

	if (!DEDICATED_SERVER)
	{
		// play intro if we're supposed to and the file exists
		if (cSettings::getInstance().shouldShowIntro())
		{
			showIntro();
		}
		else
		{
			Log.write ("Skipped intro movie due settings", cLog::eLOG_TYPE_DEBUG);
		}
	}

	SDL_WaitThread (dataThread, nullptr);

	if (DEDICATED_SERVER)
	{
		cDedicatedServer::instance().run();
	}
	else
	{
		Video.prepareGameScreen();
		Video.clearBuffer();

		cMouse mouse;
		cKeyboard keyboard;

		cApplication application;

		application.registerMouse (mouse);
		application.registerKeyboard (keyboard);

		auto startWindow = std::make_shared<cWindowStart>();
		application.show (startWindow);

		mouse.show();

		application.execute();
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

	try
	{
		cSoundDevice::getInstance().initialize (cSettings::getInstance().getFrequency(), cSettings::getInstance().getChunkSize());
	}
	catch (std::runtime_error& e)
	{
		Log.write ("Could not init SDL_mixer:", cLog::eLOG_TYPE_WARNING);
		Log.write (e.what(), cLog::eLOG_TYPE_WARNING);
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

static void logMAXRVersion()
{
	std::string sVersion = PACKAGE_NAME; sVersion += " ";
	sVersion += PACKAGE_VERSION; sVersion += " ";
	sVersion += PACKAGE_REV; sVersion += " ";
	Log.write (sVersion, cLog::eLOG_TYPE_INFO);
	std::string sBuild = "Build: "; sBuild += MAX_BUILD_DATE;
	Log.write (sBuild, cLog::eLOG_TYPE_INFO);
	Log.mark();
	Log.write (sVersion, cLog::eLOG_TYPE_NET_DEBUG);
	Log.write (sBuild, cLog::eLOG_TYPE_NET_DEBUG);
}



static void showIntro()
{
	const std::string filename = cSettings::getInstance().getMvePath() + PATH_DELIMITER + "MAXINT.MVE";

	if (!FileExists (filename.c_str()))
	{
		Log.write ("Couldn't find movie " + filename, cLog::eLOG_TYPE_WARNING);
	}
	// Close maxr sound for intro movie
	cSoundDevice::getInstance().close();

	Log.write ("Starting movie " + filename, cLog::eLOG_TYPE_DEBUG);
	const int mvereturn = MVEPlayer (filename.c_str(),
									 Video.getResolutionX(), Video.getResolutionY(),
									 !Video.getWindowMode(),
									 !cSettings::getInstance().isSoundMute());
	Log.write ("MVEPlayer returned " + iToStr (mvereturn), cLog::eLOG_TYPE_DEBUG);
	//FIXME: make this case sensitive - my mve is e.g. completly lower cases -- beko

	// reinit maxr sound
	if (cSettings::getInstance().isSoundEnabled())
	{
		try
		{
			cSoundDevice::getInstance().initialize (cSettings::getInstance().getFrequency(), cSettings::getInstance().getChunkSize());
		}
		catch (std::runtime_error& e)
		{
			Log.write ("Can't reinit sound after playing intro" + iToStr (mvereturn), cLog::eLOG_TYPE_DEBUG);
			Log.write (e.what(), cLog::eLOG_TYPE_DEBUG);
		}
	}
}

/**
 * Return if it is the main thread.
 * @note: should be called by main once by the main thread to initialize.
 */
bool is_main_thread()
{
	static const SDL_threadID main_thread_id = SDL_ThreadID();
	return main_thread_id == SDL_ThreadID();
}

/**
 *Terminates app
 *@author beko
 */
void Quit()
{
	delete font;

	//unload files here
	cSoundDevice::getInstance().close();
	SDLNet_Quit();
	Video.clearMemory();
	SDL_Quit();
	Log.write ("EOF");
	exit (0);
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

string fToStr (float x)
{
	stringstream strStream;
	strStream << x;
	return strStream.str();
}

std::string pToStr (const void* x)
{
	stringstream strStream;
	strStream << x;
	return "0x" + strStream.str();
}

std::string bToStr (bool x)
{
	return x ? "true" : "false";
}

// Round //////////////////////////////////////////////////////////////////////
// Rounds a Number to 'iDecimalPlace' digits after the comma:
float Round (float dValueToRound, unsigned int iDecimalPlace)
{
	dValueToRound *= powf (10.f, (int) iDecimalPlace);
	if (dValueToRound >= 0)
		dValueToRound = floorf (dValueToRound + 0.5f);
	else
		dValueToRound = ceilf (dValueToRound - 0.5f);
	dValueToRound /= powf (10.f, (int) iDecimalPlace);
	return dValueToRound;
}

int Round (float dValueToRound)
{
	return (int) Round (dValueToRound, 0);
}

//------------------------------------------------------------------------------
cUnitsUiData::cUnitsUiData() :
	ptr_small_beton(0),
	ptr_small_beton_org(0),
	ptr_connector(0),
	ptr_connector_org(0),
	ptr_connector_shw(0),
	ptr_connector_shw_org(0)
{}

//------------------------------------------------------------------------------
void cUnitsUiData::scaleSurfaces(float zoomFactor)
{
	// Vehicles:
	for (unsigned int i = 0; i < vehicleUIs.size(); ++i)
	{
		vehicleUIs[i].scaleSurfaces(zoomFactor);
	}
	// Buildings:
	for (unsigned int i = 0; i < buildingUIs.size(); ++i)
	{
		buildingUIs[i].scaleSurfaces(zoomFactor);
	}

	if (dirt_small_org != nullptr && dirt_small != nullptr) scaleSurface(dirt_small_org.get(), dirt_small.get(), (int)(dirt_small_org->w * zoomFactor), (int)(dirt_small_org->h * zoomFactor));
	if (dirt_small_shw_org != nullptr && dirt_small_shw != nullptr) scaleSurface(dirt_small_shw_org.get(), dirt_small_shw.get(), (int)(dirt_small_shw_org->w * zoomFactor), (int)(dirt_small_shw_org->h * zoomFactor));
	if (dirt_big_org != nullptr && dirt_big != nullptr) scaleSurface(dirt_big_org.get(), dirt_big.get(), (int)(dirt_big_org->w * zoomFactor), (int)(dirt_big_org->h * zoomFactor));
	if (dirt_big_shw_org != nullptr && dirt_big_shw != nullptr) scaleSurface(dirt_big_shw_org.get(), dirt_big_shw.get(), (int)(dirt_big_shw_org->w * zoomFactor), (int)(dirt_big_shw_org->h * zoomFactor));
}

//------------------------------------------------------------------------------
const sBuildingUIData* cUnitsUiData::getBuildingUI(sID id) const
{
	for (unsigned int i = 0; i < buildingUIs.size(); ++i)
	{
		if (buildingUIs[i].id == id)
			return &buildingUIs[i];
	}
}

//------------------------------------------------------------------------------
const sVehicleUIData* cUnitsUiData::getVehicleUI(sID id) const
{
	for (unsigned int i = 0; i < vehicleUIs.size(); ++i)
	{
		if (vehicleUIs[i].id == id)
			return &vehicleUIs[i];
	}
}

//------------------------------------------------------------------------------
sFreezeModes::sFreezeModes() :
	waitForOthers (false),
	waitForServer (false),
	waitForReconnect (false),
	waitForTurnEnd (false),
	pause (false),
	waitForPlayer (false),
	playerNumber (-1)
{}

void sFreezeModes::enable (eFreezeMode mode, int playerNumber_)
{
	switch (mode)
	{
		case FREEZE_WAIT_FOR_SERVER: waitForServer = true; break;
		case FREEZE_WAIT_FOR_OTHERS: waitForOthers = true; break;
		case FREEZE_PAUSE: pause = true; break;
		case FREEZE_WAIT_FOR_RECONNECT: waitForReconnect = true; break;
		case FREEZE_WAIT_FOR_TURNEND: waitForTurnEnd = true; break;
		case FREEZE_WAIT_FOR_PLAYER: waitForPlayer = true; break;
	}

	if (playerNumber_ != -1)
		playerNumber = playerNumber_;
}

void sFreezeModes::disable (eFreezeMode mode)
{
	switch (mode)
	{
		case FREEZE_WAIT_FOR_SERVER: waitForServer = false; break;
		case FREEZE_WAIT_FOR_OTHERS: waitForOthers = false; break;
		case FREEZE_PAUSE: pause = false; break;
		case FREEZE_WAIT_FOR_RECONNECT: waitForReconnect = false; break;
		case FREEZE_WAIT_FOR_TURNEND: waitForTurnEnd = false; break;
		case FREEZE_WAIT_FOR_PLAYER: waitForPlayer = false; break;
	}
}

bool sFreezeModes::isFreezed() const
{
	return waitForServer | waitForOthers | pause |
		   waitForReconnect | waitForTurnEnd | waitForPlayer;
}

bool sFreezeModes::isEnable (eFreezeMode mode) const
{
	switch (mode)
	{
		case FREEZE_WAIT_FOR_SERVER: return waitForServer;
		case FREEZE_WAIT_FOR_OTHERS: return waitForOthers;
		case FREEZE_PAUSE: return pause;
		case FREEZE_WAIT_FOR_RECONNECT: return waitForReconnect;
		case FREEZE_WAIT_FOR_TURNEND: return waitForTurnEnd;
		case FREEZE_WAIT_FOR_PLAYER: return waitForPlayer;
	}
	assert (0); // Incorrect parameter
	return false;
}

//--------------------------------------------------------------------------
std::string getHexValue(unsigned char byte)
{
	std::string str = "";
	const char hexChars[] = "0123456789ABCDEF";
	const unsigned char high = (byte >> 4) & 0x0F;
	const unsigned char low = byte & 0x0F;

	str += hexChars[high];
	str += hexChars[low];
	return str;
}
//--------------------------------------------------------------------------
unsigned char getByteValue(const std::string& str, int index)
{
	unsigned char first = str[index + 0] - '0';
	unsigned char second = str[index + 1] - '0';

	if (first >= 'A' - '0') first -= 'A' - '0' - 10;
	if (second >= 'A' - '0') second -= 'A' - '0' - 10;
	return (first * 16 + second);
}
