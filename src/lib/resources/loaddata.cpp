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

#include "loaddata.h"

#include "SDLutility/autosurface.h"
#include "crashreporter/debug.h"
#include "game/data/player/clans.h"
#include "game/data/units/building.h"
#include "game/data/units/vehicle.h"
#include "maxrversion.h"
#include "output/video/unifonts.h"
#include "output/video/video.h"
#include "resources/buildinguidata.h"
#include "resources/keys.h"
#include "resources/pcx.h"
#include "resources/sound.h"
#include "resources/uidata.h"
#include "resources/vehicleuidata.h"
#include "settings.h"
#include "utility/language.h"
#include "utility/listhelpers.h"
#include "utility/log.h"
#include "utility/serialization/jsonarchive.h"

#include <SDL_mixer.h>
#include <filesystem>
#include <iostream>
#include <regex>
#include <set>
#include <sstream>

#define PFEIL_COLOR 0xFF0000FF // color of a waypointarrow
#define PFEILS_COLOR 0xFF00FF00 // color of a special waypointarrow

//------------------------------------------------------------------------------
std::string getBuildVersion()
{
	std::string sVersion = PACKAGE_NAME;
	sVersion += " BUILD: ";
	sVersion += MAX_BUILD_DATE;
	sVersion += " ";
	sVersion += PACKAGE_REV;
	return sVersion;
}

/**
 * Writes a Logmessage on the SplashScreen
 * @param sTxt Text to write
 */
static void ConsoleMakeLog (const std::string& sTxt, int, int)
{
	std::cout << sTxt << std::endl;
}

/**
 * Writes a Logmessage on the SplashScreen
 * @param sTxt Text to write
 * @param ok 0 writes just text, 1 writes "OK" and else "ERROR"
 * @param pos Horizontal Positionindex on SplashScreen
 */
static void WindowMakeLog (const std::string& sTxt, int ok, int pos)
{
	auto& font = *cUnicodeFont::font;
	const SDL_Rect rDest = {22, 152, 228, Uint16 (font.getFontHeight (eUnicodeFontType::LatinBigGold))};
	const SDL_Rect rDest2 = {250, 152, 230, Uint16 (font.getFontHeight (eUnicodeFontType::LatinBigGold))};

	switch (ok)
	{
		case 0:
			font.showText (rDest.x, rDest.y + rDest.h * pos, sTxt, eUnicodeFontType::LatinNormal);
			break;

		case 1:
			font.showText (rDest2.x, rDest2.y + rDest2.h * pos, "OK", eUnicodeFontType::LatinBigGold);
			break;

		default:
			font.showText (rDest2.x, rDest2.y + rDest2.h * pos, "ERROR ..check maxr.log!", eUnicodeFontType::LatinBigGold);
			break;
	}
	// TODO: Warn that screen has been changed
	// so that Video.draw() can be called in main thread.
}

//------------------------------------------------------------------------------
void debugTranslationSize (const cLanguage& language, const cUnicodeFont& font)
{
	std::regex reg{".*_([0-9]+)"};
	for (const auto& [key, translatedText] : language.getAllTranslations())
	{
		std::smatch res;

		if (std::regex_match (key, res, reg))
		{
			std::size_t maxSize = std::stoi (res[1]);
			const char referenceLetter = 'a';

			if (font.getTextWide (std::string (maxSize, referenceLetter)) < font.getTextWide (translatedText))
			{
				Log.warn ("Maybe too long string for " + key + ": " + translatedText);
			}
		}
	}
}

static void LoadLanguage()
{
	lngPack.setLanguagesFolder (cSettings::getInstance().getLangPath());
	if (!ranges::contains (lngPack.getAvailableLanguages(), cSettings::getInstance().getLanguage()))
	{
		Log.warn ("Not a supported language: " + cSettings::getInstance().getLanguage() + ", defaulting to en.");
		cSettings::getInstance().setLanguage ("en");
		cSettings::getInstance().saveInFile();
	}
	lngPack.setCurrentLanguage (cSettings::getInstance().getLanguage());
}

/**
 * Loads a graphic to the surface
 * @param dest Destination surface
 * @param filepath Name of the file
 * @return 1 on success
 */
static int LoadGraphicToSurface (AutoSurface& dest, const std::filesystem::path& filepath)
{
	if (!std::filesystem::exists (filepath))
	{
		dest = nullptr;
		Log.error ("Missing GFX - your MAXR install seems to be incomplete!");
		return 0;
	}
	dest = LoadPCX (filepath);

	Log.debug ("File loaded: " + filepath.u8string());
	return 1;
}

//------------------------------------------------------------------------------
static AutoSurface CloneSDLSurface (SDL_Surface& src)
{
	return AutoSurface (SDL_ConvertSurface (&src, src.format, src.flags));
}

//------------------------------------------------------------------------------
static void createShadowGfx()
{
	// TODO: reduce size once we use texture.
	GraphicsData.gfx_shadow = AutoSurface (SDL_CreateRGBSurface (0, Video.getResolutionX(), Video.getResolutionY(), Video.getColDepth(), 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000));
	SDL_FillRect (GraphicsData.gfx_shadow.get(), nullptr, SDL_MapRGBA (GraphicsData.gfx_shadow->format, 0, 0, 0, 50));
}

/**
* Copy part of surface to create a new one
*/
static AutoSurface extractSurface (SDL_Surface& source, const SDL_Rect& rect)
{
	auto res = AutoSurface (SDL_CreateRGBSurface (0, rect.w, rect.h, Video.getColDepth(), 0, 0, 0, 0));

	SDL_SetColorKey (res.get(), SDL_TRUE, 0xFF00FF);
	SDL_FillRect (res.get(), nullptr, 0xFF00FF);

	SDL_BlitSurface (&source, &rect, res.get(), nullptr);
	return res;
}

/**
 * Loads all Graphics
 * @param path Directory of the graphics
 * @return 1 on success
 */
