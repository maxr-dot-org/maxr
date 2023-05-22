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
#ifndef ui_uidefinesH
#define ui_uidefinesH

#include "settings.h"

#define GRID_COLOR         0xFF305C04 // color of the grid
#define SCAN_COLOR         0xFFE3E300 // color of scan circles
#define RANGE_GROUND_COLOR 0xFFE20000 // color of range circles for ground attack
#define RANGE_AIR_COLOR    0xFFFCA800 // color of range circles for air attack

// minimap configuration
#define MINIMAP_COLOR 0xFFFC0000 // color of the screen borders on the minimap

// some defines for typical menus
// GFX On Demand /////////////////////////////////////////////////////////////
#define GFXOD(filename)        (cSettings::getInstance().getGfxPath() / filename)
#define GFXOD_MAIN             GFXOD ("main.pcx")
#define GFXOD_HELP             GFXOD ("help_screen.pcx")
#define GFXOD_OPTIONS          GFXOD ("options.pcx")
#define GFXOD_SAVELOAD         GFXOD ("load_save_menu.pcx")
#define GFXOD_PLANET_SELECT    GFXOD ("planet_select.pcx")
#define GFXOD_CLAN_SELECT      GFXOD ("clanselection.pcx")
#define GFXOD_HANGAR           GFXOD ("hangar.pcx")
#define GFXOD_BUILD_SCREEN     GFXOD ("build_screen.pcx")
#define GFXOD_FAC_BUILD_SCREEN GFXOD ("fac_build_screen.pcx")
#define GFXOD_PLAYER_SELECT    GFXOD ("customgame_menu.pcx") // hotseat 4 players
#define GFXOD_HOTSEAT          GFXOD ("hotseatplayers.pcx")  // hotseat 8 players
#define GFXOD_PLAYER_HUMAN     GFXOD ("player_human.pcx")
#define GFXOD_PLAYER_NONE      GFXOD ("player_none.pcx")
#define GFXOD_PLAYER_PC        GFXOD ("player_pc.pcx")
#define GFXOD_MULT             GFXOD ("multi.pcx")
#define GFXOD_UPGRADE          GFXOD ("upgrade.pcx")
#define GFXOD_STORAGE          GFXOD ("storage.pcx")
#define GFXOD_STORAGE_GROUND   GFXOD ("storage_ground.pcx")
#define GFXOD_MULT             GFXOD ("multi.pcx")
#define GFXOD_MINEMANAGER      GFXOD ("mine_manager.pcx")
#define GFXOD_REPORTS          GFXOD ("reports.pcx")
#define GFXOD_DIALOG2          GFXOD ("dialog2.pcx")
#define GFXOD_DIALOG4          GFXOD ("dialog4.pcx")
#define GFXOD_DIALOG5          GFXOD ("dialog5.pcx")
#define GFXOD_DIALOG6          GFXOD ("dialog6.pcx")
#define GFXOD_DIALOG_TRANSFER  GFXOD ("transfer.pcx")
#define GFXOD_DIALOG_RESEARCH  GFXOD ("research.pcx")
#define GFXOD_DESTRUCTION      GFXOD ("destruction.pcx")

// Other Resources /////////////////////////////////////////////////////////////
#define MAXR_ICON (cSettings::getInstance().getDataDir() / "maxr.bmp")
#define SPLASH_BACKGROUND (cSettings::getInstance().getDataDir() / "init.pcx")

#endif
