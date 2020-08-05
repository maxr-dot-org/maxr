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
// Main-declerations, -classes and structures for the game
// Contains all global varaibles needed for the game
//
///////////////////////////////////////////////////////////////////////////////

// Hides some warnings from the eye of VS users ///////////////////////////////
#ifdef _MSC_VER
#pragma warning(disable:4996)
#endif

#ifndef mainH
#define mainH

// Includes ///////////////////////////////////////////////////////////////////

#include <vector>
#include <SDL.h>
#include "utility/autosurface.h"
#include "defines.h"
#include "utility/language.h"
#include "game/data/units/unitdata.h"
#include "game/data/player/clans.h"

// Predeclarations
class cPlayer;
class cLanguage;
struct sBuildingUIData;
struct sVehicleUIData;
class cClanData;

///////////////////////////////////////////////////////////////////////////////
// Structures
// ------------------------
//
///////////////////////////////////////////////////////////////////////////////

enum class ePlayerConnectionState
{
	INACTIVE,       // player is not connected, but game can continue (e. g. defeated player)
	CONNECTED,      // player is connected. Normal operation.
	NOT_RESPONDING, // player is connected, but no sync message received for some time. Game should be paused.
	DISCONNECTED    // player has lost connection. Game should be paused.
};
std::string enumToString(ePlayerConnectionState value);

///////////////////////////////////////////////////////////////////////////////
// Predeclerations
// ------------------------
//
///////////////////////////////////////////////////////////////////////////////

/**
 * Return if it is the main thread.
 * @note: should be called by main once by the main thread to initialize.
 */
bool is_main_thread();

#endif
