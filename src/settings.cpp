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

#ifdef WIN32
# include <shlobj.h>
# include <direct.h>
#else
# include <sys/stat.h>
# include <unistd.h>
#endif

#include <iostream>
#include <locale>
#include <string>

#include "settings.h"

#include "extendedtinyxml.h"
#include "utility/files.h"
#include "utility/log.h"
#include "main.h"
#include "tinyxml2.h"
#include "video.h"
#include "utility/string/tolower.h"
#include "game/data/player/playercolor.h"

using namespace tinyxml2;

//------------------------------------------------------------------------------
cSettings cSettings::instance;

//------------------------------------------------------------------------------
cSettings::cSettings()
{
	initialized = false;
	initializing = false;
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
	// init absolutely needed paths
	logPath = MAX_LOG;
	netLogPath = getUserLogDir();
	// FIXME: I don't know how this is handled on win/mac/amiga -- beko
	exePath = "";
	homeDir = "";

#if MAC
	// do some rudimentary work with the user's homefolder.
	// Needs to be extended in future...
	char* cHome = getenv ("HOME");  //get $HOME on mac
	if (cHome != NULL)
		homeDir = cHome;
	if (!homeDir.empty())
	{
		homeDir += PATH_DELIMITER;
		homeDir += ".maxr";
		homeDir += PATH_DELIMITER;

		// check whether home dir is set up and readable
		if (!FileExists (homeDir.c_str())) // under mac everything is a file
		{
			if (mkdir (homeDir.c_str(), 0755) == 0)
				std::cout << "\n(II): Created new config directory " << homeDir;
			else
			{
				std::cout << "\n(EE): Can't create config directory " << homeDir;
				// reset $HOME since we can't create our config directory
				homeDir = "";
			}
		}
	}
	// this is also a good place to find out where the executable is located
	configPath = MAX_XML; // assume config in current working directory
#elif WIN32
	// this is where windowsuser should set their %HOME%
	// this is also a good place to find out where the executable is located
	configPath = MAX_XML; // assume config in current working directory

	TCHAR szPath[MAX_PATH];
	SHGetFolderPath (NULL, CSIDL_PERSONAL, NULL, 0, szPath);

#ifdef UNICODE
	std::wstring home = szPath;
#else
	std::string home = szPath;
#endif

	homeDir = std::string (home.begin(), home.end());

	std::cout << "\n(II): Read home directory " << homeDir;

	if (!homeDir.empty())
	{
		homeDir += PATH_DELIMITER;
		homeDir += "maxr";

		if (!DirExists (homeDir))
		{
			if (mkdir (homeDir.c_str()) == 0)
			{
				std::cout << "\n(II): Created new config directory " << homeDir;
			}
			else
			{
				std::cout << "\n(EE): Can't create config directory " << homeDir;
				homeDir = "";
			}
		}
		else std::cout << "\n(II): Config is read from " << homeDir;

		if (!homeDir.empty()) homeDir += PATH_DELIMITER;

		configPath = homeDir;
		configPath += MAX_XML;
	}

	// set new place for logs
	logPath = homeDir + logPath;
	netLogPath = getUserLogDir();
	std::cout << "\n(II): Starting logging to: " << logPath;
#elif __amigaos4__
	// this is where amigausers should set their %HOME%
	// this is also a good place to find out where the executable is located
	configPath = MAX_XML; // assume config in current working directory
#else
	// NOTE: I do not use cLog here on purpose.
	// Logs on linux go somewhere to $HOME/.maxr/
	// - as long as we can't access that we have to output everything to
	// the terminal because game's root dir is usually write protected! -- beko

	char* cHome = getenv ("HOME"); // get $HOME on linux
	if (cHome != NULL)
	{
		homeDir = cHome; // get $HOME on linux
	}

	if (!homeDir.empty())
	{
		homeDir += PATH_DELIMITER;
		homeDir += ".maxr";
		homeDir += PATH_DELIMITER;
		configPath = homeDir;
		configPath += MAX_XML; // set config to $HOME/.maxr/max.xml

		// check whether home dir is set up and readable.
		// under linux everything is a file -- beko
		if (!FileExists (homeDir.c_str()))
		{
			if (mkdir (homeDir.c_str(), 0755) == 0)
			{
				std::cout << "\n(II): Created new config directory " << homeDir;
			}
			else
			{
				std::cout << "\n(EE): Can't create config directory " << homeDir;
				// reset $HOME since we can't create our config directory
				homeDir = "";
			}
		}
		else
		{
			// config dir can be read - we're fine
			std::cout << "\n(II): Config is read from " << homeDir;
		}
	}
	else
	{
		std::cout << "\n(WW): $HOME is not set!";
		homeDir = "";
		configPath = MAX_XML; // assume config in current working directory
	}

	// set new place for logs
	logPath = homeDir + logPath;
	netLogPath = getUserLogDir();
	std::cout << "\n(II): Starting logging to: " << logPath;

	// determine full path to application
	// this needs /proc support that should be avaible
	// on most linux installations
	if (FileExists ("/proc/self/exe"))
	{
		int iSize;
		char cPathToExe[255];
		iSize = readlink ("/proc/self/exe", cPathToExe, sizeof (cPathToExe));
		if (iSize < 0)
		{
			Log.write ("Can't resolve full path to program. Doesn't this system feature /proc/self/exe?", cLog::eLOG_TYPE_WARNING);
		}
		else if (iSize >= 255)
		{
			Log.write ("Can't resolve full path to program since my array is to small and my programmer is to lame to write a buffer for me!", cLog::eLOG_TYPE_WARNING);
		}
		else
		{
			int iPos = 0;
			for (int i = 0; i < 255; i++)
			{
				// snip garbage after last PATH_DELIMITER + executable itself
				// (is reported on some linux systems
				//  as well using /proc/self/exe
				if (cPathToExe[i] == '/')
					iPos = i;
				// skip garbage that might lunger on heap after 0 termination
				if (cPathToExe[i] == '\0')
					i = 255;
			}

			exePath = cPathToExe;
			exePath = exePath.substr (0, iPos);
			exePath += PATH_DELIMITER;

			// check for binary itself in bin folder
			if (FileExists ((exePath + "maxr").c_str()))
			{
				Log.write ("Path to binary is: " + exePath, cLog::eLOG_TYPE_INFO);
			}
			else
			{
				// perhaps we got ourself a trailing maxr
				// in the path like /proc
				// seems to do it sometimes. remove it and try again!
				if (cPathToExe[iPos - 1] == 'r' && cPathToExe[iPos - 2] == 'x' && cPathToExe[iPos - 3] == 'a' && cPathToExe[iPos - 4] == 'm')
				{
					exePath = exePath.substr (0, iPos - 5);
					if (FileExists ((exePath + "maxr").c_str()))
					{
						Log.write ("Path to binary is: " + exePath, cLog::eLOG_TYPE_INFO);
					}
				}
			}

		}
	}
	else
	{
		Log.write ("Can't resolve full path to program. Doesn't this system feature /proc/self/exe?", cLog::eLOG_TYPE_WARNING);
		exePath = ""; //reset exePath
	}

	std::cout << "\n";
#endif
}


