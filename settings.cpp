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
#	include <shlobj.h>
#	include <direct.h>
#else
#	include <sys/stat.h>
#	include <unistd.h>
#endif

#include <string>
#include <iostream>
#include <locale>

#include "settings.h"
#include "main.h"
#include "log.h"
#include "extendedtinyxml.h"
#include "files.h"
#include "video.h"

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
bool cSettings::isInitialized()
{
	return initialized;
}

//------------------------------------------------------------------------------
void cSettings::setPaths()
{
	//init absolutly needed paths
	logPath = MAX_LOG;
	netLogPath = MAX_NET_LOG;
	exePath = ""; //FIXME: I don't know how this is handled on win/mac/amiga -- beko
	homeDir="";

	#if MAC
	// do some rudimentary work with the user's homefolder. Needs to be extended in future...
	char * cHome = getenv("HOME"); //get $HOME on mac
	if(cHome != NULL)
		homeDir = cHome;
	if (!homeDir.empty())
	{
		homeDir += PATH_DELIMITER;
		homeDir += ".maxr";
		homeDir += PATH_DELIMITER;

		// check whether home dir is set up and readable
		if (!FileExists(homeDir.c_str())) // under mac everything is a file
		{
			if (mkdir (homeDir.c_str (), 0755) == 0)
				std::cout << "\n(II): Created new config directory " << homeDir;
			else
			{
				std::cout << "\n(EE): Can't create config directory " << homeDir;
				homeDir = ""; //reset $HOME since we can't create our config directory
			}
		}
	}
	//this is also a good place to find out where the executable is located
	configPath = MAX_XML; //assume config in current working directory
	#elif WIN32
		//this is where windowsuser should set their %HOME%
		//this is also a good place to find out where the executable is located
		configPath = MAX_XML; //assume config in current working directory

		TCHAR szPath[MAX_PATH];
		SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, 0, szPath );

#ifdef UNICODE
		std::wstring home = szPath;
#else
		std::string home = szPath;
#endif

		homeDir = std::string(home.begin(), home.end());

		std::cout << "\n(II): Read home directory " << homeDir;

		bool newDirCreated = false;

		if (!homeDir.empty())
		{
			homeDir += PATH_DELIMITER;
			homeDir += "maxr";

			if ( !DirExists(homeDir) )
			{
				if(mkdir(homeDir.c_str()) == 0)
				{
					std::cout << "\n(II): Created new config directory " << homeDir;
					newDirCreated = true;
				}
				else
				{
					std::cout << "\n(EE): Can't create config directory " << homeDir;
					homeDir = "";
				}
			}
			else std::cout << "\n(II): Config is read from " << homeDir;

			if ( !homeDir.empty() ) homeDir += PATH_DELIMITER;

			configPath = homeDir;
			configPath += MAX_XML;
		}

		//set new place for logs
		logPath = homeDir + logPath;
		netLogPath = homeDir + netLogPath;
		std::cout << "\n(II): Starting logging to: " << logPath;

		if(newDirCreated)
		{
			//since the config dir didn't exist we can assume config is missing as well so we run ReadMaxXML taking care of a missing config _and_ providing us with needed PATHS and set up save directory as well -- beko
			//if( ReadMaxXml() != 0 ) Log.write("An error occured. Please check your installation!", cLog::eLOG_TYPE_ERROR);
			
			if( mkdir(getSavesPath().c_str()) == 0 ) Log.write("Created new save directory: "+getSavesPath(), cLog::eLOG_TYPE_INFO);
			else Log.write("Can't create save directory: "+getSavesPath(), cLog::eLOG_TYPE_ERROR);
		}
	#elif __amigaos4__
		//this is where amigausers should set their %HOME%
		//this is also a good place to find out where the executable is located
		configPath = MAX_XML; //assume config in current working directory
	#else
	//NOTE: I do not use cLog here on purpose. Logs on linux go somewhere to $HOME/.maxr/ - as long as we can't access that we have to output everything to the terminal because game's root dir is usually write protected! -- beko
	bool bCreateConfigDir = false;

	char * cHome = getenv("HOME"); //get $HOME on linux
	if(cHome != NULL)
	{
		homeDir = cHome; //get $HOME on linux
	}

	if (!homeDir.empty())
	{
		homeDir += PATH_DELIMITER;
		homeDir += ".maxr";
		homeDir += PATH_DELIMITER;
		configPath = homeDir;
		configPath += MAX_XML; //set config to $HOME/.maxr/max.xml

		//check whether home dir is set up and readable
		if(!FileExists(homeDir.c_str())) //under linux everything is a file -- beko
		{
			if(mkdir(homeDir.c_str(), 0755) == 0)
			{
				bCreateConfigDir = true;
				std::cout << "\n(II): Created new config directory " << homeDir;
			}
			else
			{
				cout << "\n(EE): Can't create config directory " << homeDir;
				std::homeDir = ""; //reset $HOME since we can't create our config directory
			}
		}
		else
		{
			std::cout << "\n(II): Config is read from " << homeDir; //config dir can be read - we're fine
		}
	}
	else
	{
		std::cout << "\n(WW): $HOME is not set!";
		homeDir="";
		configPath = MAX_XML; //assume config in current working directory
	}

	//set new place for logs
	logPath = homeDir + logPath;
	netLogPath = homeDir + netLogPath;
	std::cout << "\n(II): Starting logging to: " << logPath;

	//determine full path to application
	//this needs /proc support that should be avaible on most linux installations
	if(FileExists("/proc/self/exe"))
	{
		int iSize;
		char cPathToExe[255];
		iSize = readlink("/proc/self/exe", cPathToExe, sizeof(cPathToExe));
		if (iSize < 0)
		{
			Log.write("Can't resolve full path to program. Doesn't this system feature /proc/self/exe?", cLog::eLOG_TYPE_WARNING);
		}
		else if (iSize >= 255)
		{
			Log.write("Can't resolve full path to program since my array is to small and my programmer is to lame to write a buffer for me!", cLog::eLOG_TYPE_WARNING);
		}
		else
		{
			int iPos = 0;
			for(int i = 0; i<255;i++)
			{
				if(cPathToExe[i] == '/') //snip garbage after last PATH_DELIMITER + executable itself (is reported on some linux systems as well using /proc/self/exe
					iPos = i;
				if(cPathToExe[i] == '\0') //skip garbage that might lunger on heap after 0 termination
					i = 255;
			}


			exePath = cPathToExe;
			exePath = exePath.substr(0, iPos);
			exePath += PATH_DELIMITER;

			if(FileExists( (exePath+"maxr").c_str() )) //check for binary itself in bin folder
			{
				Log.write("Path to binary is: "+exePath, cLog::eLOG_TYPE_INFO);
			}
			else
			{	//perhaps we got ourself a trailing maxr in the path like /proc
				// seems to do it sometimes. remove it and try again!
				if(cPathToExe[iPos-1] == 'r' && cPathToExe[iPos-2] == 'x' && cPathToExe[iPos-3] == 'a' && cPathToExe[iPos-4] == 'm' )
				{
					exePath = exePath.substr(0, iPos-5);
					if(FileExists( (exePath+"maxr").c_str() ))
					{
						Log.write("Path to binary is: "+exePath, cLog::eLOG_TYPE_INFO);
					}
				}
			}

		}
	}
	else
	{
		Log.write("Can't resolve full path to program. Doesn't this system feature /proc/self/exe?", cLog::eLOG_TYPE_WARNING);
		exePath=""; //reset exePath
	}

	if(bCreateConfigDir)
	{
		//since the config dir didn't exist we can assume config is missing as well so we run ReadMaxXML taking care of a missing config _and_ providing us with needed PATHS and set up save directory as well -- beko
		if(ReadMaxXml()==0)
		{

		}
		else
		{
			Log.write("An error occured. Please check your installation!", cLog::eLOG_TYPE_ERROR);
		}

		if(mkdir(getSavesPath().c_str(), 0755) == 0)
		{
			Log.write("Created new save directory: "+getSavesPath(), cLog::eLOG_TYPE_INFO);
		}
		else
		{
			Log.write("Can't create save directory: "+getSavesPath(), cLog::eLOG_TYPE_ERROR);
		}
	}

	std::cout << "\n";
	#endif
}


