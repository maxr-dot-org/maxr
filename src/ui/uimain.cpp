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

#include "3rd/mveplayer/mveplayer.h"
#include "SDLutility/sdlcomponent.h"
#include "SDLutility/sdlnetcomponent.h"
#include "SDLutility/sdlversion.h"
#include "crashreporter/debug.h"
#include "defines.h"
#include "input/keyboard/keyboard.h"
#include "input/mouse/mouse.h"
#include "maxrversion.h"
#include "output/sound/sounddevice.h"
#include "output/video/video.h"
#include "resources/loaddata.h"
#include "ui/graphical/intro.h"
#include "ui/graphical/menu/windows/windowstart.h"
#include "ui/uidefines.h"
#include "ui/widgets/application.h"
#include "utility/log.h"
#include "utility/thread/ismainthread.h"

#include <filesystem>
#include <future>
#include <thread>

/**
 *Inits SDL_sound
 *@author beko
 *@return -1 on error<br>0 on success<br>1 with warnings
 */
static int initSound()
{
	if (!cSettings::getInstance().isSoundEnabled())
	{
		Log.info ("Sound disabled due configuration");
		return 1;
	}

	if (SDL_Init (SDL_INIT_AUDIO) < 0) //start sound
	{
		Log.warn ("Could not init SDL_INIT_AUDIO");
		Log.warn ("Sound won't be available!");
		Log.warn (SDL_GetError());
		cSettings::getInstance().setSoundEnabled (false);
		return -1;
	}

	try
	{
		cSoundDevice::getInstance().initialize (cSettings::getInstance().getFrequency(), cSettings::getInstance().getChunkSize());
	}
	catch (std::runtime_error& e)
	{
		Log.warn ("Could not init SDL_mixer:");
		Log.warn (e.what());
		Log.warn ("Sound won't be available!");
		cSettings::getInstance().setSoundEnabled (false);
		return -1;
	}
	Log.info ("Sound started");
	return 0;
}

struct AtExit
{
	~AtExit()
	{
		//unload files here
		cSoundDevice::getInstance().close();

		Video.clearMemory();
		Log.info ("EOF");
	}
};

int main (int, char*[])
try
{
	AtExit exitGuard; // Will clean data global data in its destructor

	if (!cSettings::getInstance().isInitialized())
	{
		return -1;
	}

	applySettings (Video, cSettings::getInstance().getVideoSettings());

	is_main_thread();
	logMAXRVersion();
	logSDLVersions();
	logNlohmannVersion();
	CR_INIT_CRASHREPORTING();

	SDLComponent sdlComponent (true);
	SDLNetComponent sdlNetComponent;

	Video.init (PACKAGE_NAME " " PACKAGE_VERSION " " PACKAGE_REV, MAXR_ICON);
	Video.showSplashScreen (SPLASH_BACKGROUND);
	initSound(); // now config is loaded and we can init sound and net

	// load files
	std::future<eLoadingState> dataThread = std::async (std::launch::async, &LoadData, true);
	using namespace std::literals;

	SDL_Event event;
	while (dataThread.wait_for (100ms) == std::future_status::timeout)
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
	std::this_thread::sleep_for (1s); // time to see loading status

	if (dataThread.get() == eLoadingState::Error)
	{
		Log.error ("Error while loading data!");
		return -1;
	}

	// play intro if we're supposed to and the file exists
	if (cSettings::getInstance().shouldShowIntro())
	{
		showIntro();
	}
	else
	{
		Log.debug ("Skipped intro movie due settings");
	}
	Video.prepareGameScreen();
	Video.clearBuffer();

	cMouse mouse;
	cKeyboard keyboard;
	cApplication application;

	application.registerMouse (mouse);
	application.registerKeyboard (keyboard);

	application.show (std::make_shared<cWindowStart>());
	mouse.show();
	application.execute();

	return 0;
}
catch (const std::exception& ex)
{
	Log.error (ex.what());
	return -1;
}
