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

#include "SDLutility/sdlcomponent.h"
#include "SDLutility/sdlnetcomponent.h"
#include "SDLutility/sdlversion.h"
#include "crashreporter/debug.h"
#include "dedicatedserver/dedicatedserver.h"
#include "defines.h"
#include "maxrversion.h"
#include "resources/loaddata.h"
#include "settings.h"
#include "utility/log.h"

//------------------------------------------------------------------------------
int main (int, char**)
try
{
	if (!cSettings::getInstance().isInitialized())
	{
		return -1;
	}
	logMAXRVersion();
	logSDLVersions();
	logNlohmannVersion();
	CR_INIT_CRASHREPORTING();

	SDLComponent sdlComponent (false);
	SDLNetComponent sdlNetComponent;

	if (LoadData (false) == eLoadingState::Error)
	{
		Log.error ("Error while loading data!");
		return -1;
	}
	cDedicatedServer (DEFAULTPORT).run();
	Log.info ("EOF");
	return 0;
}
catch (const std::exception& ex)
{
	Log.error (ex.what());
	return -1;
}
