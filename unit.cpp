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

#include "attackJobs.h"
#include "client.h"
#include "clientevents.h"
#include "dialog.h"
#include "hud.h"
#include "menus.h"
#include "mouse.h"
#include "player.h"
#include "video.h"

#include "buildings.h"
#include "vehicles.h"

#include <cmath>

using namespace std;

//------------------------------------------------------------------------------
cUnit::cUnit (const sUnitData* unitData, cPlayer* owner, unsigned int ID)
	: iID (ID)
	, PosX (0)
	, PosY (0)
	, dir (0)
	, turnsDisabled (0)
	, sentryActive (false)
	, manualFireActive (false)
	, attacking (false)
	, isBeeingAttacked (false)
	, isMarkedAsDone (false)
	, hasBeenAttacked (false)
	, owner (owner)
	, job (NULL)
	, lockerPlayer (NULL)
	, isOriginalName (true)
{
	if (unitData != 0)
		data = *unitData;
}

//------------------------------------------------------------------------------
cUnit::~cUnit()
{
	deleteStoredUnits();
	if (lockerPlayer)
	{
		lockerPlayer->deleteLock (*this);
	}
}

//------------------------------------------------------------------------------
/** returns the remaining hitpoints after an attack */
//------------------------------------------------------------------------------
int cUnit::calcHealth (int damage) const
{
	damage -= data.armor;

	// minimum damage is 1
	damage = std::max (1, damage);

	const int hp = data.hitpointsCur - damage;
	return std::max (0, hp);
}

//------------------------------------------------------------------------------
/** Checks if the target is in range */
//------------------------------------------------------------------------------
bool cUnit::isInRange (int x, int y) const
{
	x -= PosX;
	y -= PosY;

	return (Square (x) + Square (y)) <= Square (data.range);
}

//------------------------------------------------------------------------------
bool cUnit::isNextTo (int x, int y) const
{
	if (x + 1 < PosX || y + 1 < PosY)
		return false;

	const int size = data.isBig ? 2 : 1;

	if (x - size > PosX || y - size > PosY)
		return false;
	return true;
}

// http://rosettacode.org/wiki/Roman_numerals/Encode#C.2B.2B
static std::string to_roman (unsigned int value)
{
	struct romandata_t { unsigned int value; char const* numeral; };
	const struct romandata_t romandata[] =
	{
		//{1000, "M"}, {900, "CM"},
		//{500, "D"}, {400, "CD"},
		{100, "C"}, { 90, "XC"},
		{ 50, "L"}, { 40, "XL"},
		{ 10, "X"}, { 9, "IX"},
		{ 5, "V"}, { 4, "IV"},
		{ 1, "I"},
		{ 0, NULL} // end marker
	};

	std::string result;
	for (const romandata_t* current = romandata; current->value > 0; ++current)
	{
		while (value >= current->value)
		{
			result += current->numeral;
			value -= current->value;
		}
	}
	return result;
}

//------------------------------------------------------------------------------
/** generates the name for the unit depending on the versionnumber */
//------------------------------------------------------------------------------
string cUnit::getNamePrefix() const
{
	string rome = "MK ";
	// +1, because the numbers in the name start at 1, not at 0
	unsigned int nr = data.version + 1;

	return "MK " + to_roman (nr);
}

//------------------------------------------------------------------------------
/** Returns the name of the vehicle how it should be displayed */
//------------------------------------------------------------------------------
string cUnit::getDisplayName() const
{
	return getNamePrefix() + " " + (isNameOriginal() ? data.name : name);
}

//------------------------------------------------------------------------------
/** changes the name of the unit and indicates it as "not default" */
//------------------------------------------------------------------------------
void cUnit::changeName (const string& newName)
{
	name = newName;
	isOriginalName = false;
}

//------------------------------------------------------------------------------
/** Returns the size of the menu and the position */
//------------------------------------------------------------------------------
SDL_Rect cUnit::getMenuSize (const cGameGUI& gameGUI) const
{
	SDL_Rect dest;
	dest.x = getScreenPosX (gameGUI);
	dest.y = getScreenPosY (gameGUI);
	dest.h = getNumberOfMenuEntries (*gameGUI.getClient()) * 22;
	dest.w = 42;
	const int i = dest.h;
	int size = gameGUI.getTileSize();

	if (data.isBig)
		size *= 2;

	if (dest.x + size + 42 >= Video.getResolutionX() - 12)
		dest.x -= 42;
	else
		dest.x += size;

	if (dest.y - (i - size) / 2 <= 24)
	{
		dest.y -= (i - size) / 2;
		dest.y += - (dest.y - 24);
	}
	else if (dest.y - (i - size) / 2 + i >= Video.getResolutionY() - 24)
	{
		dest.y -= (i - size) / 2;
		dest.y -= (dest.y + i) - (Video.getResolutionY() - 24);
	}
	else
		dest.y -= (i - size) / 2;

	return dest;
}