//------------------------------------------------------------------------------
std::string cSettings::searchDataDir(std::string sDataDirFromConf)
{
	std::string sPathToGameData = "";
	#if MAC
		sPathToGameData =  exePath; //assuming data is in same folder like binary (or current working directory)
	#elif WIN32
		sPathToGameData = exePath; //assuming data is in same folder like binary (or current working directory)
	#elif __amigaos4__
		sPathToGameData = exePath; //assuming data is in same folder like binary (or current working directory)
	#else
	//BEGIN crude path validation to find gamedata
	Log.write ( "Probing for data paths using default values:", cLog::eLOG_TYPE_INFO );

	#define PATHCOUNT 11
	std::string sPathArray[PATHCOUNT] = {
		BUILD_DATADIR, //most important position holds value of configure --prefix to gamedata in %prefix%/$(datadir)/maxr or default path if autoversion.h wasn't used
		"/usr/local/share/maxr",
		"/usr/games/maxr",
		"/usr/local/games/maxr",
		"/usr/maxr",
		"/usr/local/maxr",
		"/opt/maxr",
		"/usr/share/games/maxr",
		"/usr/local/share/games/maxr",
		exePath, //check for gamedata in bin folder too
		"." //last resort: local dir
	};

	/*
	* Logic is:
	* BUILD_DATADIR is default search path
	* sDataDirFromConf overrides BUILD_DATADIR
	* "$MAXRDATA overrides both
	* BUILD_DATADIR is checked if sDataDirFromConf or $MAXRDATA fail the probe
	*/
	if(!sDataDirFromConf.empty())
	{
		sPathArray[0] = sDataDirFromConf; //override default path with path from config
		sPathArray[1] = BUILD_DATADIR; //and save old value one later in case sDataDirFromConf is invalid
	}

	//BEGIN SET MAXRDATA
	char * cDataDir;
	cDataDir = getenv("MAXRDATA");
	if(cDataDir == NULL)
	{
		Log.write("$MAXRDATA is not set", cLog::eLOG_TYPE_INFO);
	}
	else
	{
		sPathArray[0] = cDataDir;
		sPathArray[1] = BUILD_DATADIR;
		Log.write("$MAXRDATA is set and overrides default data search path", cLog::eLOG_TYPE_WARNING);
	}
	//END SET MAXRDATA

	for(int i=0; i<PATHCOUNT; i++)
	{
		std::string sInitFile = sPathArray[i];
		sInitFile += PATH_DELIMITER;
		sInitFile += "init.pcx";
		if(FileExists( sInitFile.c_str() ))
		{
			sPathToGameData = sPathArray[i];
			sPathToGameData += PATH_DELIMITER;
			break;
		}
	}

	if(sPathToGameData.empty()) //still empty? cry for mama - we couldn't locate any typical data folder
	{
		Log.write("No success probing for data folder!", cLog::eLOG_TYPE_ERROR);
	}
	else
	{
		Log.write("Found gamedata in: "+sPathToGameData, cLog::eLOG_TYPE_INFO);
	}
	//END crude path validation to find gamedata
	#endif
	return sPathToGameData;
}

//------------------------------------------------------------------------------
bool cSettings::createConfigFile()
{
	TiXmlDocument configFile;
	TiXmlElement *rootnode = new TiXmlElement ( "Options" );
	configFile.LinkEndChild ( rootnode );

	if(!configFile.SaveFile(configPath.c_str())) //create new empty config
	{
		Log.write("Could not write new config to " + configPath, cLog::eLOG_TYPE_ERROR);
		return false; // Generate fails
	}
	return true;
}

