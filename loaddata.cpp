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
// Loads all relevant files and data at the start of the game.
//
//
///////////////////////////////////////////////////////////////////////////////

#include <SDL_mixer.h>
#include <iostream>
#include <sstream>

#ifdef WIN32

#else
# include <sys/stat.h>
# include <unistd.h>
#endif

#include "loaddata.h"

#include "autosurface.h"
#include "buildings.h"
#include "clans.h"
#include "extendedtinyxml.h"
#include "files.h"
#include "keys.h"
#include "log.h"
#include "main.h"
#include "pcx.h"
#include "settings.h"
#include "sound.h"
#include "tinyxml2.h"
#include "unifonts.h"
#include "vehicles.h"
#include "video.h"

#ifdef WIN32
# include <shlobj.h>
# include <direct.h>
#endif

using namespace std;
using namespace tinyxml2;

tinyxml2::XMLDocument LanguageFile;

/**
 * Writes a Logmessage on the SplashScreen
 * @param sTxt Text to write
 * @param ok 0 writes just text, 1 writes "OK" and else "ERROR"
 * @param pos Horizontal Positionindex on SplashScreen
 */
static void MakeLog (const std::string& sTxt, int ok, int pos);

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
static int LoadGraphics (const char* path);

/**
 * Loads the Effects
 * @param path Directory of the Effects
 * @return 1 on success
 */
static int LoadEffects (const char* path);

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
static int LoadMusic (const char* path);

/**
 * Loads all Voices
 * @param path Directory of the Vehicles
 * @return 1 on success
 */
static int LoadVoices (const char* path);

/**
 * Loads the unitdata from the data.xml in the unitfolder
 * @param directory Unitdirectory, relative to the main game directory
 */
static void LoadUnitData (sUnitData*, char const* directory, int iID);


static bool translateUnitData (sID ID, bool vehicle);

static void LoadUnitGraphicData (sUnitData* Data, char const* directory);

// LoadData ///////////////////////////////////////////////////////////////////
// Loads all relevant files and data:
int LoadData (void* data)
{
	int& loadingState = *static_cast<int*> (data);
	loadingState = LOAD_GOING;

	if (!DEDICATED_SERVER)
	{
		const std::string& fontPath = cSettings::getInstance().getFontPath() + PATH_DELIMITER;
		if (!FileExists ( (fontPath + "latin_normal.pcx").c_str())
			|| !FileExists ( (fontPath + "latin_big.pcx").c_str())
			|| !FileExists ( (fontPath + "latin_big_gold.pcx").c_str())
			|| !FileExists ( (fontPath + "latin_small.pcx").c_str()))
		{
			Log.write ("Missing a file needed for game. Check log and config! ", LOG_TYPE_ERROR);
			loadingState = LOAD_ERROR;
			return 0;
		}

		font = new cUnicodeFont; //init ascii fonts

		Log.mark();
	}

	string sVersion = PACKAGE_NAME;
	sVersion += " BUILD: ";
	sVersion += MAX_BUILD_DATE; sVersion += " ";
	sVersion += PACKAGE_REV;

	MakeLog (sVersion, 0, 0);

	// Load Languagepack
	MakeLog ("Loading languagepack...", 0, 2);

	string sLang = cSettings::getInstance().getLanguage();
	//FIXME: here is the assumption made that the file always exists with lower cases
	for (int i = 0; i <= 2; i++)
	{
		if (sLang[i] < 97)
		{
			sLang[i] += 32;
		}
	}
	string sTmpString = cSettings::getInstance().getLangPath();
	sTmpString += PATH_DELIMITER "lang_";
	sTmpString += sLang;
	sTmpString += ".xml";
	Log.write ("Using langfile: " + sTmpString, cLog::eLOG_TYPE_DEBUG);
	if (LoadLanguage() != 1 || !FileExists (sTmpString.c_str()))
	{
		MakeLog ("", -1, 2);
		loadingState = LOAD_ERROR;
		SDL_Delay (5000);
		return -1;
	}
	else
	{
		LanguageFile.LoadFile (sTmpString.c_str());
		MakeLog ("", 1, 2);
	}
	Log.mark();

	// Load Keys
	MakeLog (lngPack.i18n ("Text~Init~Keys"), 0, 3);

	if (LoadKeys() != 1)
	{
		MakeLog ("", -1, 3);
		SDL_Delay (5000);
		loadingState = LOAD_ERROR;
		return -1;
	}
	else
	{
		MakeLog ("", 1, 3);
	}
	Log.mark();

	// Load Fonts
	MakeLog (lngPack.i18n ("Text~Init~Fonts"), 0, 4);
	// -- little bit crude but fonts are already loaded. what to do with this now? -- beko
	// Really loaded with new cUnicodeFont
	MakeLog ("", 1, 4);
	Log.mark();

	// Load Graphics
	MakeLog (lngPack.i18n ("Text~Init~GFX"), 0, 5);

	if (LoadGraphics (cSettings::getInstance().getGfxPath().c_str()) != 1)
	{
		MakeLog ("", -1, 5);
		Log.write ("Error while loading graphics", LOG_TYPE_ERROR);
		SDL_Delay (5000);
		loadingState = LOAD_ERROR;
		return -1;
	}
	else
	{
		MakeLog ("", 1, 5);
	}
	Log.mark();

	// Load Effects
	MakeLog (lngPack.i18n ("Text~Init~Effects"), 0, 6);

	if (LoadEffects (cSettings::getInstance().getFxPath().c_str()) != 1)
	{
		MakeLog ("", -1, 6);
		SDL_Delay (5000);
		loadingState = LOAD_ERROR;
		return -1;
	}
	else
	{
		MakeLog ("", 1, 6);
	}
	Log.mark();

	// Load Vehicles
	MakeLog (lngPack.i18n ("Text~Init~Vehicles"), 0, 7);

	if (LoadVehicles() != 1)
	{
		MakeLog ("", -1, 7);
		SDL_Delay (5000);
		loadingState = LOAD_ERROR;
		return -1;
	}
	else
	{
		MakeLog ("", 1, 7);
	}
	Log.mark();

	// Load Buildings
	MakeLog (lngPack.i18n ("Text~Init~Buildings"), 0, 8);

	if (LoadBuildings() != 1)
	{
		MakeLog ("", -1, 8);
		SDL_Delay (5000);
		loadingState = LOAD_ERROR;
		return -1;
	}
	else
	{
		MakeLog ("", 1, 8);
	}
	Log.mark();

	MakeLog (lngPack.i18n ("Text~Init~Clans"), 0, 9);

	// Load Clan Settings
	if (LoadClans() != 1)
	{
		SDL_Delay (5000);
		loadingState = LOAD_ERROR;
		return -1;
	}
	else
	{
		MakeLog ("", 1, 9);
	}
	Log.mark();


	if (!DEDICATED_SERVER)
	{
		// Load Music
		MakeLog (lngPack.i18n ("Text~Init~Music"), 0, 10);

		if (LoadMusic (cSettings::getInstance().getMusicPath().c_str()) != 1)
		{
			MakeLog ("", -1, 10);
			SDL_Delay (5000);
			loadingState = LOAD_ERROR;
			return -1;
		}
		else
		{
			MakeLog ("", 1, 10);
		}
		Log.mark();

		// Load Sounds
		MakeLog (lngPack.i18n ("Text~Init~Sounds"), 0, 11);
		if (SoundData.load (cSettings::getInstance().getSoundsPath().c_str()) == false)
		{
			MakeLog ("", -1, 11);
			SDL_Delay (5000);
			loadingState = LOAD_ERROR;
			return -1;
		}
		else
		{
			MakeLog ("", 1, 11);
		}
		Log.mark();

		// Load Voices
		MakeLog (lngPack.i18n ("Text~Init~Voices"), 0, 12);

		if (LoadVoices (cSettings::getInstance().getVoicesPath().c_str()) != 1)
		{
			MakeLog ("", -1, 12);
			SDL_Delay (5000);
			loadingState = LOAD_ERROR;
			return -1;
		}
		else
		{
			MakeLog ("", 1, 12);
		}
		Log.mark();
	}

	SDL_Delay (1000);
	loadingState = LOAD_FINISHED;
	return 1;
}

// MakeLog ///////////////////////////////////////////////////////////////////
// Writes a Logmessage on the SplashScreen:
/* static */ void MakeLog (const string& sTxt, int ok, int pos)
{
	if (DEDICATED_SERVER)
	{
		cout << sTxt << endl;
		return;
	}
	const SDL_Rect rDest = {22, 152, 228, Uint16 (font->getFontHeight (FONT_LATIN_BIG_GOLD)) };
	const SDL_Rect rDest2 = {250, 152, 230, Uint16 (font->getFontHeight (FONT_LATIN_BIG_GOLD)) };
	SDL_Rect rSrc;

	switch (ok)
	{
		case 0:
			font->showText (rDest.x, rDest.y + rDest.h * pos, sTxt, FONT_LATIN_NORMAL);
			rSrc = rDest;
			rSrc.y = rDest.y + rDest.h * pos;
			if (pos == 0)   //need full line for first entry version information
			{
				SDL_BlitSurface (buffer, NULL, screen, NULL);
				SDL_UpdateRect (screen, rDest.x, rDest.y + rDest.h * pos, rDest.w + rDest2.w, rDest.h);
			}
			else
			{
				SDL_BlitSurface (buffer, &rSrc, screen, &rSrc);
				SDL_UpdateRect (screen, rDest.x, rDest.y + rDest.h * pos, rDest.w, rDest.h);
			}
			break;

		case 1:
			font->showText (rDest2.x, rDest2.y + rDest2.h * pos, "OK", FONT_LATIN_BIG_GOLD);
			break;

		default:
			font->showText (rDest2.x, rDest2.y + rDest2.h * pos, "ERROR ..check maxr.log!", FONT_LATIN_BIG_GOLD);
			break;
	}

	if (ok != 0)
	{
		rSrc = rDest2;
		rSrc.y = rDest2.y + rDest2.h * pos;
		SDL_BlitSurface (buffer, &rSrc, screen, &rSrc);
		SDL_UpdateRect (screen, rDest2.x, rDest2.y + rDest2.h * pos, rDest2.w, rDest2.h);
	}
}

static SDL_Surface* CloneSDLSurface (SDL_Surface* src)
{
	return SDL_ConvertSurface (src, src->format, src->flags);
}

/**
 * Loads a graphic to the surface
 * @param dest Destination surface
 * @param directory Directory of the file
 * @param filename Name of the file
 * @return 1 on success
 */
static int LoadGraphicToSurface (SDL_Surface*& dest, const char* directory, const char* filename)
{
	string filepath;
	if (strcmp (directory, ""))
	{
		filepath = directory;
		filepath += PATH_DELIMITER;
	}
	filepath += filename;
	if (!FileExists (filepath.c_str()))
	{
		dest = NULL;
		Log.write ("Missing GFX - your MAXR install seems to be incomplete!", cLog::eLOG_TYPE_ERROR);
		return 0;
	}
	dest = LoadPCX (filepath);

	filepath.insert (0, "File loaded: ");
	Log.write (filepath.c_str(), LOG_TYPE_DEBUG);

	return 1;
}

static int LoadGraphicToSurface (AutoSurface& dest, const char* directory, const char* filename)
{
	SDL_Surface* surface = NULL;
	const int res = LoadGraphicToSurface (surface, directory, filename);
	dest = surface;
	return res;
}

/**
 * Loads a effectgraphic to the surface
 * @param dest Destination surface
 * @param directory Directory of the file
 * @param filename Name of the file
 * @return 1 on success
 */
static int LoadEffectGraphicToSurface (AutoSurface (&dest) [2], const char* directory, const char* filename)
{
	string filepath;
	if (strcmp (directory, ""))
	{
		filepath = directory;
		filepath += PATH_DELIMITER;
	}
	filepath += filename;
	if (!FileExists (filepath.c_str()))
	{
		Log.write ("Missing GFX - your MAXR install seems to be incomplete!", cLog::eLOG_TYPE_ERROR);
		return 0;
	}

	dest[0] = LoadPCX (filepath);
	dest[1] = CloneSDLSurface (dest[0]);

	filepath.insert (0, "Effect successful loaded: ");
	Log.write (filepath.c_str(), LOG_TYPE_DEBUG);

	return 1;
}