//------------------------------------------------------------------------------
/** Returns true, if the coordinates are in the menu's space */
//------------------------------------------------------------------------------
bool cUnit::areCoordsOverMenu (const cGameGUI& gameGUI, int x, int y) const
{
	const SDL_Rect r = getMenuSize (gameGUI);

	if (x < r.x || x > r.x + r.w)
		return false;
	if (y < r.y || y > r.y + r.h)
		return false;
	return true;
}

//------------------------------------------------------------------------------
void cUnit::setMenuSelection (cGameGUI& gameGUI) const
{
	SDL_Rect dest = getMenuSize (gameGUI);
	gameGUI.selectedMenuButtonIndex = (mouse->y - dest.y) / 22;
}

//------------------------------------------------------------------------------
int cUnit::getNumberOfMenuEntries (const cClient& client) const
{
	int result = 2; // Info/Help + Done

	if (owner != client.getActivePlayer())
		return result;
	if (isDisabled()) return result;

	// Attack
	if (data.canAttack && data.shotsCur)
		++result;

	// Build
	if (data.canBuild.empty() == false && isUnitBuildingABuilding() == false)
		++result;

	// Distribute
	if (data.canMineMaxRes > 0 && isUnitWorking())
		++result;

	// Transfer
	if (data.storeResType != sUnitData::STORE_RES_NONE && isUnitBuildingABuilding() == false && isUnitClearing() == false)
		++result;

	// Start
	if (data.canWork && buildingCanBeStarted())
		++result;

	// Auto survey
	if (data.canSurvey)
		++result;

	// Stop
	if (canBeStoppedViaUnitMenu())
		++result;

	// Remove
	if (data.canClearArea && client.getMap()->fields[client.getMap()->getOffset (PosX, PosY)].getRubble() && isUnitClearing() == false)
		++result;

	// Manual Fire
	if (manualFireActive || data.canAttack)
		++result;

	// Sentry
	if ( (sentryActive || data.canAttack || (!isABuilding() && !canBeStoppedViaUnitMenu())) && owner == client.getActivePlayer())
		++result;

	// Activate / Load
	if (data.storageUnitsMax > 0)
		result += 2;

	// Research
	if (data.canResearch && isUnitWorking())
		++result;

	// Gold upgrades screen
	if (data.convertsGold)
		++result;

	// Update building(s)
	if (buildingCanBeUpgraded())
		result += 2;

	// Self destruct
	if (data.canSelfDestroy)
		++result;

	// Ammo
	if (data.canRearm && data.storageResCur >= 1)
		++result;

	// Repair
	if (data.canRepair && data.storageResCur >= 1)
		++result;

	// Lay Mines
	if (data.canPlaceMines && data.storageResCur > 0)
		++result;

	// Clear Mines
	if (data.canPlaceMines && data.storageResCur < data.storageResMax)
		++result;

	// Sabotage/disable
	if (data.canCapture && data.shotsCur)
		++result;

	// Steal
	if (data.canDisable && data.shotsCur)
		++result;

	return result;
}

