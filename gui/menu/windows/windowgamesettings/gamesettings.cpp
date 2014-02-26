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

#include "gamesettings.h"
#include "../../../../menus.h" // for sSettings
#include "../../../../utility/tounderlyingtype.h"

//------------------------------------------------------------------------------
cGameSettings::cGameSettings () :
	metalAmount (eGameSettingsResourceAmount::Normal),
	oilAmount (eGameSettingsResourceAmount::Normal),
	goldAmount (eGameSettingsResourceAmount::Normal),
	resourceDensity (eGameSettingsResourceDensity::Normal),
	bridgeheadType (eGameSettingsBridgeheadType::Definite),
	gameType (eGameSettingsGameType::Simultaneous),
	clansEnabled (true),
	startCredits (150),
	victoryConditionType (eGameSettingsVictoryCondition::Death),
	victoryTurns (400),
	vectoryPoints (400)
{}

//------------------------------------------------------------------------------
eGameSettingsResourceAmount cGameSettings::getMetalAmount () const
{
	return metalAmount;
}

//------------------------------------------------------------------------------
void cGameSettings::setMetalAmount (eGameSettingsResourceAmount value)
{
	metalAmount = value;
}

//------------------------------------------------------------------------------
eGameSettingsResourceAmount cGameSettings::getOilAmount () const
{
	return oilAmount;
}

//------------------------------------------------------------------------------
void cGameSettings::setOilAmount (eGameSettingsResourceAmount value)
{
	oilAmount = value;
}

//------------------------------------------------------------------------------
eGameSettingsResourceAmount cGameSettings::getGoldAmount () const
{
	return goldAmount;
}

//------------------------------------------------------------------------------
void cGameSettings::setGoldAmount (eGameSettingsResourceAmount value)
{
	goldAmount = value;
}

//------------------------------------------------------------------------------
eGameSettingsResourceDensity cGameSettings::getResourceDensity () const
{
	return resourceDensity;
}

//------------------------------------------------------------------------------
void cGameSettings::setResourceDensity (eGameSettingsResourceDensity value)
{
	resourceDensity = value;
}

//------------------------------------------------------------------------------
eGameSettingsBridgeheadType cGameSettings::getBridgeheadType () const
{
	return bridgeheadType;
}

//------------------------------------------------------------------------------
void cGameSettings::setBridgeheadType (eGameSettingsBridgeheadType value)
{
	bridgeheadType = value;
}

//------------------------------------------------------------------------------
eGameSettingsGameType cGameSettings::getGameType () const
{
	return gameType;
}

//------------------------------------------------------------------------------
void cGameSettings::setGameType (eGameSettingsGameType value)
{
	gameType = value;
}

//------------------------------------------------------------------------------
bool cGameSettings::getClansEnabled () const
{
	return clansEnabled;
}

//------------------------------------------------------------------------------
void cGameSettings::setClansEnabled (bool value)
{
	clansEnabled = value;
}

//------------------------------------------------------------------------------
unsigned int cGameSettings::getStartCredits () const
{
	return startCredits;
}

//------------------------------------------------------------------------------
void cGameSettings::setStartCredits (unsigned int value)
{
	startCredits = value;
}

//------------------------------------------------------------------------------
eGameSettingsVictoryCondition cGameSettings::getVictoryCondition () const
{
	return victoryConditionType;
}

//------------------------------------------------------------------------------
void cGameSettings::setVictoryCondition (eGameSettingsVictoryCondition value)
{
	victoryConditionType = value;
}

//------------------------------------------------------------------------------
unsigned int cGameSettings::getVictoryTurns () const
{
	return victoryTurns;
}

//------------------------------------------------------------------------------
void cGameSettings::setVictoryTurns (unsigned int value)
{
	victoryTurns = value;
}

//------------------------------------------------------------------------------
unsigned int cGameSettings::getVictoryPoints () const
{
	return vectoryPoints;
}

//------------------------------------------------------------------------------
void cGameSettings::setVictoryPoints (unsigned int value)
{
	vectoryPoints = value;
}

//------------------------------------------------------------------------------
sSettings cGameSettings::toOldSettings () const
{
	sSettings settings;

	settings.metal = (eSettingResourceValue)toUnderlyingType (getMetalAmount ());
	settings.oil = (eSettingResourceValue)toUnderlyingType (getOilAmount ());
	settings.gold = (eSettingResourceValue)toUnderlyingType (getGoldAmount ());

	settings.resFrequency = (eSettingResFrequency)toUnderlyingType (getResourceDensity ());

	settings.credits = getStartCredits ();

	settings.bridgeHead = (eSettingsBridgeHead)toUnderlyingType (getBridgeheadType ());
	settings.alienTech = eSettingsAlienTech::SETTING_ALIENTECH_OFF;
	settings.clans = getClansEnabled () ? eSettingsClans::SETTING_CLANS_ON : eSettingsClans::SETTING_CLANS_OFF;
	settings.gameType = (eSettingsGameType)toUnderlyingType (getGameType ());
	settings.victoryType = (eSettingsVictoryType)toUnderlyingType (getVictoryCondition ());
	settings.duration = getVictoryTurns ();
	settings.iTurnDeadline = 90;
	settings.hotseat = false;

	return settings;
}