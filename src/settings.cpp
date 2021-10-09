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
#include "maxrversion.h"
#include "ui/graphical/playercolor.h"
#include "utility/extendedtinyxml.h"
#include "utility/files.h"
#include "utility/log.h"
#include "utility/string/tolower.h"
#include "output/video/video.h"

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
bool cSettings::createConfigFile()
{
	configFile.Clear();
	configFile.LinkEndChild (configFile.NewDeclaration());
	configFile.LinkEndChild (configFile.NewElement ("Options"));

	// create new empty config
	if (configFile.SaveFile (configPath.c_str()) != XML_NO_ERROR)
	{
		Log.write ("Could not write new config to " + configPath, cLog::eLOG_TYPE_ERROR);
		return false; // Generate fails
	}
	return true;
}

//------------------------------------------------------------------------------
void cSettings::initialize()
{
	std::unique_lock<std::recursive_mutex> lock (xmlDocMutex);
	initializing = true;

	if (initialized) return;

	setPaths();

	if (configFile.LoadFile (configPath.c_str()) != XML_NO_ERROR)
	{
		Log.write ("Can't read maxr.xml\n", cLog::eLOG_TYPE_WARNING);
		if (!createConfigFile()) return;
	}

	XMLElement* xmlElement = nullptr;

	// START

	if (!DEDICATED_SERVER)
	{
		// =====================================================================
		xmlElement = XmlGetFirstElement (configFile, "Options", {"Start", "Resolution"});
		if (!xmlElement || !xmlElement->Attribute ("Text"))
		{
			Log.write ("Can't load resolution from config file: using default value", cLog::eLOG_TYPE_WARNING);
			Video.setResolution (Video.getMinW(), Video.getMinH(), false);
			saveResolution();
		}
		else
		{
			std::string temp = xmlElement->Attribute ("Text");
			int wTmp = atoi (temp.substr (0, temp.find (".", 0)).c_str());
			int hTmp = atoi (temp.substr (temp.find (".", 0) + 1, temp.length()).c_str());
			Video.setResolution (wTmp, hTmp, false);
		}

		// =====================================================================
		xmlElement = XmlGetFirstElement (configFile, "Options", {"Start", "ColourDepth"});
		if (!xmlElement || !xmlElement->Attribute ("Num"))
		{
			Log.write ("Can't load color depth from config file: using default value", cLog::eLOG_TYPE_WARNING);
			Video.setColDepth (32);
			saveColorDepth();
		}
		else
		{
			Video.setColDepth (xmlElement->IntAttribute ("Num"));
		}

		// =====================================================================
		xmlElement = XmlGetFirstElement (configFile, "Options", {"Start", "Intro"});
		if (!xmlElement || !xmlElement->Attribute ("YN"))
		{
			Log.write ("Can't load intro from config file: using default value", cLog::eLOG_TYPE_WARNING);
			setShowIntro (true, true);
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
			Video.setWindowMode (true);
			saveWindowMode();
		}
		else
		{
			Video.setWindowMode (getXMLAttributeBoolFromElement (xmlElement, "YN"));
		}

		// =====================================================================
		xmlElement = XmlGetFirstElement (configFile, "Options", {"Start", "Display"});
		if (!xmlElement || !xmlElement->Attribute ("Num"))
		{
			Log.write ("Can't load display index from config file: using default value", cLog::eLOG_TYPE_WARNING);
			Video.setDisplayIndex (0);
			saveDisplayIndex();
		}
		else
		{
			Video.setDisplayIndex (xmlElement->IntAttribute ("Num"));
		}

		// =====================================================================
		xmlElement = XmlGetFirstElement (configFile, "Options", {"Start", "Fastmode"});
		if (!xmlElement || !xmlElement->Attribute ("YN"))
		{
			Log.write ("Can't load fast mode from config file: using default value", cLog::eLOG_TYPE_WARNING);
			startSettings.fastMode = false;
			saveSetting ("Options~Start~Fastmode", false);
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
			saveSetting ("Options~Start~PreScale", false);
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
			setCacheSize (400, true);
		}
		else
		{
			startSettings.cacheSize = xmlElement->IntAttribute ("Num");
		}
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Start", "Language"});
	if (!xmlElement || !xmlElement->Attribute ("Text"))
	{
		Log.write ("Can't load language from config file: using default value", cLog::eLOG_TYPE_WARNING);
		setLanguage ("en", true);
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
		saveSetting ("Options~Start~VoiceLanguage", "");
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
		setAutosave (true, true);
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
		setDebug (true, true);
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
		setAnimations (true, true);
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
		setShadows (true, true);
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
		setAlphaEffects (true, true);
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
		setShowDescription (true, true);
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
		setDamageEffects (true, true);
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
		setDamageEffectsVehicles (true, true);
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
		setMakeTracks (true, true);
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
		setScrollSpeed (32, true);
	}
	else
	{
		gameSettings.scrollSpeed = xmlElement->IntAttribute ("Num");
	}

	// GAME-SOUND
	if (!DEDICATED_SERVER)
	{
		// =====================================================================
		xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "Sound", "Enabled"});
		if (!xmlElement || !xmlElement->Attribute ("YN"))
		{
			Log.write ("Can't load sound enabled from config file: using default value", cLog::eLOG_TYPE_WARNING);
			setSoundEnabled (true, true);
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
			setMusicMute (false, true);
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
			setSoundMute (false, true);
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
			setVoiceMute (false, true);
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
			set3DSound (true, true);
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
			setMusicVol (128, true);
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
			setSoundVol (128, true);
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
			setVoiceVol (128, true);
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
			setChunkSize (2048, true);
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
			setFrequence (44100, true);
		}
		else
		{
			soundSettings.frequency = xmlElement->IntAttribute ("Num");
		}
	}

	// PATHS
	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "Paths", "Gamedata"});
	if (!xmlElement || !xmlElement->Attribute ("Text"))
	{
		Log.write ("Can't load gamedata from config file: using default value", cLog::eLOG_TYPE_WARNING);
		setDataDir (searchDataDir ("").c_str(), true);
	}
	else
	{
		setDataDir (searchDataDir (xmlElement->Attribute ("Text")).c_str(), false);
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "Paths", "Languages"});
	if (!xmlElement || !xmlElement->Attribute ("Text"))
	{
		Log.write ("Can't load language path from config file: using default value", cLog::eLOG_TYPE_WARNING);
		saveSetting ("Options~Game~Paths~Languages", "languages");
		pathSettings.langPath = dataDir + "languages";
	}
	else
	{
		pathSettings.langPath = dataDir + xmlElement->Attribute ("Text");
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "Paths", "Fonts"});
	if (!xmlElement || !xmlElement->Attribute ("Text"))
	{
		Log.write ("Can't load fonts path from config file: using default value", cLog::eLOG_TYPE_WARNING);
		saveSetting ("Options~Game~Paths~Fonts", "fonts");
		pathSettings.fontPath = dataDir + "fonts";
	}
	else
	{
		pathSettings.fontPath = dataDir + xmlElement->Attribute ("Text");
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "Paths", "FX"});
	if (!xmlElement || !xmlElement->Attribute ("Text"))
	{
		Log.write ("Can't load fx path from config file: using default value", cLog::eLOG_TYPE_WARNING);
		saveSetting ("Options~Game~Paths~FX", "fx");
		pathSettings.fxPath = dataDir + "fx";
	}
	else
	{
		pathSettings.fxPath = dataDir + xmlElement->Attribute ("Text");
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "Paths", "GFX"});
	if (!xmlElement || !xmlElement->Attribute ("Text"))
	{
		Log.write ("Can't load gfx path from config file: using default value", cLog::eLOG_TYPE_WARNING);
		saveSetting ("Options~Game~Paths~GFX", "gfx");
		pathSettings.gfxPath = dataDir + "gfx";
	}
	else
	{
		pathSettings.gfxPath = dataDir + xmlElement->Attribute ("Text");
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "Paths", "Maps"});
	if (!xmlElement || !xmlElement->Attribute ("Text"))
	{
		Log.write ("Can't load maps path from config file: using default value", cLog::eLOG_TYPE_WARNING);
		saveSetting ("Options~Game~Paths~Maps", "maps");
		pathSettings.mapsPath = dataDir + "maps";
	}
	else
	{
		pathSettings.mapsPath = dataDir + xmlElement->Attribute ("Text");
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "Paths", "Saves"});
	if (!xmlElement || !xmlElement->Attribute ("Text"))
	{
		Log.write ("Can't load saves path from config file: using default value", cLog::eLOG_TYPE_WARNING);
		saveSetting ("Options~Game~Paths~Saves", "saves");
		pathSettings.savesPath = homeDir + "saves";
	}
	else if (std::string (xmlElement->Attribute ("Text")) == "saves")
	{
		pathSettings.savesPath = homeDir + xmlElement->Attribute ("Text");
	}
	else
	{
		// use absolute paths for saves - do not add dataDir or homeDir
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

	if (!DEDICATED_SERVER)
	{
		// =====================================================================
		xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "Paths", "Sounds"});
		if (!xmlElement || !xmlElement->Attribute ("Text"))
		{
			Log.write ("Can't load sounds path from config file: using default value", cLog::eLOG_TYPE_WARNING);
			saveSetting ("Options~Game~Paths~Sounds", "sounds");
			pathSettings.soundsPath = dataDir + "sounds";
		}
		else
		{
			pathSettings.soundsPath = dataDir + xmlElement->Attribute ("Text");
		}

		// =====================================================================
		xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "Paths", "Voices"});
		if (!xmlElement || !xmlElement->Attribute ("Text"))
		{
			Log.write ("Can't load voices path from config file: using default value", cLog::eLOG_TYPE_WARNING);
			saveSetting ("Options~Game~Paths~Voices", "voices");
			pathSettings.voicesPath = dataDir + "voices";
		}
		else
		{
			pathSettings.voicesPath = dataDir + xmlElement->Attribute ("Text");
		}

		// =====================================================================
		xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "Paths", "Music"});
		if (!xmlElement || !xmlElement->Attribute ("Text"))
		{
			Log.write ("Can't load music path from config file: using default value", cLog::eLOG_TYPE_WARNING);
			saveSetting ("Options~Game~Paths~Music", "music");
			pathSettings.musicPath = dataDir + "music";
		}
		else
		{
			pathSettings.musicPath = dataDir + xmlElement->Attribute ("Text");
		}
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "Paths", "Vehicles"});
	if (!xmlElement || !xmlElement->Attribute ("Text"))
	{
		Log.write ("Can't load vehicles path from config file: using default value", cLog::eLOG_TYPE_WARNING);
		saveSetting ("Options~Game~Paths~Vehicles", "vehicles");
		pathSettings.vehiclesPath = dataDir + "vehicles";
	}
	else
	{
		pathSettings.vehiclesPath = dataDir + xmlElement->Attribute ("Text");
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "Paths", "Buildings"});
	if (!xmlElement || !xmlElement->Attribute ("Text"))
	{
		Log.write ("Can't load buildings path from config file: using default value", cLog::eLOG_TYPE_WARNING);
		saveSetting ("Options~Game~Paths~Buildings", "buildings");
		pathSettings.buildingsPath = dataDir + "buildings";
	}
	else
	{
		pathSettings.buildingsPath = dataDir + xmlElement->Attribute ("Text");
	}

	if (!DEDICATED_SERVER)
	{
		// =====================================================================
		xmlElement = XmlGetFirstElement (configFile, "Options", {"Game", "Paths", "MVEs"});
		if (!xmlElement || !xmlElement->Attribute ("Text"))
		{
			Log.write ("Can't load language path from config file: using default value", cLog::eLOG_TYPE_WARNING);
			saveSetting ("Options~Game~Paths~MVEs", "mve");
			pathSettings.mvePath = dataDir + "mve";
		}
		else
		{
			pathSettings.mvePath = dataDir + xmlElement->Attribute ("Text");
		}
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
	setNetworkAddress (xmlNetworkAddress, true);

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
	setPlayerSettings (xmlPlayerSettings, true);

	initialized = true;
	initializing = false;
}