//------------------------------------------------------------------------------
void cUnit::drawMenu (cGameGUI& gameGUI) const
{
	if (isBeeingAttacked)
		return;
	if (isUnitMoving())
		return;

	if (gameGUI.mouseInputMode == activateVehicle)
	{
		gameGUI.unitMenuActive = false;
		return;
	}

	if (factoryHasJustFinishedBuilding())
		return;

	SDL_Rect dest = getMenuSize (gameGUI);
	bool markerPossible = (areCoordsOverMenu (gameGUI, mouse->x, mouse->y) && (gameGUI.selectedMenuButtonIndex == (mouse->y - dest.y) / 22));
	const cClient& client = *gameGUI.getClient();
	int nr = 0;

	if (isDisabled() == false)
	{
		// Attack:
		if (data.canAttack && data.shotsCur && owner == client.getActivePlayer())
		{
			bool isMarked = (markerPossible && gameGUI.selectedMenuButtonIndex == nr) || gameGUI.mouseInputMode == mouseInputAttackMode;
			drawContextItem (lngPack.i18n ("Text~Context~Attack"), isMarked, dest.x, dest.y, buffer);
			dest.y += 22;
			++nr;
		}

		// Build:
		if (data.canBuild.empty() == false && isUnitBuildingABuilding() == false && owner == client.getActivePlayer())
		{
			bool isMarked = markerPossible && gameGUI.selectedMenuButtonIndex == nr;
			drawContextItem (lngPack.i18n ("Text~Context~Build"), isMarked, dest.x, dest.y, buffer);
			dest.y += 22;
			++nr;
		}

		// Distribute:
		if (data.canMineMaxRes > 0 && isUnitWorking() && owner == client.getActivePlayer())
		{
			bool isMarked = markerPossible && gameGUI.selectedMenuButtonIndex == nr;
			drawContextItem (lngPack.i18n ("Text~Context~Dist"), isMarked, dest.x, dest.y, buffer);
			dest.y += 22;
			++nr;
		}

		// Transfer:
		if (data.storeResType != sUnitData::STORE_RES_NONE && isUnitBuildingABuilding() == false && isUnitClearing() == false && owner == client.getActivePlayer())
		{
			bool isMarked = (markerPossible && gameGUI.selectedMenuButtonIndex == nr) || gameGUI.mouseInputMode == transferMode;
			drawContextItem (lngPack.i18n ("Text~Context~Transfer"), isMarked, dest.x, dest.y, buffer);
			dest.y += 22;
			++nr;
		}

		// Start:
		if (data.canWork && buildingCanBeStarted() && owner == client.getActivePlayer())
		{
			bool isMarked = markerPossible && gameGUI.selectedMenuButtonIndex == nr;
			drawContextItem (lngPack.i18n ("Text~Context~Start"), isMarked, dest.x, dest.y, buffer);
			dest.y += 22;
			++nr;
		}

		// Auto survey movejob of surveyor
		if (data.canSurvey && owner == client.getActivePlayer())
		{
			bool isMarked = (markerPossible && gameGUI.selectedMenuButtonIndex == nr) || isAutoMoveJobActive();
			drawContextItem (lngPack.i18n ("Text~Context~Auto"), isMarked, dest.x, dest.y, buffer);
			dest.y += 22;
			++nr;
		}

		// Stop:
		if (canBeStoppedViaUnitMenu() && owner == client.getActivePlayer())
		{
			bool isMarked = markerPossible && gameGUI.selectedMenuButtonIndex == nr;
			drawContextItem (lngPack.i18n ("Text~Context~Stop"), isMarked, dest.x, dest.y, buffer);
			dest.y += 22;
			++nr;
		}

		// Remove:
		if (data.canClearArea && client.getMap()->fields[client.getMap()->getOffset (PosX, PosY)].getRubble() && isUnitClearing() == false && owner == client.getActivePlayer())
		{
			bool isMarked = markerPossible && gameGUI.selectedMenuButtonIndex == nr;
			drawContextItem (lngPack.i18n ("Text~Context~Clear"), isMarked, dest.x, dest.y, buffer);
			dest.y += 22;
			++nr;
		}

		// Manual fire
		if ( (manualFireActive || data.canAttack) && owner == client.getActivePlayer())
		{
			bool isMarked = (markerPossible && gameGUI.selectedMenuButtonIndex == nr) || manualFireActive;
			drawContextItem (lngPack.i18n ("Text~Context~Manual"), isMarked, dest.x, dest.y, buffer);
			dest.y += 22;
			++nr;
		}

		// Sentry status:
		if ( (sentryActive || data.canAttack || (!isABuilding() && !canBeStoppedViaUnitMenu())) && owner == client.getActivePlayer())
		{
			bool isMarked = (markerPossible && gameGUI.selectedMenuButtonIndex == nr) || sentryActive;
			drawContextItem (lngPack.i18n ("Text~Context~Sentry"), isMarked, dest.x, dest.y, buffer);
			dest.y += 22;
			++nr;
		}

		// Activate / Load:
		if (data.storageUnitsMax > 0 && owner == client.getActivePlayer())
		{
			// Activate:
			bool isMarked = markerPossible && gameGUI.selectedMenuButtonIndex == nr;
			drawContextItem (lngPack.i18n ("Text~Context~Active"), isMarked, dest.x, dest.y, buffer);
			dest.y += 22;
			++nr;

			// Load:
			isMarked = (markerPossible && gameGUI.selectedMenuButtonIndex == nr) || gameGUI.mouseInputMode == loadMode;
			drawContextItem (lngPack.i18n ("Text~Context~Load"), isMarked, dest.x, dest.y, buffer);
			dest.y += 22;
			++nr;
		}

		// research
		if (data.canResearch && isUnitWorking() && owner == client.getActivePlayer())
		{
			bool isMarked = markerPossible && gameGUI.selectedMenuButtonIndex == nr;
			drawContextItem (lngPack.i18n ("Text~Context~Research"), isMarked, dest.x, dest.y, buffer);
			dest.y += 22;
			++nr;
		}

		// gold upgrades screen
		if (data.convertsGold && owner == client.getActivePlayer())
		{
			bool isMarked = markerPossible && gameGUI.selectedMenuButtonIndex == nr;
			drawContextItem (lngPack.i18n ("Text~Context~Upgrades"), isMarked, dest.x, dest.y, buffer);
			dest.y += 22;
			++nr;
		}

		// Updates:
		if (buildingCanBeUpgraded() && owner == client.getActivePlayer())
		{
			// Update all buildings of this type in this subbase
			bool isMarked = markerPossible && gameGUI.selectedMenuButtonIndex == nr;
			drawContextItem (lngPack.i18n ("Text~Context~UpAll"), isMarked, dest.x, dest.y, buffer);
			dest.y += 22;
			++nr;

			// update this building
			isMarked = markerPossible && gameGUI.selectedMenuButtonIndex == nr;
			drawContextItem (lngPack.i18n ("Text~Context~Upgrade"), isMarked, dest.x, dest.y, buffer);
			dest.y += 22;
			++nr;
		}

		// Self destruct
		if (data.canSelfDestroy && owner == client.getActivePlayer())
		{
			bool isMarked = markerPossible && gameGUI.selectedMenuButtonIndex == nr;
			drawContextItem (lngPack.i18n ("Text~Context~Destroy"), isMarked, dest.x, dest.y, buffer);
			dest.y += 22;
			++nr;
		}

		// Ammo:
		if (data.canRearm && data.storageResCur >= 1 && owner == client.getActivePlayer())
		{
			bool isMarked = (markerPossible && gameGUI.selectedMenuButtonIndex == nr) || gameGUI.mouseInputMode == muniActive;
			drawContextItem (lngPack.i18n ("Text~Context~Reload"), isMarked, dest.x, dest.y, buffer);
			dest.y += 22;
			++nr;
		}

		// Repair:
		if (data.canRepair && data.storageResCur >= 1 && owner == client.getActivePlayer())
		{
			bool isMarked = (markerPossible && gameGUI.selectedMenuButtonIndex == nr) || gameGUI.mouseInputMode == repairActive;
			drawContextItem (lngPack.i18n ("Text~Context~Repair"), isMarked, dest.x, dest.y, buffer);
			dest.y += 22;
			++nr;
		}

		// Lay mines:
		if (data.canPlaceMines && data.storageResCur > 0 && owner == client.getActivePlayer())
		{
			bool isMarked = (markerPossible && gameGUI.selectedMenuButtonIndex == nr) || isUnitLayingMines();
			drawContextItem (lngPack.i18n ("Text~Context~Seed"), isMarked, dest.x, dest.y, buffer);
			dest.y += 22;
			++nr;
		}

		// Collect/clear mines:
		if (data.canPlaceMines && data.storageResCur < data.storageResMax && owner == client.getActivePlayer())
		{
			bool isMarked = (markerPossible && gameGUI.selectedMenuButtonIndex == nr) || isUnitClearingMines();
			drawContextItem (lngPack.i18n ("Text~Context~Clear"), isMarked, dest.x, dest.y, buffer);
			dest.y += 22;
			++nr;
		}

		// Sabotage/disable:
		if (data.canDisable && data.shotsCur && owner == client.getActivePlayer())
		{
			bool isMarked = (markerPossible && gameGUI.selectedMenuButtonIndex == nr) || gameGUI.mouseInputMode == disableMode;
			drawContextItem (lngPack.i18n ("Text~Context~Disable"), isMarked, dest.x, dest.y, buffer);
			dest.y += 22;
			++nr;
		}

		// Steal:
		if (data.canCapture && data.shotsCur && owner == client.getActivePlayer())
		{
			bool isMarked = (markerPossible && gameGUI.selectedMenuButtonIndex == nr) || gameGUI.mouseInputMode == stealMode;
			drawContextItem (lngPack.i18n ("Text~Context~Steal"), isMarked, dest.x, dest.y, buffer);
			dest.y += 22;
			++nr;
		}

	}
	// Info:
	bool isMarked = markerPossible && gameGUI.selectedMenuButtonIndex == nr;
	drawContextItem (lngPack.i18n ("Text~Context~Info"), isMarked, dest.x, dest.y, buffer);
	dest.y += 22;
	++nr;

	// Done:
	isMarked = markerPossible && gameGUI.selectedMenuButtonIndex == nr;
	drawContextItem (lngPack.i18n ("Text~Context~Done"), isMarked, dest.x, dest.y, buffer);
}

