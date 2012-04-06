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

#include "unit.h"
#include "math.h"
#include "client.h"
#include "video.h"
#include "mouse.h"
#include "dialog.h"
#include "player.h"
#include "attackJobs.h"
#include "menus.h"
#include "clientevents.h"

#include "vehicles.h"
#include "buildings.h"

using namespace std;

//-----------------------------------------------------------------------------
cUnit::cUnit (UnitType unitType, sUnitData* unitData, cPlayer* owner)
: iID (0)
, PosX (0)
, PosY (0)
, dir (0)
, turnsDisabled (0)
, sentryActive (false)
, manualFireActive (false)
, attacking (false)
, isBeeingAttacked (false)
, next (0)
, prev (0)
, selectedMenuButtonIndex (-1)
, owner (owner)
, unitType (unitType)
, isOriginalName (true)
, isMarkedAsDone (false)
, hasBeenAttacked (false)
{
	if (unitData != 0)
		data = *unitData;
	
	sentryActive = (isBuilding () && data.canAttack != TERRAIN_NONE);
}

//-----------------------------------------------------------------------------
cUnit::~cUnit ()
{
	deleteStoredUnits ();
}

//--------------------------------------------------------------------------
/** returns the remaining hitpoints after an attack */
//--------------------------------------------------------------------------
int cUnit::calcHealth (int damage) const
{
	damage -= data.armor;
	
	if (damage <= 0)
	{
		//minimum damage is 1
		damage = 1;
	}
	
	int hp;
	hp = data.hitpointsCur - damage;
	
	if (hp < 0)
		return 0;
	
	return hp;
}

//--------------------------------------------------------------------------
/** Checks if the target is in range */
//--------------------------------------------------------------------------
bool cUnit::isInRange (int x, int y) const
{
	x -= PosX;
	y -= PosY;
	
	return (sqrt ((double) (x*x + y*y)) <= data.range);
}

//-----------------------------------------------------------------------------
bool cUnit::isNextTo (int x, int y) const
{
	if (x + 1 < PosX || y + 1 < PosY)
		return false;
	
	if (data.isBig)
	{
		if (x - 2 > PosX || y - 2 > PosY) 
			return false;
	}
	else 
	{
		if (x - 1 > PosX || y - 1 > PosY)
			return false;
	}
	
	return true;
}

//--------------------------------------------------------------------------
/** generates the name for the unit depending on the versionnumber */
//--------------------------------------------------------------------------
string cUnit::getNamePrefix () const
{
	int tmp;
	string rome = "";
	int nr = data.version + 1;	// +1, because the numbers in the name start at 1, not at 0
	
	// generate the roman versionnumber (correct until 899)
	
	if ( nr > 100 )
	{
		tmp = nr / 100;
		nr %= 100;
		
		while ( tmp-- )
			rome += "C";
	}
	
	if ( nr >= 90 )
	{
		rome += "XC";
		nr -= 90;
	}
	
	if ( nr >= 50 )
	{
		nr -= 50;
		rome += "L";
	}
	
	if ( nr >= 40 )
	{
		nr -= 40;
		rome += "XL";
	}
	
	if ( nr >= 10 )
	{
		tmp = nr / 10;
		nr %= 10;
		
		while ( tmp-- )
			rome += "X";
	}
	
	if ( nr == 9 )
	{
		nr -= 9;
		rome += "IX";
	}
	
	if ( nr >= 5 )
	{
		nr -= 5;
		rome += "V";
	}
	
	if ( nr == 4 )
	{
		nr -= 4;
		rome += "IV";
	}
	
	// alzi:
	// We had a bug, when 'nr' was negative and the following loop never terminated.
	// I modified the loop to terminate on a negative 'nr', but since this should never be the case,
	// the error has to be occured somewhere before and I added this warning.
	if ( nr < 0 )
	{
		Log.write( "cUnit: Negative 'nr' in cUnit::getNamePrefix()", cLog::eLOG_TYPE_WARNING );
	}
	while ( nr-- > 0 )
	{
		rome += "I";
	}
	
	return "MK " + rome;
}

//-----------------------------------------------------------------------------
/** Returns the name of the vehicle how it should be displayed */
//-----------------------------------------------------------------------------
string cUnit::getDisplayName () const
{
	return getNamePrefix () + " " + (isNameOriginal () ? data.name : name);
}

//-----------------------------------------------------------------------------
/** changes the name of the unit and indicates it as "not default" */
//-----------------------------------------------------------------------------
void cUnit::changeName (string newName)
{
	name = newName;
	isOriginalName = false;
}

