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
#include "game/serialization/xmlarchive.h"
#include "ui/graphical/playercolor.h"
#include "utility/extendedtinyxml.h"
#include "utility/files.h"
#include "utility/log.h"
#include "utility/string/tolower.h"

using namespace tinyxml2;

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
	networkAddress.port = DEFAULTPORT;
#ifdef WIN32
	char* user = getenv ("USERNAME");
#elif __amigaos4__
	char* user = "AmigaOS4-User";
#else
	char* user = getenv ("USER");  //get $USER on linux
#endif

	playerSettings.name = (user == nullptr ? "Commander" : user);
	playerSettings.color = cRgbColor::red();
}

//------------------------------------------------------------------------------
cSettings& cSettings::getInstance()
{
	if (!instance.initialized && !instance.initializing) instance.initialize();
	return instance;
}

//------------------------------------------------------------------------------
bool cSettings::isInitialized() const
{
	return initialized;
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

	// set config to $HOME/.maxr/maxr.xml
	configPath = homeDir + MAX_XML;

	// set new place for logs
	logPath = homeDir + "maxr.log";
	netLogPath = getUserLogDir();
	std::cout << "\n(II): Starting logging to: " << logPath << std::endl;
}


//------------------------------------------------------------------------------
void cSettings::initialize()
{
	std::unique_lock<std::recursive_mutex> lock (xmlDocMutex);
	initializing = true;

	if (initialized) return;

	setPaths();

	tinyxml2::XMLDocument configFile;
	if (!FileExists (configPath) || configFile.LoadFile (configPath.c_str()) != XML_NO_ERROR)
	{
		saveInFile();
		initializing = false;
		initialized = true;
		return;
	}
	cXmlArchiveOut archive (*configFile.RootElement());
	try
	{
		serialize (archive);
	} catch (const std::runtime_error& ex)
	{
		Log.write ("Cannot read settings, Create a new file (and backup the old one)", cLog::eLOG_TYPE_WARNING);
		copyFile (configPath, configPath + ".old");
		saveInFile();
		initializing = false;
		initialized = true;
		return;
	}

	to_lower (startSettings.voiceLanguage);
	if (!gameSettings.debug) Log.write ("Debugmode disabled - for verbose output please enable Debug in maxr.xml", cLog::eLOG_TYPE_WARNING);
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
	std::unique_lock<std::recursive_mutex> lock (xmlDocMutex);

	tinyxml2::XMLDocument configFile;
	configFile.LinkEndChild (configFile.NewDeclaration());
	configFile.LinkEndChild (configFile.NewElement ("Options"));
	cXmlArchiveIn archive (*configFile.RootElement());

	const_cast<cSettings&>(*this).serialize (archive);

	if (configFile.SaveFile (configPath.c_str()) != XML_NO_ERROR)
	{
		Log.write ("Could not write new config to " + configPath, cLog::eLOG_TYPE_ERROR);
		Log.write (configFile.GetErrorStr1(), cLog::eLOG_TYPE_ERROR);
		Log.write (configFile.GetErrorStr2(), cLog::eLOG_TYPE_ERROR);
	}
}

//------------------------------------------------------------------------------
bool cSettings::isDebug() const
{
	return gameSettings.debug;
}

//------------------------------------------------------------------------------
void cSettings::setDebug (bool debug)
{
	gameSettings.debug = debug;
}

//------------------------------------------------------------------------------
bool cSettings::shouldAutosave() const
{
	return gameSettings.autosave;
}

//------------------------------------------------------------------------------
void cSettings::setAutosave (bool autosave)
{
	gameSettings.autosave = autosave;
}

//------------------------------------------------------------------------------
bool cSettings::isAnimations() const
{
	return gameSettings.animations;
}

//------------------------------------------------------------------------------
void cSettings::setAnimations (bool animations)
{
	std::swap (gameSettings.animations, animations);
	if (gameSettings.animations != animations) animationsChanged();
}