//------------------------------------------------------------------------------
void cUnit::menuReleased (cGameGUI& gameGUI)
{
	SDL_Rect dest = getMenuSize (gameGUI);
	int exeNr = -1000;
	if (areCoordsOverMenu (gameGUI, mouse->x, mouse->y))
		exeNr = (mouse->y - dest.y) / 22;

	if (exeNr != gameGUI.selectedMenuButtonIndex)
	{
		gameGUI.selectedMenuButtonIndex = -1;
		return;
	}
	if (isUnitMoving() || isBeeingAttacked)
		return;

	if (factoryHasJustFinishedBuilding())
		return;

	int nr = 0;
	cClient& client = *gameGUI.getClient();

	// no menu if something disabled - origina behavior -- nonsinn
	if (isDisabled() == false)
	{
		// attack:
		if (data.canAttack && data.shotsCur && owner == client.getActivePlayer())
		{
			if (exeNr == nr)
			{
				gameGUI.unitMenuActive = false;
				PlayFX (SoundData.SNDObjectMenu);
				gameGUI.toggleMouseInputMode (mouseInputAttackMode);
				return;
			}
			++nr;
		}

		// Build:
		if (data.canBuild.empty() == false && isUnitBuildingABuilding() == false && owner == client.getActivePlayer())
		{
			if (exeNr == nr)
			{
				gameGUI.unitMenuActive = false;
				PlayFX (SoundData.SNDObjectMenu);
				executeBuildCommand (gameGUI);
				return;
			}
			++nr;
		}

		// distribute:
		if (data.canMineMaxRes > 0 && isUnitWorking() && owner == client.getActivePlayer())
		{
			if (exeNr == nr)
			{
				cBuilding* building = static_cast<cBuilding*> (this);
				gameGUI.unitMenuActive = false;
				PlayFX (SoundData.SNDObjectMenu);
				building->executeMineManagerCommand (client);
				return;
			}
			++nr;
		}

		// transfer:
		if (data.storeResType != sUnitData::STORE_RES_NONE && isUnitBuildingABuilding() == false && isUnitClearing() == false && owner == client.getActivePlayer())
		{
			if (exeNr == nr)
			{
				gameGUI.unitMenuActive = false;
				PlayFX (SoundData.SNDObjectMenu);
				gameGUI.toggleMouseInputMode (transferMode);
				return;
			}
			++nr;
		}

		// Start:
		if (data.canWork && buildingCanBeStarted() && owner == client.getActivePlayer())
		{
			if (exeNr == nr)
			{
				gameGUI.unitMenuActive = false;
				PlayFX (SoundData.SNDObjectMenu);
				sendWantStartWork (client, *this);
				return;
			}
			++nr;
		}

		// auto
		if (data.canSurvey && owner == client.getActivePlayer())
		{
			if (exeNr == nr)
			{
				cVehicle* vehicle = static_cast<cVehicle*> (this);
				gameGUI.unitMenuActive = false;
				PlayFX (SoundData.SNDObjectMenu);
				vehicle->executeAutoMoveJobCommand (client);
				return;
			}
			++nr;
		}

		// stop:
		if (canBeStoppedViaUnitMenu() && owner == client.getActivePlayer())
		{
			if (exeNr == nr)
			{
				gameGUI.unitMenuActive = false;
				PlayFX (SoundData.SNDObjectMenu);
				executeStopCommand (client);
				return;
			}
			++nr;
		}

		// remove:
		if (data.canClearArea && client.getMap()->fields[client.getMap()->getOffset (PosX, PosY)].getRubble() != 0 && isUnitClearing() == false && owner == client.getActivePlayer())
		{
			if (exeNr == nr)
			{
				assert (this->isAVehicle());
				gameGUI.unitMenuActive = false;
				PlayFX (SoundData.SNDObjectMenu);
				sendWantStartClear (client, static_cast<cVehicle&> (*this));
				return;
			}
			++nr;
		}

		// manual Fire:
		if ( (manualFireActive || data.canAttack) && owner == client.getActivePlayer())
		{
			if (exeNr == nr)
			{
				gameGUI.unitMenuActive = false;
				PlayFX (SoundData.SNDObjectMenu);
				sendChangeManualFireStatus (client, iID, isAVehicle());
				return;
			}
			++nr;
		}

		// sentry:
		if ( (sentryActive || data.canAttack || (!isABuilding() && !canBeStoppedViaUnitMenu())) && owner == client.getActivePlayer())
		{
			if (exeNr == nr)
			{
				gameGUI.unitMenuActive = false;
				PlayFX (SoundData.SNDObjectMenu);
				sendChangeSentry (client, iID, isAVehicle());
				return;
			}
			++nr;
		}

		// activate/load:
		if (data.storageUnitsMax > 0 && owner == client.getActivePlayer())
		{
			// activate:
			if (exeNr == nr)
			{
				gameGUI.unitMenuActive = false;
				PlayFX (SoundData.SNDObjectMenu);
				executeActivateStoredVehiclesCommand (client);
				return;
			}
			++nr;

			// load:
			if (exeNr == nr)
			{
				gameGUI.unitMenuActive = false;
				PlayFX (SoundData.SNDObjectMenu);
				gameGUI.toggleMouseInputMode (loadMode);
				return;
			}
			++nr;
		}

		// research
		if (data.canResearch && isUnitWorking() && owner == client.getActivePlayer())
		{
			if (exeNr == nr)
			{
				gameGUI.unitMenuActive = false;
				PlayFX (SoundData.SNDObjectMenu);
				cDialogResearch researchDialog (client, owner);
				researchDialog.show (&client);
				return;
			}
			++nr;
		}

		// gold upgrades screen
		if (data.convertsGold && owner == client.getActivePlayer())
		{
			if (exeNr == nr)
			{
				gameGUI.unitMenuActive = false;
				PlayFX (SoundData.SNDObjectMenu);
				cUpgradeMenu upgradeMenu (client, owner);
				upgradeMenu.show (&client);
				return;
			}
			++nr;
		}

		// Updates:
		if (buildingCanBeUpgraded() && owner == client.getActivePlayer())
		{
			// Update all buildings of this type in this subbase
			if (exeNr == nr)
			{
				cBuilding* building = static_cast<cBuilding*> (this);

				gameGUI.unitMenuActive = false;
				PlayFX (SoundData.SNDObjectMenu);
				building->executeUpdateBuildingCommmand (client, true);
				return;
			}
			++nr;

			// update this building
			if (exeNr == nr)
			{
				cBuilding* building = static_cast<cBuilding*> (this);

				gameGUI.unitMenuActive = false;
				PlayFX (SoundData.SNDObjectMenu);
				building->executeUpdateBuildingCommmand (client, false);
				return;
			}
			++nr;
		}

		// Self destruct
		if (data.canSelfDestroy && owner == client.getActivePlayer())
		{
			if (exeNr == nr)
			{
				cBuilding* building = static_cast<cBuilding*> (this);

				gameGUI.unitMenuActive = false;
				PlayFX (SoundData.SNDObjectMenu);
				building->executeSelfDestroyCommand (client);
				return;
			}
			++nr;
		}

		// rearm:
		if (data.canRearm && data.storageResCur >= 1 && owner == client.getActivePlayer())
		{
			if (exeNr == nr)
			{
				gameGUI.unitMenuActive = false;
				PlayFX (SoundData.SNDObjectMenu);
				gameGUI.toggleMouseInputMode (muniActive);
				return;
			}
			++nr;
		}

		// repair:
		if (data.canRepair && data.storageResCur >= 1 && owner == client.getActivePlayer())
		{
			if (exeNr == nr)
			{
				gameGUI.unitMenuActive = false;
				PlayFX (SoundData.SNDObjectMenu);
				gameGUI.toggleMouseInputMode (repairActive);
				return;
			}
			++nr;
		}

		// lay mines:
		if (data.canPlaceMines && data.storageResCur > 0 && owner == client.getActivePlayer())
		{
			if (exeNr == nr)
			{
				cVehicle* vehicle = static_cast<cVehicle*> (this);
				gameGUI.unitMenuActive = false;
				PlayFX (SoundData.SNDObjectMenu);
				vehicle->executeLayMinesCommand (client);
				return;
			}
			++nr;
		}

		// clear mines:
		if (data.canPlaceMines && data.storageResCur < data.storageResMax && owner == client.getActivePlayer())
		{
			if (exeNr == nr)
			{
				cVehicle* vehicle = static_cast<cVehicle*> (this);
				gameGUI.unitMenuActive = false;
				PlayFX (SoundData.SNDObjectMenu);
				vehicle->executeClearMinesCommand (client);
				return;
			}
			++nr;
		}

		// disable:
		if (data.canDisable && data.shotsCur > 0 && owner == client.getActivePlayer())
		{
			if (exeNr == nr)
			{
				gameGUI.unitMenuActive = false;
				PlayFX (SoundData.SNDObjectMenu);
				gameGUI.toggleMouseInputMode (disableMode);
				return;
			}
			++nr;
		}

		// steal:
		if (data.canCapture && data.shotsCur > 0 && owner == client.getActivePlayer())
		{
			if (exeNr == nr)
			{
				gameGUI.unitMenuActive = false;
				PlayFX (SoundData.SNDObjectMenu);
				gameGUI.toggleMouseInputMode (stealMode);
				return;
			}
			++nr;
		}
	}
	// help/info:
	if (exeNr == nr)
	{
		gameGUI.unitMenuActive = false;
		PlayFX (SoundData.SNDObjectMenu);
		cUnitHelpMenu helpMenu (&data, owner);
		helpMenu.show (&client);
		return;
	}
	++nr;

	// done:
	if (exeNr == nr)
	{
		gameGUI.unitMenuActive = false;
		PlayFX (SoundData.SNDObjectMenu);
		if (owner == client.getActivePlayer())
		{
			isMarkedAsDone = true;
			sendMoveJobResume (client, iID);
		}
		return;
	}
}