//-----------------------------------------------------------------------------
/** Returns the size of the menu and the position */
//-----------------------------------------------------------------------------
SDL_Rect cUnit::getMenuSize () const
{
	SDL_Rect dest;
	int i, size;
	dest.x = getScreenPosX ();
	dest.y = getScreenPosY ();
	dest.h = i = getNumberOfMenuEntries () * 22;
	dest.w = 42;
	size = Client->gameGUI.getTileSize ();
	
	if (data.isBig)
		size *= 2;
	
	if (dest.x + size + 42 >= Video.getResolutionX () - 12)
		dest.x -= 42;
	else
		dest.x += size;
	
	if (dest.y - (i - size) / 2 <= 24)
	{
		dest.y -= (i - size) / 2;
		dest.y += - (dest.y - 24);
	}
	else if (dest.y - (i - size) / 2 + i >= Video.getResolutionY () - 24)
	{
		dest.y -= (i - size) / 2;
		dest.y -= (dest.y + i) - (Video.getResolutionY () - 24);
	}
	else
		dest.y -= (i - size) / 2;
	
	return dest;
}

//-----------------------------------------------------------------------------
/** Returns true, if the coordinates are in the menu's space */
//-----------------------------------------------------------------------------
bool cUnit::areCoordsOverMenu (int x, int y)
{
	SDL_Rect r = getMenuSize();
	
	if (x < r.x || x > r.x + r.w)
		return false;
	
	if (y < r.y || y > r.y + r.h)
		return false;
	
	return true;
}

//--------------------------------------------------------------------------
void cUnit::setMenuSelection ()
{
	SDL_Rect dest = getMenuSize ();
	selectedMenuButtonIndex = (mouse->y - dest.y) / 22;
}

//--------------------------------------------------------------------------
int cUnit::getNumberOfMenuEntries () const
{
	int result = 2; // Info/Help + Done

	if (owner != Client->ActivePlayer)
		return result;
	
	// Attack
	if (data.canAttack && data.shotsCur)
		result++;

	// Build
	if (data.canBuild.empty () == false && isUnitBuildingABuilding () == false)
		result++;

	// Distribute
	if (data.canMineMaxRes > 0 && isUnitWorking ())
		result++;

	// Transfer
	if (data.storeResType != sUnitData::STORE_RES_NONE && isUnitBuildingABuilding () == false && isUnitClearing () == false)
		result++;
	
	// Start
	if (data.canWork && buildingCanBeStarted ())
		result++;
	
	// Auto survey
	if (data.canSurvey)
		result++;
	
	// Stop
	if (canBeStoppedViaUnitMenu ())
		result++;

	// Remove
	if (data.canClearArea && Client->Map->fields[PosX + PosY*Client->Map->size].getRubble () && isUnitClearing () == false)
		result++;
	
	// Manual Fire
	if (manualFireActive || data.canAttack)
		result++;

	// Sentry
	if ( (sentryActive || data.canAttack || (!isBuilding () && !canBeStoppedViaUnitMenu () )) && owner == Client->ActivePlayer )
		result++;

	// Activate / Load
	if (data.storageUnitsMax > 0)
		result += 2;

	// Research
	if (data.canResearch && isUnitWorking ())
		result++;

	// Gold upgrades screen
	if (data.convertsGold)
		result++;

	// Update building(s)
	if (buildingCanBeUpgraded ())
		result += 2;
	
	// Self destruct
	if (data.canSelfDestroy)
		result++;

	// Ammo
	if (data.canRearm && data.storageResCur >= 2)
		result++;
	
	// Repair
	if (data.canRepair && data.storageResCur >= 2)
		result++;
	
	// Lay Mines
	if (data.canPlaceMines && data.storageResCur > 0)
		result++;
	
	// Clear Mines
	if (data.canPlaceMines && data.storageResCur < data.storageResMax)
		result++;
	
	// Sabotage/disable
	if (data.canCapture && data.shotsCur)
		result++;
	
	// Steal
	if (data.canDisable && data.shotsCur)
		result++;
	
	return result;
	
}

