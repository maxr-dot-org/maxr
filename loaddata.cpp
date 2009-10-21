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
///////////////////////////////////////////////////////////////////////////////
//
// Loads all relevant files and datas at the start of the game.
//
//
///////////////////////////////////////////////////////////////////////////////

#include <sstream>

#ifdef WIN32

#else
	#include <sys/stat.h>
	#include <unistd.h>
#endif

#include "autosurface.h"
#include "buildings.h"
#include "extendedtinyxml.h"
#include "loaddata.h"
#include "files.h"
#include "log.h"
#include "pcx.h"
#include "unifonts.h"
#include "keys.h"
#include "vehicles.h"
#include "clans.h"
#include "main.h"
#include "settings.h"

TiXmlDocument LanguageFile;

/**
 * Writes a Logmessage on the SplashScreen
 * @param sTxt Text to write
 * @param ok 0 writes just text, 1 writes "OK" and else "ERROR"
 * @param pos Horizontal Positionindex on SplashScreen
 */
static void MakeLog(std::string sTxt, int ok,int pos);

/**
 * Loads the selected languagepack
 * @return 1 on success
 */
static int LoadLanguage();

/**
 * Loads all Graphics
 * @param path Directory of the graphics
 * @return 1 on success
 */
static int LoadGraphics(const char* path);

/**
 * Loads the Effects
 * @param path Directory of the Effects
 * @return 1 on success
 */
static int LoadEffects(const char* path);

/**
 * Loads all Buildings
 * @param path Directory of the Buildings
 * @return 1 on success
 */
static int LoadBuildings();

/**
 * Loads all Vehicles
 * @param path Directory of the Vehicles
 * @return 1 on success
 */
static int LoadVehicles();

/**
 * Loads the clan values and stores them in the cUnitData class
 * @return 1 on success
 */
static int LoadClans();

/**
 * Loads all Musicfiles
 * @param path Directory of the Vehicles
 * @return 1 on success
 */
static int LoadMusic(const char* path);

/**
 * Loads all Sounds
 * @param path Directory of the Vehicles
 * @return 1 on success
 */
static int LoadSounds(const char* path);

/**
 * Loads all Voices
 * @param path Directory of the Vehicles
 * @return 1 on success
 */
static int LoadVoices(const char* path);

// LoadData ///////////////////////////////////////////////////////////////////
// Loads all relevant files and datas:
int LoadData ( void * )
{
	LoadingData=LOAD_GOING;


	// Load fonts for SplashMessages
	Log.write ( "Loading font for Splash Messages", LOG_TYPE_INFO );

	if(!FileExists((SettingsData.sFontPath + PATH_DELIMITER + "latin_normal.pcx").c_str())) NECESSARY_FILE_FAILURE
	if(!FileExists((SettingsData.sFontPath + PATH_DELIMITER + "latin_big.pcx").c_str())) NECESSARY_FILE_FAILURE
	if(!FileExists((SettingsData.sFontPath + PATH_DELIMITER + "latin_big_gold.pcx").c_str())) NECESSARY_FILE_FAILURE
	if(!FileExists((SettingsData.sFontPath + PATH_DELIMITER + "latin_small.pcx").c_str())) NECESSARY_FILE_FAILURE

	font = new cUnicodeFont; //init ascii fonts

	Log.mark();

	string sVersion = PACKAGE_NAME;
	sVersion += " BUILD: ";
	sVersion += MAX_BUILD_DATE; sVersion += " ";
	sVersion += PACKAGE_REV;

	MakeLog( sVersion.c_str(),0,0);

	// Load Languagepack
	MakeLog ( "Loading languagepack...", 0, 2 );

	string sTmpString;
	string sLang = SettingsData.sLanguage;
	//FIXME: here is the assumption made that the file always exists with lower cases
	for(int i=0 ; i<=2 ; i++)
	{
		if( sLang[i] < 97 )
		{
			sLang[i] += 32;
		}
	}
	sTmpString = SettingsData.sLangPath;
	sTmpString += PATH_DELIMITER;
	sTmpString += "lang_";
	sTmpString += sLang;
	sTmpString += ".xml";
	Log.write("Using langfile: " + sTmpString, cLog::eLOG_TYPE_DEBUG );
	if ( LoadLanguage()!=1 || !FileExists( sTmpString.c_str() ) )
	{
		MakeLog("",-1,2);
		LoadingData = LOAD_ERROR;
		SDL_Delay(5000);
		return -1;
	}
	else
	{
		LanguageFile.LoadFile(sTmpString.c_str());
		MakeLog ( "", 1, 2 );
	}
	Log.mark();

	// Load Keys
	MakeLog ( lngPack.i18n ( "Text~Init~Keys" ), 0, 3 );

	if(LoadKeys()!=1)
	{
		MakeLog("",-1,3);
		SDL_Delay(5000);
		LoadingData = LOAD_ERROR;
		return -1;
	}
	else
	{
		MakeLog ( "", 1, 3 );
	}
	Log.mark();

	// Load Fonts
	MakeLog ( lngPack.i18n ( "Text~Init~Fonts" ), 0, 4 );
	/* -- little bit crude but fonts are already loaded. what to do with this now? -- beko
	if (LoadFonts ( SettingsData.sFontPath.c_str() ) != 1 )
	{
		MakeLog("",-1,4);
		SDL_Delay(5000);
		LoadingData = LOAD_ERROR;
		return -1;
	}
	else
	{*/
		MakeLog ( "", 1, 4 );
	//}
	Log.mark();

	// Load Graphics
	MakeLog ( lngPack.i18n ( "Text~Init~GFX" ), 0, 5 );

	if ( LoadGraphics ( SettingsData.sGfxPath.c_str() ) != 1 )
	{
		MakeLog("",-1,5);
		Log.write ( "Error while loading graphics", LOG_TYPE_ERROR );
		SDL_Delay(5000);
		LoadingData = LOAD_ERROR;
		return -1;
	}
	else
	{
		MakeLog ( "", 1, 5 );
	}
	Log.mark();

	// Load Effects
	MakeLog ( lngPack.i18n ( "Text~Init~Effects" ), 0, 6 );

	if(LoadEffects ( SettingsData.sFxPath.c_str() ) != 1)
	{
		MakeLog("",-1,6);
		SDL_Delay(5000);
		LoadingData = LOAD_ERROR;
		return -1;
	}
	else
	{
		MakeLog ( "", 1, 6 );
	}
	Log.mark();

	// Load Vehicles
	MakeLog ( lngPack.i18n ( "Text~Init~Vehicles" ), 0, 7 );

	if ( LoadVehicles() != 1 )
	{
		MakeLog("",-1,7);
		SDL_Delay(5000);
		LoadingData = LOAD_ERROR;
		return -1;
	}
	else
	{
		MakeLog ( "", 1, 7 );
	}
	Log.mark();

	// Load Buildings
	MakeLog ( lngPack.i18n ( "Text~Init~Buildings" ), 0, 8 );

	if ( LoadBuildings() != 1)
	{
		MakeLog("",-1,8);
		SDL_Delay(5000);
		LoadingData = LOAD_ERROR;
		return -1;
	}
	else
	{
		MakeLog ( "", 1, 8 );
	}
	Log.mark();

	MakeLog ( lngPack.i18n ( "Text~Init~Clans" ), 0, 9 );

	// Load Clan Settings
	if (LoadClans () != 1)
	{
		SDL_Delay(5000);
		LoadingData = LOAD_ERROR;
		return -1;
	}
	else
	{
		MakeLog ( "", 1, 9 );
	}
	Log.mark();

	// Load Music
	MakeLog ( lngPack.i18n ( "Text~Init~Music" ), 0, 10 );

	if ( LoadMusic ( SettingsData.sMusicPath.c_str() ) != 1)
	{
		MakeLog("",-1,10);
		SDL_Delay(5000);
		LoadingData = LOAD_ERROR;
		return -1;
	}
	else
	{
		MakeLog ( "", 1, 10 );
	}
	Log.mark();

	// Load Sounds
	MakeLog ( lngPack.i18n ( "Text~Init~Sounds" ), 0, 11 );

	if ( LoadSounds ( SettingsData.sSoundsPath.c_str() ) != 1)
	{
		MakeLog("",-1,11);
		SDL_Delay(5000);
		LoadingData = LOAD_ERROR;
		return -1;
	}
	else
	{
		MakeLog ( "", 1, 11 );
	}
	Log.mark();

	// Load Voices
	MakeLog ( lngPack.i18n ( "Text~Init~Voices" ), 0, 12 );

	if(LoadVoices ( SettingsData.sVoicesPath.c_str() ) != 1)
	{
		MakeLog("",-1,12);
		SDL_Delay(5000);
		LoadingData = LOAD_ERROR;
		return -1;
	}
	else
	{
		MakeLog ( "", 1, 12 );
	}
	Log.mark();

	SDL_Delay(1000);
	LoadingData=LOAD_FINISHED;
	return 1;
}

// MakeLog ///////////////////////////////////////////////////////////////////
// Writes a Logmessage on the SplashScreen:
void MakeLog ( string sTxt,int ok,int pos )
{
	SDL_Rect rDest = {22, 152, 228, font->getFontHeight(FONT_LATIN_BIG_GOLD)};
	SDL_Rect rDest2 = {250, 152, 230, font->getFontHeight(FONT_LATIN_BIG_GOLD)};
	SDL_Rect rSrc;

	switch ( ok )
	{
	case 0:
		font->showText(rDest.x, rDest.y + rDest.h*pos, sTxt, FONT_LATIN_NORMAL);
		rSrc=rDest;
		rSrc.y = rDest.y + rDest.h*pos;
		if(pos == 0) //need full line for first entry version information
		{
			SDL_BlitSurface ( buffer, NULL, screen, NULL );
			SDL_UpdateRect ( screen, rDest.x, rDest.y + rDest.h*pos, rDest.w+rDest2.w, rDest.h );
		}
		else
		{
			SDL_BlitSurface ( buffer, &rSrc, screen, &rSrc );
			SDL_UpdateRect ( screen, rDest.x, rDest.y + rDest.h*pos, rDest.w, rDest.h );
		}
		break;

	case 1:
		font->showText(rDest2.x, rDest2.y + rDest2.h*pos, "OK", FONT_LATIN_BIG_GOLD);
		break;

	default:
		font->showText(rDest2.x, rDest2.y + rDest2.h*pos, "ERROR ..check maxr.log!", FONT_LATIN_BIG_GOLD);
		break;
	}

	if ( ok != 0 )
	{
		rSrc=rDest2;
		rSrc.y = rDest2.y + rDest2.h*pos;
		SDL_BlitSurface ( buffer, &rSrc, screen, &rSrc );
		SDL_UpdateRect ( screen, rDest2.x, rDest2.y + rDest2.h*pos, rDest2.w, rDest2.h );
	}
}

/**
 * Loades a graphic to the surface
 * @param dest Destination surface
 * @param directory Directory of the file
 * @param filename Name of the file
 * @return 1 on success
 */
static int LoadGraphicToSurface(SDL_Surface* &dest, const char* directory, const char* filename)
{
	string filepath;
	if(strcmp(directory,""))
	{
		filepath = directory;
		filepath += PATH_DELIMITER;
	}
	filepath += filename;
	if(!FileExists(filepath.c_str()))
	{
		dest = NULL;
		Log.write("Missing GFX - your MAXR install seems to be incomplete!", cLog::eLOG_TYPE_ERROR);
		//LoadingData=LOAD_ERROR;
		return 0;
	}

	dest = LoadPCX(filepath.c_str());

	filepath.insert(0,"File loaded: ");
	Log.write ( filepath.c_str(), LOG_TYPE_DEBUG );

	return 1;
}

/**
 * Loades a effectgraphic to the surface
 * @param dest Destination surface
 * @param directory Directory of the file
 * @param filename Name of the file
 * @return 1 on success
 */
static int LoadEffectGraphicToSurface(SDL_Surface** &dest, const char* directory, const char* filename)
{
	string filepath;
	if(strcmp(directory,""))
	{
		filepath = directory;
		filepath += PATH_DELIMITER;
	}
	filepath += filename;
	if(!FileExists(filepath.c_str()))
	{
		Log.write("Missing GFX - your MAXR install seems to be incomplete!", cLog::eLOG_TYPE_ERROR);
		//LoadingData=LOAD_ERROR;
		return 0;
	}


	dest = new SDL_Surface*[2];
	if(!dest) { Log.write("Out of memory", cLog::eLOG_TYPE_MEM); }
	dest[0] = LoadPCX(filepath.c_str());
	dest[1] = LoadPCX(filepath.c_str());

	filepath.insert(0,"Effect successful loaded: ");
	Log.write ( filepath.c_str(), LOG_TYPE_DEBUG );

	return 1;
}

// LoadEffectAlphacToSurface /////////////////////////////////////////////////
// Loades a effectgraphic as aplha to the surface:
int LoadEffectAlphaToSurface(SDL_Surface** &dest, const char* directory, const char* filename, int alpha)
{
	string filepath;
	if(strcmp(directory,""))
	{
		filepath = directory;
		filepath += PATH_DELIMITER;
	}
	filepath += filename;
	if(!FileExists(filepath.c_str()))
		return 0;

	dest = new SDL_Surface*[2];
	if(!dest) { Log.write("Out of memory", cLog::eLOG_TYPE_MEM); }
	dest[0] = LoadPCX(filepath.c_str());
	SDL_SetAlpha(dest[0],SDL_SRCALPHA,alpha);
	dest[1] = LoadPCX(filepath.c_str());
	SDL_SetAlpha(dest[1],SDL_SRCALPHA,alpha);

	filepath.insert(0,"Effectalpha loaded: ");
	Log.write ( filepath.c_str(), LOG_TYPE_DEBUG );

	return 1;
}

/**
 * Loades a soundfile to the Mix_Chunk
 * @param dest Destination Mix_Chunk
 * @param directory Directory of the file
 * @param filename Name of the file
 * @return 1 on success
 */
static int LoadSoundfile(sSOUND *&dest, const char* directory, const char* filename)
{
	string filepath;
	if(strcmp(directory,""))
	{
		filepath = directory;
		filepath += PATH_DELIMITER;
	}
	filepath += filename;
	if(!FileExists(filepath.c_str()))
		return 0;

	dest = Mix_LoadWAV(filepath.c_str());

	return 1;
}

/**
 * Loades a unitsoundfile to the Mix_Chunk. If the file doesn't exists a dummy file will be loaded
 * @param dest Destination Mix_Chunk
 * @param directory Directory of the file, relativ to the main vehicles directory
 * @param filename Name of the file
 */
static void LoadUnitSoundfile(sSOUND *&dest, const char* directory, const char* filename)
{
	SDL_RWops *file;
	string filepath;
	if(strcmp(directory,""))
		filepath += directory;
	filepath += filename;
	if(!SoundData.DummySound)
	{
		string sTmpString;
		sTmpString = SettingsData.sSoundsPath + PATH_DELIMITER + "dummy.wav";
		if(FileExists(sTmpString.c_str()))
		{
			SoundData.DummySound = Mix_LoadWAV(sTmpString.c_str());
			if(!SoundData.DummySound)
				Log.write("Can't load dummy.wav", LOG_TYPE_WARNING);
		}
	}
	// Not using FileExists to avoid unnecessary warnings in log file
	if(!(file=SDL_RWFromFile ( filepath.c_str(), "r" )))
	{
		dest = SoundData.DummySound;
		return;
	}
	SDL_RWclose ( file );

	dest = Mix_LoadWAV(filepath.c_str());
}