static int LoadGraphics (const std::filesystem::path& directory)
{
	Log.info ("Loading Graphics");

	Log.debug ("Gamegraphics...");
	// clang-format off
	if (!LoadGraphicToSurface (GraphicsData.gfx_Chand, directory / "hand.pcx") ||
		!LoadGraphicToSurface (GraphicsData.gfx_Cno, directory / "no.pcx") ||
		!LoadGraphicToSurface (GraphicsData.gfx_Cselect, directory / "select.pcx") ||
		!LoadGraphicToSurface (GraphicsData.gfx_Cmove, directory / "move.pcx") ||
		!LoadGraphicToSurface (GraphicsData.gfx_Cmove_draft, directory / "move_draft.pcx") ||
		!LoadGraphicToSurface (GraphicsData.gfx_Chelp, directory / "help.pcx") ||
		!LoadGraphicToSurface (GraphicsData.gfx_Ctransf, directory / "transf.pcx") ||
		!LoadGraphicToSurface (GraphicsData.gfx_Cload, directory / "load.pcx") ||
		!LoadGraphicToSurface (GraphicsData.gfx_Cmuni, directory / "muni.pcx") ||
		!LoadGraphicToSurface (GraphicsData.gfx_Cband, directory / "band_cur.pcx") ||
		!LoadGraphicToSurface (GraphicsData.gfx_Cactivate, directory / "activate.pcx") ||
		!LoadGraphicToSurface (GraphicsData.gfx_Crepair, directory / "repair.pcx") ||
		!LoadGraphicToSurface (GraphicsData.gfx_Csteal, directory / "steal.pcx") ||
		!LoadGraphicToSurface (GraphicsData.gfx_Cdisable, directory / "disable.pcx") ||
		!LoadGraphicToSurface (GraphicsData.gfx_Cattack, directory / "attack.pcx") ||
		!LoadGraphicToSurface (GraphicsData.gfx_Cattackoor, directory / "attack_oor.pcx") ||
		!LoadGraphicToSurface (GraphicsData.gfx_hud_stuff, directory / "hud_stuff.pcx") ||
		!LoadGraphicToSurface (GraphicsData.gfx_hud_extra_players, directory / "hud_extra_players.pcx") ||
		!LoadGraphicToSurface (GraphicsData.gfx_panel_top, directory / "panel_top.pcx") ||
		!LoadGraphicToSurface (GraphicsData.gfx_panel_bottom, directory / "panel_bottom.pcx") ||
		!LoadGraphicToSurface (GraphicsData.gfx_menu_stuff, directory / "menu_stuff.pcx"))
	{
		return 0;
	}
	// clang-format on

	GraphicsData.gfx_horizontal_bar_blocked = extractSurface (*GraphicsData.gfx_hud_stuff, {156, 307, 240, 30});
	GraphicsData.gfx_horizontal_bar_gold = extractSurface (*GraphicsData.gfx_hud_stuff, {156, 400, 240, 30});
	GraphicsData.gfx_horizontal_bar_metal = extractSurface (*GraphicsData.gfx_hud_stuff, {156, 339, 240, 30});
	GraphicsData.gfx_horizontal_bar_oil = extractSurface (*GraphicsData.gfx_hud_stuff, {156, 369, 240, 30});
	GraphicsData.gfx_horizontal_bar_slim_metal = extractSurface (*GraphicsData.gfx_hud_stuff, {156, 256, 223, 16});
	GraphicsData.gfx_horizontal_bar_slim_oil = extractSurface (*GraphicsData.gfx_hud_stuff, {156, 273, 223, 16});
	GraphicsData.gfx_horizontal_bar_slim_gold = extractSurface (*GraphicsData.gfx_hud_stuff, {156, 290, 223, 16});
	GraphicsData.gfx_vertical_bar_slim_metal = extractSurface (*GraphicsData.gfx_hud_stuff, {135, 336, 20, 115});
	GraphicsData.gfx_vertical_bar_slim_oil = extractSurface (*GraphicsData.gfx_hud_stuff, {400, 348, 20, 115});
	GraphicsData.gfx_vertical_bar_slim_gold = extractSurface (*GraphicsData.gfx_hud_stuff, {114, 336, 20, 115});

	LoadGraphicToSurface (GraphicsData.gfx_Cpfeil1, directory / "pf_1.pcx");
	LoadGraphicToSurface (GraphicsData.gfx_Cpfeil2, directory / "pf_2.pcx");
	LoadGraphicToSurface (GraphicsData.gfx_Cpfeil3, directory / "pf_3.pcx");
	LoadGraphicToSurface (GraphicsData.gfx_Cpfeil4, directory / "pf_4.pcx");
	LoadGraphicToSurface (GraphicsData.gfx_Cpfeil6, directory / "pf_6.pcx");
	LoadGraphicToSurface (GraphicsData.gfx_Cpfeil7, directory / "pf_7.pcx");
	LoadGraphicToSurface (GraphicsData.gfx_Cpfeil8, directory / "pf_8.pcx");
	LoadGraphicToSurface (GraphicsData.gfx_Cpfeil9, directory / "pf_9.pcx");
	LoadGraphicToSurface (GraphicsData.gfx_context_menu, directory / "object_menu2.pcx");
	LoadGraphicToSurface (GraphicsData.gfx_destruction, directory / "destruction.pcx");
	LoadGraphicToSurface (GraphicsData.gfx_band_small_org, directory / "band_small.pcx");
	GraphicsData.gfx_band_small = CloneSDLSurface (*GraphicsData.gfx_band_small_org);
	LoadGraphicToSurface (GraphicsData.gfx_band_big_org, directory / "band_big.pcx");
	GraphicsData.gfx_band_big = CloneSDLSurface (*GraphicsData.gfx_band_big_org);
	LoadGraphicToSurface (GraphicsData.gfx_big_beton_org, directory / "big_beton.pcx");
	GraphicsData.gfx_big_beton = CloneSDLSurface (*GraphicsData.gfx_big_beton_org);
	LoadGraphicToSurface (GraphicsData.gfx_storage, directory / "storage.pcx");
	LoadGraphicToSurface (GraphicsData.gfx_storage_ground, directory / "storage_ground.pcx");
	LoadGraphicToSurface (GraphicsData.gfx_dialog, directory / "dialog.pcx");
	LoadGraphicToSurface (GraphicsData.gfx_edock, directory / "edock.pcx");
	LoadGraphicToSurface (GraphicsData.gfx_edepot, directory / "edepot.pcx");
	LoadGraphicToSurface (GraphicsData.gfx_ehangar, directory / "ehangar.pcx");
	LoadGraphicToSurface (GraphicsData.gfx_player_pc, directory / "player_pc.pcx");
	LoadGraphicToSurface (GraphicsData.gfx_player_human, directory / "player_human.pcx");
	LoadGraphicToSurface (GraphicsData.gfx_player_none, directory / "player_none.pcx");
	LoadGraphicToSurface (GraphicsData.gfx_exitpoints_org, directory / "activate_field.pcx");
	GraphicsData.gfx_exitpoints = CloneSDLSurface (*GraphicsData.gfx_exitpoints_org);
	LoadGraphicToSurface (GraphicsData.gfx_player_select, directory / "customgame_menu.pcx");
	LoadGraphicToSurface (GraphicsData.gfx_menu_buttons, directory / "menu_buttons.pcx");
	LoadGraphicToSurface (GraphicsData.gfx_player_ready, directory / "player_ready.pcx");
	LoadGraphicToSurface (GraphicsData.gfx_hud_chatbox, directory / "hud_chatbox.pcx");

#if 0
	GraphicsData.DialogPath = cSettings::getInstance().getGfxPath() / "dialog.pcx";
	GraphicsData.Dialog2Path = cSettings::getInstance().getGfxPath() / "dialog2.pcx";
	GraphicsData.Dialog3Path = cSettings::getInstance().getGfxPath() / "dialog3.pcx";
#endif
	Log.debug ("Shadowgraphics...");
	// Shadow:
	createShadowGfx();
	Video.resolutionChanged.connect (&createShadowGfx);

	GraphicsData.gfx_tmp = AutoSurface (SDL_CreateRGBSurface (0, 128, 128, Video.getColDepth(), 0, 0, 0, 0));
	SDL_SetSurfaceBlendMode (GraphicsData.gfx_tmp.get(), SDL_BLENDMODE_BLEND);
	SDL_SetColorKey (GraphicsData.gfx_tmp.get(), SDL_TRUE, 0xFF00FF);

	// Glas:
	Log.debug ("Glassgraphic...");
	LoadGraphicToSurface (GraphicsData.gfx_destruction_glas, directory / "destruction_glas.pcx");
	SDL_SetSurfaceAlphaMod (GraphicsData.gfx_destruction_glas.get(), 150);

	// Waypoints:
	Log.debug ("Waypointgraphics...");
	OtherData.loadWayPoints();

	// Resources:
	Log.debug ("Resourcegraphics...");
	ResourceData.load (directory);
	return 1;
}

namespace
{
	//------------------------------------------------------------------------------
	struct sInitialDynamicUnitData
	{
		int ammoMax = 0;
		int shotsMax = 0;
		int range = 0;
		int damage = 0;
		int buildCost = 0;
		int speedMax = 0;
		int armor = 0;
		int hitpointsMax = 0;
		int scan = 0;

		template <typename Archive>
		void serialize (Archive& archive)
		{
			// clang-format off
			// See https://github.com/llvm/llvm-project/issues/44312
			archive & NVP (ammoMax);
			archive & NVP (shotsMax);
			archive & NVP (range);
			archive & NVP (damage);
			archive & NVP (buildCost);
			archive & NVP (speedMax);
			archive & NVP (armor);
			archive & NVP (hitpointsMax);
			archive & NVP (scan);
			// clang-format on
		}
	};

	//------------------------------------------------------------------------------
	struct sInitialBuildingData
	{
		sID id;
		std::string defaultName;
		std::string description;
		sStaticCommonUnitData commonData;
		sInitialDynamicUnitData dynamicData;
		sStaticBuildingData staticBuildingData;
		sBuildingUIStaticData graphic;

		template <typename Archive>
		void serialize (Archive& archive)
		{
			// clang-format off
			// See https://github.com/llvm/llvm-project/issues/44312
			archive & NVP (id);

			archive & NVP (defaultName);
			archive & NVP (description);
			commonData.serialize (archive);
			dynamicData.serialize (archive);
			staticBuildingData.serialize (archive);
			archive & NVP (graphic);
			// clang-format on
		}
	};

	//------------------------------------------------------------------------------
	struct sInitialVehicleData
	{
		sID id;
		std::string defaultName;
		std::string description;
		sStaticCommonUnitData commonData;
		sInitialDynamicUnitData dynamicData;
		sStaticVehicleData staticVehicleData;
		sVehicleUIStaticData graphic;

		template <typename Archive>
		void serialize (Archive& archive)
		{
			// clang-format off
			// See https://github.com/llvm/llvm-project/issues/44312
			archive & NVP (id);

			archive & NVP (defaultName);
			archive & NVP (description);
			commonData.serialize (archive);
			dynamicData.serialize (archive);
			staticVehicleData.serialize (archive);
			archive & NVP (graphic);
			// clang-format on
		}
	};
} // namespace
/**
 * Loads the unitdata from the data.json in the unitfolder
 * @param directory Unitdirectory, relative to the main game directory
 */