//--------------------------------------------------------------------------
void cUnit::drawMenu ()
{
	int nr = 0;
	SDL_Rect dest = getMenuSize ();
	
	if (isBeeingAttacked)
		return;
	if (isUnitMoving ())
		return;
	
	if (Client->gameGUI.mouseInputMode == activateVehicle)
	{
		Client->gameGUI.unitMenuActive = false;
		return;
	}
	
	if (factoryHasJustFinishedBuilding ())
		return;
	
	bool markerPossible = (areCoordsOverMenu (mouse->x, mouse->y) && (selectedMenuButtonIndex == (mouse->y - dest.y) / 22));
	
	// Attack:
	if (data.canAttack && data.shotsCur && owner == Client->ActivePlayer )
	{
		bool isMarked = (markerPossible && selectedMenuButtonIndex == nr) || Client->gameGUI.mouseInputMode == attackMode;
		drawContextItem (lngPack.i18n ("Text~Context~Attack"), isMarked, dest.x, dest.y, buffer);
		dest.y += 22;
		nr++;
	}

	// Build:
	if (data.canBuild.empty () == false && isUnitBuildingABuilding () == false && owner == Client->ActivePlayer)
	{
		bool isMarked = markerPossible && selectedMenuButtonIndex == nr;		
		drawContextItem (lngPack.i18n ("Text~Context~Build"), isMarked, dest.x, dest.y, buffer);
		dest.y += 22;
		nr++;
	}
	
	// Distribute:
	if (data.canMineMaxRes > 0 && isUnitWorking () && owner == Client->ActivePlayer)
	{
		bool isMarked = markerPossible && selectedMenuButtonIndex == nr;
		drawContextItem (lngPack.i18n ("Text~Context~Dist"), isMarked, dest.x, dest.y, buffer);
		dest.y += 22;
		nr++;
	}

	// Transfer:
	if (data.storeResType != sUnitData::STORE_RES_NONE && isUnitBuildingABuilding () == false && isUnitClearing () == false && owner == Client->ActivePlayer)
	{
		bool isMarked = (markerPossible && selectedMenuButtonIndex == nr) || Client->gameGUI.mouseInputMode == transferMode;
		drawContextItem (lngPack.i18n ("Text~Context~Transfer"), isMarked, dest.x, dest.y, buffer);
		dest.y += 22;
		nr++;
	}

	// Start:
	if (data.canWork && buildingCanBeStarted () && owner == Client->ActivePlayer)
	{
		bool isMarked = markerPossible && selectedMenuButtonIndex == nr;
		drawContextItem (lngPack.i18n ("Text~Context~Start"), isMarked, dest.x, dest.y, buffer);
		dest.y += 22;
		nr++;
	}
	
	// Auto survey movejob of surveyor
	if (data.canSurvey && owner == Client->ActivePlayer)
	{
		bool isMarked = (markerPossible && selectedMenuButtonIndex == nr) || isAutoMoveJobActive ();
		drawContextItem (lngPack.i18n ("Text~Context~Auto"), isMarked, dest.x, dest.y, buffer);
		dest.y += 22;
		nr++;
	}
	
	// Stop:
	if (canBeStoppedViaUnitMenu () && owner == Client->ActivePlayer)
	{
		bool isMarked = markerPossible && selectedMenuButtonIndex == nr;
		drawContextItem (lngPack.i18n ("Text~Context~Stop"), isMarked, dest.x, dest.y, buffer);
		dest.y += 22;
		nr++;
	}
	
	// Remove:
	if (data.canClearArea && Client->Map->fields[PosX + PosY*Client->Map->size].getRubble () && isUnitClearing () == false && owner == Client->ActivePlayer)
	{
		bool isMarked = markerPossible && selectedMenuButtonIndex == nr;		
		drawContextItem (lngPack.i18n ("Text~Context~Clear"), isMarked, dest.x, dest.y, buffer);
		dest.y += 22;
		nr++;
	}	
	
	// Manual fire
	if ((manualFireActive || data.canAttack) && owner == Client->ActivePlayer)
	{
		bool isMarked = (markerPossible && selectedMenuButtonIndex == nr) || manualFireActive;
		drawContextItem (lngPack.i18n ("Text~Context~Manual"), isMarked, dest.x, dest.y, buffer);
		dest.y += 22;
		nr++;
	}	
	
	// Sentry status:
	if ( (sentryActive || data.canAttack || (!isBuilding () && !canBeStoppedViaUnitMenu () )) && owner == Client->ActivePlayer )
	{
		bool isMarked = (markerPossible && selectedMenuButtonIndex == nr) || sentryActive;
		drawContextItem (lngPack.i18n ("Text~Context~Sentry"), isMarked, dest.x, dest.y, buffer);
		dest.y += 22;
		nr++;
	}
	
	// Activate / Load:
	if (data.storageUnitsMax > 0 && owner == Client->ActivePlayer)
	{
		// Activate:
		bool isMarked = markerPossible && selectedMenuButtonIndex == nr;
		drawContextItem (lngPack.i18n ("Text~Context~Active"), isMarked, dest.x, dest.y, buffer);
		dest.y += 22;
		nr++;
		
		// Load:
		isMarked = (markerPossible && selectedMenuButtonIndex == nr) || Client->gameGUI.mouseInputMode == loadMode;
		drawContextItem (lngPack.i18n ("Text~Context~Load"), isMarked, dest.x, dest.y, buffer);
		dest.y += 22;
		nr++;
	}
	
	// research
	if (data.canResearch && isUnitWorking () && owner == Client->ActivePlayer)
	{
		bool isMarked = markerPossible && selectedMenuButtonIndex == nr;
		drawContextItem (lngPack.i18n ("Text~Context~Research"), isMarked, dest.x, dest.y, buffer);
		dest.y += 22;
		nr++;
	}
	
	// gold upgrades screen
	if (data.convertsGold && owner == Client->ActivePlayer)
	{
		bool isMarked = markerPossible && selectedMenuButtonIndex == nr;
		drawContextItem (lngPack.i18n ("Text~Context~Upgrades"), isMarked, dest.x, dest.y, buffer);
		dest.y += 22;
		nr++;
	}
	
	// Updates:
	if (buildingCanBeUpgraded () && owner == Client->ActivePlayer)
	{
		// Update all buildings of this type in this subbase
		bool isMarked = markerPossible && selectedMenuButtonIndex == nr;
		drawContextItem (lngPack.i18n ("Text~Context~UpAll"), isMarked, dest.x, dest.y, buffer);
		dest.y += 22;
		nr++;
		
		// update this building
		isMarked = markerPossible && selectedMenuButtonIndex == nr;
		drawContextItem (lngPack.i18n ("Text~Context~Upgrade"), isMarked, dest.x, dest.y, buffer);
		dest.y += 22;
		nr++;		
	}
	
	// Self destruct
	if (data.canSelfDestroy && owner == Client->ActivePlayer)
	{
		bool isMarked = markerPossible && selectedMenuButtonIndex == nr;
		drawContextItem (lngPack.i18n ("Text~Context~Destroy"), isMarked, dest.x, dest.y, buffer);
		dest.y += 22;
		nr++;
	}
	
	// Ammo:
	if (data.canRearm && data.storageResCur >= 2 && owner == Client->ActivePlayer)
	{
		bool isMarked = (markerPossible && selectedMenuButtonIndex == nr) || Client->gameGUI.mouseInputMode == muniActive;
		drawContextItem (lngPack.i18n ("Text~Context~Reload"), isMarked, dest.x, dest.y, buffer);
		dest.y += 22;
		nr++;
	}	
	
	// Repair:
	if (data.canRepair && data.storageResCur >= 2 && owner == Client->ActivePlayer)
	{
		bool isMarked = (markerPossible && selectedMenuButtonIndex == nr) || Client->gameGUI.mouseInputMode == repairActive;		
		drawContextItem (lngPack.i18n ("Text~Context~Repair"), isMarked, dest.x, dest.y, buffer);
		dest.y += 22;
		nr++;
	}	
	
	// Lay mines:
	if (data.canPlaceMines && data.storageResCur > 0 && owner == Client->ActivePlayer)
	{
		bool isMarked = (markerPossible && selectedMenuButtonIndex == nr) || isUnitLayingMines ();
		drawContextItem (lngPack.i18n ("Text~Context~Seed"), isMarked, dest.x, dest.y, buffer);
		dest.y += 22;
		nr++;
	}
	
	// Collect/clear mines:
	if (data.canPlaceMines && data.storageResCur < data.storageResMax && owner == Client->ActivePlayer)
	{
		bool isMarked = (markerPossible && selectedMenuButtonIndex == nr) || isUnitClearingMines ();
		drawContextItem (lngPack.i18n ("Text~Context~Clear"), isMarked, dest.x, dest.y, buffer);
		dest.y += 22;
		nr++;
	}
	
	// Sabotage/disable:
	if (data.canDisable && data.shotsCur && owner == Client->ActivePlayer)
	{
		bool isMarked = (markerPossible && selectedMenuButtonIndex == nr) || Client->gameGUI.mouseInputMode == disableMode;
		drawContextItem (lngPack.i18n ("Text~Context~Disable"), isMarked, dest.x, dest.y, buffer);
		dest.y += 22;
		nr++;
	}
	
	// Steal:
	if (data.canCapture && data.shotsCur && owner == Client->ActivePlayer)
	{
		bool isMarked = (markerPossible && selectedMenuButtonIndex == nr) || Client->gameGUI.mouseInputMode == stealMode;
		drawContextItem (lngPack.i18n ("Text~Context~Steal"), isMarked, dest.x, dest.y, buffer);
		dest.y += 22;
		nr++;
	}
	
	// Info:
	bool isMarked = markerPossible && selectedMenuButtonIndex == nr;
	drawContextItem (lngPack.i18n ("Text~Context~Info"), isMarked, dest.x, dest.y, buffer);
	dest.y += 22;
	nr++;
	
	// Done:
	isMarked = markerPossible && selectedMenuButtonIndex == nr;
	drawContextItem (lngPack.i18n ("Text~Context~Done"), isMarked, dest.x, dest.y, buffer);	
}