// ReadMaxXml /////////////////////////////////////////////////////////////////
// Reads the Information from the max.xml:
int ReadMaxXml()
{

	string sTmpString;
	// Prepare max.xml for reading
	TiXmlDocument MaxXml;
	ExTiXmlNode * pXmlNode = NULL;

	if(SettingsData.sConfig.empty() == 1) //we need at least sConfig here
	{
		Log.write ( "sConfig not set!", cLog::eLOG_TYPE_WARNING );
		SettingsData.sConfig = MAX_XML;
	}

	if(!FileExists(SettingsData.sConfig.c_str()))
	{
		Log.write ( "Generating new config file max.xml", cLog::eLOG_TYPE_INFO );
		if (GenerateMaxXml() == -1)
		{
			return -1;
		}
	}
	if (!MaxXml.LoadFile(SettingsData.sConfig.c_str()))
	{
		Log.write ( "Can't read max.xml\n", LOG_TYPE_WARNING );
		if( GenerateMaxXml() == -1)
		{
			return -1;
		}
	}

	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Paths", "Gamedata", NULL)))
	{
		Log.write ( "Can't find gamedata path node in max.xml", LOG_TYPE_WARNING );
	}
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
	{
		SettingsData.sDataDir = searchData(sTmpString); //verify datadir or search for other paths
	}
	else
	{
		Log.write ( "Can't find gamedata path in max.xml", LOG_TYPE_WARNING );
		SettingsData.sDataDir = searchData(); //do default gamedata search
	}

	string sDataDir = "";
	if(!SettingsData.sDataDir.empty())
	{
		sDataDir = SettingsData.sDataDir + PATH_DELIMITER;
	}

	// START Options
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Paths", "Languages", NULL)))
	{
		Log.write ( "Can't find language path node in max.xml", LOG_TYPE_WARNING );
	}
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
	{
		SettingsData.sLangPath = sDataDir + sTmpString;
	}
	else
	{
		Log.write ( "Can't find language path in max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.sLangPath =  sDataDir + "languages";
	}

	// Resolution
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Start","Resolution", NULL)))
		Log.write ( "Can't find Resolution-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
	{
		SettingsData.iScreenW = atoi(sTmpString.substr(0,sTmpString.find(".",0)).c_str());
		SettingsData.iScreenH = atoi(sTmpString.substr(sTmpString.find(".",0)+1,sTmpString.length()).c_str());
	}
	else
	{
		Log.write ( "Can't load resolution from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.iScreenW = 640;
		SettingsData.iScreenH = 480;
	}

	// ColourDepth
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Start","ColourDepth", NULL)))
		Log.write ( "Can't find ColourDepth-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Num"))
		SettingsData.iColourDepth = atoi(sTmpString.c_str());
	else
	{
		Log.write ( "Can't load ColourDepth from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.iColourDepth = 32;
	}
	// Intro
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Start","Intro", NULL)))
		Log.write ( "Can't find Intro-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"YN"))
		SettingsData.bIntro = pXmlNode->XmlDataToBool(sTmpString);
	else
	{
		Log.write ( "Can't load Intro from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.bIntro = true;
	}
	// Windowmode
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Start","Windowmode", NULL)))
		Log.write ( "Can't find Windowmode-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"YN"))
		SettingsData.bWindowMode = pXmlNode->XmlDataToBool(sTmpString);
	else
	{
		Log.write ( "Can't load Windowmode from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.bWindowMode = true;
	}
	// Intro
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Start","Fastmode", NULL)))
		Log.write ( "Can't find Fastmode-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"YN"))
		SettingsData.bFastMode = pXmlNode->XmlDataToBool(sTmpString);
	else
	{
		Log.write ( "Can't load Fastmode from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.bFastMode = false;
	}
	// PreScale
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Start","PreScale", NULL)))
		Log.write ( "Can't find PreScale-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"YN"))
		SettingsData.bPreScale = pXmlNode->XmlDataToBool(sTmpString);
	else
	{
		Log.write ( "Can't load PreScale-Mode from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.bPreScale = false;
	}

	// MaxCacheSize for prescaling of units
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Start","CacheSize", NULL)))
		Log.write ( "Can't find CacheSize-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Num"))
		SettingsData.iCacheSize = atoi(sTmpString.c_str());
	else
	{
		Log.write ( "Can't load CacheSize-Node from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.iCacheSize=400;
	}

	// Language
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Start","Language", NULL)))
		Log.write ( "Can't find Language-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
	{
		for(int i=0; i < ( int )sTmpString.size(); i++) //make sure we've only upper characters for compares later
		{
			sTmpString[i] = toupper(sTmpString[i]);
		}
		SettingsData.sLanguage = sTmpString;
	}
	else
	{
		Log.write ( "Can't load Language-Path from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.sLanguage = "ENG";
	}

	// GAME Options
	// EnableAutosave
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","EnableAutosave", NULL)))
		Log.write ( "Can't find EnableAutosave-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"YN"))
		SettingsData.bAutoSave = pXmlNode->XmlDataToBool(sTmpString);
	else
	{
		Log.write ( "Can't load EnableAutosave from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.bAutoSave = true;
	}

	//Enable Debug
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","EnableDebug", NULL)))
		Log.write ( "Can't find EnableDebug-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"YN"))
	{
		SettingsData.bDebug = pXmlNode->XmlDataToBool(sTmpString);
		if(!SettingsData.bDebug)
		{
			Log.write("Debugmode disabled - for verbose output please enable Debug in max.xml", cLog::eLOG_TYPE_WARNING);
		}
		else
		{
			Log.write("Debugmode enabled", cLog::eLOG_TYPE_INFO);
		}
	}
	else
	{
		Log.write ( "Can't load EnableDebug from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.bDebug = false;
	}

	// EnableAnimations
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","EnableAnimations", NULL)))
		Log.write ( "Can't find EnableAnimations-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"YN"))
		SettingsData.bAnimations = pXmlNode->XmlDataToBool(sTmpString);
	else
	{
		Log.write ( "Can't load EnableAnimations from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.bAnimations = true;
	}
	// EnableShadows
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","EnableShadows", NULL)))
		Log.write ( "Can't find EnableShadows-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"YN"))
		SettingsData.bShadows = pXmlNode->XmlDataToBool(sTmpString);
	else
	{
		Log.write ( "Can't load EnableShadows from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.bShadows = true;
	}
	// EnableAlphaEffects
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","EnableAlphaEffects", NULL)))
		Log.write ( "Can't find EnableAlphaEffects-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"YN"))
		SettingsData.bAlphaEffects = pXmlNode->XmlDataToBool(sTmpString);
	else
	{
		Log.write ( "Can't load EnableAlphaEffects from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.bAlphaEffects = true;
	}
	// EnableDescribtions
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","EnableDescribtions", NULL)))
		Log.write ( "Can't find EnableDescribtions-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"YN"))
		SettingsData.bShowDescription = pXmlNode->XmlDataToBool(sTmpString);
	else
	{
		Log.write ( "Can't load EnableDescribtions from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.bShowDescription = true;
	}
	// EnableDamageEffects
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","EnableDamageEffects", NULL)))
		Log.write ( "Can't find EnableDamageEffects-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"YN"))
		SettingsData.bDamageEffects = pXmlNode->XmlDataToBool(sTmpString);
	else
	{
		Log.write ( "Can't load EnableDamageEffects from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.bDamageEffects = true;
	}
	// EnableDamageEffectsVehicles
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","EnableDamageEffectsVehicles", NULL)))
		Log.write ( "Can't find EnableDamageEffectsVehicles-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"YN"))
		SettingsData.bDamageEffectsVehicles = pXmlNode->XmlDataToBool(sTmpString);
	else
	{
		Log.write ( "Can't load EnableDamageEffectsVehicles from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.bDamageEffectsVehicles = true;
	}
	// EnableMakeTracks
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","EnableMakeTracks", NULL)))
		Log.write ( "Can't find EnableMakeTracks-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"YN"))
		SettingsData.bMakeTracks = pXmlNode->XmlDataToBool(sTmpString);
	else
	{
		Log.write ( "Can't load EnableMakeTracks from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.bMakeTracks = true;
	}
	// ScrollSpeed
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","ScrollSpeed", NULL)))
		Log.write ( "Can't find ScrollSpeed-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Num"))
		SettingsData.iScrollSpeed = atoi(sTmpString.c_str());
	else
	{
		Log.write ( "Can't load ColourDepth from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.iScrollSpeed = 32;
	}

	// GAME-NET Options
	//IP
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Net","IP", NULL)))
		Log.write ( "Can't find Net-IP-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		SettingsData.sIP = sTmpString;
	else
	{
		Log.write ( "Can't load IP from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.sIP = "127.0.0.1";
	}
	//Port
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Net","Port", NULL)))
		Log.write ( "Can't find Net-Port-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Num"))
		SettingsData.iPort = atoi(sTmpString.c_str());
	else
	{
		Log.write ( "Can't load Port from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.iPort = 58600;
	}
	//PlayerName and PlayerColor
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Net","PlayerName", NULL)))
		Log.write ( "Can't find Net-PlayerName-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		SettingsData.sPlayerName = sTmpString;
	else
	{
		Log.write ( "Can't load PlayerName from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.sPlayerName = "Commander";
	}
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Num"))
	{
		int iTmp = atoi(sTmpString.c_str());
		if(iTmp >= PLAYERCOLORS || iTmp < 0)
		{
			Log.write ( "Invalid playercolour read: "+ sTmpString + ". Using default value", LOG_TYPE_WARNING );
			iTmp=0;
		}
		SettingsData.iColor = iTmp;
	}
	else
	{
		Log.write ( "Can't load playercolor from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.iColor = 0;
	}

	// GAME-SOUND Options
	// Enabled
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Sound", "Enabled", NULL)))
		Log.write ( "Can't find Sound-Enabled-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"YN"))
		SettingsData.bSoundEnabled = pXmlNode->XmlDataToBool(sTmpString);
	else
	{
		Log.write ( "Can't load Enabled from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.bSoundEnabled = true;
	}
	// MusicMute
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Sound", "MusicMute", NULL)))
		Log.write ( "Can't find Sound-MusicMute-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"YN"))
		SettingsData.MusicMute = pXmlNode->XmlDataToBool(sTmpString);
	else
	{
		Log.write ( "Can't load MusicMute from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.MusicMute = false;
	}
	// SoundMute
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Sound", "SoundMute", NULL)))
		Log.write ( "Can't find Sound-SoundMute-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"YN"))
		SettingsData.SoundMute = pXmlNode->XmlDataToBool(sTmpString);
	else
	{
		Log.write ( "Can't load SoundMute from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.SoundMute = false;
	}
	// VoiceMute
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Sound", "VoiceMute", NULL)))
		Log.write ( "Can't find Sound-VoiceMute-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"YN"))
		SettingsData.VoiceMute = pXmlNode->XmlDataToBool(sTmpString);
	else
	{
		Log.write ( "Can't load VoiceMute from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.VoiceMute = false;
	}
	//MusicVol
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Sound","MusicVol", NULL)))
		Log.write ( "Can't find Sound-MusicVol-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Num"))
		SettingsData.MusicVol  = atoi(sTmpString.c_str());
	else
	{
		Log.write ( "Can't load MusicVol from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.MusicVol  = 128;
	}
	//SoundVol
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Sound","SoundVol", NULL)))
		Log.write ( "Can't find Sound-SoundVol-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Num"))
		SettingsData.SoundVol  = atoi(sTmpString.c_str());
	else
	{
		Log.write ( "Can't load SoundVol from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.SoundVol  = 128;
	}
	//VoiceVol
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Sound","VoiceVol", NULL)))
		Log.write ( "Can't find Sound-VoiceVol-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Num"))
		SettingsData.VoiceVol  = atoi(sTmpString.c_str());
	else
	{
		Log.write ( "Can't load VoiceVol from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.VoiceVol  = 128;
	}
	//ChunkSize
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Sound","ChunkSize", NULL)))
		Log.write ( "Can't find Sound-ChunkSize-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Num"))
		SettingsData.iChunkSize  = atoi(sTmpString.c_str());
	else
	{
		Log.write ( "Can't load ChunkSize from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.iChunkSize  = 2048;
	}
	//Frequency
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Sound","Frequency", NULL)))
		Log.write ( "Can't find Sound-Frequency-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Num"))
		SettingsData.iFrequency  = atoi(sTmpString.c_str());
	else
	{
		Log.write ( "Can't load Frequency from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.iFrequency  = 44100;
	}

	// PATHs
	//Fonts
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Paths","Fonts", NULL)))
		Log.write ( "Can't find Path-Fonts-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		SettingsData.sFontPath = sDataDir + sTmpString;
	else
	{
		Log.write ( "Can't load FontsPath from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.sFontPath = sDataDir + "fonts";
	}
	//FX
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Paths","FX", NULL)))
		Log.write ( "Can't find Path-FX-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		SettingsData.sFxPath = sDataDir + sTmpString;
	else
	{
		Log.write ( "Can't load FX-Path from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.sFxPath = sDataDir + "fx";
	}
	//Graphics
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Paths","GFX", NULL)))
		Log.write ( "Can't find Path-GFX-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		SettingsData.sGfxPath = sDataDir +  sTmpString;
	else
	{
		Log.write ( "Can't load GFX-Path from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.sGfxPath = sDataDir + "gfx";
	}

	//Maps
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Paths","Maps", NULL)))
		Log.write ( "Can't find Path-Maps-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		SettingsData.sMapsPath = sDataDir + sTmpString;
	else
	{
		Log.write ( "Can't load Maps-Path from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.sMapsPath = sDataDir + "maps";
	}
	//Saves
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Paths","Saves", NULL)))
		Log.write ( "Can't find Path-Saves-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		SettingsData.sSavesPath = sTmpString; //use absolut paths for saves - do not add sDataDir or sHome
	else
	{
		Log.write ( "Can't load Saves-Path from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.sSavesPath = SettingsData.sHome + PATH_DELIMITER + "saves";
	}
	//Sounds
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Paths","Sounds", NULL)))
		Log.write ( "Can't find Path-Sounds-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		SettingsData.sSoundsPath = sDataDir + sTmpString;
	else
	{
		Log.write ( "Can't load Sounds-Path from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.sSoundsPath = sDataDir + "sounds";
	}
	//Voices
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Paths","Voices", NULL)))
		Log.write ( "Can't find Path-Voices-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		SettingsData.sVoicesPath = sDataDir + sTmpString;
	else
	{
		Log.write ( "Can't load Voices-Path from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.sVoicesPath = sDataDir + "voices";
	}
	//Music
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Paths","Music", NULL)))
		Log.write ( "Can't find Path-Music-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		SettingsData.sMusicPath = sDataDir+ sTmpString;
	else
	{
		Log.write ( "Can't load Music-Path from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.sMusicPath = sDataDir + "music";
	}
	//Vehicles
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Paths","Vehicles", NULL)))
		Log.write ( "Can't find Path-Vehicles-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		SettingsData.sVehiclesPath = sDataDir + sTmpString;
	else
	{
		Log.write ( "Can't load Vehicles-Path from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.sVehiclesPath = sDataDir + "vehicles";
	}
	//Buildings
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Paths","Buildings", NULL)))
		Log.write ( "Can't find Path-Buildings-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		SettingsData.sBuildingsPath = sDataDir + sTmpString;
	else
	{
		Log.write ( "Can't load Buildings-Path from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.sBuildingsPath = sDataDir + "buildings";
	}
	//MVEs
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Paths","MVEs", NULL)))
		Log.write ( "Can't find Path-MVEs-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		SettingsData.sMVEPath = sDataDir + sTmpString;
	else
	{
		Log.write ( "Can't load MVEs-Path from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.sMVEPath = sDataDir + "mve";
	}


	//BEGIN SANITY CHECK SCREEN RES

	const SDL_VideoInfo *vInfo = SDL_GetVideoInfo();
	Uint8 uBpp = vInfo->vfmt->BitsPerPixel;

	SDL_Rect** rModes;
	/* Get available fullscreen/hardware modes. Check for HWSURFACE first. If that doesn't work use a SWSURFACE instead. If that doesn't work.. die.*/
	if(!SettingsData.bWindowMode)
	  rModes = SDL_ListModes(vInfo->vfmt, SDL_FULLSCREEN|SDL_HWSURFACE); //try with HWSURFACE
	else
	  rModes = SDL_ListModes(vInfo->vfmt, SDL_HWSURFACE);
	/* Check if there are any modes available */
	if (rModes == (SDL_Rect**)0)
	{
	  if(!SettingsData.bWindowMode)
	    rModes = SDL_ListModes(vInfo->vfmt, SDL_FULLSCREEN|SDL_SWSURFACE); //HWSURFACE didn't work. Try with SWSURFACE
	  else
	    rModes = SDL_ListModes(vInfo->vfmt, SDL_HWSURFACE);

	    //we really can't use HWSURFACE or SWSURFACE :(
	    if (rModes == (SDL_Rect**)0)
	    {
	      Log.write("No video modes available", cLog::eLOG_TYPE_ERROR);
	      return -1;
	    }
	    OtherData.iSurface = SDL_SWSURFACE;
	    Log.write("Switched to SDL_SWSURFACE", cLog::eLOG_TYPE_WARNING);
	}
	else
	{
	  OtherData.iSurface = SDL_HWSURFACE;
	  Log.write("Switched to SDL_HWSURFACE", cLog::eLOG_TYPE_DEBUG);
	}
	/* Check if our resolution is restricted */
	if (rModes == (SDL_Rect**)-1) {
	    Log.write("All resolutions available", cLog::eLOG_TYPE_DEBUG);
	}
	else
	{
	    Log.write("Not all resolutions available", cLog::eLOG_TYPE_WARNING);
	    /* Print valid modes */
	    Log.write("Available video modes for "+iToStr(uBpp)+" bpp", cLog::eLOG_TYPE_DEBUG);
	    bool bRequestedMode=false;
	    bool bFoundMinimalRes=false; //we need at least(!)! 640x480
	    for (int i=0; rModes[i]; ++i)
	    {
	      Log.write(iToStr(rModes[i]->w)+"x"+iToStr(rModes[i]->h), cLog::eLOG_TYPE_DEBUG);
	      if(rModes[i]->w == SettingsData.iScreenW && rModes[i]->h == SettingsData.iScreenH)
	      {
		bRequestedMode=true;
		Log.write(" => Found requested video mode :)", cLog::eLOG_TYPE_DEBUG);
	      }

	      if(rModes[i]->w == 640 && rModes[i]->h == 480)
	      {
		bFoundMinimalRes=true; //and we even found our default mode
	      }
	    }

	    if(!bRequestedMode)
	    {
	      bool bFoundAlternate=false;
	      Log.write(" => Could not find requested video mode :(", cLog::eLOG_TYPE_ERROR);
	      if(bFoundMinimalRes)
	      {
		bFoundAlternate=true;
		SettingsData.iScreenW=640;
		SettingsData.iScreenH=480;
	      }
	      else //uuh, we can't use our minimal video mode. let's hope we find a mode > 640x480 instead
	      {

		for (int i=0; rModes[i]; ++i)
		{
		    if(rModes[i]->w > 640 && rModes[i]->h > 480)
		    {
			//this may become a very HIGH resolution maxr might look odd with
		      bFoundAlternate=true;
		      SettingsData.iScreenW=rModes[i]->w;
		      SettingsData.iScreenH=rModes[i]->h;
		    }
		}
	      }
	      if(bFoundAlternate)
	      {
		Log.write("  ==> Using "+iToStr(SettingsData.iScreenW)+"x"+iToStr(SettingsData.iScreenH)+" instead ;)", cLog::eLOG_TYPE_INFO);
	      }
	      else
	      {
		Log.write("  ==> No alternative video mode found ;(", cLog::eLOG_TYPE_INFO);
		return -1;
	      }
	    }
	}

	//END SANITY CHECK SCREEN RES

	if(SettingsData.bDebug) //Print settingslist to log
	{
		string sTmp;

		#define SON "Enabled"
		#define SOFF "Disabled"
		Log.mark();
		Log.write ("I read the following settings:", cLog::eLOG_TYPE_DEBUG);
		Log.write ("Screensize    == " + iToStr(SettingsData.iScreenW) + "x" + iToStr(SettingsData.iScreenH) + "x" + iToStr(SettingsData.iColourDepth) + "bpp", cLog::eLOG_TYPE_DEBUG);
		sTmp =  SettingsData.bIntro?SON:SOFF;
		Log.write ("Intro         == "+sTmp, cLog::eLOG_TYPE_DEBUG);
		sTmp =  SettingsData.bWindowMode?SON:SOFF;
		Log.write ("Windowmode    == "+ sTmp, cLog::eLOG_TYPE_DEBUG);
		sTmp =  SettingsData.bFastMode?SON:SOFF;
		Log.write ("Fastmode      == "+ sTmp, cLog::eLOG_TYPE_DEBUG);
		sTmp =  SettingsData.bPreScale?SON:SOFF;
		Log.write ("PreScale      == "+ sTmp, cLog::eLOG_TYPE_DEBUG);
		Log.write ("CacheSize     == "+ iToStr(SettingsData.iCacheSize), cLog::eLOG_TYPE_DEBUG)	;
		//sTmp =  SettingsData.bDebug?SON:SOFF; //we don't need debug value because we only print thall on debug!
		//Log.write ("Debugmode "+ sTmp, cLog::eLOG_TYPE_DEBUG);
		sTmp =  SettingsData.bAutoSave?SON:SOFF;
		Log.write ("Autosave      == "+ sTmp, cLog::eLOG_TYPE_DEBUG);
		sTmp =  SettingsData.bAnimations?SON:SOFF;
		Log.write ("Animation     == "+ sTmp, cLog::eLOG_TYPE_DEBUG);
		sTmp =  SettingsData.bShadows?SON:SOFF;
		Log.write ("Shadows       == "+ sTmp, cLog::eLOG_TYPE_DEBUG);
		sTmp =  SettingsData.bAlphaEffects?SON:SOFF;
		Log.write ("Alphaeffect   == "+ sTmp, cLog::eLOG_TYPE_DEBUG);
		sTmp =  SettingsData.bDamageEffects?SON:SOFF;
		Log.write ("Damageeffect  == "+ sTmp, cLog::eLOG_TYPE_DEBUG);
		sTmp =  SettingsData.bDamageEffectsVehicles?SON:SOFF;
		Log.write ("Vehicledamage == "+ sTmp, cLog::eLOG_TYPE_DEBUG);
		sTmp =  SettingsData.bMakeTracks?SON:SOFF;
		Log.write ("Vehicletracks == "+ sTmp, cLog::eLOG_TYPE_DEBUG);
		sTmp =  SettingsData.bShowDescription?SON:SOFF;
		Log.write ("Description   == "+ sTmp, cLog::eLOG_TYPE_DEBUG);
		Log.write ("Language      == "+ SettingsData.sLanguage, cLog::eLOG_TYPE_DEBUG);
		Log.write ("Scrollspeed   == " + iToStr(SettingsData.iScrollSpeed), cLog::eLOG_TYPE_DEBUG);
		Log.write ("IP            == "+ SettingsData.sIP, cLog::eLOG_TYPE_DEBUG);
		Log.write ("Port          == " + iToStr(SettingsData.iPort), cLog::eLOG_TYPE_DEBUG);
		Log.write ("Playername    == "+ SettingsData.sPlayerName, cLog::eLOG_TYPE_DEBUG);
		Log.write ("Playercolor   == " + iToStr(SettingsData.iColor), cLog::eLOG_TYPE_DEBUG);
		sTmp =  SettingsData.bSoundEnabled?SON:SOFF;
		Log.write ("Sound         == "+ sTmp, cLog::eLOG_TYPE_DEBUG);
		Log.write ("Chunksize     == " + iToStr(SettingsData.iChunkSize), cLog::eLOG_TYPE_DEBUG);
		Log.write ("Frequency     == " + iToStr(SettingsData.iFrequency), cLog::eLOG_TYPE_DEBUG);
		sTmp =  SettingsData.MusicMute?SON:SOFF;
		Log.write ("Musicvolume   == " + iToStr(SettingsData.MusicVol) + " Mute: " + sTmp, cLog::eLOG_TYPE_DEBUG);
		sTmp =  SettingsData.SoundMute?SON:SOFF;
		Log.write ("Soundvolume   == " + iToStr(SettingsData.SoundVol) + " Mute: " + sTmp, cLog::eLOG_TYPE_DEBUG);
		sTmp =  SettingsData.VoiceMute?SON:SOFF;
		Log.write ("Voicevolume   == " + iToStr(SettingsData.VoiceVol) + " Mute: " + sTmp, cLog::eLOG_TYPE_DEBUG);
		Log.mark();

	}
	return 0;
}

static int LoadLanguage()
{
	if( lngPack.SetCurrentLanguage(SettingsData.sLanguage) != 0 )			// Set the language code
	{
		// Not a valid language code, critical fail!
		Log.write( "Not a valid language code!" , cLog::eLOG_TYPE_ERROR );
		return 0;
	}
	if( lngPack.ReadLanguagePack() != 0 )					// Load the translations
	{
		// Could not load the language, critical fail!
		Log.write( "Could not load the language!" , cLog::eLOG_TYPE_ERROR );
		return 0;
	}
	return 1;
}

int GenerateMaxXml()
{
	TiXmlDocument ConfigDoc;
	TiXmlElement *element = NULL;
	TiXmlElement *rootnode = new TiXmlElement ( "Options" );
	ConfigDoc.LinkEndChild ( rootnode );

	//BEGIN STARTNODE

	TiXmlElement *startnode = new TiXmlElement("Start");

	element = new TiXmlElement ( "Resolution" );
	element->SetAttribute ( "Text", "640.480");
	startnode->LinkEndChild(element);

	element = new TiXmlElement ( "ColourDepth" );
	element->SetAttribute ( "Num", "32");
	startnode->LinkEndChild(element);

	element = new TiXmlElement ( "Intro" );
	element->SetAttribute ( "YN", "Yes");
	startnode->LinkEndChild(element);

	element = new TiXmlElement ( "Windowmode" );
	element->SetAttribute ( "YN", "Yes");
	startnode->LinkEndChild(element);

	element = new TiXmlElement ( "Fastmode" );
	element->SetAttribute ( "YN", "Yes");
	startnode->LinkEndChild(element);

	element = new TiXmlElement ( "PreScale" );
	element->SetAttribute ( "YN", "No");
	startnode->LinkEndChild(element);

	element = new TiXmlElement ( "CacheSize" );
	element->SetAttribute ( "Num", "400");
	startnode->LinkEndChild(element);

	element = new TiXmlElement ( "Language" );
	element->SetAttribute ( "Text", "ENG");
	startnode->LinkEndChild(element);

	ConfigDoc.RootElement()->LinkEndChild(startnode);

	//END STARTNODE
	//BEGIN GAMENODE
	TiXmlElement *gamenode = new TiXmlElement("Game");
	TiXmlElement *netnode = new TiXmlElement("Net");
	TiXmlElement *soundnode = new TiXmlElement("Sound");
	TiXmlElement *pathsnode = new TiXmlElement("Paths");

	element = new TiXmlElement ( "EnableAutosave" );
	element->SetAttribute ( "YN", "Yes");
	gamenode->LinkEndChild(element);

	element = new TiXmlElement ( "EnableDebug" );
	element->SetAttribute ( "YN", "Yes");
	gamenode->LinkEndChild(element);

	element = new TiXmlElement ( "EnableAnimations" );
	element->SetAttribute ( "YN", "Yes");
	gamenode->LinkEndChild(element);

	element = new TiXmlElement ( "EnableShadows" );
	element->SetAttribute ( "YN", "Yes");
	gamenode->LinkEndChild(element);

	element = new TiXmlElement ( "EnableAlphaEffects" );
	element->SetAttribute ( "YN", "Yes");
	gamenode->LinkEndChild(element);

	element = new TiXmlElement ( "EnableDescribtions" );
	element->SetAttribute ( "YN", "Yes");
	gamenode->LinkEndChild(element);

	element = new TiXmlElement ( "EnableDamageEffects" );
	element->SetAttribute ( "YN", "Yes");
	gamenode->LinkEndChild(element);

	element = new TiXmlElement ( "EnableDamageEffectsVehicles" );
	element->SetAttribute ( "YN", "Yes");
	gamenode->LinkEndChild(element);

	element = new TiXmlElement ( "EnableMakeTracks" );
	element->SetAttribute ( "YN", "Yes");
	gamenode->LinkEndChild(element);

	element = new TiXmlElement ( "ScrollSpeed" );
	element->SetAttribute ( "Num", "50");
	gamenode->LinkEndChild(element);

	element = new TiXmlElement ( "IP" );
	element->SetAttribute ( "Text", "127.0.0.1");
	netnode->LinkEndChild(element);

	element = new TiXmlElement ( "Port" );
	element->SetAttribute ( "Num", "58600");
	netnode->LinkEndChild(element);

	element = new TiXmlElement ( "PlayerName" );

	char * cHome;
	string sUser = "";

	#ifdef WIN32
		cHome = getenv("USERNAME");
	#elif __amigaos4__
		cHome = "AmigaOS4-User";
	#else
		cHome = getenv("USER"); //get $USER on linux
	#endif

	if(cHome != NULL)
	{
		sUser = cHome;
	}


	if(sUser.empty() != 1)
	{
		element->SetAttribute ( "Text", sUser.c_str());
	}
	else
	{
		element->SetAttribute ( "Text", "Commander");
	}

	element->SetAttribute ( "Num", "0"); //default playercolor

	netnode->LinkEndChild(element);

	element = new TiXmlElement ( "Enabled" );
	element->SetAttribute ( "YN", "Yes");
	soundnode->LinkEndChild(element);

	element = new TiXmlElement ( "MusicMute" );
	element->SetAttribute ( "YN", "No");
	soundnode->LinkEndChild(element);

	element = new TiXmlElement ( "SoundMute" );
	element->SetAttribute ( "YN", "No");
	soundnode->LinkEndChild(element);

	element = new TiXmlElement ( "VoiceMute" );
	element->SetAttribute ( "YN", "No");
	soundnode->LinkEndChild(element);

	element = new TiXmlElement ( "MusicVol" );
	element->SetAttribute ( "Num", "50");
	soundnode->LinkEndChild(element);

	element = new TiXmlElement ( "SoundVol" );
	element->SetAttribute ( "Num", "60");
	soundnode->LinkEndChild(element);

	element = new TiXmlElement ( "VoiceVol" );
	element->SetAttribute ( "Num", "70");
	soundnode->LinkEndChild(element);

	element = new TiXmlElement ( "ChunkSize" );
	element->SetAttribute ( "Num", "2048");
	soundnode->LinkEndChild(element);

	element = new TiXmlElement ( "Frequency" );
	element->SetAttribute ( "Num", "44100");
	soundnode->LinkEndChild(element);

	string sTmp ="";

	SettingsData.sDataDir = searchData();

	sTmp = SettingsData.sDataDir;
	element = new TiXmlElement ( "Gamedata" );
	element->SetAttribute ( "Text", sTmp.c_str());
	pathsnode->LinkEndChild(element);

	element = new TiXmlElement ( "Fonts" );
	element->SetAttribute ( "Text", "fonts");
	pathsnode->LinkEndChild(element);

	element = new TiXmlElement ( "FX" );
	element->SetAttribute ( "Text", "fx");
	pathsnode->LinkEndChild(element);

	element = new TiXmlElement ( "GFX" );
	element->SetAttribute ( "Text", "gfx");
	pathsnode->LinkEndChild(element);

	element = new TiXmlElement ( "Languages" );
	element->SetAttribute ( "Text", "languages");
	pathsnode->LinkEndChild(element);

	element = new TiXmlElement ( "Maps" );
	element->SetAttribute ( "Text", "maps");
	pathsnode->LinkEndChild(element);

	#ifdef WIN32
		sTmp = "save";
	#elif __amigaos4__
		sTmp = "save";
	#else
		sTmp = SettingsData.sHome;
		sTmp += "save";
	#endif
	element = new TiXmlElement ( "Saves" );
	element->SetAttribute ( "Text", sTmp.c_str());
	pathsnode->LinkEndChild(element);

	element = new TiXmlElement ( "Sounds" );
	element->SetAttribute ( "Text", "sounds");
	pathsnode->LinkEndChild(element);

	element = new TiXmlElement ( "Voices" );
	element->SetAttribute ( "Text", "voices");
	pathsnode->LinkEndChild(element);

	element = new TiXmlElement ( "Music" );
	element->SetAttribute ( "Text", "music");
	pathsnode->LinkEndChild(element);

	element = new TiXmlElement ( "Vehicles" );
	element->SetAttribute ( "Text", "vehicles");
	pathsnode->LinkEndChild(element);

	element = new TiXmlElement ( "Buildings" );
	element->SetAttribute ( "Text", "buildings");
	pathsnode->LinkEndChild(element);

	element = new TiXmlElement ( "MVEs" );
	element->SetAttribute ( "Text", "mve");
	pathsnode->LinkEndChild(element);

	gamenode->LinkEndChild(netnode);
	gamenode->LinkEndChild(soundnode);
	gamenode->LinkEndChild(pathsnode);

	ConfigDoc.RootElement()->LinkEndChild(gamenode);

	//END GAMENODE

	if(!ConfigDoc.SaveFile(SettingsData.sConfig.c_str())) //create new empty config
	{
		Log.write("Could not write new config to " + SettingsData.sConfig, cLog::eLOG_TYPE_ERROR);
		return -1; // Generate fails
	}
	else
	{
		return 0; // Generate success
	}
}

static int LoadEffects(const char* path)
{
	Log.write ( "Loading Effects", LOG_TYPE_INFO );

	LoadEffectGraphicToSurface ( EffectsData.fx_explo_small,path,"explo_small.pcx" );
	LoadEffectGraphicToSurface ( EffectsData.fx_explo_big,path,"explo_big.pcx" );
	LoadEffectGraphicToSurface ( EffectsData.fx_explo_water,path,"explo_water.pcx" );
	LoadEffectGraphicToSurface ( EffectsData.fx_explo_air,path,"explo_air.pcx" );
	LoadEffectGraphicToSurface ( EffectsData.fx_muzzle_big,path,"muzzle_big.pcx" );
	LoadEffectGraphicToSurface ( EffectsData.fx_muzzle_small,path,"muzzle_small.pcx" );
	LoadEffectGraphicToSurface ( EffectsData.fx_muzzle_med,path,"muzzle_med.pcx" );
	LoadEffectGraphicToSurface ( EffectsData.fx_hit,path,"hit.pcx" );
	LoadEffectAlphaToSurface ( EffectsData.fx_smoke,path,"smoke.pcx",100 );
	LoadEffectGraphicToSurface ( EffectsData.fx_rocket,path,"rocket.pcx" );
	LoadEffectAlphaToSurface ( EffectsData.fx_dark_smoke,path,"dark_smoke.pcx",100 );
	LoadEffectAlphaToSurface ( EffectsData.fx_tracks,path,"tracks.pcx",100 );
	LoadEffectAlphaToSurface ( EffectsData.fx_corpse,path,"corpse.pcx",255 );
	LoadEffectAlphaToSurface ( EffectsData.fx_absorb,path,"absorb.pcx",150 );

	return 1;
}

static int LoadMusic(const char* path)
{
	Log.write ( "Loading music", LOG_TYPE_INFO );
	string sTmpString;
	char sztmp[32];

	// Prepare music.xml for reading
	TiXmlDocument MusicXml;
	ExTiXmlNode * pXmlNode = NULL;
	sTmpString = path;
	sTmpString += PATH_DELIMITER;
	sTmpString += "music.xml";
	if(!FileExists(sTmpString.c_str()))
	{
		return 0;
	}
	if(!(MusicXml.LoadFile(sTmpString.c_str())))
	{
		Log.write ( "Can't load music.xml ", LOG_TYPE_ERROR );
		return 0;
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(MusicXml,"Music","Menus","main", NULL);
	if(!(pXmlNode->XmlReadNodeData(MainMusicFile,ExTiXmlNode::eXML_ATTRIBUTE,"Text")))
	{
		Log.write ( "Can't find \"main\" in music.xml ", LOG_TYPE_ERROR );
		return 0;
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(MusicXml,"Music","Menus","credits", NULL);
	if(!(pXmlNode->XmlReadNodeData(CreditsMusicFile,ExTiXmlNode::eXML_ATTRIBUTE,"Text")))
	{
		Log.write ( "Can't find \"credits\" in music.xml ", LOG_TYPE_ERROR );
		return 0;
	}


	pXmlNode = pXmlNode->XmlGetFirstNode(MusicXml,"Music","Game","bkgcount", NULL);
	if (!pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Num"))
	{
		Log.write ( "Can't find \"bkgcount\" in music.xml ", LOG_TYPE_ERROR );
		return 0;
	}
	int const MusicAnz = atoi(sTmpString.c_str());
	for ( int i=1;i <= MusicAnz; i++ )
	{
		sprintf ( sztmp,"%d",i );
		sTmpString = "bkg"; sTmpString += sztmp;
		pXmlNode = pXmlNode->XmlGetFirstNode(MusicXml,"Music","Game",sTmpString.c_str(), NULL);
		if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		{
			sTmpString.insert(0,PATH_DELIMITER);
			sTmpString.insert(0,path);
		}
		else
		{
			sTmpString = "Can't find \"bkg\" in music.xml";
			sprintf ( sztmp,"%d",i );
			sTmpString.insert(16,sztmp);
			Log.write ( sTmpString.c_str(), LOG_TYPE_WARNING );
			continue;
		}
		if(!FileExists(sTmpString.c_str()))
			continue;
		MusicFiles.Add ( sTmpString );
	}

	return 1;
}

static int LoadSounds(const char* path)
{
	Log.write ( "Loading Sounds", LOG_TYPE_INFO );

	LoadSoundfile ( SoundData.SNDHudSwitch, path, "HudSwitch.wav" );
	LoadSoundfile ( SoundData.SNDHudButton, path, "HudButton.wav" );
	LoadSoundfile ( SoundData.SNDMenuButton, path, "MenuButton.wav" );
	LoadSoundfile ( SoundData.SNDChat, path, "Chat.wav" );
	LoadSoundfile ( SoundData.SNDObjectMenu, path, "ObjectMenu.wav" );
	LoadSoundfile ( SoundData.EXPBigWet0, path, "exp_big_wet0.wav" );
	LoadSoundfile ( SoundData.EXPBigWet1, path, "exp_big_wet1.wav" );
	LoadSoundfile ( SoundData.EXPBig0, path, "exp_big0.wav" );
	LoadSoundfile ( SoundData.EXPBig1, path, "exp_big1.wav" );
	LoadSoundfile ( SoundData.EXPBig2, path, "exp_big2.wav" );
	LoadSoundfile ( SoundData.EXPBig3, path, "exp_big3.wav" );
	LoadSoundfile ( SoundData.EXPSmallWet0, path, "exp_small_wet0.wav" );
	LoadSoundfile ( SoundData.EXPSmallWet1, path, "exp_small_wet1.wav" );
	LoadSoundfile ( SoundData.EXPSmallWet2, path, "exp_small_wet2.wav" );
	LoadSoundfile ( SoundData.EXPSmall0, path, "exp_small0.wav" );
	LoadSoundfile ( SoundData.EXPSmall1, path, "exp_small1.wav" );
	LoadSoundfile ( SoundData.EXPSmall2, path, "exp_small2.wav" );
	LoadSoundfile ( SoundData.SNDArm, path, "arm.wav" );
	LoadSoundfile ( SoundData.SNDBuilding, path, "building.wav" );
	LoadSoundfile ( SoundData.SNDClearing, path, "clearing.wav" );
	LoadSoundfile ( SoundData.SNDQuitsch, path, "quitsch.wav" );
	LoadSoundfile ( SoundData.SNDActivate, path, "activate.wav" );
	LoadSoundfile ( SoundData.SNDLoad, path, "load.wav" );
	LoadSoundfile ( SoundData.SNDReload, path, "reload.wav" );
	LoadSoundfile ( SoundData.SNDRepair, path, "repair.wav" );
	LoadSoundfile ( SoundData.SNDLandMinePlace, path, "land_mine_place.wav" );
	LoadSoundfile ( SoundData.SNDLandMineClear, path, "land_mine_clear.wav" );
	LoadSoundfile ( SoundData.SNDSeaMinePlace, path, "sea_mine_place.wav" );
	LoadSoundfile ( SoundData.SNDSeaMineClear, path, "sea_mine_clear.wav" );
	LoadSoundfile ( SoundData.SNDPanelOpen, path, "panel_open.wav" );
	LoadSoundfile ( SoundData.SNDPanelClose, path, "panel_close.wav" );
	LoadSoundfile ( SoundData.SNDAbsorb, path, "absorb.wav" );

	return 1;
}

static int LoadVoices(const char* path)
{
	Log.write ( "Loading Voices", LOG_TYPE_INFO );

	LoadSoundfile ( VoiceData.VOINoPath1,path, "no_path1.wav" );
	LoadSoundfile ( VoiceData.VOINoPath2,path, "no_path2.wav" );
	LoadSoundfile ( VoiceData.VOIBuildDone1,path, "build_done1.wav" );
	LoadSoundfile ( VoiceData.VOIBuildDone2,path, "build_done2.wav" );
	LoadSoundfile ( VoiceData.VOINoSpeed,path, "no_speed.wav" );
	LoadSoundfile ( VoiceData.VOIStatusRed,path, "status_red.wav" );
	LoadSoundfile ( VoiceData.VOIStatusYellow,path, "status_yellow.wav" );
	LoadSoundfile ( VoiceData.VOIClearing,path, "clearing.wav" );
	LoadSoundfile ( VoiceData.VOILowAmmo1,path, "low_ammo1.wav" );
	LoadSoundfile ( VoiceData.VOILowAmmo2,path, "low_ammo2.wav" );
	LoadSoundfile ( VoiceData.VOIOK1,path, "ok1.wav" );
	LoadSoundfile ( VoiceData.VOIOK2,path, "ok2.wav" );
	LoadSoundfile ( VoiceData.VOIOK3,path, "ok3.wav" );
	LoadSoundfile ( VoiceData.VOIWachposten,path, "wachposten.wav" );
	LoadSoundfile ( VoiceData.VOITransferDone,path, "transfer_done.wav" );
	LoadSoundfile ( VoiceData.VOILoaded,path, "loaded.wav" );
	LoadSoundfile ( VoiceData.VOIRepaired,path, "repaired.wav" );
	LoadSoundfile ( VoiceData.VOILayingMines,path, "laying_mines.wav" );
	LoadSoundfile ( VoiceData.VOIClearingMines,path, "clearing_mines.wav" );
	LoadSoundfile ( VoiceData.VOIResearchComplete,path, "research_complete.wav" );
	LoadSoundfile ( VoiceData.VOIUnitStolen,path, "unit_stolen.wav" );
	LoadSoundfile ( VoiceData.VOIUnitDisabled,path, "unit_disabled.wav" );
	LoadSoundfile ( VoiceData.VOICommandoDetected,path, "commando_detected.wav" );
	LoadSoundfile ( VoiceData.VOIDisabled,path, "disabled.wav" );
	LoadSoundfile ( VoiceData.VOISaved,path, "saved.wav" );
	LoadSoundfile ( VoiceData.VOIStartNone,path, "start_none.wav" );
	LoadSoundfile ( VoiceData.VOIStartOne,path, "start_one.wav" );
	LoadSoundfile ( VoiceData.VOIStartMore,path, "start_more.wav" );
	LoadSoundfile ( VoiceData.VOIDetected1,path, "detected1.wav" );
	LoadSoundfile ( VoiceData.VOIDetected2,path, "detected2.wav" );
	LoadSoundfile ( VoiceData.VOIAttackingUs,path, "attacking_us.wav" );
	LoadSoundfile ( VoiceData.VOIDestroyedUs,path, "destroyed_us.wav" );

	return 1;
}

static int LoadGraphics(const char* path)
{
	Log.write ( "Loading Graphics", LOG_TYPE_INFO );
	string stmp;

	Log.write ( "Gamegraphics...", LOG_TYPE_DEBUG );
	if ( !LoadGraphicToSurface ( GraphicsData.gfx_Chand,path,"hand.pcx" ) ||
		!LoadGraphicToSurface ( GraphicsData.gfx_Cno,path,"no.pcx" ) ||
		!LoadGraphicToSurface ( GraphicsData.gfx_Cselect,path,"select.pcx" ) ||
		!LoadGraphicToSurface ( GraphicsData.gfx_Cmove,path,"move.pcx" ) ||
		!LoadGraphicToSurface ( GraphicsData.gfx_Chelp,path,"help.pcx" ) ||
		!LoadGraphicToSurface ( GraphicsData.gfx_Ctransf,path,"transf.pcx" ) ||
		!LoadGraphicToSurface ( GraphicsData.gfx_Cload,path,"load.pcx" ) ||
		!LoadGraphicToSurface ( GraphicsData.gfx_Cmuni,path,"muni.pcx" ) ||
		!LoadGraphicToSurface ( GraphicsData.gfx_Cband,path,"band_cur.pcx" ) ||
		!LoadGraphicToSurface ( GraphicsData.gfx_Cactivate,path,"activate.pcx" ) ||
		!LoadGraphicToSurface ( GraphicsData.gfx_Crepair,path,"repair.pcx" ) ||
		!LoadGraphicToSurface ( GraphicsData.gfx_Csteal,path,"steal.pcx" ) ||
		!LoadGraphicToSurface ( GraphicsData.gfx_Cdisable,path,"disable.pcx" ) ||
		!LoadGraphicToSurface ( GraphicsData.gfx_Cattack,path,"attack.pcx" ) ||
		!LoadGraphicToSurface ( GraphicsData.gfx_hud_stuff,path,"hud_stuff.pcx" ) ||
		!LoadGraphicToSurface ( GraphicsData.gfx_hud_extra_players,path,"hud_extra_players.pcx" ) ||
		!LoadGraphicToSurface ( GraphicsData.gfx_panel_top,path,"panel_top.pcx" ) ||
		!LoadGraphicToSurface ( GraphicsData.gfx_panel_bottom,path,"panel_bottom.pcx" ) ||
		!LoadGraphicToSurface ( GraphicsData.gfx_menu_stuff,path,"menu_stuff.pcx" ) )
	{
		return 0;
	}
	LoadGraphicToSurface ( GraphicsData.gfx_Cpfeil1,path,"pf_1.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_Cpfeil2,path,"pf_2.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_Cpfeil3,path,"pf_3.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_Cpfeil4,path,"pf_4.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_Cpfeil6,path,"pf_6.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_Cpfeil7,path,"pf_7.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_Cpfeil8,path,"pf_8.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_Cpfeil9,path,"pf_9.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_context_menu,path,"object_menu2.pcx");
	LoadGraphicToSurface ( GraphicsData.gfx_destruction,path,"destruction.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_band_small_org,path,"band_small.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_band_small,path,"band_small.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_band_big_org,path,"band_big.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_band_big,path,"band_big.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_big_beton_org,path,"big_beton.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_big_beton,path,"big_beton.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_storage,path,"storage.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_storage_ground,path,"storage_ground.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_dialog,path,"dialog.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_edock,path,"edock.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_edepot,path,"edepot.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_ehangar,path,"ehangar.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_player_pc,path,"player_pc.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_player_human,path,"player_human.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_player_none,path,"player_none.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_exitpoints_org,path,"activate_field.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_exitpoints,path,"activate_field.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_player_select,path,"customgame_menu.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_menu_buttons,path,"menu_buttons.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_player_ready,path,"player_ready.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_hud_chatbox,path,"hud_chatbox.pcx" );

	GraphicsData.DialogPath = SettingsData.sGfxPath + PATH_DELIMITER + "dialog.pcx";
	GraphicsData.Dialog2Path = SettingsData.sGfxPath + PATH_DELIMITER + "dialog2.pcx";
	GraphicsData.Dialog3Path = SettingsData.sGfxPath + PATH_DELIMITER + "dialog3.pcx";
	FileExists(GraphicsData.DialogPath.c_str());
	FileExists(GraphicsData.Dialog2Path.c_str());
	FileExists(GraphicsData.Dialog3Path.c_str());

	// Hud:
	Log.write ( "Hudgraphics...", LOG_TYPE_DEBUG );
	SDL_Rect scr,dest;
	GraphicsData.gfx_hud = SDL_CreateRGBSurface ( OtherData.iSurface, SettingsData.iScreenW,
		SettingsData.iScreenH, SettingsData.iColourDepth, 0, 0, 0, 0 );
	SDL_FillRect ( GraphicsData.gfx_hud, NULL, 0xFF00FF );
	SDL_SetColorKey ( GraphicsData.gfx_hud, SDL_SRCCOLORKEY, 0xFF00FF );

	if( !LoadGraphicToSurface ( GraphicsData.gfx_tmp, path, "hud_left.pcx" ))
		return 0;
	SDL_BlitSurface ( GraphicsData.gfx_tmp, NULL, GraphicsData.gfx_hud, NULL );
	SDL_FreeSurface ( GraphicsData.gfx_tmp );

	if( !LoadGraphicToSurface ( GraphicsData.gfx_tmp,path,"hud_top.pcx" ))
		return 0;
	scr.x = 0;
	scr.y=0;
	scr.w = GraphicsData.gfx_tmp->w;
	scr.h = GraphicsData.gfx_tmp->h;
	dest.x = 180;
	dest.y = 0;
	SDL_BlitSurface ( GraphicsData.gfx_tmp, &scr, GraphicsData.gfx_hud, &dest );
	scr.x = 1275;
	scr.w = 18;
	scr.h = 18;
	dest.x = GraphicsData.gfx_hud->w-18;
	SDL_BlitSurface ( GraphicsData.gfx_tmp, &scr, GraphicsData.gfx_hud, &dest );
	SDL_FreeSurface ( GraphicsData.gfx_tmp );

	if( !LoadGraphicToSurface ( GraphicsData.gfx_tmp, path, "hud_right.pcx" ))
		return 0;
	scr.x = 0;
	scr.y = 0;
	scr.w = GraphicsData.gfx_tmp->w;
	scr.h = GraphicsData.gfx_tmp->h;
	dest.x = GraphicsData.gfx_hud->w-12;
	dest.y = 18;
	SDL_BlitSurface ( GraphicsData.gfx_tmp, &scr, GraphicsData.gfx_hud,&dest );
	SDL_FreeSurface ( GraphicsData.gfx_tmp );

	if( !LoadGraphicToSurface ( GraphicsData.gfx_tmp,path,"hud_bottom.pcx" ))
		return 0;
	scr.x = 0;
	scr.y = 0;
	scr.w = GraphicsData.gfx_tmp->w;
	scr.h = GraphicsData.gfx_tmp->h;
	dest.x = 180;
	dest.y = GraphicsData.gfx_hud->h-24;
	SDL_BlitSurface ( GraphicsData.gfx_tmp,&scr,GraphicsData.gfx_hud,&dest );
	scr.x = 1275;
	scr.w = 23;
	scr.h = 24;
	dest.x = GraphicsData.gfx_hud->w-23;
	SDL_BlitSurface ( GraphicsData.gfx_tmp,&scr,GraphicsData.gfx_hud,&dest );
	scr.x = 1299;
	scr.w = 16;
	scr.h = 22;
	dest.x = 180-16;
	dest.y = GraphicsData.gfx_hud->h-22;
	SDL_BlitSurface ( GraphicsData.gfx_tmp,&scr,GraphicsData.gfx_hud,&dest );
	SDL_FreeSurface ( GraphicsData.gfx_tmp );

	if ( SettingsData.iScreenH > 480 )
	{
		if( !LoadGraphicToSurface ( GraphicsData.gfx_tmp, path, "logo.pcx" ))
			return 0;
		dest.x = 9;
		dest.y = SettingsData.iScreenH-32-15;
		SDL_BlitSurface ( GraphicsData.gfx_tmp,NULL,GraphicsData.gfx_hud,&dest );
		SDL_FreeSurface ( GraphicsData.gfx_tmp );
	}

	Log.write ( "Colourgraphics...", LOG_TYPE_DEBUG );
	// Farben:
	OtherData.colors = new SDL_Surface*[PLAYERCOLORS];
	if(!OtherData.colors) { Log.write("Out of memory", cLog::eLOG_TYPE_MEM); }
	LoadGraphicToSurface ( OtherData.colors[cl_red],path,"cl_red.pcx" );
	LoadGraphicToSurface ( OtherData.colors[cl_blue],path,"cl_blue.pcx" );
	LoadGraphicToSurface ( OtherData.colors[cl_green],path,"cl_green.pcx" );
	LoadGraphicToSurface ( OtherData.colors[cl_grey],path,"cl_grey.pcx" );
	LoadGraphicToSurface ( OtherData.colors[cl_orange],path,"cl_orange.pcx" );
	LoadGraphicToSurface ( OtherData.colors[cl_yellow],path,"cl_yellow.pcx" );
	LoadGraphicToSurface ( OtherData.colors[cl_purple],path,"cl_purple.pcx" );
	LoadGraphicToSurface ( OtherData.colors[cl_aqua],path,"cl_aqua.pcx" );

	OtherData.colors_org = new SDL_Surface*[PLAYERCOLORS];
	if(!OtherData.colors) { Log.write("Out of memory", cLog::eLOG_TYPE_MEM); }
	LoadGraphicToSurface ( OtherData.colors_org[cl_red],path,"cl_red.pcx" );
	LoadGraphicToSurface ( OtherData.colors_org[cl_blue],path,"cl_blue.pcx" );
	LoadGraphicToSurface ( OtherData.colors_org[cl_green],path,"cl_green.pcx" );
	LoadGraphicToSurface ( OtherData.colors_org[cl_grey],path,"cl_grey.pcx" );
	LoadGraphicToSurface ( OtherData.colors_org[cl_orange],path,"cl_orange.pcx" );
	LoadGraphicToSurface ( OtherData.colors_org[cl_yellow],path,"cl_yellow.pcx" );
	LoadGraphicToSurface ( OtherData.colors_org[cl_purple],path,"cl_purple.pcx" );
	LoadGraphicToSurface ( OtherData.colors_org[cl_aqua],path,"cl_aqua.pcx" );


	Log.write ( "Shadowgraphics...", LOG_TYPE_DEBUG );
	// Shadow:
	GraphicsData.gfx_shadow = SDL_CreateRGBSurface ( OtherData.iSurface, SettingsData.iScreenW,
		SettingsData.iScreenH, SettingsData.iColourDepth, 0, 0, 0, 0 );
	SDL_FillRect ( GraphicsData.gfx_shadow, NULL, 0x0 );
	SDL_SetAlpha ( GraphicsData.gfx_shadow, SDL_SRCALPHA, 50 );
	GraphicsData.gfx_tmp = SDL_CreateRGBSurface ( OtherData.iSurface, 128, 128, SettingsData.iColourDepth, 0, 0, 0, 0 );
	SDL_SetColorKey ( GraphicsData.gfx_tmp, SDL_SRCCOLORKEY, 0xFF00FF );

	// Glas:
	Log.write ( "Glassgraphic...", LOG_TYPE_DEBUG );
	LoadGraphicToSurface ( GraphicsData.gfx_destruction_glas, path, "destruction_glas.pcx" );
	SDL_SetAlpha ( GraphicsData.gfx_destruction_glas, SDL_SRCALPHA, 150 );

	// Waypoints:
	Log.write ( "Waypointgraphics...", LOG_TYPE_DEBUG );
	for ( int i = 0; i < 60; i++ )
	{
		OtherData.WayPointPfeile[0][i] = CreatePfeil ( 26,11,51,36,14,48,PFEIL_COLOR,64-i );
		OtherData.WayPointPfeile[1][i] = CreatePfeil ( 14,14,49,14,31,49,PFEIL_COLOR,64-i );
		OtherData.WayPointPfeile[2][i] = CreatePfeil ( 37,11,12,36,49,48,PFEIL_COLOR,64-i );
		OtherData.WayPointPfeile[3][i] = CreatePfeil ( 49,14,49,49,14,31,PFEIL_COLOR,64-i );
		OtherData.WayPointPfeile[4][i] = CreatePfeil ( 14,14,14,49,49,31,PFEIL_COLOR,64-i );
		OtherData.WayPointPfeile[5][i] = CreatePfeil ( 15,14,52,26,27,51,PFEIL_COLOR,64-i );
		OtherData.WayPointPfeile[6][i] = CreatePfeil ( 31,14,14,49,49,49,PFEIL_COLOR,64-i );
		OtherData.WayPointPfeile[7][i] = CreatePfeil ( 48,14,36,51,11,26,PFEIL_COLOR,64-i );

		OtherData.WayPointPfeileSpecial[0][i] = CreatePfeil ( 26,11,51,36,14,48,PFEILS_COLOR,64-i );
		OtherData.WayPointPfeileSpecial[1][i] = CreatePfeil ( 14,14,49,14,31,49,PFEILS_COLOR,64-i );
		OtherData.WayPointPfeileSpecial[2][i] = CreatePfeil ( 37,11,12,36,49,48,PFEILS_COLOR,64-i );
		OtherData.WayPointPfeileSpecial[3][i] = CreatePfeil ( 49,14,49,49,14,31,PFEILS_COLOR,64-i );
		OtherData.WayPointPfeileSpecial[4][i] = CreatePfeil ( 14,14,14,49,49,31,PFEILS_COLOR,64-i );
		OtherData.WayPointPfeileSpecial[5][i] = CreatePfeil ( 15,14,52,26,27,51,PFEILS_COLOR,64-i );
		OtherData.WayPointPfeileSpecial[6][i] = CreatePfeil ( 31,14,14,49,49,49,PFEILS_COLOR,64-i );
		OtherData.WayPointPfeileSpecial[7][i] = CreatePfeil ( 48,14,36,51,11,26,PFEILS_COLOR,64-i );
	}

	// Resources:
	Log.write ( "Resourcegraphics...", LOG_TYPE_DEBUG );
	//metal
	if ( LoadGraphicToSurface ( ResourceData.res_metal_org,path,"res.pcx" ) == 1 )
	{
		LoadGraphicToSurface ( ResourceData.res_metal,path,"res.pcx" );
		SDL_SetColorKey ( ResourceData.res_metal,SDL_SRCCOLORKEY,0xFF00FF );
	}

	//fuel
	if ( LoadGraphicToSurface ( ResourceData.res_gold_org,path,"gold.pcx" ) == 1 )
	{
		LoadGraphicToSurface ( ResourceData.res_gold,path,"gold.pcx" );
		SDL_SetColorKey ( ResourceData.res_gold,SDL_SRCCOLORKEY,0xFF00FF );
	}

	//fuel
	if ( LoadGraphicToSurface ( ResourceData.res_oil_org,path,"fuel.pcx" ) == 1 )
	{
		LoadGraphicToSurface ( ResourceData.res_oil,path,"fuel.pcx" );
		SDL_SetColorKey ( ResourceData.res_oil,SDL_SRCCOLORKEY,0xFF00FF );
	}

	return 1;
}

static int LoadVehicles()
{
	Log.write ( "Loading Vehicles", LOG_TYPE_INFO );

	string sTmpString, sVehiclePath;
	char sztmp[16];
	const char *pszTmp;
	TiXmlDocument VehiclesXml;
	TiXmlNode *pXmlNode;
	TiXmlElement * pXmlElement;

	sTmpString = SettingsData.sVehiclesPath;
	sTmpString += PATH_DELIMITER;
	sTmpString += "vehicles.xml";
	if( !FileExists( sTmpString.c_str() ) )
	{
		return 0;
	}
	if ( !VehiclesXml.LoadFile ( sTmpString.c_str() ) )
	{
		Log.write("Can't load vehicles.xml!",LOG_TYPE_ERROR);
		return 0;
	}
	if(!(pXmlNode = VehiclesXml.FirstChildElement ( "VehicleData" )->FirstChildElement ( "Vehicles" )))
	{
		Log.write("Can't read \"VehicleData->Vehicles\" node!",LOG_TYPE_ERROR);
		return 0;
	}
	// read vehicles.xml
	cList<string> VehicleList;
	cList<string> IDList;
	pXmlNode = pXmlNode->FirstChildElement();
	pXmlElement = pXmlNode->ToElement();
	if ( pXmlElement )
	{
		pszTmp = pXmlElement->Attribute( "directory" );
		if(pszTmp != 0)
			VehicleList.Add(pszTmp);
		else
		{
			sTmpString = "Can't read dierectory-attribute from \"\" - node";
			sTmpString.insert(38,pXmlNode->Value());
			Log.write(sTmpString.c_str(),LOG_TYPE_WARNING);
		}
		pszTmp = pXmlElement->Attribute( "num" );
		if(pszTmp != 0)
			IDList.Add(pszTmp);
		else
		{
			VehicleList.Delete(VehicleList.Size());
			sTmpString = "Can't read num-attribute from \"\" - node";
			sTmpString.insert(32,pXmlNode->Value());
			Log.write(sTmpString.c_str(),LOG_TYPE_WARNING);
		}
	}
	else
		Log.write("No vehicles defined in vehicles.xml!",LOG_TYPE_WARNING);
	while ( pXmlNode != NULL)
	{
		pXmlNode = pXmlNode->NextSibling();
		if ( pXmlNode == NULL)
			break;
		if( pXmlNode->Type() !=1 )
			continue;
		pszTmp = pXmlNode->ToElement()->Attribute( "directory" );
		if(pszTmp != 0)
			VehicleList.Add(pszTmp);
		else
		{
			sTmpString = "Can't read dierectory-attribute from \"\" - node";
			sTmpString.insert(38,pXmlNode->Value());
			Log.write(sTmpString.c_str(),LOG_TYPE_WARNING);
		}
		pszTmp = pXmlNode->ToElement()->Attribute( "num" );
		if(pszTmp != 0)
			IDList.Add(pszTmp);
		else
		{
			VehicleList.Delete(VehicleList.Size());
			sTmpString = "Can't read num-attribute from \"\" - node";
			sTmpString.insert(32,pXmlNode->Value());
			Log.write(sTmpString.c_str(),LOG_TYPE_WARNING);
		}
	}
	// load found units
	UnitsData.vehicle.Reserve(0);
	for ( unsigned int i = 0; i < VehicleList.Size(); i++)
	{
		sVehiclePath = SettingsData.sVehiclesPath;
		sVehiclePath += PATH_DELIMITER;
		sVehiclePath += VehicleList[i];
		sVehiclePath += PATH_DELIMITER;

		// Prepare memory for next unit
		UnitsData.vehicle.Add( sVehicle() );
		sVehicle &v = UnitsData.vehicle[i];

		Log.write("Reading values from XML", cLog::eLOG_TYPE_DEBUG);
		LoadUnitData(&v.data, sVehiclePath.c_str(), atoi(IDList[i].c_str()));
		translateUnitData(v.data.ID, true);

		Log.write("Loading graphics", cLog::eLOG_TYPE_DEBUG);

		// laod infantery graphics
		if (v.data.animationMovement)
		{
			SDL_Rect rcDest;
			for (int n = 0; n < 8; n++)
			{
				v.img[n] = SDL_CreateRGBSurface (OtherData.iSurface | SDL_SRCCOLORKEY, 64 * 13, 64, SettingsData.iColourDepth, 0, 0, 0, 0);
				SDL_SetColorKey(v.img[n], SDL_SRCCOLORKEY, 0xFFFFFF);
				SDL_FillRect(v.img[n], NULL, 0xFF00FF);

				for ( int j = 0; j < 13; j++ )
				{
					sTmpString = sVehiclePath;
					sprintf(sztmp, "img%d_%.2d.pcx", n, j);
					sTmpString += sztmp;



					if(FileExists(sTmpString.c_str()))
					{
						AutoSurface sfTempSurface(LoadPCX(sTmpString.c_str()));
						if(!sfTempSurface)
						{
							Log.write(SDL_GetError(), cLog::eLOG_TYPE_WARNING);
						}
						else
						{
							rcDest.x = 64*j + 32 - sfTempSurface->w/2;
							rcDest.y = 32 - sfTempSurface->h/2;
							SDL_BlitSurface(sfTempSurface, NULL, v.img[n], &rcDest);
						}
					}
				}
				v.img_org[n] = SDL_CreateRGBSurface ( OtherData.iSurface | SDL_SRCCOLORKEY, 64*13, 64, SettingsData.iColourDepth, 0, 0, 0, 0 );
				SDL_SetColorKey(v.img[n], SDL_SRCCOLORKEY, 0xFFFFFF);
				SDL_FillRect(v.img_org[n], NULL, 0xFFFFFF);
				SDL_BlitSurface(v.img[n], NULL, v.img_org[n], NULL);

				v.shw[n] = SDL_CreateRGBSurface(OtherData.iSurface | SDL_SRCCOLORKEY, 64 * 13, 64, SettingsData.iColourDepth, 0, 0, 0, 0);
				SDL_SetColorKey(v.shw[n], SDL_SRCCOLORKEY, 0xFF00FF);
				SDL_FillRect(v.shw[n], NULL, 0xFF00FF);
				v.shw_org[n] = SDL_CreateRGBSurface(OtherData.iSurface | SDL_SRCCOLORKEY, 64 * 13, 64, SettingsData.iColourDepth, 0, 0, 0, 0);
				SDL_SetColorKey(v.shw_org[n], SDL_SRCCOLORKEY, 0xFF00FF);
				SDL_FillRect(v.shw_org[n], NULL, 0xFF00FF);

				int *ptr;
				rcDest.x=3;
				rcDest.y=3;
				SDL_BlitSurface(v.img_org[n], NULL, v.shw_org[n], &rcDest);
				SDL_LockSurface(v.shw_org[n]);
				ptr = (int*)(v.shw_org[n]->pixels);
				for ( int j = 0; j < 64*13*64; j++ )
				{
					if ( *ptr != 0xFF00FF )
						*ptr=0;
					ptr++;
				}
				SDL_UnlockSurface(v.shw_org[n]);
				SDL_BlitSurface(v.shw_org[n], NULL, v.shw[n], NULL);
				SDL_SetAlpha(v.shw_org[n], SDL_SRCALPHA, 50);
				SDL_SetAlpha(v.shw[n], SDL_SRCALPHA, 50);
			}
		}
		// load other vehicle graphics
		else
		{
			for(int n = 0; n < 8; n++)
			{
				// load image
				sTmpString = sVehiclePath;
				sprintf(sztmp,"img%d.pcx",n);
				sTmpString += sztmp;
				Log.write(sTmpString, cLog::eLOG_TYPE_DEBUG);
				if(FileExists(sTmpString.c_str()))
				{
					v.img_org[n] = LoadPCX(sTmpString.c_str());
					SDL_SetColorKey(v.img_org[n], SDL_SRCCOLORKEY, 0xFFFFFF);
					v.img[n] = LoadPCX(sTmpString.c_str());
					SDL_SetColorKey(v.img[n], SDL_SRCCOLORKEY, 0xFFFFFF);
				}
				else
				{
					Log.write("Missing GFX - your MAXR install seems to be incomplete!", cLog::eLOG_TYPE_ERROR);
					return -1;
				}

				// load shadow
				sTmpString.replace(sTmpString.length()-8,3,"shw");
				if(FileExists(sTmpString.c_str()))
				{
					v.shw_org[n] = LoadPCX(sTmpString.c_str());
					v.shw[n] = LoadPCX(sTmpString.c_str());
					SDL_SetAlpha(v.shw[n], SDL_SRCALPHA, 50);
				}
				else
				{
					v.shw_org[n] = NULL;
					v.shw[n]     = NULL;
				}
			}
		}
		// load video
		sTmpString = sVehiclePath;
		sTmpString += "video.flc";
		Log.write("Loading video " + sTmpString, cLog::eLOG_TYPE_DEBUG);
		if( !FileExists(sTmpString.c_str()))
		{
			sTmpString = "";
		}
		v.FLCFile = new char[sTmpString.length()+1];
		if (!v.FLCFile) { Log.write("Out of memory", cLog::eLOG_TYPE_MEM); }
		strcpy(v.FLCFile, sTmpString.c_str());


		// load infoimage
		sTmpString = sVehiclePath;
		sTmpString += "info.pcx";
		Log.write("Loading portrait" + sTmpString, cLog::eLOG_TYPE_DEBUG);
		if(FileExists(sTmpString.c_str()))
		{
			v.info = LoadPCX(sTmpString.c_str());
		}
		else
		{
			Log.write("Missing GFX - your MAXR install seems to be incomplete!", cLog::eLOG_TYPE_ERROR);
			return -1;
		}

		// load storageimage
		sTmpString = sVehiclePath;
		sTmpString += "store.pcx";
		Log.write("Loading storageportrait" +sTmpString, cLog::eLOG_TYPE_DEBUG);
		if(FileExists(sTmpString.c_str()))
		{
			v.storage = LoadPCX(sTmpString.c_str());
		}
		else
		{
			Log.write("Missing GFX - your MAXR install seems to be incomplete!", cLog::eLOG_TYPE_ERROR);
			return -1;
		}

		// load overlaygraphics if necessary
		Log.write("Loading overlay", cLog::eLOG_TYPE_DEBUG);
		if (v.data.hasOverlay)
		{
			sTmpString = sVehiclePath;
			sTmpString += "overlay.pcx";
			if(FileExists(sTmpString.c_str()))
			{
				v.overlay_org = LoadPCX(sTmpString.c_str());
				v.overlay     = LoadPCX(sTmpString.c_str());
			}
			else
			{
				Log.write("Missing GFX - your MAXR install seems to be incomplete!", cLog::eLOG_TYPE_WARNING);
				v.overlay_org       = NULL;
				v.overlay           = NULL;
				v.data.hasOverlay = false;
			}
		}
		else
		{
			v.overlay_org = NULL;
			v.overlay     = NULL;
		}

		// load buildgraphics if necessary
		Log.write("Loading buildgraphics", cLog::eLOG_TYPE_DEBUG);
		if (v.data.buildUpGraphic)
		{
			// load image
			sTmpString = sVehiclePath;
			sTmpString += "build.pcx";
			if(FileExists(sTmpString.c_str()))
			{
				v.build_org = LoadPCX(sTmpString.c_str());
				SDL_SetColorKey(v.build_org, SDL_SRCCOLORKEY, 0xFFFFFF);
				v.build = LoadPCX(sTmpString.c_str());
				SDL_SetColorKey(v.build, SDL_SRCCOLORKEY, 0xFFFFFF);
			}
			else
			{
				Log.write("Missing GFX - your MAXR install seems to be incomplete!", cLog::eLOG_TYPE_WARNING);
				v.build_org             = NULL;
				v.build                 = NULL;
				v.data.buildUpGraphic = false;
			}
			// load shadow
			sTmpString = sVehiclePath;
			sTmpString += "build_shw.pcx";
			if(FileExists(sTmpString.c_str()))
			{
				v.build_shw_org = LoadPCX(sTmpString.c_str());
				v.build_shw     = LoadPCX(sTmpString.c_str());
				SDL_SetAlpha(v.build_shw, SDL_SRCALPHA, 50);
			}
			else
			{
				Log.write("Missing GFX - your MAXR install seems to be incomplete!", cLog::eLOG_TYPE_WARNING);
				v.build_shw_org         = NULL;
				v.build_shw             = NULL;
				v.data.buildUpGraphic = false;
			}
		}
		else
		{
			v.build_org     = NULL;
			v.build         = NULL;
			v.build_shw_org = NULL;
			v.build_shw     = NULL;
		}
		// load cleargraphics if necessary
		Log.write("Loading cleargraphics", cLog::eLOG_TYPE_DEBUG);
		if (v.data.canClearArea)
		{
			// load image (small)
			sTmpString = sVehiclePath;
			sTmpString += "clear_small.pcx";
			if(FileExists(sTmpString.c_str()))
			{
				v.clear_small_org = LoadPCX(sTmpString.c_str());
				SDL_SetColorKey(v.clear_small_org, SDL_SRCCOLORKEY, 0xFFFFFF);
				v.clear_small = LoadPCX(sTmpString.c_str());
				SDL_SetColorKey(v.clear_small, SDL_SRCCOLORKEY, 0xFFFFFF);
			}
			else
			{
				Log.write("Missing GFX - your MAXR install seems to be incomplete!", cLog::eLOG_TYPE_WARNING);
				v.clear_small_org      = NULL;
				v.clear_small          = NULL;
				v.data.canClearArea = false;
			}
			// load shadow (small)
			sTmpString = sVehiclePath;
			sTmpString += "clear_small_shw.pcx";
			if(FileExists(sTmpString.c_str()))
			{
				v.clear_small_shw_org = LoadPCX(sTmpString.c_str());
				v.clear_small_shw     = LoadPCX(sTmpString.c_str());
				SDL_SetAlpha(v.clear_small_shw, SDL_SRCALPHA, 50);
			}
			else
			{
				Log.write("Missing GFX - your MAXR install seems to be incomplete!", cLog::eLOG_TYPE_WARNING);
				v.clear_small_shw_org  = NULL;
				v.clear_small_shw      = NULL;
				v.data.canClearArea = false;
			}
			// load image (big)
			sTmpString = sVehiclePath;
			sTmpString += "clear_big.pcx";
			if(FileExists(sTmpString.c_str()))
			{
				v.build_org = LoadPCX(sTmpString.c_str());
				SDL_SetColorKey(v.build_org, SDL_SRCCOLORKEY, 0xFFFFFF);
				v.build = LoadPCX(sTmpString.c_str());
				SDL_SetColorKey(v.build, SDL_SRCCOLORKEY, 0xFFFFFF);
			}
			else
			{
				Log.write("Missing GFX - your MAXR install seems to be incomplete!", cLog::eLOG_TYPE_WARNING);
				v.build_org            = NULL;
				v.build                = NULL;
				v.data.canClearArea = false;
			}
			// load shadow (big)
			sTmpString = sVehiclePath;
			sTmpString += "clear_big_shw.pcx";
			if(FileExists(sTmpString.c_str()))
			{
				v.build_shw_org = LoadPCX(sTmpString.c_str());
				v.build_shw     = LoadPCX(sTmpString.c_str());
				SDL_SetAlpha(v.build_shw, SDL_SRCALPHA, 50);
			}
			else
			{
				Log.write("Missing GFX - your MAXR install seems to be incomplete!", cLog::eLOG_TYPE_WARNING);
				v.build_shw_org        = NULL;
				v.build_shw            = NULL;
				v.data.canClearArea = false;
			}
		}
		else
		{
			v.clear_small_org     = NULL;
			v.clear_small         = NULL;
			v.clear_small_shw_org = NULL;
			v.clear_small_shw     = NULL;
		}

		// load sounds
		Log.write("Loading sounds", cLog::eLOG_TYPE_DEBUG);
		LoadUnitSoundfile(v.Wait,       sVehiclePath.c_str(), "wait.wav");
		LoadUnitSoundfile(v.WaitWater,  sVehiclePath.c_str(), "wait_water.wav");
		LoadUnitSoundfile(v.Start,      sVehiclePath.c_str(), "start.wav");
		LoadUnitSoundfile(v.StartWater, sVehiclePath.c_str(), "start_water.wav");
		LoadUnitSoundfile(v.Stop,       sVehiclePath.c_str(), "stop.wav");
		LoadUnitSoundfile(v.StopWater,  sVehiclePath.c_str(), "stop_water.wav");
		LoadUnitSoundfile(v.Drive,      sVehiclePath.c_str(), "drive.wav");
		LoadUnitSoundfile(v.DriveWater, sVehiclePath.c_str(), "drive_water.wav");
		LoadUnitSoundfile(v.Attack,     sVehiclePath.c_str(), "attack.wav");
	}

	for (unsigned int i = 0 ; i < UnitsData.vehicle.Size(); ++i) UnitsData.vehicle[i].nr = (int)i;
	return 1;
}

void translateUnitData(sID ID, bool vehicle)
{
	sUnitData *Data = NULL;
	TiXmlNode * pXmlNode = NULL;
	string sTmpString;

	if ( vehicle )
	{
		for (size_t i = 0; i <= UnitsData.vehicle.Size(); ++i)
		{
			if ( UnitsData.vehicle[i].data.ID.iFirstPart == ID.iFirstPart && UnitsData.vehicle[i].data.ID.iSecondPart == ID.iSecondPart )
			{
				Data = &UnitsData.vehicle[i].data;
				break;
			}
		}
	}
	else
	{
		for (size_t i = 0; i <= UnitsData.building.Size(); ++i)
		{
			if ( UnitsData.building[i].data.ID.iFirstPart == ID.iFirstPart && UnitsData.building[i].data.ID.iSecondPart == ID.iSecondPart )
			{
				Data = &UnitsData.building[i].data;
				break;
			}
		}
	}
	if ( Data == NULL ) return;
	pXmlNode = LanguageFile.FirstChild( "MAX_Language_File" )->FirstChildElement( "Units" );
	pXmlNode = pXmlNode->FirstChildElement();
	while ( pXmlNode != NULL)
	{
		sTmpString = pXmlNode->ToElement()->Attribute( "ID" );
		if( atoi( sTmpString.substr( 0,sTmpString.find( " ",0 ) ).c_str() ) == ID.iFirstPart && atoi( sTmpString.substr( sTmpString.find( " ",0 ),sTmpString.length() ).c_str() ) == ID.iSecondPart )
		{
			if( SettingsData.sLanguage.compare ( "ENG" ) != 0 ) Data->name = pXmlNode->ToElement()->Attribute( "localized" );
			else Data->name = pXmlNode->ToElement()->Attribute( "ENG" );

			sTmpString = pXmlNode->ToElement()->GetText();
			int iPosition = (int)sTmpString.find("\\n",0);
			while(iPosition != string::npos)
			{
				sTmpString.replace( iPosition, 2, "\n" );
				iPosition = (int)sTmpString.find("\\n",iPosition);
			}
			Data->description =  sTmpString;
		}
		pXmlNode = pXmlNode->NextSibling();
	}
}

static int LoadBuildings()
{
	Log.write ( "Loading Buildings", LOG_TYPE_INFO );

	string sTmpString, sBuildingPath;
	const char *pszTmp;
	TiXmlDocument BuildingsXml;
	TiXmlNode *pXmlNode;
	TiXmlElement * pXmlElement;

	// read buildings.xml
	sTmpString = SettingsData.sBuildingsPath;
	sTmpString += PATH_DELIMITER;
	sTmpString += "buildings.xml";
	if( !FileExists( sTmpString.c_str() ) )
	{
		return 0;
	}
	if ( !BuildingsXml.LoadFile ( sTmpString.c_str() ) )
	{
		Log.write("Can't load buildings.xml!",LOG_TYPE_ERROR);
		return 0;
	}
	if(!(pXmlNode = BuildingsXml.FirstChildElement ( "BuildingsData" )->FirstChildElement ( "Buildings" )))
	{
		Log.write("Can't read \"BuildingData->Building\" node!",LOG_TYPE_ERROR);
		return 0;
	}
	cList<string> BuildingList;
	cList<string> IDList;
	pXmlNode = pXmlNode->FirstChildElement();
	if ( !pXmlNode )
	{
		Log.write("There are no buildings in the buildings.xml defined",LOG_TYPE_ERROR);
		return 1;
	}
	pXmlElement = pXmlNode->ToElement();
	if ( pXmlElement )
	{
		pszTmp = pXmlElement->Attribute( "directory" );
		if(pszTmp != 0)
			BuildingList.Add(pszTmp);
		else
		{
			sTmpString = "Can't read dierectory-attribute from \"\" - node";
			sTmpString.insert(38,pXmlNode->Value());
			Log.write(sTmpString.c_str(),LOG_TYPE_WARNING);
		}
		pszTmp = pXmlElement->Attribute( "num" );
		if(pszTmp != 0)
			IDList.Add(pszTmp);
		else
		{
			BuildingList.Delete(BuildingList.Size());
			sTmpString = "Can't read num-attribute from \"\" - node";
			sTmpString.insert(32,pXmlNode->Value());
			Log.write(sTmpString.c_str(),LOG_TYPE_WARNING);
		}

		pszTmp = pXmlNode->ToElement()->Attribute( "special" );
		if ( pszTmp != 0 )
		{
			string specialString = pszTmp;
			if ( specialString.compare ( "mine" ) == 0 ) specialIDMine.iSecondPart = atoi ( IDList[IDList.Size()-1].c_str() );
			else if ( specialString.compare ( "energy" ) == 0 ) specialIDSmallGen.iSecondPart = atoi ( IDList[IDList.Size()-1].c_str() );
			else if ( specialString.compare ( "connector" ) == 0 ) specialIDConnector.iSecondPart = atoi ( IDList[IDList.Size()-1].c_str() );
			else if ( specialString.compare ( "landmine" ) == 0 ) specialIDLandMine.iSecondPart = atoi ( IDList[IDList.Size()-1].c_str() );
			else if ( specialString.compare ( "seamine" ) == 0 ) specialIDSeaMine.iSecondPart = atoi ( IDList[IDList.Size()-1].c_str() );
			else if ( specialString.compare ( "smallBeton" ) == 0 ) specialIDSmallBeton.iSecondPart = atoi ( IDList[IDList.Size()-1].c_str() );
			else Log.write ( "Unknown spacial in buildings.xml \"" + specialString + "\"", LOG_TYPE_WARNING );
		}
	}
	else
		Log.write("No buildings defined in buildings.xml!",LOG_TYPE_WARNING);
	while ( pXmlNode != NULL)
	{
		pXmlNode = pXmlNode->NextSibling();
		if ( pXmlNode == NULL)
			break;
		if( pXmlNode->Type() !=1 )
			continue;
		pszTmp = pXmlNode->ToElement()->Attribute( "directory" );
		if(pszTmp != 0)
			BuildingList.Add(pszTmp);
		else
		{
			sTmpString = "Can't read dierectory-attribute from \"\" - node";
			sTmpString.insert(38,pXmlNode->Value());
			Log.write(sTmpString.c_str(),LOG_TYPE_WARNING);
		}
		pszTmp = pXmlNode->ToElement()->Attribute( "num" );
		if(pszTmp != 0)
			IDList.Add(pszTmp);
		else
		{
			BuildingList.Delete(BuildingList.Size());
			sTmpString = "Can't read num-attribute from \"\" - node";
			sTmpString.insert(32,pXmlNode->Value());
			Log.write(sTmpString.c_str(),LOG_TYPE_WARNING);
		}

		pszTmp = pXmlNode->ToElement()->Attribute( "special" );
		if ( pszTmp != 0 )
		{
			string specialString = pszTmp;
			if ( specialString.compare ( "mine" ) == 0 ) specialIDMine.iSecondPart = atoi ( IDList[IDList.Size()-1].c_str() );
			else if ( specialString.compare ( "energy" ) == 0 ) specialIDSmallGen.iSecondPart = atoi ( IDList[IDList.Size()-1].c_str() );
			else if ( specialString.compare ( "connector" ) == 0 ) specialIDConnector.iSecondPart = atoi ( IDList[IDList.Size()-1].c_str() );
			else if ( specialString.compare ( "landmine" ) == 0 ) specialIDLandMine.iSecondPart = atoi ( IDList[IDList.Size()-1].c_str() );
			else if ( specialString.compare ( "seamine" ) == 0 ) specialIDSeaMine.iSecondPart = atoi ( IDList[IDList.Size()-1].c_str() );
			else if ( specialString.compare ( "smallBeton" ) == 0 ) specialIDSmallBeton.iSecondPart = atoi ( IDList[IDList.Size()-1].c_str() );
			else Log.write ( "Unknown spacial in buildings.xml \"" + specialString + "\"", LOG_TYPE_WARNING );
		}
	}

	if ( specialIDMine.iSecondPart == 0 ) Log.write ( "special \"mine\" missing in buildings.xml", LOG_TYPE_WARNING );
	if ( specialIDSmallGen.iSecondPart == 0 ) Log.write ( "special \"energy\" missing in buildings.xml", LOG_TYPE_WARNING );
	if ( specialIDConnector.iSecondPart == 0 ) Log.write ( "special \"connector\" missing in buildings.xml", LOG_TYPE_WARNING );
	if ( specialIDLandMine.iSecondPart == 0 ) Log.write ( "special \"landmine\" missing in buildings.xml", LOG_TYPE_WARNING );
	if ( specialIDSeaMine.iSecondPart == 0 ) Log.write ( "special \"seamine\" missing in buildings.xml", LOG_TYPE_WARNING );
	if ( specialIDSmallBeton.iSecondPart == 0 ) Log.write ( "special \"smallBeton\" missing in buildings.xml", LOG_TYPE_WARNING );

	specialIDMine.iFirstPart = specialIDSmallGen.iFirstPart = specialIDConnector.iFirstPart = specialIDLandMine.iFirstPart = specialIDSeaMine.iFirstPart = specialIDSmallBeton.iFirstPart = 1;
	// load found units
	UnitsData.building.Reserve(0);
	for( unsigned int i = 0; i < BuildingList.Size(); i++)
	{
		sBuildingPath = SettingsData.sBuildingsPath;
		sBuildingPath += PATH_DELIMITER;
		sBuildingPath += BuildingList[i];
		sBuildingPath += PATH_DELIMITER;

		// Prepare memory for next unit
		UnitsData.building.Add(sBuilding());

		sBuilding& b = UnitsData.building.Back();
		LoadUnitData(&b.data, sBuildingPath.c_str(), atoi(IDList[i].c_str()));
		translateUnitData(b.data.ID, false);

		// load img
		sTmpString = sBuildingPath;
		sTmpString += "img.pcx";
		if(FileExists(sTmpString.c_str()))
		{
			b.img_org = LoadPCX(sTmpString.c_str());
			SDL_SetColorKey(b.img_org, SDL_SRCCOLORKEY, 0xFFFFFF);
			b.img = LoadPCX(sTmpString.c_str());
			SDL_SetColorKey(b.img, SDL_SRCCOLORKEY, 0xFFFFFF);
		}
		else
		{
			Log.write("Missing GFX - your MAXR install seems to be incomplete!", cLog::eLOG_TYPE_ERROR);
			return -1;
		}
		// load shadow
		sTmpString = sBuildingPath;
		sTmpString += "shw.pcx";
		if(FileExists(sTmpString.c_str()))
		{
			b.shw_org = LoadPCX(sTmpString.c_str());
			b.shw     = LoadPCX(sTmpString.c_str());
			SDL_SetAlpha(b.shw, SDL_SRCALPHA, 50);
		}

		// load video
		sTmpString = sBuildingPath;
		sTmpString += "video.pcx";
		if(FileExists(sTmpString.c_str()))
			b.video = LoadPCX(sTmpString.c_str());

		// load infoimage
		sTmpString = sBuildingPath;
		sTmpString += "info.pcx";
		if(FileExists(sTmpString.c_str()))
			b.info = LoadPCX(sTmpString.c_str());

		// load effectgraphics if necessary
		if (b.data.powerOnGraphic)
		{
			sTmpString = sBuildingPath;
			sTmpString += "effect.pcx";
			if(FileExists(sTmpString.c_str()))
			{
				b.eff_org = LoadPCX(sTmpString.c_str());
				b.eff     = LoadPCX(sTmpString.c_str());
				SDL_SetAlpha(b.eff, SDL_SRCALPHA, 10);
			}
		}
		else
		{
			b.eff_org = NULL;
			b.eff     = NULL;
		}

		// load sounds
		LoadUnitSoundfile(b.Start,   sBuildingPath.c_str(), "start.wav");
		LoadUnitSoundfile(b.Running, sBuildingPath.c_str(), "running.wav");
		LoadUnitSoundfile(b.Stop,    sBuildingPath.c_str(), "stop.wav");
		LoadUnitSoundfile(b.Attack,  sBuildingPath.c_str(), "attack.wav");

		// Get Ptr if necessary:
		if ( b.data.ID == specialIDConnector )
		{
			b.data.isConnectorGraphic = true;
			UnitsData.ptr_connector = b.img;
			UnitsData.ptr_connector_org = b.img_org;
			SDL_SetColorKey(UnitsData.ptr_connector,SDL_SRCCOLORKEY,0xFF00FF);
			UnitsData.ptr_connector_shw = b.shw;
			UnitsData.ptr_connector_shw_org = b.shw_org;
			SDL_SetColorKey(UnitsData.ptr_connector_shw,SDL_SRCCOLORKEY,0xFF00FF);
		}
		else if (b.data.ID == specialIDSmallBeton )
		{
			UnitsData.ptr_small_beton = b.img;
			UnitsData.ptr_small_beton_org = b.img_org;
			SDL_SetColorKey(UnitsData.ptr_small_beton,SDL_SRCCOLORKEY,0xFF00FF);
		}

		// Check if there is more than one frame
		// use 129 here becouse some images from the res_installer are one pixel to large
		if (b.img_org->w > 129 && !b.data.isConnectorGraphic && !b.data.hasClanLogos ) b.data.hasFrames = b.img_org->w / b.img_org->h;
		else b.data.hasFrames = 0;
	}

	// Dirtsurfaces
	LoadGraphicToSurface ( UnitsData.dirt_big,SettingsData.sBuildingsPath.c_str(),"dirt_big.pcx" );
	LoadGraphicToSurface ( UnitsData.dirt_big_org,SettingsData.sBuildingsPath.c_str(),"dirt_big.pcx" );
	LoadGraphicToSurface ( UnitsData.dirt_big_shw,SettingsData.sBuildingsPath.c_str(),"dirt_big_shw.pcx" );
	if ( UnitsData.dirt_big_shw ) SDL_SetAlpha(UnitsData.dirt_big_shw,SDL_SRCALPHA,50);
	LoadGraphicToSurface ( UnitsData.dirt_big_shw_org,SettingsData.sBuildingsPath.c_str(),"dirt_big_shw.pcx" );
	LoadGraphicToSurface ( UnitsData.dirt_small,SettingsData.sBuildingsPath.c_str(),"dirt_small.pcx" );
	LoadGraphicToSurface ( UnitsData.dirt_small_org,SettingsData.sBuildingsPath.c_str(),"dirt_small.pcx" );
	LoadGraphicToSurface ( UnitsData.dirt_small_shw,SettingsData.sBuildingsPath.c_str(),"dirt_small_shw.pcx" );
	if ( UnitsData.dirt_small_shw ) SDL_SetAlpha(UnitsData.dirt_small_shw,SDL_SRCALPHA,50);
	LoadGraphicToSurface ( UnitsData.dirt_small_shw_org,SettingsData.sBuildingsPath.c_str(),"dirt_small_shw.pcx" );

	// set building numbers
	for (unsigned int i = 0; i < UnitsData.building.Size(); ++i)
	{
		UnitsData.building[i].nr = (int)i;
	}
	return 1;
}

//-------------------------------------------------------------------------------------
int getXMLNodeInt( TiXmlDocument &document, const char *path0, const char *path1,const char *path2 )
{
	string tmpString;
	ExTiXmlNode *pExXmlNode = NULL;
	pExXmlNode = pExXmlNode->XmlGetFirstNode ( document, path0, path1, path2, NULL );

	string pathText = "";
	if ( path0 ) pathText += (string)path0;
	if ( path1 ) pathText += (string)"~" + path1;
	if ( path2 ) pathText += (string)"~" + path2;

	if ( pExXmlNode == NULL )
	{
		// if( SettingsData.bDebug ) Log.write( ((string)"Can't find \"") + pathText + "\" ", cLog::eLOG_TYPE_DEBUG);
		return 0;
	}
	if ( pExXmlNode->XmlReadNodeData( tmpString, ExTiXmlNode::eXML_ATTRIBUTE, "Num" ) ) return atoi ( tmpString.c_str() );
	else
	{
		Log.write( ((string)"Can't read \"Num\" from \"") + pathText + "\"", cLog::eLOG_TYPE_WARNING );
		return 0;
	}
}

//-------------------------------------------------------------------------------------
float getXMLNodeFloat( TiXmlDocument &document, const char *path0, const char *path1,const char *path2 )
{
	string tmpString;
	ExTiXmlNode *pExXmlNode = NULL;
	pExXmlNode = pExXmlNode->XmlGetFirstNode ( document, path0, path1, path2, NULL );

	double tmpDouble;
	string pathText = "";
	if ( path0 ) pathText += (string)path0;
	if ( path1 ) pathText += (string)"~" + path1;
	if ( path2 ) pathText += (string)"~" + path2;

	if ( pExXmlNode == NULL )
	{
		// if( SettingsData.bDebug ) Log.write( ((string)"Can't find \"") + pathText + "\" ", cLog::eLOG_TYPE_DEBUG);
		return 0;
	}
	if ( pExXmlNode->ToElement()->Attribute( "Num", &tmpDouble ) ) return (float)( tmpDouble );
	else
	{
		Log.write( ((string)"Can't read \"Num\" from \"") + pathText + "\"", cLog::eLOG_TYPE_WARNING );
		return 0;
	}
}

//-------------------------------------------------------------------------------------
string getXMLNodeString( TiXmlDocument &document, const char *attribut, const char *path0, const char *path1,const char *path2 )
{
	string tmpString;
	ExTiXmlNode *pExXmlNode = NULL;
	pExXmlNode = pExXmlNode->XmlGetFirstNode ( document, path0, path1, path2, NULL );

	string pathText = "";
	if ( path0 ) pathText += (string)path0;
	if ( path1 ) pathText += (string)"~" + path1;
	if ( path2 ) pathText += (string)"~" + path2;

	if ( pExXmlNode == NULL )
	{
		// if( SettingsData.bDebug ) Log.write( ((string)"Can't find \"") + pathText + "\" ", cLog::eLOG_TYPE_DEBUG);
		return "";
	}
	if ( pExXmlNode->XmlReadNodeData( tmpString, ExTiXmlNode::eXML_ATTRIBUTE, attribut ) ) return tmpString;
	else
	{
		Log.write( ((string)"Can't read \"") + attribut + "\" from \"" + pathText + "\"", cLog::eLOG_TYPE_WARNING );
		return "";
	}
}

//-------------------------------------------------------------------------------------
bool getXMLNodeBool( TiXmlDocument &document, const char *path0, const char *path1,const char *path2, const char *path3 )
{
	string tmpString;
	ExTiXmlNode *pExXmlNode = NULL;
	pExXmlNode = pExXmlNode->XmlGetFirstNode ( document, path0, path1, path2, path3, NULL );

	string pathText = "";
	if ( path0 ) pathText += (string)path0;
	if ( path1 ) pathText += (string)"~" + path1;
	if ( path2 ) pathText += (string)"~" + path2;
	if ( path3 ) pathText += (string)"~" + path3;

	if ( pExXmlNode == NULL )
	{
		//if( SettingsData.bDebug ) Log.write( ((string)"Can't find \"") + pathText + "\" ", cLog::eLOG_TYPE_DEBUG);
		return false;
	}
	if ( pExXmlNode->XmlReadNodeData( tmpString, ExTiXmlNode::eXML_ATTRIBUTE, "YN" ) )
	{
		if ( tmpString.compare ( "Yes" ) == 0 ) return true;
		else return false;
	}
	else
	{
		Log.write( ((string)"Can't read \"YN\" from \"") + pathText +"\"", cLog::eLOG_TYPE_WARNING );
		return false;
	}
}

//-------------------------------------------------------------------------------------
void LoadUnitData(sUnitData* const Data, char const* const directory, int const iID)
{
	TiXmlDocument unitDataXml;

	string path = directory;
	path += "data.xml";
	if( !FileExists( path.c_str() ) ) return ;

	if ( !unitDataXml.LoadFile ( path.c_str() ) )
	{
		Log.write( "Can't load " + path,LOG_TYPE_WARNING);
		return ;
	}
	// Read minimal game version
	string gameVersion = getXMLNodeString ( unitDataXml, "text", "Unit", "Header", "Game_Version" );

	//TODO check game version

	//read id
	string idString = getXMLNodeString ( unitDataXml, "ID", "Unit" );
	char szTmp[100];
	// check whether the id exists twice
	Data->ID.iFirstPart = atoi(idString.substr(0,idString.find(" ",0)).c_str());
	if(Data->ID.iFirstPart == 0)
	{
		for (size_t i = 0; i < UnitsData.vehicle.Size(); ++i)
		{
			if( UnitsData.vehicle[i].data.ID.iSecondPart == atoi(idString.substr(idString.find(" ",0),idString.length()).c_str()))
			{
				sprintf(szTmp, "unit with id %.2d %.2d already exists", UnitsData.vehicle[i].data.ID.iFirstPart, UnitsData.vehicle[i].data.ID.iSecondPart);
				Log.write(szTmp,LOG_TYPE_WARNING);
				return ;
			}
		}
	}
	else
	{
		for (size_t i = 0; i < UnitsData.building.Size(); ++i)
		{
			if( UnitsData.building[i].data.ID.iSecondPart == atoi(idString.substr(idString.find(" ",0),idString.length()).c_str()))
			{
				sprintf(szTmp, "unit with id %.2d %.2d already exists", UnitsData.vehicle[i].data.ID.iFirstPart, UnitsData.vehicle[i].data.ID.iSecondPart);
				Log.write(szTmp,LOG_TYPE_WARNING);
				return ;
			}
		}
	}
	Data->ID.iSecondPart = atoi(idString.substr(idString.find(" ",0),idString.length()).c_str());

	// check whether the read id is the same as the one from vehicles.xml or buildins.xml
	if(iID != atoi(idString.substr(idString.find(" ",0),idString.length()).c_str()))
	{
		sprintf(szTmp, "ID %.2d %.2d isn't equal with ID for unit \"%s\" ", atoi(idString.substr(0, idString.find(" ", 0)).c_str()), atoi(idString.substr(idString.find(" ", 0), idString.length()).c_str()), directory);
		Log.write(szTmp,LOG_TYPE_WARNING);
		return ;
	}
	else
	{
		sprintf(szTmp, "ID %.2d %.2d verified", atoi(idString.substr(0, idString.find(" ", 0)).c_str()), atoi(idString.substr(idString.find(" ", 0), idString.length()).c_str()));
		Log.write(szTmp,LOG_TYPE_DEBUG);
	}
	//read name
	Data->name = getXMLNodeString ( unitDataXml, "name", "Unit" );
	//read description
	if (ExTiXmlNode* const pExXmlNode = ExTiXmlNode::XmlGetFirstNode(unitDataXml, "Unit", "Description", NULL))
	{
		Data->description = pExXmlNode->ToElement()->GetText();
		int iPosition = (int)Data->description.find("\\n",0);
		while(iPosition != string::npos)
		{
			Data->description.replace(iPosition,2,"\n");
			iPosition = (int)Data->description.find("\\n",iPosition);
		}
	}

	// Weapon
	string muzzleType = getXMLNodeString ( unitDataXml, "Const", "Unit", "Weapon", "Muzzle_Type" );
	if ( muzzleType.compare ( "Big" ) == 0 ) Data->muzzleType = sUnitData::MUZZLE_TYPE_BIG;
	else if ( muzzleType.compare ( "Rocket" ) == 0 ) Data->muzzleType = sUnitData::MUZZLE_TYPE_ROCKET;
	else if ( muzzleType.compare ( "Small" ) == 0 ) Data->muzzleType = sUnitData::MUZZLE_TYPE_SMALL;
	else if ( muzzleType.compare ( "Med" ) == 0 ) Data->muzzleType = sUnitData::MUZZLE_TYPE_MED;
	else if ( muzzleType.compare ( "Med_Long" ) == 0 ) Data->muzzleType = sUnitData::MUZZLE_TYPE_MED_LONG;
	else if ( muzzleType.compare ( "Rocket_Cluster" ) == 0 ) Data->muzzleType = sUnitData::MUZZLE_TYPE_ROCKET_CLUSTER;
	else if ( muzzleType.compare ( "Torpedo" ) == 0 ) Data->muzzleType = sUnitData::MUZZLE_TYPE_TORPEDO;
	else if ( muzzleType.compare ( "Sniper" ) == 0 ) Data->muzzleType =sUnitData:: MUZZLE_TYPE_SNIPER;
	else Data->muzzleType = sUnitData::MUZZLE_TYPE_NONE;

	Data->ammoMax = getXMLNodeInt ( unitDataXml, "Unit", "Weapon", "Ammo_Quantity" );
	Data->shotsMax = getXMLNodeInt ( unitDataXml, "Unit", "Weapon", "Shots" );
	Data->range = getXMLNodeInt ( unitDataXml, "Unit", "Weapon", "Range" );
	Data->damage = getXMLNodeInt ( unitDataXml, "Unit", "Weapon", "Damage" );
	Data->canAttack = getXMLNodeInt ( unitDataXml, "Unit", "Weapon", "Can_Attack" );

	// TODO: make the code differ between attacking sea units and land units.
	// until this is done being able to attack sea units means being able to attack ground units.
	if ( Data->canAttack & TERRAIN_SEA ) Data->canAttack |= TERRAIN_GROUND;

	Data->canDriveAndFire = getXMLNodeBool ( unitDataXml, "Unit", "Weapon", "Can_Drive_And_Fire" );

	// Production
	Data->buildCosts = getXMLNodeInt ( unitDataXml, "Unit", "Production", "Built_Costs" );

	Data->canBuild = getXMLNodeString ( unitDataXml, "String", "Unit", "Production", "Can_Build" );
	Data->buildAs = getXMLNodeString ( unitDataXml, "String", "Unit", "Production", "Build_As" );

	Data->maxBuildFactor = getXMLNodeInt ( unitDataXml, "Unit", "Production", "Max_Build_Factor" );

	Data->canBuildPath = getXMLNodeBool ( unitDataXml, "Unit", "Production", "Can_Build_Path" );
	Data->canBuildRepeat = getXMLNodeBool ( unitDataXml, "Unit", "Production", "Can_Build_Repeat" );
	Data->buildIntern = getXMLNodeBool ( unitDataXml, "Unit", "Production", "Builds_Intern" );

	// Movement
	Data->speedMax = getXMLNodeInt ( unitDataXml, "Unit", "Movement", "Movement_Sum" );
	Data->speedMax *= 4;

	Data->factorGround = getXMLNodeFloat ( unitDataXml, "Unit", "Movement", "Factor_Ground" );
	Data->factorSea = getXMLNodeFloat ( unitDataXml, "Unit", "Movement", "Factor_Sea" );
	Data->factorAir = getXMLNodeFloat ( unitDataXml, "Unit", "Movement", "Factor_Air" );
	Data->factorCoast = getXMLNodeFloat ( unitDataXml, "Unit", "Movement", "Factor_Coast" );

	// Abilities
	Data->isBig = getXMLNodeBool ( unitDataXml, "Unit", "Abilities", "Is_Big" );
	Data->connectsToBase = getXMLNodeBool ( unitDataXml, "Unit", "Abilities", "Connects_To_Base" );
	Data->armor = getXMLNodeInt ( unitDataXml, "Unit", "Abilities", "Armor" );
	Data->hitpointsMax = getXMLNodeInt ( unitDataXml, "Unit", "Abilities", "Hitpoints" );
	Data->scan = getXMLNodeInt ( unitDataXml, "Unit", "Abilities", "Scan_Range" );
	Data->modifiesSpeed = getXMLNodeFloat ( unitDataXml, "Unit", "Abilities", "Modifies_Speed" );
	Data->canClearArea = getXMLNodeBool ( unitDataXml, "Unit", "Abilities", "Can_Clear_Area" );
	Data->canBeCaptured = getXMLNodeBool ( unitDataXml, "Unit", "Abilities", "Can_Be_Captured" );
	Data->canBeDisabled = getXMLNodeBool ( unitDataXml, "Unit", "Abilities", "Can_Be_Disabled" );
	Data->canCapture = getXMLNodeBool ( unitDataXml, "Unit", "Abilities", "Can_Capture" );
	Data->canDisable = getXMLNodeBool ( unitDataXml, "Unit", "Abilities", "Can_Disable" );
	Data->canRepair = getXMLNodeBool ( unitDataXml, "Unit", "Abilities", "Can_Repair" );
	Data->canRearm = getXMLNodeBool ( unitDataXml, "Unit", "Abilities", "Can_Rearm" );
	Data->canResearch = getXMLNodeBool ( unitDataXml, "Unit", "Abilities", "Can_Research" );
	Data->canPlaceMines = getXMLNodeBool ( unitDataXml, "Unit", "Abilities", "Can_Place_Mines" );
	Data->canSurvey = getXMLNodeBool ( unitDataXml, "Unit", "Abilities", "Can_Survey" );
	Data->doesSelfRepair = getXMLNodeBool ( unitDataXml, "Unit", "Abilities", "Does_Self_Repair" );
	Data->convertsGold = getXMLNodeInt ( unitDataXml, "Unit", "Abilities", "Converts_Gold" );
	Data->canSelfDestroy = getXMLNodeBool ( unitDataXml, "Unit", "Abilities", "Can_Self_Destroy" );
	Data->canScore = getXMLNodeBool ( unitDataXml, "Unit", "Abilities", "Can_Score" );

	Data->canMineMaxRes = getXMLNodeInt ( unitDataXml, "Unit", "Abilities", "Can_Mine_Max_Resource" );

	Data->needsMetal = getXMLNodeInt ( unitDataXml, "Unit", "Abilities", "Needs_Metal" );
	Data->needsOil = getXMLNodeInt ( unitDataXml, "Unit", "Abilities", "Needs_Oil" );
	Data->needsEnergy = getXMLNodeInt ( unitDataXml, "Unit", "Abilities", "Needs_Energy" );
	Data->needsHumans = getXMLNodeInt ( unitDataXml, "Unit", "Abilities", "Needs_Humans" );
	if ( Data->needsEnergy < 0 )
	{
		Data->produceEnergy = abs( Data->needsEnergy );
		Data->needsEnergy = 0;
	} else Data->produceEnergy = 0;
	if ( Data->needsHumans < 0 )
	{
		Data->produceHumans = abs( Data->needsHumans );
		Data->needsHumans = 0;
	} else Data->produceHumans = 0;

	Data->isStealthOn = getXMLNodeInt ( unitDataXml, "Unit", "Abilities", "Is_Stealth_On" );
	Data->canDetectStealthOn = getXMLNodeInt ( unitDataXml, "Unit", "Abilities", "Can_Detect_Stealth_On" );

	string surfacePosString = getXMLNodeString ( unitDataXml, "Const", "Unit", "Abilities", "Surface_Position" );
	if ( surfacePosString.compare ( "BeneathSea" ) == 0 ) Data->surfacePosition = sUnitData::SURFACE_POS_BENEATH_SEA;
	else if ( surfacePosString.compare ( "AboveSea" ) == 0 ) Data->surfacePosition = sUnitData::SURFACE_POS_ABOVE_SEA;
	else if ( surfacePosString.compare ( "Base" ) == 0 ) Data->surfacePosition = sUnitData::SURFACE_POS_BASE;
	else if ( surfacePosString.compare ( "AboveBase" ) == 0 ) Data->surfacePosition = sUnitData::SURFACE_POS_ABOVE_BASE;
	else if ( surfacePosString.compare ( "Above" ) == 0 ) Data->surfacePosition = sUnitData::SURFACE_POS_ABOVE;
	else Data->surfacePosition = sUnitData::SURFACE_POS_GROUND;

	string overbuildString = getXMLNodeString ( unitDataXml, "Const", "Unit", "Abilities", "Can_Be_Overbuild" );
	if ( overbuildString.compare ( "Yes" ) == 0 ) Data->canBeOverbuild = sUnitData::OVERBUILD_TYPE_YES;
	else if ( overbuildString.compare ( "YesNRemove" ) == 0 ) Data->canBeOverbuild = sUnitData::OVERBUILD_TYPE_YESNREMOVE;
	else Data->canBeOverbuild = sUnitData::OVERBUILD_TYPE_NO;

	Data->canBeLandedOn = getXMLNodeBool ( unitDataXml, "Unit", "Abilities", "Can_Be_Landed_On" );
	Data->canWork = getXMLNodeBool ( unitDataXml, "Unit", "Abilities", "Is_Activatable" );
	Data->explodesOnContact = getXMLNodeBool ( unitDataXml, "Unit", "Abilities", "Explodes_On_Contact" );
	Data->isHuman = getXMLNodeBool ( unitDataXml, "Unit", "Abilities", "Is_Human" );

	// Storage
	Data->storageResMax = getXMLNodeInt ( unitDataXml, "Unit", "Storage", "Capacity_Resources" );

	string storeResString = getXMLNodeString ( unitDataXml, "Const", "Unit", "Storage", "Capacity_Res_Type" );
	if ( storeResString.compare ( "Metal" ) == 0 ) Data->storeResType = sUnitData::STORE_RES_METAL;
	else if ( storeResString.compare ( "Oil" ) == 0 ) Data->storeResType = sUnitData::STORE_RES_OIL;
	else if ( storeResString.compare ( "Gold" ) == 0 ) Data->storeResType = sUnitData::STORE_RES_GOLD;
	else Data->storeResType = sUnitData::STORE_RES_NONE;

	Data->storageUnitsMax = getXMLNodeInt ( unitDataXml, "Unit", "Storage", "Capacity_Units" );

	string storeUnitImgString = getXMLNodeString ( unitDataXml, "Const", "Unit", "Storage", "Capacity_Units_Image_Type" );
	if ( storeUnitImgString.compare ( "Plane" ) == 0 ) Data->storeUnitsImageType = sUnitData::STORE_UNIT_IMG_PLANE;
	else if ( storeUnitImgString.compare ( "Human" ) == 0 ) Data->storeUnitsImageType = sUnitData::STORE_UNIT_IMG_HUMAN;
	else if ( storeUnitImgString.compare ( "Tank" ) == 0 ) Data->storeUnitsImageType = sUnitData::STORE_UNIT_IMG_TANK;
	else if ( storeUnitImgString.compare ( "Ship" ) == 0 ) Data->storeUnitsImageType = sUnitData::STORE_UNIT_IMG_SHIP;
	else Data->storeUnitsImageType = sUnitData::STORE_UNIT_IMG_TANK;

	string storeUnitsString = getXMLNodeString ( unitDataXml, "String", "Unit", "Storage", "Capacity_Units_Type" );
	if ( storeUnitsString.length() > 0 )
	{
		int pos = -1;
		do
		{
			int lastpos = pos;
			pos = storeUnitsString.find_first_of ( "+", pos+1 );
			if ( pos == string::npos ) pos = storeUnitsString.length();
			Data->storeUnitsTypes.push_back ( storeUnitsString.substr ( lastpos+1, pos-(lastpos+1) ) );
		}
		while ( pos < (int)storeUnitsString.length() );
	}

	Data->isStorageType = getXMLNodeString ( unitDataXml, "String", "Unit", "Storage", "Is_Storage_Type" );

	// load graphics.xml
	LoadUnitGraphicData ( Data, directory );

	// finish
	Log.write("Unitdata read", cLog::eLOG_TYPE_DEBUG);
	if( SettingsData.bDebug ) Log.mark();
	return ;
}

//-------------------------------------------------------------------------------------
void LoadUnitGraphicData( sUnitData *Data, char const* directory )
{
	TiXmlDocument unitGraphicsXml;

	string path = directory;
	path += "graphics.xml";
	if( !FileExists( path.c_str() ) ) return ;

	if ( !unitGraphicsXml.LoadFile ( path.c_str() ) )
	{
		Log.write( "Can't load " + path, LOG_TYPE_WARNING );
		return ;
	}

	Data->hasClanLogos = getXMLNodeBool ( unitGraphicsXml, "Unit", "Graphic", "Has_Clan_Logos" );
	Data->hasCorpse = getXMLNodeBool ( unitGraphicsXml, "Unit", "Graphic", "Has_Corpse" );
	Data->hasDamageEffect = getXMLNodeBool ( unitGraphicsXml, "Unit", "Graphic", "Has_Damage_Effect" );
	Data->hasBetonUnderground = getXMLNodeBool ( unitGraphicsXml, "Unit", "Graphic", "Has_Beton_Underground" );
	Data->hasPlayerColor = getXMLNodeBool ( unitGraphicsXml, "Unit", "Graphic", "Has_Player_Color" );
	Data->hasOverlay = getXMLNodeBool ( unitGraphicsXml, "Unit", "Graphic", "Has_Overlay" );

	Data->buildUpGraphic = getXMLNodeBool ( unitGraphicsXml, "Unit", "Graphic", "Animations", "Build_Up" );
	Data->animationMovement = getXMLNodeBool ( unitGraphicsXml, "Unit", "Graphic", "Animations", "Movement" );
	Data->powerOnGraphic = getXMLNodeBool ( unitGraphicsXml, "Unit", "Graphic", "Animations", "Power_On" );
	Data->isAnimated = getXMLNodeBool ( unitGraphicsXml, "Unit", "Graphic", "Animations", "Is_Animated" );
	Data->makeTracks = getXMLNodeBool ( unitGraphicsXml, "Unit", "Graphic", "Animations", "Makes_Tracks" );
}

//-------------------------------------------------------------------------------------
static int LoadClans()
{
	TiXmlDocument clansXml;

	string clansXMLPath = CLANS_XML;
	if (!FileExists(clansXMLPath.c_str()))
		return 0;
	if (!clansXml.LoadFile(clansXMLPath.c_str()))
	{
		Log.write ("Can't load "+clansXMLPath, LOG_TYPE_ERROR);
		return 0;
	}

	TiXmlNode* xmlNode = clansXml.FirstChildElement ("Clans");
	if (xmlNode == 0)
	{
		Log.write ("Can't read \"Clans\" node!", LOG_TYPE_ERROR);
		return 0;
	}

	for (TiXmlNode* clanNode = 0; (clanNode = xmlNode->IterateChildren(clanNode));)
	{
		TiXmlElement* clanElement = clanNode->ToElement ();
		if (clanElement)
		{
			cClan* newClan = cClanData::instance ().addClan ();
			string nameAttr = clanElement->Attribute ("Name");
			newClan->setName (nameAttr);

			const TiXmlNode* descriptionNode = clanNode->FirstChild ("Description");
			if (descriptionNode)
			{
				string descriptionString = descriptionNode->ToElement ()->GetText ();
				newClan->setDescription (descriptionString);
			}

			for (TiXmlNode* changedUnitStatsNode = 0; (changedUnitStatsNode = clanNode->IterateChildren("ChangedUnitStat", changedUnitStatsNode));)
			{
				TiXmlElement* statsElement = changedUnitStatsNode->ToElement ();
				if (statsElement)
				{
					const char* idAttr = statsElement->Attribute ("UnitID");
					if (idAttr == 0)
					{
						Log.write ("Couldn't read UnitID for ChangedUnitStat for clans", LOG_TYPE_ERROR);
						continue;
					}
					string idAttrStr (idAttr);
					int firstPart = atoi (idAttrStr.substr (0, idAttrStr.find (" ", 0)).c_str ());
					int secondPart = atoi (idAttrStr.substr (idAttrStr.find (" ", 0), idAttrStr.length ()).c_str ());

					cClanUnitStat* newStat = newClan->addUnitStat (firstPart, secondPart);

					for (TiXmlNode* modificationNode = 0; (modificationNode = changedUnitStatsNode->IterateChildren(modificationNode));)
					{
						string modName = modificationNode->Value ();
						TiXmlElement* modificationElement = modificationNode->ToElement ();
						if (modName != "" && modificationElement)
						{
							const char* numAttr = modificationElement->Attribute ("Num");
							if (numAttr != 0)
							{
								int value = atoi (numAttr);
								newStat->addModification (modName, value);
							}
						}
					}
				}
			}

		}
	}
	return 1;
}

/**
 * Saves the value. Do not use bye yourselve. Only used by SaveOption()-function.
 * @param pXmlNode Node to which the value should be set
 * @param sAttributName Name (which means typ) of the attribut to set ("YN", "Num" or "Text")
 * @param bValue bool value to set
 * @param iValue int value to set
 * @param sValue string value to set
 * @return 1 on success
 */
static void SaveValue(ExTiXmlNode *pXmlNode, string sAttributName, bool bValue, int iValue, string sValue)
{
	if( !pXmlNode )
	{
		Log.write ( "Can't find necessary node in max.xml", LOG_TYPE_WARNING );
	}
	else
	{
		if( sAttributName.compare("YN") == 0 )
		{
			if( bValue )
				pXmlNode->ToElement()->SetAttribute ( "YN", "Yes" );
			else
				pXmlNode->ToElement()->SetAttribute ( "YN", "No" );
		}
		else if( sAttributName.compare("Num") == 0 )
		{
			pXmlNode->ToElement()->SetAttribute ( "Num", iValue );
		}
		else if( sAttributName.compare("Text") == 0 )
		{
			pXmlNode->ToElement()->SetAttribute ( "Text", sValue.c_str() );
		}
	}
}
int SaveOption ( int iTyp )
{
	Log.write ( "Saving option " + iToStr(iTyp), LOG_TYPE_INFO );

	// Prepare max.xml for writing
	TiXmlDocument MaxXml;
	ExTiXmlNode * pXmlNode = NULL;
	if(!FileExists(SettingsData.sConfig.c_str()))
	{
		Log.write ( "Generating new config file", LOG_TYPE_WARNING );
		if( GenerateMaxXml() == -1)
		{
			return -1;
		}
	}
	if(!MaxXml.LoadFile(SettingsData.sConfig.c_str()))
	{
		Log.write ( "Can't read from config file "+SettingsData.sConfig, LOG_TYPE_WARNING );
		if( GenerateMaxXml() == -1)
		{
			return -1;
		}
	}
	switch (iTyp )
	{
	case SAVETYPE_ANIMATIONS:
		pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","EnableAnimations", NULL);
		SaveValue ( pXmlNode, "YN",SettingsData.bAnimations,0,"");
		break;
	case SAVETYPE_SHADOWS:
		pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","EnableShadows", NULL);
		SaveValue ( pXmlNode, "YN",SettingsData.bShadows,0,"");
		break;
	case SAVETYPE_ALPHA:
		pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","EnableAlphaEffects", NULL);
		SaveValue ( pXmlNode, "YN",SettingsData.bAlphaEffects,0,"");
		break;
	case SAVETYPE_DAMAGEEFFECTS_BUILDINGS:
		pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","EnableDamageEffects", NULL);
		SaveValue ( pXmlNode, "YN",SettingsData.bDamageEffects,0,"");
		break;
	case SAVETYPE_DAMAGEEFFECTS_VEHICLES:
		pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","EnableDamageEffectsVehicles", NULL);
		SaveValue ( pXmlNode, "YN",SettingsData.bDamageEffectsVehicles,0,"");
		break;
	case SAVETYPE_TRACKS:
		pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","EnableMakeTracks", NULL);
		SaveValue ( pXmlNode, "YN",SettingsData.bMakeTracks,0,"");
		break;
	case SAVETYPE_AUTOSAVE:
		pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","EnableAutosave", NULL);
		SaveValue ( pXmlNode, "YN",SettingsData.bAutoSave,0,"");
		break;
	case SAVETYPE_NAME:
		pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Net","PlayerName", NULL);
		SaveValue ( pXmlNode, "Text",false,0,SettingsData.sPlayerName);
		break;
	case SAVETYPE_COLOR:
		pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Net","PlayerName", NULL);
		SaveValue ( pXmlNode, "Num",false,SettingsData.iColor,"");
		break;
	case SAVETYPE_IP:
		pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Net","IP", NULL);
		SaveValue ( pXmlNode, "Text",false,0,SettingsData.sIP);
		break;
	case SAVETYPE_PORT:
		pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Net","Port", NULL);
		SaveValue ( pXmlNode, "Num",false,SettingsData.iPort,"");
		break;
	case SAVETYPE_MUSICMUTE:
		pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Sound","MusicMute", NULL);
		SaveValue ( pXmlNode, "YN",SettingsData.MusicMute,0,"");
		break;
	case SAVETYPE_VOICEMUTE:
		pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Sound","VoiceMute", NULL);
		SaveValue ( pXmlNode, "YN",SettingsData.VoiceMute,0,"");
		break;
	case SAVETYPE_SOUNDMUTE:
		pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Sound","SoundMute", NULL);
		SaveValue ( pXmlNode, "YN",SettingsData.SoundMute,0,"");
		break;
	case SAVETYPE_MUSICVOL:
		pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Sound","MusicVol", NULL);
		SaveValue ( pXmlNode, "Num",false,SettingsData.MusicVol,"");
		break;
	case SAVETYPE_VOICEVOL:
		pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Sound","VoiceVol", NULL);
		SaveValue ( pXmlNode, "Num",false,SettingsData.VoiceVol,"");
		break;
	case SAVETYPE_SOUNDVOL:
		pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Sound","SoundVol", NULL);
		SaveValue ( pXmlNode, "Num",false,SettingsData.SoundVol,"");
		break;
	case SAVETYPE_SCROLLSPEED:
		pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","ScrollSpeed", NULL);
		SaveValue ( pXmlNode, "Num",false,SettingsData.iScrollSpeed,"");
		break;
	case SAVETYPE_INTRO:
		pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Start","Intro", NULL);
		SaveValue ( pXmlNode, "YN",SettingsData.bIntro,0,"");
		break;
	case SAVETYPE_WINDOW:
		pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Start","Windowmode", NULL);
		SaveValue ( pXmlNode, "YN",SettingsData.bWindowMode,0,"");
		break;
	case SAVETYPE_RESOLUTION:
		pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Start","Resolution", NULL);
		SaveValue ( pXmlNode, "Text", false, 0, iToStr(SettingsData.iScreenW)+"."+iToStr(SettingsData.iScreenH));
		break;
	case SAVETYPE_CACHESIZE:
		pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Start","CacheSize", NULL);
		SaveValue ( pXmlNode, "Num",false,SettingsData.iCacheSize,"");
		break;
	}
	MaxXml.SaveFile(); // Write the new values to the file
	return 1;
}

void reloadUnitValues ()
{
	TiXmlDocument UnitsXml;
	TiXmlElement *Element;
	if( !FileExists( (SettingsData.sVehiclesPath + PATH_DELIMITER + "vehicles.xml").c_str() ) ) return ;
	if ( !UnitsXml.LoadFile ( (SettingsData.sVehiclesPath + PATH_DELIMITER + "vehicles.xml" ).c_str() ) ) return;
	if( !( Element = UnitsXml.FirstChildElement ( "VehicleData" )->FirstChildElement ( "Vehicles" ) ) ) return;

	Element = Element->FirstChildElement();
	int i = 0;
	while ( Element != NULL )
	{
		int num;
		Element->Attribute( "num", &num );
		LoadUnitData ( &UnitsData.vehicle[i].data, (SettingsData.sVehiclesPath+PATH_DELIMITER+Element->Attribute( "directory" )+PATH_DELIMITER).c_str(), num );
		translateUnitData( UnitsData.vehicle[i].data.ID, true );
		if ( Element->NextSibling() ) Element = Element->NextSibling()->ToElement();
		else Element = NULL;
		i++;
	}

	if( !FileExists( (SettingsData.sBuildingsPath + PATH_DELIMITER + "buildings.xml").c_str() ) ) return ;
	if ( !UnitsXml.LoadFile ( (SettingsData.sBuildingsPath + PATH_DELIMITER + "buildings.xml" ).c_str() ) ) return;
	if( !( Element = UnitsXml.FirstChildElement ( "BuildingsData" )->FirstChildElement ( "Buildings" ) ) ) return;

	Element = Element->FirstChildElement();
	i = 0;
	while ( Element != NULL )
	{
		int num;
		Element->Attribute( "num", &num );
		LoadUnitData ( &UnitsData.building[i].data, (SettingsData.sBuildingsPath+PATH_DELIMITER+Element->Attribute( "directory" )+PATH_DELIMITER).c_str(), num );
		translateUnitData( UnitsData.building[i].data.ID, false );
		if ( Element->NextSibling() ) Element = Element->NextSibling()->ToElement();
		else Element = NULL;
		i++;
	}
}

void setPaths()
{
	//init absolutly needed paths
	SettingsData.sLog = MAX_LOG;
	SettingsData.sNetLog = MAX_NET_LOG;
	SettingsData.sExePath = ""; //FIXME: I don't know how this is handled on win/mac/amiga -- beko
	SettingsData.sHome="";

	#if MAC
	// do some rudimentary work with the user's homefolder. Needs to be extended in future...
	char * cHome = getenv("HOME"); //get $HOME on mac
	if(cHome != NULL)
		SettingsData.sHome = cHome;
	if (!SettingsData.sHome.empty())
	{
		SettingsData.sHome += PATH_DELIMITER;
		SettingsData.sHome += ".maxr";
		SettingsData.sHome += PATH_DELIMITER;

		// check whether home dir is set up and readable
		if (!FileExists(SettingsData.sHome.c_str())) // under mac everything is a file
		{
			if (mkdir (SettingsData.sHome.c_str (), 0755) == 0)
				cout << "\n(II): Created new config directory " << SettingsData.sHome;
			else
			{
				cout << "\n(EE): Can't create config directory " << SettingsData.sHome;
				SettingsData.sHome = ""; //reset $HOME since we can't create our config directory
			}
		}
	}
	//this is also a good place to find out where the executable is located
	SettingsData.sConfig = MAX_XML; //assume config in current working directory
	#elif WIN32
		//this is where windowsuser should set their %HOME%
		//this is also a good place to find out where the executable is located
		SettingsData.sConfig = MAX_XML; //assume config in current working directory
	#elif __amigaos4__
		//this is where amigausers should set their %HOME%
		//this is also a good place to find out where the executable is located
		SettingsData.sConfig = MAX_XML; //assume config in current working directory
	#else
	//NOTE: I do not use cLog here on purpose. Logs on linux go somewhere to $HOME/.maxr/ - as long as we can't access that we have to output everything to the terminal because game's root dir is usually write protected! -- beko
	bool bCreateConfigDir = false;

	char * cHome = getenv("HOME"); //get $HOME on linux
	if(cHome != NULL)
	{
		SettingsData.sHome = cHome; //get $HOME on linux
	}

	if (!SettingsData.sHome.empty())
	{
		SettingsData.sHome += PATH_DELIMITER;
		SettingsData.sHome += ".maxr";
		SettingsData.sHome += PATH_DELIMITER;
		SettingsData.sConfig = SettingsData.sHome;
		SettingsData.sConfig += MAX_XML; //set config to $HOME/.maxr/max.xml

		//check whether home dir is set up and readable
		if(!FileExists(SettingsData.sHome.c_str())) //under linux everything is a file -- beko
		{
			if(mkdir(SettingsData.sHome.c_str(), 0755) == 0)
			{
				bCreateConfigDir = true;
				cout << "\n(II): Created new config directory " << SettingsData.sHome;
			}
			else
			{
				cout << "\n(EE): Can't create config directory " << SettingsData.sHome;
				SettingsData.sHome = ""; //reset $HOME since we can't create our config directory
			}
		}
		else
		{
			cout << "\n(II): Config is read from " << SettingsData.sHome; //config dir can be read - we're fine
		}
	}
	else
	{
		cout << "\n(WW): $HOME is not set!";
		SettingsData.sHome="";
		SettingsData.sConfig = MAX_XML; //assume config in current working directory
	}

	//set new place for logs
	SettingsData.sLog = SettingsData.sHome + SettingsData.sLog;
	SettingsData.sNetLog = SettingsData.sHome + SettingsData.sNetLog;
	cout << "\n(II): Starting logging to: " << SettingsData.sLog;

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


			SettingsData.sExePath = cPathToExe;
			SettingsData.sExePath = SettingsData.sExePath.substr(0, iPos);
			SettingsData.sExePath += PATH_DELIMITER;

			if(FileExists( (SettingsData.sExePath+"maxr").c_str() )) //check for binary itself in bin folder
			{
				Log.write("Path to binary is: "+SettingsData.sExePath, cLog::eLOG_TYPE_INFO);
			}
			else
			{	//perhaps we got ourself a trailing maxr in the path like /proc
				// seems to do it sometimes. remove it and try again!
				if(cPathToExe[iPos-1] == 'r' && cPathToExe[iPos-2] == 'x' && cPathToExe[iPos-3] == 'a' && cPathToExe[iPos-4] == 'm' )
				{
					SettingsData.sExePath = SettingsData.sExePath.substr(0, iPos-5);
					if(FileExists( (SettingsData.sExePath+"maxr").c_str() ))
					{
						Log.write("Path to binary is: "+SettingsData.sExePath, cLog::eLOG_TYPE_INFO);
					}
				}
			}

		}
	}
	else
	{
		Log.write("Can't resolve full path to program. Doesn't this system feature /proc/self/exe?", cLog::eLOG_TYPE_WARNING);
		SettingsData.sExePath=""; //reset sExePath
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

		if(mkdir(SettingsData.sSavesPath.c_str(), 0755) == 0)
		{
			Log.write("Created new save directory: "+SettingsData.sSavesPath, cLog::eLOG_TYPE_INFO);
		}
		else
		{
			Log.write("Can't create save directory: "+SettingsData.sSavesPath, cLog::eLOG_TYPE_ERROR);
		}
	}

	cout << "\n";
	#endif
}

string searchData(string sDataDirFromConf)
{
	string sPathToGameData = "";
	#if MAC
		sPathToGameData =  SettingsData.sExePath; //assuming data is in same folder like binary (or current working directory)
	#elif WIN32
		sPathToGameData = SettingsData.sExePath; //assuming data is in same folder like binary (or current working directory)
	#elif __amigaos4__
		sPathToGameData = SettingsData.sExePath; //assuming data is in same folder like binary (or current working directory)
	#else
	//BEGIN crude path validation to find gamedata
	Log.write ( "Probing for data paths using default values:", cLog::eLOG_TYPE_INFO );

	#define PATHCOUNT 11
	string sPathArray[PATHCOUNT] = {
		BUILD_DATADIR, //most important position holds value of configure --prefix to gamedata in %prefix%/$(datadir)/maxr or default path if autoversion.h wasn't used
		"/usr/local/share/maxr",
		"/usr/games/maxr",
		"/usr/local/games/maxr",
		"/usr/maxr",
		"/usr/local/maxr",
		"/opt/maxr",
		"/usr/share/games/maxr",
		"/usr/local/share/games/maxr",
		SettingsData.sExePath, //check for gamedata in bin folder too
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
		string sInitFile = sPathArray[i];
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