//------------------------------------------------------------------------------
void cSettings::saveSetting (const std::string& path, const char* value)
{
	saveSetting (path, value, "Text");
}

//------------------------------------------------------------------------------
void cSettings::saveSetting (const std::string& path, int value)
{
	saveSetting (path, value, "Num");
}

//------------------------------------------------------------------------------
void cSettings::saveSetting (const std::string& path, unsigned int value)
{
	saveSetting (path, value, "Num");
}

//------------------------------------------------------------------------------
void cSettings::saveSetting (const std::string& path, bool value)
{
	if (value) saveSetting (path, "Yes", "YN");
	else saveSetting (path, "No", "YN");
}

//------------------------------------------------------------------------------
template <typename T>
void cSettings::saveSetting (const std::string& path, T value, const char* valueName)
{
	std::unique_lock<std::recursive_mutex> lock (xmlDocMutex);

	XMLElement* xmlElement = getOrCreateXmlElement (configFile, path);
	if (xmlElement == nullptr) return;

	xmlElement->SetAttribute (valueName, value);

	if (configFile.SaveFile (configPath.c_str()) != XML_NO_ERROR)
	{
		Log.write ("Could not write new config to " + configPath, cLog::eLOG_TYPE_ERROR);
	}
}

//------------------------------------------------------------------------------
void cSettings::saveResolution()
{
	saveSetting ("Options~Start~Resolution", (std::to_string (Video.getResolutionX()) + "." + std::to_string (Video.getResolutionY())).c_str());
}