static void LoadUnitData (sInitialBuildingData& buildingData, const std::filesystem::path& directory)
{
	const auto path = directory / "data.json";
	if (!std::filesystem::exists (path)) return;

	std::ifstream file (path);
	nlohmann::json json;

	if (!(file >> json))
	{
		Log.warn ("Can't load " + path.u8string());
		return;
	}
	cJsonArchiveIn in (json);
	in >> buildingData;
}

/**
 * Loads the unitdata from the data.json in the unitfolder
 * @param directory Unitdirectory, relative to the main game directory
 */
static void LoadUnitData (sInitialVehicleData& vehicleData, const std::filesystem::path& directory)
{
	auto path = directory / "data.json";
	if (!std::filesystem::exists (path)) return;

	std::ifstream file (path);
	nlohmann::json json;

	if (!(file >> json))
	{
		Log.warn ("Can't load " + path.u8string());
		return;
	}
	cJsonArchiveIn in (json);
	in >> vehicleData;
}
//------------------------------------------------------------------------------
static bool checkUniqueness (const sID& id)
{
	if (ranges::any_of (UnitsDataGlobal.getStaticUnitsData(), [&] (const auto& data) { return data.ID == id; }))
	{
		char szTmp[100];
		snprintf (szTmp, sizeof (szTmp), "unit with id %.2d %.2d already exists", id.firstPart, id.secondPart);
		Log.warn (szTmp);
		return false;
	}
	return true;
}

//------------------------------------------------------------------------------
static cDynamicUnitData createDynamicUnitData (const sID& id, const sInitialDynamicUnitData& dynamic)
{
	cDynamicUnitData res;

	res.setId (id);
	res.setAmmoMax (dynamic.ammoMax);
	res.setShotsMax (dynamic.shotsMax);
	res.setRange (dynamic.range);
	res.setDamage (dynamic.damage);
	res.setBuildCost (dynamic.buildCost);
	res.setSpeedMax (dynamic.speedMax * 4);
	res.setArmor (dynamic.armor);
	res.setHitpointsMax (dynamic.hitpointsMax);
	res.setScan (dynamic.scan);
	return res;
}

//------------------------------------------------------------------------------
static cStaticUnitData createStaticUnitData (const sID& id, const sStaticCommonUnitData& commonData, const std::string& name, const std::string& desc)
{
	cStaticUnitData res;
	static_cast<sStaticCommonUnitData&> (res) = commonData;
	res.ID = id;
	res.setDefaultName (name);
	res.setDefaultDescription (desc);

	// TODO: make the code differ between attacking sea units and land units.
	// until this is done being able to attack sea units means being able to attack ground units.
	if (res.canAttack & eTerrainFlag::Sea) res.canAttack |= eTerrainFlag::Ground;

	return res;
}

/**
 * Loads a soundfile to the Mix_Chunk
 * @param dest Destination Mix_Chunk
 * @param directory Directory of the file
 * @param filename Name of the file
 * @param localize When true, sVoiceLanguage is appended to the filename. Used for loading voice files.
 * @return 1 on success
 */
static int LoadSoundfile (cSoundChunk& dest, const std::filesystem::path& filepath, bool localize = false)
{
	if (localize && !cSettings::getInstance().getVoiceLanguage().empty())
	{
		auto localizedPath = filepath.string();
		localizedPath.insert (localizedPath.rfind ("."), "_" + cSettings::getInstance().getVoiceLanguage());
		if (std::filesystem::exists (localizedPath))
		{
			dest.load (localizedPath);
			return 1;
		}
	}
	//no localized voice file. Try opening without lang code
	if (!std::filesystem::exists (filepath))
		return 0;

	dest.load (filepath);
	return 1;
}

/**
 * Loads a unitsoundfile to the Mix_Chunk. If the file doesn't exists a dummy file will be loaded
 * @param dest Destination Mix_Chunk
 * @param filepath Name of the file
 */
static void LoadUnitSoundfile (cSoundChunk& dest, const std::filesystem::path& filepath)
{
	if (SoundData.DummySound.empty())
	{
		auto sTmpString = cSettings::getInstance().getSoundsPath() / "dummy.ogg";
		if (std::filesystem::exists (sTmpString))
		{
			try
			{
				SoundData.DummySound.load (sTmpString);
			}
			catch (const std::runtime_error& e)
			{
				Log.warn (std::string ("Can't load dummy.ogg: ") + e.what());
			}
		}
	}
	if (!std::filesystem::exists (filepath))
	{
		//dest = SoundData.DummySound;
		return;
	}

	dest.load (filepath);
}

//------------------------------------------------------------------------------
static bool LoadUiData (const std::filesystem::path& sBuildingPath, sBuildingUIData& ui)
{
	// load img
	auto sTmpString = sBuildingPath / "img.pcx";
	if (std::filesystem::exists (sTmpString))
	{
		ui.img_org = LoadPCX (sTmpString);
		ui.img = CloneSDLSurface (*ui.img_org);
		SDL_SetColorKey (ui.img_org.get(), SDL_TRUE, 0xFFFFFF);
		SDL_SetColorKey (ui.img.get(), SDL_TRUE, 0xFFFFFF);
	}
	else
	{
		Log.error ("Missing GFX - your MAXR install seems to be incomplete!");
		return false;
	}
	// load shadow
	sTmpString = sBuildingPath / "shw.pcx";
	if (std::filesystem::exists (sTmpString))
	{
		ui.shw_org = LoadPCX (sTmpString);
		ui.shw = CloneSDLSurface (*ui.shw_org);
		SDL_SetSurfaceAlphaMod (ui.shw.get(), 50);
	}

	// load video
	sTmpString = sBuildingPath / "video.pcx";
	if (std::filesystem::exists (sTmpString))
		ui.video = LoadPCX (sTmpString);

	// load infoimage
	sTmpString = sBuildingPath / "info.pcx";
	if (std::filesystem::exists (sTmpString))
		ui.info = LoadPCX (sTmpString);

	// load effectgraphics if necessary
	if (ui.staticData.powerOnGraphic)
	{
		sTmpString = sBuildingPath / "effect.pcx";
		if (std::filesystem::exists (sTmpString))
		{
			ui.eff_org = LoadPCX (sTmpString);
			ui.eff = CloneSDLSurface (*ui.eff_org);
			SDL_SetSurfaceAlphaMod (ui.eff.get(), 10);
		}
	}
	else
	{
		ui.eff_org = nullptr;
		ui.eff = nullptr;
	}

	// load sounds
	LoadUnitSoundfile (ui.Wait, sBuildingPath / "wait.ogg");
	LoadUnitSoundfile (ui.Start, sBuildingPath / "start.ogg");
	LoadUnitSoundfile (ui.Running, sBuildingPath / "running.ogg");
	LoadUnitSoundfile (ui.Stop, sBuildingPath / "stop.ogg");
	LoadUnitSoundfile (ui.Attack, sBuildingPath / "attack.ogg");

	// Get Ptr if necessary:
	if (ui.id == UnitsDataGlobal.getConnectorID())
	{
		ui.isConnectorGraphic = true;
		UnitsUiData.ptr_connector = ui.img.get();
		UnitsUiData.ptr_connector_org = ui.img_org.get();
		SDL_SetColorKey (UnitsUiData.ptr_connector, SDL_TRUE, 0xFF00FF);
		UnitsUiData.ptr_connector_shw = ui.shw.get();
		UnitsUiData.ptr_connector_shw_org = ui.shw_org.get();
		SDL_SetColorKey (UnitsUiData.ptr_connector_shw, SDL_TRUE, 0xFF00FF);
	}
	else if (ui.id == UnitsDataGlobal.getSmallBetonID())
	{
		UnitsUiData.ptr_small_beton = ui.img.get();
		UnitsUiData.ptr_small_beton_org = ui.img_org.get();
		SDL_SetColorKey (UnitsUiData.ptr_small_beton, SDL_TRUE, 0xFF00FF);
	}

	// Check if there is more than one frame
	// use 129 here because some images from the res_installer are one pixel too large
	if (ui.img_org->w > 129 && !ui.isConnectorGraphic && !ui.staticData.hasClanLogos)
		ui.hasFrames = ui.img_org->w / ui.img_org->h;
	else
		ui.hasFrames = 0;

	return true;
}

