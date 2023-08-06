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
#include "output/sound/sounddevice.h"
#include "output/video/video.h"
#include "ui/uidefines.h"
#include "utility/log.h"
#include "utility/random.h"

#include <filesystem>

//------------------------------------------------------------------------------
static void showScene (const std::filesystem::path& filename)
{
	if (!std::filesystem::exists (filename))
	{
		Log.warn ("Couldn't find movie " + filename.u8string());
	}
	// Close maxr sound for movie
	cSoundDevice::getInstance().close();
	const int oldCursorStatus = SDL_ShowCursor (SDL_QUERY);
	SDL_ShowCursor (SDL_DISABLE);

	Log.debug ("Starting movie " + filename.u8string());
	const int mvereturn = MVEPlayer (filename.string().c_str(),
	                                 Video.getResolutionX(),
	                                 Video.getResolutionY(),
	                                 !Video.getWindowMode(),
	                                 MAXR_ICON.string().c_str(),
	                                 !cSettings::getInstance().isSoundMute());
	Log.debug ("MVEPlayer returned " + std::to_string (mvereturn));
	SDL_ShowCursor (oldCursorStatus);

	// reinit maxr sound
	if (cSettings::getInstance().isSoundEnabled())
	{
		try
		{
			cSoundDevice::getInstance().initialize (cSettings::getInstance().getFrequency(), cSettings::getInstance().getChunkSize());
		}
		catch (const std::runtime_error& e)
		{
			Log.debug ("Can't reinit sound after playing scene " + std::to_string (mvereturn));
			Log.debug (e.what());
		}
	}
}

//------------------------------------------------------------------------------
static std::filesystem::path getIntroPath()
{
	return cSettings::getInstance().getMvePath() / "MAXINT.MVE";
}

//------------------------------------------------------------------------------
bool hasIntro()
{
	return std::filesystem::exists (getIntroPath());
}

//------------------------------------------------------------------------------
void showIntro()
{
	showScene (getIntroPath());
}

//------------------------------------------------------------------------------
void showBeginGameScene()
{
	const std::array<std::string, 2> mves{"MAXMVE1.MVE", "MAXMVE2.MVE"};
	showScene (cSettings::getInstance().getMvePath() / getRandom (mves));
}