// LoadEffectAlphacToSurface /////////////////////////////////////////////////
// Loads a effectgraphic as aplha to the surface:
static int LoadEffectAlphaToSurface (AutoSurface (&dest) [2], const char* directory, const char* filename, int alpha)
{
	string filepath;
	if (strcmp (directory, ""))
	{
		filepath = directory;
		filepath += PATH_DELIMITER;
	}
	filepath += filename;
	if (!FileExists (filepath.c_str()))
		return 0;

	dest[0] = LoadPCX (filepath);
	dest[1] = CloneSDLSurface (dest[0]);
	SDL_SetAlpha (dest[0], SDL_SRCALPHA, alpha);
	SDL_SetAlpha (dest[1], SDL_SRCALPHA, alpha);

	filepath.insert (0, "Effectalpha loaded: ");
	Log.write (filepath.c_str(), LOG_TYPE_DEBUG);

	return 1;
}

/**
 * Loads a soundfile to the Mix_Chunk
 * @param dest Destination Mix_Chunk
 * @param directory Directory of the file
 * @param filename Name of the file
 * @param localize When true, sVoiceLanguage is appended to the filename. Used for loading voice files.
 * @return 1 on success
 */
static int LoadSoundfile (sSOUND*& dest, const char* directory, const char* filename, bool localize = false)
{
	string filepath;
	string fullPath;
	if (strcmp (directory, ""))
	{
		filepath = directory;
		filepath += PATH_DELIMITER;
	}
	//add lang code to voice
	fullPath = filepath + filename;
	if (localize && !cSettings::getInstance().getVoiceLanguage().empty())
	{
		fullPath.insert (fullPath.rfind ("."), "_" + cSettings::getInstance().getVoiceLanguage());
		if (FileExists (fullPath.c_str()))
		{
			dest = Mix_LoadWAV (fullPath.c_str());
			return 1;
		}
	}

	//no localized voice file. Try opening without lang code
	fullPath = filepath + filename;
	if (!FileExists (fullPath.c_str()))
		return 0;

	dest = Mix_LoadWAV (fullPath.c_str());

	return 1;
}

static int LoadSoundfile (AutoSound& dest, const char* directory, const char* filename, bool localize = false)
{
	sSOUND* sound = NULL;
	const int res = LoadSoundfile (sound, directory, filename, localize);
	dest = sound;
	return res;
}

/**
 * Loads a unitsoundfile to the Mix_Chunk. If the file doesn't exists a dummy file will be loaded
 * @param dest Destination Mix_Chunk
 * @param directory Directory of the file, relative to the main vehicles directory
 * @param filename Name of the file
 */
static void LoadUnitSoundfile (sSOUND*& dest, const char* directory, const char* filename)
{
	string filepath;
	if (strcmp (directory, ""))
		filepath += directory;
	filepath += filename;
	if (!SoundData.DummySound)
	{
		string sTmpString;
		sTmpString = cSettings::getInstance().getSoundsPath() + PATH_DELIMITER + "dummy.ogg";
		if (FileExists (sTmpString.c_str()))
		{
			SoundData.DummySound = Mix_LoadWAV (sTmpString.c_str());
			if (!SoundData.DummySound)
				Log.write ("Can't load dummy.ogg", LOG_TYPE_WARNING);
		}
	}
	// Not using FileExists to avoid unnecessary warnings in log file
	SDL_RWops* file = SDL_RWFromFile (filepath.c_str(), "r");
	if (!file)
	{
		dest = SoundData.DummySound;
		return;
	}
	SDL_RWclose (file);

	dest = Mix_LoadWAV (filepath.c_str());
}

static int LoadLanguage()
{
	if (lngPack.SetCurrentLanguage (cSettings::getInstance().getLanguage()) != 0) // Set the language code
	{
		// Not a valid language code, critical fail!
		Log.write ("Not a valid language code!", cLog::eLOG_TYPE_ERROR);
		return 0;
	}
	if (lngPack.ReadLanguagePack() != 0) // Load the translations
	{
		// Could not load the language, critical fail!
		Log.write ("Could not load the language!", cLog::eLOG_TYPE_ERROR);
		return 0;
	}
	return 1;
}

static int LoadEffects (const char* path)
{
	Log.write ("Loading Effects", LOG_TYPE_INFO);

	if (DEDICATED_SERVER) return 1;

	LoadEffectGraphicToSurface (EffectsData.fx_explo_small, path, "explo_small.pcx");
	LoadEffectGraphicToSurface (EffectsData.fx_explo_big, path, "explo_big.pcx");
	LoadEffectGraphicToSurface (EffectsData.fx_explo_water, path, "explo_water.pcx");
	LoadEffectGraphicToSurface (EffectsData.fx_explo_air, path, "explo_air.pcx");
	LoadEffectGraphicToSurface (EffectsData.fx_muzzle_big, path, "muzzle_big.pcx");
	LoadEffectGraphicToSurface (EffectsData.fx_muzzle_small, path, "muzzle_small.pcx");
	LoadEffectGraphicToSurface (EffectsData.fx_muzzle_med, path, "muzzle_med.pcx");
	LoadEffectGraphicToSurface (EffectsData.fx_hit, path, "hit.pcx");
	LoadEffectAlphaToSurface (EffectsData.fx_smoke, path, "smoke.pcx", 100);
	LoadEffectGraphicToSurface (EffectsData.fx_rocket, path, "rocket.pcx");
	LoadEffectAlphaToSurface (EffectsData.fx_dark_smoke, path, "dark_smoke.pcx", 100);
	LoadEffectAlphaToSurface (EffectsData.fx_tracks, path, "tracks.pcx", 100);
	LoadEffectAlphaToSurface (EffectsData.fx_corpse, path, "corpse.pcx", 255);
	LoadEffectAlphaToSurface (EffectsData.fx_absorb, path, "absorb.pcx", 150);

	return 1;
}

static int LoadMusic (const char* path)
{
	Log.write ("Loading music", LOG_TYPE_INFO);

	// Prepare music.xml for reading
	tinyxml2::XMLDocument MusicXml;
	string sTmpString = path;
	sTmpString += PATH_DELIMITER "music.xml";
	if (!FileExists (sTmpString.c_str()))
	{
		return 0;
	}
	if (MusicXml.LoadFile (sTmpString.c_str()) != XML_NO_ERROR)
	{
		Log.write ("Can't load music.xml ", LOG_TYPE_ERROR);
		return 0;
	}
	XMLElement* xmlElement;
#if 0 // unused
	xmlElement = XmlGetFirstElement (MusicXml, "Music", "Menus", "main", NULL);
	if (!xmlElement || !xmlElement->Attribute ("Text"))
	{
		Log.write ("Can't find \"main\" in music.xml ", LOG_TYPE_ERROR);
		return 0;
	}
	MainMusicFile = xmlElement->Attribute ("Text");

	xmlElement = XmlGetFirstElement (MusicXml, "Music", "Menus", "credits", NULL);
	if (!xmlElement || !xmlElement->Attribute ("Text"))
	{
		Log.write ("Can't find \"credits\" in music.xml ", LOG_TYPE_ERROR);
		return 0;
	}
	CreditsMusicFile = xmlElement->Attribute ("Text");
#endif
	xmlElement = XmlGetFirstElement (MusicXml, "Music", "Game", "bkgcount", NULL);
	if (!xmlElement || !xmlElement->Attribute ("Num"))
	{
		Log.write ("Can't find \"bkgcount\" in music.xml ", LOG_TYPE_ERROR);
		return 0;
	}
	int const MusicAnz = xmlElement->IntAttribute ("Num");
	for (int i = 1; i <= MusicAnz; i++)
	{
		std::string name = "bkg" + iToStr (i);
		XMLElement* xmlElement = XmlGetFirstElement (MusicXml, "Music", "Game", name.c_str(), NULL);
		if (xmlElement && xmlElement->Attribute ("Text"))
		{
			name = string (path) + PATH_DELIMITER + xmlElement->Attribute ("Text");
		}
		else
		{
			Log.write ("Can't find \"bkg" + iToStr (i) + "\" in music.xml", LOG_TYPE_WARNING);
			continue;
		}
		if (!FileExists (name.c_str()))
			continue;
		MusicFiles.push_back (name);
	}
	return 1;
}

bool cSoundData::load (const char* path)
{
	Log.write ("Loading Sounds", LOG_TYPE_INFO);

	LoadSoundfile (SNDHudSwitch, path, "HudSwitch.ogg");
	LoadSoundfile (SNDHudButton, path, "HudButton.ogg");
	LoadSoundfile (SNDMenuButton, path, "MenuButton.ogg");
	LoadSoundfile (SNDChat, path, "Chat.ogg");
	LoadSoundfile (SNDObjectMenu, path, "ObjectMenu.ogg");
	LoadSoundfile (EXPBigWet[0], path, "exp_big_wet0.ogg");
	LoadSoundfile (EXPBigWet[1], path, "exp_big_wet1.ogg");
	LoadSoundfile (EXPBig[0], path, "exp_big0.ogg");
	LoadSoundfile (EXPBig[1], path, "exp_big1.ogg");
	LoadSoundfile (EXPBig[2], path, "exp_big2.ogg");
	LoadSoundfile (EXPBig[3], path, "exp_big3.ogg");
	LoadSoundfile (EXPSmallWet[0], path, "exp_small_wet0.ogg");
	LoadSoundfile (EXPSmallWet[1], path, "exp_small_wet1.ogg");
	LoadSoundfile (EXPSmallWet[2], path, "exp_small_wet2.ogg");
	LoadSoundfile (EXPSmall[0], path, "exp_small0.ogg");
	LoadSoundfile (EXPSmall[1], path, "exp_small1.ogg");
	LoadSoundfile (EXPSmall[2], path, "exp_small2.ogg");
	LoadSoundfile (SNDArm, path, "arm.ogg");
	LoadSoundfile (SNDBuilding, path, "building.ogg");
	LoadSoundfile (SNDClearing, path, "clearing.ogg");
	LoadSoundfile (SNDQuitsch, path, "quitsch.ogg");
	LoadSoundfile (SNDActivate, path, "activate.ogg");
	LoadSoundfile (SNDLoad, path, "load.ogg");
	LoadSoundfile (SNDReload, path, "reload.ogg");
	LoadSoundfile (SNDRepair, path, "repair.ogg");
	LoadSoundfile (SNDLandMinePlace, path, "land_mine_place.ogg");
	LoadSoundfile (SNDLandMineClear, path, "land_mine_clear.ogg");
	LoadSoundfile (SNDSeaMinePlace, path, "sea_mine_place.ogg");
	LoadSoundfile (SNDSeaMineClear, path, "sea_mine_clear.ogg");
	LoadSoundfile (SNDPanelOpen, path, "panel_open.ogg");
	LoadSoundfile (SNDPanelClose, path, "panel_close.ogg");
	LoadSoundfile (SNDAbsorb, path, "absorb.ogg");
	return true;
}