//------------------------------------------------------------------------------
static bool LoadUiData (const std::filesystem::path& sVehiclePath, const cStaticUnitData& staticData, sVehicleUIData& ui)
{
	Log.debug ("Loading graphics");
	// load infantry graphics
	if (staticData.vehicleData.animationMovement)
	{
		SDL_Rect rcDest;
		for (int n = 0; n < 8; n++)
		{
			ui.img[n] = AutoSurface (SDL_CreateRGBSurface (0, 64 * 13, 64, Video.getColDepth(), 0, 0, 0, 0));
			SDL_SetColorKey (ui.img[n].get(), SDL_TRUE, 0x00FFFFFF);
			SDL_FillRect (ui.img[n].get(), nullptr, 0x00FF00FF);

			for (int j = 0; j < 13; j++)
			{
				char sztmp[16];
				snprintf (sztmp, sizeof (sztmp), "img%d_%.2d.pcx", n, j);
				auto sTmpString = sVehiclePath / sztmp;

				if (std::filesystem::exists (sTmpString))
				{
					Log.debug (sTmpString.u8string());
					AutoSurface sfTempSurface (LoadPCX (sTmpString));
					if (!sfTempSurface)
					{
						Log.warn (SDL_GetError());
					}
					else
					{
						rcDest.x = 64 * j + 32 - sfTempSurface->w / 2;
						rcDest.y = 32 - sfTempSurface->h / 2;
						SDL_BlitSurface (sfTempSurface.get(), nullptr, ui.img[n].get(), &rcDest);
					}
				}
			}
			ui.img_org[n] = AutoSurface (SDL_CreateRGBSurface (0, 64 * 13, 64, Video.getColDepth(), 0, 0, 0, 0));
			SDL_SetColorKey (ui.img[n].get(), SDL_TRUE, 0x00FFFFFF);
			SDL_FillRect (ui.img_org[n].get(), nullptr, 0x00FFFFFF);
			SDL_BlitSurface (ui.img[n].get(), nullptr, ui.img_org[n].get(), nullptr);

			ui.shw[n] = AutoSurface (SDL_CreateRGBSurface (0, 64 * 13, 64, Video.getColDepth(), 0, 0, 0, 0));
			SDL_SetColorKey (ui.shw[n].get(), SDL_TRUE, 0x00FF00FF);
			SDL_FillRect (ui.shw[n].get(), nullptr, 0x00FF00FF);
			ui.shw_org[n] = AutoSurface (SDL_CreateRGBSurface (0, 64 * 13, 64, Video.getColDepth(), 0, 0, 0, 0));
			SDL_SetColorKey (ui.shw_org[n].get(), SDL_TRUE, 0x00FF00FF);
			SDL_FillRect (ui.shw_org[n].get(), nullptr, 0x00FF00FF);

			rcDest.x = 3;
			rcDest.y = 3;
			SDL_BlitSurface (ui.img_org[n].get(), nullptr, ui.shw_org[n].get(), &rcDest);
			SDL_LockSurface (ui.shw_org[n].get());
			Uint32* ptr = static_cast<Uint32*> (ui.shw_org[n]->pixels);
			for (int j = 0; j < 64 * 13 * 64; j++)
			{
				if (*ptr != 0x00FF00FF)
					*ptr = 0;
				ptr++;
			}
			SDL_UnlockSurface (ui.shw_org[n].get());
			SDL_BlitSurface (ui.shw_org[n].get(), nullptr, ui.shw[n].get(), nullptr);
			SDL_SetSurfaceAlphaMod (ui.shw_org[n].get(), 50);
			SDL_SetSurfaceAlphaMod (ui.shw[n].get(), 50);
		}
	}
	// load other vehicle graphics
	else
	{
		for (int n = 0; n < 8; n++)
		{
			// load image
			char sztmp[16];
			snprintf (sztmp, sizeof (sztmp), "img%d.pcx", n);
			auto sTmpString = sVehiclePath / sztmp;
			Log.debug (sTmpString.u8string());
			if (std::filesystem::exists (sTmpString))
			{
				ui.img_org[n] = LoadPCX (sTmpString);
				ui.img[n] = CloneSDLSurface (*ui.img_org[n]);
				SDL_SetColorKey (ui.img_org[n].get(), SDL_TRUE, 0xFFFFFF);
				SDL_SetColorKey (ui.img[n].get(), SDL_TRUE, 0xFFFFFF);
			}
			else
			{
				Log.error ("Missing GFX - your MAXR install seems to be incomplete!");
				return false;
			}

			// load shadow
			snprintf (sztmp, sizeof (sztmp), "shw%d.pcx", n);
			sTmpString = sVehiclePath / sztmp;
			if (std::filesystem::exists (sTmpString))
			{
				ui.shw_org[n] = LoadPCX (sTmpString);
				ui.shw[n] = CloneSDLSurface (*ui.shw_org[n]);
				SDL_SetSurfaceAlphaMod (ui.shw[n].get(), 50);
			}
			else
			{
				ui.shw_org[n] = nullptr;
				ui.shw[n] = nullptr;
			}
		}
	}
	// load video
	ui.FLCFile = sVehiclePath / "video.flc";
	Log.debug ("Loading video: " + ui.FLCFile.u8string());
	if (!std::filesystem::exists (ui.FLCFile))
	{
		ui.FLCFile = "";
	}

	// load infoimage
	auto sTmpString = sVehiclePath / "info.pcx";
	Log.debug ("Loading portrait: " + sTmpString.u8string());
	if (std::filesystem::exists (sTmpString))
	{
		ui.info = LoadPCX (sTmpString);
	}
	else
	{
		Log.error ("Missing GFX - your MAXR install seems to be incomplete!");
		return false;
	}

	// load storageimage
	sTmpString = sVehiclePath / "store.pcx";
	Log.debug ("Loading storageportrait: " + sTmpString.u8string());
	if (std::filesystem::exists (sTmpString))
	{
		ui.storage = LoadPCX (sTmpString);
	}
	else
	{
		Log.error ("Missing GFX - your MAXR install seems to be incomplete!");
		return false;
	}

	// load overlaygraphics if necessary
	if (ui.staticData.hasOverlay)
	{
		sTmpString = sVehiclePath / "overlay.pcx";
		Log.debug ("Loading overlay: " + sTmpString.u8string());
		if (std::filesystem::exists (sTmpString))
		{
			ui.overlay_org = LoadPCX (sTmpString);
			ui.overlay = CloneSDLSurface (*ui.overlay_org);
		}
		else
		{
			Log.warn ("Missing GFX - your MAXR install seems to be incomplete!");
			ui.overlay_org = nullptr;
			ui.overlay = nullptr;
			ui.staticData.hasOverlay = false;
		}
	}
	else
	{
		ui.overlay_org = nullptr;
		ui.overlay = nullptr;
	}

	// load buildgraphics if necessary
	if (ui.staticData.buildUpGraphic)
	{
		// load image
		sTmpString = sVehiclePath / "build.pcx";
		Log.debug ("Loading buildgraphics: " + sTmpString.u8string());
		if (std::filesystem::exists (sTmpString))
		{
			ui.build_org = LoadPCX (sTmpString);
			ui.build = CloneSDLSurface (*ui.build_org);
			SDL_SetColorKey (ui.build_org.get(), SDL_TRUE, 0xFFFFFF);
			SDL_SetColorKey (ui.build.get(), SDL_TRUE, 0xFFFFFF);
		}
		else
		{
			Log.warn ("Missing GFX - your MAXR install seems to be incomplete!");
			ui.build_org = nullptr;
			ui.build = nullptr;
			ui.staticData.buildUpGraphic = false;
		}
		// load shadow
		sTmpString = sVehiclePath / "build_shw.pcx";
		Log.debug ("Loading buildgraphics: " + sTmpString.u8string());
		if (std::filesystem::exists (sTmpString))
		{
			ui.build_shw_org = LoadPCX (sTmpString);
			ui.build_shw = CloneSDLSurface (*ui.build_shw_org);
			SDL_SetSurfaceAlphaMod (ui.build_shw.get(), 50);
		}
		else
		{
			Log.warn ("Missing GFX - your MAXR install seems to be incomplete!");
			ui.build_shw_org = nullptr;
			ui.build_shw = nullptr;
			ui.staticData.buildUpGraphic = false;
		}
	}
	else
	{
		ui.build_org = nullptr;
		ui.build = nullptr;
		ui.build_shw_org = nullptr;
		ui.build_shw = nullptr;
	}
	// load cleargraphics if necessary
	if (staticData.vehicleData.canClearArea)
	{
		// load image (small)
		sTmpString = sVehiclePath / "clear_small.pcx";
		Log.debug ("Loading cleargraphics: " + sTmpString.u8string());
		if (std::filesystem::exists (sTmpString))
		{
			ui.clear_small_org = LoadPCX (sTmpString);
			ui.clear_small = CloneSDLSurface (*ui.clear_small_org);
			SDL_SetColorKey (ui.clear_small_org.get(), SDL_TRUE, 0xFFFFFF);
			SDL_SetColorKey (ui.clear_small.get(), SDL_TRUE, 0xFFFFFF);
		}
		else
		{
			Log.warn ("Missing GFX - your MAXR install seems to be incomplete!");
			ui.clear_small_org = nullptr;
			ui.clear_small = nullptr;
			return false;
		}
		// load shadow (small)
		sTmpString = sVehiclePath / "clear_small_shw.pcx";
		Log.debug ("Loading cleargraphics: " + sTmpString.u8string());
		if (std::filesystem::exists (sTmpString))
		{
			ui.clear_small_shw_org = LoadPCX (sTmpString);
			ui.clear_small_shw = CloneSDLSurface (*ui.clear_small_shw_org);
			SDL_SetSurfaceAlphaMod (ui.clear_small_shw.get(), 50);
		}
		else
		{
			Log.warn ("Missing GFX - your MAXR install seems to be incomplete!");
			ui.clear_small_shw_org = nullptr;
			ui.clear_small_shw = nullptr;
			return false;
		}
		// load image (big)
		sTmpString = sVehiclePath / "clear_big.pcx";
		Log.debug ("Loading cleargraphics: " + sTmpString.u8string());
		if (std::filesystem::exists (sTmpString))
		{
			ui.build_org = LoadPCX (sTmpString);
			ui.build = CloneSDLSurface (*ui.build_org);
			SDL_SetColorKey (ui.build_org.get(), SDL_TRUE, 0xFFFFFF);
			SDL_SetColorKey (ui.build.get(), SDL_TRUE, 0xFFFFFF);
		}
		else
		{
			Log.warn ("Missing GFX - your MAXR install seems to be incomplete!");
			ui.build_org = nullptr;
			ui.build = nullptr;
			return false;
		}
		// load shadow (big)
		sTmpString = sVehiclePath / "clear_big_shw.pcx";
		Log.debug ("Loading cleargraphics: " + sTmpString.u8string());
		if (std::filesystem::exists (sTmpString))
		{
			ui.build_shw_org = LoadPCX (sTmpString);
			ui.build_shw = CloneSDLSurface (*ui.build_shw_org);
			SDL_SetSurfaceAlphaMod (ui.build_shw.get(), 50);
		}
		else
		{
			Log.warn ("Missing GFX - your MAXR install seems to be incomplete!");
			ui.build_shw_org = nullptr;
			ui.build_shw = nullptr;
			return false;
		}
	}
	else
	{
		ui.clear_small_org = nullptr;
		ui.clear_small = nullptr;
		ui.clear_small_shw_org = nullptr;
		ui.clear_small_shw = nullptr;
	}

	// load sounds
	Log.debug ("Loading sounds");
	LoadUnitSoundfile (ui.Wait, sVehiclePath / "wait.ogg");
	LoadUnitSoundfile (ui.WaitWater, sVehiclePath / "wait_water.ogg");
	LoadUnitSoundfile (ui.Start, sVehiclePath / "start.ogg");
	LoadUnitSoundfile (ui.StartWater, sVehiclePath / "start_water.ogg");
	LoadUnitSoundfile (ui.Stop, sVehiclePath / "stop.ogg");
	LoadUnitSoundfile (ui.StopWater, sVehiclePath / "stop_water.ogg");
	LoadUnitSoundfile (ui.Drive, sVehiclePath / "drive.ogg");
	LoadUnitSoundfile (ui.DriveWater, sVehiclePath / "drive_water.ogg");
	LoadUnitSoundfile (ui.Attack, sVehiclePath / "attack.ogg");
	return true;
}
namespace
{