//------------------------------------------------------------------------------
void cSettings::initialize()
{
	initializing = true;
	TiXmlDocument configFile;

	setPaths();

	if(configPath.empty()) //we need at least configPath here
	{
		Log.write ( "configPath not set!", cLog::eLOG_TYPE_WARNING );
		configPath = MAX_XML;
	}
	if (!configFile.LoadFile(configPath.c_str()))
	{
		Log.write ( "Can't read max.xml\n", LOG_TYPE_WARNING );
		if ( !createConfigFile() ) return;
	}
	
	ExTiXmlNode *xmlNode = NULL;
	std::string temp;

	//START
	// =============================================================================
	xmlNode = ExTiXmlNode::XmlGetFirstNode(configFile,"Options","Start","Resolution", NULL);
	if(!xmlNode || !xmlNode->XmlReadNodeData(temp, ExTiXmlNode::eXML_ATTRIBUTE, "Text"))
	{
		Log.write ( "Can't load resolution from config file: using default value", LOG_TYPE_WARNING );
		Video.setResolution(Video.getMinW(), Video.getMinH());
		saveResolution();
	}
	else
	{
		int wTmp = atoi(temp.substr(0,temp.find(".",0)).c_str());
		int hTmp = atoi(temp.substr(temp.find(".",0)+1,temp.length()).c_str());
		Video.setResolution(wTmp, hTmp);
	}
	
	// =============================================================================
	xmlNode = ExTiXmlNode::XmlGetFirstNode(configFile,"Options","Start","ColourDepth", NULL);
	if(!xmlNode || !xmlNode->XmlReadNodeData(temp, ExTiXmlNode::eXML_ATTRIBUTE, "Num"))
	{
		Log.write ( "Can't load color depth from config file: using default value", LOG_TYPE_WARNING );
		Video.setColDepth(atoi(temp.c_str()));
		saveColorDepth();
	}
	else Video.setColDepth(32);
	
	// =============================================================================
	xmlNode = ExTiXmlNode::XmlGetFirstNode(configFile,"Options","Start","Intro", NULL);
	if(!xmlNode || !xmlNode->XmlReadNodeData(temp, ExTiXmlNode::eXML_ATTRIBUTE, "YN"))
	{
		Log.write ( "Can't load intro from config file: using default value", LOG_TYPE_WARNING );
		setShowIntro(true);
	}
	else showIntro = xmlNode->XmlDataToBool(temp);
	
	// =============================================================================
	xmlNode = ExTiXmlNode::XmlGetFirstNode(configFile,"Options","Start","Windowmode", NULL);
	if(!xmlNode || !xmlNode->XmlReadNodeData(temp, ExTiXmlNode::eXML_ATTRIBUTE, "YN"))
	{
		Log.write ( "Can't load window mode from config file: using default value", LOG_TYPE_WARNING );
		Video.setWindowMode(true);
		saveWindowMode();
	}
	else Video.setWindowMode(xmlNode->XmlDataToBool(temp));
	
	// =============================================================================
	xmlNode = ExTiXmlNode::XmlGetFirstNode(configFile,"Options","Start","Fastmode", NULL);
	if(!xmlNode || !xmlNode->XmlReadNodeData(temp, ExTiXmlNode::eXML_ATTRIBUTE, "YN"))
	{
		Log.write ( "Can't load fast mode from config file: using default value", LOG_TYPE_WARNING );
		setFastMode(false);
	}
	else fastMode = xmlNode->XmlDataToBool(temp);
	
	// =============================================================================
	xmlNode = ExTiXmlNode::XmlGetFirstNode(configFile,"Options","Start","PreScale", NULL);
	if(!xmlNode || !xmlNode->XmlReadNodeData(temp, ExTiXmlNode::eXML_ATTRIBUTE, "YN"))
	{
		Log.write ( "Can't load pre scale from config file: using default value", LOG_TYPE_WARNING );
		setDoPrescale(false);
	}
	else preScale = xmlNode->XmlDataToBool(temp);
	
	// =============================================================================
	xmlNode = ExTiXmlNode::XmlGetFirstNode(configFile,"Options","Start","CacheSize", NULL);
	if(!xmlNode || !xmlNode->XmlReadNodeData(temp, ExTiXmlNode::eXML_ATTRIBUTE, "Num"))
	{
		Log.write ( "Can't load cache size from config file: using default value", LOG_TYPE_WARNING );
		setCacheSize(400);
	}
	else cacheSize = atoi(temp.c_str());
	
	// =============================================================================
	xmlNode = ExTiXmlNode::XmlGetFirstNode(configFile,"Options","Start","Language", NULL);
	if(!xmlNode || !xmlNode->XmlReadNodeData(temp, ExTiXmlNode::eXML_ATTRIBUTE, "Text"))
	{
		Log.write ( "Can't load language from config file: using default value", LOG_TYPE_WARNING );
		setLanguage("ENG");
	}
	else
	{
		struct upper
		{
			locale loc;
			int operator()(int c) { return std::toupper((char)c, loc); }
		};
		std::transform(temp.begin(), temp.end(), temp.begin(), upper());
		language = temp.c_str();
	}

	//GAME
	// =============================================================================
	xmlNode = ExTiXmlNode::XmlGetFirstNode(configFile,"Options","Game","EnableAutosave", NULL);
	if(!xmlNode || !xmlNode->XmlReadNodeData(temp, ExTiXmlNode::eXML_ATTRIBUTE, "YN"))
	{
		Log.write ( "Can't load autosave from config file: using default value", LOG_TYPE_WARNING );
		setAutosave(true);
	}
	else autosave = xmlNode->XmlDataToBool(temp);

	// =============================================================================
	xmlNode = ExTiXmlNode::XmlGetFirstNode(configFile,"Options","Game","EnableDebug", NULL);
	if(!xmlNode || !xmlNode->XmlReadNodeData(temp, ExTiXmlNode::eXML_ATTRIBUTE, "YN"))
	{
		Log.write ( "Can't load debug from config file: using default value", LOG_TYPE_WARNING );
		setDebug(true);
	}
	else
	{
		debug = xmlNode->XmlDataToBool(temp);
		if(!debug)Log.write("Debugmode disabled - for verbose output please enable Debug in max.xml", cLog::eLOG_TYPE_WARNING);
		else Log.write("Debugmode enabled", cLog::eLOG_TYPE_INFO);
	}

	// =============================================================================
	xmlNode = ExTiXmlNode::XmlGetFirstNode(configFile,"Options","Game","EnableAnimations", NULL);
	if(!xmlNode || !xmlNode->XmlReadNodeData(temp, ExTiXmlNode::eXML_ATTRIBUTE, "YN"))
	{
		Log.write ( "Can't load animations from config file: using default value", LOG_TYPE_WARNING );
		setAnimations(true);
	}
	else animations = xmlNode->XmlDataToBool(temp);

	// =============================================================================
	xmlNode = ExTiXmlNode::XmlGetFirstNode(configFile,"Options","Game","EnableShadows", NULL);
	if(!xmlNode || !xmlNode->XmlReadNodeData(temp, ExTiXmlNode::eXML_ATTRIBUTE, "YN"))
	{
		Log.write ( "Can't load shadows from config file: using default value", LOG_TYPE_WARNING );
		setShadows(true);
	}
	else shadows = xmlNode->XmlDataToBool(temp);

	// =============================================================================
	xmlNode = ExTiXmlNode::XmlGetFirstNode(configFile,"Options","Game","EnableAlphaEffects", NULL);
	if(!xmlNode || !xmlNode->XmlReadNodeData(temp, ExTiXmlNode::eXML_ATTRIBUTE, "YN"))
	{
		Log.write ( "Can't load alpha effects from config file: using default value", LOG_TYPE_WARNING );
		setAlphaEffects(true);
	}
	else alphaEffects = xmlNode->XmlDataToBool(temp);

	// =============================================================================
	xmlNode = ExTiXmlNode::XmlGetFirstNode(configFile,"Options","Game","EnableDescribtions", NULL);
	if(!xmlNode || !xmlNode->XmlReadNodeData(temp, ExTiXmlNode::eXML_ATTRIBUTE, "YN"))
	{
		Log.write ( "Can't load descriptions from config file: using default value", LOG_TYPE_WARNING );
		setShowDescription(true);
	}
	else showDescription = xmlNode->XmlDataToBool(temp);

	// =============================================================================
	xmlNode = ExTiXmlNode::XmlGetFirstNode(configFile,"Options","Game","EnableDamageEffects", NULL);
	if(!xmlNode || !xmlNode->XmlReadNodeData(temp, ExTiXmlNode::eXML_ATTRIBUTE, "YN"))
	{
		Log.write ( "Can't load damage effects from config file: using default value", LOG_TYPE_WARNING );
		setDamageEffects(true);
	}
	else damageEffects = xmlNode->XmlDataToBool(temp);

	// =============================================================================
	xmlNode = ExTiXmlNode::XmlGetFirstNode(configFile,"Options","Game","EnableDamageEffectsVehicles", NULL);
	if(!xmlNode || !xmlNode->XmlReadNodeData(temp, ExTiXmlNode::eXML_ATTRIBUTE, "YN"))
	{
		Log.write ( "Can't load damaga effects vehicles from config file: using default value", LOG_TYPE_WARNING );
		setDamageEffectsVehicles(true);
	}
	else damageEffectsVehicles = xmlNode->XmlDataToBool(temp);
	
	// =============================================================================
	xmlNode = ExTiXmlNode::XmlGetFirstNode(configFile,"Options","Game","EnableMakeTracks", NULL);
	if(!xmlNode || !xmlNode->XmlReadNodeData(temp, ExTiXmlNode::eXML_ATTRIBUTE, "YN"))
	{
		Log.write ( "Can't load make tracks from config file: using default value", LOG_TYPE_WARNING );
		setMakeTracks(true);
	}
	else makeTracks = xmlNode->XmlDataToBool(temp);
	
	// =============================================================================
	xmlNode = ExTiXmlNode::XmlGetFirstNode(configFile,"Options","Game","ScrollSpeed", NULL);
	if(!xmlNode || !xmlNode->XmlReadNodeData(temp, ExTiXmlNode::eXML_ATTRIBUTE, "Num"))
	{
		Log.write ( "Can't load scroll speed from config file: using default value", LOG_TYPE_WARNING );
		setScrollSpeed(32);
	}
	else scrollSpeed = atoi(temp.c_str());

	//GAME-SOUND
	// =============================================================================
	xmlNode = ExTiXmlNode::XmlGetFirstNode(configFile,"Options","Game","Sound", "Enabled", NULL);
	if(!xmlNode || !xmlNode->XmlReadNodeData(temp, ExTiXmlNode::eXML_ATTRIBUTE, "YN"))
	{
		Log.write ( "Can't load sound enabled from config file: using default value", LOG_TYPE_WARNING );
		setSoundEnabled(true);
	}
	else soundEnabled = xmlNode->XmlDataToBool(temp);
	
	// =============================================================================
	xmlNode = ExTiXmlNode::XmlGetFirstNode(configFile,"Options","Game","Sound", "MusicMute", NULL);
	if(!xmlNode || !xmlNode->XmlReadNodeData(temp, ExTiXmlNode::eXML_ATTRIBUTE, "YN"))
	{
		Log.write ( "Can't load music mute from config file: using default value", LOG_TYPE_WARNING );
		setMusicMute(false);
	}
	else musicMute = xmlNode->XmlDataToBool(temp);
	
	// =============================================================================
	xmlNode = ExTiXmlNode::XmlGetFirstNode(configFile,"Options","Game","Sound", "SoundMute", NULL);
	if(!xmlNode || !xmlNode->XmlReadNodeData(temp, ExTiXmlNode::eXML_ATTRIBUTE, "YN"))
	{
		Log.write ( "Can't load sound mute from config file: using default value", LOG_TYPE_WARNING );
		setSoundMute(false);
	}
	else soundMute = xmlNode->XmlDataToBool(temp);
	
	// =============================================================================
	xmlNode = ExTiXmlNode::XmlGetFirstNode(configFile,"Options","Game","Sound", "VoiceMute", NULL);
	if(!xmlNode || !xmlNode->XmlReadNodeData(temp, ExTiXmlNode::eXML_ATTRIBUTE, "YN"))
	{
		Log.write ( "Can't load voice mute from config file: using default value", LOG_TYPE_WARNING );
		setVoiceMute(false);
	}
	else voiceMute = xmlNode->XmlDataToBool(temp);
	
	// =============================================================================
	xmlNode = ExTiXmlNode::XmlGetFirstNode(configFile,"Options","Game","Sound", "MusicVol", NULL);
	if(!xmlNode || !xmlNode->XmlReadNodeData(temp, ExTiXmlNode::eXML_ATTRIBUTE, "Num"))
	{
		Log.write ( "Can't load music volume from config file: using default value", LOG_TYPE_WARNING );
		setMusicVol(128);
	}
	else musicVol = atoi(temp.c_str());
	
	// =============================================================================
	xmlNode = ExTiXmlNode::XmlGetFirstNode(configFile,"Options","Game","Sound", "SoundVol", NULL);
	if(!xmlNode || !xmlNode->XmlReadNodeData(temp, ExTiXmlNode::eXML_ATTRIBUTE, "Num"))
	{
		Log.write ( "Can't load music volume from config file: using default value", LOG_TYPE_WARNING );
		setSoundVol(128);
	}
	else soundVol = atoi(temp.c_str());
	
	// =============================================================================
	xmlNode = ExTiXmlNode::XmlGetFirstNode(configFile,"Options","Game","Sound", "VoiceVol", NULL);
	if(!xmlNode || !xmlNode->XmlReadNodeData(temp, ExTiXmlNode::eXML_ATTRIBUTE, "Num"))
	{
		Log.write ( "Can't load music volume from config file: using default value", LOG_TYPE_WARNING );
		setVoiceVol(128);
	}
	else voiceVol = atoi(temp.c_str());
	
	// =============================================================================
	xmlNode = ExTiXmlNode::XmlGetFirstNode(configFile,"Options","Game","Sound", "ChunkSize", NULL);
	if(!xmlNode || !xmlNode->XmlReadNodeData(temp, ExTiXmlNode::eXML_ATTRIBUTE, "Num"))
	{
		Log.write ( "Can't load music volume from config file: using default value", LOG_TYPE_WARNING );
		setChunkSize(2048);
	}
	else chunkSize = atoi(temp.c_str());
	
	// =============================================================================
	xmlNode = ExTiXmlNode::XmlGetFirstNode(configFile,"Options","Game","Sound", "Frequency", NULL);
	if(!xmlNode || !xmlNode->XmlReadNodeData(temp, ExTiXmlNode::eXML_ATTRIBUTE, "Num"))
	{
		Log.write ( "Can't load music volume from config file: using default value", LOG_TYPE_WARNING );
		setFrequence(44100);
	}
	else frequency = atoi(temp.c_str());

	//PATHS
	// =============================================================================
	xmlNode = ExTiXmlNode::XmlGetFirstNode(configFile,"Options","Game","Paths","Gamedata", NULL);
	if(!xmlNode || !xmlNode->XmlReadNodeData(temp, ExTiXmlNode::eXML_ATTRIBUTE, "Text"))
	{
		Log.write ( "Can't load gamedata from config file: using default value", LOG_TYPE_WARNING );
		setDataDir(searchDataDir().c_str());
	}
	else setDataDir(searchDataDir(temp).c_str());

	// =============================================================================
	xmlNode = ExTiXmlNode::XmlGetFirstNode(configFile,"Options","Game","Paths","Languages", NULL);
	if(!xmlNode || !xmlNode->XmlReadNodeData(temp, ExTiXmlNode::eXML_ATTRIBUTE, "Text"))
	{
		Log.write ( "Can't load language path from config file: using default value", LOG_TYPE_WARNING );
		setLangPath((dataDir + "languages").c_str());
	}
	else setLangPath((dataDir + temp).c_str());

	// =============================================================================
	xmlNode = ExTiXmlNode::XmlGetFirstNode(configFile,"Options","Game","Paths","Fonts", NULL);
	if(!xmlNode || !xmlNode->XmlReadNodeData(temp, ExTiXmlNode::eXML_ATTRIBUTE, "Text"))
	{
		Log.write ( "Can't load fonts path from config file: using default value", LOG_TYPE_WARNING );
		setFontPath((dataDir + "fonts").c_str());
	}
	else setFontPath((dataDir + temp).c_str());

	// =============================================================================
	xmlNode = ExTiXmlNode::XmlGetFirstNode(configFile,"Options","Game","Paths","FX", NULL);
	if(!xmlNode || !xmlNode->XmlReadNodeData(temp, ExTiXmlNode::eXML_ATTRIBUTE, "Text"))
	{
		Log.write ( "Can't load fx path from config file: using default value", LOG_TYPE_WARNING );
		setFxPath((dataDir + "fx").c_str());
	}
	else setFxPath((dataDir + temp).c_str());

	// =============================================================================
	xmlNode = ExTiXmlNode::XmlGetFirstNode(configFile,"Options","Game","Paths","GFX", NULL);
	if(!xmlNode || !xmlNode->XmlReadNodeData(temp, ExTiXmlNode::eXML_ATTRIBUTE, "Text"))
	{
		Log.write ( "Can't load gfx path from config file: using default value", LOG_TYPE_WARNING );
		setGfxPath((dataDir + "gfx").c_str());
	}
	else setGfxPath((dataDir + temp).c_str());

	// =============================================================================
	xmlNode = ExTiXmlNode::XmlGetFirstNode(configFile,"Options","Game","Paths","Maps", NULL);
	if(!xmlNode || !xmlNode->XmlReadNodeData(temp, ExTiXmlNode::eXML_ATTRIBUTE, "Text"))
	{
		Log.write ( "Can't load maps path from config file: using default value", LOG_TYPE_WARNING );
		setMapsPath((dataDir + "maps").c_str());
	}
	else setMapsPath((dataDir + temp).c_str());

	// =============================================================================
	xmlNode = ExTiXmlNode::XmlGetFirstNode(configFile,"Options","Game","Paths","Sounds", NULL);
	if(!xmlNode || !xmlNode->XmlReadNodeData(temp, ExTiXmlNode::eXML_ATTRIBUTE, "Text"))
	{
		Log.write ( "Can't load sounds path from config file: using default value", LOG_TYPE_WARNING );
		setSoundsPath((dataDir + "sounds").c_str());
	}
	else setSoundsPath((dataDir + temp).c_str());

	// =============================================================================
	xmlNode = ExTiXmlNode::XmlGetFirstNode(configFile,"Options","Game","Paths","Voices", NULL);
	if(!xmlNode || !xmlNode->XmlReadNodeData(temp, ExTiXmlNode::eXML_ATTRIBUTE, "Text"))
	{
		Log.write ( "Can't load voices path from config file: using default value", LOG_TYPE_WARNING );
		setVoicesPath((dataDir + "voices").c_str());
	}
	else setVoicesPath((dataDir + temp).c_str());

	// =============================================================================
	xmlNode = ExTiXmlNode::XmlGetFirstNode(configFile,"Options","Game","Paths","Music", NULL);
	if(!xmlNode || !xmlNode->XmlReadNodeData(temp, ExTiXmlNode::eXML_ATTRIBUTE, "Text"))
	{
		Log.write ( "Can't load music path from config file: using default value", LOG_TYPE_WARNING );
		setMusicPath((dataDir + "music").c_str());
	}
	else setMusicPath((dataDir + temp).c_str());

	// =============================================================================
	xmlNode = ExTiXmlNode::XmlGetFirstNode(configFile,"Options","Game","Paths","Vehicles", NULL);
	if(!xmlNode || !xmlNode->XmlReadNodeData(temp, ExTiXmlNode::eXML_ATTRIBUTE, "Text"))
	{
		Log.write ( "Can't load vehicles path from config file: using default value", LOG_TYPE_WARNING );
		setVehiclesPath((dataDir + "vehicles").c_str());
	}
	else setVehiclesPath((dataDir + temp).c_str());

	// =============================================================================
	xmlNode = ExTiXmlNode::XmlGetFirstNode(configFile,"Options","Game","Paths","Buildings", NULL);
	if(!xmlNode || !xmlNode->XmlReadNodeData(temp, ExTiXmlNode::eXML_ATTRIBUTE, "Text"))
	{
		Log.write ( "Can't load buildings path from config file: using default value", LOG_TYPE_WARNING );
		setBuildingsPath((dataDir + "buildings").c_str());
	}
	else setBuildingsPath((dataDir + temp).c_str());

	// =============================================================================
	xmlNode = ExTiXmlNode::XmlGetFirstNode(configFile,"Options","Game","Paths","MVEs", NULL);
	if(!xmlNode || !xmlNode->XmlReadNodeData(temp, ExTiXmlNode::eXML_ATTRIBUTE, "Text"))
	{
		Log.write ( "Can't load language path from config file: using default value", LOG_TYPE_WARNING );
		setMvePath((dataDir + "mve").c_str());
	}
	else setMvePath((dataDir + temp).c_str());

	//GAME-NET
	// =============================================================================
	xmlNode = ExTiXmlNode::XmlGetFirstNode(configFile,"Options","Game","Net","IP", NULL);
	if(!xmlNode || !xmlNode->XmlReadNodeData(ip, ExTiXmlNode::eXML_ATTRIBUTE, "Text"))
	{
		Log.write ( "Can't load IP from config file: using default value", LOG_TYPE_WARNING );
		setIP("127.0.0.1");
	}

	// =============================================================================
	xmlNode = ExTiXmlNode::XmlGetFirstNode(configFile,"Options","Game","Net","Port", NULL);
	if(!xmlNode || !xmlNode->XmlReadNodeData(temp, ExTiXmlNode::eXML_ATTRIBUTE, "Num"))
	{
		Log.write ( "Can't load Port config file: using default value", LOG_TYPE_WARNING );
		setPort(DEFAULTPORT);
	}
	else port = atoi(temp.c_str());

	// =============================================================================
	xmlNode = ExTiXmlNode::XmlGetFirstNode(configFile,"Options","Game","Net","PlayerName", NULL);
	if(!xmlNode || !xmlNode->XmlReadNodeData(playerName, ExTiXmlNode::eXML_ATTRIBUTE, "Text"))
	{
		Log.write ( "Can't load player name from config file: using default value", LOG_TYPE_WARNING );

		char* user;

		#ifdef WIN32
			user = getenv("USERNAME");
		#elif __amigaos4__
			user = "AmigaOS4-User";
		#else
			user = getenv("USER"); //get $USER on linux
		#endif

		if(user != NULL) setPlayerName(user);
		else setPlayerName("Commander");
	}

	// =============================================================================
	if(!xmlNode || !xmlNode->XmlReadNodeData(temp, ExTiXmlNode::eXML_ATTRIBUTE, "Num"))
	{
		Log.write ( "Can't load player color from config file: using default value", LOG_TYPE_WARNING );
		setPlayerColor(0);
	}
	else playerColor = atoi(temp.c_str());

	initialized = true;
	initializing = false;
}