static int LoadVoices (const char* path)
{
	Log.write ("Loading Voices", LOG_TYPE_INFO);
	LoadSoundfile (VoiceData.VOIAmmoLow[0], path, "ammo_low1.ogg", true);
	LoadSoundfile (VoiceData.VOIAmmoLow[1], path, "ammo_low2.ogg", true);
	LoadSoundfile (VoiceData.VOIAmmoEmpty[0], path, "ammo_empty1.ogg", true);
	LoadSoundfile (VoiceData.VOIAmmoEmpty[1], path, "ammo_empty2.ogg", true);
	LoadSoundfile (VoiceData.VOIAttacking[0], path, "attacking1.ogg", true);
	LoadSoundfile (VoiceData.VOIAttacking[1], path, "attacking2.ogg", true);
	LoadSoundfile (VoiceData.VOIAttackingEnemy[0], path, "attacking_enemy1.ogg", true);
	LoadSoundfile (VoiceData.VOIAttackingEnemy[1], path, "attacking_enemy2.ogg", true);
	LoadSoundfile (VoiceData.VOIAttackingUs[0], path, "attacking_us.ogg", true);
	LoadSoundfile (VoiceData.VOIAttackingUs[1], path, "attacking_us2.ogg", true);
	LoadSoundfile (VoiceData.VOIAttackingUs[2], path, "attacking_us3.ogg", true);
	LoadSoundfile (VoiceData.VOIBuildDone[0], path, "build_done1.ogg", true);
	LoadSoundfile (VoiceData.VOIBuildDone[1], path, "build_done2.ogg", true);
	LoadSoundfile (VoiceData.VOIBuildDone[2], path, "build_done3.ogg", true);
	LoadSoundfile (VoiceData.VOIBuildDone[3], path, "build_done4.ogg", true);
	LoadSoundfile (VoiceData.VOIClearing, path, "clearing.ogg", true);
	LoadSoundfile (VoiceData.VOIClearingMines[0], path, "clearing_mines.ogg", true);
	LoadSoundfile (VoiceData.VOIClearingMines[1], path, "clearing_mines2.ogg", true);
	LoadSoundfile (VoiceData.VOICommandoFailed[0], path, "commando_failed1.ogg", true);
	LoadSoundfile (VoiceData.VOICommandoFailed[1], path, "commando_failed2.ogg", true);
	LoadSoundfile (VoiceData.VOICommandoFailed[2], path, "commando_failed3.ogg", true);
	LoadSoundfile (VoiceData.VOIDestroyedUs[0], path, "destroyed_us1.ogg", true);
	LoadSoundfile (VoiceData.VOIDestroyedUs[1], path, "destroyed_us2.ogg", true);
	LoadSoundfile (VoiceData.VOIDetected[0], path, "detected1.ogg", true);
	LoadSoundfile (VoiceData.VOIDetected[1], path, "detected2.ogg", true);
	LoadSoundfile (VoiceData.VOILanding[0], path, "landing1.ogg", true);
	LoadSoundfile (VoiceData.VOILanding[1], path, "landing2.ogg", true);
	LoadSoundfile (VoiceData.VOILanding[2], path, "landing3.ogg", true);
	LoadSoundfile (VoiceData.VOILayingMines, path, "laying_mines.ogg", true);
	LoadSoundfile (VoiceData.VOINoPath[0], path, "no_path1.ogg", true);
	LoadSoundfile (VoiceData.VOINoPath[1], path, "no_path2.ogg", true);
	LoadSoundfile (VoiceData.VOINoSpeed, path, "no_speed.ogg", true);
	LoadSoundfile (VoiceData.VOIOK[0], path, "ok1.ogg", true);
	LoadSoundfile (VoiceData.VOIOK[1], path, "ok2.ogg", true);
	LoadSoundfile (VoiceData.VOIOK[2], path, "ok3.ogg", true);
	LoadSoundfile (VoiceData.VOIOK[3], path, "ok4.ogg", true);
	LoadSoundfile (VoiceData.VOIReammo, path, "reammo.ogg", true);
	LoadSoundfile (VoiceData.VOIReammoAll, path, "reammo_all.ogg", true);
	LoadSoundfile (VoiceData.VOIRepaired[0], path, "repaired.ogg", true);
	LoadSoundfile (VoiceData.VOIRepaired[1], path, "repaired2.ogg", true);
	LoadSoundfile (VoiceData.VOIRepairedAll[0], path, "repaired_all1.ogg", true);
	LoadSoundfile (VoiceData.VOIRepairedAll[1], path, "repaired_all2.ogg", true);
	LoadSoundfile (VoiceData.VOIResearchComplete, path, "research_complete.ogg", true);
	LoadSoundfile (VoiceData.VOISaved, path, "saved.ogg", true);
	LoadSoundfile (VoiceData.VOISentry, path, "sentry.ogg", true);
	LoadSoundfile (VoiceData.VOIStartMore, path, "start_more.ogg", true);
	LoadSoundfile (VoiceData.VOIStartNone, path, "start_none.ogg", true);
	LoadSoundfile (VoiceData.VOIStartOne, path, "start_one.ogg", true);
	LoadSoundfile (VoiceData.VOIStatusRed[0], path, "status_red1.ogg", true);
	LoadSoundfile (VoiceData.VOIStatusRed[1], path, "status_red2.ogg", true);
	LoadSoundfile (VoiceData.VOIStatusYellow[0], path, "status_yellow1.ogg", true);
	LoadSoundfile (VoiceData.VOIStatusYellow[1], path, "status_yellow2.ogg", true);
	LoadSoundfile (VoiceData.VOISubDetected, path, "sub_detected.ogg", true);
	LoadSoundfile (VoiceData.VOISurveying[0], path, "surveying.ogg", true);
	LoadSoundfile (VoiceData.VOISurveying[1], path, "surveying2.ogg", true);
	LoadSoundfile (VoiceData.VOITransferDone, path, "transfer_done.ogg", true);
	LoadSoundfile (VoiceData.VOITurnEnd20Sec[0], path, "turn_end_20_sec1.ogg", true);//not used yet
	LoadSoundfile (VoiceData.VOITurnEnd20Sec[1], path, "turn_end_20_sec2.ogg", true);//not used yet
	LoadSoundfile (VoiceData.VOITurnEnd20Sec[2], path, "turn_end_20_sec3.ogg", true);//not used yet
	LoadSoundfile (VoiceData.VOIUnitDisabled, path, "unit_disabled.ogg", true);
	LoadSoundfile (VoiceData.VOIUnitDisabledByEnemy[0], path, "unit_disabled_by_enemy1.ogg", true);
	LoadSoundfile (VoiceData.VOIUnitDisabledByEnemy[1], path, "unit_disabled_by_enemy2.ogg", true);
	LoadSoundfile (VoiceData.VOIUnitStolen[0], path, "unit_stolen1.ogg", true);
	LoadSoundfile (VoiceData.VOIUnitStolen[1], path, "unit_stolen2.ogg", true);
	LoadSoundfile (VoiceData.VOIUnitStolenByEnemy, path, "unit_stolen_by_enemy.ogg", true);
	return 1;
}