//--------------------------------------------------------------------------
void cUnit::menuReleased ()
{
	SDL_Rect dest = getMenuSize ();

	int exeNr = -1000;
	if (areCoordsOverMenu (mouse->x, mouse->y)) 
		exeNr = (mouse->y - dest.y) / 22;

	if (exeNr != selectedMenuButtonIndex)
	{
		selectedMenuButtonIndex = -1;
		return;
	}
	
	if (isUnitMoving () || isBeeingAttacked) 
		return;

	if (factoryHasJustFinishedBuilding ())
		return;

	int nr = 0;

	// attack:
	if (data.canAttack && data.shotsCur && owner == Client->ActivePlayer)
	{
		if (exeNr == nr)
		{
			Client->gameGUI.unitMenuActive = false;
			PlayFX (SoundData.SNDObjectMenu);
			Client->gameGUI.toggleMouseInputMode (attackMode);
			return;
		}
		nr++;
	}
	
	// Build:
	if (data.canBuild.empty () == false && isUnitBuildingABuilding () == false && owner == Client->ActivePlayer)
	{
		if (exeNr == nr)
		{
			Client->gameGUI.unitMenuActive = false;
			PlayFX (SoundData.SNDObjectMenu);
			executeBuildCommand ();
			return;
		}
		nr++;
	}
	
	// distribute:
	if (data.canMineMaxRes > 0 && isUnitWorking () && owner == Client->ActivePlayer)
	{
		if (exeNr == nr)
		{
			Client->gameGUI.unitMenuActive = false;
			PlayFX (SoundData.SNDObjectMenu);
			executeMineManagerCommand ();
			return;
		}
		nr++;
	}
	
	// transfer:
	if (data.storeResType != sUnitData::STORE_RES_NONE && isUnitBuildingABuilding () == false && isUnitClearing () == false && owner == Client->ActivePlayer)
	{
		if (exeNr == nr)
		{
			Client->gameGUI.unitMenuActive = false;
			PlayFX (SoundData.SNDObjectMenu);
			Client->gameGUI.toggleMouseInputMode (transferMode);
			return;
		}
		nr++;
	}
	
	// Start:
	if (data.canWork && buildingCanBeStarted () && owner == Client->ActivePlayer)
	{
		if (exeNr == nr)
		{
			Client->gameGUI.unitMenuActive = false;
			PlayFX (SoundData.SNDObjectMenu);
			sendWantStartWork (this);
			return;
		}
		nr++;
	}
	
	// auto
	if (data.canSurvey && owner == Client->ActivePlayer)
	{
		if (exeNr == nr)
		{
			Client->gameGUI.unitMenuActive = false;
			PlayFX (SoundData.SNDObjectMenu);
			executeAutoMoveJobCommand ();
			return;
		}
		nr++;
	}
	
	// stop:
	if (canBeStoppedViaUnitMenu () && owner == Client->ActivePlayer )
	{
		if (exeNr == nr)
		{
			Client->gameGUI.unitMenuActive = false;
			PlayFX (SoundData.SNDObjectMenu);
			executeStopCommand ();
			return;
		}
		nr++;
	}
	
	// remove:
	if (data.canClearArea && Client->Map->fields[PosX+PosY*Client->Map->size].getRubble () != 0 && isUnitClearing () == false && owner == Client->ActivePlayer)
	{
		if (exeNr == nr)
		{
			Client->gameGUI.unitMenuActive = false;
			PlayFX (SoundData.SNDObjectMenu);
			sendWantStartClear (this);
			return;
		}
		nr++;
	}
	
	// manual Fire:
	if ((manualFireActive || data.canAttack) && owner == Client->ActivePlayer)
	{
		if (exeNr == nr)
		{
			Client->gameGUI.unitMenuActive = false;
			PlayFX (SoundData.SNDObjectMenu);
			sendChangeManualFireStatus (iID, isVehicle ());
			return;
		}
		nr++;
	}

	// sentry:
	if ((sentryActive || data.canAttack || (!isBuilding () && !canBeStoppedViaUnitMenu () )) && owner == Client->ActivePlayer )
	{
		if (exeNr == nr)
		{
			Client->gameGUI.unitMenuActive = false;
			PlayFX (SoundData.SNDObjectMenu);
			sendChangeSentry (iID, isVehicle () );
			return;
		}
		nr++;
	}
	
	// activate/load:
	if (data.storageUnitsMax > 0 && owner == Client->ActivePlayer)
	{
		// activate:
		if (exeNr == nr)
		{
			Client->gameGUI.unitMenuActive = false;
			PlayFX (SoundData.SNDObjectMenu);
			executeActivateStoredVehiclesCommand ();
			return;
		}
		nr++;
		
		// load:
		if (exeNr == nr)
		{
			Client->gameGUI.unitMenuActive = false;
			PlayFX (SoundData.SNDObjectMenu);
			Client->gameGUI.toggleMouseInputMode (loadMode);
			return;
		}
		nr++;
	}
	
	// research
	if (data.canResearch && isUnitWorking () && owner == Client->ActivePlayer)
	{
		if (exeNr == nr)
		{
			Client->gameGUI.unitMenuActive = false;
			PlayFX (SoundData.SNDObjectMenu);
			cDialogResearch researchDialog (owner);
			researchDialog.show();
			return;
		}
		nr++;
	}
	
	// gold upgrades screen
	if (data.convertsGold && owner == Client->ActivePlayer)
	{
		if (exeNr == nr)
		{
			Client->gameGUI.unitMenuActive = false;
			PlayFX (SoundData.SNDObjectMenu);
			cUpgradeMenu upgradeMenu (owner);
			upgradeMenu.show ();
			return;
		}
		nr++;
	}
	
	// Updates:
	if (buildingCanBeUpgraded () && owner == Client->ActivePlayer )
	{
		// Update all buildings of this type in this subbase
		if (exeNr == nr)
		{
			Client->gameGUI.unitMenuActive = false;
			PlayFX (SoundData.SNDObjectMenu);
			executeUpdateBuildingCommmand (true);
			return;
		}
		nr++;
		
		// update this building
		if (exeNr == nr)
		{
			Client->gameGUI.unitMenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			executeUpdateBuildingCommmand (false);
			return;
		}
		nr++;
	}
	
	// Self destruct
	if (data.canSelfDestroy && owner == Client->ActivePlayer)
	{
		if (exeNr == nr)
		{
			Client->gameGUI.unitMenuActive = false;
			PlayFX (SoundData.SNDObjectMenu);
			executeSelfDestroyCommand ();
			return;
		}
		nr++;
	}
	
	// rearm:
	if (data.canRearm && data.storageResCur >= 2 && owner == Client->ActivePlayer)
	{
		if (exeNr == nr)
		{
			Client->gameGUI.unitMenuActive = false;
			PlayFX (SoundData.SNDObjectMenu);
			Client->gameGUI.toggleMouseInputMode (muniActive);
			return;
		}
		nr++;
	}
	
	// repair:
	if (data.canRepair && data.storageResCur >= 2 && owner == Client->ActivePlayer)
	{
		if (exeNr == nr)
		{
			Client->gameGUI.unitMenuActive = false;
			PlayFX (SoundData.SNDObjectMenu);
			Client->gameGUI.toggleMouseInputMode (repairActive);
			return;
		}
		nr++;
	}
	
	// lay mines:
	if (data.canPlaceMines && data.storageResCur > 0 && owner == Client->ActivePlayer)
	{
		if (exeNr == nr)
		{
			Client->gameGUI.unitMenuActive = false;
			PlayFX (SoundData.SNDObjectMenu);
			executeLayMinesCommand ();
			return;
		}
		nr++;
	}
	
	// clear mines:
	if (data.canPlaceMines && data.storageResCur < data.storageResMax && owner == Client->ActivePlayer)
	{
		if (exeNr == nr)
		{
			Client->gameGUI.unitMenuActive = false;
			PlayFX (SoundData.SNDObjectMenu);
			executeClearMinesCommand ();
			return;
		}
		nr++;
	}
	
	// disable:
	if (data.canDisable && data.shotsCur > 0 && owner == Client->ActivePlayer)
	{
		if (exeNr == nr)
		{
			Client->gameGUI.unitMenuActive = false;
			PlayFX (SoundData.SNDObjectMenu);
			Client->gameGUI.toggleMouseInputMode (disableMode);
			return;
		}
		nr++;
	}
	
	// steal:
	if (data.canCapture && data.shotsCur > 0 && owner == Client->ActivePlayer)
	{
		if (exeNr == nr)
		{
			Client->gameGUI.unitMenuActive = false;
			PlayFX (SoundData.SNDObjectMenu);
			Client->gameGUI.toggleMouseInputMode (stealMode);
			return;
		}
		nr++;
	}
	
	// help/info:
	if (exeNr == nr)
	{
		Client->gameGUI.unitMenuActive = false;
		PlayFX (SoundData.SNDObjectMenu);
		cUnitHelpMenu helpMenu (&data, owner);
		helpMenu.show ();
		return;
	}
	nr++;
	
	// done:
	if (exeNr == nr)
	{
		Client->gameGUI.unitMenuActive = false;
		PlayFX (SoundData.SNDObjectMenu);
		if (owner == Client->ActivePlayer)
		{
			isMarkedAsDone = true;
			sendMoveJobResume (iID);
		}
		return;
	}
}


