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

#include "uimain.h"

#include "3rd/mveplayer/mveplayer.h"

#include "debug.h"
#include "defines.h"
#include "input/mouse/mouse.h"
#include "input/keyboard/keyboard.h"
#include "maxrversion.h"
#include "output/sound/sounddevice.h"
#include "output/video/video.h"
#include "resources/loaddata.h"
#include "ui/graphical/application.h"
#include "ui/graphical/menu/windows/windowstart.h"
#include "utility/files.h"
#include "utility/log.h"
#include "utility/thread/ismainthread.h"

#include "SDLutility/sdlcomponent.h"
#include "SDLutility/sdlnetcomponent.h"

#include <SDL_mixer.h>

#include <future>

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
	//FIXME: make this case sensitive - my mve is e.g. completely lower cases -- beko

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

struct AtExit
{
	~AtExit()
	{
		//unload files here
		cSoundDevice::getInstance().close();

		Video.clearMemory();
		Log.write ("EOF");
	}
};

int uimain (int argc, char* argv[])
try
{
	AtExit exitGuard; // Will clean data global data in its destructor

	if (!cSettings::getInstance().isInitialized())
	{
		return -1;
	}

	CR_INIT_CRASHREPORTING();
	is_main_thread();
	logMAXRVersion();

	SDLComponent sdlComponent (true);
	SDLNetComponent sdlNetComponent;

	Video.init();
	Video.showSplashScreen();
	initSound(); // now config is loaded and we can init sound and net

	// load files
	std::future<eLoadingState> dataThread = std::async(std::launch::async, &LoadData);
	using namespace std::literals;

	SDL_Event event;
	while (dataThread.wait_for(100ms) == std::future_status::timeout)
	{
		while (SDL_PollEvent (&event))
		{
			if (event.type == SDL_WINDOWEVENT
				&& event.window.event == SDL_WINDOWEVENT_EXPOSED)
			{
				Video.draw();
			}
		}
		// The draw may be conditioned when screen has changed.
		Video.draw();
	}
	Video.draw();
	std::this_thread::sleep_for(1s); // time to see loading status

	if (dataThread.get() == eLoadingState::Error)
	{
		Log.write ("Error while loading data!", cLog::eLOG_TYPE_ERROR);
		return -1;
	}

	// play intro if we're supposed to and the file exists
	if (cSettings::getInstance().shouldShowIntro())
	{
		showIntro();
	}
	else
	{
		Log.write ("Skipped intro movie due settings", cLog::eLOG_TYPE_DEBUG);
	}
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

	return 0;
}
catch (const std::exception&)
{
	return -1;
}