static int LoadGraphics (const char* path)
{
	Log.write ("Loading Graphics", LOG_TYPE_INFO);
	string stmp;
	if (DEDICATED_SERVER) return 1;

	Log.write ("Gamegraphics...", LOG_TYPE_DEBUG);
	if (!LoadGraphicToSurface (GraphicsData.gfx_Chand, path, "hand.pcx") ||
		!LoadGraphicToSurface (GraphicsData.gfx_Cno, path, "no.pcx") ||
		!LoadGraphicToSurface (GraphicsData.gfx_Cselect, path, "select.pcx") ||
		!LoadGraphicToSurface (GraphicsData.gfx_Cmove, path, "move.pcx") ||
		!LoadGraphicToSurface (GraphicsData.gfx_Cmove_draft, path, "move_draft.pcx") ||
		!LoadGraphicToSurface (GraphicsData.gfx_Chelp, path, "help.pcx") ||
		!LoadGraphicToSurface (GraphicsData.gfx_Ctransf, path, "transf.pcx") ||
		!LoadGraphicToSurface (GraphicsData.gfx_Cload, path, "load.pcx") ||
		!LoadGraphicToSurface (GraphicsData.gfx_Cmuni, path, "muni.pcx") ||
		!LoadGraphicToSurface (GraphicsData.gfx_Cband, path, "band_cur.pcx") ||
		!LoadGraphicToSurface (GraphicsData.gfx_Cactivate, path, "activate.pcx") ||
		!LoadGraphicToSurface (GraphicsData.gfx_Crepair, path, "repair.pcx") ||
		!LoadGraphicToSurface (GraphicsData.gfx_Csteal, path, "steal.pcx") ||
		!LoadGraphicToSurface (GraphicsData.gfx_Cdisable, path, "disable.pcx") ||
		!LoadGraphicToSurface (GraphicsData.gfx_Cattack, path, "attack.pcx") ||
		!LoadGraphicToSurface (GraphicsData.gfx_Cattackoor, path, "attack_oor.pcx") ||
		!LoadGraphicToSurface (GraphicsData.gfx_hud_stuff, path, "hud_stuff.pcx") ||
		!LoadGraphicToSurface (GraphicsData.gfx_hud_extra_players, path, "hud_extra_players.pcx") ||
		!LoadGraphicToSurface (GraphicsData.gfx_panel_top, path, "panel_top.pcx") ||
		!LoadGraphicToSurface (GraphicsData.gfx_panel_bottom, path, "panel_bottom.pcx") ||
		!LoadGraphicToSurface (GraphicsData.gfx_menu_stuff, path, "menu_stuff.pcx"))
	{
		return 0;
	}
	LoadGraphicToSurface (GraphicsData.gfx_Cpfeil1, path, "pf_1.pcx");
	LoadGraphicToSurface (GraphicsData.gfx_Cpfeil2, path, "pf_2.pcx");
	LoadGraphicToSurface (GraphicsData.gfx_Cpfeil3, path, "pf_3.pcx");
	LoadGraphicToSurface (GraphicsData.gfx_Cpfeil4, path, "pf_4.pcx");
	LoadGraphicToSurface (GraphicsData.gfx_Cpfeil6, path, "pf_6.pcx");
	LoadGraphicToSurface (GraphicsData.gfx_Cpfeil7, path, "pf_7.pcx");
	LoadGraphicToSurface (GraphicsData.gfx_Cpfeil8, path, "pf_8.pcx");
	LoadGraphicToSurface (GraphicsData.gfx_Cpfeil9, path, "pf_9.pcx");
	LoadGraphicToSurface (GraphicsData.gfx_context_menu, path, "object_menu2.pcx");
	LoadGraphicToSurface (GraphicsData.gfx_destruction, path, "destruction.pcx");
	LoadGraphicToSurface (GraphicsData.gfx_band_small_org, path, "band_small.pcx");
	GraphicsData.gfx_band_small = CloneSDLSurface (GraphicsData.gfx_band_small_org);
	LoadGraphicToSurface (GraphicsData.gfx_band_big_org, path, "band_big.pcx");
	GraphicsData.gfx_band_big = CloneSDLSurface (GraphicsData.gfx_band_big_org);
	LoadGraphicToSurface (GraphicsData.gfx_big_beton_org, path, "big_beton.pcx");
	GraphicsData.gfx_big_beton = CloneSDLSurface (GraphicsData.gfx_big_beton_org);
	LoadGraphicToSurface (GraphicsData.gfx_storage, path, "storage.pcx");
	LoadGraphicToSurface (GraphicsData.gfx_storage_ground, path, "storage_ground.pcx");
	LoadGraphicToSurface (GraphicsData.gfx_dialog, path, "dialog.pcx");
	LoadGraphicToSurface (GraphicsData.gfx_edock, path, "edock.pcx");
	LoadGraphicToSurface (GraphicsData.gfx_edepot, path, "edepot.pcx");
	LoadGraphicToSurface (GraphicsData.gfx_ehangar, path, "ehangar.pcx");
	LoadGraphicToSurface (GraphicsData.gfx_player_pc, path, "player_pc.pcx");
	LoadGraphicToSurface (GraphicsData.gfx_player_human, path, "player_human.pcx");
	LoadGraphicToSurface (GraphicsData.gfx_player_none, path, "player_none.pcx");
	LoadGraphicToSurface (GraphicsData.gfx_exitpoints_org, path, "activate_field.pcx");
	GraphicsData.gfx_exitpoints = CloneSDLSurface (GraphicsData.gfx_exitpoints_org);
	LoadGraphicToSurface (GraphicsData.gfx_player_select, path, "customgame_menu.pcx");
	LoadGraphicToSurface (GraphicsData.gfx_menu_buttons, path, "menu_buttons.pcx");
	LoadGraphicToSurface (GraphicsData.gfx_player_ready, path, "player_ready.pcx");
	LoadGraphicToSurface (GraphicsData.gfx_hud_chatbox, path, "hud_chatbox.pcx");

	GraphicsData.DialogPath = cSettings::getInstance().getGfxPath() + PATH_DELIMITER + "dialog.pcx";
	GraphicsData.Dialog2Path = cSettings::getInstance().getGfxPath() + PATH_DELIMITER + "dialog2.pcx";
	GraphicsData.Dialog3Path = cSettings::getInstance().getGfxPath() + PATH_DELIMITER + "dialog3.pcx";
	FileExists (GraphicsData.DialogPath.c_str());
	FileExists (GraphicsData.Dialog2Path.c_str());
	FileExists (GraphicsData.Dialog3Path.c_str());

	// load colors even for dedicated server
	// Colors:
	Log.write ("Colourgraphics...", LOG_TYPE_DEBUG);
	LoadGraphicToSurface (OtherData.colors[cl_red], path, "cl_red.pcx");
	LoadGraphicToSurface (OtherData.colors[cl_blue], path, "cl_blue.pcx");
	LoadGraphicToSurface (OtherData.colors[cl_green], path, "cl_green.pcx");
	LoadGraphicToSurface (OtherData.colors[cl_grey], path, "cl_grey.pcx");
	LoadGraphicToSurface (OtherData.colors[cl_orange], path, "cl_orange.pcx");
	LoadGraphicToSurface (OtherData.colors[cl_yellow], path, "cl_yellow.pcx");
	LoadGraphicToSurface (OtherData.colors[cl_purple], path, "cl_purple.pcx");
	LoadGraphicToSurface (OtherData.colors[cl_aqua], path, "cl_aqua.pcx");

	for (int i = 0; i != PLAYERCOLORS; ++i)
		OtherData.colors_org[i] = CloneSDLSurface (OtherData.colors[i]);

	Log.write ("Shadowgraphics...", LOG_TYPE_DEBUG);
	// Shadow:
	GraphicsData.gfx_shadow = SDL_CreateRGBSurface (Video.getSurfaceType(), Video.getResolutionX(),
													Video.getResolutionY(), Video.getColDepth(), 0, 0, 0, 0);
	SDL_FillRect (GraphicsData.gfx_shadow, NULL, 0x0);
	SDL_SetAlpha (GraphicsData.gfx_shadow, SDL_SRCALPHA, 50);
	GraphicsData.gfx_tmp = SDL_CreateRGBSurface (Video.getSurfaceType(), 128, 128, Video.getColDepth(), 0, 0, 0, 0);
	SDL_SetColorKey (GraphicsData.gfx_tmp, SDL_SRCCOLORKEY, 0xFF00FF);

	// Glas:
	Log.write ("Glassgraphic...", LOG_TYPE_DEBUG);
	LoadGraphicToSurface (GraphicsData.gfx_destruction_glas, path, "destruction_glas.pcx");
	SDL_SetAlpha (GraphicsData.gfx_destruction_glas, SDL_SRCALPHA, 150);

	// Waypoints:
	Log.write ("Waypointgraphics...", LOG_TYPE_DEBUG);
	for (int i = 0; i < 60; i++)
	{
		OtherData.WayPointPfeile[0][i] = CreatePfeil (26, 11, 51, 36, 14, 48, PFEIL_COLOR, 64 - i);
		OtherData.WayPointPfeile[1][i] = CreatePfeil (14, 14, 49, 14, 31, 49, PFEIL_COLOR, 64 - i);
		OtherData.WayPointPfeile[2][i] = CreatePfeil (37, 11, 12, 36, 49, 48, PFEIL_COLOR, 64 - i);
		OtherData.WayPointPfeile[3][i] = CreatePfeil (49, 14, 49, 49, 14, 31, PFEIL_COLOR, 64 - i);
		OtherData.WayPointPfeile[4][i] = CreatePfeil (14, 14, 14, 49, 49, 31, PFEIL_COLOR, 64 - i);
		OtherData.WayPointPfeile[5][i] = CreatePfeil (15, 14, 52, 26, 27, 51, PFEIL_COLOR, 64 - i);
		OtherData.WayPointPfeile[6][i] = CreatePfeil (31, 14, 14, 49, 49, 49, PFEIL_COLOR, 64 - i);
		OtherData.WayPointPfeile[7][i] = CreatePfeil (48, 14, 36, 51, 11, 26, PFEIL_COLOR, 64 - i);

		OtherData.WayPointPfeileSpecial[0][i] = CreatePfeil (26, 11, 51, 36, 14, 48, PFEILS_COLOR, 64 - i);
		OtherData.WayPointPfeileSpecial[1][i] = CreatePfeil (14, 14, 49, 14, 31, 49, PFEILS_COLOR, 64 - i);
		OtherData.WayPointPfeileSpecial[2][i] = CreatePfeil (37, 11, 12, 36, 49, 48, PFEILS_COLOR, 64 - i);
		OtherData.WayPointPfeileSpecial[3][i] = CreatePfeil (49, 14, 49, 49, 14, 31, PFEILS_COLOR, 64 - i);
		OtherData.WayPointPfeileSpecial[4][i] = CreatePfeil (14, 14, 14, 49, 49, 31, PFEILS_COLOR, 64 - i);
		OtherData.WayPointPfeileSpecial[5][i] = CreatePfeil (15, 14, 52, 26, 27, 51, PFEILS_COLOR, 64 - i);
		OtherData.WayPointPfeileSpecial[6][i] = CreatePfeil (31, 14, 14, 49, 49, 49, PFEILS_COLOR, 64 - i);
		OtherData.WayPointPfeileSpecial[7][i] = CreatePfeil (48, 14, 36, 51, 11, 26, PFEILS_COLOR, 64 - i);
	}

	// Resources:
	Log.write ("Resourcegraphics...", LOG_TYPE_DEBUG);
	//metal
	if (LoadGraphicToSurface (ResourceData.res_metal_org, path, "res.pcx") == 1)
	{
		ResourceData.res_metal = CloneSDLSurface (ResourceData.res_metal_org);
		SDL_SetColorKey (ResourceData.res_metal, SDL_SRCCOLORKEY, 0xFF00FF);
	}

	//gold
	if (LoadGraphicToSurface (ResourceData.res_gold_org, path, "gold.pcx") == 1)
	{
		ResourceData.res_gold = CloneSDLSurface (ResourceData.res_gold_org);
		SDL_SetColorKey (ResourceData.res_gold, SDL_SRCCOLORKEY, 0xFF00FF);
	}

	//fuel
	if (LoadGraphicToSurface (ResourceData.res_oil_org, path, "fuel.pcx") == 1)
	{
		ResourceData.res_oil = CloneSDLSurface (ResourceData.res_oil_org);
		SDL_SetColorKey (ResourceData.res_oil, SDL_SRCCOLORKEY, 0xFF00FF);
	}

	return 1;
}