//------------------------------------------------------------------------------
/** Returns the screen x position of the unit */
//------------------------------------------------------------------------------
int cUnit::getScreenPosX (const cGameGUI& gameGUI, bool movementOffset) const
{
	const int offset = movementOffset ? getMovementOffsetX() : 0;
	return 180 - ( (int) ( (gameGUI.getOffsetX() - offset) * gameGUI.getZoom())) + (int) (gameGUI.getTileSize()) * PosX;
}

//------------------------------------------------------------------------------
/** Returns the screen y position of the unit */
//------------------------------------------------------------------------------
int cUnit::getScreenPosY (const cGameGUI& gameGUI, bool movementOffset) const
{
	const int offset = movementOffset ? getMovementOffsetY() : 0;
	return 18 - ( (int) ( (gameGUI.getOffsetY() - offset) * gameGUI.getZoom())) + (int) (gameGUI.getTileSize()) * PosY;
}

//------------------------------------------------------------------------------
/** Centers on this unit */
//------------------------------------------------------------------------------
void cUnit::center (cGameGUI& gameGUI) const
{
	const int offX = PosX * 64 - ( (int) ( ( (float) (Video.getResolutionX() - 192) / (2 * gameGUI.getTileSize())) * 64)) + 32;
	const int offY = PosY * 64 - ( (int) ( ( (float) (Video.getResolutionY() - 32)  / (2 * gameGUI.getTileSize())) * 64)) + 32;
	gameGUI.setOffsetPosition (offX, offY);
}

