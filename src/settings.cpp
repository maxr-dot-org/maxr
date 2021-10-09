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

#include <3rd/tinyxml2/tinyxml2.h>

#include "defines.h"
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


	//--------------------------------------------------------------------------
	/**
	 * Template function for set a setting.
	 * @param path See #getXmlNode() for more information on
	 *        how to use this parameter.
	 * @param value The value to set as attribute to the setting node.
	 * @param valueName The name of the attribute to set to the setting node.
	 */
	template <typename T>
	void setSetting (tinyxml2::XMLDocument& configFile, const std::string& path, T value, const char* valueName)
	{
		XMLElement* xmlElement = getOrCreateXmlElement (configFile, path);
		if (xmlElement == nullptr) return;

		xmlElement->SetAttribute (valueName, value);
	}

	//------------------------------------------------------------------------------
	// Overloads for the setSetting template function.
	// Each type has to call the template setSetting() method and pass the
	// corresponding attribute name to it.
	void setSetting (tinyxml2::XMLDocument& configFile, const std::string& path, const char* value)
	{
		setSetting (configFile, path, value, "Text");
	}

	//------------------------------------------------------------------------------
	void setSetting (tinyxml2::XMLDocument& configFile, const std::string& path, int value)
	{
		setSetting (configFile, path, value, "Num");
	}

	//------------------------------------------------------------------------------
	void setSetting (tinyxml2::XMLDocument& configFile, const std::string& path, unsigned int value)
	{
		setSetting (configFile, path, value, "Num");
	}

	//------------------------------------------------------------------------------
	void setSetting (tinyxml2::XMLDocument& configFile, const std::string& path, bool value)
	{
		if (value) setSetting (configFile, path, "Yes", "YN");
		else setSetting (configFile, path, "No", "YN");
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

	XMLElement* xmlElement = nullptr;

	// START

	// =====================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Start", "Resolution"});
	if (!xmlElement || !xmlElement->Attribute ("Text"))
	{
		Log.write ("Can't load resolution from config file: using default value", cLog::eLOG_TYPE_WARNING);
		videoSettings.resolution = std::nullopt;
	}
	else
	{
		std::string temp = xmlElement->Attribute ("Text");
		int w = atoi (temp.substr (0, temp.find (".", 0)).c_str());
		int h = atoi (temp.substr (temp.find (".", 0) + 1, temp.length()).c_str());
		videoSettings.resolution = cPosition{w, h};
	}

	// =====================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Start", "ColourDepth"});
	if (!xmlElement || !xmlElement->Attribute ("Num"))
	{
		Log.write ("Can't load color depth from config file: using default value", cLog::eLOG_TYPE_WARNING);
		videoSettings.colourDepth = 32;
	}
	else
	{
		videoSettings.colourDepth = xmlElement->IntAttribute ("Num");
	}

	// =====================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Start", "Intro"});
	if (!xmlElement || !xmlElement->Attribute ("YN"))
	{
		Log.write ("Can't load intro from config file: using default value", cLog::eLOG_TYPE_WARNING);
		startSettings.showIntro = true;
	}
	else
	{
		startSettings.showIntro = getXMLAttributeBoolFromElement (xmlElement, "YN");
	}

	// =====================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Start", "Windowmode"});
	if (!xmlElement || !xmlElement->Attribute ("YN"))
	{
		Log.write ("Can't load window mode from config file: using default value", cLog::eLOG_TYPE_WARNING);
		videoSettings.windowMode = true;
	}
	else
	{
		videoSettings.windowMode = getXMLAttributeBoolFromElement (xmlElement, "YN");
	}

	// =====================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Start", "Display"});
	if (!xmlElement || !xmlElement->Attribute ("Num"))
	{
		Log.write ("Can't load display index from config file: using default value", cLog::eLOG_TYPE_WARNING);
		videoSettings.displayIndex = 0;
	}
	else
	{
		videoSettings.displayIndex = xmlElement->IntAttribute ("Num");
	}

	// =====================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Start", "Fastmode"});
	if (!xmlElement || !xmlElement->Attribute ("YN"))
	{
		Log.write ("Can't load fast mode from config file: using default value", cLog::eLOG_TYPE_WARNING);
		startSettings.fastMode = false;
	}
	else
	{
		startSettings.fastMode = getXMLAttributeBoolFromElement (xmlElement, "YN");
	}

	// =====================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Start", "PreScale"});
	if (!xmlElement || !xmlElement->Attribute ("YN"))
	{
		Log.write ("Can't load pre scale from config file: using default value", cLog::eLOG_TYPE_WARNING);
		startSettings.preScale = false;
	}
	else
	{
		startSettings.preScale = getXMLAttributeBoolFromElement (xmlElement, "YN");
	}

	// =====================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Start", "CacheSize"});
	if (!xmlElement || !xmlElement->Attribute ("Num"))
	{
		Log.write ("Can't load cache size from config file: using default value", cLog::eLOG_TYPE_WARNING);
		setCacheSize (400);
	}
	else
	{
		startSettings.cacheSize = xmlElement->IntAttribute ("Num");
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Start", "Language"});
	if (!xmlElement || !xmlElement->Attribute ("Text"))
	{
		Log.write ("Can't load language from config file: using default value", cLog::eLOG_TYPE_WARNING);
		setLanguage ("en");
	}
	else
	{
		startSettings.language = xmlElement->Attribute ("Text");
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Start", "VoiceLanguage"});
	if (!xmlElement || !xmlElement->Attribute ("Text"))
	{
		Log.write ("Can't load language from config file: using default value", cLog::eLOG_TYPE_WARNING);
		startSettings.voiceLanguage = "";
	}
	else
	{
		startSettings.voiceLanguage = xmlElement->Attribute ("Text");
		to_lower (startSettings.voiceLanguage);
	}

	// GAME
	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "EnableAutosave"});
	if (!xmlElement || !xmlElement->Attribute ("YN"))
	{
		Log.write ("Can't load autosave from config file: using default value", cLog::eLOG_TYPE_WARNING);
		setAutosave (true);
	}
	else
	{
		gameSettings.autosave = getXMLAttributeBoolFromElement (xmlElement, "YN");
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "EnableDebug"});
	if (!xmlElement || !xmlElement->Attribute ("YN"))
	{
		Log.write ("Can't load debug from config file: using default value", cLog::eLOG_TYPE_WARNING);
		setDebug (true);
	}
	else
	{
		gameSettings.debug = getXMLAttributeBoolFromElement (xmlElement, "YN");
		if (!gameSettings.debug) Log.write ("Debugmode disabled - for verbose output please enable Debug in maxr.xml", cLog::eLOG_TYPE_WARNING);
		else Log.write ("Debugmode enabled", cLog::eLOG_TYPE_INFO);
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "EnableAnimations"});
	if (!xmlElement || !xmlElement->Attribute ("YN"))
	{
		Log.write ("Can't load animations from config file: using default value", cLog::eLOG_TYPE_WARNING);
		setAnimations (true);
	}
	else
	{
		gameSettings.animations = getXMLAttributeBoolFromElement (xmlElement, "YN");
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "EnableShadows"});
	if (!xmlElement || !xmlElement->Attribute ("YN"))
	{
		Log.write ("Can't load shadows from config file: using default value", cLog::eLOG_TYPE_WARNING);
		setShadows (true);
	}
	else
	{
		gameSettings.shadows = getXMLAttributeBoolFromElement (xmlElement, "YN");
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "EnableAlphaEffects"});
	if (!xmlElement || !xmlElement->Attribute ("YN"))
	{
		Log.write ("Can't load alpha effects from config file: using default value", cLog::eLOG_TYPE_WARNING);
		setAlphaEffects (true);
	}
	else
	{
		gameSettings.alphaEffects = getXMLAttributeBoolFromElement (xmlElement, "YN");
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "EnableDescribtions"});
	if (!xmlElement || !xmlElement->Attribute ("YN"))
	{
		Log.write ("Can't load descriptions from config file: using default value", cLog::eLOG_TYPE_WARNING);
		setShowDescription (true);
	}
	else
	{
		gameSettings.showDescription = getXMLAttributeBoolFromElement (xmlElement, "YN");
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "EnableDamageEffects"});
	if (!xmlElement || !xmlElement->Attribute ("YN"))
	{
		Log.write ("Can't load damage effects from config file: using default value", cLog::eLOG_TYPE_WARNING);
		setDamageEffects (true);
	}
	else
	{
		gameSettings.damageEffects = getXMLAttributeBoolFromElement (xmlElement, "YN");
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "EnableDamageEffectsVehicles"});
	if (!xmlElement || !xmlElement->Attribute ("YN"))
	{
		Log.write ("Can't load damaga effects vehicles from config file: using default value", cLog::eLOG_TYPE_WARNING);
		setDamageEffectsVehicles (true);
	}
	else
	{
		gameSettings.damageEffectsVehicles = getXMLAttributeBoolFromElement (xmlElement, "YN");
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "EnableMakeTracks"});
	if (!xmlElement || !xmlElement->Attribute ("YN"))
	{
		Log.write ("Can't load make tracks from config file: using default value", cLog::eLOG_TYPE_WARNING);
		setMakeTracks (true);
	}
	else
	{
		gameSettings.makeTracks = getXMLAttributeBoolFromElement (xmlElement, "YN");
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "ScrollSpeed"});
	if (!xmlElement || !xmlElement->Attribute ("Num"))
	{
		Log.write ("Can't load scroll speed from config file: using default value", cLog::eLOG_TYPE_WARNING);
		setScrollSpeed (32);
	}
	else
	{
		gameSettings.scrollSpeed = xmlElement->IntAttribute ("Num");
	}

	// GAME-SOUND
	// =====================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "Sound", "Enabled"});
	if (!xmlElement || !xmlElement->Attribute ("YN"))
	{
		Log.write ("Can't load sound enabled from config file: using default value", cLog::eLOG_TYPE_WARNING);
		setSoundEnabled (true);
	}
	else
	{
		soundSettings.soundEnabled = getXMLAttributeBoolFromElement (xmlElement, "YN");
	}

	// =====================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "Sound", "MusicMute"});
	if (!xmlElement || !xmlElement->Attribute ("YN"))
	{
		Log.write ("Can't load music mute from config file: using default value", cLog::eLOG_TYPE_WARNING);
		setMusicMute (false);
	}
	else
	{
		soundSettings.musicMute = getXMLAttributeBoolFromElement (xmlElement, "YN");
	}

	// =====================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "Sound", "SoundMute"});
	if (!xmlElement || !xmlElement->Attribute ("YN"))
	{
		Log.write ("Can't load sound mute from config file: using default value", cLog::eLOG_TYPE_WARNING);
		setSoundMute (false);
	}
	else
	{
		soundSettings.soundMute = getXMLAttributeBoolFromElement (xmlElement, "YN");
	}

	// =====================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "Sound", "VoiceMute"});
	if (!xmlElement || !xmlElement->Attribute ("YN"))
	{
		Log.write ("Can't load voice mute from config file: using default value", cLog::eLOG_TYPE_WARNING);
		setVoiceMute (false);
	}
	else
	{
		soundSettings.voiceMute = getXMLAttributeBoolFromElement (xmlElement, "YN");
	}

	// =====================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "Sound", "Sound3D"});
	if (!xmlElement || !xmlElement->Attribute ("YN"))
	{
		Log.write ("Can't load 3D sound from config file: using default value", cLog::eLOG_TYPE_WARNING);
		set3DSound (true);
	}
	else
	{
		soundSettings.sound3d = getXMLAttributeBoolFromElement (xmlElement, "YN");
	}

	// =====================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "Sound", "MusicVol"});
	if (!xmlElement || !xmlElement->Attribute ("Num"))
	{
		Log.write ("Can't load music volume from config file: using default value", cLog::eLOG_TYPE_WARNING);
		setMusicVol (128);
	}
	else
	{
		soundSettings.musicVol = xmlElement->IntAttribute ("Num");
	}

	// =====================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "Sound", "SoundVol"});
	if (!xmlElement || !xmlElement->Attribute ("Num"))
	{
		Log.write ("Can't load music volume from config file: using default value", cLog::eLOG_TYPE_WARNING);
		setSoundVol (128);
	}
	else
	{
		soundSettings.soundVol = xmlElement->IntAttribute ("Num");
	}

	// =====================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "Sound", "VoiceVol"});
	if (!xmlElement || !xmlElement->Attribute ("Num"))
	{
		Log.write ("Can't load music volume from config file: using default value", cLog::eLOG_TYPE_WARNING);
		setVoiceVol (128);
	}
	else
	{
		soundSettings.voiceVol = xmlElement->IntAttribute ("Num");
	}

	// =====================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "Sound", "ChunkSize"});
	if (!xmlElement || !xmlElement->Attribute ("Num"))
	{
		Log.write ("Can't load music volume from config file: using default value", cLog::eLOG_TYPE_WARNING);
		setChunkSize (2048);
	}
	else
	{
		soundSettings.chunkSize = xmlElement->IntAttribute ("Num");
	}

	// =====================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "Sound", "Frequency"});
	if (!xmlElement || !xmlElement->Attribute ("Num"))
	{
		Log.write ("Can't load music volume from config file: using default value", cLog::eLOG_TYPE_WARNING);
		setFrequence (44100);
	}
	else
	{
		soundSettings.frequency = xmlElement->IntAttribute ("Num");
	}

	// PATHS
	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "Paths", "Gamedata"});
	if (!xmlElement || !xmlElement->Attribute ("Text"))
	{
		Log.write ("Can't load gamedata from config file: using default value", cLog::eLOG_TYPE_WARNING);
		setDataDir (searchDataDir ("").c_str());
	}
	else
	{
		setDataDir (searchDataDir (xmlElement->Attribute ("Text")).c_str());
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "Paths", "Languages"});
	if (!xmlElement || !xmlElement->Attribute ("Text"))
	{
		Log.write ("Can't load language path from config file: using default value", cLog::eLOG_TYPE_WARNING);
		pathSettings.langPath = "languages";
	}
	else
	{
		pathSettings.langPath = xmlElement->Attribute ("Text");
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "Paths", "Fonts"});
	if (!xmlElement || !xmlElement->Attribute ("Text"))
	{
		Log.write ("Can't load fonts path from config file: using default value", cLog::eLOG_TYPE_WARNING);
		pathSettings.fontPath = "fonts";
	}
	else
	{
		pathSettings.fontPath = xmlElement->Attribute ("Text");
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "Paths", "FX"});
	if (!xmlElement || !xmlElement->Attribute ("Text"))
	{
		Log.write ("Can't load fx path from config file: using default value", cLog::eLOG_TYPE_WARNING);
		pathSettings.fxPath = "fx";
	}
	else
	{
		pathSettings.fxPath = xmlElement->Attribute ("Text");
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "Paths", "GFX"});
	if (!xmlElement || !xmlElement->Attribute ("Text"))
	{
		Log.write ("Can't load gfx path from config file: using default value", cLog::eLOG_TYPE_WARNING);
		pathSettings.gfxPath = "gfx";
	}
	else
	{
		pathSettings.gfxPath = xmlElement->Attribute ("Text");
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "Paths", "Maps"});
	if (!xmlElement || !xmlElement->Attribute ("Text"))
	{
		Log.write ("Can't load maps path from config file: using default value", cLog::eLOG_TYPE_WARNING);
		pathSettings.mapsPath = "maps";
	}
	else
	{
		pathSettings.mapsPath = xmlElement->Attribute ("Text");
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "Paths", "Saves"});
	if (!xmlElement || !xmlElement->Attribute ("Text"))
	{
		Log.write ("Can't load saves path from config file: using default value", cLog::eLOG_TYPE_WARNING);
		pathSettings.savesPath = "saves";
	}
	else
	{
		pathSettings.savesPath = xmlElement->Attribute ("Text");
	}
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

	// =====================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "Paths", "Sounds"});
	if (!xmlElement || !xmlElement->Attribute ("Text"))
	{
		Log.write ("Can't load sounds path from config file: using default value", cLog::eLOG_TYPE_WARNING);
		pathSettings.soundsPath = "sounds";
	}
	else
	{
		pathSettings.soundsPath = xmlElement->Attribute ("Text");
	}

	// =====================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "Paths", "Voices"});
	if (!xmlElement || !xmlElement->Attribute ("Text"))
	{
		Log.write ("Can't load voices path from config file: using default value", cLog::eLOG_TYPE_WARNING);
		pathSettings.voicesPath = "voices";
	}
	else
	{
		pathSettings.voicesPath = xmlElement->Attribute ("Text");
	}

	// =====================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "Paths", "Music"});
	if (!xmlElement || !xmlElement->Attribute ("Text"))
	{
		Log.write ("Can't load music path from config file: using default value", cLog::eLOG_TYPE_WARNING);
		pathSettings.musicPath = "music";
	}
	else
	{
		pathSettings.musicPath = xmlElement->Attribute ("Text");
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "Paths", "Vehicles"});
	if (!xmlElement || !xmlElement->Attribute ("Text"))
	{
		Log.write ("Can't load vehicles path from config file: using default value", cLog::eLOG_TYPE_WARNING);
		pathSettings.vehiclesPath = "vehicles";
	}
	else
	{
		pathSettings.vehiclesPath = xmlElement->Attribute ("Text");
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "Paths", "Buildings"});
	if (!xmlElement || !xmlElement->Attribute ("Text"))
	{
		Log.write ("Can't load buildings path from config file: using default value", cLog::eLOG_TYPE_WARNING);
		pathSettings.buildingsPath = "buildings";
	}
	else
	{
		pathSettings.buildingsPath = xmlElement->Attribute ("Text");
	}

	// =====================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "Paths", "MVEs"});
	if (!xmlElement || !xmlElement->Attribute ("Text"))
	{
		Log.write ("Can't load language path from config file: using default value", cLog::eLOG_TYPE_WARNING);
		pathSettings.mvePath = "mve";
	}
	else
	{
		pathSettings.mvePath = xmlElement->Attribute ("Text");
	}

	// GAME-NET
	sNetworkAddress xmlNetworkAddress;
	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "Net", "IP"});
	if (!xmlElement || !xmlElement->Attribute ("Text"))
	{
		Log.write ("Can't load IP from config file: using default value", cLog::eLOG_TYPE_WARNING);
		xmlNetworkAddress.ip = "127.0.0.1";
	}
	else
	{
		xmlNetworkAddress.ip = xmlElement->Attribute ("Text");
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "Net", "Port"});
	if (!xmlElement || !xmlElement->Attribute ("Num"))
	{
		Log.write ("Can't load Port config file: using default value", cLog::eLOG_TYPE_WARNING);
		xmlNetworkAddress.port = DEFAULTPORT;
	}
	else
	{
		xmlNetworkAddress.port = xmlElement->IntAttribute ("Num");
	}
	setNetworkAddress (xmlNetworkAddress);

	// =========================================================================
	sPlayerSettings xmlPlayerSettings;
	auto playerXmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "Net", "PlayerName"});
	if (!playerXmlElement || !playerXmlElement->Attribute ("Text"))
	{
		Log.write ("Can't load player name from config file: using default value", cLog::eLOG_TYPE_WARNING);

#ifdef WIN32
		char* user = getenv ("USERNAME");
#elif __amigaos4__
		char* user = "AmigaOS4-User";
#else
		char* user = getenv ("USER");  //get $USER on linux
#endif

		xmlPlayerSettings.name = (user == nullptr ? "Commander" : user);
	}
	else
	{
		xmlPlayerSettings.name = playerXmlElement->Attribute ("Text");
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "Net", "PlayerColor"});
	if (!xmlElement || !xmlElement->Attribute ("red") || !xmlElement->Attribute ("green") || !xmlElement->Attribute ("blue"))
	{
		// try to load color by index for downward compatibility
		if (playerXmlElement && playerXmlElement->Attribute ("Num"))
		{
			const auto colorIndex = playerXmlElement->IntAttribute ("Num") % cPlayerColor::predefinedColorsCount;
			xmlPlayerSettings.color = cPlayerColor::predefinedColors[colorIndex];
		}
		else
		{
			Log.write ("Can't load player color from config file: using default value", cLog::eLOG_TYPE_WARNING);
			xmlPlayerSettings.color = cRgbColor::red();
		}
	}
	else
	{
		xmlPlayerSettings.color = cRgbColor (xmlElement->IntAttribute ("red"), xmlElement->IntAttribute ("green"), xmlElement->IntAttribute ("blue"));
	}
	setPlayerSettings (xmlPlayerSettings);
	saveInFile();

	initialized = true;
	initializing = false;
}