//------------------------------------------------------------------------------
std::string cSettings::searchDataDir (const std::string& sDataDirFromConf)
{
	std::string sPathToGameData = "";
#if MAC
	// assuming data is in same folder as binary (or current working directory)
	sPathToGameData = exePath;
#elif WIN32
	// assuming data is in same folder as binary (or current working directory)
	sPathToGameData = exePath;
#elif __amigaos4__
	// assuming data is in same folder as binary (or current working directory)
	sPathToGameData = exePath;
#else
	// BEGIN crude path validation to find gamedata
	Log.write ("Probing for data paths using default values:", cLog::eLOG_TYPE_INFO);

#define PATHCOUNT 11
	std::string sPathArray[PATHCOUNT] =
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
		exePath, // check for gamedata in bin folder too
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
	if (cDataDir == NULL)
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

	for (int i = 0; i < PATHCOUNT; i++)
	{
		std::string sInitFile = sPathArray[i];
		sInitFile += PATH_DELIMITER;
		sInitFile += "init.pcx";
		if (FileExists (sInitFile.c_str()))
		{
			sPathToGameData = sPathArray[i];
			sPathToGameData += PATH_DELIMITER;
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

//------------------------------------------------------------------------------
bool cSettings::createConfigFile()
{
	configFile.Clear();

	configFile.LinkEndChild (configFile.NewDeclaration ());

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
	cLockGuard<cMutex> lock (xmlDocMutex);
	initializing = true;

	if (initialized) return;

	setPaths();

	if (configPath.empty()) // we need at least configPath here
	{
		Log.write ("configPath not set!", cLog::eLOG_TYPE_WARNING);
		configPath = MAX_XML;
	}
	if (configFile.LoadFile (configPath.c_str()) != XML_NO_ERROR)
	{
		Log.write ("Can't read max.xml\n", LOG_TYPE_WARNING);
		if (!createConfigFile()) return;
	}

	XMLElement* xmlElement = NULL;

	// START

	if (!DEDICATED_SERVER)
	{
		// =====================================================================
		xmlElement = XmlGetFirstElement (configFile, "Options", "Start", "Resolution", NULL);
		if (!xmlElement || !xmlElement->Attribute ("Text"))
		{
			Log.write ("Can't load resolution from config file: using default value", LOG_TYPE_WARNING);
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
		xmlElement = XmlGetFirstElement (configFile, "Options", "Start", "ColourDepth", NULL);
		if (!xmlElement || !xmlElement->Attribute ("Num"))
		{
			Log.write ("Can't load color depth from config file: using default value", LOG_TYPE_WARNING);
			Video.setColDepth (32);
			saveColorDepth();
		}
		else
		{
			Video.setColDepth (xmlElement->IntAttribute ("Num"));
		}

		// =====================================================================
		xmlElement = XmlGetFirstElement (configFile, "Options", "Start", "Intro", NULL);
		if (!xmlElement || !xmlElement->Attribute ("YN"))
		{
			Log.write ("Can't load intro from config file: using default value", LOG_TYPE_WARNING);
			setShowIntro (true);
		}
		else
		{
			showIntro = xmlElement->BoolAttribute ("YN");
		}

		// =====================================================================
		xmlElement = XmlGetFirstElement (configFile, "Options", "Start", "Windowmode", NULL);
		if (!xmlElement || !xmlElement->Attribute ("YN"))
		{
			Log.write ("Can't load window mode from config file: using default value", LOG_TYPE_WARNING);
			Video.setWindowMode (true);
			saveWindowMode();
		}
		else
		{
			Video.setWindowMode (xmlElement->BoolAttribute ("YN"));
		}

		// =====================================================================
		xmlElement = XmlGetFirstElement (configFile, "Options", "Start", "Display", NULL);
		if (!xmlElement || !xmlElement->Attribute ("Num"))
		{
			Log.write ("Can't load display index from config file: using default value", LOG_TYPE_WARNING);
			Video.setDisplayIndex (0);
			saveDisplayIndex ();
		}
		else
		{
			Video.setDisplayIndex (xmlElement->IntAttribute ("Num"));
		}

		// =====================================================================
		xmlElement = XmlGetFirstElement (configFile, "Options", "Start", "Fastmode", NULL);
		if (!xmlElement || !xmlElement->Attribute ("YN"))
		{
			Log.write ("Can't load fast mode from config file: using default value", LOG_TYPE_WARNING);
			setFastMode (false);
		}
		else
		{
			fastMode = xmlElement->BoolAttribute ("YN");
		}

		// =====================================================================
		xmlElement = XmlGetFirstElement (configFile, "Options", "Start", "PreScale", NULL);
		if (!xmlElement || !xmlElement->Attribute ("YN"))
		{
			Log.write ("Can't load pre scale from config file: using default value", LOG_TYPE_WARNING);
			setDoPrescale (false);
		}
		else
		{
			preScale = xmlElement->BoolAttribute ("YN");
		}

		// =====================================================================
		xmlElement = XmlGetFirstElement (configFile, "Options", "Start", "CacheSize", NULL);
		if (!xmlElement || !xmlElement->Attribute ("Num"))
		{
			Log.write ("Can't load cache size from config file: using default value", LOG_TYPE_WARNING);
			setCacheSize (400);
		}
		else
		{
			cacheSize = xmlElement->IntAttribute ("Num");
		}
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", "Start", "Language", NULL);
	if (!xmlElement || !xmlElement->Attribute ("Text"))
	{
		Log.write ("Can't load language from config file: using default value", LOG_TYPE_WARNING);
		setLanguage ("ENG");
	}
	else
	{
		language = xmlElement->Attribute ("Text");
		for (std::string::iterator i = language.begin(), end = language.end(); i != end; ++i)
			*i = std::toupper ((unsigned char) * i);
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", "Start", "VoiceLanguage", NULL);
	if (!xmlElement || !xmlElement->Attribute ("Text"))
	{
		Log.write ("Can't load language from config file: using default value", LOG_TYPE_WARNING);
		setVoiceLanguage ("");
	}
	else
	{
		voiceLanguage = xmlElement->Attribute ("Text");
		to_lower (voiceLanguage);
	}

	// GAME
	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", "Game", "EnableAutosave", NULL);
	if (!xmlElement || !xmlElement->Attribute ("YN"))
	{
		Log.write ("Can't load autosave from config file: using default value", LOG_TYPE_WARNING);
		setAutosave (true);
	}
	else
	{
		autosave = xmlElement->BoolAttribute ("YN");
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", "Game", "EnableDebug", NULL);
	if (!xmlElement || !xmlElement->Attribute ("YN"))
	{
		Log.write ("Can't load debug from config file: using default value", LOG_TYPE_WARNING);
		setDebug (true);
	}
	else
	{
		debug = xmlElement->BoolAttribute ("YN");
		if (!debug) Log.write ("Debugmode disabled - for verbose output please enable Debug in max.xml", cLog::eLOG_TYPE_WARNING);
		else Log.write ("Debugmode enabled", cLog::eLOG_TYPE_INFO);
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", "Game", "EnableAnimations", NULL);
	if (!xmlElement || !xmlElement->Attribute ("YN"))
	{
		Log.write ("Can't load animations from config file: using default value", LOG_TYPE_WARNING);
		setAnimations (true);
	}
	else
	{
		animations = xmlElement->BoolAttribute ("YN");
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", "Game", "EnableShadows", NULL);
	if (!xmlElement || !xmlElement->Attribute ("YN"))
	{
		Log.write ("Can't load shadows from config file: using default value", LOG_TYPE_WARNING);
		setShadows (true);
	}
	else
	{
		shadows = xmlElement->BoolAttribute ("YN");
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", "Game", "EnableAlphaEffects", NULL);
	if (!xmlElement || !xmlElement->Attribute ("YN"))
	{
		Log.write ("Can't load alpha effects from config file: using default value", LOG_TYPE_WARNING);
		setAlphaEffects (true);
	}
	else
	{
		alphaEffects = xmlElement->BoolAttribute ("YN");
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", "Game", "EnableDescribtions", NULL);
	if (!xmlElement || !xmlElement->Attribute ("YN"))
	{
		Log.write ("Can't load descriptions from config file: using default value", LOG_TYPE_WARNING);
		setShowDescription (true);
	}
	else
	{
		showDescription = xmlElement->BoolAttribute ("YN");
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", "Game", "EnableDamageEffects", NULL);
	if (!xmlElement || !xmlElement->Attribute ("YN"))
	{
		Log.write ("Can't load damage effects from config file: using default value", LOG_TYPE_WARNING);
		setDamageEffects (true);
	}
	else
	{
		damageEffects = xmlElement->BoolAttribute ("YN");
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", "Game", "EnableDamageEffectsVehicles", NULL);
	if (!xmlElement || !xmlElement->Attribute ("YN"))
	{
		Log.write ("Can't load damaga effects vehicles from config file: using default value", LOG_TYPE_WARNING);
		setDamageEffectsVehicles (true);
	}
	else
	{
		damageEffectsVehicles = xmlElement->BoolAttribute ("YN");
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", "Game", "EnableMakeTracks", NULL);
	if (!xmlElement || !xmlElement->Attribute ("YN"))
	{
		Log.write ("Can't load make tracks from config file: using default value", LOG_TYPE_WARNING);
		setMakeTracks (true);
	}
	else
	{
		makeTracks = xmlElement->BoolAttribute ("YN");
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", "Game", "ScrollSpeed", NULL);
	if (!xmlElement || !xmlElement->Attribute ("Num"))
	{
		Log.write ("Can't load scroll speed from config file: using default value", LOG_TYPE_WARNING);
		setScrollSpeed (32);
	}
	else
	{
		scrollSpeed = xmlElement->IntAttribute ("Num");
	}

	// GAME-SOUND
	if (!DEDICATED_SERVER)
	{
		// =====================================================================
		xmlElement = XmlGetFirstElement (configFile, "Options", "Game", "Sound", "Enabled", NULL);
		if (!xmlElement || !xmlElement->Attribute ("YN"))
		{
			Log.write ("Can't load sound enabled from config file: using default value", LOG_TYPE_WARNING);
			setSoundEnabled (true);
		}
		else
		{
			soundEnabled = xmlElement->BoolAttribute ("YN");
		}

		// =====================================================================
		xmlElement = XmlGetFirstElement (configFile, "Options", "Game", "Sound", "MusicMute", NULL);
		if (!xmlElement || !xmlElement->Attribute ("YN"))
		{
			Log.write ("Can't load music mute from config file: using default value", LOG_TYPE_WARNING);
			setMusicMute (false);
		}
		else
		{
			musicMute = xmlElement->BoolAttribute ("YN");
		}

		// =====================================================================
		xmlElement = XmlGetFirstElement (configFile, "Options", "Game", "Sound", "SoundMute", NULL);
		if (!xmlElement || !xmlElement->Attribute ("YN"))
		{
			Log.write ("Can't load sound mute from config file: using default value", LOG_TYPE_WARNING);
			setSoundMute (false);
		}
		else
		{
			soundMute = xmlElement->BoolAttribute ("YN");
		}

		// =====================================================================
		xmlElement = XmlGetFirstElement (configFile, "Options", "Game", "Sound", "VoiceMute", NULL);
		if (!xmlElement || !xmlElement->Attribute ("YN"))
		{
			Log.write ("Can't load voice mute from config file: using default value", LOG_TYPE_WARNING);
			setVoiceMute (false);
		}
		else
		{
			voiceMute = xmlElement->BoolAttribute ("YN");
		}

		// =====================================================================
		xmlElement = XmlGetFirstElement (configFile, "Options", "Game", "Sound", "Sound3D", NULL);
		if (!xmlElement || !xmlElement->Attribute ("YN"))
		{
			Log.write ("Can't load 3D sound from config file: using default value", LOG_TYPE_WARNING);
			set3DSound (true);
		}
		else
		{
			sound3d = xmlElement->BoolAttribute ("YN");
		}

		// =====================================================================
		xmlElement = XmlGetFirstElement (configFile, "Options", "Game", "Sound", "MusicVol", NULL);
		if (!xmlElement || !xmlElement->Attribute ("Num"))
		{
			Log.write ("Can't load music volume from config file: using default value", LOG_TYPE_WARNING);
			setMusicVol (128);
		}
		else
		{
			musicVol = xmlElement->IntAttribute ("Num");
		}

		// =====================================================================
		xmlElement = XmlGetFirstElement (configFile, "Options", "Game", "Sound", "SoundVol", NULL);
		if (!xmlElement || !xmlElement->Attribute ("Num"))
		{
			Log.write ("Can't load music volume from config file: using default value", LOG_TYPE_WARNING);
			setSoundVol (128);
		}
		else
		{
			soundVol = xmlElement->IntAttribute ("Num");
		}

		// =====================================================================
		xmlElement = XmlGetFirstElement (configFile, "Options", "Game", "Sound", "VoiceVol", NULL);
		if (!xmlElement || !xmlElement->Attribute ("Num"))
		{
			Log.write ("Can't load music volume from config file: using default value", LOG_TYPE_WARNING);
			setVoiceVol (128);
		}
		else
		{
			voiceVol = xmlElement->IntAttribute ("Num");
		}

		// =====================================================================
		xmlElement = XmlGetFirstElement (configFile, "Options", "Game", "Sound", "ChunkSize", NULL);
		if (!xmlElement || !xmlElement->Attribute ("Num"))
		{
			Log.write ("Can't load music volume from config file: using default value", LOG_TYPE_WARNING);
			setChunkSize (2048);
		}
		else
		{
			chunkSize = xmlElement->IntAttribute ("Num");
		}

		// =====================================================================
		xmlElement = XmlGetFirstElement (configFile, "Options", "Game", "Sound", "Frequency", NULL);
		if (!xmlElement || !xmlElement->Attribute ("Num"))
		{
			Log.write ("Can't load music volume from config file: using default value", LOG_TYPE_WARNING);
			setFrequence (44100);
		}
		else
		{
			frequency = xmlElement->IntAttribute ("Num");
		}
	}

	// PATHS
	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", "Game", "Paths", "Gamedata", NULL);
	if (!xmlElement || !xmlElement->Attribute ("Text"))
	{
		Log.write ("Can't load gamedata from config file: using default value", LOG_TYPE_WARNING);
		setDataDir (searchDataDir().c_str(), true);
	}
	else
	{
		setDataDir (searchDataDir (xmlElement->Attribute ("Text")).c_str(), false);
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", "Game", "Paths", "Languages", NULL);
	if (!xmlElement || !xmlElement->Attribute ("Text"))
	{
		Log.write ("Can't load language path from config file: using default value", LOG_TYPE_WARNING);
		setLangPath ("languages");
		langPath = dataDir + "languages";
	}
	else
	{
		langPath = dataDir + xmlElement->Attribute ("Text");
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", "Game", "Paths", "Fonts", NULL);
	if (!xmlElement || !xmlElement->Attribute ("Text"))
	{
		Log.write ("Can't load fonts path from config file: using default value", LOG_TYPE_WARNING);
		setFontPath ("fonts");
		fontPath = dataDir + "fonts";
	}
	else
	{
		fontPath = dataDir + xmlElement->Attribute ("Text");
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", "Game", "Paths", "FX", NULL);
	if (!xmlElement || !xmlElement->Attribute ("Text"))
	{
		Log.write ("Can't load fx path from config file: using default value", LOG_TYPE_WARNING);
		setFxPath ("fx");
		fxPath = dataDir + "fx";
	}
	else
	{
		fxPath = dataDir + xmlElement->Attribute ("Text");
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", "Game", "Paths", "GFX", NULL);
	if (!xmlElement || !xmlElement->Attribute ("Text"))
	{
		Log.write ("Can't load gfx path from config file: using default value", LOG_TYPE_WARNING);
		setGfxPath ("gfx");
		gfxPath = dataDir + "gfx";
	}
	else
	{
		gfxPath = dataDir + xmlElement->Attribute ("Text");
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", "Game", "Paths", "Maps", NULL);
	if (!xmlElement || !xmlElement->Attribute ("Text"))
	{
		Log.write ("Can't load maps path from config file: using default value", LOG_TYPE_WARNING);
		setMapsPath ("maps");
		mapsPath = dataDir + "maps";
	}
	else
	{
		mapsPath = dataDir + xmlElement->Attribute ("Text");
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", "Game", "Paths", "Saves", NULL);
	if (!xmlElement || !xmlElement->Attribute ("Text"))
	{
		Log.write ("Can't load saves path from config file: using default value", LOG_TYPE_WARNING);
		setSavesPath ("saves");
		savesPath = homeDir + "saves";
	}
	else if (std::string (xmlElement->Attribute ("Text")) == "saves")
	{
		savesPath = homeDir + xmlElement->Attribute ("Text");
	}
	else
	{
		// use absolute paths for saves - do not add dataDir or homeDir
		savesPath = xmlElement->Attribute ("Text");
	}
#if MAC
	// Create saves directory, if it doesn't exist, yet.
	// Creating it during setPaths is too early, because it was not read yet.
	if (!FileExists (getSavesPath().c_str()))
	{
		if (mkdir (getSavesPath().c_str(), 0755) == 0)
			Log.write ("Created new save directory " + getSavesPath(), LOG_TYPE_INFO);
		else
			Log.write ("Can't create save directory " + getSavesPath(), LOG_TYPE_ERROR);
	}
#endif

	if (!DEDICATED_SERVER)
	{
		// =====================================================================
		xmlElement = XmlGetFirstElement (configFile, "Options", "Game", "Paths", "Sounds", NULL);
		if (!xmlElement || !xmlElement->Attribute ("Text"))
		{
			Log.write ("Can't load sounds path from config file: using default value", LOG_TYPE_WARNING);
			setSoundsPath ("sounds");
			soundsPath = dataDir + "sounds";
		}
		else
		{
			soundsPath = dataDir + xmlElement->Attribute ("Text");
		}

		// =====================================================================
		xmlElement = XmlGetFirstElement (configFile, "Options", "Game", "Paths", "Voices", NULL);
		if (!xmlElement || !xmlElement->Attribute ("Text"))
		{
			Log.write ("Can't load voices path from config file: using default value", LOG_TYPE_WARNING);
			setVoicesPath ("voices");
			voicesPath = dataDir + "voices";
		}
		else
		{
			voicesPath = dataDir + xmlElement->Attribute ("Text");
		}

		// =====================================================================
		xmlElement = XmlGetFirstElement (configFile, "Options", "Game", "Paths", "Music", NULL);
		if (!xmlElement || !xmlElement->Attribute ("Text"))
		{
			Log.write ("Can't load music path from config file: using default value", LOG_TYPE_WARNING);
			setMusicPath ("music");
			musicPath = dataDir + "music";
		}
		else
		{
			musicPath = dataDir + xmlElement->Attribute ("Text");
		}
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", "Game", "Paths", "Vehicles", NULL);
	if (!xmlElement || !xmlElement->Attribute ("Text"))
	{
		Log.write ("Can't load vehicles path from config file: using default value", LOG_TYPE_WARNING);
		setVehiclesPath ("vehicles");
		vehiclesPath = dataDir + "vehicles";
	}
	else
	{
		vehiclesPath = dataDir + xmlElement->Attribute ("Text");
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", "Game", "Paths", "Buildings", NULL);
	if (!xmlElement || !xmlElement->Attribute ("Text"))
	{
		Log.write ("Can't load buildings path from config file: using default value", LOG_TYPE_WARNING);
		setBuildingsPath ("buildings");
		buildingsPath = dataDir + "buildings";
	}
	else
	{
		buildingsPath = dataDir + xmlElement->Attribute ("Text");
	}

	if (!DEDICATED_SERVER)
	{
		// =====================================================================
		xmlElement = XmlGetFirstElement (configFile, "Options", "Game", "Paths", "MVEs", NULL);
		if (!xmlElement || !xmlElement->Attribute ("Text"))
		{
			Log.write ("Can't load language path from config file: using default value", LOG_TYPE_WARNING);
			setMvePath ("mve");
			mvePath = dataDir + "mve";
		}
		else
		{
			mvePath = dataDir + xmlElement->Attribute ("Text");
		}
	}

	// GAME-NET
	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", "Game", "Net", "IP", NULL);
	if (!xmlElement || !xmlElement->Attribute ("Text"))
	{
		Log.write ("Can't load IP from config file: using default value", LOG_TYPE_WARNING);
		setIP ("127.0.0.1");
	}
	else
	{
		setIP (xmlElement->Attribute ("Text"));
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", "Game", "Net", "Port", NULL);
	if (!xmlElement || !xmlElement->Attribute ("Num"))
	{
		Log.write ("Can't load Port config file: using default value", LOG_TYPE_WARNING);
		setPort (DEFAULTPORT);
	}
	else
	{
		port = xmlElement->IntAttribute ("Num");
	}

	// =========================================================================
	auto playerXmlElement = XmlGetFirstElement (configFile, "Options", "Game", "Net", "PlayerName", NULL);
	if (!playerXmlElement || !playerXmlElement->Attribute ("Text"))
	{
		Log.write ("Can't load player name from config file: using default value", LOG_TYPE_WARNING);

		char* user;

#ifdef WIN32
		user = getenv ("USERNAME");
#elif __amigaos4__
		user = "AmigaOS4-User";
#else
		user = getenv ("USER");  //get $USER on linux
#endif

		if (user != NULL) setPlayerName (user);
		else setPlayerName ("Commander");
	}
	else
	{
		setPlayerName (playerXmlElement->Attribute ("Text"));
	}

	// =========================================================================
	xmlElement = XmlGetFirstElement (configFile, "Options", "Game", "Net", "PlayerColor", NULL);
	if (!xmlElement || !xmlElement->Attribute ("red") || !xmlElement->Attribute ("green") || !xmlElement->Attribute ("blue"))
	{
		// try to load color by index for downward compatibility
		if (playerXmlElement && playerXmlElement->Attribute ("Num"))
		{
			const auto colorIndex = playerXmlElement->IntAttribute ("Num") % cPlayerColor::predefinedColorsCount;
			playerColor = cRgbColor (cPlayerColor::predefinedColors[colorIndex]);
		}
		else
		{
			Log.write ("Can't load player color from config file: using default value", LOG_TYPE_WARNING);
			setPlayerColor (cRgbColor::red ());
		}
	}
	else
	{
		playerColor = cRgbColor (xmlElement->IntAttribute ("red"), xmlElement->IntAttribute ("green"), xmlElement->IntAttribute ("blue"));
	}

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
template<typename T>
void cSettings::saveSetting (const std::string& path, T value, const char* valueName)
{
	cLockGuard<cMutex> lock (xmlDocMutex);

	XMLElement* xmlElement = getOrCreateXmlElement (configFile, path);
	if (xmlElement == NULL) return;

	xmlElement->SetAttribute (valueName, value);

	if (configFile.SaveFile (configPath.c_str()) != XML_NO_ERROR)
	{
		Log.write ("Could not write new config to " + configPath, cLog::eLOG_TYPE_ERROR);
	}
}


//------------------------------------------------------------------------------
void cSettings::saveResolution()
{
	saveSetting ("Options~Start~Resolution", (iToStr (Video.getResolutionX()) + "." + iToStr (Video.getResolutionY())).c_str());
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
void cSettings::saveDisplayIndex ()
{
    saveSetting ("Options~Start~Display", Video.getDisplayIndex());
}



//------------------------------------------------------------------------------
bool cSettings::isDebug() const
{
	return debug;
}

//------------------------------------------------------------------------------
void cSettings::setDebug (bool debug, bool save)
{
	this->debug = debug;
	if (save) saveSetting ("Options~Game~EnableDebug", debug);
}

//------------------------------------------------------------------------------
bool cSettings::shouldAutosave() const
{
	return autosave;
}

//------------------------------------------------------------------------------
void cSettings::setAutosave (bool autosave, bool save)
{
	this->autosave = autosave;
	if (save) saveSetting ("Options~Game~EnableAutosave", autosave);
}

//------------------------------------------------------------------------------
bool cSettings::isAnimations() const
{
	return animations;
}

//------------------------------------------------------------------------------
void cSettings::setAnimations (bool animations, bool save)
{
	std::swap(this->animations, animations);
	if (save) saveSetting ("Options~Game~EnableAnimations", this->animations);
	if (this->animations != animations) animationsChanged ();
}

//------------------------------------------------------------------------------
bool cSettings::isShadows() const
{
	return shadows;
}

//------------------------------------------------------------------------------
void cSettings::setShadows (bool shadows, bool save)
{
	this->shadows = shadows;
	if (save) saveSetting ("Options~Game~EnableShadows", shadows);
}

//------------------------------------------------------------------------------
bool cSettings::isAlphaEffects() const
{
	return alphaEffects;
}

//------------------------------------------------------------------------------
void cSettings::setAlphaEffects (bool alphaEffects, bool save)
{
	this->alphaEffects = alphaEffects;
	if (save) saveSetting ("Options~Game~EnableAlphaEffects", alphaEffects);
}

//------------------------------------------------------------------------------
bool cSettings::shouldShowDescription() const
{
	return showDescription;
}

//------------------------------------------------------------------------------
void cSettings::setShowDescription (bool showDescription, bool save)
{
	this->showDescription = showDescription;
	if (save) saveSetting ("Options~Game~EnableDescribtions", showDescription);
}

//------------------------------------------------------------------------------
bool cSettings::isDamageEffects() const
{
	return damageEffects;
}

//------------------------------------------------------------------------------
void cSettings::setDamageEffects (bool damageEffects, bool save)
{
	this->damageEffects = damageEffects;
	if (save) saveSetting ("Options~Game~EnableDamageEffects", damageEffects);
}

//------------------------------------------------------------------------------
bool cSettings::isDamageEffectsVehicles() const
{
	return damageEffectsVehicles;
}

//------------------------------------------------------------------------------
void cSettings::setDamageEffectsVehicles (bool damageEffectsVehicles, bool save)
{
	this->damageEffectsVehicles = damageEffectsVehicles;
	if (save) saveSetting ("Options~Game~EnableDamageEffectsVehicles", damageEffectsVehicles);
}

//------------------------------------------------------------------------------
bool cSettings::isMakeTracks() const
{
	return makeTracks;
}

//------------------------------------------------------------------------------
void cSettings::setMakeTracks (bool makeTracks, bool save)
{
	this->makeTracks = makeTracks;
	if (save) saveSetting ("Options~Game~EnableMakeTracks", makeTracks);
}

//------------------------------------------------------------------------------
int cSettings::getScrollSpeed() const
{
	return scrollSpeed;
}

//------------------------------------------------------------------------------
void cSettings::setScrollSpeed (int scrollSpeed, bool save)
{
	this->scrollSpeed = scrollSpeed;
	if (save) saveSetting ("Options~Game~ScrollSpeed", scrollSpeed);
}

//------------------------------------------------------------------------------
const std::string& cSettings::getIP() const
{
	return ip;
}

//------------------------------------------------------------------------------
void cSettings::setIP (const char* ip, bool save)
{
	this->ip = ip;
	if (save) saveSetting ("Options~Game~Net~IP", ip);
}

//------------------------------------------------------------------------------
unsigned short cSettings::getPort() const
{
	return port;
}

//------------------------------------------------------------------------------
void cSettings::setPort (unsigned short port, bool save)
{
	this->port = port;
	if (save) saveSetting ("Options~Game~Net~Port", port);
}

//------------------------------------------------------------------------------
const std::string& cSettings::getPlayerName() const
{
	return playerName;
}

//------------------------------------------------------------------------------
void cSettings::setPlayerName (const char* playerName, bool save)
{
	this->playerName = playerName;
	if (save) saveSetting ("Options~Game~Net~PlayerName", playerName);
}

//------------------------------------------------------------------------------
const cRgbColor& cSettings::getPlayerColor() const
{
	return playerColor;
}

//------------------------------------------------------------------------------
void cSettings::setPlayerColor (const cRgbColor& color, bool save)
{
	this->playerColor = color;
	if (save)
	{
		saveSetting ("Options~Game~Net~PlayerColor", color.r, "red");
		saveSetting ("Options~Game~Net~PlayerColor", color.g, "green");
		saveSetting ("Options~Game~Net~PlayerColor", color.b, "blue");
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
const std::string& cSettings::getExePath() const
{
	return exePath;
}

//------------------------------------------------------------------------------
void cSettings::setExePath (const char* exePath)
{
	this->exePath = exePath;
}

//------------------------------------------------------------------------------
const std::string& cSettings::getLogPath() const
{
	return logPath;
}

//------------------------------------------------------------------------------
void cSettings::setLogPath (const char* logPath)
{
	this->logPath = logPath;
}

//------------------------------------------------------------------------------
const std::string& cSettings::getHomeDir() const
{
	return homeDir;
}

//------------------------------------------------------------------------------
void cSettings::setHomeDir (const char* homeDir)
{
	this->homeDir = homeDir;
}

//------------------------------------------------------------------------------
bool cSettings::isSoundEnabled () const
{
	return soundEnabled;
}

//------------------------------------------------------------------------------
void cSettings::setSoundEnabled (bool soundEnabled, bool save)
{
	this->soundEnabled = soundEnabled;
	if (save) saveSetting ("Options~Game~Sound~Enabled", soundEnabled);
}

//------------------------------------------------------------------------------
int cSettings::getMusicVol() const
{
	return musicVol;
}

//------------------------------------------------------------------------------
void cSettings::setMusicVol (int musicVol, bool save)
{
	this->musicVol = musicVol;
	if (save) saveSetting ("Options~Game~Sound~MusicVol", musicVol);
}

//------------------------------------------------------------------------------
int cSettings::getSoundVol() const
{
	return soundVol;
}

//------------------------------------------------------------------------------
void cSettings::setSoundVol (int soundVol, bool save)
{
	this->soundVol = soundVol;
	if (save) saveSetting ("Options~Game~Sound~SoundVol", soundVol);
}

//------------------------------------------------------------------------------
int cSettings::getVoiceVol() const
{
	return voiceVol;
}

//------------------------------------------------------------------------------
void cSettings::setVoiceVol (int voiceVol, bool save)
{
	this->voiceVol = voiceVol;
	if (save) saveSetting ("Options~Game~Sound~VoiceVol", voiceVol);
}

//------------------------------------------------------------------------------
int cSettings::getChunkSize() const
{
	return chunkSize;
}

//------------------------------------------------------------------------------
void cSettings::setChunkSize (int chunkSize, bool save)
{
	this->chunkSize = chunkSize;
	if (save) saveSetting ("Options~Game~Sound~ChunkSize", chunkSize);
}

//------------------------------------------------------------------------------
int cSettings::getFrequency() const
{
	return frequency;
}

//------------------------------------------------------------------------------
void cSettings::setFrequence (int frequency, bool save)
{
	this->frequency = frequency;
	if (save) saveSetting ("Options~Game~Sound~Frequency", frequency);
}

//------------------------------------------------------------------------------
bool cSettings::isMusicMute() const
{
	return musicMute;
}

//------------------------------------------------------------------------------
void cSettings::setMusicMute (bool musicMute, bool save)
{
	this->musicMute = musicMute;
	if (save) saveSetting ("Options~Game~Sound~MusicMute", musicMute);
}

//------------------------------------------------------------------------------
bool cSettings::isSoundMute() const
{
	return soundMute;
}

//------------------------------------------------------------------------------
void cSettings::setSoundMute (bool soundMute, bool save)
{
	this->soundMute = soundMute;
	if (save) saveSetting ("Options~Game~Sound~SoundMute", soundMute);
}

//------------------------------------------------------------------------------
bool cSettings::isVoiceMute() const
{
	return voiceMute;
}

//------------------------------------------------------------------------------
void cSettings::setVoiceMute (bool voiceMute, bool save)
{
	this->voiceMute = voiceMute;
	if (save) saveSetting ("Options~Game~Sound~VoiceMute", voiceMute);
}

//------------------------------------------------------------------------------
bool cSettings::is3DSound () const
{
	return sound3d;
}

//------------------------------------------------------------------------------
void cSettings::set3DSound (bool sound3d, bool save)
{
	this->sound3d = sound3d;
	if (save) saveSetting ("Options~Game~Sound~Sound3D", sound3d);
}

//------------------------------------------------------------------------------
bool cSettings::shouldShowIntro() const
{
	return showIntro;
}

//------------------------------------------------------------------------------
void cSettings::setShowIntro (bool showIntro, bool save)
{
	this->showIntro = showIntro;
	if (save) saveSetting ("Options~Start~Intro", showIntro);
}

//------------------------------------------------------------------------------
bool cSettings::shouldUseFastMode() const
{
	return fastMode;
}

//------------------------------------------------------------------------------
void cSettings::setFastMode (bool fastMode, bool save)
{
	this->fastMode = fastMode;
	if (save) saveSetting ("Options~Start~Fastmode", fastMode);
}

//------------------------------------------------------------------------------
bool cSettings::shouldDoPrescale() const
{
	return preScale;
}

//------------------------------------------------------------------------------
void cSettings::setDoPrescale (bool preScale, bool save)
{
	this->preScale = preScale;
	if (save) saveSetting ("Options~Start~PreScale", preScale);
}

//------------------------------------------------------------------------------
const std::string& cSettings::getLanguage() const
{
	return language;
}

//------------------------------------------------------------------------------
void cSettings::setLanguage (const char* language, bool save)
{
	this->language = language;
	if (save) saveSetting ("Options~Start~Language", language);
}
//------------------------------------------------------------------------------
const std::string& cSettings::getVoiceLanguage() const
{
	return voiceLanguage;
}

//------------------------------------------------------------------------------
void cSettings::setVoiceLanguage (const char* language, bool save)
{
	this->voiceLanguage = language;
	if (save) saveSetting ("Options~Start~VoiceLanguage", language);
}
//------------------------------------------------------------------------------
unsigned int cSettings::getCacheSize() const
{
	return cacheSize;
}

//------------------------------------------------------------------------------
void cSettings::setCacheSize (unsigned int cacheSize, bool save)
{
	this->cacheSize = cacheSize;
	if (save) saveSetting ("Options~Start~CacheSize", cacheSize);
}

//------------------------------------------------------------------------------
const std::string& cSettings::getFontPath() const
{
	return fontPath;
}

//------------------------------------------------------------------------------
void cSettings::setFontPath (const char* fontPath, bool save)
{
	this->fontPath = fontPath;
	if (save) saveSetting ("Options~Game~Paths~Fonts", fontPath);
}

//------------------------------------------------------------------------------
const std::string& cSettings::getFxPath() const
{
	return fxPath;
}

//------------------------------------------------------------------------------
void cSettings::setFxPath (const char* fxPath, bool save)
{
	this->fxPath = fxPath;
	if (save) saveSetting ("Options~Game~Paths~FX", fxPath);
}

//------------------------------------------------------------------------------
const std::string& cSettings::getGfxPath() const
{
	return gfxPath;
}

//------------------------------------------------------------------------------
void cSettings::setGfxPath (const char* gfxPath, bool save)
{
	this->gfxPath = gfxPath;
	if (save) saveSetting ("Options~Game~Paths~GFX", gfxPath);
}

//------------------------------------------------------------------------------
const std::string& cSettings::getLangPath() const
{
	return langPath;
}

//------------------------------------------------------------------------------
void cSettings::setLangPath (const char* langPath, bool save)
{
	this->langPath = langPath;
	if (save) saveSetting ("Options~Game~Paths~Languages", langPath);
}

//------------------------------------------------------------------------------
const std::string& cSettings::getMapsPath() const
{
	return mapsPath;
}

//------------------------------------------------------------------------------
void cSettings::setMapsPath (const char* mapsPath, bool save)
{
	this->mapsPath = mapsPath;
	if (save) saveSetting ("Options~Game~Paths~Maps", mapsPath);
}

//------------------------------------------------------------------------------
const std::string& cSettings::getSavesPath() const
{
	return savesPath;
}

//------------------------------------------------------------------------------
void cSettings::setSavesPath (const char* savesPath, bool save)
{
	this->savesPath = savesPath;
	if (save) saveSetting ("Options~Game~Paths~Saves", savesPath);
}

//------------------------------------------------------------------------------
const std::string& cSettings::getSoundsPath() const
{
	return soundsPath;
}

//------------------------------------------------------------------------------
void cSettings::setSoundsPath (const char* soundsPath, bool save)
{
	this->soundsPath = soundsPath;
	if (save) saveSetting ("Options~Game~Paths~Sounds", soundsPath);
}

//------------------------------------------------------------------------------
const std::string& cSettings::getVoicesPath() const
{
	return voicesPath;
}

//------------------------------------------------------------------------------
void cSettings::setVoicesPath (const char* voicesPath, bool save)
{
	this->voicesPath = voicesPath;
	if (save) saveSetting ("Options~Game~Paths~Voices", voicesPath);
}

//------------------------------------------------------------------------------
const std::string& cSettings::getMusicPath() const
{
	return musicPath;
}

//------------------------------------------------------------------------------
void cSettings::setMusicPath (const char* musicPath, bool save)
{
	this->musicPath = musicPath;
	if (save) saveSetting ("Options~Game~Paths~Music", musicPath);
}

//------------------------------------------------------------------------------
const std::string& cSettings::getVehiclesPath() const
{
	return vehiclesPath;
}

//------------------------------------------------------------------------------
void cSettings::setVehiclesPath (const char* vehiclesPath, bool save)
{
	this->vehiclesPath = vehiclesPath;
	if (save) saveSetting ("Options~Game~Paths~Vehicles", vehiclesPath);
}

//------------------------------------------------------------------------------
const std::string& cSettings::getBuildingsPath() const
{
	return buildingsPath;
}

//------------------------------------------------------------------------------
void cSettings::setBuildingsPath (const char* buildingsPath, bool save)
{
	this->buildingsPath = buildingsPath;
	if (save) saveSetting ("Options~Game~Paths~Buildings", buildingsPath);
}

//------------------------------------------------------------------------------
const std::string& cSettings::getMvePath() const
{
	return mvePath;
}

//------------------------------------------------------------------------------
void cSettings::setMvePath (const char* mvePath, bool save)
{
	this->mvePath = mvePath;
	if (save) saveSetting ("Options~Game~Paths~MVEs", mvePath);
}