//------------------------------------------------------------------------------
TiXmlNode *cSettings::getXmlNode(std::string path, TiXmlDocument &configFile)
{
	if (!configFile.LoadFile(configPath.c_str()))
	{
		if ( !createConfigFile() ) return NULL;
	}

	std::vector<std::string> parts;
	size_t i = 0, j;
	do
	{
		j = path.find('~', i);
		if (j == std::string::npos) j = path.length();
		parts.push_back(path.substr(i, j-i));
		i = j+1;
	}
	while(j != path.length());

	TiXmlNode *xmlNode;
	TiXmlNode *lastNode = &configFile;
	for ( std::vector<std::string>::const_iterator i = parts.begin(); i != parts.end(); i++ )
	{
		xmlNode = lastNode->FirstChild( (*i).c_str() );
		if ( xmlNode == NULL ) xmlNode = lastNode->LinkEndChild(new TiXmlElement( (*i).c_str() ));
		lastNode = xmlNode;
	}

	return xmlNode;
}

//------------------------------------------------------------------------------
void cSettings::saveSetting(std::string path, const char* value)
{
	saveSetting(path, value, "Text");
}

//------------------------------------------------------------------------------
void cSettings::saveSetting(std::string path, int value)
{
	saveSetting(path, value, "Num");
}