//------------------------------------------------------------------------------
void cSettings::saveInFile() /*const*/
{
	std::unique_lock<std::recursive_mutex> lock (xmlDocMutex);

	tinyxml2::XMLDocument configFile;
	if (!FileExists (configPath))
	{
		configFile.Clear();
		configFile.LinkEndChild (configFile.NewDeclaration());
		configFile.LinkEndChild (configFile.NewElement ("Options"));
	}
	else if (configFile.LoadFile (configPath.c_str()) != XML_NO_ERROR)
	{
		Log.write ("Could not load config to " + configPath, cLog::eLOG_TYPE_ERROR);
		Log.write (configFile.GetErrorStr1(), cLog::eLOG_TYPE_ERROR);
		Log.write (configFile.GetErrorStr2(), cLog::eLOG_TYPE_ERROR);
	}

	setSetting (configFile, "Options~Game~EnableDebug", gameSettings.debug);
	setSetting (configFile, "Options~Game~EnableAutosave", gameSettings.autosave);
	setSetting (configFile, "Options~Game~EnableAnimations", gameSettings.animations);
	setSetting (configFile, "Options~Game~EnableShadows", gameSettings.shadows);
	setSetting (configFile, "Options~Game~EnableAlphaEffects", gameSettings.alphaEffects);

	setSetting (configFile, "Options~Game~EnableDescribtions", gameSettings.showDescription);
	setSetting (configFile, "Options~Game~EnableDamageEffects", gameSettings.damageEffects);
	setSetting (configFile, "Options~Game~EnableDamageEffectsVehicles", gameSettings.damageEffectsVehicles);
	setSetting (configFile, "Options~Game~EnableMakeTracks", gameSettings.makeTracks);
	setSetting (configFile, "Options~Game~ScrollSpeed", gameSettings.scrollSpeed);

	setSetting (configFile, "Options~Game~Net~IP", networkAddress.ip.c_str());
	setSetting (configFile, "Options~Game~Net~Port", networkAddress.port);

	setSetting (configFile, "Options~Game~Net~PlayerName", playerSettings.name.c_str());
	setSetting (configFile, "Options~Game~Net~PlayerColor", playerSettings.color.r, "red");
	setSetting (configFile, "Options~Game~Net~PlayerColor", playerSettings.color.g, "green");
	setSetting (configFile, "Options~Game~Net~PlayerColor", playerSettings.color.b, "blue");

	setSetting (configFile, "Options~Game~Paths~Gamedata", dataDir.c_str());
	setSetting (configFile, "Options~Game~Paths~Saves", pathSettings.savesPath.c_str());

	setSetting (configFile, "Options~Game~Paths~Buildings", pathSettings.buildingsPath.c_str());
	setSetting (configFile, "Options~Game~Paths~Fonts", pathSettings.fontPath.c_str());
	setSetting (configFile, "Options~Game~Paths~FX", pathSettings.fxPath.c_str());
	setSetting (configFile, "Options~Game~Paths~GFX", pathSettings.gfxPath.c_str());
	setSetting (configFile, "Options~Game~Paths~Languages", pathSettings.langPath.c_str());
	setSetting (configFile, "Options~Game~Paths~Maps", pathSettings.mapsPath.c_str());
	setSetting (configFile, "Options~Game~Paths~MVEs", pathSettings.mvePath.c_str());
	setSetting (configFile, "Options~Game~Paths~Music", pathSettings.musicPath.c_str());
	setSetting (configFile, "Options~Game~Paths~Sounds", pathSettings.soundsPath.c_str());
	setSetting (configFile, "Options~Game~Paths~Vehicles", pathSettings.vehiclesPath.c_str());
	setSetting (configFile, "Options~Game~Paths~Voices", pathSettings.voicesPath.c_str());

	setSetting (configFile, "Options~Game~Sound~Enabled", soundSettings.soundEnabled);
	setSetting (configFile, "Options~Game~Sound~MusicVol", soundSettings.musicVol);
	setSetting (configFile, "Options~Game~Sound~SoundVol", soundSettings.soundVol);
	setSetting (configFile, "Options~Game~Sound~VoiceVol", soundSettings.voiceVol);
	setSetting (configFile, "Options~Game~Sound~ChunkSize", soundSettings.chunkSize);
	setSetting (configFile, "Options~Game~Sound~Frequency", soundSettings.frequency);
	setSetting (configFile, "Options~Game~Sound~MusicMute", soundSettings.musicMute);
	setSetting (configFile, "Options~Game~Sound~SoundMute", soundSettings.soundMute);
	setSetting (configFile, "Options~Game~Sound~VoiceMute", soundSettings.voiceMute);
	setSetting (configFile, "Options~Game~Sound~Sound3D", soundSettings.sound3d);

	if (videoSettings.resolution)
	{
		setSetting (configFile, "Options~Start~Resolution", (std::to_string (videoSettings.resolution->x()) + "." + std::to_string (videoSettings.resolution->y())).c_str());
	}
	setSetting (configFile, "Options~Start~Windowmode", videoSettings.windowMode);
	setSetting (configFile, "Options~Start~ColourDepth", videoSettings.colourDepth);
	setSetting (configFile, "Options~Start~Display", videoSettings.displayIndex);

	setSetting (configFile, "Options~Start~Intro", startSettings.showIntro);
	setSetting (configFile, "Options~Start~Language", startSettings.language.c_str());
	setSetting (configFile, "Options~Start~CacheSize", startSettings.cacheSize);
	setSetting (configFile, "Options~Start~Fastmode", startSettings.fastMode);
	setSetting (configFile, "Options~Start~PreScale", startSettings.preScale);
	setSetting (configFile, "Options~Start~VoiceLanguage", startSettings.voiceLanguage.c_str());

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
