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

#include <algorithm>
#include <cctype>
#include <cassert>

#include "gamesettings.h"

#include "utility/crc.h"
#include "utility/language.h"
#include "utility/tounderlyingtype.h"
#include "utility/string/iequals.h"

const std::chrono::seconds cGameSettings::defaultTurnLimitOption0 (60);
const std::chrono::seconds cGameSettings::defaultTurnLimitOption1 (120);
const std::chrono::seconds cGameSettings::defaultTurnLimitOption2 (180);
const std::chrono::seconds cGameSettings::defaultTurnLimitOption3 (240);
const std::chrono::seconds cGameSettings::defaultTurnLimitOption4 (300);
const std::chrono::seconds cGameSettings::defaultTurnLimitOption5 (350);

const std::chrono::seconds cGameSettings::defaultEndTurnDeadlineOption0 (15);
const std::chrono::seconds cGameSettings::defaultEndTurnDeadlineOption1 (30);
const std::chrono::seconds cGameSettings::defaultEndTurnDeadlineOption2 (45);
const std::chrono::seconds cGameSettings::defaultEndTurnDeadlineOption3 (60);
const std::chrono::seconds cGameSettings::defaultEndTurnDeadlineOption4 (75);
const std::chrono::seconds cGameSettings::defaultEndTurnDeadlineOption5 (90);

//------------------------------------------------------------------------------
std::string toTranslatedString (eGameSettingsResourceAmount amount)
{
	switch (amount)
	{
		case eGameSettingsResourceAmount::Limited:
			return lngPack.i18n ("Text~Option~Limited");
		case eGameSettingsResourceAmount::Normal:
			return lngPack.i18n ("Text~Option~Normal");
		case eGameSettingsResourceAmount::High:
			return lngPack.i18n ("Text~Option~High");
		case eGameSettingsResourceAmount::TooMuch:
			return lngPack.i18n ("Text~Option~TooMuch");
	}
	assert (false);
	return "";
}

//------------------------------------------------------------------------------
std::string toTranslatedString (eGameSettingsResourceDensity density)
{
	switch (density)
	{
		case eGameSettingsResourceDensity::Sparse:
			return lngPack.i18n ("Text~Option~Sparse");
		case eGameSettingsResourceDensity::Normal:
			return lngPack.i18n ("Text~Option~Normal");
		case eGameSettingsResourceDensity::Dense:
			return lngPack.i18n ("Text~Option~Dense");
		case eGameSettingsResourceDensity::TooMuch:
			return lngPack.i18n ("Text~Option~TooMuch");
	}
	assert (false);
	return "";
}

//------------------------------------------------------------------------------
std::string toTranslatedString (eGameSettingsBridgeheadType type)
{
	switch (type)
	{
		case eGameSettingsBridgeheadType::Definite:
			return lngPack.i18n ("Text~Option~Definite");
		case eGameSettingsBridgeheadType::Mobile:
			return lngPack.i18n ("Text~Option~Mobile");
	}
	assert (false);
	return "";
}

//------------------------------------------------------------------------------
std::string toTranslatedString (eGameSettingsGameType type)
{
	switch (type)
	{
		case eGameSettingsGameType::Simultaneous:
			return lngPack.i18n ("Text~Option~Type_Simu");
		case eGameSettingsGameType::Turns:
			return lngPack.i18n ("Text~Option~Type_Turns");
		case eGameSettingsGameType::HotSeat:
			return "Hot Seat"; // TODO: translation?!
	}
	assert (false);
	return "";
}

//------------------------------------------------------------------------------
std::string toTranslatedString (eGameSettingsVictoryCondition condition)
{
	switch (condition)
	{
		case eGameSettingsVictoryCondition::Turns:
			return lngPack.i18n ("Text~Comp~Turns");
		case eGameSettingsVictoryCondition::Points:
			return lngPack.i18n ("Text~Comp~Points");
		case eGameSettingsVictoryCondition::Death:
			return lngPack.i18n ("Text~Comp~NoLimit");
	}
	assert (false);
	return "";
}

//------------------------------------------------------------------------------
eGameSettingsResourceAmount cGameSettings::getMetalAmount() const
{
	return metalAmount;
}

//------------------------------------------------------------------------------
void cGameSettings::setMetalAmount (eGameSettingsResourceAmount value)
{
	metalAmount = value;
}

//------------------------------------------------------------------------------
eGameSettingsResourceAmount cGameSettings::getOilAmount() const
{
	return oilAmount;
}

//------------------------------------------------------------------------------
void cGameSettings::setOilAmount (eGameSettingsResourceAmount value)
{
	oilAmount = value;
}