//------------------------------------------------------------------------------
bool cSettings::isShadows() const
{
	return gameSettings.shadows;
}

//------------------------------------------------------------------------------
void cSettings::setShadows (bool shadows)
{
	gameSettings.shadows = shadows;
}

//------------------------------------------------------------------------------
bool cSettings::isAlphaEffects() const
{
	return gameSettings.alphaEffects;
}

//------------------------------------------------------------------------------
void cSettings::setAlphaEffects (bool alphaEffects)
{
	gameSettings.alphaEffects = alphaEffects;
}

//------------------------------------------------------------------------------
bool cSettings::shouldShowDescription() const
{
	return gameSettings.showDescription;
}

//------------------------------------------------------------------------------
void cSettings::setShowDescription (bool showDescription)
{
	gameSettings.showDescription = showDescription;
}

//------------------------------------------------------------------------------
bool cSettings::isDamageEffects() const
{
	return gameSettings.damageEffects;
}

//------------------------------------------------------------------------------
void cSettings::setDamageEffects (bool damageEffects)
{
	gameSettings.damageEffects = damageEffects;
}

//------------------------------------------------------------------------------
bool cSettings::isDamageEffectsVehicles() const
{
	return gameSettings.damageEffectsVehicles;
}

//------------------------------------------------------------------------------
void cSettings::setDamageEffectsVehicles (bool damageEffectsVehicles)
{
	gameSettings.damageEffectsVehicles = damageEffectsVehicles;
}

//------------------------------------------------------------------------------
bool cSettings::isMakeTracks() const
{
	return gameSettings.makeTracks;
}

//------------------------------------------------------------------------------
void cSettings::setMakeTracks (bool makeTracks)
{
	gameSettings.makeTracks = makeTracks;
}

//------------------------------------------------------------------------------
int cSettings::getScrollSpeed() const
{
	return gameSettings.scrollSpeed;
}

//------------------------------------------------------------------------------
void cSettings::setScrollSpeed (int scrollSpeed)
{
	gameSettings.scrollSpeed = scrollSpeed;
}

//------------------------------------------------------------------------------
void cSettings::setNetworkAddress (const sNetworkAddress& networkAddress)
{
	this->networkAddress = networkAddress;
}