static int LoadVehicles()
{
	Log.write ("Loading Vehicles", LOG_TYPE_INFO);


	tinyxml2::XMLDocument VehiclesXml;

	string sTmpString = cSettings::getInstance().getVehiclesPath();
	sTmpString += PATH_DELIMITER "vehicles.xml";
	if (!FileExists (sTmpString.c_str()))
	{
		return 0;
	}
	if (VehiclesXml.LoadFile (sTmpString.c_str()) != XML_NO_ERROR)
	{
		Log.write ("Can't load vehicles.xml!", LOG_TYPE_ERROR);
		return 0;
	}
	XMLElement* xmlElement = XmlGetFirstElement (VehiclesXml, "VehicleData", "Vehicles", NULL);
	if (xmlElement == NULL)
	{
		Log.write ("Can't read \"VehicleData->Vehicles\" node!", LOG_TYPE_ERROR);
		return 0;
	}
	// read vehicles.xml
	std::vector<std::string> VehicleList;
	std::vector<int> IDList;
	xmlElement = xmlElement->FirstChildElement();
	if (xmlElement)
	{
		const char* directory = xmlElement->Attribute ("directory");
		if (directory != NULL)
			VehicleList.push_back (directory);
		else
		{
			string msg = string ("Can't read directory-attribute from \"") + xmlElement->Value() + "\" - node";
			Log.write (msg, LOG_TYPE_WARNING);
		}

		if (xmlElement->Attribute ("num"))
			IDList.push_back (xmlElement->IntAttribute ("num"));
		else
		{
			string msg = string ("Can't read num-attribute from \"") + xmlElement->Value() + "\" - node";
			Log.write (msg, LOG_TYPE_WARNING);
		}
	}
	else
		Log.write ("No vehicles defined in vehicles.xml!", LOG_TYPE_WARNING);
	while (xmlElement != NULL)
	{
		xmlElement = xmlElement->NextSiblingElement();
		if (xmlElement == NULL)
			break;

		const char* directory = xmlElement->Attribute ("directory");
		if (directory != NULL)
			VehicleList.push_back (directory);
		else
		{
			string msg = string ("Can't read directory-attribute from \"") + xmlElement->Value() + "\" - node";
			Log.write (msg, LOG_TYPE_WARNING);
		}

		if (xmlElement->Attribute ("num"))
			IDList.push_back (xmlElement->IntAttribute ("num"));
		else
		{
			string msg = string ("Can't read num-attribute from \"") + xmlElement->Value() + "\" - node";
			Log.write (msg, LOG_TYPE_WARNING);
		}
	}
	// load found units
	UnitsData.svehicles.clear();
	UnitsData.svehicles.resize (VehicleList.size());
	UnitsData.vehicleUIs.resize (VehicleList.size());
	string sVehiclePath;
	for (size_t i = 0; i != VehicleList.size(); ++i)
	{
		sVehiclePath = cSettings::getInstance().getVehiclesPath();
		sVehiclePath += PATH_DELIMITER;
		sVehiclePath += VehicleList[i];
		sVehiclePath += PATH_DELIMITER;

		sUnitData& v = UnitsData.svehicles[i];

		Log.write ("Reading values from XML", cLog::eLOG_TYPE_DEBUG);
		LoadUnitData (&v, sVehiclePath.c_str(), IDList[i]);
		if (!translateUnitData (v.ID, true))
			Log.write ("Can not translate Unit data. Check your lang file!", cLog::eLOG_TYPE_WARNING);

		if (DEDICATED_SERVER) continue;

		Log.write ("Loading graphics", cLog::eLOG_TYPE_DEBUG);
		sVehicleUIData& ui = UnitsData.vehicleUIs[i];
		// load infantery graphics
		if (v.animationMovement)
		{
			SDL_Rect rcDest;
			for (int n = 0; n < 8; n++)
			{
				ui.img[n] = SDL_CreateRGBSurface (Video.getSurfaceType() | SDL_SRCCOLORKEY, 64 * 13, 64, Video.getColDepth(), 0, 0, 0, 0);
				SDL_SetColorKey (ui.img[n], SDL_SRCCOLORKEY, 0x00FFFFFF);
				SDL_FillRect (ui.img[n], NULL, 0x00FF00FF);

				for (int j = 0; j < 13; j++)
				{
					sTmpString = sVehiclePath;
					char sztmp[16];
					TIXML_SNPRINTF (sztmp, sizeof (sztmp), "img%d_%.2d.pcx", n, j);
					sTmpString += sztmp;

					if (FileExists (sTmpString.c_str()))
					{
						AutoSurface sfTempSurface (LoadPCX (sTmpString));
						if (!sfTempSurface)
						{
							Log.write (SDL_GetError(), cLog::eLOG_TYPE_WARNING);
						}
						else
						{
							rcDest.x = 64 * j + 32 - sfTempSurface->w / 2;
							rcDest.y = 32 - sfTempSurface->h / 2;
							SDL_BlitSurface (sfTempSurface, NULL, ui.img[n], &rcDest);
						}
					}
				}
				ui.img_org[n] = SDL_CreateRGBSurface (Video.getSurfaceType() | SDL_SRCCOLORKEY, 64 * 13, 64, Video.getColDepth(), 0, 0, 0, 0);
				SDL_SetColorKey (ui.img[n], SDL_SRCCOLORKEY, 0x00FFFFFF);
				SDL_FillRect (ui.img_org[n], NULL, 0x00FFFFFF);
				SDL_BlitSurface (ui.img[n], NULL, ui.img_org[n], NULL);

				ui.shw[n] = SDL_CreateRGBSurface (Video.getSurfaceType() | SDL_SRCCOLORKEY, 64 * 13, 64, Video.getColDepth(), 0, 0, 0, 0);
				SDL_SetColorKey (ui.shw[n], SDL_SRCCOLORKEY, 0x00FF00FF);
				SDL_FillRect (ui.shw[n], NULL, 0x00FF00FF);
				ui.shw_org[n] = SDL_CreateRGBSurface (Video.getSurfaceType() | SDL_SRCCOLORKEY, 64 * 13, 64, Video.getColDepth(), 0, 0, 0, 0);
				SDL_SetColorKey (ui.shw_org[n], SDL_SRCCOLORKEY, 0x00FF00FF);
				SDL_FillRect (ui.shw_org[n], NULL, 0x00FF00FF);

				rcDest.x = 3;
				rcDest.y = 3;
				SDL_BlitSurface (ui.img_org[n], NULL, ui.shw_org[n], &rcDest);
				SDL_LockSurface (ui.shw_org[n]);
				Uint32* ptr = static_cast<Uint32*> (ui.shw_org[n]->pixels);
				for (int j = 0; j < 64 * 13 * 64; j++)
				{
					if (*ptr != 0x00FF00FF)
						*ptr = 0;
					ptr++;
				}
				SDL_UnlockSurface (ui.shw_org[n]);
				SDL_BlitSurface (ui.shw_org[n], NULL, ui.shw[n], NULL);
				SDL_SetAlpha (ui.shw_org[n], SDL_SRCALPHA, 50);
				SDL_SetAlpha (ui.shw[n], SDL_SRCALPHA, 50);
			}
		}
		// load other vehicle graphics
		else
		{
			for (int n = 0; n < 8; n++)
			{
				// load image
				sTmpString = sVehiclePath;
				char sztmp[16];
				TIXML_SNPRINTF (sztmp, sizeof (sztmp), "img%d.pcx", n);
				sTmpString += sztmp;
				Log.write (sTmpString, cLog::eLOG_TYPE_DEBUG);
				if (FileExists (sTmpString.c_str()))
				{
					ui.img_org[n] = LoadPCX (sTmpString);
					ui.img[n] = CloneSDLSurface (ui.img_org[n]);
					SDL_SetColorKey (ui.img_org[n], SDL_SRCCOLORKEY, 0xFFFFFF);
					SDL_SetColorKey (ui.img[n], SDL_SRCCOLORKEY, 0xFFFFFF);
				}
				else
				{
					Log.write ("Missing GFX - your MAXR install seems to be incomplete!", cLog::eLOG_TYPE_ERROR);
					return -1;
				}

				// load shadow
				sTmpString.replace (sTmpString.length() - 8, 3, "shw");
				if (FileExists (sTmpString.c_str()))
				{
					ui.shw_org[n] = LoadPCX (sTmpString);
					ui.shw[n] = CloneSDLSurface (ui.shw_org[n]);
					SDL_SetAlpha (ui.shw[n], SDL_SRCALPHA, 50);
				}
				else
				{
					ui.shw_org[n] = NULL;
					ui.shw[n]     = NULL;
				}
			}
		}
		// load video
		sTmpString = sVehiclePath;
		sTmpString += "video.flc";
		Log.write ("Loading video " + sTmpString, cLog::eLOG_TYPE_DEBUG);
		if (!FileExists (sTmpString.c_str()))
		{
			sTmpString = "";
		}
		ui.FLCFile = new char[sTmpString.length() + 1];
		if (!ui.FLCFile) { Log.write ("Out of memory", cLog::eLOG_TYPE_MEM); }
		strcpy (ui.FLCFile, sTmpString.c_str());

		// load infoimage
		sTmpString = sVehiclePath;
		sTmpString += "info.pcx";
		Log.write ("Loading portrait" + sTmpString, cLog::eLOG_TYPE_DEBUG);
		if (FileExists (sTmpString.c_str()))
		{
			ui.info = LoadPCX (sTmpString);
		}
		else
		{
			Log.write ("Missing GFX - your MAXR install seems to be incomplete!", cLog::eLOG_TYPE_ERROR);
			return -1;
		}

		// load storageimage
		sTmpString = sVehiclePath;
		sTmpString += "store.pcx";
		Log.write ("Loading storageportrait" + sTmpString, cLog::eLOG_TYPE_DEBUG);
		if (FileExists (sTmpString.c_str()))
		{
			ui.storage = LoadPCX (sTmpString);
		}
		else
		{
			Log.write ("Missing GFX - your MAXR install seems to be incomplete!", cLog::eLOG_TYPE_ERROR);
			return -1;
		}

		// load overlaygraphics if necessary
		Log.write ("Loading overlay", cLog::eLOG_TYPE_DEBUG);
		if (v.hasOverlay)
		{
			sTmpString = sVehiclePath;
			sTmpString += "overlay.pcx";
			if (FileExists (sTmpString.c_str()))
			{
				ui.overlay_org = LoadPCX (sTmpString);
				ui.overlay = CloneSDLSurface (ui.overlay_org);
			}
			else
			{
				Log.write ("Missing GFX - your MAXR install seems to be incomplete!", cLog::eLOG_TYPE_WARNING);
				ui.overlay_org       = NULL;
				ui.overlay           = NULL;
				v.hasOverlay = false;
			}
		}
		else
		{
			ui.overlay_org = NULL;
			ui.overlay     = NULL;
		}

		// load buildgraphics if necessary
		Log.write ("Loading buildgraphics", cLog::eLOG_TYPE_DEBUG);
		if (v.buildUpGraphic)
		{
			// load image
			sTmpString = sVehiclePath;
			sTmpString += "build.pcx";
			if (FileExists (sTmpString.c_str()))
			{
				ui.build_org = LoadPCX (sTmpString);
				ui.build = CloneSDLSurface (ui.build_org);
				SDL_SetColorKey (ui.build_org, SDL_SRCCOLORKEY, 0xFFFFFF);
				SDL_SetColorKey (ui.build, SDL_SRCCOLORKEY, 0xFFFFFF);
			}
			else
			{
				Log.write ("Missing GFX - your MAXR install seems to be incomplete!", cLog::eLOG_TYPE_WARNING);
				ui.build_org             = NULL;
				ui.build                 = NULL;
				v.buildUpGraphic = false;
			}
			// load shadow
			sTmpString = sVehiclePath;
			sTmpString += "build_shw.pcx";
			if (FileExists (sTmpString.c_str()))
			{
				ui.build_shw_org = LoadPCX (sTmpString);
				ui.build_shw = CloneSDLSurface (ui.build_shw_org);
				SDL_SetAlpha (ui.build_shw, SDL_SRCALPHA, 50);
			}
			else
			{
				Log.write ("Missing GFX - your MAXR install seems to be incomplete!", cLog::eLOG_TYPE_WARNING);
				ui.build_shw_org         = NULL;
				ui.build_shw             = NULL;
				v.buildUpGraphic = false;
			}
		}
		else
		{
			ui.build_org     = NULL;
			ui.build         = NULL;
			ui.build_shw_org = NULL;
			ui.build_shw     = NULL;
		}
		// load cleargraphics if necessary
		Log.write ("Loading cleargraphics", cLog::eLOG_TYPE_DEBUG);
		if (v.canClearArea)
		{
			// load image (small)
			sTmpString = sVehiclePath;
			sTmpString += "clear_small.pcx";
			if (FileExists (sTmpString.c_str()))
			{
				ui.clear_small_org = LoadPCX (sTmpString);
				ui.clear_small = CloneSDLSurface (ui.clear_small_org);
				SDL_SetColorKey (ui.clear_small_org, SDL_SRCCOLORKEY, 0xFFFFFF);
				SDL_SetColorKey (ui.clear_small, SDL_SRCCOLORKEY, 0xFFFFFF);
			}
			else
			{
				Log.write ("Missing GFX - your MAXR install seems to be incomplete!", cLog::eLOG_TYPE_WARNING);
				ui.clear_small_org      = NULL;
				ui.clear_small          = NULL;
				v.canClearArea = false;
			}
			// load shadow (small)
			sTmpString = sVehiclePath;
			sTmpString += "clear_small_shw.pcx";
			if (FileExists (sTmpString.c_str()))
			{
				ui.clear_small_shw_org = LoadPCX (sTmpString);
				ui.clear_small_shw = CloneSDLSurface (ui.clear_small_shw_org);
				SDL_SetAlpha (ui.clear_small_shw, SDL_SRCALPHA, 50);
			}
			else
			{
				Log.write ("Missing GFX - your MAXR install seems to be incomplete!", cLog::eLOG_TYPE_WARNING);
				ui.clear_small_shw_org  = NULL;
				ui.clear_small_shw      = NULL;
				v.canClearArea = false;
			}
			// load image (big)
			sTmpString = sVehiclePath;
			sTmpString += "clear_big.pcx";
			if (FileExists (sTmpString.c_str()))
			{
				ui.build_org = LoadPCX (sTmpString);
				ui.build = CloneSDLSurface (ui.build_org);
				SDL_SetColorKey (ui.build_org, SDL_SRCCOLORKEY, 0xFFFFFF);
				SDL_SetColorKey (ui.build, SDL_SRCCOLORKEY, 0xFFFFFF);
			}
			else
			{
				Log.write ("Missing GFX - your MAXR install seems to be incomplete!", cLog::eLOG_TYPE_WARNING);
				ui.build_org            = NULL;
				ui.build                = NULL;
				v.canClearArea = false;
			}
			// load shadow (big)
			sTmpString = sVehiclePath;
			sTmpString += "clear_big_shw.pcx";
			if (FileExists (sTmpString.c_str()))
			{
				ui.build_shw_org = LoadPCX (sTmpString);
				ui.build_shw = CloneSDLSurface (ui.build_shw_org);
				SDL_SetAlpha (ui.build_shw, SDL_SRCALPHA, 50);
			}
			else
			{
				Log.write ("Missing GFX - your MAXR install seems to be incomplete!", cLog::eLOG_TYPE_WARNING);
				ui.build_shw_org        = NULL;
				ui.build_shw            = NULL;
				v.canClearArea = false;
			}
		}
		else
		{
			ui.clear_small_org     = NULL;
			ui.clear_small         = NULL;
			ui.clear_small_shw_org = NULL;
			ui.clear_small_shw     = NULL;
		}

		// load sounds
		Log.write ("Loading sounds", cLog::eLOG_TYPE_DEBUG);
		LoadUnitSoundfile (ui.Wait,       sVehiclePath.c_str(), "wait.ogg");
		LoadUnitSoundfile (ui.WaitWater,  sVehiclePath.c_str(), "wait_water.ogg");
		LoadUnitSoundfile (ui.Start,      sVehiclePath.c_str(), "start.ogg");
		LoadUnitSoundfile (ui.StartWater, sVehiclePath.c_str(), "start_water.ogg");
		LoadUnitSoundfile (ui.Stop,       sVehiclePath.c_str(), "stop.ogg");
		LoadUnitSoundfile (ui.StopWater,  sVehiclePath.c_str(), "stop_water.ogg");
		LoadUnitSoundfile (ui.Drive,      sVehiclePath.c_str(), "drive.ogg");
		LoadUnitSoundfile (ui.DriveWater, sVehiclePath.c_str(), "drive_water.ogg");
		LoadUnitSoundfile (ui.Attack,     sVehiclePath.c_str(), "attack.ogg");
	}

	UnitsData.initializeIDData();

	return 1;
}

