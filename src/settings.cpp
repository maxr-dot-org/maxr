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

#include "defines.h"
#include "maxrversion.h"
#include "utility/files.h"
#include "utility/log.h"
#include "utility/serialization/jsonarchive.h"
#include "utility/string/tolower.h"

#include <config/workaround/cpp17/filesystem.h>
#include <iostream>
#include <locale>
#include <string>

namespace
{
	//--------------------------------------------------------------------------
	/**
	 * Platform dependent implementations.
	 * On most platforms just the executable folder is used.
	 * On Linux it tries to verify the path from the configuration file
	 * @return The really selected data location.
	 */
	std::filesystem::path searchDataDir()
	{
#if MAC
		// assuming data is in same folder as binary (or current working directory)
		return getCurrentExeDir();
#elif WIN32
		// assuming data is in same folder as binary (or current working directory)
		return getCurrentExeDir();
#elif __amigaos4__
		// assuming data is in same folder as binary (or current working directory)
		return getCurrentExeDir();
#else
		// BEGIN crude path validation to find gamedata
		Log.write ("Probing for data paths using default values:", cLog::eLogType::Info);

		std::vector<std::filesystem::path> sPathArray =
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

		// BEGIN SET MAXRDATA
		const char* cDataDir = getenv ("MAXRDATA");
		if (cDataDir == nullptr)
		{
			Log.write ("$MAXRDATA is not set", cLog::eLogType::Info);
		}
		else
		{
			sPathArray.insert (sPathArray.begin(), cDataDir);
			Log.write ("$MAXRDATA is set and overrides default data search path", cLog::eLogType::Warning);
		}
		// END SET MAXRDATA

		for (auto sInitFile : sPathArray)
		{
			if (std::filesystem::exists (sInitFile / "init.pcx"))
			{
				Log.write ("Found gamedata in: " + sInitFile, cLog::eLogType::Info);
				return sInitFile;
			}
		}
		// still empty? cry for mama - we couldn't locate any typical data folder
		Log.write ("No success probing for data folder!", cLog::eLogType::Error);
		// END crude path validation to find gamedata
		return "";
#endif
	}

} // namespace

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
	char* user = getenv ("USER"); //get $USER on linux
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
	homeDir /= maxrDir;
	std::cout << "\n(II): Read home directory " << homeDir.string();
	std::filesystem::create_directories (homeDir);

	// set new place for logs
	logPath = homeDir / "maxr.log";
	netLogPath = getUserLogDir();
	std::cout << "\n(II): Starting logging to: " << logPath.string() << std::endl;

	dataDir = searchDataDir();
}