//------------------------------------------------------------------------------
void cSettings::saveSetting(std::string path, unsigned int value)
{
	saveSetting(path, value, "Num");
}

//------------------------------------------------------------------------------
void cSettings::saveSetting(std::string path, bool value)
{
	if ( value ) saveSetting(path, "Yes", "YN");
	else saveSetting(path, "No", "YN");
}

//------------------------------------------------------------------------------
template<typename T>
void cSettings::saveSetting(std::string path, T value, const char *valueName)
{
	TiXmlDocument configFile;
	
	TiXmlNode *xmlNode = getXmlNode(path, configFile);
	if ( xmlNode == NULL ) return;

	xmlNode->ToElement()->SetAttribute ( valueName, value );

	if (!configFile.SaveFile(configPath.c_str()))
	{
		Log.write("Could not write new config to " + configPath, cLog::eLOG_TYPE_ERROR);
	}
}


//------------------------------------------------------------------------------
void cSettings::saveResolution()
{
	saveSetting("Options~Start~Resolution", (iToStr(Video.getResolutionX())+"."+iToStr(Video.getResolutionY())).c_str());
}

//------------------------------------------------------------------------------
void cSettings::saveWindowMode()
{
	saveSetting("Options~Start~Windowmode", Video.getWindowMode());
}
//------------------------------------------------------------------------------
void cSettings::saveColorDepth()
{
	saveSetting("Options~Start~ColorDepth", Video.getColDepth());
}