//------------------------------------------------------------------------------
/** Draws the ammunition bar over the unit */
//------------------------------------------------------------------------------
void cUnit::drawMunBar (const cGameGUI& gameGUI, const SDL_Rect& screenPos) const
{
	if (owner != gameGUI.getClient()->getActivePlayer())
		return;

	SDL_Rect r1;
	r1.x = screenPos.x + gameGUI.getTileSize() / 10 + 1;
	r1.y = screenPos.y + gameGUI.getTileSize() / 10 + gameGUI.getTileSize() / 8;
	r1.w = gameGUI.getTileSize() * 8 / 10;
	r1.h = gameGUI.getTileSize() / 8;

	if (r1.h <= 2)
	{
		r1.y += 1;
		r1.h = 3;
	}

	SDL_Rect r2;
	r2.x = r1.x + 1;
	r2.y = r1.y + 1;
	r2.w = (int) ( ( (float) (r1.w - 2) / data.ammoMax) * data.ammoCur);
	r2.h = r1.h - 2;

	SDL_FillRect (buffer, &r1, 0);

	if (data.ammoCur > data.ammoMax / 2)
		SDL_FillRect (buffer, &r2, 0x0004AE04);
	else if (data.ammoCur > data.ammoMax / 4)
		SDL_FillRect (buffer, &r2, 0x00DBDE00);
	else
		SDL_FillRect (buffer, &r2, 0x00E60000);
}