//--------------------------------------------------------------------------
/** Returns the screen x position of the unit */
//--------------------------------------------------------------------------
int cUnit::getScreenPosX () const
{
	return 180 - ((int)((Client->gameGUI.getOffsetX () - getMovementOffsetX ()) * Client->gameGUI.getZoom ())) + (int)(Client->gameGUI.getTileSize ()) * PosX;
}

//--------------------------------------------------------------------------
/** Returns the screen y position of the unit */
//--------------------------------------------------------------------------
int cUnit::getScreenPosY () const
{
	return 18 - ((int)((Client->gameGUI.getOffsetY () - getMovementOffsetY ()) * Client->gameGUI.getZoom ())) + (int)(Client->gameGUI.getTileSize ()) * PosY;
}

//-----------------------------------------------------------------------------
/** Centers on this unit */
//-----------------------------------------------------------------------------
void cUnit::center () const
{
	int offX = PosX * 64 - ((int)( ((float)(Video.getResolutionX () - 192) / (2 * Client->gameGUI.getTileSize ())) * 64)) + 32;
	int offY = PosY * 64 - ((int)( ((float)(Video.getResolutionY () - 32)  / (2 * Client->gameGUI.getTileSize ())) * 64)) + 32;
	Client->gameGUI.setOffsetPosition (offX, offY);
}

