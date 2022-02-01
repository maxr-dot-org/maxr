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

#include "settings.h"

#include <iostream>
#include <locale>
#include <string>

#include "defines.h"
#include "game/serialization/jsonarchive.h"
#include "maxrversion.h"
#include "utility/files.h"
#include "utility/log.h"
#include "utility/string/tolower.h"

namespace
{
	//--------------------------------------------------------------------------
	/**
	 * Platform dependent implementations.
	 * On most platforms just the executable folder is used.
	 * On Linux it tries to verify the path from the configuration file
	 * @param sDataDirFromConf The data location that has been read
	 *        from the configuration file.
	 * @return The really selected data location.
	 */
	std::string searchDataDir (const std::string& sDataDirFromConf)
	{
		std::string sPathToGameData = "";
#if MAC
		// assuming data is in same folder as binary (or current working directory)
		sPathToGameData = getCurrentExeDir();
#elif WIN32
		if (!sDataDirFromConf.empty())
		{
			sPathToGameData = sDataDirFromConf;
			sPathToGameData += PATH_DELIMITER;
		}
#elif __amigaos4__
		// assuming data is in same folder as binary (or current working directory)
		sPathToGameData = getCurrentExeDir();
#else
		// BEGIN crude path validation to find gamedata
		Log.write ("Probing for data paths using default values:", cLog::eLOG_TYPE_INFO);

		std::string sPathArray[] =
		{
			// most important position holds value of configure --prefix
			// to gamedata in %prefix%/$(datadir)/maxr or default path
			// if autoversion.h wasn't used
			BUILD_DATADIR,
			"/usr/local/share/maxr",
			"/usr/games/maxr",
			"/usr/local/games/maxr",
			"/usr/maxr",
			"/usr/local/maxr",
			"/opt/maxr",
			"/usr/share/games/maxr",
			"/usr/local/share/games/maxr",
			getCurrentExeDir(), // check for gamedata in bin folder too
			"." // last resort: local dir
		};

		/*
		* Logic is:
		* BUILD_DATADIR is default search path
		* sDataDirFromConf overrides BUILD_DATADIR
		* "$MAXRDATA overrides both
		* BUILD_DATADIR is checked if sDataDirFromConf or $MAXRDATA fail the probe
		*/
		if (!sDataDirFromConf.empty())
		{
			// override default path with path from config
			sPathArray[0] = sDataDirFromConf;
			// and save old value one later in case sDataDirFromConf is invalid
			sPathArray[1] = BUILD_DATADIR;
		}

		// BEGIN SET MAXRDATA
		char* cDataDir;
		cDataDir = getenv ("MAXRDATA");
		if (cDataDir == nullptr)
		{
			Log.write ("$MAXRDATA is not set", cLog::eLOG_TYPE_INFO);
		}
		else
		{
			sPathArray[0] = cDataDir;
			sPathArray[1] = BUILD_DATADIR;
			Log.write ("$MAXRDATA is set and overrides default data search path", cLog::eLOG_TYPE_WARNING);
		}
		// END SET MAXRDATA

		for (auto sInitFile : sPathArray)
		{
			sInitFile += PATH_DELIMITER;
			if (FileExists (sInitFile + "init.pcx"))
			{
				sPathToGameData = sInitFile;
				break;
			}
		}

		// still empty? cry for mama - we couldn't locate any typical data folder
		if (sPathToGameData.empty())
		{
			Log.write ("No success probing for data folder!", cLog::eLOG_TYPE_ERROR);
		}
		else
		{
			Log.write ("Found gamedata in: " + sPathToGameData, cLog::eLOG_TYPE_INFO);
		}
		// END crude path validation to find gamedata
#endif
		return sPathToGameData;
	}

}

//------------------------------------------------------------------------------
cSettings cSettings::instance;

//------------------------------------------------------------------------------
cSettings::cSettings()
{
	network.port = DEFAULTPORT;
#ifdef WIN32
	char* user = getenv ("USERNAME");
#elif __amigaos4__
	char* user = "AmigaOS4-User";
#else
	char* user = getenv ("USER");  //get $USER on linux
#endif

	player.name = (user == nullptr ? "Commander" : user);
	player.color = cRgbColor::red();
}

//------------------------------------------------------------------------------
cSettings& cSettings::getInstance()
{
	if (!instance.initialized && !instance.initializing) instance.initialize();
	return instance;
}

//------------------------------------------------------------------------------
void cSettings::setPaths()
{
	homeDir = ::getHomeDir();

	// NOTE: I do not use cLog here on purpose.
	// Logs on linux go somewhere to $HOME/.maxr/
	// - as long as we can't access that we have to output everything to
	// the terminal because game's root dir is usually write protected! -- beko

#if WIN32
	const std::string maxrDir = std::string ("maxr");
#else
	const std::string maxrDir = std::string (".maxr");
#endif
	homeDir += (homeDir.empty() ? "" : PATH_DELIMITER) + maxrDir + PATH_DELIMITER;
	std::cout << "\n(II): Read home directory " << homeDir;
	makeDirectories(homeDir);

	// set new place for logs
	logPath = homeDir + "maxr.log";
	netLogPath = getUserLogDir();
	std::cout << "\n(II): Starting logging to: " << logPath << std::endl;
}