//------------------------------------------------------------------------------
eGameSettingsResourceAmount cGameSettings::getGoldAmount() const
{
	return goldAmount;
}

//------------------------------------------------------------------------------
void cGameSettings::setGoldAmount (eGameSettingsResourceAmount value)
{
	goldAmount = value;
}

//------------------------------------------------------------------------------
eGameSettingsResourceDensity cGameSettings::getResourceDensity() const
{
	return resourceDensity;
}

//------------------------------------------------------------------------------
void cGameSettings::setResourceDensity (eGameSettingsResourceDensity value)
{
	resourceDensity = value;
}

//------------------------------------------------------------------------------
eGameSettingsBridgeheadType cGameSettings::getBridgeheadType() const
{
	return bridgeheadType;
}

//------------------------------------------------------------------------------
void cGameSettings::setBridgeheadType (eGameSettingsBridgeheadType value)
{
	bridgeheadType = value;
}

//------------------------------------------------------------------------------
eGameSettingsGameType cGameSettings::getGameType() const
{
	return gameType;
}

//------------------------------------------------------------------------------
void cGameSettings::setGameType (eGameSettingsGameType value)
{
	gameType = value;
}

//------------------------------------------------------------------------------
bool cGameSettings::getClansEnabled() const
{
	return clansEnabled;
}

//------------------------------------------------------------------------------
void cGameSettings::setClansEnabled (bool value)
{
	clansEnabled = value;
}

//------------------------------------------------------------------------------
unsigned int cGameSettings::getStartCredits() const
{
	return startCredits;
}

//------------------------------------------------------------------------------
void cGameSettings::setStartCredits (unsigned int value)
{
	startCredits = value;
}

//------------------------------------------------------------------------------
eGameSettingsVictoryCondition cGameSettings::getVictoryCondition() const
{
	return victoryConditionType;
}

//------------------------------------------------------------------------------
void cGameSettings::setVictoryCondition (eGameSettingsVictoryCondition value)
{
	victoryConditionType = value;
}

//------------------------------------------------------------------------------
unsigned int cGameSettings::getVictoryTurns() const
{
	return victoryTurns;
}

//------------------------------------------------------------------------------
void cGameSettings::setVictoryTurns (unsigned int value)
{
	victoryTurns = value;
}

//------------------------------------------------------------------------------
unsigned int cGameSettings::getVictoryPoints() const
{
	return victoryPoints;
}

//------------------------------------------------------------------------------
void cGameSettings::setVictoryPoints (unsigned int value)
{
	victoryPoints = value;
}

//------------------------------------------------------------------------------
const std::chrono::seconds& cGameSettings::getTurnEndDeadline() const
{
	return turnEndDeadline;
}

//------------------------------------------------------------------------------
void cGameSettings::setTurnEndDeadline (const std::chrono::seconds& value)
{
	turnEndDeadline = value;
}

//------------------------------------------------------------------------------
bool cGameSettings::isTurnEndDeadlineActive() const
{
	return turnEndDeadlineActive;
}

//------------------------------------------------------------------------------
void cGameSettings::setTurnEndDeadlineActive (bool value)
{
	turnEndDeadlineActive = value;
}

//------------------------------------------------------------------------------
const std::chrono::seconds& cGameSettings::getTurnLimit() const
{
	return turnLimit;
}

//------------------------------------------------------------------------------
void cGameSettings::setTurnLimit (const std::chrono::seconds& value)
{
	turnLimit = value;
}

//------------------------------------------------------------------------------
bool cGameSettings::isTurnLimitActive() const
{
	return turnLimitActive;
}

//------------------------------------------------------------------------------
void cGameSettings::setTurnLimitActive (bool value)
{
	turnLimitActive = value;
}

//------------------------------------------------------------------------------
uint32_t cGameSettings::getChecksum(uint32_t crc) const
{
	crc = calcCheckSum(metalAmount, crc);
	crc = calcCheckSum(oilAmount, crc);
	crc = calcCheckSum(goldAmount, crc);
	crc = calcCheckSum(resourceDensity, crc);
	crc = calcCheckSum(bridgeheadType, crc);
	crc = calcCheckSum(gameType, crc);
	crc = calcCheckSum(clansEnabled, crc);
	crc = calcCheckSum(startCredits, crc);
	crc = calcCheckSum(victoryConditionType, crc);
	crc = calcCheckSum(victoryTurns, crc);
	crc = calcCheckSum(victoryPoints, crc);
	crc = calcCheckSum(turnEndDeadline.count(), crc);
	crc = calcCheckSum(turnEndDeadlineActive, crc);
	crc = calcCheckSum(turnLimit.count(), crc);
	crc = calcCheckSum(turnLimitActive, crc);

	return crc;
}
