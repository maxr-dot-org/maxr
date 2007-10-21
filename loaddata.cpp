///////////////////////////////////////////////////////////////////////////////
//
// M.A.X. Reloaded - loaddata.cpp
//
///////////////////////////////////////////////////////////////////////////////
//
// Loads all relevant files and datas at the start of the game.
// 
//
///////////////////////////////////////////////////////////////////////////////

// Defines for more Debuginformations in Log

//#define TERRAIN_DEBUG	// Logs the using of default values while loading terrain

#include "extendedtinyxml.h"
#include "loaddata.h"
#include "files.h"
#include "log.h"
#include "pcx.h"
#include "fonts.h"
#include "keys.h"
#include "vehicles.h"

// LoadData ///////////////////////////////////////////////////////////////////
// Loads all relevant files and datas:
int LoadData ( void * )
{
	LoadingData=LOAD_GOING;

	string sTmpString;

	// Load fonts for SplashMessages
	cLog::write ( "Loading font for Splash Messages", LOG_TYPE_INFO );
	if(!LoadGraphicToSurface(FontsData.font, SettingsData.sFontPath.c_str(), "font.pcx")) NECESSARY_FILE_FAILURE
	if(!LoadGraphicToSurface(FontsData.font_big, SettingsData.sFontPath.c_str(), "font_big.pcx")) NECESSARY_FILE_FAILURE
	if(!LoadGraphicToSurface(FontsData.font_small_white, SettingsData.sFontPath.c_str(), "font_small_white.pcx")) NECESSARY_FILE_FAILURE
	fonts = new cFonts;
	cLog::write ( "Done", LOG_TYPE_DEBUG );
	cLog::mark();
	
	MakeLog(MAXVERSION,false,0);

	// Load Languagepack
	MakeLog("Loading languagepack...",false,2);
	if(!LoadLanguage()) return 0;
	MakeLog(lngPack.Translate( "Text~Initialisation~Load_Lang").c_str(),true,2); cLog::mark();

	// Load Keys
	
	MakeLog(lngPack.Translate( "Text~Initialisation~Load_Keys").c_str(),false,3);
	LoadKeys();
	MakeLog(lngPack.Translate( "Text~Initialisation~Load_Keys").c_str(),true,3); cLog::mark();
	
	// Load Fonts
	MakeLog(lngPack.Translate( "Text~Initialisation~Load_Fonts").c_str(),false,4);
	if(!LoadFonts(SettingsData.sFontPath.c_str()))	return 0;
	MakeLog(lngPack.Translate( "Text~Initialisation~Load_Fonts").c_str(),true,4); cLog::mark();

	// Load Graphics
	MakeLog(lngPack.Translate( "Text~Initialisation~Load_GFX").c_str(),false,5);
	if(!LoadGraphics(SettingsData.sGfxPath.c_str()))  //FIXME: is always 1
	{
		cLog::write ( "Error while loading graphics", LOG_TYPE_ERROR );
		LoadingData=LOAD_ERROR;
		return 0;
	}
	MakeLog(lngPack.Translate( "Text~Initialisation~Load_GFX").c_str(),true,5); cLog::mark();

	// Load Effects
	MakeLog(lngPack.Translate( "Text~Initialisation~Load_Effects").c_str(),false,6);
	LoadEffects(SettingsData.sFxPath.c_str()); //FIXME: proceeds on errors
	MakeLog(lngPack.Translate( "Text~Initialisation~Load_Effects").c_str(),true,6); cLog::mark();

	// Load Terrain
	MakeLog(lngPack.Translate( "Text~Initialisation~Load_Terrain").c_str(),false,7);
	if(!LoadTerrain(SettingsData.sTerrainPath.c_str())) return 0;
	MakeLog(lngPack.Translate( "Text~Initialisation~Load_Terrain").c_str(),true,7); cLog::mark();

	// Load Vehicles
	MakeLog(lngPack.Translate( "Text~Initialisation~Load_Vehicles").c_str(),false,8);
	if(!LoadVehicles()) return 0; 
	MakeLog(lngPack.Translate( "Text~Initialisation~Load_Vehicles").c_str(),true,8); cLog::mark();

	// Load Buildings
	MakeLog(lngPack.Translate( "Text~Initialisation~Load_Buildings").c_str(),false,9);
	if(!LoadBuildings()) return 0; 
	MakeLog(lngPack.Translate( "Text~Initialisation~Load_Buildings").c_str(),true,9); cLog::mark();

	// Load Music
	MakeLog(lngPack.Translate( "Text~Initialisation~Load_Music").c_str(),false,10);
	if(!LoadMusic(SettingsData.sMusicPath.c_str())) return 0;
	MakeLog(lngPack.Translate( "Text~Initialisation~Load_Music").c_str(),true,10); cLog::mark();

	// Load Sounds
	MakeLog(lngPack.Translate( "Text~Initialisation~Load_Sounds").c_str(),false,11);
	LoadSounds(SettingsData.sSoundsPath.c_str());
	MakeLog(lngPack.Translate( "Text~Initialisation~Load_Sounds").c_str(),true,11); cLog::mark();

	// Load Voices
	MakeLog(lngPack.Translate( "Text~Initialisation~Load_Voices").c_str(),false,12);
	LoadVoices(SettingsData.sVoicesPath.c_str());
	MakeLog(lngPack.Translate( "Text~Initialisation~Load_Voices").c_str(),true,12); cLog::mark();

	SDL_Delay(1000);
	LoadingData=LOAD_FINISHED;
	return 1;
}

// MakeLog ///////////////////////////////////////////////////////////////////
// Writes a Logmessage on the SplashScreen:
void MakeLog ( const char* sztxt,bool ok,int pos )
{
	if ( !ok )
		fonts->OutTextBig ( (char *)sztxt,22,152+16*pos,buffer );
	else
		fonts->OutTextBig ( "OK",250,152+16*pos,buffer );
	SDL_BlitSurface ( buffer,NULL,screen,NULL );
	SDL_UpdateRect ( screen,0,0,0,0 );
	return;
}

// LoadGraphicToSurface ///////////////////////////////////////////////////////
// Loades a graphic to the surface:
int LoadGraphicToSurface(SDL_Surface* &dest, const char* directory, const char* filename)
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

	dest = LoadPCX((char *)filepath.c_str());

	filepath.insert(0,"File loaded: ");
	cLog::write ( filepath.c_str(), LOG_TYPE_DEBUG );

	return 1;
}

// LoadEffectGraphicToSurface /////////////////////////////////////////////////
// Loades a effectgraphic to the surface:
int LoadEffectGraphicToSurface(SDL_Surface** &dest, const char* directory, const char* filename)
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

	
	dest = (SDL_Surface**)malloc(sizeof(SDL_Surface*)*2);
	if(!dest) { cLog::write("Out of memory", cLog::eLOG_TYPE_MEM); }
	dest[0] = LoadPCX((char *)filepath.c_str());
	dest[1] = LoadPCX((char *)filepath.c_str());

	filepath.insert(0,"Effect successful loaded: ");
	cLog::write ( filepath.c_str(), LOG_TYPE_DEBUG );

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
	
	dest = (SDL_Surface**)malloc(sizeof(SDL_Surface*)*2);
	if(!dest) { cLog::write("Out of memory", cLog::eLOG_TYPE_MEM); }
	dest[0] = LoadPCX((char *)filepath.c_str());
	SDL_SetAlpha(dest[0],SDL_SRCALPHA,alpha);
	dest[1] = LoadPCX((char *)filepath.c_str());
	SDL_SetAlpha(dest[1],SDL_SRCALPHA,alpha);

	filepath.insert(0,"Effectalpha loaded: ");
	cLog::write ( filepath.c_str(), LOG_TYPE_DEBUG );

	return 1;
}

// LoadSoundfile //////////////////////////////////////////////////////////////
// Loades a sounfile to the Mix_Chunk
int LoadSoundfile(sSOUND *&dest, const char* directory, const char* filename)
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