//------------------------------------------------------------------------------
void cSettings::loadFromJsonFile (const std::filesystem::path& path)
{
	std::ifstream file (path.string());
	nlohmann::json json;

	if (!(file >> json))
	{
		Log.write ("cannot load maxr.json\ngenerating new file", cLog::eLogType::Warning);
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
		Log.write (std::string ("Error while reading settings: ") + e.what(), cLog::eLogType::Warning);
		Log.write ("Overwriting with default settings", cLog::eLogType::Warning);
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

	const auto settingsJson = homeDir / "maxr.json";

	if (std::filesystem::exists (settingsJson))
	{
		loadFromJsonFile (settingsJson);
	}
	else
	{
		Log.write ("generating new settings", cLog::eLogType::Warning);
		saveInFile();
	}

	to_lower (global.voiceLanguage);
	if (!global.debug)
		Log.write ("Debugmode disabled - for verbose output please enable Debug in maxr.json", cLog::eLogType::Warning);
	else
		Log.write ("Debugmode enabled", cLog::eLogType::Info);

	// Create saves directory, if it doesn't exist, yet.
	// Creating it during setPaths is too early, because it was not read yet.
	std::filesystem::create_directories (getSavesPath());

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

	std::ofstream file ((homeDir / "maxr.json").string());
	file << json.dump (1);
}

//------------------------------------------------------------------------------
void cSettings::setAnimations (bool animations)
{
	std::swap (inGame.animations, animations);
	if (inGame.animations != animations) animationsChanged();
}

//------------------------------------------------------------------------------
const std::filesystem::path& cSettings::getNetLogPath() const
{
	return netLogPath;
}

//------------------------------------------------------------------------------
void cSettings::setNetLogPath (const std::filesystem::path& netLogPath)
{
	this->netLogPath = netLogPath;
}

//------------------------------------------------------------------------------
const std::filesystem::path& cSettings::getDataDir() const
{
	return dataDir;
}

//------------------------------------------------------------------------------
void cSettings::setDataDir (const std::filesystem::path& dataDir)
{
	this->dataDir = dataDir;
}

//------------------------------------------------------------------------------
const std::filesystem::path& cSettings::getLogPath() const
{
	return logPath;
}

//------------------------------------------------------------------------------
const std::filesystem::path& cSettings::getHomeDir() const
{
	return homeDir;
}

//------------------------------------------------------------------------------
std::filesystem::path cSettings::getFontPath() const
{
	return dataDir / path.font;
}

//------------------------------------------------------------------------------
std::filesystem::path cSettings::getFxPath() const
{
	return dataDir / path.fx;
}

//------------------------------------------------------------------------------
std::filesystem::path cSettings::getGfxPath() const
{
	return dataDir / path.gfx;
}

//------------------------------------------------------------------------------
std::filesystem::path cSettings::getLangPath() const
{
	return dataDir / path.languages;
}

//------------------------------------------------------------------------------
std::filesystem::path cSettings::getMapsPath() const
{
	return dataDir / path.maps;
}

//------------------------------------------------------------------------------
std::filesystem::path cSettings::getSavesPath() const
{
	return homeDir / path.saves;
}

//------------------------------------------------------------------------------
std::filesystem::path cSettings::getSoundsPath() const
{
	return dataDir / path.sounds;
}

//------------------------------------------------------------------------------
std::filesystem::path cSettings::getVoicesPath() const
{
	return dataDir / path.voices;
}

//------------------------------------------------------------------------------
std::filesystem::path cSettings::getMusicPath() const
{
	return dataDir / path.music;
}

//------------------------------------------------------------------------------
std::filesystem::path cSettings::getVehiclesPath() const
{
	return dataDir / path.vehicles;
}

//------------------------------------------------------------------------------
std::filesystem::path cSettings::getBuildingsPath() const
{
	return dataDir / path.buildings;
}

//------------------------------------------------------------------------------
std::filesystem::path cSettings::getMvePath() const
{
	return dataDir / path.mve;
}

//------------------------------------------------------------------------------
std::filesystem::path cSettings::getUserMapsDir() const
{
#ifdef __amigaos4__
	return "";
#else
	if (getHomeDir().empty()) return "";
	auto mapFolder = getHomeDir() / "maps";
	std::filesystem::create_directories (mapFolder);
	return mapFolder;
#endif
}

//------------------------------------------------------------------------------
std::filesystem::path cSettings::getUserScreenshotsDir() const
{
#ifdef __amigaos4__
	return "";
#elif defined(MAC)
	std::filesystem::path homeFolder = ::getHomeDir();
	if (homeFolder.empty())
		return "";
	// store screenshots directly on the desktop of the user
	return homeFolder / "Desktop";
#else
	if (getHomeDir().empty())
		return "";
	auto screenshotsFolder = getHomeDir() / "screenies";
	std::filesystem::create_directories (screenshotsFolder);
	return screenshotsFolder;
#endif
}

//------------------------------------------------------------------------------
std::filesystem::path cSettings::getUserLogDir() const
{
#ifdef __amigaos4__
	return "";
#elif defined(MAC)
	auto homeFolder = ::getHomeDir();
	if (homeFolder.empty())
		return "";
	// store Log directly on the desktop of the user
	return homeFolder / "Desktop";
#else
	if (homeDir.empty())
		return "";
	auto LogDir = homeDir / "log_files";
	std::filesystem::create_directories (LogDir);
	return LogDir;
#endif
}