/**
* Gets the name and (text) description for clan with internal id num from language file
* If no translation exists a warning is issued and the existing strings are not altered
* @param num engine internal ID of clan sorted by oder of clans in clan.xml
*/
static void translateClanData (int num)
{
	cClanData& clanData = cClanData::instance();
	cClan* clan = clanData.getClan (num);

	if (clan == 0)
	{
		Log.write ("Can't find clan id " + iToStr (num) + " for translation", LOG_TYPE_WARNING);
		return;
	}
	XMLElement* xmlElement = LanguageFile.RootElement()->FirstChildElement ("Clans");
	if (!xmlElement)
	{
		Log.write ("Can't find clan node in language file. Please report this to your translation team!", LOG_TYPE_WARNING);
		return;
	}

	for (xmlElement = xmlElement->FirstChildElement ("Clan"); xmlElement; xmlElement = xmlElement->NextSiblingElement ("Clan"))
	{
		int id;
		if (xmlElement->QueryIntAttribute ("ID", &id) != XML_NO_ERROR) continue;
		if (id != num) continue;

		Log.write ("Found clan translation for clan id " + iToStr (num), LOG_TYPE_DEBUG);
		if (cSettings::getInstance().getLanguage() != "ENG")
		{
			const char* name = xmlElement->Attribute ("localized");
			if (!name) continue;
			clan->setName (name);
		}
		else
		{
			const char* name = xmlElement->Attribute ("ENG");
			if (!name) continue;
			clan->setName (name);
		}
		const char* description = xmlElement->GetText();
		if (description != NULL)
			clan->setDescription (description);
	}
}

/**
* Gets the name and the description for the unit from the selected language file
* @param ID Id of the unit
*/
static bool translateUnitData (sID ID, bool vehicle)
{
	sUnitData* Data = NULL;
	if (vehicle)
	{
		for (size_t i = 0; i != UnitsData.svehicles.size(); ++i)
		{
			if (UnitsData.svehicles[i].ID == ID)
			{
				Data = &UnitsData.svehicles[i];
				break;
			}
		}
	}
	else
	{
		for (size_t i = 0; i != UnitsData.sbuildings.size(); ++i)
		{
			if (UnitsData.sbuildings[i].ID == ID)
			{
				Data = &UnitsData.sbuildings[i];
				break;
			}
		}
	}
	if (Data == NULL) return false;

	XMLElement* xmlElement = LanguageFile.RootElement()->FirstChildElement ("Units");
	if (xmlElement == NULL) return false;

	xmlElement = xmlElement->FirstChildElement();
	while (xmlElement != NULL)
	{
		const char* value = xmlElement->Attribute ("ID");
		if (value == NULL) return false;

		sID elementID;
		elementID.iFirstPart = atoi (value);
		string idString = value;
		elementID.iSecondPart = atoi (idString.substr (idString.find (" ")).c_str());

		if (elementID == ID)
		{
			const char* value;
			if (cSettings::getInstance().getLanguage() != "ENG")
				value = xmlElement->Attribute ("localized");
			else
				value = xmlElement->Attribute ("ENG");
			if (value == NULL) return false;

			Data->name = value;

			value = xmlElement->GetText();
			if (value == NULL) return false;

			Data->description = value;
			size_t pos;
			while ( (pos = Data->description.find ("\\n")) != string::npos)
			{
				Data->description.replace (pos, 2, "\n");
			}
			return true;
		}
		xmlElement = xmlElement->NextSiblingElement();
	}

	return false;
}

static int LoadBuildings()
{
	Log.write ("Loading Buildings", LOG_TYPE_INFO);

	// read buildings.xml
	string sTmpString = cSettings::getInstance().getBuildingsPath();
	sTmpString += PATH_DELIMITER "buildings.xml";
	if (!FileExists (sTmpString.c_str()))
	{
		return 0;
	}

	tinyxml2::XMLDocument BuildingsXml;
	if (BuildingsXml.LoadFile (sTmpString.c_str()) != XML_NO_ERROR)
	{
		Log.write ("Can't load buildings.xml!", LOG_TYPE_ERROR);
		return 0;
	}
	XMLElement* xmlElement = XmlGetFirstElement (BuildingsXml, "BuildingsData", "Buildings", NULL);
	if (xmlElement == NULL)
	{
		Log.write ("Can't read \"BuildingData->Building\" node!", LOG_TYPE_ERROR);
		return 0;
	}
	std::vector<std::string> BuildingList;
	std::vector<int> IDList;
	xmlElement = xmlElement->FirstChildElement();
	if (xmlElement == NULL)
	{
		Log.write ("There are no buildings in the buildings.xml defined", LOG_TYPE_ERROR);
		return 1;
	}

	const char* directory = xmlElement->Attribute ("directory");
	if (directory != NULL)
		BuildingList.push_back (directory);
	else
	{
		string msg = string ("Can't read directory-attribute from \"") + xmlElement->Value() + "\" - node";
		Log.write (msg, LOG_TYPE_WARNING);
	}

	if (xmlElement->Attribute ("num"))
		IDList.push_back (xmlElement->IntAttribute ("num"));
	else
	{
		string msg = string ("Can't read num-attribute from \"") + xmlElement->Value() + "\" - node";
		Log.write (msg, LOG_TYPE_WARNING);
	}

	const char* spezial = xmlElement->Attribute ("special");
	if (spezial != NULL)
	{
		string specialString = spezial;
		if (specialString == "mine")            UnitsData.specialIDMine.iSecondPart       = IDList.back();
		else if (specialString == "energy")     UnitsData.specialIDSmallGen.iSecondPart   = IDList.back();
		else if (specialString == "connector")  UnitsData.specialIDConnector.iSecondPart  = IDList.back();
		else if (specialString == "landmine")   UnitsData.specialIDLandMine.iSecondPart   = IDList.back();
		else if (specialString == "seamine")    UnitsData.specialIDSeaMine.iSecondPart    = IDList.back();
		else if (specialString == "smallBeton") UnitsData.specialIDSmallBeton.iSecondPart = IDList.back();
		else Log.write ("Unknown spacial in buildings.xml \"" + specialString + "\"", LOG_TYPE_WARNING);
	}

	while (xmlElement != NULL)
	{
		xmlElement = xmlElement->NextSiblingElement();
		if (xmlElement == NULL)
			break;

		const char* directory = xmlElement->Attribute ("directory");
		if (directory != NULL)
			BuildingList.push_back (directory);
		else
		{
			string msg = string ("Can't read directory-attribute from \"") + xmlElement->Value() + "\" - node";
			Log.write (msg, LOG_TYPE_WARNING);
		}

		if (xmlElement->Attribute ("num"))
			IDList.push_back (xmlElement->IntAttribute ("num"));
		else
		{
			string msg = string ("Can't read directory-attribute from \"") + xmlElement->Value() + "\" - node";
			Log.write (msg, LOG_TYPE_WARNING);
		}

		const char* spezial = xmlElement->Attribute ("special");
		if (spezial != NULL)
		{
			string specialString = spezial;
			if (specialString == "mine")            UnitsData.specialIDMine.iSecondPart       = IDList.back();
			else if (specialString == "energy")     UnitsData.specialIDSmallGen.iSecondPart   = IDList.back();
			else if (specialString == "connector")  UnitsData.specialIDConnector.iSecondPart  = IDList.back();
			else if (specialString == "landmine")   UnitsData.specialIDLandMine.iSecondPart   = IDList.back();
			else if (specialString == "seamine")    UnitsData.specialIDSeaMine.iSecondPart    = IDList.back();
			else if (specialString == "smallBeton") UnitsData.specialIDSmallBeton.iSecondPart = IDList.back();
			else Log.write ("Unknown spacial in buildings.xml \"" + specialString + "\"", LOG_TYPE_WARNING);
		}
	}

	if (UnitsData.specialIDMine.iSecondPart       == 0) Log.write ("special \"mine\" missing in buildings.xml", LOG_TYPE_WARNING);
	if (UnitsData.specialIDSmallGen.iSecondPart   == 0) Log.write ("special \"energy\" missing in buildings.xml", LOG_TYPE_WARNING);
	if (UnitsData.specialIDConnector.iSecondPart  == 0) Log.write ("special \"connector\" missing in buildings.xml", LOG_TYPE_WARNING);
	if (UnitsData.specialIDLandMine.iSecondPart   == 0) Log.write ("special \"landmine\" missing in buildings.xml", LOG_TYPE_WARNING);
	if (UnitsData.specialIDSeaMine.iSecondPart    == 0) Log.write ("special \"seamine\" missing in buildings.xml", LOG_TYPE_WARNING);
	if (UnitsData.specialIDSmallBeton.iSecondPart == 0) Log.write ("special \"smallBeton\" missing in buildings.xml", LOG_TYPE_WARNING);

	UnitsData.specialIDMine.iFirstPart = UnitsData.specialIDSmallGen.iFirstPart = UnitsData.specialIDConnector.iFirstPart = UnitsData.specialIDLandMine.iFirstPart = UnitsData.specialIDSeaMine.iFirstPart = UnitsData.specialIDSmallBeton.iFirstPart = 1;
	// load found units
	UnitsData.sbuildings.clear();
	UnitsData.sbuildings.resize (BuildingList.size());
	UnitsData.buildingUIs.resize (BuildingList.size());
	for (size_t i = 0; i != BuildingList.size(); ++i)
	{
		string sBuildingPath = cSettings::getInstance().getBuildingsPath();
		sBuildingPath += PATH_DELIMITER;
		sBuildingPath += BuildingList[i];
		sBuildingPath += PATH_DELIMITER;

		sUnitData& b = UnitsData.sbuildings[i];
		LoadUnitData (&b, sBuildingPath.c_str(), IDList[i]);
		translateUnitData (b.ID, false);

		if (DEDICATED_SERVER) continue;

		// load img
		sBuildingUIData& ui = UnitsData.buildingUIs[i];
		sTmpString = sBuildingPath;
		sTmpString += "img.pcx";
		if (FileExists (sTmpString.c_str()))
		{
			ui.img_org = LoadPCX (sTmpString);
			ui.img = CloneSDLSurface (ui.img_org);
			SDL_SetColorKey (ui.img_org, SDL_SRCCOLORKEY, 0xFFFFFF);
			SDL_SetColorKey (ui.img, SDL_SRCCOLORKEY, 0xFFFFFF);
		}
		else
		{
			Log.write ("Missing GFX - your MAXR install seems to be incomplete!", cLog::eLOG_TYPE_ERROR);
			return -1;
		}
		// load shadow
		sTmpString = sBuildingPath;
		sTmpString += "shw.pcx";
		if (FileExists (sTmpString.c_str()))
		{
			ui.shw_org = LoadPCX (sTmpString);
			ui.shw     = CloneSDLSurface (ui.shw_org);
			SDL_SetAlpha (ui.shw, SDL_SRCALPHA, 50);
		}

		// load video
		sTmpString = sBuildingPath;
		sTmpString += "video.pcx";
		if (FileExists (sTmpString.c_str()))
			ui.video = LoadPCX (sTmpString);

		// load infoimage
		sTmpString = sBuildingPath;
		sTmpString += "info.pcx";
		if (FileExists (sTmpString.c_str()))
			ui.info = LoadPCX (sTmpString);

		// load effectgraphics if necessary
		if (b.powerOnGraphic)
		{
			sTmpString = sBuildingPath;
			sTmpString += "effect.pcx";
			if (FileExists (sTmpString.c_str()))
			{
				ui.eff_org = LoadPCX (sTmpString);
				ui.eff = CloneSDLSurface (ui.eff_org);
				SDL_SetAlpha (ui.eff, SDL_SRCALPHA, 10);
			}
		}
		else
		{
			ui.eff_org = NULL;
			ui.eff     = NULL;
		}

		// load sounds
		LoadUnitSoundfile (ui.Wait,    sBuildingPath.c_str(), "wait.ogg");
		LoadUnitSoundfile (ui.Start,   sBuildingPath.c_str(), "start.ogg");
		LoadUnitSoundfile (ui.Running, sBuildingPath.c_str(), "running.ogg");
		LoadUnitSoundfile (ui.Stop,    sBuildingPath.c_str(), "stop.ogg");
		LoadUnitSoundfile (ui.Attack,  sBuildingPath.c_str(), "attack.ogg");

		// Get Ptr if necessary:
		if (b.ID == UnitsData.specialIDConnector)
		{
			b.isConnectorGraphic = true;
			UnitsData.ptr_connector = ui.img;
			UnitsData.ptr_connector_org = ui.img_org;
			SDL_SetColorKey (UnitsData.ptr_connector, SDL_SRCCOLORKEY, 0xFF00FF);
			UnitsData.ptr_connector_shw = ui.shw;
			UnitsData.ptr_connector_shw_org = ui.shw_org;
			SDL_SetColorKey (UnitsData.ptr_connector_shw, SDL_SRCCOLORKEY, 0xFF00FF);
		}
		else if (b.ID == UnitsData.specialIDSmallBeton)
		{
			UnitsData.ptr_small_beton = ui.img;
			UnitsData.ptr_small_beton_org = ui.img_org;
			SDL_SetColorKey (UnitsData.ptr_small_beton, SDL_SRCCOLORKEY, 0xFF00FF);
		}

		// Check if there is more than one frame
		// use 129 here because some images from the res_installer are one pixel to large
		if (ui.img_org->w > 129 && !b.isConnectorGraphic && !b.hasClanLogos) b.hasFrames = ui.img_org->w / ui.img_org->h;
		else b.hasFrames = 0;
	}

	// Dirtsurfaces
	if (!DEDICATED_SERVER)
	{
		LoadGraphicToSurface (UnitsData.dirt_big_org, cSettings::getInstance().getBuildingsPath().c_str(), "dirt_big.pcx");
		UnitsData.dirt_big = CloneSDLSurface (UnitsData.dirt_big_org);
		LoadGraphicToSurface (UnitsData.dirt_big_shw_org, cSettings::getInstance().getBuildingsPath().c_str(), "dirt_big_shw.pcx");
		UnitsData.dirt_big_shw = CloneSDLSurface (UnitsData.dirt_big_shw_org);
		if (UnitsData.dirt_big_shw) SDL_SetAlpha (UnitsData.dirt_big_shw, SDL_SRCALPHA, 50);
		LoadGraphicToSurface (UnitsData.dirt_small_org, cSettings::getInstance().getBuildingsPath().c_str(), "dirt_small.pcx");
		UnitsData.dirt_small = CloneSDLSurface (UnitsData.dirt_small_org);
		LoadGraphicToSurface (UnitsData.dirt_small_shw_org, cSettings::getInstance().getBuildingsPath().c_str(), "dirt_small_shw.pcx");
		UnitsData.dirt_small_shw = CloneSDLSurface (UnitsData.dirt_small_shw_org);
		if (UnitsData.dirt_small_shw) SDL_SetAlpha (UnitsData.dirt_small_shw, SDL_SRCALPHA, 50);
	}
	return 1;
}