//------------------------------------------------------------------------------
void cSettings::setPlayerSettings (const sPlayerSettings& playerSettings)
{
	this->playerSettings = playerSettings;
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
bool cSettings::isSoundEnabled() const
{
	return soundSettings.soundEnabled;
}

//------------------------------------------------------------------------------
void cSettings::setSoundEnabled (bool soundEnabled)
{
	soundSettings.soundEnabled = soundEnabled;
}

//------------------------------------------------------------------------------
int cSettings::getMusicVol() const
{
	return soundSettings.musicVol;
}

//------------------------------------------------------------------------------
void cSettings::setMusicVol (int musicVol)
{
	soundSettings.musicVol = musicVol;
}

//------------------------------------------------------------------------------
int cSettings::getSoundVol() const
{
	return soundSettings.soundVol;
}

//------------------------------------------------------------------------------
void cSettings::setSoundVol (int soundVol)
{
	soundSettings.soundVol = soundVol;
}

//------------------------------------------------------------------------------
int cSettings::getVoiceVol() const
{
	return soundSettings.voiceVol;
}

//------------------------------------------------------------------------------
void cSettings::setVoiceVol (int voiceVol)
{
	soundSettings.voiceVol = voiceVol;
}

//------------------------------------------------------------------------------
int cSettings::getChunkSize() const
{
	return soundSettings.chunkSize;
}

//------------------------------------------------------------------------------
void cSettings::setChunkSize (int chunkSize)
{
	soundSettings.chunkSize = chunkSize;
}

//------------------------------------------------------------------------------
int cSettings::getFrequency() const
{
	return soundSettings.frequency;
}

//------------------------------------------------------------------------------
void cSettings::setFrequence (int frequency)
{
	soundSettings.frequency = frequency;
}

//------------------------------------------------------------------------------
bool cSettings::isMusicMute() const
{
	return soundSettings.musicMute;
}

//------------------------------------------------------------------------------
void cSettings::setMusicMute (bool musicMute)
{
	soundSettings.musicMute = musicMute;
}

//------------------------------------------------------------------------------
bool cSettings::isSoundMute() const
{
	return soundSettings.soundMute;
}

//------------------------------------------------------------------------------
void cSettings::setSoundMute (bool soundMute)
{
	soundSettings.soundMute = soundMute;
}

//------------------------------------------------------------------------------
bool cSettings::isVoiceMute() const
{
	return soundSettings.voiceMute;
}

//------------------------------------------------------------------------------
void cSettings::setVoiceMute (bool voiceMute)
{
	soundSettings.voiceMute = voiceMute;
}

//------------------------------------------------------------------------------
bool cSettings::is3DSound() const
{
	return soundSettings.sound3d;
}

//------------------------------------------------------------------------------
void cSettings::set3DSound (bool sound3d)
{
	soundSettings.sound3d = sound3d;
}

//------------------------------------------------------------------------------
bool cSettings::shouldShowIntro() const
{
	return startSettings.showIntro;
}

//------------------------------------------------------------------------------
void cSettings::setShowIntro (bool showIntro)
{
	startSettings.showIntro = showIntro;
}

//------------------------------------------------------------------------------
bool cSettings::shouldUseFastMode() const
{
	return startSettings.fastMode;
}

//------------------------------------------------------------------------------
bool cSettings::shouldDoPrescale() const
{
	return startSettings.preScale;
}

//------------------------------------------------------------------------------
const std::string& cSettings::getLanguage() const
{
	return startSettings.language;
}

//------------------------------------------------------------------------------
void cSettings::setLanguage (const char* language)
{
	startSettings.language = language;
}
//------------------------------------------------------------------------------
const std::string& cSettings::getVoiceLanguage() const
{
	return startSettings.voiceLanguage;
}

//------------------------------------------------------------------------------
unsigned int cSettings::getCacheSize() const
{
	return startSettings.cacheSize;
}

//------------------------------------------------------------------------------
void cSettings::setCacheSize (unsigned int cacheSize)
{
	startSettings.cacheSize = cacheSize;
}

//------------------------------------------------------------------------------
std::string cSettings::getFontPath() const
{
	return dataDir + pathSettings.fontPath;
}

//------------------------------------------------------------------------------
std::string cSettings::getFxPath() const
{
	return dataDir + pathSettings.fxPath;
}

//------------------------------------------------------------------------------
std::string cSettings::getGfxPath() const
{
	return dataDir + pathSettings.gfxPath;
}

//------------------------------------------------------------------------------
std::string cSettings::getLangPath() const
{
	return dataDir + pathSettings.langPath;
}

//------------------------------------------------------------------------------
std::string cSettings::getMapsPath() const
{
	return dataDir + pathSettings.mapsPath;
}

//------------------------------------------------------------------------------
std::string cSettings::getSavesPath() const
{
	return homeDir + pathSettings.savesPath;
}

//------------------------------------------------------------------------------
std::string cSettings::getSoundsPath() const
{
	return dataDir + pathSettings.soundsPath;
}

//------------------------------------------------------------------------------
std::string cSettings::getVoicesPath() const
{
	return dataDir + pathSettings.voicesPath;
}

//------------------------------------------------------------------------------
std::string cSettings::getMusicPath() const
{
	return dataDir + pathSettings.musicPath;
}

//------------------------------------------------------------------------------
std::string cSettings::getVehiclesPath() const
{
	return dataDir + pathSettings.vehiclesPath;
}

//------------------------------------------------------------------------------
std::string cSettings::getBuildingsPath() const
{
	return dataDir + pathSettings.buildingsPath;
}

//------------------------------------------------------------------------------
std::string cSettings::getMvePath() const
{
	return dataDir + pathSettings.mvePath;
}