//-----------------------------------------------------------------------------
/** Draws the ammunition bar over the unit */
//-----------------------------------------------------------------------------
void cUnit::drawMunBar () const
{
	if (owner != Client->ActivePlayer)
		return;
	
	SDL_Rect r1, r2;
	r1.x = getScreenPosX () + Client->gameGUI.getTileSize () / 10 + 1;
	r1.w = Client->gameGUI.getTileSize () * 8 / 10 ;
	r1.h = Client->gameGUI.getTileSize () / 8;
	r1.y = getScreenPosY () + Client->gameGUI.getTileSize () / 10 + Client->gameGUI.getTileSize () / 8;
	
	if (r1.h <= 2)
	{
		r1.y += 1;
		r1.h = 3;
	}
	
	r2.x = r1.x + 1;
	r2.y = r1.y + 1;
	r2.h = r1.h - 2;
	r2.w = (int)(((float)(r1.w - 2) / data.ammoMax) * data.ammoCur);
	
	SDL_FillRect (buffer, &r1, 0);
	
	if (data.ammoCur > data.ammoMax / 2)
		SDL_FillRect (buffer, &r2, 0x04AE04);
	else if (data.ammoCur > data.ammoMax / 4)
		SDL_FillRect (buffer, &r2, 0xDBDE00);
	else
		SDL_FillRect (buffer, &r2, 0xE60000);
}