//------------------------------------------------------------------------------
bool cSettings::isDebug()
{
	return debug;
}

//------------------------------------------------------------------------------
void cSettings::setDebug(bool debug, bool save)
{
	this->debug = debug;
	if(save) saveSetting("Options~Game~EnableDebug", debug);
}

//------------------------------------------------------------------------------
bool cSettings::shouldAutosave()
{
	return autosave;
}

//------------------------------------------------------------------------------
void cSettings::setAutosave(bool autosave, bool save)
{
	this->autosave = autosave;
	if(save) saveSetting("Options~Game~EnableAutosave", autosave);
}

//------------------------------------------------------------------------------
bool cSettings::isAnimations()
{
	return animations;
}

//------------------------------------------------------------------------------
void cSettings::setAnimations(bool animations, bool save)
{
	this->animations = animations;
	if(save) saveSetting("Options~Game~EnableAnimations", animations);
}

//------------------------------------------------------------------------------
bool cSettings::isShadows()
{
	return shadows;
}

//------------------------------------------------------------------------------
void cSettings::setShadows(bool shadows, bool save)
{
	this->shadows = shadows;
	if(save) saveSetting("Options~Game~EnableShadows", shadows);
}

//------------------------------------------------------------------------------
bool cSettings::isAlphaEffects()
{
	return alphaEffects;
}

//------------------------------------------------------------------------------
void cSettings::setAlphaEffects(bool alphaEffects, bool save)
{
	this->alphaEffects = alphaEffects;
	if(save) saveSetting("Options~Game~EnableAlphaEffects", alphaEffects);
}