//------------------------------------------------------------------------------
/** draws the health bar over the unit */
//------------------------------------------------------------------------------
void cUnit::drawHealthBar (const cGameGUI& gameGUI, const SDL_Rect& screenPos) const
{
	SDL_Rect r1;
	r1.x = screenPos.x + gameGUI.getTileSize() / 10 + 1;
	r1.y = screenPos.y + gameGUI.getTileSize() / 10;
	r1.w = gameGUI.getTileSize() * 8 / 10;
	r1.h = gameGUI.getTileSize() / 8;

	if (data.isBig)
	{
		r1.w += gameGUI.getTileSize();
		r1.h *= 2;
	}

	if (r1.h <= 2)
		r1.h = 3;

	SDL_Rect r2;
	r2.x = r1.x + 1;
	r2.y = r1.y + 1;
	r2.w = (int) ( ( (float) (r1.w - 2) / data.hitpointsMax) * data.hitpointsCur);
	r2.h = r1.h - 2;

	SDL_FillRect (buffer, &r1, 0);

	if (data.hitpointsCur > data.hitpointsMax / 2)
		SDL_FillRect (buffer, &r2, 0x0004AE04);
	else if (data.hitpointsCur > data.hitpointsMax / 4)
		SDL_FillRect (buffer, &r2, 0x00DBDE00);
	else
		SDL_FillRect (buffer, &r2, 0x00E60000);
}