	struct sUnitDirectory
	{
		int id;
		std::filesystem::path path;
		int insertionIndex = ++currentIndex;
		static int currentIndex;

		template <typename Archive>
		void serialize (Archive& archive)
		{
			// clang-format off
			// See https://github.com/llvm/llvm-project/issues/44312
			archive & NVP (id);
			archive & NVP (path);
			// clang-format on
		}
	};

	/*static*/ int sUnitDirectory::currentIndex = 0;

	//--------------------------------------------------------------------------
	void checkDuplicateId (std::vector<sUnitDirectory>& v)
	{
		std::sort (v.begin(), v.end(), [] (const auto& lhs, const auto& rhs) { return lhs.id < rhs.id; });
		auto sameId = [] (const auto& lhs, const auto& rhs) { return lhs.id == rhs.id; };
		auto it = std::adjacent_find (v.begin(), v.end(), sameId);
		while (it != v.end())
		{
			Log.warn ("duplicated id " + std::to_string (it->id) + ", skipping unit.");
			it = std::adjacent_find (it + 1, v.end(), sameId);
		}
		v.erase (std::unique (v.begin(), v.end(), sameId), v.end());
		std::sort (v.begin(), v.end(), [] (const auto& lhs, const auto& rhs) { return lhs.insertionIndex < rhs.insertionIndex; });
	}

	struct sBuildingsList
	{
		sSpecialBuildingsId special;
		std::vector<sUnitDirectory> buildings;

		template <typename Archive>
		void serialize (Archive& archive)
		{
			// clang-format off
			// See https://github.com/llvm/llvm-project/issues/44312
			archive & NVP (special);
			archive & NVP (buildings);
			// clang-format on
		}
	};

	struct sVehiclesList
	{
		std::vector<sUnitDirectory> vehicles;

		template <typename Archive>
		void serialize (Archive& archive)
		{
			// clang-format off
			// See https://github.com/llvm/llvm-project/issues/44312
			archive & NVP (vehicles);
			// clang-format on
		}
	};

} // namespace
/**
 * Loads all Buildings
 * @return 1 on success
 */
static int LoadBuildings (bool includingUiData)
{
	Log.info ("Loading Buildings");

	auto buildingsJsonPath = cSettings::getInstance().getBuildingsPath() / "buildings.json";
	if (!std::filesystem::exists (buildingsJsonPath))
	{
		Log.error ("buildings.json doesn't exist!");
		return 0;
	}

	std::ifstream file (buildingsJsonPath);
	nlohmann::json json;

	if (!(file >> json))
	{
		Log.error ("Can't load " + buildingsJsonPath.u8string());
		return 0;
	}
	sBuildingsList buildingsList;
	cJsonArchiveIn in (json);
	in >> buildingsList;

	checkDuplicateId (buildingsList.buildings);
	buildingsList.special.logMissing();
	UnitsDataGlobal.setSpecialBuildingIDs (buildingsList.special);

	// load found units
	UnitsUiData.buildingUIs.reserve (buildingsList.buildings.size());
	for (const auto& p : buildingsList.buildings)
	{
		const auto sBuildingPath = cSettings::getInstance().getBuildingsPath() / p.path;

		sInitialBuildingData buildingData;
		LoadUnitData (buildingData, sBuildingPath);

		if (p.id != buildingData.id.secondPart)
		// check whether the read id is the same as the one from building.json
		{
			Log.error ("ID " + std::to_string (p.id) + " isn't equal with ID from directory " + sBuildingPath.u8string());
			return 0;
		}
		else
		{
			Log.debug ("id " + std::to_string (p.id) + " verified for " + sBuildingPath.u8string());
		}
		if (!checkUniqueness (buildingData.id)) return 0;

		cStaticUnitData staticData = createStaticUnitData (buildingData.id, buildingData.commonData, buildingData.defaultName, buildingData.description);
		cDynamicUnitData dynamicData = createDynamicUnitData (buildingData.id, buildingData.dynamicData);
		staticData.buildingData = buildingData.staticBuildingData;

		if (includingUiData)
		{
			UnitsUiData.buildingUIs.emplace_back();
			sBuildingUIData& ui = UnitsUiData.buildingUIs.back();

			ui.id = buildingData.id;
			ui.staticData = buildingData.graphic;

			if (!LoadUiData (sBuildingPath, ui))
			{
				return -1;
			}
		}
		UnitsDataGlobal.addData (staticData);
		UnitsDataGlobal.addData (dynamicData);

		if (cSettings::getInstance().isDebug()) Log.mark();
	}

	// Dirtsurfaces
	if (includingUiData)
	{
		LoadGraphicToSurface (UnitsUiData.rubbleBig->img_org, cSettings::getInstance().getBuildingsPath() / "dirt_big.pcx");
		UnitsUiData.rubbleBig->img = CloneSDLSurface (*UnitsUiData.rubbleBig->img_org);
		LoadGraphicToSurface (UnitsUiData.rubbleBig->shw_org, cSettings::getInstance().getBuildingsPath() / "dirt_big_shw.pcx");
		UnitsUiData.rubbleBig->shw = CloneSDLSurface (*UnitsUiData.rubbleBig->shw_org);
		if (UnitsUiData.rubbleBig->shw != nullptr) SDL_SetSurfaceAlphaMod (UnitsUiData.rubbleBig->shw.get(), 50);

		LoadGraphicToSurface (UnitsUiData.rubbleSmall->img_org, cSettings::getInstance().getBuildingsPath() / "dirt_small.pcx");
		UnitsUiData.rubbleSmall->img = CloneSDLSurface (*UnitsUiData.rubbleSmall->img_org);
		LoadGraphicToSurface (UnitsUiData.rubbleSmall->shw_org, cSettings::getInstance().getBuildingsPath() / "dirt_small_shw.pcx");
		UnitsUiData.rubbleSmall->shw = CloneSDLSurface (*UnitsUiData.rubbleSmall->shw_org);
		if (UnitsUiData.rubbleSmall->shw != nullptr) SDL_SetSurfaceAlphaMod (UnitsUiData.rubbleSmall->shw.get(), 50);
	}
	return 1;
}