//------------------------------------------------------------------------------
bool cSettings::shouldShowDescription()
{
	return showDescription;
}

//------------------------------------------------------------------------------
void cSettings::setShowDescription(bool showDescription, bool save)
{
	this->showDescription = showDescription;
	if(save) saveSetting("Options~Game~EnableDescribtions", showDescription);
}

//------------------------------------------------------------------------------
bool cSettings::isDamageEffects()
{
	return damageEffects;
}

//------------------------------------------------------------------------------
void cSettings::setDamageEffects(bool damageEffects, bool save)
{
	this->damageEffects = damageEffects;
	if(save) saveSetting("Options~Game~EnableDamageEffects", damageEffects);
}

//------------------------------------------------------------------------------
bool cSettings::isDamageEffectsVehicles()
{
	return damageEffectsVehicles;
}

//------------------------------------------------------------------------------
void cSettings::setDamageEffectsVehicles(bool damageEffectsVehicles, bool save)
{
	this->damageEffectsVehicles = damageEffectsVehicles;
	if(save) saveSetting("Options~Game~EnableDamageEffectsVehicles", damageEffectsVehicles);
}

//------------------------------------------------------------------------------
bool cSettings::isMakeTracks()
{
	return makeTracks;
}

//------------------------------------------------------------------------------
void cSettings::setMakeTracks(bool makeTracks, bool save)
{
	this->makeTracks = makeTracks;
	if(save) saveSetting("Options~Game~EnableMakeTracks", makeTracks);
}

//------------------------------------------------------------------------------
int cSettings::getScrollSpeed()
{
	return scrollSpeed;
}

//------------------------------------------------------------------------------
void cSettings::setScrollSpeed(int scrollSpeed, bool save)
{
	this->scrollSpeed = scrollSpeed;
	if(save) saveSetting("Options~Game~ScrollSpeed", scrollSpeed);
}

//------------------------------------------------------------------------------
std::string cSettings::getIP()
{
	return ip;
}

//------------------------------------------------------------------------------
void cSettings::setIP(const char* ip, bool save)
{
	this->ip = ip;
	if(save) saveSetting("Options~Game~Net~IP", ip);
}

//------------------------------------------------------------------------------
unsigned short cSettings::getPort()
{
	return port;
}

//------------------------------------------------------------------------------
void cSettings::setPort(unsigned short port, bool save)
{
	this->port = port;
	if(save) saveSetting("Options~Game~Net~Port", port);
}

//------------------------------------------------------------------------------
std::string cSettings::getPlayerName()
{
	return playerName;
}

//------------------------------------------------------------------------------
void cSettings::setPlayerName(const char *playerName, bool save)
{
	this->playerName = playerName;
	if(save) saveSetting("Options~Game~Net~PlayerName", playerName);
}

//------------------------------------------------------------------------------
int cSettings::getPlayerColor()
{
	return playerColor;
}

//------------------------------------------------------------------------------
void cSettings::setPlayerColor(int color, bool save)
{
	this->playerColor = color;
	if(save) saveSetting("Options~Game~Net~PlayerName", color);
}

//------------------------------------------------------------------------------
std::string cSettings::getNetLogPath()
{
	return netLogPath;
}

//------------------------------------------------------------------------------
void cSettings::setNetLogPath(const char *netLogPath)
{
	this->netLogPath = netLogPath;
}

//------------------------------------------------------------------------------
std::string cSettings::getDataDir()
{
	return dataDir;
}

//------------------------------------------------------------------------------
void cSettings::setDataDir(const char *dataDir, bool save)
{
	this->dataDir = dataDir;
	if(save) saveSetting("Options~Game~Paths~Gamedata", dataDir);
}

//------------------------------------------------------------------------------
std::string cSettings::getExePath()
{
	return exePath;
}

//------------------------------------------------------------------------------
void cSettings::setExePath(const char *exePath)
{
	this->exePath = exePath;
}

//------------------------------------------------------------------------------
std::string cSettings::getLogPath()
{
	return logPath;
}

//------------------------------------------------------------------------------
void cSettings::setLogPath(const char *logPath)
{
	this->logPath = logPath;
}

//------------------------------------------------------------------------------
std::string cSettings::getHomeDir()
{
	return homeDir;
}

//------------------------------------------------------------------------------
void cSettings::setHomeDir(const char *homeDir)
{
	this->homeDir = homeDir;
}

//------------------------------------------------------------------------------
bool cSettings::isSoundEnabled()
{
	return soundEnabled;
}

//------------------------------------------------------------------------------
void cSettings::setSoundEnabled(bool soundEnabled, bool save)
{
	this->soundEnabled = soundEnabled;
	if(save) saveSetting("Options~Game~Sound~Enabled", soundEnabled);
}

//------------------------------------------------------------------------------
int cSettings::getMusicVol()
{
	return musicVol;
}

//------------------------------------------------------------------------------
void cSettings::setMusicVol(int musicVol, bool save)
{
	this->musicVol = musicVol;
	if(save) saveSetting("Options~Game~Sound~MusicVol", musicVol);
}

//------------------------------------------------------------------------------
int cSettings::getSoundVol()
{
	return soundVol;
}

//------------------------------------------------------------------------------
void cSettings::setSoundVol(int soundVol, bool save)
{
	this->soundVol = soundVol;
	if(save) saveSetting("Options~Game~Sound~SoundVol", soundVol);
}

//------------------------------------------------------------------------------
int cSettings::getVoiceVol()
{
	return voiceVol;
}

//------------------------------------------------------------------------------
void cSettings::setVoiceVol(int voiceVol, bool save)
{
	this->voiceVol = voiceVol;
	if(save) saveSetting("Options~Game~Sound~VoiceVol", voiceVol);
}

//------------------------------------------------------------------------------
int cSettings::getChunkSize()
{
	return chunkSize;
}

//------------------------------------------------------------------------------
void cSettings::setChunkSize(int chunkSize, bool save)
{
	this->chunkSize = chunkSize;
	if(save) saveSetting("Options~Game~Sound~ChunkSize", chunkSize);
}

//------------------------------------------------------------------------------
int cSettings::getFrequency()
{
	return frequency;
}

//------------------------------------------------------------------------------
void cSettings::setFrequence(int frequency, bool save)
{
	this->frequency = frequency;
	if(save) saveSetting("Options~Game~Sound~Frequency", frequency);
}

//------------------------------------------------------------------------------
bool cSettings::isMusicMute()
{
	return musicMute;
}