//------------------------------------------------------------------------------
void cSettings::saveWindowMode()
{
	saveSetting ("Options~Start~Windowmode", Video.getWindowMode());
}
//------------------------------------------------------------------------------
void cSettings::saveColorDepth()
{
	saveSetting ("Options~Start~ColorDepth", Video.getColDepth());
}
//------------------------------------------------------------------------------
void cSettings::saveDisplayIndex()
{
	saveSetting ("Options~Start~Display", Video.getDisplayIndex());
}

//------------------------------------------------------------------------------
bool cSettings::isDebug() const
{
	return gameSettings.debug;
}

//------------------------------------------------------------------------------
void cSettings::setDebug (bool debug, bool save)
{
	gameSettings.debug = debug;
	if (save) saveSetting ("Options~Game~EnableDebug", debug);
}

//------------------------------------------------------------------------------
bool cSettings::shouldAutosave() const
{
	return gameSettings.autosave;
}

//------------------------------------------------------------------------------
void cSettings::setAutosave (bool autosave, bool save)
{
	gameSettings.autosave = autosave;
	if (save) saveSetting ("Options~Game~EnableAutosave", autosave);
}

//------------------------------------------------------------------------------
bool cSettings::isAnimations() const
{
	return gameSettings.animations;
}

//------------------------------------------------------------------------------
void cSettings::setAnimations (bool animations, bool save)
{
	std::swap (gameSettings.animations, animations);
	if (save) saveSetting ("Options~Game~EnableAnimations", gameSettings.animations);
	if (gameSettings.animations != animations) animationsChanged();
}