/**
 * Loads all Vehicles
 * @return 1 on success
 */
static int LoadVehicles (bool includingUiData)
{
	Log.info ("Loading Vehicles");

	auto vehicleJsonPath = cSettings::getInstance().getVehiclesPath() / "vehicles.json";
	if (!std::filesystem::exists (vehicleJsonPath))
	{
		Log.error ("vehicles.json doesn't exist!");
		return 0;
	}

	std::ifstream file (vehicleJsonPath);
	nlohmann::json json;

	if (!(file >> json))
	{
		Log.error ("Can't load " + vehicleJsonPath.u8string());
		return 0;
	}
	sVehiclesList vehiclesList;
	cJsonArchiveIn in (json);
	in >> vehiclesList;
	checkDuplicateId (vehiclesList.vehicles);

	// load found units
	UnitsUiData.vehicleUIs.reserve (vehiclesList.vehicles.size());
	for (const auto& p : vehiclesList.vehicles)
	{
		auto sVehiclePath = cSettings::getInstance().getVehiclesPath() / p.path;

		sInitialVehicleData vehicleData;
		LoadUnitData (vehicleData, sVehiclePath);

		// check whether the read id is the same as the one from vehicles.json
		if (p.id != vehicleData.id.secondPart)
		{
			Log.error ("ID " + std::to_string (p.id) + " isn't equal with ID from directory " + sVehiclePath.u8string());
			return 0;
		}
		else
		{
			Log.debug ("id " + std::to_string (p.id) + " verified for " + sVehiclePath.u8string());
		}
		if (!checkUniqueness (vehicleData.id)) return 0;

		cStaticUnitData staticData = createStaticUnitData (vehicleData.id, vehicleData.commonData, vehicleData.defaultName, vehicleData.description);
		cDynamicUnitData dynamicData = createDynamicUnitData (vehicleData.id, vehicleData.dynamicData);

		if (staticData.factorGround == 0 && staticData.factorSea == 0 && staticData.factorAir == 0 && staticData.factorCoast == 0)
		{
			Log.warn ("Unit cannot move");
		}
		staticData.vehicleData = vehicleData.staticVehicleData;

		if (includingUiData)
		{
			UnitsUiData.vehicleUIs.emplace_back();
			sVehicleUIData& ui = UnitsUiData.vehicleUIs.back();

			ui.id = vehicleData.id;
			ui.staticData = vehicleData.graphic;

			if (!LoadUiData (sVehiclePath, staticData, ui))
			{
				return -1;
			}
		}
		UnitsDataGlobal.addData (staticData);
		UnitsDataGlobal.addData (dynamicData);
		if (cSettings::getInstance().isDebug()) Log.mark();
	}

	UnitsDataGlobal.initializeIDData();

	return 1;
}

/**
 * Loads the clan values and stores them in the cUnitData class
 * @return 1 on success
 */
static int LoadClans()
{
	auto clansPath = cSettings::getInstance().getDataDir() / "clans.json";

	if (!std::filesystem::exists (clansPath))
	{
		Log.error ("File doesn't exist: " + clansPath.u8string());
		return 0;
	}
	std::ifstream file (clansPath);
	nlohmann::json json;
	if (!(file >> json))
	{
		Log.error ("Can't load " + clansPath.u8string());
		return 0;
	}
	cJsonArchiveIn in (json);

	in >> ClanDataGlobal;

	UnitsDataGlobal.initializeClanUnitData (ClanDataGlobal);
	return 1;
}

/**
 * Loads all Musicfiles
 * @param path Directory of the Vehicles
 * @return 1 on success
 */
static int LoadMusic (const std::filesystem::path& directory)
{
	const auto musicPath = directory / "musics.json";

	Log.info ("Loading music: " + musicPath.u8string());
	if (!std::filesystem::exists (musicPath))
	{
		Log.error ("file doesn't exist");
		return 0;
	}
	std::ifstream file (musicPath);
	nlohmann::json json;

	if (!(file >> json))
	{
		Log.error ("Can't load music.json");
		return 0;
	}

	cJsonArchiveIn in (json);

	in >> MusicFiles;

	if (!MusicFiles.start.empty())
	{
		MusicFiles.start = directory / MusicFiles.start;
		if (!std::filesystem::exists (MusicFiles.start)) Log.warn ("music file doesn't exist: " + MusicFiles.start.u8string());
	}
	for (auto& filename : MusicFiles.backgrounds)
	{
		filename = directory / filename;
	}
	auto it = std::stable_partition (MusicFiles.backgrounds.begin(), MusicFiles.backgrounds.end(), [] (const auto& p) { return std::filesystem::exists (p); });
	for (auto it2 = it; it2 != MusicFiles.backgrounds.end(); ++it2)
	{
		Log.warn ("music file doesn't exist: " + it2->string());
	}
	MusicFiles.backgrounds.erase (it, MusicFiles.backgrounds.end());
	return 1;
}

//------------------------------------------------------------------------------
bool loadFonts()
{
	const auto& fontPath = cSettings::getInstance().getFontPath();
	if (!std::filesystem::exists (fontPath / "latin_normal.pcx")
	    || !std::filesystem::exists (fontPath / "latin_big.pcx")
	    || !std::filesystem::exists (fontPath / "latin_big_gold.pcx")
	    || !std::filesystem::exists (fontPath / "latin_small.pcx"))
	{
		Log.error ("Missing a file needed for game. Check log and config! ");
		return false;
	}

	cUnicodeFont::font = std::make_unique<cUnicodeFont>(); // init ascii fonts
	cUnicodeFont::font->setTargetSurface (cVideo::buffer);
	return true;
}

// LoadData ///////////////////////////////////////////////////////////////////
// Loads all relevant files and data:
eLoadingState LoadData (bool includingUiData)
{
	CR_ENABLE_CRASH_RPT_CURRENT_THREAD();

	if (includingUiData)
	{
		if (!loadFonts())
		{
			return eLoadingState::Error;
		}
		Log.mark();
	}
	auto MakeLog = includingUiData ? WindowMakeLog : ConsoleMakeLog;

	MakeLog (getBuildVersion(), 0, 0);

	// Load Languagepack
	MakeLog ("Loading languagepack...", 0, 2);
	LoadLanguage();
	if (includingUiData && cSettings::getInstance().isDebug())
	{
		debugTranslationSize (lngPack, *cUnicodeFont::font);
	}
	MakeLog ("", 1, 2);
	Log.mark();

	if (includingUiData)
	{
		// Load Keys
		MakeLog (lngPack.i18n ("Init~Keys"), 0, 3);

		try
		{
			KeysList.loadFromFile();
			MakeLog ("", 1, 3);
		}
		catch (std::runtime_error& e)
		{
			Log.error (e.what());
			MakeLog ("", -1, 3);
			return eLoadingState::Error;
		}
		Log.mark();

		// Load Fonts
		MakeLog (lngPack.i18n ("Init~Fonts"), 0, 4);
		// -- little bit crude but fonts are already loaded.
		// what to do with this now? -- beko
		// Really loaded with new cUnicodeFont
		MakeLog ("", 1, 4);
		Log.mark();

		// Load Graphics
		MakeLog (lngPack.i18n ("Init~GFX"), 0, 5);

		if (LoadGraphics (cSettings::getInstance().getGfxPath()) != 1)
		{
			MakeLog ("", -1, 5);
			Log.error ("Error while loading graphics");
			return eLoadingState::Error;
		}
		else
		{
			MakeLog ("", 1, 5);
		}
		Log.mark();

		// Load Effects
		MakeLog (lngPack.i18n ("Init~Effects"), 0, 6);
		Log.info ("Loading Effects");
		EffectsData.load (cSettings::getInstance().getFxPath());
		MakeLog ("", 1, 6);
		Log.mark();
	}

	// Load Vehicles
	MakeLog (lngPack.i18n ("Init~Vehicles"), 0, 7);

	if (LoadVehicles (includingUiData) != 1)
	{
		MakeLog ("", -1, 7);
		return eLoadingState::Error;
	}
	else
	{
		MakeLog ("", 1, 7);
	}
	Log.mark();

	// Load Buildings
	MakeLog (lngPack.i18n ("Init~Buildings"), 0, 8);

	if (LoadBuildings (includingUiData) != 1)
	{
		MakeLog ("", -1, 8);
		return eLoadingState::Error;
	}
	else
	{
		MakeLog ("", 1, 8);
	}
	Log.mark();

	MakeLog (lngPack.i18n ("Init~Clans"), 0, 9);

	// Load Clan Settings
	if (LoadClans() != 1)
	{
		return eLoadingState::Error;
	}
	else
	{
		MakeLog ("", 1, 9);
	}
	Log.mark();

	if (includingUiData)
	{
		// Load Music
		MakeLog (lngPack.i18n ("Init~Music"), 0, 10);

		if (LoadMusic (cSettings::getInstance().getMusicPath()) != 1)
		{
			MakeLog ("", -1, 10);
			return eLoadingState::Error;
		}
		else
		{
			MakeLog ("", 1, 10);
		}
		Log.mark();

		// Load Sounds
		MakeLog (lngPack.i18n ("Init~Sounds"), 0, 11);
		Log.info ("Loading Sounds");
		SoundData.load (cSettings::getInstance().getSoundsPath());
		MakeLog ("", 1, 11);
		Log.mark();

		// Load Voices
		MakeLog (lngPack.i18n ("Init~Voices"), 0, 12);
		Log.info ("Loading Voices");
		VoiceData.load (cSettings::getInstance().getVoicesPath());
		MakeLog ("", 1, 12);
		Log.mark();
	}
	return eLoadingState::Finished;
}