//------------------------------------------------------------------------------
static void LoadUnitData (sUnitData* const Data, char const* const directory, int const iID)
{
	tinyxml2::XMLDocument unitDataXml;

	string path = directory;
	path += "data.xml";
	if (!FileExists (path.c_str())) return ;

	if (unitDataXml.LoadFile (path.c_str()) != XML_NO_ERROR)
	{
		Log.write ("Can't load " + path, LOG_TYPE_WARNING);
		return ;
	}
	// Read minimal game version
	string gameVersion = getXMLAttributeString (unitDataXml, "text", "Unit", "Header", "Game_Version", NULL);

	//TODO check game version

	//read id
	string idString = getXMLAttributeString (unitDataXml, "ID", "Unit", NULL);
	char szTmp[100];
	// check whether the id exists twice
	sID id;
	id.iFirstPart = atoi (idString.substr (0, idString.find (" ", 0)).c_str());
	id.iSecondPart = atoi (idString.substr (idString.find (" ", 0), idString.length()).c_str());
	if (id.isAVehicle())
	{
		for (size_t i = 0; i != UnitsData.svehicles.size(); ++i)
		{
			if (UnitsData.svehicles[i].ID == id)
			{
				TIXML_SNPRINTF (szTmp, sizeof (szTmp), "unit with id %.2d %.2d already exists", id.iFirstPart, id.iSecondPart);
				Log.write (szTmp, LOG_TYPE_WARNING);
				return ;
			}
		}
	}
	else
	{
		for (size_t i = 0; i != UnitsData.sbuildings.size(); ++i)
		{
			if (UnitsData.sbuildings[i].ID == id)
			{
				TIXML_SNPRINTF (szTmp, sizeof (szTmp), "unit with id %.2d %.2d already exists", id.iFirstPart, id.iSecondPart);
				Log.write (szTmp, LOG_TYPE_WARNING);
				return ;
			}
		}
	}
	Data->ID = id;
	// check whether the read id is the same as the one from vehicles.xml or buildins.xml
	if (iID != atoi (idString.substr (idString.find (" ", 0), idString.length()).c_str()))
	{
		TIXML_SNPRINTF (szTmp, sizeof (szTmp), "ID %.2d %.2d isn't equal with ID for unit \"%s\" ", atoi (idString.substr (0, idString.find (" ", 0)).c_str()), atoi (idString.substr (idString.find (" ", 0), idString.length()).c_str()), directory);
		Log.write (szTmp, LOG_TYPE_WARNING);
		return ;
	}
	else
	{
		TIXML_SNPRINTF (szTmp, sizeof (szTmp), "ID %.2d %.2d verified", atoi (idString.substr (0, idString.find (" ", 0)).c_str()), atoi (idString.substr (idString.find (" ", 0), idString.length()).c_str()));
		Log.write (szTmp, LOG_TYPE_DEBUG);
	}
	//read name
	Data->name = getXMLAttributeString (unitDataXml, "name", "Unit", NULL);
	//read description
	if (XMLElement* const XMLElement = XmlGetFirstElement (unitDataXml, "Unit", "Description", NULL))
	{
		Data->description = XMLElement->GetText();
		size_t pos;
		while ( (pos = Data->description.find ("\\n")) != string::npos)
		{
			Data->description.replace (pos, 2, "\n");
		}
	}

	// Weapon
	string muzzleType = getXMLAttributeString (unitDataXml, "Const", "Unit", "Weapon", "Muzzle_Type", NULL);
	if (muzzleType.compare ("Big") == 0) Data->muzzleType = sUnitData::MUZZLE_TYPE_BIG;
	else if (muzzleType.compare ("Rocket") == 0) Data->muzzleType = sUnitData::MUZZLE_TYPE_ROCKET;
	else if (muzzleType.compare ("Small") == 0) Data->muzzleType = sUnitData::MUZZLE_TYPE_SMALL;
	else if (muzzleType.compare ("Med") == 0) Data->muzzleType = sUnitData::MUZZLE_TYPE_MED;
	else if (muzzleType.compare ("Med_Long") == 0) Data->muzzleType = sUnitData::MUZZLE_TYPE_MED_LONG;
	else if (muzzleType.compare ("Rocket_Cluster") == 0) Data->muzzleType = sUnitData::MUZZLE_TYPE_ROCKET_CLUSTER;
	else if (muzzleType.compare ("Torpedo") == 0) Data->muzzleType = sUnitData::MUZZLE_TYPE_TORPEDO;
	else if (muzzleType.compare ("Sniper") == 0) Data->muzzleType = sUnitData:: MUZZLE_TYPE_SNIPER;
	else Data->muzzleType = sUnitData::MUZZLE_TYPE_NONE;

	Data->ammoMax = getXMLAttributeInt (unitDataXml, "Unit", "Weapon", "Ammo_Quantity", NULL);
	Data->shotsMax = getXMLAttributeInt (unitDataXml, "Unit", "Weapon", "Shots", NULL);
	Data->range = getXMLAttributeInt (unitDataXml, "Unit", "Weapon", "Range", NULL);
	Data->damage = getXMLAttributeInt (unitDataXml, "Unit", "Weapon", "Damage", NULL);
	Data->canAttack = getXMLAttributeInt (unitDataXml, "Unit", "Weapon", "Can_Attack", NULL);

	// TODO: make the code differ between attacking sea units and land units.
	// until this is done being able to attack sea units means being able to attack ground units.
	if (Data->canAttack & TERRAIN_SEA) Data->canAttack |= TERRAIN_GROUND;

	Data->canDriveAndFire = getXMLAttributeBool (unitDataXml, "Unit", "Weapon", "Can_Drive_And_Fire", NULL);

	// Production
	Data->buildCosts = getXMLAttributeInt (unitDataXml, "Unit", "Production", "Built_Costs", NULL);

	Data->canBuild = getXMLAttributeString (unitDataXml, "String", "Unit", "Production", "Can_Build", NULL);
	Data->buildAs = getXMLAttributeString (unitDataXml, "String", "Unit", "Production", "Build_As", NULL);

	Data->maxBuildFactor = getXMLAttributeInt (unitDataXml, "Unit", "Production", "Max_Build_Factor", NULL);

	Data->canBuildPath = getXMLAttributeBool (unitDataXml, "Unit", "Production", "Can_Build_Path", NULL);
	Data->canBuildRepeat = getXMLAttributeBool (unitDataXml, "Unit", "Production", "Can_Build_Repeat", NULL);

	// Movement
	Data->speedMax = getXMLAttributeInt (unitDataXml, "Unit", "Movement", "Movement_Sum", NULL);
	Data->speedMax *= 4;

	Data->factorGround = getXMLAttributeFloat (unitDataXml, "Unit", "Movement", "Factor_Ground", NULL);
	Data->factorSea = getXMLAttributeFloat (unitDataXml, "Unit", "Movement", "Factor_Sea", NULL);
	Data->factorAir = getXMLAttributeFloat (unitDataXml, "Unit", "Movement", "Factor_Air", NULL);
	Data->factorCoast = getXMLAttributeFloat (unitDataXml, "Unit", "Movement", "Factor_Coast", NULL);

	// Abilities
	Data->isBig = getXMLAttributeBool (unitDataXml, "Unit", "Abilities", "Is_Big", NULL);
	Data->connectsToBase = getXMLAttributeBool (unitDataXml, "Unit", "Abilities", "Connects_To_Base", NULL);
	Data->armor = getXMLAttributeInt (unitDataXml, "Unit", "Abilities", "Armor", NULL);
	Data->hitpointsMax = getXMLAttributeInt (unitDataXml, "Unit", "Abilities", "Hitpoints", NULL);
	Data->scan = getXMLAttributeInt (unitDataXml, "Unit", "Abilities", "Scan_Range", NULL);
	Data->modifiesSpeed = getXMLAttributeFloat (unitDataXml, "Unit", "Abilities", "Modifies_Speed", NULL);
	Data->canClearArea = getXMLAttributeBool (unitDataXml, "Unit", "Abilities", "Can_Clear_Area", NULL);
	Data->canBeCaptured = getXMLAttributeBool (unitDataXml, "Unit", "Abilities", "Can_Be_Captured", NULL);
	Data->canBeDisabled = getXMLAttributeBool (unitDataXml, "Unit", "Abilities", "Can_Be_Disabled", NULL);
	Data->canCapture = getXMLAttributeBool (unitDataXml, "Unit", "Abilities", "Can_Capture", NULL);
	Data->canDisable = getXMLAttributeBool (unitDataXml, "Unit", "Abilities", "Can_Disable", NULL);
	Data->canRepair = getXMLAttributeBool (unitDataXml, "Unit", "Abilities", "Can_Repair", NULL);
	Data->canRearm = getXMLAttributeBool (unitDataXml, "Unit", "Abilities", "Can_Rearm", NULL);
	Data->canResearch = getXMLAttributeBool (unitDataXml, "Unit", "Abilities", "Can_Research", NULL);
	Data->canPlaceMines = getXMLAttributeBool (unitDataXml, "Unit", "Abilities", "Can_Place_Mines", NULL);
	Data->canSurvey = getXMLAttributeBool (unitDataXml, "Unit", "Abilities", "Can_Survey", NULL);
	Data->doesSelfRepair = getXMLAttributeBool (unitDataXml, "Unit", "Abilities", "Does_Self_Repair", NULL);
	Data->convertsGold = getXMLAttributeInt (unitDataXml, "Unit", "Abilities", "Converts_Gold", NULL);
	Data->canSelfDestroy = getXMLAttributeBool (unitDataXml, "Unit", "Abilities", "Can_Self_Destroy", NULL);
	Data->canScore = getXMLAttributeBool (unitDataXml, "Unit", "Abilities", "Can_Score", NULL);

	Data->canMineMaxRes = getXMLAttributeInt (unitDataXml, "Unit", "Abilities", "Can_Mine_Max_Resource", NULL);

	Data->needsMetal = getXMLAttributeInt (unitDataXml, "Unit", "Abilities", "Needs_Metal", NULL);
	Data->needsOil = getXMLAttributeInt (unitDataXml, "Unit", "Abilities", "Needs_Oil", NULL);
	Data->needsEnergy = getXMLAttributeInt (unitDataXml, "Unit", "Abilities", "Needs_Energy", NULL);
	Data->needsHumans = getXMLAttributeInt (unitDataXml, "Unit", "Abilities", "Needs_Humans", NULL);
	if (Data->needsEnergy < 0)
	{
		Data->produceEnergy = abs (Data->needsEnergy);
		Data->needsEnergy = 0;
	}
	else Data->produceEnergy = 0;
	if (Data->needsHumans < 0)
	{
		Data->produceHumans = abs (Data->needsHumans);
		Data->needsHumans = 0;
	}
	else Data->produceHumans = 0;

	Data->isStealthOn = getXMLAttributeInt (unitDataXml, "Unit", "Abilities", "Is_Stealth_On", NULL);
	Data->canDetectStealthOn = getXMLAttributeInt (unitDataXml, "Unit", "Abilities", "Can_Detect_Stealth_On", NULL);

	string surfacePosString = getXMLAttributeString (unitDataXml, "Const", "Unit", "Abilities", "Surface_Position", NULL);
	if (surfacePosString.compare ("BeneathSea") == 0) Data->surfacePosition = sUnitData::SURFACE_POS_BENEATH_SEA;
	else if (surfacePosString.compare ("AboveSea") == 0) Data->surfacePosition = sUnitData::SURFACE_POS_ABOVE_SEA;
	else if (surfacePosString.compare ("Base") == 0) Data->surfacePosition = sUnitData::SURFACE_POS_BASE;
	else if (surfacePosString.compare ("AboveBase") == 0) Data->surfacePosition = sUnitData::SURFACE_POS_ABOVE_BASE;
	else if (surfacePosString.compare ("Above") == 0) Data->surfacePosition = sUnitData::SURFACE_POS_ABOVE;
	else Data->surfacePosition = sUnitData::SURFACE_POS_GROUND;

	string overbuildString = getXMLAttributeString (unitDataXml, "Const", "Unit", "Abilities", "Can_Be_Overbuild", NULL);
	if (overbuildString.compare ("Yes") == 0) Data->canBeOverbuild = sUnitData::OVERBUILD_TYPE_YES;
	else if (overbuildString.compare ("YesNRemove") == 0) Data->canBeOverbuild = sUnitData::OVERBUILD_TYPE_YESNREMOVE;
	else Data->canBeOverbuild = sUnitData::OVERBUILD_TYPE_NO;

	Data->canBeLandedOn = getXMLAttributeBool (unitDataXml, "Unit", "Abilities", "Can_Be_Landed_On", NULL);
	Data->canWork = getXMLAttributeBool (unitDataXml, "Unit", "Abilities", "Is_Activatable", NULL);
	Data->explodesOnContact = getXMLAttributeBool (unitDataXml, "Unit", "Abilities", "Explodes_On_Contact", NULL);
	Data->isHuman = getXMLAttributeBool (unitDataXml, "Unit", "Abilities", "Is_Human", NULL);

	// Storage
	Data->storageResMax = getXMLAttributeInt (unitDataXml, "Unit", "Storage", "Capacity_Resources", NULL);

	string storeResString = getXMLAttributeString (unitDataXml, "Const", "Unit", "Storage", "Capacity_Res_Type", NULL);
	if (storeResString.compare ("Metal") == 0) Data->storeResType = sUnitData::STORE_RES_METAL;
	else if (storeResString.compare ("Oil") == 0) Data->storeResType = sUnitData::STORE_RES_OIL;
	else if (storeResString.compare ("Gold") == 0) Data->storeResType = sUnitData::STORE_RES_GOLD;
	else Data->storeResType = sUnitData::STORE_RES_NONE;

	Data->storageUnitsMax = getXMLAttributeInt (unitDataXml, "Unit", "Storage", "Capacity_Units", NULL);

	string storeUnitImgString = getXMLAttributeString (unitDataXml, "Const", "Unit", "Storage", "Capacity_Units_Image_Type", NULL);
	if (storeUnitImgString.compare ("Plane") == 0) Data->storeUnitsImageType = sUnitData::STORE_UNIT_IMG_PLANE;
	else if (storeUnitImgString.compare ("Human") == 0) Data->storeUnitsImageType = sUnitData::STORE_UNIT_IMG_HUMAN;
	else if (storeUnitImgString.compare ("Tank") == 0) Data->storeUnitsImageType = sUnitData::STORE_UNIT_IMG_TANK;
	else if (storeUnitImgString.compare ("Ship") == 0) Data->storeUnitsImageType = sUnitData::STORE_UNIT_IMG_SHIP;
	else Data->storeUnitsImageType = sUnitData::STORE_UNIT_IMG_TANK;

	string storeUnitsString = getXMLAttributeString (unitDataXml, "String", "Unit", "Storage", "Capacity_Units_Type", NULL);
	Split (storeUnitsString, "+", Data->storeUnitsTypes);

	Data->isStorageType = getXMLAttributeString (unitDataXml, "String", "Unit", "Storage", "Is_Storage_Type", NULL);

	// load graphics.xml
	LoadUnitGraphicData (Data, directory);

	// finish
	Log.write ("Unitdata read", cLog::eLOG_TYPE_DEBUG);
	if (cSettings::getInstance().isDebug()) Log.mark();
	return ;
}