//------------------------------------------------------------------------------
bool cSettings::isShadows() const
{
	return gameSettings.shadows;
}

//------------------------------------------------------------------------------
void cSettings::setShadows (bool shadows, bool save)
{
	gameSettings.shadows = shadows;
	if (save) saveSetting ("Options~Game~EnableShadows", shadows);
}

//------------------------------------------------------------------------------
bool cSettings::isAlphaEffects() const
{
	return gameSettings.alphaEffects;
}

//------------------------------------------------------------------------------
void cSettings::setAlphaEffects (bool alphaEffects, bool save)
{
	gameSettings.alphaEffects = alphaEffects;
	if (save) saveSetting ("Options~Game~EnableAlphaEffects", alphaEffects);
}

//------------------------------------------------------------------------------
bool cSettings::shouldShowDescription() const
{
	return gameSettings.showDescription;
}

//------------------------------------------------------------------------------
void cSettings::setShowDescription (bool showDescription, bool save)
{
	gameSettings.showDescription = showDescription;
	if (save) saveSetting ("Options~Game~EnableDescribtions", showDescription);
}

//------------------------------------------------------------------------------
bool cSettings::isDamageEffects() const
{
	return gameSettings.damageEffects;
}

//------------------------------------------------------------------------------
void cSettings::setDamageEffects (bool damageEffects, bool save)
{
	gameSettings.damageEffects = damageEffects;
	if (save) saveSetting ("Options~Game~EnableDamageEffects", damageEffects);
}