/**
 * Loads a effectgraphic to the surface
 * @param dest Destination surface
 * @param filepath Name of the file
 * @return 1 on success
 */
static int LoadEffectGraphicToSurface (AutoSurface (&dest)[2], const std::filesystem::path& filepath)
{
	if (!std::filesystem::exists (filepath))
	{
		Log.error ("Missing GFX - your MAXR install seems to be incomplete!");
		return 0;
	}

	dest[0] = LoadPCX (filepath);
	dest[1] = CloneSDLSurface (*dest[0]);

	Log.debug ("Effect successful loaded: " + filepath.u8string());
	return 1;
}

// LoadEffectAlphacToSurface /////////////////////////////////////////////////
// Loads a effectgraphic as alpha to the surface:
static int LoadEffectAlphaToSurface (AutoSurface (&dest)[2], const std::filesystem::path& filepath, int alpha)
{
	if (!std::filesystem::exists (filepath))
		return 0;

	dest[0] = LoadPCX (filepath);
	dest[1] = CloneSDLSurface (*dest[0]);
	SDL_SetSurfaceAlphaMod (dest[0].get(), alpha);
	SDL_SetSurfaceAlphaMod (dest[1].get(), alpha);

	Log.debug ("Effectalpha loaded: " + filepath.u8string());
	return 1;
}

//------------------------------------------------------------------------------
void cEffectsData::load (const std::filesystem::path& directory)
{
	LoadEffectGraphicToSurface (fx_explo_small, directory / "explo_small.pcx");
	LoadEffectGraphicToSurface (fx_explo_big, directory / "explo_big.pcx");
	LoadEffectGraphicToSurface (fx_explo_water, directory / "explo_water.pcx");
	LoadEffectGraphicToSurface (fx_explo_air, directory / "explo_air.pcx");
	LoadEffectGraphicToSurface (fx_muzzle_big, directory / "muzzle_big.pcx");
	LoadEffectGraphicToSurface (fx_muzzle_small, directory / "muzzle_small.pcx");
	LoadEffectGraphicToSurface (fx_muzzle_med, directory / "muzzle_med.pcx");
	LoadEffectGraphicToSurface (fx_hit, directory / "hit.pcx");
	LoadEffectAlphaToSurface (fx_smoke, directory / "smoke.pcx", 100);
	LoadEffectGraphicToSurface (fx_rocket, directory / "rocket.pcx");
	LoadEffectAlphaToSurface (fx_dark_smoke, directory / "dark_smoke.pcx", 100);
	LoadEffectAlphaToSurface (fx_tracks, directory / "tracks.pcx", 100);
	LoadEffectAlphaToSurface (fx_corpse, directory / "corpse.pcx", 254);
	LoadEffectAlphaToSurface (fx_absorb, directory / "absorb.pcx", 150);
}

//------------------------------------------------------------------------------
void cSoundData::load (const std::filesystem::path& directory)
{
	LoadSoundfile (SNDHudSwitch, directory / "HudSwitch.ogg");
	LoadSoundfile (SNDHudButton, directory / "HudButton.ogg");
	LoadSoundfile (SNDMenuButton, directory / "MenuButton.ogg");
	LoadSoundfile (SNDChat, directory / "Chat.ogg");
	LoadSoundfile (SNDObjectMenu, directory / "ObjectMenu.ogg");
	LoadSoundfile (EXPBigWet[0], directory / "exp_big_wet0.ogg");
	LoadSoundfile (EXPBigWet[1], directory / "exp_big_wet1.ogg");
	LoadSoundfile (EXPBig[0], directory / "exp_big0.ogg");
	LoadSoundfile (EXPBig[1], directory / "exp_big1.ogg");
	LoadSoundfile (EXPBig[2], directory / "exp_big2.ogg");
	LoadSoundfile (EXPBig[3], directory / "exp_big3.ogg");
	LoadSoundfile (EXPSmallWet[0], directory / "exp_small_wet0.ogg");
	LoadSoundfile (EXPSmallWet[1], directory / "exp_small_wet1.ogg");
	LoadSoundfile (EXPSmallWet[2], directory / "exp_small_wet2.ogg");
	LoadSoundfile (EXPSmall[0], directory / "exp_small0.ogg");
	LoadSoundfile (EXPSmall[1], directory / "exp_small1.ogg");
	LoadSoundfile (EXPSmall[2], directory / "exp_small2.ogg");
	LoadSoundfile (SNDArm, directory / "arm.ogg");
	LoadSoundfile (SNDBuilding, directory / "building.ogg");
	LoadSoundfile (SNDClearing, directory / "clearing.ogg");
	LoadSoundfile (SNDQuitsch, directory / "quitsch.ogg");
	LoadSoundfile (SNDActivate, directory / "activate.ogg");
	LoadSoundfile (SNDLoad, directory / "load.ogg");
	LoadSoundfile (SNDReload, directory / "reload.ogg");
	LoadSoundfile (SNDRepair, directory / "repair.ogg");
	LoadSoundfile (SNDLandMinePlace, directory / "land_mine_place.ogg");
	LoadSoundfile (SNDLandMineClear, directory / "land_mine_clear.ogg");
	LoadSoundfile (SNDSeaMinePlace, directory / "sea_mine_place.ogg");
	LoadSoundfile (SNDSeaMineClear, directory / "sea_mine_clear.ogg");
	LoadSoundfile (SNDPanelOpen, directory / "panel_open.ogg");
	LoadSoundfile (SNDPanelClose, directory / "panel_close.ogg");
	LoadSoundfile (SNDAbsorb, directory / "absorb.ogg");
	LoadSoundfile (SNDHitSmall, directory / "hit_small.ogg");
	LoadSoundfile (SNDHitMed, directory / "hit_med.ogg");
	LoadSoundfile (SNDHitLarge, directory / "hit_large.ogg");
	LoadSoundfile (SNDPlaneLand, directory / "plane_land.ogg");
	LoadSoundfile (SNDPlaneTakeoff, directory / "plane_takeoff.ogg");
}