//------------------------------------------------------------------------------
static void LoadUnitGraphicData (sUnitData* Data, char const* directory)
{
	tinyxml2::XMLDocument unitGraphicsXml;

	string path = directory;
	path += "graphics.xml";
	if (!FileExists (path.c_str())) return ;

	if (unitGraphicsXml.LoadFile (path.c_str()) != XML_NO_ERROR)
	{
		Log.write ("Can't load " + path, LOG_TYPE_WARNING);
		return ;
	}

	Data->hasClanLogos = getXMLAttributeBool (unitGraphicsXml, "Unit", "Graphic", "Has_Clan_Logos", NULL);
	Data->hasCorpse = getXMLAttributeBool (unitGraphicsXml, "Unit", "Graphic", "Has_Corpse", NULL);
	Data->hasDamageEffect = getXMLAttributeBool (unitGraphicsXml, "Unit", "Graphic", "Has_Damage_Effect", NULL);
	Data->hasBetonUnderground = getXMLAttributeBool (unitGraphicsXml, "Unit", "Graphic", "Has_Beton_Underground", NULL);
	Data->hasPlayerColor = getXMLAttributeBool (unitGraphicsXml, "Unit", "Graphic", "Has_Player_Color", NULL);
	Data->hasOverlay = getXMLAttributeBool (unitGraphicsXml, "Unit", "Graphic", "Has_Overlay", NULL);

	Data->buildUpGraphic = getXMLAttributeBool (unitGraphicsXml, "Unit", "Graphic", "Animations", "Build_Up", NULL);
	Data->animationMovement = getXMLAttributeBool (unitGraphicsXml, "Unit", "Graphic", "Animations", "Movement", NULL);
	Data->powerOnGraphic = getXMLAttributeBool (unitGraphicsXml, "Unit", "Graphic", "Animations", "Power_On", NULL);
	Data->isAnimated = getXMLAttributeBool (unitGraphicsXml, "Unit", "Graphic", "Animations", "Is_Animated", NULL);
	Data->makeTracks = getXMLAttributeBool (unitGraphicsXml, "Unit", "Graphic", "Animations", "Makes_Tracks", NULL);
}

//------------------------------------------------------------------------------
static int LoadClans()
{
	tinyxml2::XMLDocument clansXml;

	string clansXMLPath = CLANS_XML;
	if (!FileExists (clansXMLPath.c_str()))
		return 0;
	if (clansXml.LoadFile (clansXMLPath.c_str()) != XML_NO_ERROR)
	{
		Log.write ("Can't load " + clansXMLPath, LOG_TYPE_ERROR);
		return 0;
	}

	XMLElement* xmlElement = clansXml.FirstChildElement ("Clans");
	if (xmlElement == 0)
	{
		Log.write ("Can't read \"Clans\" node!", LOG_TYPE_ERROR);
		return 0;
	}

	for (XMLElement* clanElement = xmlElement->FirstChildElement ("Clan"); clanElement; clanElement = clanElement->NextSiblingElement ("Clan"))
	{
		cClan* newClan = cClanData::instance().addClan();
		string nameAttr = clanElement->Attribute ("Name");
		newClan->setName (nameAttr);

		const XMLElement* descriptionNode = clanElement->FirstChildElement ("Description");
		if (descriptionNode)
		{
			string descriptionString = descriptionNode->GetText();
			newClan->setDescription (descriptionString);
		}

		translateClanData (newClan->getClanID());

		for (XMLElement* statsElement = clanElement->FirstChildElement ("ChangedUnitStat"); statsElement; statsElement = statsElement->NextSiblingElement ("ChangedUnitStat"))
		{
			const char* idAttr = statsElement->Attribute ("UnitID");
			if (idAttr == 0)
			{
				Log.write ("Couldn't read UnitID for ChangedUnitStat for clans", LOG_TYPE_ERROR);
				continue;
			}
			string idAttrStr (idAttr);
			sID id;
			id.iFirstPart = atoi (idAttrStr.substr (0, idAttrStr.find (" ", 0)).c_str());
			id.iSecondPart = atoi (idAttrStr.substr (idAttrStr.find (" ", 0), idAttrStr.length()).c_str());

			cClanUnitStat* newStat = newClan->addUnitStat (id);

			for (XMLElement* modificationElement = statsElement->FirstChildElement(); modificationElement; modificationElement = modificationElement->NextSiblingElement())
			{
				string modName = modificationElement->Value();
				if (modificationElement->Attribute ("Num"))
				{
					newStat->addModification (modName, modificationElement->IntAttribute ("Num"));
				}
			}
		}
	}
	return 1;
}

void reloadUnitValues()
{
	tinyxml2::XMLDocument UnitsXml;
	XMLElement* Element;
	string path = cSettings::getInstance().getVehiclesPath() + PATH_DELIMITER + "vehicles.xml";
	if (!FileExists (path.c_str())) return ;
	if (UnitsXml.LoadFile (path.c_str()) != XML_NO_ERROR) return;
	if (! (Element = XmlGetFirstElement (UnitsXml, "VehicleData", "Vehicles", NULL))) return;

	Element = Element->FirstChildElement();
	int i = 0;
	while (Element != NULL)
	{
		int num = Element->IntAttribute ("num");
		LoadUnitData (&UnitsData.svehicles[i], (cSettings::getInstance().getVehiclesPath() + PATH_DELIMITER + Element->Attribute ("directory") + PATH_DELIMITER).c_str(), num);
		translateUnitData (UnitsData.svehicles[i].ID, true);
		if (Element->NextSibling()) Element = Element->NextSibling()->ToElement();
		else Element = NULL;
		i++;
	}

	path = cSettings::getInstance().getBuildingsPath() + PATH_DELIMITER + "buildings.xml";
	if (!FileExists (path.c_str())) return ;
	if (UnitsXml.LoadFile (path.c_str()) != XML_NO_ERROR) return;
	if (! (Element = XmlGetFirstElement (UnitsXml, "BuildingsData", "Buildings", NULL))) return;

	Element = Element->FirstChildElement();
	i = 0;
	while (Element != NULL)
	{
		int num = Element->IntAttribute ("num");
		LoadUnitData (&UnitsData.sbuildings[i], (cSettings::getInstance().getBuildingsPath() + PATH_DELIMITER + Element->Attribute ("directory") + PATH_DELIMITER).c_str(), num);
		translateUnitData (UnitsData.sbuildings[i].ID, false);
		if (Element->NextSibling()) Element = Element->NextSibling()->ToElement();
		else Element = NULL;
		i++;
	}
}