//------------------------------------------------------------------------------
bool cSettings::isDamageEffectsVehicles() const
{
	return gameSettings.damageEffectsVehicles;
}

//------------------------------------------------------------------------------
void cSettings::setDamageEffectsVehicles (bool damageEffectsVehicles, bool save)
{
	gameSettings.damageEffectsVehicles = damageEffectsVehicles;
	if (save) saveSetting ("Options~Game~EnableDamageEffectsVehicles", damageEffectsVehicles);
}

//------------------------------------------------------------------------------
bool cSettings::isMakeTracks() const
{
	return gameSettings.makeTracks;
}

//------------------------------------------------------------------------------
void cSettings::setMakeTracks (bool makeTracks, bool save)
{
	gameSettings.makeTracks = makeTracks;
	if (save) saveSetting ("Options~Game~EnableMakeTracks", makeTracks);
}

//------------------------------------------------------------------------------
int cSettings::getScrollSpeed() const
{
	return gameSettings.scrollSpeed;
}

//------------------------------------------------------------------------------
void cSettings::setScrollSpeed (int scrollSpeed, bool save)
{
	gameSettings.scrollSpeed = scrollSpeed;
	if (save) saveSetting ("Options~Game~ScrollSpeed", scrollSpeed);
}

//------------------------------------------------------------------------------
void cSettings::setNetworkAddress (const sNetworkAddress& networkAddress, bool save)
{
	this->networkAddress = networkAddress;
	if (save)
	{
		saveSetting ("Options~Game~Net~IP", networkAddress.ip.c_str());
		saveSetting ("Options~Game~Net~Port", networkAddress.port);
	}
}