//------------------------------------------------------------------------------
void cVoiceData::load (const std::filesystem::path& directory)
{
	LoadSoundfile (VOIAmmoLow[0], directory / "ammo_low1.ogg", true);
	LoadSoundfile (VOIAmmoLow[1], directory / "ammo_low2.ogg", true);
	LoadSoundfile (VOIAmmoEmpty[0], directory / "ammo_empty1.ogg", true);
	LoadSoundfile (VOIAmmoEmpty[1], directory / "ammo_empty2.ogg", true);
	LoadSoundfile (VOIAttacking[0], directory / "attacking1.ogg", true);
	LoadSoundfile (VOIAttacking[1], directory / "attacking2.ogg", true);
	LoadSoundfile (VOIAttackingEnemy[0], directory / "attacking_enemy1.ogg", true);
	LoadSoundfile (VOIAttackingEnemy[1], directory / "attacking_enemy2.ogg", true);
	LoadSoundfile (VOIAttackingUs[0], directory / "attacking_us.ogg", true);
	LoadSoundfile (VOIAttackingUs[1], directory / "attacking_us2.ogg", true);
	LoadSoundfile (VOIAttackingUs[2], directory / "attacking_us3.ogg", true);
	LoadSoundfile (VOIBuildDone[0], directory / "build_done1.ogg", true);
	LoadSoundfile (VOIBuildDone[1], directory / "build_done2.ogg", true);
	LoadSoundfile (VOIBuildDone[2], directory / "build_done3.ogg", true);
	LoadSoundfile (VOIBuildDone[3], directory / "build_done4.ogg", true);
	LoadSoundfile (VOIClearing, directory / "clearing.ogg", true);
	LoadSoundfile (VOIClearingMines[0], directory / "clearing_mines.ogg", true);
	LoadSoundfile (VOIClearingMines[1], directory / "clearing_mines2.ogg", true);
	LoadSoundfile (VOICommandoFailed[0], directory / "commando_failed1.ogg", true);
	LoadSoundfile (VOICommandoFailed[1], directory / "commando_failed2.ogg", true);
	LoadSoundfile (VOICommandoFailed[2], directory / "commando_failed3.ogg", true);
	LoadSoundfile (VOIDestroyedUs[0], directory / "destroyed_us1.ogg", true);
	LoadSoundfile (VOIDestroyedUs[1], directory / "destroyed_us2.ogg", true);
	LoadSoundfile (VOIDetected[0], directory / "detected1.ogg", true);
	LoadSoundfile (VOIDetected[1], directory / "detected2.ogg", true);
	LoadSoundfile (VOILanding[0], directory / "landing1.ogg", true);
	LoadSoundfile (VOILanding[1], directory / "landing2.ogg", true);
	LoadSoundfile (VOILanding[2], directory / "landing3.ogg", true);
	LoadSoundfile (VOILayingMines, directory / "laying_mines.ogg", true);
	LoadSoundfile (VOINoPath[0], directory / "no_path1.ogg", true);
	LoadSoundfile (VOINoPath[1], directory / "no_path2.ogg", true);
	LoadSoundfile (VOINoSpeed, directory / "no_speed.ogg", true);
	LoadSoundfile (VOIOK[0], directory / "ok1.ogg", true);
	LoadSoundfile (VOIOK[1], directory / "ok2.ogg", true);
	LoadSoundfile (VOIOK[2], directory / "ok3.ogg", true);
	LoadSoundfile (VOIOK[3], directory / "ok4.ogg", true);
	LoadSoundfile (VOIReammo, directory / "reammo.ogg", true);
	LoadSoundfile (VOIReammoAll, directory / "reammo_all.ogg", true);
	LoadSoundfile (VOIRepaired[0], directory / "repaired.ogg", true);
	LoadSoundfile (VOIRepaired[1], directory / "repaired2.ogg", true);
	LoadSoundfile (VOIRepairedAll[0], directory / "repaired_all1.ogg", true);
	LoadSoundfile (VOIRepairedAll[1], directory / "repaired_all2.ogg", true);
	LoadSoundfile (VOIResearchComplete, directory / "research_complete.ogg", true);
	LoadSoundfile (VOISaved, directory / "saved.ogg", true);
	LoadSoundfile (VOISentry, directory / "sentry.ogg", true);
	LoadSoundfile (VOIStartMore, directory / "start_more.ogg", true);
	LoadSoundfile (VOIStartNone, directory / "start_none.ogg", true);
	LoadSoundfile (VOIStartOne, directory / "start_one.ogg", true);
	LoadSoundfile (VOIStatusRed[0], directory / "status_red1.ogg", true);
	LoadSoundfile (VOIStatusRed[1], directory / "status_red2.ogg", true);
	LoadSoundfile (VOIStatusYellow[0], directory / "status_yellow1.ogg", true);
	LoadSoundfile (VOIStatusYellow[1], directory / "status_yellow2.ogg", true);
	LoadSoundfile (VOISubDetected, directory / "sub_detected.ogg", true);
	LoadSoundfile (VOISurveying[0], directory / "surveying.ogg", true);
	LoadSoundfile (VOISurveying[1], directory / "surveying2.ogg", true);
	LoadSoundfile (VOITransferDone, directory / "transfer_done.ogg", true);
	LoadSoundfile (VOITurnEnd20Sec[0], directory / "turn_end_20_sec1.ogg", true);
	LoadSoundfile (VOITurnEnd20Sec[1], directory / "turn_end_20_sec2.ogg", true);
	LoadSoundfile (VOITurnEnd20Sec[2], directory / "turn_end_20_sec3.ogg", true);
	LoadSoundfile (VOIUnitDisabled, directory / "unit_disabled.ogg", true);
	LoadSoundfile (VOIUnitDisabledByEnemy[0], directory / "unit_disabled_by_enemy1.ogg", true);
	LoadSoundfile (VOIUnitDisabledByEnemy[1], directory / "unit_disabled_by_enemy2.ogg", true);
	LoadSoundfile (VOIUnitStolen[0], directory / "unit_stolen1.ogg", true);
	LoadSoundfile (VOIUnitStolen[1], directory / "unit_stolen2.ogg", true);
	LoadSoundfile (VOIUnitStolenByEnemy, directory / "unit_stolen_by_enemy.ogg", true);
}

//------------------------------------------------------------------------------
void cOtherData::loadWayPoints()
{
	for (int i = 0; i < 60; i++)
	{
		WayPointPfeile[0][i] = CreatePfeil (26, 11, 51, 36, 14, 48, PFEIL_COLOR, 64 - i);
		WayPointPfeile[1][i] = CreatePfeil (14, 14, 49, 14, 31, 49, PFEIL_COLOR, 64 - i);
		WayPointPfeile[2][i] = CreatePfeil (37, 11, 12, 36, 49, 48, PFEIL_COLOR, 64 - i);
		WayPointPfeile[3][i] = CreatePfeil (49, 14, 49, 49, 14, 31, PFEIL_COLOR, 64 - i);
		WayPointPfeile[4][i] = CreatePfeil (14, 14, 14, 49, 49, 31, PFEIL_COLOR, 64 - i);
		WayPointPfeile[5][i] = CreatePfeil (15, 14, 52, 26, 27, 51, PFEIL_COLOR, 64 - i);
		WayPointPfeile[6][i] = CreatePfeil (31, 14, 14, 49, 49, 49, PFEIL_COLOR, 64 - i);
		WayPointPfeile[7][i] = CreatePfeil (48, 14, 36, 51, 11, 26, PFEIL_COLOR, 64 - i);

		WayPointPfeileSpecial[0][i] = CreatePfeil (26, 11, 51, 36, 14, 48, PFEILS_COLOR, 64 - i);
		WayPointPfeileSpecial[1][i] = CreatePfeil (14, 14, 49, 14, 31, 49, PFEILS_COLOR, 64 - i);
		WayPointPfeileSpecial[2][i] = CreatePfeil (37, 11, 12, 36, 49, 48, PFEILS_COLOR, 64 - i);
		WayPointPfeileSpecial[3][i] = CreatePfeil (49, 14, 49, 49, 14, 31, PFEILS_COLOR, 64 - i);
		WayPointPfeileSpecial[4][i] = CreatePfeil (14, 14, 14, 49, 49, 31, PFEILS_COLOR, 64 - i);
		WayPointPfeileSpecial[5][i] = CreatePfeil (15, 14, 52, 26, 27, 51, PFEILS_COLOR, 64 - i);
		WayPointPfeileSpecial[6][i] = CreatePfeil (31, 14, 14, 49, 49, 49, PFEILS_COLOR, 64 - i);
		WayPointPfeileSpecial[7][i] = CreatePfeil (48, 14, 36, 51, 11, 26, PFEILS_COLOR, 64 - i);
	}
}

//------------------------------------------------------------------------------
void cResourceData::load (const std::filesystem::path& directory)
{
	// metal
	if (LoadGraphicToSurface (res_metal_org, directory / "res.pcx") == 1)
	{
		res_metal = CloneSDLSurface (*res_metal_org);
		SDL_SetColorKey (res_metal.get(), SDL_TRUE, 0xFF00FF);
	}

	// gold
	if (LoadGraphicToSurface (res_gold_org, directory / "gold.pcx") == 1)
	{
		res_gold = CloneSDLSurface (*res_gold_org);
		SDL_SetColorKey (res_gold.get(), SDL_TRUE, 0xFF00FF);
	}

	// fuel
	if (LoadGraphicToSurface (res_oil_org, directory / "fuel.pcx") == 1)
	{
		res_oil = CloneSDLSurface (*res_oil_org);
		SDL_SetColorKey (res_oil.get(), SDL_TRUE, 0xFF00FF);
	}
}