//------------------------------------------------------------------------------
void cSettings::loadFromJsonFile (const std::string& path)
{
	std::ifstream file (path);
	nlohmann::json json;

	if (!(file >> json))
	{
		Log.write ("cannot load maxr.json\ngenerating new file", cLog::eLOG_TYPE_WARNING);
		saveInFile();
		return;
	}
	try
	{
		cJsonArchiveIn in (json);
		in >> *this;
	}
	catch (const std::exception& e)
	{
		Log.write (std::string ("Error while reading settings: ") + e.what(), cLog::eLOG_TYPE_WARNING);
		Log.write ("Overwriting with default settings", cLog::eLOG_TYPE_WARNING);
		saveInFile();
	}
}
//------------------------------------------------------------------------------
void cSettings::initialize()
{
	std::unique_lock<std::recursive_mutex> lock (docMutex);
	initializing = true;

	if (initialized) return;

	setPaths();

	const auto settingsJson = homeDir + "maxr.json";

	if (FileExists (settingsJson))
	{
		loadFromJsonFile (settingsJson);
	}
	else
	{
		Log.write ("generating new settings", cLog::eLOG_TYPE_WARNING);
		saveInFile();
	}

	to_lower (global.voiceLanguage);
	if (!global.debug) Log.write ("Debugmode disabled - for verbose output please enable Debug in maxr.json", cLog::eLOG_TYPE_WARNING);
	else Log.write ("Debugmode enabled", cLog::eLOG_TYPE_INFO);

#if MAC
	// Create saves directory, if it doesn't exist, yet.
	// Creating it during setPaths is too early, because it was not read yet.
	if (!FileExists (getSavesPath().c_str()))
	{
		if (mkdir (getSavesPath().c_str(), 0755) == 0)
			Log.write ("Created new save directory " + getSavesPath(), cLog::eLOG_TYPE_INFO);
		else
			Log.write ("Can't create save directory " + getSavesPath(), cLog::eLOG_TYPE_ERROR);
	}
#endif
	initialized = true;
	initializing = false;
}

//------------------------------------------------------------------------------
void cSettings::saveInFile() const
{
	std::unique_lock<std::recursive_mutex> lock (docMutex);

	nlohmann::json json;
	cJsonArchiveOut out (json);
	out << *this;

	std::ofstream file (homeDir + "maxr.json");
	file << json.dump(1);
}

//------------------------------------------------------------------------------
void cSettings::setAnimations (bool animations)
{
	std::swap (inGame.animations, animations);
	if (inGame.animations != animations) animationsChanged();
}

//------------------------------------------------------------------------------
const std::string& cSettings::getNetLogPath() const
{
	return netLogPath;
}

//------------------------------------------------------------------------------
void cSettings::setNetLogPath (const char* netLogPath)
{
	this->netLogPath = netLogPath;
}

//------------------------------------------------------------------------------
const std::string& cSettings::getDataDir() const
{
	return dataDir;
}

//------------------------------------------------------------------------------
void cSettings::setDataDir (const char* dataDir)
{
	this->dataDir = dataDir;
}

//------------------------------------------------------------------------------
const std::string& cSettings::getLogPath() const
{
	return logPath;
}

//------------------------------------------------------------------------------
const std::string& cSettings::getHomeDir() const
{
	return homeDir;
}

//------------------------------------------------------------------------------
std::string cSettings::getFontPath() const
{
	return dataDir + path.font;
}

//------------------------------------------------------------------------------
std::string cSettings::getFxPath() const
{
	return dataDir + path.fx;
}

//------------------------------------------------------------------------------
std::string cSettings::getGfxPath() const
{
	return dataDir + path.gfx;
}

//------------------------------------------------------------------------------
std::string cSettings::getLangPath() const
{
	return dataDir + path.languages;
}

//------------------------------------------------------------------------------
std::string cSettings::getMapsPath() const
{
	return dataDir + path.maps;
}

//------------------------------------------------------------------------------
std::string cSettings::getSavesPath() const
{
	return homeDir + path.saves;
}

//------------------------------------------------------------------------------
std::string cSettings::getSoundsPath() const
{
	return dataDir + path.sounds;
}

//------------------------------------------------------------------------------
std::string cSettings::getVoicesPath() const
{
	return dataDir + path.voices;
}

//------------------------------------------------------------------------------
std::string cSettings::getMusicPath() const
{
	return dataDir + path.music;
}

//------------------------------------------------------------------------------
std::string cSettings::getVehiclesPath() const
{
	return dataDir + path.vehicles;
}

//------------------------------------------------------------------------------
std::string cSettings::getBuildingsPath() const
{
	return dataDir + path.buildings;
}

//------------------------------------------------------------------------------
std::string cSettings::getMvePath() const
{
	return dataDir + path.mve;
}