//------------------------------------------------------------------------------
void cUnit::drawStatus (const cGameGUI& gameGUI, const SDL_Rect& screenPos) const
{
	SDL_Rect speedSymbol = {244, 97, 8, 10};
	SDL_Rect shotsSymbol = {254, 97, 5, 10};
	SDL_Rect disabledSymbol = {150, 109, 25, 25};
	SDL_Rect dest;

	if (isDisabled())
	{
		if (gameGUI.getTileSize() < 25)
			return;
		dest.x = screenPos.x + gameGUI.getTileSize() / 2 - 12;
		dest.y = screenPos.y + gameGUI.getTileSize() / 2 - 12;
		SDL_BlitSurface (GraphicsData.gfx_hud_stuff, &disabledSymbol, buffer, &dest);
	}
	else
	{
		dest.y = screenPos.y + gameGUI.getTileSize() - 11;
		dest.x = screenPos.x + gameGUI.getTileSize() / 2 - 4;
		if (data.isBig)
		{
			dest.y += (gameGUI.getTileSize() / 2);
			dest.x += (gameGUI.getTileSize() / 2);
		}
		if (data.speedCur >= 4)
		{
			if (data.shotsCur)
				dest.x -= gameGUI.getTileSize() / 4;

			SDL_Rect destCopy = dest;
			SDL_BlitSurface (GraphicsData.gfx_hud_stuff, &speedSymbol, buffer, &destCopy);
		}

		dest.x = screenPos.x + gameGUI.getTileSize() / 2 - 4;
		if (data.shotsCur)
		{
			if (data.speedCur)
				dest.x += gameGUI.getTileSize() / 4;
			SDL_BlitSurface (GraphicsData.gfx_hud_stuff, &shotsSymbol, buffer, &dest);
		}
	}
}

//------------------------------------------------------------------------------
/** rotates the unit to the given direction */
//------------------------------------------------------------------------------
void cUnit::rotateTo (int newDir)
{
	if (newDir < 0 || newDir >= 8 || newDir == dir)
		return;

	int t = dir;
	int dest = 0;

	for (int i = 0; i < 8; ++i)
	{
		if (t == newDir)
		{
			dest = i;
			break;
		}
		++t;

		if (t > 7)
			t = 0;
	}

	if (dest < 4)
		++dir;
	else
		--dir;

	if (dir < 0)
		dir += 8;
	else
	{
		if (dir > 7)
			dir -= 8;
	}
}

//------------------------------------------------------------------------------
/** Checks, if the unit can attack an object at the given coordinates*/
//------------------------------------------------------------------------------
bool cUnit::canAttackObjectAt (int x, int y, cMap* map, bool forceAttack, bool checkRange) const
{
	if (data.canAttack == false) return false;
	if (data.shotsCur <= 0) return false;
	if (data.ammoCur <= 0) return false;
	if (attacking) return false;
	if (isBeeingAttacked) return false;
	if (isAVehicle() && static_cast<const cVehicle*> (this)->isUnitLoaded()) return false;
	if (map->isValidPos (x, y) == false) return false;
	if (checkRange && isInRange (x, y) == false) return false;
	const int off = map->getOffset (x, y);

	if (data.muzzleType == sUnitData::MUZZLE_TYPE_TORPEDO && map->isWaterOrCoast (x, y) == false)
		return false;

	const cUnit* target = selectTarget (x, y, data.canAttack, map);

	if (target && target->iID == iID)  // a unit cannot fire on itself
		return false;

	if (owner->ScanMap[off] == false && !forceAttack)
		return false;

	if (forceAttack)
		return true;

	if (target == NULL)
		return false;

	// do not fire on e.g. platforms, connectors etc.
	// see ticket #436 on bug tracker
	if (target->isABuilding() && isAVehicle() && data.factorAir == 0 && map->possiblePlace (*static_cast<const cVehicle*> (this), x, y))
		return false;

	if (target->owner == owner)
		return false;

	return true;
}

//------------------------------------------------------------------------------
void cUnit::upgradeToCurrentVersion()
{
	if (owner == NULL) return;
	const sUnitData* upgradeVersion = owner->getUnitDataCurrentVersion (data.ID);
	if (upgradeVersion == NULL) return;

	data.version = upgradeVersion->version;

	// TODO: check behaviour in original
	if (data.hitpointsCur == data.hitpointsMax)
		data.hitpointsCur = upgradeVersion->hitpointsMax;
	data.hitpointsMax = upgradeVersion->hitpointsMax;

	// don't change the current ammo-amount!
	data.ammoMax = upgradeVersion->ammoMax;

	data.speedMax = upgradeVersion->speedMax;

	data.armor = upgradeVersion->armor;
	data.scan = upgradeVersion->scan;
	data.range = upgradeVersion->range;
	// TODO: check behaviour in original
	data.shotsMax = upgradeVersion->shotsMax;
	data.damage = upgradeVersion->damage;
	data.buildCosts = upgradeVersion->buildCosts;
}

//------------------------------------------------------------------------------
void cUnit::deleteStoredUnits()
{
	for (size_t i = 0; i != storedUnits.size(); ++i)
	{
		cVehicle* vehicle = storedUnits[i];
		remove_from_intrusivelist (vehicle->owner->VehicleList, *vehicle);
		vehicle->deleteStoredUnits();

		delete vehicle;
	}
	storedUnits.clear();
}