//------------------------------------------------------------------------
/** draws the health bar over the unit */
//--------------------------------------------------------------------------
void cUnit::drawHealthBar () const
{
	SDL_Rect r1, r2;
	r1.x = getScreenPosX () + Client->gameGUI.getTileSize () / 10 + 1;
	r1.w = Client->gameGUI.getTileSize () * 8 / 10 ;
	r1.h = Client->gameGUI.getTileSize () / 8;
	r1.y = getScreenPosY () + Client->gameGUI.getTileSize () / 10;
	
	if (data.isBig)
	{
		r1.w += Client->gameGUI.getTileSize ();
		r1.h *= 2;
	}
	
	if (r1.h <= 2)
		r1.h = 3;
	
	r2.x = r1.x + 1;
	r2.y = r1.y + 1;
	r2.h = r1.h - 2;
	r2.w = (int)( ((float)(r1.w - 2) / data.hitpointsMax) * data.hitpointsCur);
	
	SDL_FillRect (buffer, &r1, 0);
	
	if (data.hitpointsCur > data.hitpointsMax / 2)
		SDL_FillRect (buffer, &r2, 0x04AE04);
	else if (data.hitpointsCur > data.hitpointsMax / 4)
		SDL_FillRect (buffer, &r2, 0xDBDE00);
	else
		SDL_FillRect (buffer, &r2, 0xE60000);
}

//-----------------------------------------------------------------------------
void cUnit::drawStatus () const
{
	SDL_Rect dest;
	SDL_Rect speedSymbol = {244, 97, 8, 10};
	SDL_Rect shotsSymbol = {254, 97, 5, 10};
	SDL_Rect disabledSymbol = {150, 109, 25, 25};
	
	if (turnsDisabled > 0)
	{
		if (Client->gameGUI.getTileSize () < 25) 
			return;
		dest.x = getScreenPosX () + Client->gameGUI.getTileSize () / 2 - 12;
		dest.y = getScreenPosY () + Client->gameGUI.getTileSize () / 2 - 12;
		SDL_BlitSurface (GraphicsData.gfx_hud_stuff, &disabledSymbol, buffer, &dest);
	}
	else
	{
		dest.y = getScreenPosY () + Client->gameGUI.getTileSize () - 11;
		dest.x = getScreenPosX () + Client->gameGUI.getTileSize () / 2 - 4;
		if (data.isBig)
		{
			dest.y += (Client->gameGUI.getTileSize () / 2);
			dest.x += (Client->gameGUI.getTileSize () / 2);
		}
		if (data.speedCur >= 4)
		{
			if (data.shotsCur) 
				dest.x -= Client->gameGUI.getTileSize () / 4;

			SDL_Rect destCopy = dest;
			SDL_BlitSurface (GraphicsData.gfx_hud_stuff, &speedSymbol, buffer, &destCopy);
		}
		
		dest.x = getScreenPosX () + Client->gameGUI.getTileSize () / 2 - 4;
		if (data.shotsCur)
		{
			if (data.speedCur)
				dest.x += Client->gameGUI.getTileSize () / 4;
			SDL_BlitSurface (GraphicsData.gfx_hud_stuff, &shotsSymbol, buffer, &dest);
		}
	}
}