// LoadUnitSoundfile //////////////////////////////////////////////////////////
// Loades a unitsoundfile to the Mix_Chunk
void LoadUnitSoundfile(sSOUND *&dest, const char* directory, const char* filename)
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
				cLog::write("Can't load dummy.wav", LOG_TYPE_WARNING);
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
	cLog::write ( "Reading max.xml", LOG_TYPE_INFO );

	string sTmpString;
	// Prepare max.xml for reading
	TiXmlDocument MaxXml;
	ExTiXmlNode * pXmlNode = NULL;
	if(!FileExists("max.xml"))
	{
		cLog::write ( "Generating new config file max.xml", LOG_TYPE_WARNING );
		if( GenerateMaxXml() == -1)
		{
			return -1;
		}
	}
	if(!MaxXml.LoadFile("max.xml"))
	{
		cLog::write ( "Can't read max.xml\n", LOG_TYPE_WARNING );
		if( GenerateMaxXml() == -1)
		{
			return -1;
		}
	}

	// START Options
	// Resolution
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Start","Resolution", NULL)))
		cLog::write ( "Can't find Resolution-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Num"))
	{
		SettingsData.iScreenW = atoi(sTmpString.substr(0,sTmpString.find(".",0)).c_str());
		SettingsData.iScreenH = atoi(sTmpString.substr(sTmpString.find(".",0)+1,sTmpString.length()).c_str());
	}
	else
	{
		cLog::write ( "Can't load resolution from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.iScreenW = 640;
		SettingsData.iScreenH = 480;
	}
	// ColourDepth
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Start","ColourDepth", NULL)))
		cLog::write ( "Can't find ColourDepth-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Num"))
		SettingsData.iColourDepth = atoi(sTmpString.c_str());
	else
	{
		cLog::write ( "Can't load ColourDepth from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.iColourDepth = 32;
	}
	// Intro
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Start","Intro", NULL)))
		cLog::write ( "Can't find Intro-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"YN"))
		SettingsData.bIntro = pXmlNode->XmlDataToBool(sTmpString);
	else
	{
		cLog::write ( "Can't load Intro from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.bIntro = false;
	}
	// Windowmode
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Start","Windowmode", NULL)))
		cLog::write ( "Can't find Windowmode-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"YN"))
		SettingsData.bWindowMode = pXmlNode->XmlDataToBool(sTmpString);
	else
	{
		cLog::write ( "Can't load Windowmode from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.bWindowMode = true;
	}
	// Intro
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Start","Fastmode", NULL)))
		cLog::write ( "Can't find Fastmode-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"YN"))
		SettingsData.bFastMode = pXmlNode->XmlDataToBool(sTmpString);
	else
	{
		cLog::write ( "Can't load Fastmode from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.bFastMode = false;
	}
	// Language
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Start","Language", NULL)))
		cLog::write ( "Can't find Language-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		SettingsData.sLanguage = sTmpString;
	else
	{
		cLog::write ( "Can't load Language-Path from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.sLanguage = "ENG";
	}

	// GAME Options
	// EnableAutosave
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","EnableAutosave", NULL)))
		cLog::write ( "Can't find EnableAutosave-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"YN"))
		SettingsData.bAutoSave = pXmlNode->XmlDataToBool(sTmpString);
	else
	{
		cLog::write ( "Can't load EnableAutosave from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.bAutoSave = true;
	}
	// EnableAnimations
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","EnableAnimations", NULL)))
		cLog::write ( "Can't find EnableAnimations-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"YN"))
		SettingsData.bAnimations = pXmlNode->XmlDataToBool(sTmpString);
	else
	{
		cLog::write ( "Can't load EnableAnimations from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.bAnimations = true;
	}
	// EnableShadows
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","EnableShadows", NULL)))
		cLog::write ( "Can't find EnableShadows-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"YN"))
		SettingsData.bShadows = pXmlNode->XmlDataToBool(sTmpString);
	else
	{
		cLog::write ( "Can't load EnableShadows from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.bShadows = true;
	}
	// EnableAlphaEffects
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","EnableAlphaEffects", NULL)))
		cLog::write ( "Can't find EnableAlphaEffects-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"YN"))
		SettingsData.bAlphaEffects = pXmlNode->XmlDataToBool(sTmpString);
	else
	{
		cLog::write ( "Can't load EnableAlphaEffects from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.bAlphaEffects = true;
	}
	// EnableDescribtions
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","EnableDescribtions", NULL)))
		cLog::write ( "Can't find EnableDescribtions-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"YN"))
		SettingsData.bShowDescription = pXmlNode->XmlDataToBool(sTmpString);
	else
	{
		cLog::write ( "Can't load EnableDescribtions from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.bShowDescription = true;
	}
	// EnableDamageEffects
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","EnableDamageEffects", NULL)))
		cLog::write ( "Can't find EnableDamageEffects-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"YN"))
		SettingsData.bDamageEffects = pXmlNode->XmlDataToBool(sTmpString);
	else
	{
		cLog::write ( "Can't load EnableDamageEffects from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.bDamageEffects = true;
	}
	// EnableDamageEffectsVehicles
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","EnableDamageEffectsVehicles", NULL)))
		cLog::write ( "Can't find EnableDamageEffectsVehicles-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"YN"))
		SettingsData.bDamageEffectsVehicles = pXmlNode->XmlDataToBool(sTmpString);
	else
	{
		cLog::write ( "Can't load EnableDamageEffectsVehicles from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.bDamageEffectsVehicles = true;
	}
	// EnableMakeTracks  
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","EnableMakeTracks", NULL)))
		cLog::write ( "Can't find EnableMakeTracks-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"YN"))
		SettingsData.bMakeTracks = pXmlNode->XmlDataToBool(sTmpString);
	else
	{
		cLog::write ( "Can't load EnableMakeTracks from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.bMakeTracks = true;
	}
	// ScrollSpeed
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","ScrollSpeed", NULL)))
		cLog::write ( "Can't find ScrollSpeed-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Num"))
		SettingsData.iScrollSpeed = atoi(sTmpString.c_str());
	else
	{
		cLog::write ( "Can't load ColourDepth from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.iScrollSpeed = 32;
	}

	// GAME-NET Options
	//IP
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Net","IP", NULL)))
		cLog::write ( "Can't find Net-IP-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		SettingsData.sIP = sTmpString;
	else
	{
		cLog::write ( "Can't load IP from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.sIP = "127.0.0.1";
	}
	//Port
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Net","Port", NULL)))
		cLog::write ( "Can't find Net-Port-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Num"))
		SettingsData.iPort = atoi(sTmpString.c_str());
	else
	{
		cLog::write ( "Can't load Port from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.iPort = 58600;
	}
	//PlayerName
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Net","PlayerName", NULL)))
		cLog::write ( "Can't find Net-PlayerName-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		SettingsData.sPlayerName = sTmpString;
	else
	{
		cLog::write ( "Can't load PlayerName from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.sPlayerName = "Mechkommandant";
	}


	// GAME-SOUND Options
	// Enabled
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Sound", "Enabled", NULL)))
		cLog::write ( "Can't find Sound-Enabled-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"YN"))
		SettingsData.bSoundEnabled = pXmlNode->XmlDataToBool(sTmpString);
	else
	{
		cLog::write ( "Can't load Enabled from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.bSoundEnabled = true;
	}
	// MusicMute
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Sound", "MusicMute", NULL)))
		cLog::write ( "Can't find Sound-MusicMute-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"YN"))
		SettingsData.MusicMute = pXmlNode->XmlDataToBool(sTmpString);
	else
	{
		cLog::write ( "Can't load MusicMute from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.MusicMute = false;
	}
	// SoundMute
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Sound", "SoundMute", NULL)))
		cLog::write ( "Can't find Sound-SoundMute-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"YN"))
		SettingsData.SoundMute = pXmlNode->XmlDataToBool(sTmpString);
	else
	{
		cLog::write ( "Can't load SoundMute from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.SoundMute = false;
	}
	// VoiceMute
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Sound", "VoiceMute", NULL)))
		cLog::write ( "Can't find Sound-VoiceMute-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"YN"))
		SettingsData.VoiceMute = pXmlNode->XmlDataToBool(sTmpString);
	else
	{
		cLog::write ( "Can't load VoiceMute from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.VoiceMute = false;
	}
	//MusicVol
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Sound","MusicVol", NULL)))
		cLog::write ( "Can't find Sound-MusicVol-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Num"))
		SettingsData.MusicVol  = atoi(sTmpString.c_str());
	else
	{
		cLog::write ( "Can't load MusicVol from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.MusicVol  = 128;
	}
	//SoundVol
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Sound","SoundVol", NULL)))
		cLog::write ( "Can't find Sound-SoundVol-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Num"))
		SettingsData.SoundVol  = atoi(sTmpString.c_str());
	else
	{
		cLog::write ( "Can't load SoundVol from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.SoundVol  = 128;
	}
	//VoiceVol
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Sound","VoiceVol", NULL)))
		cLog::write ( "Can't find Sound-VoiceVol-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Num"))
		SettingsData.VoiceVol  = atoi(sTmpString.c_str());
	else
	{
		cLog::write ( "Can't load VoiceVol from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.VoiceVol  = 128;
	}
	//ChunkSize
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Sound","ChunkSize", NULL)))
		cLog::write ( "Can't find Sound-ChunkSize-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Num"))
		SettingsData.iChunkSize  = atoi(sTmpString.c_str());
	else
	{
		cLog::write ( "Can't load ChunkSize from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.iChunkSize  = 2048;
	}
	//Frequency
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Sound","Frequency", NULL)))
		cLog::write ( "Can't find Sound-Frequency-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Num"))
		SettingsData.iFrequency  = atoi(sTmpString.c_str());
	else
	{
		cLog::write ( "Can't load Frequency from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.iFrequency  = 44100;
	}

	// PATHs
	//Fonts 
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Paths","Fonts", NULL)))
		cLog::write ( "Can't find Path-Fonts-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		SettingsData.sFontPath = sTmpString;
	else
	{
		cLog::write ( "Can't load FontsPath from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.sFontPath = "fonts";
	}
	//FX 
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Paths","FX", NULL)))
		cLog::write ( "Can't find Path-FX-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		SettingsData.sFxPath = sTmpString;
	else
	{
		cLog::write ( "Can't load FX-Path from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.sFxPath = "fx";
	}
	//Graphics
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Paths","GFX", NULL)))
		cLog::write ( "Can't find Path-GFX-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		SettingsData.sGfxPath = sTmpString;
	else
	{
		cLog::write ( "Can't load GFX-Path from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.sGfxPath = "gfx";
	}
	//Graphics on demmand
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Paths","GFXOD", NULL)))
		cLog::write ( "Can't find Path-GFXOD-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		SettingsData.sGfxODPath = sTmpString;
	else
	{
		cLog::write ( "Can't load GFXOD-Path from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.sGfxODPath = "gfx_od";
	}
	//Maps
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Paths","Maps", NULL)))
		cLog::write ( "Can't find Path-Maps-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		SettingsData.sMapsPath = sTmpString;
	else
	{
		cLog::write ( "Can't load Maps-Path from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.sMapsPath = "maps";
	}
	//Saves
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Paths","Saves", NULL)))
		cLog::write ( "Can't find Path-Saves-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		SettingsData.sSavesPath = sTmpString;
	else
	{
		cLog::write ( "Can't load Saves-Path from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.sSavesPath = "saves";
	}
	//Sounds
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Paths","Sounds", NULL)))
		cLog::write ( "Can't find Path-Sounds-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		SettingsData.sSoundsPath = sTmpString;
	else
	{
		cLog::write ( "Can't load Sounds-Path from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.sSoundsPath = "sounds";
	}
	//Voices
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Paths","Voices", NULL)))
		cLog::write ( "Can't find Path-Voices-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		SettingsData.sVoicesPath = sTmpString;
	else
	{
		cLog::write ( "Can't load Voices-Path from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.sVoicesPath = "voices";
	}
	//Music
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Paths","Music", NULL)))
		cLog::write ( "Can't find Path-Music-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		SettingsData.sMusicPath = sTmpString;
	else
	{
		cLog::write ( "Can't load Music-Path from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.sMusicPath = "music";
	}
	//Terrain
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Paths","Terrain", NULL)))
		cLog::write ( "Can't find Path-Terrain-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		SettingsData.sTerrainPath = sTmpString;
	else
	{
		cLog::write ( "Can't load Terrain-Path from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.sTerrainPath = "terrain";
	}
	//Vehicles
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Paths","Vehicles", NULL)))
		cLog::write ( "Can't find Path-Vehicles-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		SettingsData.sVehiclesPath = sTmpString;
	else
	{
		cLog::write ( "Can't load Vehicles-Path from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.sVehiclesPath = "vehicles";
	}
	//Buildings
	if(!(pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Paths","Buildings", NULL)))
		cLog::write ( "Can't find Path-Buildings-Node in max.xml", LOG_TYPE_WARNING );
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		SettingsData.sBuildingsPath = sTmpString;
	else
	{
		cLog::write ( "Can't load Buildings-Path from max.xml: using default value", LOG_TYPE_WARNING );
		SettingsData.sBuildingsPath = "buildings";
	}

	cLog::write ( "Done", LOG_TYPE_DEBUG );
	return 0;
}

// LoadLanguage ///////////////////////////////////////////////////////////////
// Loads the selected languagepack:
int LoadLanguage()
{
	if( lngPack.SetCurrentLanguage(SettingsData.sLanguage) != 0 )			// Set the language code
	{
		// Not a valid language code, critical fail!
		cLog::write( "Not a valid language code!" , cLog::eLOG_TYPE_ERROR );
		return 0;
	}
	if( lngPack.ReadLanguagePack() != 0 )					// Load the translations
	{
		// Could not load the language, critical fail!
		cLog::write( "Could not load the language!" , cLog::eLOG_TYPE_ERROR );
		return 0;
	}
	return 1;
}

// GenerateMaxXml /////////////////////////////////////////////////////////////
// Generats a new max.xml file
int GenerateMaxXml()
{
	static int iGenerateTrys = 0;
	iGenerateTrys++;
	return -1; // Generate fails
	// return 0; // Generate success
}

// LoadFonts ///////////////////////////////////////////////////////////////////
// Loads all Fonts
int LoadFonts(const char* path)
{
	cLog::write ( "Loading Fonts", LOG_TYPE_INFO );

	if(!LoadGraphicToSurface ( FontsData.font_small_red,path,"font_small_red.pcx" )) LoadingData=LOAD_ERROR;
	if(!LoadGraphicToSurface ( FontsData.font_small_green,path,"font_small_green.pcx" )) LoadingData=LOAD_ERROR;
	if(!LoadGraphicToSurface ( FontsData.font_small_yellow,path,"font_small_yellow.pcx" )) LoadingData=LOAD_ERROR;
	if(!LoadGraphicToSurface ( FontsData.font_big_gold,path,"font_big_gold.pcx" )) LoadingData=LOAD_ERROR;

	if(LoadingData==LOAD_ERROR)
	{
		return 0;
	}
	else
	{
		cLog::write ( "Done", LOG_TYPE_DEBUG );
		return 1;
	}
}

// LoadEffects ///////////////////////////////////////////////////////////////////
// Loads all Effects
int LoadEffects(const char* path)
{
	cLog::write ( "Loading Effects", LOG_TYPE_INFO );

	LoadEffectGraphicToSurface ( EffectsData.fx_explo_small0,path,"explo_small0.pcx" );
	LoadEffectGraphicToSurface ( EffectsData.fx_explo_small1,path,"explo_small1.pcx" );
	LoadEffectGraphicToSurface ( EffectsData.fx_explo_small2,path,"explo_small2.pcx" );
	LoadEffectGraphicToSurface ( EffectsData.fx_explo_big0,path,"explo_big0.pcx" );
	LoadEffectGraphicToSurface ( EffectsData.fx_explo_big1,path,"explo_big1.pcx" );
	LoadEffectGraphicToSurface ( EffectsData.fx_explo_big2,path,"explo_big2.pcx" );
	LoadEffectGraphicToSurface ( EffectsData.fx_explo_big3,path,"explo_big3.pcx" );
	LoadEffectGraphicToSurface ( EffectsData.fx_explo_big4,path,"explo_big4.pcx" );
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

	cLog::write ( "Done", LOG_TYPE_DEBUG );
	return 1;
}

// LoadMusic ///////////////////////////////////////////////////////////////////
// Loads all Musicfiles
int LoadMusic(const char* path)
{
	cLog::write ( "Loading music", LOG_TYPE_INFO );
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
		LoadingData=LOAD_ERROR;
		return 0;
	}
	if(!(MusicXml.LoadFile(sTmpString.c_str())))
	{
		cLog::write ( "Can't load music.xml ", LOG_TYPE_ERROR );
		LoadingData=LOAD_ERROR;
		return 0;
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(MusicXml,"Music","Menus","main", NULL);
	if(!(pXmlNode->XmlReadNodeData(MainMusicFile,ExTiXmlNode::eXML_ATTRIBUTE,"Text")))
	{
		cLog::write ( "Can't find \"main\" in music.xml ", LOG_TYPE_ERROR );
		LoadingData=LOAD_ERROR;
		return 0;
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(MusicXml,"Music","Menus","credits", NULL);
	if(!(pXmlNode->XmlReadNodeData(CreditsMusicFile,ExTiXmlNode::eXML_ATTRIBUTE,"Text")))
	{
		cLog::write ( "Can't find \"credits\" in music.xml ", LOG_TYPE_ERROR );
		LoadingData=LOAD_ERROR;
		return 0;
	}


	pXmlNode = pXmlNode->XmlGetFirstNode(MusicXml,"Music","Game","bkgcount", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Num"))
		MusicAnz = atoi(sTmpString.c_str());
	else
	{
		cLog::write ( "Can't find \"bkgcount\" in music.xml ", LOG_TYPE_ERROR );
		LoadingData=LOAD_ERROR;
		return 0;
	}
	
		
	
	MusicFiles = new TList;
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
			cLog::write ( sTmpString.c_str(), LOG_TYPE_WARNING );
			continue;
		}
		if(!FileExists(sTmpString.c_str()))
			continue;
		MusicFiles->Add ( sTmpString );
	}

	cLog::write ( "Done", LOG_TYPE_DEBUG );
	return 1;
}

// LoadSounds ///////////////////////////////////////////////////////////////////
// Loads all Sounds
int LoadSounds(const char* path)
{
	cLog::write ( "Loading Sounds", LOG_TYPE_INFO );

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

	cLog::write ( "Done", LOG_TYPE_DEBUG );
	return 1;
}

// LoadVoices ///////////////////////////////////////////////////////////////////
// Loads all Voices
int LoadVoices(const char* path)
{
	cLog::write ( "Loading Voices", LOG_TYPE_INFO );

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

	cLog::write ( "Done", LOG_TYPE_DEBUG );
	return 1;
}

// LoadGFX ///////////////////////////////////////////////////////////////////
// Loads all graphics
int LoadGraphics(const char* path)
{
	//FIXME: missing sanity checks for graphics
	cLog::write ( "Loading Graphics", LOG_TYPE_INFO );
	string stmp;

	cLog::write ( "Gamegraphics...", LOG_TYPE_DEBUG );
	LoadGraphicToSurface ( GraphicsData.gfx_Chand,path,"hand.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_Cno,path,"no.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_Cselect,path,"select.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_Cmove,path,"move.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_Chelp,path,"help.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_Cattack,path,"attack.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_Cpfeil1,path,"pf_1.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_Cpfeil2,path,"pf_2.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_Cpfeil3,path,"pf_3.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_Cpfeil4,path,"pf_4.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_Cpfeil6,path,"pf_6.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_Cpfeil7,path,"pf_7.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_Cpfeil8,path,"pf_8.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_Cpfeil9,path,"pf_9.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_hud_stuff,path,"hud_stuff.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_praefer,path,"praefer.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_help_screen,path,"help_screen.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_object_menu,path,"object_menu.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_destruction,path,"destruction.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_build_screen,path,"build_screen.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_fac_build_screen,path,"fac_build_screen.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_Cband,path,"band_cur.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_band_small,path,"band_small.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_band_big,path,"band_big.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_band_small_org,path,"band_small.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_band_big_org,path,"band_big.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_big_beton_org,path,"big_beton.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_big_beton,path,"big_beton.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_Ctransf,path,"transf.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_transfer,path,"transfer.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_mine_manager,path,"mine_manager.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_Cload,path,"load.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_Cactivate,path,"activate.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_storage,path,"storage.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_storage_ground,path,"storage_ground.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_dialog,path,"dialog.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_edock,path,"edock.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_Cmuni,path,"muni.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_Crepair,path,"repair.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_research,path,"research.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_upgrade,path,"upgrade.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_panel_top,path,"panel_top.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_panel_bottom,path,"panel_bottom.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_Csteal,path,"steal.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_Cdisable,path,"disable.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_menu_stuff,path,"menu_stuff.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_player_pc,path,"player_pc.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_player_human,path,"player_human.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_player_none,path,"player_none.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_load_save_menu,path,"load_save_menu.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_exitpoints_org,path,"activate_field.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_exitpoints,path,"activate_field.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_player_select,path,"customgame_menu.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_menu_buttons,path,"menu_buttons.pcx" );

	CheckFile(path,"dialog.pcx");
	CheckFile(path,"dialog2.pcx");
	CheckFile(path,"dialog3.pcx");

	// Hud:
	cLog::write ( "Hudgraphics...", LOG_TYPE_DEBUG );
	SDL_Rect scr,dest;
	GraphicsData.gfx_hud = SDL_CreateRGBSurface ( SDL_HWSURFACE, SettingsData.iScreenW,
		SettingsData.iScreenH, SettingsData.iColourDepth, 0, 0, 0, 0 );
	SDL_FillRect ( GraphicsData.gfx_hud, NULL, 0xFF00FF );
	SDL_SetColorKey ( GraphicsData.gfx_hud, SDL_SRCCOLORKEY, 0xFF00FF );

	LoadGraphicToSurface ( GraphicsData.gfx_tmp, path, "hud_left.pcx" );
	SDL_BlitSurface ( GraphicsData.gfx_tmp, NULL, GraphicsData.gfx_hud, NULL );
	SDL_FreeSurface ( GraphicsData.gfx_tmp );

	LoadGraphicToSurface ( GraphicsData.gfx_tmp,path,"hud_top.pcx" );
	scr.x = 0;
	scr.y=0;
	scr.w = GraphicsData.gfx_tmp->w;
	scr.h = GraphicsData.gfx_tmp->h;
	dest.x = 180;
	dest.y = 0;
	dest.w = GraphicsData.gfx_hud->w-180;
	dest.h = 18;
	SDL_BlitSurface ( GraphicsData.gfx_tmp, &scr, GraphicsData.gfx_hud, &dest );
	scr.x = 1275;
	scr.w = 18;
	scr.h = 18;
	dest.x = GraphicsData.gfx_hud->w-18;
	SDL_BlitSurface ( GraphicsData.gfx_tmp, &scr, GraphicsData.gfx_hud, &dest );
	SDL_FreeSurface ( GraphicsData.gfx_tmp );

	LoadGraphicToSurface ( GraphicsData.gfx_tmp, path, "hud_right.pcx" );
	scr.x = 0;
	scr.y = 0;
	scr.w = GraphicsData.gfx_tmp->w;
	scr.h = GraphicsData.gfx_tmp->h;
	dest.x = GraphicsData.gfx_hud->w-12;
	dest.y = 18;
	dest.w = 12;
	dest.h = GraphicsData.gfx_hud->h;
	SDL_BlitSurface ( GraphicsData.gfx_tmp, &scr, GraphicsData.gfx_hud,&dest );
	SDL_FreeSurface ( GraphicsData.gfx_tmp );

	LoadGraphicToSurface ( GraphicsData.gfx_tmp,path,"hud_bottom.pcx" );
	scr.x = 0;
	scr.y = 0;
	scr.w = GraphicsData.gfx_tmp->w;
	scr.h = GraphicsData.gfx_tmp->h;
	dest.x = 180;
	dest.y = GraphicsData.gfx_hud->h-24;
	dest.w = GraphicsData.gfx_hud->w-180;
	dest.h = 24;
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
		LoadGraphicToSurface ( GraphicsData.gfx_tmp, path, "logo.pcx" );
		dest.x = 9;
		dest.y = SettingsData.iScreenH-32-15;
		dest.w = 152;
		dest.h = 32;
		SDL_BlitSurface ( GraphicsData.gfx_tmp,NULL,GraphicsData.gfx_hud,&dest );
		SDL_FreeSurface ( GraphicsData.gfx_tmp );
	}

	cLog::write ( "Colourgraphics...", LOG_TYPE_DEBUG );
	// Farben:
	OtherData.colors= ( SDL_Surface** ) malloc ( sizeof ( SDL_Surface* ) *8 );
	if(!OtherData.colors) { cLog::write("Out of memory", cLog::eLOG_TYPE_MEM); }
	LoadGraphicToSurface ( OtherData.colors[cl_red],path,"cl_red.pcx" );
	LoadGraphicToSurface ( OtherData.colors[cl_blue],path,"cl_blue.pcx" );
	LoadGraphicToSurface ( OtherData.colors[cl_green],path,"cl_green.pcx" );
	LoadGraphicToSurface ( OtherData.colors[cl_grey],path,"cl_grey.pcx" );
	LoadGraphicToSurface ( OtherData.colors[cl_orange],path,"cl_orange.pcx" );
	LoadGraphicToSurface ( OtherData.colors[cl_yellow],path,"cl_yellow.pcx" );
	LoadGraphicToSurface ( OtherData.colors[cl_purple],path,"cl_purple.pcx" );
	LoadGraphicToSurface ( OtherData.colors[cl_aqua],path,"cl_aqua.pcx" );

	OtherData.ShieldColors= ( SDL_Surface** ) malloc ( sizeof ( SDL_Surface* ) *8 );
	if(!OtherData.ShieldColors) { cLog::write("Out of memory", cLog::eLOG_TYPE_MEM); }
	MakeShieldColor ( & ( OtherData.ShieldColors[0] ),OtherData.colors[0] );
	MakeShieldColor ( & ( OtherData.ShieldColors[1] ),OtherData.colors[1] );
	MakeShieldColor ( & ( OtherData.ShieldColors[2] ),OtherData.colors[2] );
	MakeShieldColor ( & ( OtherData.ShieldColors[3] ),OtherData.colors[3] );
	MakeShieldColor ( & ( OtherData.ShieldColors[4] ),OtherData.colors[4] );
	MakeShieldColor ( & ( OtherData.ShieldColors[5] ),OtherData.colors[5] );
	MakeShieldColor ( & ( OtherData.ShieldColors[6] ),OtherData.colors[6] );
	MakeShieldColor ( & ( OtherData.ShieldColors[7] ),OtherData.colors[7] );

	cLog::write ( "Shadowgraphics...", LOG_TYPE_DEBUG );
	// Shadow:
	GraphicsData.gfx_shadow = SDL_CreateRGBSurface ( SDL_HWSURFACE, SettingsData.iScreenW,
		SettingsData.iScreenH, SettingsData.iColourDepth, 0, 0, 0, 0 );
	SDL_FillRect ( GraphicsData.gfx_shadow, NULL, 0x0 );
	SDL_SetAlpha ( GraphicsData.gfx_shadow, SDL_SRCALPHA, 50 );
	GraphicsData.gfx_tmp = SDL_CreateRGBSurface ( SDL_HWSURFACE, 128, 128, SettingsData.iColourDepth, 0, 0, 0, 0 );
	SDL_SetColorKey ( GraphicsData.gfx_tmp, SDL_SRCCOLORKEY, 0xFF00FF );

	// Glas:
	cLog::write ( "Glassgraphic...", LOG_TYPE_DEBUG );
	LoadGraphicToSurface ( GraphicsData.gfx_destruction_glas, path, "destruction_glas.pcx" );
	SDL_SetAlpha ( GraphicsData.gfx_destruction_glas, SDL_SRCALPHA, 150 );

	// Waypoints:
	cLog::write ( "Waypointgraphics...", LOG_TYPE_DEBUG );
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
	cLog::write ( "Resourcegraphics...", LOG_TYPE_DEBUG );
	LoadGraphicToSurface ( ResourceData.res_metal_org,path,"res.pcx" );
	SDL_SetColorKey ( ResourceData.res_metal_org,SDL_SRCCOLORKEY,-1 );
	ResourceData.res_metal=SDL_CreateRGBSurface ( SDL_HWSURFACE,ResourceData.res_metal_org->w,ResourceData.res_metal_org->h,SettingsData.iColourDepth,0,0,0,0 );
	SDL_BlitSurface ( ResourceData.res_metal_org,NULL,ResourceData.res_metal,NULL );
	SDL_SetColorKey ( ResourceData.res_metal,SDL_SRCCOLORKEY,0xFFFFFF );

	ResourceData.res_oil_org=SDL_CreateRGBSurface ( SDL_HWSURFACE,ResourceData.res_metal_org->w,ResourceData.res_metal_org->h,SettingsData.iColourDepth,0,0,0,0 );
	SDL_FillRect ( ResourceData.res_oil_org,NULL,0x00FF00 );
	SDL_SetColorKey ( ResourceData.res_oil_org,SDL_SRCCOLORKEY,0xFF00FF );
	SDL_BlitSurface ( ResourceData.res_metal,NULL,ResourceData.res_oil_org,NULL );
	ResourceData.res_oil=SDL_CreateRGBSurface ( SDL_HWSURFACE,ResourceData.res_metal_org->w,ResourceData.res_metal_org->h,SettingsData.iColourDepth,0,0,0,0 );
	SDL_FillRect ( ResourceData.res_oil,NULL,0x00FF00 );
	SDL_SetColorKey ( ResourceData.res_oil,SDL_SRCCOLORKEY,0xFF00FF );
	SDL_BlitSurface ( ResourceData.res_metal,NULL,ResourceData.res_oil,NULL );

	ResourceData.res_gold_org=SDL_CreateRGBSurface ( SDL_HWSURFACE,ResourceData.res_metal_org->w,ResourceData.res_metal_org->h,SettingsData.iColourDepth,0,0,0,0 );
	SDL_FillRect ( ResourceData.res_gold_org,NULL,0xFFFF00 );
	SDL_SetColorKey ( ResourceData.res_gold_org,SDL_SRCCOLORKEY,0xFF00FF );
	SDL_BlitSurface ( ResourceData.res_metal,NULL,ResourceData.res_gold_org,NULL );
	ResourceData.res_gold=SDL_CreateRGBSurface ( SDL_HWSURFACE,ResourceData.res_metal_org->w,ResourceData.res_metal_org->h,SettingsData.iColourDepth,0,0,0,0 );
	SDL_FillRect ( ResourceData.res_gold,NULL,0xFFFF00 );
	SDL_SetColorKey ( ResourceData.res_gold,SDL_SRCCOLORKEY,0xFF00FF );
	SDL_BlitSurface ( ResourceData.res_metal,NULL,ResourceData.res_gold,NULL );

	SDL_SetColorKey ( ResourceData.res_metal,SDL_SRCCOLORKEY,0xFF00FF );

	cLog::write ( "Done", LOG_TYPE_DEBUG );
	return 1;
}


// LoadTerrain ////////////////////////////////////////////////////////////////
// Loads the Terrain
int LoadTerrain(const char* path)
{
	cLog::write ( "Loading Terrain", LOG_TYPE_INFO );
	string sTmpString;
	TiXmlDocument TerrainXml;
	TiXmlNode *pXmlNode;
	ExTiXmlNode *pExXmlNode = NULL;

	sTmpString = path;
	sTmpString += PATH_DELIMITER;
	sTmpString += "terrain.xml";
	if( !FileExists( sTmpString.c_str() ) )
	{
		LoadingData=LOAD_ERROR;
		return 0;
	}
	if ( !TerrainXml.LoadFile ( sTmpString.c_str() ) )
	{
		LoadingData=LOAD_ERROR;
		cLog::write("Can't load terrain.xml!",LOG_TYPE_ERROR);
		return 0;
	}
		
	pXmlNode = TerrainXml.FirstChildElement ( "Terrains" );

	TList *sections;
	sections = new TList();
	pXmlNode = pXmlNode->FirstChildElement();
	if ( pXmlNode )
		sections->Add ( pXmlNode->ToElement()->Value() );
	while ( pXmlNode != NULL)
	{
		pXmlNode=pXmlNode->NextSibling();
		if ( pXmlNode && pXmlNode->Type() ==1 )
		{
			sections->Add ( pXmlNode->ToElement()->Value() );
		}
	}

	for ( int i=0;i<sections->Count;i++ )
	{
		if(!(pExXmlNode = pExXmlNode->XmlGetFirstNode(TerrainXml,"Terrains",sections->Items[i].c_str(), NULL)))
		{
			sTmpString = "Can't read \"\" in terrain.xml"; //FIXME: is it smart to proceed at this point? --beko
			sTmpString.insert(11, sections->Items[i]);
			cLog::write ( sTmpString.c_str(), LOG_TYPE_WARNING );
			continue;
		}
		TerrainData.terrain_anz++;
		TerrainData.terrain= ( sTerrain* ) realloc ( TerrainData.terrain,sizeof ( sTerrain ) *TerrainData.terrain_anz );
		if(!TerrainData.terrain) { cLog::write("Out of memory", cLog::eLOG_TYPE_MEM); }

		
		if(!(pExXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"file")))
		{
			sTmpString = "Can't read fileattribute ";
			sTmpString += sections->Items[i];
			cLog::write ( sTmpString.c_str(), LOG_TYPE_WARNING );
			continue;
		}
		sTmpString.insert(0,PATH_DELIMITER);
		sTmpString.insert(0,path);
		if ( !FileExists ( sTmpString.c_str() ) )
			continue;

		TerrainData.terrain[i].sf_org = LoadPCX ( ( char * ) sTmpString.c_str() );
		DupSurface ( TerrainData.terrain[i].sf_org,TerrainData.terrain[i].sf );

		DupSurface ( TerrainData.terrain[i].sf_org,TerrainData.terrain[i].shw_org );
		SDL_BlitSurface ( GraphicsData.gfx_shadow,NULL,TerrainData.terrain[i].shw_org,NULL );
		DupSurface ( TerrainData.terrain[i].shw_org,TerrainData.terrain[i].shw );

		pExXmlNode = pExXmlNode->XmlGetFirstNode(TerrainXml,"Terrains",sections->Items[i].c_str(), NULL);
		if(pExXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"water"))
			TerrainData.terrain[i].water = pExXmlNode->XmlDataToBool(sTmpString);
		else
		{
			#ifdef TERRAIN_DEBUG
			sTmpString = "Attribute water of \"\" not found: using default value";
			sTmpString.insert(20,sections->Items[i]);
			cLog::write ( sTmpString.c_str(), LOG_TYPE_WARNING );
			#endif
			TerrainData.terrain[i].water = false;
		}
		pExXmlNode = pExXmlNode->XmlGetFirstNode(TerrainXml,"Terrains",sections->Items[i].c_str(), NULL);
		if(pExXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"blocked"))
			TerrainData.terrain[i].blocked = pExXmlNode->XmlDataToBool(sTmpString);
		else
		{
			#ifdef TERRAIN_DEBUG
			sTmpString = "Attribute blocked of \"\" not found: using default value";
			sTmpString.insert(22,sections->Items[i]);
			cLog::write ( sTmpString.c_str(), LOG_TYPE_WARNING );
			#endif
			TerrainData.terrain[i].blocked = false;
		}
		pExXmlNode = pExXmlNode->XmlGetFirstNode(TerrainXml,"Terrains",sections->Items[i].c_str(), NULL);
		if(pExXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"coast"))
			TerrainData.terrain[i].coast = pExXmlNode->XmlDataToBool(sTmpString);
		else
		{
			#ifdef TERRAIN_DEBUG
			sTmpString = "Attribute coast of \"\" not found: using default value";
			sTmpString.insert(20,sections->Items[i]);
			cLog::write ( sTmpString.c_str(), LOG_TYPE_WARNING );
			#endif
			TerrainData.terrain[i].coast = false;
		}
		pExXmlNode = pExXmlNode->XmlGetFirstNode(TerrainXml,"Terrains",sections->Items[i].c_str(), NULL);
		if(pExXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"overlay"))
			TerrainData.terrain[i].overlay = pExXmlNode->XmlDataToBool(sTmpString);
		else
		{
			#ifdef TERRAIN_DEBUG
			sTmpString = "Attribute overlay of \"\" not found: using default value";
			sTmpString.insert(22,sections->Items[i]);
			cLog::write ( sTmpString.c_str(), LOG_TYPE_WARNING );
			#endif
			TerrainData.terrain[i].overlay = false;
		}

		TerrainData.terrain[i].frames = TerrainData.terrain[i].sf_org->w/64;
		TerrainData.terrain[i].id= ( char* ) malloc ( ( sections->Items[i] ).length() +1 );
		if(!TerrainData.terrain[i].id) { cLog::write("Out of memory", cLog::eLOG_TYPE_MEM); }
		strcpy ( TerrainData.terrain[i].id, ( sections->Items[i] ).c_str() );

		if ( TerrainData.terrain[i].overlay )
		{
			int t=0xFFCD00CD;
			SDL_SetColorKey ( TerrainData.terrain[i].sf_org,SDL_SRCCOLORKEY,0xFF00FF );
			SDL_SetColorKey ( TerrainData.terrain[i].shw_org,SDL_SRCCOLORKEY,t );
			SDL_SetColorKey ( TerrainData.terrain[i].sf,SDL_SRCCOLORKEY,0xFF00FF );
			SDL_SetColorKey ( TerrainData.terrain[i].shw,SDL_SRCCOLORKEY,t );
		}
	}
	delete sections;
	cLog::write ( "Done", LOG_TYPE_DEBUG );
	return 1;
}

// DupSurface ////////////////////////////////////////////////////////////////
void DupSurface(SDL_Surface *&src,SDL_Surface *&dest)
{
	dest = SDL_CreateRGBSurface(SDL_HWSURFACE|SDL_SRCCOLORKEY,src->w,src->h,SettingsData.iColourDepth,0,0,0,0);
	SDL_FillRect(dest,NULL,0xFF00FF);
	SDL_BlitSurface(src,NULL,dest,NULL);
	return;
}

// LoadVehicles ///////////////////////////////////////////////////////////////
// Loads all Vehicles
int LoadVehicles()
{
	cLog::write ( "Loading Vehicles", LOG_TYPE_INFO );

	string sTmpString, sVehiclePath;
	char sztmp[16];
	const char *pszTmp;
	TList *VehicleList;
	TiXmlDocument VehiclesXml;
	TiXmlNode *pXmlNode;
	TiXmlElement * pXmlElement;
	ExTiXmlNode *pExXmlNode = NULL;

	sTmpString = SettingsData.sVehiclesPath;
	sTmpString += PATH_DELIMITER;
	sTmpString += "vehicles.xml";
	if( !FileExists( sTmpString.c_str() ) )
	{
		LoadingData=LOAD_ERROR;
		return 0;
	}
	if ( !VehiclesXml.LoadFile ( sTmpString.c_str() ) )
	{
		LoadingData=LOAD_ERROR;
		cLog::write("Can't load vehicles.xml!",LOG_TYPE_ERROR);
		return 0;
	}
	if(!(pXmlNode = VehiclesXml.FirstChildElement ( "VehicleData" )->FirstChildElement ( "Vehicles" )))
	{
		LoadingData=LOAD_ERROR;
		cLog::write("Can't read \"VehicleData->Vehicles\" node!",LOG_TYPE_ERROR);
		return 0;
	}
	// read vehicles.xml
	VehicleList = new TList();
	pXmlNode = pXmlNode->FirstChildElement();
	pXmlElement = pXmlNode->ToElement();
	if ( pXmlElement )
	{
		pszTmp = pXmlElement->Attribute( "directory" );
		if(pszTmp != 0)
			VehicleList->Add ( pszTmp );
		else
		{
			sTmpString = "Can't read dierectory-attribute from \"\" - node";
			sTmpString.insert(38,pXmlNode->Value());
			cLog::write(sTmpString.c_str(),LOG_TYPE_WARNING);
		}
	}
	else
		cLog::write("No vehicles defined in vehicles.xml!",LOG_TYPE_WARNING);
	while ( pXmlNode != NULL)
	{
		pXmlNode = pXmlNode->NextSibling();
		if ( pXmlNode == NULL)
			break;
		if( pXmlNode->Type() !=1 )
			continue;
		pszTmp = pXmlNode->ToElement()->Attribute( "directory" );
		if(pszTmp != 0)
			VehicleList->Add ( pszTmp );
		else
		{
			sTmpString = "Can't read dierectory-attribute from \"\" - node";
			sTmpString.insert(38,pXmlNode->Value());
			cLog::write(sTmpString.c_str(),LOG_TYPE_WARNING);
		}
	}
	// load found units
	UnitsData.vehicle_anz = 0;
	for( int i = 0; i < VehicleList->Count; i++)
	{
		sVehiclePath = SettingsData.sVehiclesPath;
		sVehiclePath += PATH_DELIMITER;
		sVehiclePath += VehicleList->Items[i];
		sVehiclePath += PATH_DELIMITER;

		// Prepare memory for next unit
		UnitsData.vehicle = ( sVehicle* ) realloc ( UnitsData.vehicle, sizeof ( sVehicle ) * (UnitsData.vehicle_anz+1));
		if(!UnitsData.vehicle) { cLog::write("Out of memory", cLog::eLOG_TYPE_MEM); }


		// Set default data-values
		SetDefaultUnitData(UnitsData.vehicle_anz, true);
		// Load Data from data.xml
		LoadUnitData(UnitsData.vehicle_anz,sVehiclePath.c_str(), true);

		// laod infantery graphics
		if(UnitsData.vehicle[UnitsData.vehicle_anz].data.bAnimation_Movement)
		{
			SDL_Surface *sfTempSurface;
			SDL_Rect rcDest;
			for(int n = 0; n < 8; n++)
			{
				for ( int j = 0; j < 13; j++ )
				{
					sTmpString = sVehiclePath;
					sprintf(sztmp,"img%d_%0.2d.pcx",n,j);
					sTmpString += sztmp;
					if(FileExists(sTmpString.c_str()))
					{
						sfTempSurface = LoadPCX ( (char *) sTmpString.c_str() );
						rcDest.x = 64*j + 32 - sfTempSurface->w/2;
						rcDest.y = 32 - sfTempSurface->h/2;
						rcDest.w = sfTempSurface->w;
						rcDest.h = sfTempSurface->h;
						SDL_BlitSurface ( sfTempSurface, NULL, UnitsData.vehicle[UnitsData.vehicle_anz].img[n], &rcDest );
						SDL_FreeSurface ( sfTempSurface );
					}
				}
				UnitsData.vehicle[UnitsData.vehicle_anz].img_org[n] = SDL_CreateRGBSurface ( SDL_HWSURFACE | SDL_SRCCOLORKEY, 64*13, 64, SettingsData.iColourDepth, 0, 0, 0, 0 );
				SDL_SetColorKey ( UnitsData.vehicle[UnitsData.vehicle_anz].img[n], SDL_SRCCOLORKEY, 0xFFFFFF );
				SDL_FillRect ( UnitsData.vehicle[UnitsData.vehicle_anz].img_org[n], NULL, 0xFFFFFF );
				SDL_BlitSurface ( UnitsData.vehicle[UnitsData.vehicle_anz].img[n],NULL,UnitsData.vehicle[UnitsData.vehicle_anz].img_org[n], NULL );

				UnitsData.vehicle[UnitsData.vehicle_anz].shw[n] = SDL_CreateRGBSurface ( SDL_HWSURFACE | SDL_SRCCOLORKEY, 64*13, 64, SettingsData.iColourDepth, 0, 0, 0, 0 );
				SDL_SetColorKey (UnitsData.vehicle[UnitsData.vehicle_anz].shw[n], SDL_SRCCOLORKEY, 0xFF00FF );
				SDL_FillRect ( UnitsData.vehicle[UnitsData.vehicle_anz].shw[n], NULL, 0xFF00FF );
				UnitsData.vehicle[UnitsData.vehicle_anz].shw_org[n] = SDL_CreateRGBSurface ( SDL_HWSURFACE | SDL_SRCCOLORKEY, 64*13, 64, SettingsData.iColourDepth, 0, 0, 0, 0 );
				SDL_SetColorKey ( UnitsData.vehicle[UnitsData.vehicle_anz].shw_org[n], SDL_SRCCOLORKEY, 0xFF00FF );
				SDL_FillRect ( UnitsData.vehicle[UnitsData.vehicle_anz].shw_org[n], NULL, 0xFF00FF );

				int *ptr;
				rcDest.x=3;
				rcDest.y=3;
				rcDest.w=64*13;
				rcDest.h=64;
				SDL_BlitSurface ( UnitsData.vehicle[UnitsData.vehicle_anz].img_org[n], NULL, UnitsData.vehicle[UnitsData.vehicle_anz].shw_org[n], &rcDest );
				SDL_LockSurface ( UnitsData.vehicle[UnitsData.vehicle_anz].shw_org[n] );
				ptr = ( int* ) ( UnitsData.vehicle[UnitsData.vehicle_anz].shw_org[n]->pixels );
				for ( int j = 0; j < 64*13*64; j++ )
				{
					if ( *ptr != 0xFF00FF )
						*ptr=0;
					ptr++;
				}
				SDL_UnlockSurface ( UnitsData.vehicle[UnitsData.vehicle_anz].shw_org[n] );
				SDL_BlitSurface ( UnitsData.vehicle[UnitsData.vehicle_anz].shw_org[n], NULL, UnitsData.vehicle[UnitsData.vehicle_anz].shw[n], NULL );
				SDL_SetAlpha ( UnitsData.vehicle[UnitsData.vehicle_anz].shw_org[n], SDL_SRCALPHA, 50 );
				SDL_SetAlpha ( UnitsData.vehicle[UnitsData.vehicle_anz].shw[n], SDL_SRCALPHA, 50 );
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
				if(FileExists(sTmpString.c_str()))
				{
					UnitsData.vehicle[UnitsData.vehicle_anz].img_org[n] = LoadPCX ( (char *) sTmpString.c_str() );
					SDL_SetColorKey ( UnitsData.vehicle[UnitsData.vehicle_anz].img_org[n],SDL_SRCCOLORKEY,0xFFFFFF );
					UnitsData.vehicle[UnitsData.vehicle_anz].img[n] = LoadPCX ( (char *) sTmpString.c_str() );
					SDL_SetColorKey ( UnitsData.vehicle[UnitsData.vehicle_anz].img[n],SDL_SRCCOLORKEY,0xFFFFFF );
				}

				// load shadow
				sTmpString.replace(sTmpString.length()-8,3,"shw");
				if(FileExists(sTmpString.c_str()))
				{
					UnitsData.vehicle[UnitsData.vehicle_anz].shw_org[n] = LoadPCX ( (char *) sTmpString.c_str() );
					SDL_SetColorKey ( UnitsData.vehicle[UnitsData.vehicle_anz].shw_org[n],SDL_SRCCOLORKEY,0xFFFFFF );
					UnitsData.vehicle[UnitsData.vehicle_anz].shw[n] = LoadPCX ( (char *) sTmpString.c_str() );
					SDL_SetAlpha ( UnitsData.vehicle[UnitsData.vehicle_anz].shw[n],SDL_SRCALPHA,50 );
					SDL_SetColorKey ( UnitsData.vehicle[UnitsData.vehicle_anz].shw[n],SDL_SRCCOLORKEY,0xFFFFFF );
				}
			}
		}
		// load video
		sTmpString = sVehiclePath;
		sTmpString += "video.flc";
		if(FileExists(sTmpString.c_str()))
		{
			UnitsData.vehicle[UnitsData.vehicle_anz].FLCFile= ( char* ) malloc ( sVehiclePath.length() +1 );
			if(!UnitsData.vehicle[UnitsData.vehicle_anz].FLCFile) { cLog::write("Out of memory", cLog::eLOG_TYPE_MEM); }
			strcpy ( UnitsData.vehicle[UnitsData.vehicle_anz].FLCFile,sVehiclePath.c_str() );
		}

		// load infoimage
		sTmpString = sVehiclePath;
		sTmpString += "info.pcx";
		if(FileExists(sTmpString.c_str()))
			UnitsData.vehicle[UnitsData.vehicle_anz].info = LoadPCX ( (char *) sTmpString.c_str() );

		// load storageimage
		sTmpString = sVehiclePath;
		sTmpString += "store.pcx";
		if(FileExists(sTmpString.c_str()))
			UnitsData.vehicle[UnitsData.vehicle_anz].storage = LoadPCX ( (char *) sTmpString.c_str() );

		// load overlaygraphics if necessary
		if(UnitsData.vehicle[UnitsData.vehicle_anz].data.bHas_Overlay)
		{
			sTmpString = sVehiclePath;
			sTmpString += "overlay.pcx";
			if(FileExists(sTmpString.c_str()))
			{
				UnitsData.vehicle[UnitsData.vehicle_anz].overlay_org = LoadPCX ( (char *) sTmpString.c_str() );
				UnitsData.vehicle[UnitsData.vehicle_anz].overlay = LoadPCX ( (char *) sTmpString.c_str() );
			}
		}
		else
		{
			UnitsData.vehicle[UnitsData.vehicle_anz].overlay_org = NULL;
			UnitsData.vehicle[UnitsData.vehicle_anz].overlay = NULL;
		}

		// load buildgraphics if necessary
		if(UnitsData.vehicle[UnitsData.vehicle_anz].data.bBuild_Up_Grafic)
		{
			// load image
			sTmpString = sVehiclePath;
			sTmpString += "build.pcx";
			if(FileExists(sTmpString.c_str()))
			{
				UnitsData.vehicle[UnitsData.vehicle_anz].build_org = LoadPCX ( (char *) sTmpString.c_str() );
				UnitsData.vehicle[UnitsData.vehicle_anz].build = LoadPCX ( (char *) sTmpString.c_str() );
			}
			// load shadow
			sTmpString = sVehiclePath;
			sTmpString += "build_shw.pcx";
			if(FileExists(sTmpString.c_str()))
			{
				UnitsData.vehicle[UnitsData.vehicle_anz].build_shw_org = LoadPCX ( (char *) sTmpString.c_str() );
				UnitsData.vehicle[UnitsData.vehicle_anz].build_shw = LoadPCX ( (char *) sTmpString.c_str() );
			}
		}
		else
		{
			UnitsData.vehicle[UnitsData.vehicle_anz].build_org = NULL;
			UnitsData.vehicle[UnitsData.vehicle_anz].build = NULL;
			UnitsData.vehicle[UnitsData.vehicle_anz].build_shw_org = NULL;
			UnitsData.vehicle[UnitsData.vehicle_anz].build_shw = NULL;
		}
		// load cleargraphics if necessary
		if(UnitsData.vehicle[UnitsData.vehicle_anz].data.bCan_Clear_Area)
		{
			// load image (small)
			sTmpString = sVehiclePath;
			sTmpString += "clear_small.pcx";
			if(FileExists(sTmpString.c_str()))
			{
				UnitsData.vehicle[UnitsData.vehicle_anz].clear_small_org = LoadPCX ( (char *) sTmpString.c_str() );
				UnitsData.vehicle[UnitsData.vehicle_anz].clear_small = LoadPCX ( (char *) sTmpString.c_str() );
			}
			// load shadow (small)
			sTmpString = sVehiclePath;
			sTmpString += "clear_small_shw.pcx";
			if(FileExists(sTmpString.c_str()))
			{
				UnitsData.vehicle[UnitsData.vehicle_anz].clear_small_shw_org = LoadPCX ( (char *) sTmpString.c_str() );
				UnitsData.vehicle[UnitsData.vehicle_anz].clear_small_shw = LoadPCX ( (char *) sTmpString.c_str() );
			}
			// load image (big)
			sTmpString = sVehiclePath;
			sTmpString += "clear_big.pcx";
			if(FileExists(sTmpString.c_str()))
			{
				UnitsData.vehicle[UnitsData.vehicle_anz].build_org = LoadPCX ( (char *) sTmpString.c_str() );
				UnitsData.vehicle[UnitsData.vehicle_anz].build = LoadPCX ( (char *) sTmpString.c_str() );
			}
			// load shadow (big)
			sTmpString = sVehiclePath;
			sTmpString += "clear_big_shw.pcx";
			if(FileExists(sTmpString.c_str()))
			{
				UnitsData.vehicle[UnitsData.vehicle_anz].build_shw_org = LoadPCX ( (char *) sTmpString.c_str() );
				UnitsData.vehicle[UnitsData.vehicle_anz].build_shw = LoadPCX ( (char *) sTmpString.c_str() );
			}
		}
		else
		{
			UnitsData.vehicle[UnitsData.vehicle_anz].clear_small_org = NULL;
			UnitsData.vehicle[UnitsData.vehicle_anz].clear_small = NULL;
			UnitsData.vehicle[UnitsData.vehicle_anz].clear_small_shw_org = NULL;
			UnitsData.vehicle[UnitsData.vehicle_anz].clear_small_shw = NULL;
		}

		// load sounds
		LoadUnitSoundfile(UnitsData.vehicle[UnitsData.vehicle_anz].Wait,sVehiclePath.c_str(),"wait.wav");
		LoadUnitSoundfile(UnitsData.vehicle[UnitsData.vehicle_anz].WaitWater,sVehiclePath.c_str(),"wait_water.wav");
		LoadUnitSoundfile(UnitsData.vehicle[UnitsData.vehicle_anz].Start,sVehiclePath.c_str(),"start.wav");
		LoadUnitSoundfile(UnitsData.vehicle[UnitsData.vehicle_anz].StartWater,sVehiclePath.c_str(),"start_water.wav");
		LoadUnitSoundfile(UnitsData.vehicle[UnitsData.vehicle_anz].Stop,sVehiclePath.c_str(),"stop.wav");
		LoadUnitSoundfile(UnitsData.vehicle[UnitsData.vehicle_anz].StopWater,sVehiclePath.c_str(),"stop_water.wav");
		LoadUnitSoundfile(UnitsData.vehicle[UnitsData.vehicle_anz].Drive,sVehiclePath.c_str(),"drive.wav");
		LoadUnitSoundfile(UnitsData.vehicle[UnitsData.vehicle_anz].DriveWater,sVehiclePath.c_str(),"drive_water.wav");
		LoadUnitSoundfile(UnitsData.vehicle[UnitsData.vehicle_anz].Attack,sVehiclePath.c_str(),"attack.wav");

		UnitsData.vehicle_anz++;
	}

	cLog::write ( "Done", LOG_TYPE_DEBUG );
	return 1;
}

// LoadBuildings //////////////////////////////////////////////////////////////
// Loads all Buildings
int LoadBuildings()
{
	cLog::write ( "Loading Buildings", LOG_TYPE_INFO );

	string sTmpString, sBuildingPath;
	const char *pszTmp;
	TList *BuildingList;
	TiXmlDocument BuildingsXml;
	TiXmlNode *pXmlNode;
	TiXmlElement * pXmlElement;
	ExTiXmlNode *pExXmlNode = NULL;

	// read buildings.xml
	sTmpString = SettingsData.sBuildingsPath;
	sTmpString += PATH_DELIMITER;
	sTmpString += "buildings.xml";
	if( !FileExists( sTmpString.c_str() ) )
	{
		LoadingData=LOAD_ERROR;
		return 0;
	}
	if ( !BuildingsXml.LoadFile ( sTmpString.c_str() ) )
	{
		LoadingData=LOAD_ERROR;
		cLog::write("Can't load buildings.xml!",LOG_TYPE_ERROR);
		return 0;
	}
	if(!(pXmlNode = BuildingsXml.FirstChildElement ( "BuildingsData" )->FirstChildElement ( "Buildings" )))
	{
		LoadingData=LOAD_ERROR;
		cLog::write("Can't read \"BuildingData->Building\" node!",LOG_TYPE_ERROR);
		return 0;
	}
	BuildingList = new TList();
	pXmlNode = pXmlNode->FirstChildElement();
	pXmlElement = pXmlNode->ToElement();
	if ( pXmlElement )
	{
		pszTmp = pXmlElement->Attribute( "directory" );
		if(pszTmp != 0)
			BuildingList->Add ( pszTmp );
		else
		{
			sTmpString = "Can't read dierectory-attribute from \"\" - node";
			sTmpString.insert(38,pXmlNode->Value());
			cLog::write(sTmpString.c_str(),LOG_TYPE_WARNING);
		}
	}
	else
		cLog::write("No buildings defined in buildings.xml!",LOG_TYPE_WARNING);
	while ( pXmlNode != NULL)
	{
		pXmlNode = pXmlNode->NextSibling();
		if ( pXmlNode == NULL)
			break;
		if( pXmlNode->Type() !=1 )
			continue;
		pszTmp = pXmlNode->ToElement()->Attribute( "directory" );
		if(pszTmp != 0)
			BuildingList->Add ( pszTmp );
		else
		{
			sTmpString = "Can't read dierectory-attribute from \"\" - node";
			sTmpString.insert(38,pXmlNode->Value());
			cLog::write(sTmpString.c_str(),LOG_TYPE_WARNING);
		}
	}
	// load found units
	UnitsData.building_anz = 0;
	for( int i = 0; i < BuildingList->Count; i++)
	{
		sBuildingPath = SettingsData.sBuildingsPath;
		sBuildingPath += PATH_DELIMITER;
		sBuildingPath += BuildingList->Items[i];
		sBuildingPath += PATH_DELIMITER;

		// Prepare memory for next unit
		UnitsData.building = ( sBuilding* ) realloc ( UnitsData.building,sizeof ( sBuilding ) * (UnitsData.building_anz+1) );
		if(!UnitsData.building) { cLog::write("Out of memory", cLog::eLOG_TYPE_MEM); }


		// Set default data-values
		SetDefaultUnitData(UnitsData.building_anz, false);
		// Load Data from data.xml
		LoadUnitData(UnitsData.building_anz,sBuildingPath.c_str(), false);

		// load img
		sTmpString = sBuildingPath;
		sTmpString += "img.pcx";
		if(FileExists(sTmpString.c_str()))
		{
			UnitsData.building[UnitsData.building_anz].img_org = LoadPCX ( (char *) sTmpString.c_str() );
			SDL_SetColorKey ( UnitsData.building[UnitsData.building_anz].img_org,SDL_SRCCOLORKEY,0xFFFFFF );
			UnitsData.building[UnitsData.building_anz].img = LoadPCX ( (char *) sTmpString.c_str() );
			SDL_SetColorKey ( UnitsData.building[UnitsData.building_anz].img,SDL_SRCCOLORKEY,0xFFFFFF );
		}
		// load shadow
		sTmpString = sBuildingPath;
		sTmpString += "shw.pcx";
		if(FileExists(sTmpString.c_str()))
		{
			UnitsData.building[UnitsData.building_anz].shw_org = LoadPCX ( (char *) sTmpString.c_str() );
			SDL_SetColorKey ( UnitsData.building[UnitsData.building_anz].shw_org,SDL_SRCCOLORKEY,0xFF00FF );
			UnitsData.building[UnitsData.building_anz].shw = LoadPCX ( (char *) sTmpString.c_str() );
			SDL_SetAlpha ( UnitsData.building[UnitsData.building_anz].shw,SDL_SRCALPHA,50 );
			SDL_SetColorKey ( UnitsData.building[UnitsData.building_anz].shw,SDL_SRCCOLORKEY,0xFF00FF );
		}

		// load video
		sTmpString = sBuildingPath;
		sTmpString += "video.pcx";
		if(FileExists(sTmpString.c_str()))
			UnitsData.building[UnitsData.building_anz].video = LoadPCX ( (char *) sTmpString.c_str() );

		// load infoimage
		sTmpString = sBuildingPath;
		sTmpString += "info.pcx";
		if(FileExists(sTmpString.c_str()))
			UnitsData.building[UnitsData.building_anz].info = LoadPCX ( (char *) sTmpString.c_str() );

		// load effectgraphics if necessary
		if(false/*UnitsData.vehicle[UnitsData.vehicle_anz].data.bHasEffect*/)
		{
			sTmpString = sBuildingPath;
			sTmpString += "effect.pcx";
			if(FileExists(sTmpString.c_str()))
			{
				UnitsData.building[UnitsData.building_anz].eff_org = LoadPCX ( (char *) sTmpString.c_str() );
				SDL_SetColorKey ( UnitsData.building[UnitsData.building_anz].eff_org,SDL_SRCCOLORKEY,0xFF00FF );
				UnitsData.building[UnitsData.building_anz].eff = LoadPCX ( (char *) sTmpString.c_str() );
				SDL_SetColorKey ( UnitsData.building[UnitsData.building_anz].eff,SDL_SRCCOLORKEY,0xFF00FF );
				SDL_SetAlpha ( UnitsData.building[UnitsData.building_anz].eff,SDL_SRCALPHA,10 );
			}
		}
		else
		{
			UnitsData.building[UnitsData.building_anz].eff_org = NULL;
			UnitsData.building[UnitsData.building_anz].eff = NULL;
		}

		// load sounds
		LoadUnitSoundfile(UnitsData.building[UnitsData.building_anz].Start,sBuildingPath.c_str(),"start.wav");
		LoadUnitSoundfile(UnitsData.building[UnitsData.building_anz].Running,sBuildingPath.c_str(),"running.wav");
		LoadUnitSoundfile(UnitsData.building[UnitsData.building_anz].Stop,sBuildingPath.c_str(),"stop.wav");
		LoadUnitSoundfile(UnitsData.building[UnitsData.building_anz].Attack,sBuildingPath.c_str(),"attack.wav");

		UnitsData.building_anz++;
	}

	cLog::write ( "Done", LOG_TYPE_DEBUG );
	return 1;
}

// LoadUnitData ////////////////////////////////////////////////////////////////
// Loades the unitdata from the data.xml in the unitfolder
void LoadUnitData(int unitnum, const char *directory, bool vehicle)
{
	char *DataStructure[] = {
		// General
		"Unit","General_Info","Is_Controllable", NULL,
		"Unit","General_Info","Can_Be_Captured", NULL,
		"Unit","General_Info","Can_Be_Disabled", NULL,
		"Unit","General_Info","Size_Length", NULL,
		"Unit","General_Info","Size_Width", NULL,

		// Defence
		"Unit","Defence","Is_Target_Land", NULL,
		"Unit","Defence","Is_Target_Sea", NULL,
		"Unit","Defence","Is_Target_Air", NULL,
		"Unit","Defence","Is_Target_Underwater", NULL,
		"Unit","Defence","Is_Target_Mine", NULL,
		"Unit","Defence","Is_Target_Building", NULL,
		"Unit","Defence","Is_Target_Satellite", NULL,
		"Unit","Defence","Is_Target_WMD", NULL,
		"Unit","Defence","Armor", NULL,
		"Unit","Defence","Hitpoints", NULL,

		// Production
		"Unit","Production","Built_Costs", NULL,
		"Unit","Production","Built_Costs_Max", NULL,
		"Unit","Production","Is_Produced_by", NULL,

		// Weapons
		"Unit","Weapons","Weapon","Turret_Gfx", NULL,
		"Unit","Weapons","Weapon","Shot_Trajectory", NULL,
		"Unit","Weapons","Weapon","Ammo_Type", NULL,
		"Unit","Weapons","Weapon","Ammo_Quantity", NULL,

		"Unit","Weapons","Weapon","Allowed_Targets","Target_Land","Damage", NULL,
		"Unit","Weapons","Weapon","Allowed_Targets","Target_Land","Range", NULL,
		"Unit","Weapons","Weapon","Allowed_Targets","Target_Sea","Damage", NULL,
		"Unit","Weapons","Weapon","Allowed_Targets","Target_Sea","Range", NULL,
		"Unit","Weapons","Weapon","Allowed_Targets","Target_Air","Damage", NULL,
		"Unit","Weapons","Weapon","Allowed_Targets","Target_Air","Range", NULL,
		"Unit","Weapons","Weapon","Allowed_Targets","Target_Mine","Damage", NULL,
		"Unit","Weapons","Weapon","Allowed_Targets","Target_Mine","Range", NULL,
		"Unit","Weapons","Weapon","Allowed_Targets","Target_Submarine","Damage", NULL,
		"Unit","Weapons","Weapon","Allowed_Targets","Target_Submarine","Range", NULL,
		"Unit","Weapons","Weapon","Allowed_Targets","Target_Infantry","Damage", NULL,
		"Unit","Weapons","Weapon","Allowed_Targets","Target_Infantry","Range", NULL,
		"Unit","Weapons","Weapon","Allowed_Targets","Target_WMD","Damage", NULL,
		"Unit","Weapons","Weapon","Allowed_Targets","Target_WMD","Range", NULL,

		"Unit","Weapons","Weapon","Shots", NULL,
		"Unit","Weapons","Weapon","Destination_Area", NULL,
		"Unit","Weapons","Weapon","Destination_Type", NULL,
		"Unit","Weapons","Weapon","Movement_Allowed", NULL,

		// Abilities
		"Unit","Abilities","Can_Clear_Area", NULL,
		"Unit","Abilities","Gets_Experience", NULL,
		"Unit","Abilities","Can_Disable", NULL,
		"Unit","Abilities","Can_Capture", NULL,
		"Unit","Abilities","Can_Dive", NULL,
		"Unit","Abilities","Landing_Type", NULL,
		"Unit","Abilities","Can_Upgrade", NULL,
		"Unit","Abilities","Can_Repair", NULL,
		"Unit","Abilities","Is_Kamikaze", NULL,
		"Unit","Abilities","Is_Infrastructure", NULL,
		"Unit","Abilities","Can_Place_Mines", NULL,
		"Unit","Abilities","Makes_Tracks", NULL,
		"Unit","Abilities","Self_Repair_Type", NULL,
		"Unit","Abilities","Converts_Gold", NULL,
		"Unit","Abilities","Needs_Energy", NULL,
		"Unit","Abilities","Needs_Oil", NULL,
		"Unit","Abilities","Needs_Metal", NULL,
		"Unit","Abilities","Needs_Humans", NULL,
		"Unit","Abilities","Mines_Resources", NULL,
		"Unit","Abilities","Can_Launch_SRBM", NULL,
		"Unit","Abilities","Energy_Shield_Strength", NULL,
		"Unit","Abilities","Energy_Shield_Size", NULL,

		// Scan_Abilities
		"Unit","Scan_Abilities","Range_Sight", NULL,
		"Unit","Scan_Abilities","Range_Air", NULL,
		"Unit","Scan_Abilities","Range_Ground", NULL,
		"Unit","Scan_Abilities","Range_Sea", NULL,
		"Unit","Scan_Abilities","Range_Submarine", NULL,
		"Unit","Scan_Abilities","Range_Mine", NULL,
		"Unit","Scan_Abilities","Range_Infantry", NULL,
		"Unit","Scan_Abilities","Range_Resources", NULL,
		"Unit","Scan_Abilities","Range_Jammer", NULL,

		// Movement
		"Unit","Movement","Movement_Sum", NULL,
		"Unit","Movement","Costs_Air", NULL,
		"Unit","Movement","Costs_Sea", NULL,
		"Unit","Movement","Costs_Submarine", NULL,
		"Unit","Movement","Costs_Ground", NULL,
		"Unit","Movement","Factor_Coast", NULL,
		"Unit","Movement","Factor_Wood", NULL,
		"Unit","Movement","Factor_Road", NULL,
		"Unit","Movement","Factor_Bridge", NULL,
		"Unit","Movement","Factor_Platform", NULL,
		"Unit","Movement","Factor_Monorail", NULL,
		"Unit","Movement","Factor_Wreck", NULL,
		"Unit","Movement","Factor_Mountains", NULL,

		// Storage
		"Unit","Storage","Is_Garage", NULL,
		"Unit","Storage","Capacity_Metal", NULL,
		"Unit","Storage","Capacity_Oil", NULL,
		"Unit","Storage","Capacity_Gold", NULL,
		"Unit","Storage","Capacity_Energy", NULL,
		"Unit","Storage","Capacity_Units_Air", NULL,
		"Unit","Storage","Capacity_Units_Sea", NULL,
		"Unit","Storage","Capacity_Units_Ground", NULL,
		"Unit","Storage","Capacity_Units_Infantry", NULL,
		"Unit","Storage","Can_Use_Unit_As_Garage", NULL,

		// Grafics
		"Unit","Graphic","Has_Overlay", NULL,
		"Unit","Graphic","Animations","Build_Up", NULL,
		"Unit","Graphic","Animations","Movement", NULL,
		"Unit","Graphic","Animations","Power_On", NULL
	};
	sUnitData *Data;
	if(vehicle)
		Data = &UnitsData.vehicle[unitnum].data;
	else
		Data = &UnitsData.building[unitnum].data;
	int i, n, arraycount;
	string sTmpString, sVehicleDataPath, sNodePath;
	char szTmp[8];
	TiXmlDocument VehicleDataXml;
	ExTiXmlNode *pExXmlNode = NULL;

	sVehicleDataPath = directory;
	sVehicleDataPath += "data.xml";
	if( !FileExists( sVehicleDataPath.c_str() ) )
		return ;

	if ( !VehicleDataXml.LoadFile ( sVehicleDataPath.c_str() ) )
	{
		sTmpString = "Can't load data.xml in ";
		sTmpString += directory;
		cLog::write(sTmpString.c_str(),LOG_TYPE_WARNING);
		return ;
	}
	// Read minimal game version, id, name and desciption first!
	if(pExXmlNode = pExXmlNode->XmlGetFirstNode(VehicleDataXml,"Unit", "Header", "Game_Version", NULL))
		if(pExXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"text"))
		{
			//TODO: Check game version
		}
	if(pExXmlNode = pExXmlNode->XmlGetFirstNode(VehicleDataXml,"Unit", NULL))
		if(pExXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"ID"))
		{
			Data->ID.iFirstPart = atoi(sTmpString.substr(0,sTmpString.find(" ",0)).c_str());
			if(Data->ID.iFirstPart == 0)
			{
				for( i = 0; i < UnitsData.vehicle_anz; i++)
				{
					if( UnitsData.vehicle[i].data.ID.iSecondPart == atoi(sTmpString.substr(sTmpString.find(" ",0),sTmpString.length()).c_str()))
					{
						sprintf( szTmp, "unit with id %0.2d %0.2d already exists", UnitsData.vehicle[i].data.ID.iFirstPart, UnitsData.vehicle[i].data.ID.iSecondPart);
						cLog::write(szTmp,LOG_TYPE_WARNING);
						return ;
					}
				}
			}
			else
			{
				for( i = 0; i < UnitsData.building_anz; i++)
				{
					if( UnitsData.building[i].data.ID.iSecondPart == atoi(sTmpString.substr(sTmpString.find(" ",0),sTmpString.length()).c_str()))
					{
						sprintf( szTmp, "unit with id %0.2d %0.2d already exists", UnitsData.vehicle[i].data.ID.iFirstPart, UnitsData.vehicle[i].data.ID.iSecondPart);
						cLog::write(szTmp,LOG_TYPE_WARNING);
						return ;
					}
				}
			}
			Data->ID.iSecondPart = atoi(sTmpString.substr(sTmpString.find(" ",0),sTmpString.length()).c_str());
		}
	if(pExXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"name"))
	{
		Data->szName = (char *)malloc(sTmpString.length());
		if(!Data->szName) { cLog::write("Out of memory", cLog::eLOG_TYPE_MEM); }
		strcpy(Data->szName,sTmpString.c_str());
	}
	if(pExXmlNode = pExXmlNode->XmlGetFirstNode(VehicleDataXml,"Unit", "Description", NULL))
	{
		Data->szDescribtion = (char *)malloc(strlen(pExXmlNode->ToElement()->GetText()));
		if(!Data->szDescribtion) { cLog::write("Out of memory", cLog::eLOG_TYPE_MEM); }
		strcpy(Data->szDescribtion,pExXmlNode->ToElement()->GetText());
	}

	// get array count
	i = sizeof(DataStructure);
	arraycount = 0;
	while(i)
	{
		i -= sizeof(DataStructure[arraycount]);
		arraycount++;
	}
	// Read infos
	for( i = 0; i < arraycount; i += n )
	{
		n = 0;
		while(DataStructure[i+n] != NULL)
			n++;

		pExXmlNode = pExXmlNode->XmlGetFirstNode(VehicleDataXml,DataStructure[i], DataStructure[i+1], DataStructure[i+2],
												 DataStructure[i+3], DataStructure[i+4], DataStructure[i+5], DataStructure[i+6]);
		sNodePath="";
		for(int j = 0; j<n;j++)
		{
			sNodePath += DataStructure[i+j];
			sNodePath += ";";
		}

#define GENERAL_NODE (string)"Unit;General_Info;"
#define DEFENCE_NODE (string)"Unit;Defence;"
#define PRODUCTION_NODE (string)"Unit;Production;"
#define WEAPONS_NODE (string)"Unit;Weapons;Weapon;"
#define ABILITIES_NODE (string)"Unit;Abilities;"
#define SCAN_ABILITIES_NODE (string)"Unit;Scan_Abilities;"
#define MOVEMENT_NODE (string)"Unit;Movement;"
#define STORAGE_NODE (string)"Unit;Storage;"
#define GRAFIC_NODE (string)"Unit;Graphic;"

		// is bool?
		if(pExXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"YN"))
		{
			// General
			if(sNodePath.compare(GENERAL_NODE + "Is_Controllable;") == 0)
				Data->bIs_Controllable = pExXmlNode->XmlDataToBool(sTmpString);
			else if(sNodePath.compare(GENERAL_NODE + "Can_Be_Captured;") == 0)
				Data->bCan_Be_Captured = pExXmlNode->XmlDataToBool(sTmpString);
			else if(sNodePath.compare(GENERAL_NODE + "Can_Be_Disabled;") == 0)
				Data->bCan_Be_Disabled = pExXmlNode->XmlDataToBool(sTmpString);
			// Defence
			else if(sNodePath.compare(DEFENCE_NODE + "Is_Target_Land;") == 0)
				Data->bIs_Target_Land = pExXmlNode->XmlDataToBool(sTmpString);
			else if(sNodePath.compare(DEFENCE_NODE + "Is_Target_Sea;") == 0)
				Data->bIs_Target_Sea = pExXmlNode->XmlDataToBool(sTmpString);
			else if(sNodePath.compare(DEFENCE_NODE + "Is_Target_Air;") == 0)
				Data->bIs_Target_Air = pExXmlNode->XmlDataToBool(sTmpString);
			else if(sNodePath.compare(DEFENCE_NODE + "Is_Target_Underwater;") == 0)
				Data->bIs_Target_Underwater = pExXmlNode->XmlDataToBool(sTmpString);
			else if(sNodePath.compare(DEFENCE_NODE + "Is_Target_Mine;") == 0)
				Data->bIs_Target_Mine = pExXmlNode->XmlDataToBool(sTmpString);
			else if(sNodePath.compare(DEFENCE_NODE + "Is_Target_Building;") == 0)
				Data->bIs_Target_Building = pExXmlNode->XmlDataToBool(sTmpString);
			else if(sNodePath.compare(DEFENCE_NODE + "Is_Target_Satellite;") == 0)
				Data->bIs_Target_Satellite = pExXmlNode->XmlDataToBool(sTmpString);
			else if(sNodePath.compare(DEFENCE_NODE + "Is_Target_WMD;") == 0)
				Data->bIs_Target_WMD = pExXmlNode->XmlDataToBool(sTmpString);
			// Abilities
			else if(sNodePath.compare(ABILITIES_NODE + "Can_Clear_Area;") == 0)
				Data->bCan_Clear_Area = pExXmlNode->XmlDataToBool(sTmpString);
			else if(sNodePath.compare(ABILITIES_NODE + "Gets_Experience;") == 0)
				Data->bGets_Experience = pExXmlNode->XmlDataToBool(sTmpString);
			else if(sNodePath.compare(ABILITIES_NODE + "Can_Disable;") == 0)
				Data->bCan_Disable = pExXmlNode->XmlDataToBool(sTmpString);
			else if(sNodePath.compare(ABILITIES_NODE + "Can_Capture;") == 0)
				Data->bCan_Capture = pExXmlNode->XmlDataToBool(sTmpString);
			else if(sNodePath.compare(ABILITIES_NODE + "Can_Dive;") == 0)
				Data->bCan_Dive = pExXmlNode->XmlDataToBool(sTmpString);
			else if(sNodePath.compare(ABILITIES_NODE + "Can_Upgrade;") == 0)
				Data->bCan_Upgrade = pExXmlNode->XmlDataToBool(sTmpString);
			else if(sNodePath.compare(ABILITIES_NODE + "Can_Repair;") == 0)
				Data->bCan_Repair = pExXmlNode->XmlDataToBool(sTmpString);
			else if(sNodePath.compare(ABILITIES_NODE + "Can_Research;") == 0)
				Data->bCan_Research = pExXmlNode->XmlDataToBool(sTmpString);
			else if(sNodePath.compare(ABILITIES_NODE + "Is_Kamikaze;") == 0)
				Data->bIs_Kamikaze = pExXmlNode->XmlDataToBool(sTmpString);
			else if(sNodePath.compare(ABILITIES_NODE + "Is_Infrastructure;") == 0)
				Data->bIs_Infrastructure = pExXmlNode->XmlDataToBool(sTmpString);
			else if(sNodePath.compare(ABILITIES_NODE + "Can_Place_Mines;") == 0)
				Data->bCan_Place_Mines = pExXmlNode->XmlDataToBool(sTmpString);
			else if(sNodePath.compare(ABILITIES_NODE + "Can_Launch_SRBM;") == 0)
				Data->bCan_Launch_SRBM = pExXmlNode->XmlDataToBool(sTmpString);
			else if(sNodePath.compare(ABILITIES_NODE + "Has_Connector;") == 0)
				Data->bHas_Connector = pExXmlNode->XmlDataToBool(sTmpString);
			else if(sNodePath.compare(ABILITIES_NODE + "Can_Be_Passed;") == 0)
				Data->bCan_Be_Passed = pExXmlNode->XmlDataToBool(sTmpString);
			// Storage
			else if(sNodePath.compare(STORAGE_NODE + "Is_Garage;") == 0)
				Data->bIs_Garage = pExXmlNode->XmlDataToBool(sTmpString);
			// Grafic
			else if(sNodePath.compare(GRAFIC_NODE + "Has_Overlay;") == 0)
				Data->bHas_Overlay = pExXmlNode->XmlDataToBool(sTmpString);
			else if(sNodePath.compare(GRAFIC_NODE + "Animations;Build_Up;") == 0)
				Data->bBuild_Up_Grafic = pExXmlNode->XmlDataToBool(sTmpString);
			else if(sNodePath.compare(GRAFIC_NODE + "Animations;Movement;") == 0)
				Data->bAnimation_Movement = pExXmlNode->XmlDataToBool(sTmpString);
			else if(sNodePath.compare(GRAFIC_NODE + "Animations;Power_On;") == 0)
				Data->bPower_On_Grafic = pExXmlNode->XmlDataToBool(sTmpString);
			n++;
		}
		// is int?
		else if(pExXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Num"))
		{
			// General
			if(sNodePath.compare(GENERAL_NODE + "Size_Length;") == 0)
				Data->iSize_Length = atoi(sTmpString.c_str());
			else if(sNodePath.compare(GENERAL_NODE + "Size_Width;") == 0)
				Data->iSize_Width = atoi(sTmpString.c_str());
			// Defence
			else if(sNodePath.compare(DEFENCE_NODE + "Armor;") == 0)
				Data->iArmor = atoi(sTmpString.c_str());
			else if(sNodePath.compare(DEFENCE_NODE + "Hitpoints;") == 0)
				Data->iHitpoints = atoi(sTmpString.c_str());
			// Production
			else if(sNodePath.compare(PRODUCTION_NODE + "Built_Costs;") == 0)
				Data->iBuilt_Costs = atoi(sTmpString.c_str());
			else if(sNodePath.compare(PRODUCTION_NODE + "Built_Costs_Max;") == 0)
				Data->iBuilt_Costs_Max = atoi(sTmpString.c_str());
			// Weapons
			else if(sNodePath.compare(WEAPONS_NODE + "Turret_Gfx;") == 0)
				Data->Weapons[Data->iWeaponsCount].iSequence = atoi(sTmpString.c_str());
			else if(sNodePath.compare(WEAPONS_NODE + "Ammo_Quantity;") == 0)
				Data->Weapons[Data->iWeaponsCount].iAmmo_Quantity = atoi(sTmpString.c_str());
			else if(sNodePath.compare(WEAPONS_NODE + "Allowed_Targets;Target_Land;Damage;") == 0)
				Data->Weapons[Data->iWeaponsCount].iTarget_Land_Damage = atoi(sTmpString.c_str());
			else if(sNodePath.compare(WEAPONS_NODE + "Allowed_Targets;Target_Sea;Damage;") == 0)
				Data->Weapons[Data->iWeaponsCount].iTarget_Sea_Damage = atoi(sTmpString.c_str());
			else if(sNodePath.compare(WEAPONS_NODE + "Allowed_Targets;Target_Air;Damage;") == 0)
				Data->Weapons[Data->iWeaponsCount].iTarget_Air_Damage = atoi(sTmpString.c_str());
			else if(sNodePath.compare(WEAPONS_NODE + "Allowed_Targets;Target_Mine;Damage;") == 0)
				Data->Weapons[Data->iWeaponsCount].iTarget_Mine_Damage = atoi(sTmpString.c_str());
			else if(sNodePath.compare(WEAPONS_NODE + "Allowed_Targets;Target_Submarine;Damage;") == 0)
				Data->Weapons[Data->iWeaponsCount].iTarget_Submarine_Damage = atoi(sTmpString.c_str());
			else if(sNodePath.compare(WEAPONS_NODE + "Allowed_Targets;Target_Infantry;Damage;") == 0)
				Data->Weapons[Data->iWeaponsCount].iTarget_Infantry_Damage = atoi(sTmpString.c_str());
			else if(sNodePath.compare(WEAPONS_NODE + "Allowed_Targets;Target_WMD;Damage;") == 0)
				Data->Weapons[Data->iWeaponsCount].iTarget_WMD_Damage = atoi(sTmpString.c_str());
			else if(sNodePath.compare(WEAPONS_NODE + "Shots;") == 0)
				Data->Weapons[Data->iWeaponsCount].iShots = atoi(sTmpString.c_str());
			else if(sNodePath.compare(WEAPONS_NODE + "Movement_Allowed;") == 0)
				Data->Weapons[Data->iWeaponsCount].iMovement_Allowed = atoi(sTmpString.c_str());
			// Abilities
			else if(sNodePath.compare(ABILITIES_NODE + "Converts_Gold;") == 0)
				Data->iConverts_Gold = atoi(sTmpString.c_str());
			else if(sNodePath.compare(ABILITIES_NODE + "Needs_Energy;") == 0)
				Data->iNeeds_Energy = atoi(sTmpString.c_str());
			else if(sNodePath.compare(ABILITIES_NODE + "Needs_Oil;") == 0)
				Data->iNeeds_Oil = atoi(sTmpString.c_str());
			else if(sNodePath.compare(ABILITIES_NODE + "Needs_Metal;") == 0)
				Data->iNeeds_Metal = atoi(sTmpString.c_str());
			else if(sNodePath.compare(ABILITIES_NODE + "Needs_Humans;") == 0)
				Data->iNeeds_Humans = atoi(sTmpString.c_str());
			else if(sNodePath.compare(ABILITIES_NODE + "Mines_Resources;") == 0)
				Data->iMines_Resources = atoi(sTmpString.c_str());
			else if(sNodePath.compare(ABILITIES_NODE + "Energy_Shield_Strength;") == 0)
				Data->iEnergy_Shield_Strength = atoi(sTmpString.c_str());
			else if(sNodePath.compare(ABILITIES_NODE + "Energy_Shield_Size;") == 0)
				Data->iEnergy_Shield_Size = atoi(sTmpString.c_str());
			// Movement
			else if(sNodePath.compare(MOVEMENT_NODE + "Movement_Sum;") == 0)
				Data->iMovement_Sum = atoi(sTmpString.c_str());
			else if(sNodePath.compare(MOVEMENT_NODE + "Costs_Air;") == 0)
				Data->fCosts_Air = (float) pExXmlNode->XmlDataToDouble(sTmpString);
			else if(sNodePath.compare(MOVEMENT_NODE + "Costs_Sea;") == 0)
				Data->fCosts_Sea = (float) pExXmlNode->XmlDataToDouble(sTmpString);
			else if(sNodePath.compare(MOVEMENT_NODE + "Costs_Submarine;") == 0)
				Data->fCosts_Submarine = (float) pExXmlNode->XmlDataToDouble(sTmpString);
			else if(sNodePath.compare(MOVEMENT_NODE + "Costs_Ground;") == 0)
				Data->fCosts_Ground = (float) pExXmlNode->XmlDataToDouble(sTmpString);
			else if(sNodePath.compare(MOVEMENT_NODE + "Factor_Coast;") == 0)
				Data->fFactor_Coast = (float) pExXmlNode->XmlDataToDouble(sTmpString);
			else if(sNodePath.compare(MOVEMENT_NODE + "Factor_Wood;") == 0)
				Data->fFactor_Wood = (float) pExXmlNode->XmlDataToDouble(sTmpString);
			else if(sNodePath.compare(MOVEMENT_NODE + "Factor_Road;") == 0)
				Data->fFactor_Road = (float) pExXmlNode->XmlDataToDouble(sTmpString);
			else if(sNodePath.compare(MOVEMENT_NODE + "Factor_Bridge;") == 0)
				Data->fFactor_Bridge = (float) pExXmlNode->XmlDataToDouble(sTmpString);
			else if(sNodePath.compare(MOVEMENT_NODE + "Factor_Platform;") == 0)
				Data->fFactor_Platform = (float) pExXmlNode->XmlDataToDouble(sTmpString);
			else if(sNodePath.compare(MOVEMENT_NODE + "Factor_Monorail;") == 0)
				Data->fFactor_Monorail = (float) pExXmlNode->XmlDataToDouble(sTmpString);
			else if(sNodePath.compare(MOVEMENT_NODE + "Factor_Wreck;") == 0)
				Data->fFactor_Wreck = (float) pExXmlNode->XmlDataToDouble(sTmpString);
			else if(sNodePath.compare(MOVEMENT_NODE + "Factor_Mountains;") == 0)
				Data->fFactor_Mountains = (float) pExXmlNode->XmlDataToDouble(sTmpString);
			// Storage
			else if(sNodePath.compare(STORAGE_NODE + "Capacity_Metal;") == 0)
				Data->iCapacity_Metal = atoi(sTmpString.c_str());
			else if(sNodePath.compare(STORAGE_NODE + "Capacity_Oil;") == 0)
				Data->iCapacity_Oil = atoi(sTmpString.c_str());
			else if(sNodePath.compare(STORAGE_NODE + "Capacity_Gold;") == 0)
				Data->iConverts_Gold = atoi(sTmpString.c_str());
			else if(sNodePath.compare(STORAGE_NODE + "Capacity_Energy;") == 0)
				Data->iCapacity_Energy = atoi(sTmpString.c_str());
			else if(sNodePath.compare(STORAGE_NODE + "Capacity_Units_Air;") == 0)
				Data->iCapacity_Units_Air = atoi(sTmpString.c_str());
			else if(sNodePath.compare(STORAGE_NODE + "Capacity_Units_Sea;") == 0)
				Data->iCapacity_Units_Sea = atoi(sTmpString.c_str());
			else if(sNodePath.compare(STORAGE_NODE + "Capacity_Units_Ground;") == 0)
				Data->iCapacity_Units_Ground = atoi(sTmpString.c_str());
			else if(sNodePath.compare(STORAGE_NODE + "Capacity_Units_Infantry;") == 0)
				Data->iCapacity_Units_Infantry = atoi(sTmpString.c_str());
			n++;
		}
		// is text?
		else if(pExXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		{
			// Weapon
			if(sNodePath.compare(WEAPONS_NODE + "Turret_Gfx;") == 0)
				Data->Weapons[Data->iWeaponsCount].szTurret_Gfx = (char *)sTmpString.c_str();
			n++;
		}
		// is fd?
		else if(pExXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Fd"))
		{
			// Weapons
			if(sNodePath.compare(WEAPONS_NODE + "Allowed_Targets;Target_Land;Range;") == 0)
				Data->Weapons[Data->iWeaponsCount].iTarget_Air_Range = atoi(sTmpString.c_str());
			else if(sNodePath.compare(WEAPONS_NODE + "Allowed_Targets;Target_Sea;Range;") == 0)
				Data->Weapons[Data->iWeaponsCount].iTarget_Sea_Range = atoi(sTmpString.c_str());
			else if(sNodePath.compare(WEAPONS_NODE + "Allowed_Targets;Target_Air;Range;") == 0)
				Data->Weapons[Data->iWeaponsCount].iTarget_Air_Range = atoi(sTmpString.c_str());
			else if(sNodePath.compare(WEAPONS_NODE + "Allowed_Targets;Target_Mine;Range;") == 0)
				Data->Weapons[Data->iWeaponsCount].iTarget_Mine_Range = atoi(sTmpString.c_str());
			else if(sNodePath.compare(WEAPONS_NODE + "Allowed_Targets;Target_Submarine;Range;") == 0)
				Data->Weapons[Data->iWeaponsCount].iTarget_Submarine_Range = atoi(sTmpString.c_str());
			else if(sNodePath.compare(WEAPONS_NODE + "Allowed_Targets;Target_Infantry;Range;") == 0)
				Data->Weapons[Data->iWeaponsCount].iTarget_Infantry_Range = atoi(sTmpString.c_str());
			else if(sNodePath.compare(WEAPONS_NODE + "Allowed_Targets;Target_WMD;Range;") == 0)
				Data->Weapons[Data->iWeaponsCount].iTarget_WMD_Range = atoi(sTmpString.c_str());
			else if(sNodePath.compare(WEAPONS_NODE + "Destination_Area") == 0)
				Data->Weapons[Data->iWeaponsCount].iDestination_Area = atoi(sTmpString.c_str());
			// Abilities
			else if(sNodePath.compare(ABILITIES_NODE + "Makes_Tracks;") == 0)
				Data->iMakes_Tracks = atoi(sTmpString.c_str());
			// Scan-Abilities
			else if(sNodePath.compare(SCAN_ABILITIES_NODE + "Range_Sight;") == 0)
				Data->iScan_Range_Sight = atoi(sTmpString.c_str());
			else if(sNodePath.compare(SCAN_ABILITIES_NODE + "Range_Air;") == 0)
				Data->iScan_Range_Air = atoi(sTmpString.c_str());
			else if(sNodePath.compare(SCAN_ABILITIES_NODE + "Range_Ground;") == 0)
				Data->iScan_Range_Ground = atoi(sTmpString.c_str());
			else if(sNodePath.compare(SCAN_ABILITIES_NODE + "Range_Sea;") == 0)
				Data->iScan_Range_Sea = atoi(sTmpString.c_str());
			else if(sNodePath.compare(SCAN_ABILITIES_NODE + "Range_Submarine;") == 0)
				Data->iScan_Range_Submarine = atoi(sTmpString.c_str());
			else if(sNodePath.compare(SCAN_ABILITIES_NODE + "Range_Mine;") == 0)
				Data->iScan_Range_Mine = atoi(sTmpString.c_str());
			else if(sNodePath.compare(SCAN_ABILITIES_NODE + "Range_Infantry;") == 0)
				Data->iScan_Range_Infantry = atoi(sTmpString.c_str());
			else if(sNodePath.compare(SCAN_ABILITIES_NODE + "Range_Resources;") == 0)
				Data->iScan_Range_Resources = atoi(sTmpString.c_str());
			else if(sNodePath.compare(SCAN_ABILITIES_NODE + "Range_Jammer;") == 0)
				Data->iScan_Range_Jammer = atoi(sTmpString.c_str());
			n++;
		}
		// is const?
		else if(pExXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Const"))
		{
			// Weapons
			if(sNodePath.compare(WEAPONS_NODE + "Shot_Trajectory;") == 0)
			{
				if(sTmpString.compare("straight") == 0)
					Data->Weapons[Data->iWeaponsCount].iShot_Trajectory = SHOT_TRAJECTURY_STRAIGHT;
				else if(sTmpString.compare("ballistic") == 0)
					Data->Weapons[Data->iWeaponsCount].iShot_Trajectory = SHOT_TRAJECTURY_BALISTIC;
				else if(sTmpString.compare("controlled") == 0)
					Data->Weapons[Data->iWeaponsCount].iShot_Trajectory = SHOT_TRAJECTURY_CONTROLED;
			}
			else if(sNodePath.compare(WEAPONS_NODE + "Ammo_Type;") == 0)
			{
				if(sTmpString.compare("Standard") == 0)
					Data->Weapons[Data->iWeaponsCount].iAmmo_Type = AMMO_TYPE_STANDARD;
				else if(sTmpString.compare("Energy") == 0)
					Data->Weapons[Data->iWeaponsCount].iAmmo_Type = AMMO_TYPE_ENERGY;
			}
			else if(sNodePath.compare(WEAPONS_NODE + "Destination_Type;") == 0)
			{
				if(sTmpString.compare("Point") == 0)
					Data->Weapons[Data->iWeaponsCount].iDestination_Type = DESTINATION_TYPE_POINT;
				else if(sTmpString.compare("MIRV") == 0)
					Data->Weapons[Data->iWeaponsCount].iDestination_Type = DESTINATION_TYPE_MIRV;
				else if(sTmpString.compare("Scatter") == 0)
					Data->Weapons[Data->iWeaponsCount].iDestination_Type = DESTINATION_TYPE_SCATTER;
			}
			// Abilities
			else if(sNodePath.compare(ABILITIES_NODE + "Landing_Type;") == 0)
			{
				if(sTmpString.compare("OnlyGarage") == 0)
					Data->iLanding_Type = LANDING_TYPE_ONLY_GARAGE;
				else if(sTmpString.compare("GarageAndPlatform") == 0)
					Data->iLanding_Type = LANDING_TYPE_GARAGE_AND_PLATFORM;
				else if(sTmpString.compare("Everywhere") == 0)
					Data->iLanding_Type = LANDING_TYPE_EVERYWHERE;
			}
			else if(sNodePath.compare(ABILITIES_NODE + "Self_Repair_Type;") == 0)
			{
				if(sTmpString.compare("None") == 0)
					Data->iSelf_Repair_Type = SELF_REPAIR_TYPE_NONE;
				else if(sTmpString.compare("Automatic") == 0)
					Data->iSelf_Repair_Type = SELF_REPAIR_TYPE_AUTOMATIC;
				else if(sTmpString.compare("Normal") == 0)
					Data->iSelf_Repair_Type = SELF_REPAIR_TYPE_NORMAL;
			}
			else if(sNodePath.compare(ABILITIES_NODE + "Changes_Terrain;") == 0)
			{
				if(sTmpString.compare("None") == 0)
					Data->iSelf_Repair_Type = CHANGES_TERRAIN_TYPE_NONE;
				else if(sTmpString.compare("Street") == 0)
					Data->iSelf_Repair_Type = CHANGES_TERRAIN_TYPE_STREET;
				else if(sTmpString.compare("Plattform") == 0)
					Data->iSelf_Repair_Type = CHANGES_TERRAIN_TYPE_PLATTFORM;
				else if(sTmpString.compare("Bridge") == 0)
					Data->iSelf_Repair_Type = CHANGES_TERRAIN_TYPE_BRIDGE;
				else if(sTmpString.compare("Mine") == 0)
					Data->iSelf_Repair_Type = CHANGES_TERRAIN_TYPE_MINE;
				else if(sTmpString.compare("Connector") == 0)
					Data->iSelf_Repair_Type = CHANGES_TERRAIN_TYPE_CONNECTOR;
			}
			n++;
		}
		// is id?
		else if(sNodePath.compare(PRODUCTION_NODE + "Is_Produced_by;") == 0 || sNodePath.compare(STORAGE_NODE + "Can_Use_Unit_As_Garage;") == 0)
		{
			// Production
			if(sNodePath.compare(PRODUCTION_NODE + "Is_Produced_by;") == 0)
			{
				int k = 0;
				pExXmlNode = pExXmlNode->XmlGetFirstNodeChild();
				while(pExXmlNode)
				{
					if(!pExXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"ID"))
						break;
					Data->iIs_Produced_by_ID[k].iFirstPart = atoi(sTmpString.substr(0,sTmpString.find(" ",0)).c_str());
					Data->iIs_Produced_by_ID[k].iSecondPart = atoi(sTmpString.substr(sTmpString.find(" ",0),sTmpString.length()).c_str());
					pExXmlNode = pExXmlNode->XmlGetNextNodeSibling();
					k++;
				}
			}
			// Storage
			else if(sNodePath.compare(STORAGE_NODE + "Can_Use_Unit_As_Garage;") == 0)
			{
				int k = 0;
				pExXmlNode = pExXmlNode->XmlGetFirstNodeChild();
				while(pExXmlNode)
				{
					if(!pExXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"ID"))
						break;
					Data->iCan_Use_Unit_As_Garage_ID[0].iFirstPart = atoi(sTmpString.substr(0,sTmpString.find(" ",0)).c_str());
					Data->iCan_Use_Unit_As_Garage_ID[0].iSecondPart = atoi(sTmpString.substr(sTmpString.find(" ",0),sTmpString.length()).c_str());
					pExXmlNode = pExXmlNode->XmlGetNextNodeSibling();
					k++;
				}
			}
			n++;
		}
		// is nothing known: cannot be readed!
		else
		{
			sTmpString = "Can't read -Node in ";
			sTmpString.insert(sTmpString.length()-9,DataStructure[i+n-1]);
			sTmpString += sVehicleDataPath;
			cLog::write(sTmpString.c_str(),LOG_TYPE_WARNING);
			n++;
		}
	}
	return ;
}

// SetDefaultUnitData /////////////////////////////////////////////////////////
// Sets all unitdata to default values
void SetDefaultUnitData(int unitnum, bool vehicle)
{
	sUnitData *Data;
	if(vehicle)
		Data = &UnitsData.vehicle[unitnum].data;
	else
		Data = &UnitsData.building[unitnum].data;

	// Main info
	Data->ID.iFirstPart = -1;
	Data->ID.iSecondPart = -1;
	Data->szName = "unknown";
	Data->szDescribtion = "Default";

	// General info
	Data->bIs_Controllable = true;
	Data->bCan_Be_Captured = true;
	Data->bCan_Be_Disabled = true;
	Data->iSize_Length = 1;
	Data->iSize_Width = 1;

	// Defence
	Data->bIs_Target_Land = true;
	Data->bIs_Target_Sea = false;
	Data->bIs_Target_Air = false;
	Data->bIs_Target_Underwater = false;
	Data->bIs_Target_Mine = true;
	Data->bIs_Target_Building = true;
	Data->bIs_Target_Satellite = false;
	Data->bIs_Target_WMD = false;
	Data->iArmor = 1;
	Data->iHitpoints = 1;

	// Production
	Data->iBuilt_Costs = 1;
	Data->iBuilt_Costs_Max = 1;
	Data->iIs_Produced_by_ID = (sID*) malloc(sizeof(sID));
	if(!Data->iIs_Produced_by_ID) { cLog::write("Out of memory", cLog::eLOG_TYPE_MEM); }
	Data->iIs_Produced_by_ID[0].iFirstPart = -1;
	Data->iIs_Produced_by_ID[0].iSecondPart = -1;

	// Weapons
	Data->iWeaponsCount = 0;
	Data->Weapons = ( sWeaponData* ) calloc( 1, sizeof ( sWeaponData ) );
	if(!Data->Weapons) { cLog::write("Out of memory", cLog::eLOG_TYPE_MEM); }
	Data->Weapons[Data->iWeaponsCount].szTurret_Gfx = "None";
	Data->Weapons[Data->iWeaponsCount].iSequence = 0;
	Data->Weapons[Data->iWeaponsCount].iShot_Trajectory = SHOT_TRAJECTURY_STRAIGHT;
	Data->Weapons[Data->iWeaponsCount].iAmmo_Type = AMMO_TYPE_STANDARD;
	Data->Weapons[Data->iWeaponsCount].iAmmo_Quantity = 1;

	Data->Weapons[Data->iWeaponsCount].iTarget_Land_Damage = 1;
	Data->Weapons[Data->iWeaponsCount].iTarget_Land_Range = 1;
	Data->Weapons[Data->iWeaponsCount].iTarget_Sea_Damage = 1;
	Data->Weapons[Data->iWeaponsCount].iTarget_Sea_Range = 1;
	Data->Weapons[Data->iWeaponsCount].iTarget_Air_Damage = 1;
	Data->Weapons[Data->iWeaponsCount].iTarget_Air_Range = 1;
	Data->Weapons[Data->iWeaponsCount].iTarget_Mine_Damage = 1;
	Data->Weapons[Data->iWeaponsCount].iTarget_Mine_Range = 1;
	Data->Weapons[Data->iWeaponsCount].iTarget_Submarine_Damage = 1;
	Data->Weapons[Data->iWeaponsCount].iTarget_Submarine_Range = 1;
	Data->Weapons[Data->iWeaponsCount].iTarget_Infantry_Damage = 1;
	Data->Weapons[Data->iWeaponsCount].iTarget_Infantry_Range = 1;
	Data->Weapons[Data->iWeaponsCount].iTarget_WMD_Damage = 1;
	Data->Weapons[Data->iWeaponsCount].iTarget_WMD_Range = 1;

	Data->Weapons[Data->iWeaponsCount].iShots = 1;
	Data->Weapons[Data->iWeaponsCount].iDestination_Area = 1;
	Data->Weapons[Data->iWeaponsCount].iDestination_Type = DESTINATION_TYPE_POINT;
	Data->Weapons[Data->iWeaponsCount].iMovement_Allowed = 0;

	// Abilities
	Data->bCan_Clear_Area = false;
	Data->bGets_Experience = false;
	Data->bCan_Disable = false;
	Data->bCan_Capture = false;
	Data->bCan_Dive = false;
	Data->iLanding_Type = LANDING_TYPE_GARAGE_AND_PLATFORM;
	Data->bCan_Upgrade = false;
	Data->bCan_Repair = false;
	Data->bCan_Research = false;
	Data->bIs_Kamikaze = false;
	Data->bIs_Infrastructure = false;
	Data->bCan_Place_Mines = false;
	Data->iMakes_Tracks = 12;
	Data->iSelf_Repair_Type = SELF_REPAIR_TYPE_NONE;
	Data->iConverts_Gold = 0;
	Data->iNeeds_Energy = 0;
	Data->iNeeds_Oil = 0;
	Data->iNeeds_Metal = 0;
	Data->iNeeds_Humans = 0;
	Data->iMines_Resources = 0;
	Data->bCan_Launch_SRBM = false;
	Data->iEnergy_Shield_Strength = 0;
	Data->iEnergy_Shield_Size = 0;
	Data->bHas_Connector = false;
	Data->bCan_Be_Passed = false;
	Data->iChanges_Terrain = CHANGES_TERRAIN_TYPE_NONE;

	// Scan_Abilities
	Data->iScan_Range_Sight = 1;
	Data->iScan_Range_Air = 0;
	Data->iScan_Range_Ground = 0;
	Data->iScan_Range_Sea = 0;
	Data->iScan_Range_Submarine = 0;
	Data->iScan_Range_Mine = 0;
	Data->iScan_Range_Infantry = 0;
	Data->iScan_Range_Resources = 0;
	Data->iScan_Range_Jammer = 0;

	// Movement
	Data->iMovement_Sum = 1;
	Data->fCosts_Air = 0;
	Data->fCosts_Sea = 0;
	Data->fCosts_Submarine = 0;
	Data->fCosts_Ground = 1;
	Data->fFactor_Coast = 1.5;
	Data->fFactor_Wood = 1.5;
	Data->fFactor_Road = 0.5;
	Data->fFactor_Bridge = 1;
	Data->fFactor_Platform = 1;
	Data->fFactor_Monorail = 0;
	Data->fFactor_Wreck = 1;
	Data->fFactor_Mountains = 0;

	// Storage
	Data->bIs_Garage = false;
	Data->iCapacity_Metal = 0;
	Data->iCapacity_Oil = 0;
	Data->iCapacity_Gold = 0;
	Data->iCapacity_Energy = 0;
	Data->iCapacity_Units_Air = 0;
	Data->iCapacity_Units_Sea = 0;
	Data->iCapacity_Units_Ground = 0;
	Data->iCapacity_Units_Infantry = 0;
	Data->iCan_Use_Unit_As_Garage_ID = (sID*) malloc(sizeof(sID));
	if(!Data->iCan_Use_Unit_As_Garage_ID) { cLog::write("Out of memory", cLog::eLOG_TYPE_MEM); }
	Data->iCan_Use_Unit_As_Garage_ID[0].iFirstPart = -1;
	Data->iCan_Use_Unit_As_Garage_ID[0].iSecondPart = -1;

	// Grafics
	Data->bHas_Overlay = false;
	Data->bBuild_Up_Grafic = false;
	Data->bAnimation_Movement = false;
	Data->bPower_On_Grafic = false;
	return ;
}