//------------------------------------------------------------------------------
void cSettings::setPlayerSettings (const sPlayerSettings& playerSettings, bool save)
{
	this->playerSettings = playerSettings;
	if (save)
	{
		saveSetting ("Options~Game~Net~PlayerName", playerSettings.name.c_str());
		saveSetting ("Options~Game~Net~PlayerColor", playerSettings.color.r, "red");
		saveSetting ("Options~Game~Net~PlayerColor", playerSettings.color.g, "green");
		saveSetting ("Options~Game~Net~PlayerColor", playerSettings.color.b, "blue");
	}
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
void cSettings::setDataDir (const char* dataDir, bool save)
{
	this->dataDir = dataDir;
	if (save) saveSetting ("Options~Game~Paths~Gamedata", dataDir);
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
void cSettings::setSoundEnabled (bool soundEnabled, bool save)
{
	soundSettings.soundEnabled = soundEnabled;
	if (save) saveSetting ("Options~Game~Sound~Enabled", soundEnabled);
}

//------------------------------------------------------------------------------
int cSettings::getMusicVol() const
{
	return soundSettings.musicVol;
}

//------------------------------------------------------------------------------
void cSettings::setMusicVol (int musicVol, bool save)
{
	soundSettings.musicVol = musicVol;
	if (save) saveSetting ("Options~Game~Sound~MusicVol", musicVol);
}

//------------------------------------------------------------------------------
int cSettings::getSoundVol() const
{
	return soundSettings.soundVol;
}

//------------------------------------------------------------------------------
void cSettings::setSoundVol (int soundVol, bool save)
{
	soundSettings.soundVol = soundVol;
	if (save) saveSetting ("Options~Game~Sound~SoundVol", soundVol);
}

//------------------------------------------------------------------------------
int cSettings::getVoiceVol() const
{
	return soundSettings.voiceVol;
}

//------------------------------------------------------------------------------
void cSettings::setVoiceVol (int voiceVol, bool save)
{
	soundSettings.voiceVol = voiceVol;
	if (save) saveSetting ("Options~Game~Sound~VoiceVol", voiceVol);
}

//------------------------------------------------------------------------------
int cSettings::getChunkSize() const
{
	return soundSettings.chunkSize;
}

//------------------------------------------------------------------------------
void cSettings::setChunkSize (int chunkSize, bool save)
{
	soundSettings.chunkSize = chunkSize;
	if (save) saveSetting ("Options~Game~Sound~ChunkSize", chunkSize);
}

//------------------------------------------------------------------------------
int cSettings::getFrequency() const
{
	return soundSettings.frequency;
}

//------------------------------------------------------------------------------
void cSettings::setFrequence (int frequency, bool save)
{
	soundSettings.frequency = frequency;
	if (save) saveSetting ("Options~Game~Sound~Frequency", frequency);
}

//------------------------------------------------------------------------------
bool cSettings::isMusicMute() const
{
	return soundSettings.musicMute;
}

//------------------------------------------------------------------------------
void cSettings::setMusicMute (bool musicMute, bool save)
{
	soundSettings.musicMute = musicMute;
	if (save) saveSetting ("Options~Game~Sound~MusicMute", musicMute);
}

//------------------------------------------------------------------------------
bool cSettings::isSoundMute() const
{
	return soundSettings.soundMute;
}

//------------------------------------------------------------------------------
void cSettings::setSoundMute (bool soundMute, bool save)
{
	soundSettings.soundMute = soundMute;
	if (save) saveSetting ("Options~Game~Sound~SoundMute", soundMute);
}

//------------------------------------------------------------------------------
bool cSettings::isVoiceMute() const
{
	return soundSettings.voiceMute;
}

//------------------------------------------------------------------------------
void cSettings::setVoiceMute (bool voiceMute, bool save)
{
	soundSettings.voiceMute = voiceMute;
	if (save) saveSetting ("Options~Game~Sound~VoiceMute", voiceMute);
}

//------------------------------------------------------------------------------
bool cSettings::is3DSound() const
{
	return soundSettings.sound3d;
}

//------------------------------------------------------------------------------
void cSettings::set3DSound (bool sound3d, bool save)
{
	soundSettings.sound3d = sound3d;
	if (save) saveSetting ("Options~Game~Sound~Sound3D", sound3d);
}

//------------------------------------------------------------------------------
bool cSettings::shouldShowIntro() const
{
	return startSettings.showIntro;
}

//------------------------------------------------------------------------------
void cSettings::setShowIntro (bool showIntro, bool save)
{
	startSettings.showIntro = showIntro;
	if (save) saveSetting ("Options~Start~Intro", showIntro);
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
void cSettings::setLanguage (const char* language, bool save)
{
	startSettings.language = language;
	if (save) saveSetting ("Options~Start~Language", language);
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
void cSettings::setCacheSize (unsigned int cacheSize, bool save)
{
	startSettings.cacheSize = cacheSize;
	if (save) saveSetting ("Options~Start~CacheSize", cacheSize);
}

//------------------------------------------------------------------------------
const std::string& cSettings::getFontPath() const
{
	return pathSettings.fontPath;
}

//------------------------------------------------------------------------------
const std::string& cSettings::getFxPath() const
{
	return pathSettings.fxPath;
}

//------------------------------------------------------------------------------
const std::string& cSettings::getGfxPath() const
{
	return pathSettings.gfxPath;
}

//------------------------------------------------------------------------------
const std::string& cSettings::getLangPath() const
{
	return pathSettings.langPath;
}

//------------------------------------------------------------------------------
const std::string& cSettings::getMapsPath() const
{
	return pathSettings.mapsPath;
}

//------------------------------------------------------------------------------
const std::string& cSettings::getSavesPath() const
{
	return pathSettings.savesPath;
}

//------------------------------------------------------------------------------
const std::string& cSettings::getSoundsPath() const
{
	return pathSettings.soundsPath;
}

//------------------------------------------------------------------------------
const std::string& cSettings::getVoicesPath() const
{
	return pathSettings.voicesPath;
}

//------------------------------------------------------------------------------
const std::string& cSettings::getMusicPath() const
{
	return pathSettings.musicPath;
}

//------------------------------------------------------------------------------
const std::string& cSettings::getVehiclesPath() const
{
	return pathSettings.vehiclesPath;
}

//------------------------------------------------------------------------------
const std::string& cSettings::getBuildingsPath() const
{
	return pathSettings.buildingsPath;
}

//------------------------------------------------------------------------------
const std::string& cSettings::getMvePath() const
{
	return pathSettings.mvePath;
}