//-----------------------------------------------------------------------------
/** rotates the unit to the given direction */
//-----------------------------------------------------------------------------
void cUnit::rotateTo (int newDir)
{
	if (newDir < 0 || newDir >= 8 || newDir == dir) 
		return;
		
	int t = dir;
	int dest;
	
	for (int i = 0; i < 8; i++)
	{
		if (t == newDir)
		{
			dest = i;
			break;
		}
		t++;
		
		if (t > 7)
			t = 0;
	}
	
	if (dest < 4)
		dir++;
	else
		dir--;
	
	if (dir < 0)
		dir += 8;
	else
	{
		if (dir > 7)
			dir -= 8;
	}
}



//-----------------------------------------------------------------------------
/** Checks, if the unit can attack an object at the given coordinates*/
//-----------------------------------------------------------------------------
bool cUnit::canAttackObjectAt (int x, int y, cMap* map, bool forceAttack, bool checkRange) const
{
	int off = x + y * map->size;
	
	if (isUnitLoaded ())
		return false;
	
	if (data.canAttack == false)
		return false;
	
	if (data.shotsCur <= 0)
		return false;
	
	if (data.ammoCur <= 0)
		return false;
	
	if (attacking)
		return false;
	
	if (isBeeingAttacked)
		return false;
	
	if (off < 0)
		return false;
	
	if (checkRange && isInRange (x, y) == false)
		return false;
	
	if (data.muzzleType == sUnitData::MUZZLE_TYPE_TORPEDO && map->isWater(x, y) == false)
		return false;
	
	cVehicle* targetVehicle = 0;
	cBuilding* targetBuilding = 0;	
	selectTarget (targetVehicle, targetBuilding, x, y, data.canAttack, map);
	
	if (targetVehicle && targetVehicle->iID == iID) //a unit cannot fire on it self
		return false;
	
	if (targetBuilding && targetBuilding->iID == iID) //a unit cannot fire on it self
		return false;

	if (owner->ScanMap[off] == false)
		return forceAttack ? true : false;
	
	if (forceAttack)
		return true;
	
	if (targetBuilding && isVehicle () && map->possiblePlace ((cVehicle*)this, x, y))  //do not fire on e. g. platforms, connectors etc.
		return false;																	//see ticket #436 on bug tracker
	
	if ((targetBuilding && targetBuilding->owner == owner) || (targetVehicle && targetVehicle->owner == owner))
		return false;
		
	if (targetBuilding == 0 && targetVehicle == 0)
		return false;
	
	return true;
}

//-----------------------------------------------------------------------------
void cUnit::upgradeToCurrentVersion ()
{
	sUnitData* upgradeVersion = getUpgradedUnitData ();
	if (upgradeVersion != 0)
	{
		data.version = upgradeVersion->version;
		
		if (data.hitpointsCur == data.hitpointsMax)
			data.hitpointsCur = upgradeVersion->hitpointsMax; // TODO: check behaviour in original
		data.hitpointsMax = upgradeVersion->hitpointsMax;
		
		data.ammoMax = upgradeVersion->ammoMax; // don't change the current ammo-amount!
		
		data.speedMax = upgradeVersion->speedMax;
		
		data.armor = upgradeVersion->armor;
		data.scan = upgradeVersion->scan;
		data.range = upgradeVersion->range;
		data.shotsMax = upgradeVersion->shotsMax; // TODO: check behaviour in original
		data.damage = upgradeVersion->damage;
		data.buildCosts = upgradeVersion->buildCosts;
	}
}

//-----------------------------------------------------------------------------
void cUnit::deleteStoredUnits ()
{
	while (storedUnits.Size ())
	{
		cUnit* unit = storedUnits[0];
		if (unit->prev)
		{
			cUnit* prevUnit;
			prevUnit = unit->prev;
			prevUnit->next = unit->next;
			
			if (unit->next)
				unit->next->prev = prevUnit;
		}
		else
		{
			unit->owner->VehicleList = (cVehicle*)unit->next;
			
			if (unit->next)
				unit->next->prev = 0;
		}
		if (unit->isVehicle ())
			unit->deleteStoredUnits ();
		
		delete unit;
		storedUnits.Delete (0);
	}
}