//------------------------------------------------------------------------------
void cSettings::setMusicMute(bool musicMute, bool save)
{
	this->musicMute = musicMute;
	if(save) saveSetting("Options~Game~Sound~MusicMute", musicMute);
}

//------------------------------------------------------------------------------
bool cSettings::isSoundMute()
{
	return soundMute;
}

//------------------------------------------------------------------------------
void cSettings::setSoundMute(bool soundMute, bool save)
{
	this->soundMute = soundMute;
	if(save) saveSetting("Options~Game~Sound~SoundMute", soundMute);
}

//------------------------------------------------------------------------------
bool cSettings::isVoiceMute()
{
	return voiceMute;
}

//------------------------------------------------------------------------------
void cSettings::setVoiceMute(bool voiceMute, bool save)
{
	this->voiceMute = voiceMute;
	if(save) saveSetting("Options~Game~Sound~VoiceMute", voiceMute);
}

//------------------------------------------------------------------------------
bool cSettings::shouldShowIntro()
{
	return showIntro;
}

//------------------------------------------------------------------------------
void cSettings::setShowIntro(bool showIntro, bool save)
{
	this->showIntro = showIntro;
	if(save) saveSetting("Options~Start~Intro", showIntro);
}

//------------------------------------------------------------------------------
bool cSettings::shouldUseFastMode()
{
	return fastMode;
}

//------------------------------------------------------------------------------
void cSettings::setFastMode(bool fastMode, bool save)
{
	this->fastMode = fastMode;
	if(save) saveSetting("Options~Start~Fastmode", fastMode);
}

//------------------------------------------------------------------------------
bool cSettings::shouldDoPrescale()
{
	return preScale;
}

//------------------------------------------------------------------------------
void cSettings::setDoPrescale(bool preScale, bool save)
{
	this->preScale = preScale;
	if(save) saveSetting("Options~Start~PreScale", preScale);
}

//------------------------------------------------------------------------------
std::string cSettings::getLanguage()
{
	return language;
}

//------------------------------------------------------------------------------
void cSettings::setLanguage(const char *language, bool save)
{
	this->language = language;
	if(save) saveSetting("Options~Start~Language", language);
}

//------------------------------------------------------------------------------
unsigned int cSettings::getCacheSize()
{
	return cacheSize;
}

//------------------------------------------------------------------------------
void cSettings::setCacheSize(unsigned int cacheSize, bool save)
{
	this->cacheSize = cacheSize;
	if(save) saveSetting("Options~Start~CacheSize", cacheSize);
}

//------------------------------------------------------------------------------
std::string cSettings::getFontPath()
{
	return fontPath;
}

//------------------------------------------------------------------------------
void cSettings::setFontPath(const char *fontPath, bool save)
{
	this->fontPath = fontPath;
	if(save) saveSetting("Options~Game~Paths~Fonts", fontPath);
}

//------------------------------------------------------------------------------
std::string cSettings::getFxPath()
{
	return fxPath;
}

//------------------------------------------------------------------------------
void cSettings::setFxPath(const char *fxPath, bool save)
{
	this->fxPath = fxPath;
	if(save) saveSetting("Options~Game~Paths~FX", fxPath);
}

//------------------------------------------------------------------------------
std::string cSettings::getGfxPath()
{
	return gfxPath;
}

//------------------------------------------------------------------------------
void cSettings::setGfxPath(const char *gfxPath, bool save)
{
	this->gfxPath = gfxPath;
	if(save) saveSetting("Options~Game~Paths~GFX", gfxPath);
}

//------------------------------------------------------------------------------
std::string cSettings::getLangPath()
{
	return langPath;
}

//------------------------------------------------------------------------------
void cSettings::setLangPath(const char *langPath, bool save)
{
	this->langPath = langPath;
	if(save) saveSetting("Options~Game~Paths~Languages", langPath);
}

//------------------------------------------------------------------------------
std::string cSettings::getMapsPath()
{
	return mapsPath;
}

//------------------------------------------------------------------------------
void cSettings::setMapsPath(const char *mapsPath, bool save)
{
	this->mapsPath = mapsPath;
	if(save) saveSetting("Options~Game~Paths~Maps", mapsPath);
}

//------------------------------------------------------------------------------
std::string cSettings::getSavesPath()
{
	return savesPath;
}

//------------------------------------------------------------------------------
void cSettings::setSavesPath(const char *savesPath, bool save)
{
	this->savesPath = savesPath;
	if(save) saveSetting("Options~Game~Paths~Saves", savesPath);
}

//------------------------------------------------------------------------------
std::string cSettings::getSoundsPath()
{
	return soundsPath;
}

//------------------------------------------------------------------------------
void cSettings::setSoundsPath(const char *soundsPath, bool save)
{
	this->soundsPath = soundsPath;
	if(save) saveSetting("Options~Game~Paths~Sounds", soundsPath);
}

//------------------------------------------------------------------------------
std::string cSettings::getVoicesPath()
{
	return voicesPath;
}

//------------------------------------------------------------------------------
void cSettings::setVoicesPath(const char *voicesPath, bool save)
{
	this->voicesPath = voicesPath;
	if(save) saveSetting("Options~Game~Paths~Voices", voicesPath);
}

//------------------------------------------------------------------------------
std::string cSettings::getMusicPath()
{
	return musicPath;
}

//------------------------------------------------------------------------------
void cSettings::setMusicPath(const char *musicPath, bool save)
{
	this->musicPath = musicPath;
	if(save) saveSetting("Options~Game~Paths~Music", musicPath);
}

//------------------------------------------------------------------------------
std::string cSettings::getVehiclesPath()
{
	return vehiclesPath;
}

//------------------------------------------------------------------------------
void cSettings::setVehiclesPath(const char *vehiclesPath, bool save)
{
	this->vehiclesPath = vehiclesPath;
	if(save) saveSetting("Options~Game~Paths~SaveValue", vehiclesPath);
}

//------------------------------------------------------------------------------
std::string cSettings::getBuildingsPath()
{
	return buildingsPath;
}

//------------------------------------------------------------------------------
void cSettings::setBuildingsPath(const char *buildingsPath, bool save)
{
	this->buildingsPath = buildingsPath;
	if(save) saveSetting("Options~Game~Paths~Buildings", buildingsPath);
}

//------------------------------------------------------------------------------
std::string cSettings::getMvePath()
{
	return mvePath;
}

//------------------------------------------------------------------------------
void cSettings::setMvePath(const char *mvePath, bool save)
{
	this->mvePath = mvePath;
	if(save) saveSetting("Options~Game~Paths~MVEs", mvePath);
}