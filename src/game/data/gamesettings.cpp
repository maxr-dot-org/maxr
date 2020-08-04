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
#include "utility/tounderlyingtype.h"
#include "utility/string/iequals.h"
#include "main.h"
#include "utility/crc.h"

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
std::string gameSettingsResourceAmountToString (eGameSettingsResourceAmount amount, bool translated)
{
	if (translated)
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
	}
	else
	{
		switch (amount)
		{
			case eGameSettingsResourceAmount::Limited:
				return "limited";
			case eGameSettingsResourceAmount::Normal:
				return "normal";
			case eGameSettingsResourceAmount::High:
				return "high";
			case eGameSettingsResourceAmount::TooMuch:
				return "toomuch";
		}
	}
	assert (false);
	return "";
}

//------------------------------------------------------------------------------
std::string gameSettingsResourceDensityToString (eGameSettingsResourceDensity density, bool translated)
{
	if (translated)
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
	}
	else
	{
		switch (density)
		{
			case eGameSettingsResourceDensity::Sparse:
				return "sparse";
			case eGameSettingsResourceDensity::Normal:
				return "normal";
			case eGameSettingsResourceDensity::Dense:
				return "dense";
			case eGameSettingsResourceDensity::TooMuch:
				return "toomuch";
		}
	}
	assert (false);
	return "";
}

//------------------------------------------------------------------------------
std::string gameSettingsBridgeheadTypeToString (eGameSettingsBridgeheadType type, bool translated)
{
	if (translated)
	{
		switch (type)
		{
			case eGameSettingsBridgeheadType::Definite:
				return lngPack.i18n ("Text~Option~Definite");
			case eGameSettingsBridgeheadType::Mobile:
				return lngPack.i18n ("Text~Option~Mobile");
		}
	}
	else
	{
		switch (type)
		{
			case eGameSettingsBridgeheadType::Definite:
				return "definite";
			case eGameSettingsBridgeheadType::Mobile:
				return "mobile";
		}
	}
	assert (false);
	return "";
}

//------------------------------------------------------------------------------
std::string gameSettingsGameTypeToString (eGameSettingsGameType type, bool translated)
{
	if (translated)
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
	}
	else
	{
		switch (type)
		{
			case eGameSettingsGameType::Simultaneous:
				return "simultaneous";
			case eGameSettingsGameType::Turns:
				return "turns";
			case eGameSettingsGameType::HotSeat:
				return "hotseat";
		}
	}
	assert (false);
	return "";
}

//------------------------------------------------------------------------------
std::string gameSettingsVictoryConditionToString (eGameSettingsVictoryCondition condition, bool translated)
{
	if (translated)
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
	}
	else
	{
		switch (condition)
		{
			case eGameSettingsVictoryCondition::Turns:
				return "turns";
			case eGameSettingsVictoryCondition::Points:
				return "points";
			case eGameSettingsVictoryCondition::Death:
				return "death";
		}
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
	std::swap (metalAmount, value);
	if (metalAmount != value) metalAmountChanged();
}

//------------------------------------------------------------------------------
eGameSettingsResourceAmount cGameSettings::getOilAmount() const
{
	return oilAmount;
}

//------------------------------------------------------------------------------
void cGameSettings::setOilAmount (eGameSettingsResourceAmount value)
{
	std::swap (oilAmount, value);
	if (oilAmount != value) oilAmountChanged();
}

//------------------------------------------------------------------------------
eGameSettingsResourceAmount cGameSettings::getGoldAmount() const
{
	return goldAmount;
}

//------------------------------------------------------------------------------
void cGameSettings::setGoldAmount (eGameSettingsResourceAmount value)
{
	std::swap (goldAmount, value);
	if (goldAmount != value) goldAmountChanged();
}

//------------------------------------------------------------------------------
eGameSettingsResourceDensity cGameSettings::getResourceDensity() const
{
	return resourceDensity;
}

//------------------------------------------------------------------------------
void cGameSettings::setResourceDensity (eGameSettingsResourceDensity value)
{
	std::swap (resourceDensity, value);
	if (resourceDensity != value) resourceDensityChanged();
}

//------------------------------------------------------------------------------
eGameSettingsBridgeheadType cGameSettings::getBridgeheadType() const
{
	return bridgeheadType;
}

//------------------------------------------------------------------------------
void cGameSettings::setBridgeheadType (eGameSettingsBridgeheadType value)
{
	std::swap (bridgeheadType, value);
	if (bridgeheadType != value) bridgeheadTypeChanged();
}

//------------------------------------------------------------------------------
eGameSettingsGameType cGameSettings::getGameType() const
{
	return gameType;
}

//------------------------------------------------------------------------------
void cGameSettings::setGameType (eGameSettingsGameType value)
{
	std::swap (gameType, value);
	if (gameType != value) gameTypeChanged();
}

//------------------------------------------------------------------------------
bool cGameSettings::getClansEnabled() const
{
	return clansEnabled;
}

//------------------------------------------------------------------------------
void cGameSettings::setClansEnabled (bool value)
{
	std::swap (clansEnabled, value);
	if (clansEnabled != value) clansEnabledChanged();
}

//------------------------------------------------------------------------------
unsigned int cGameSettings::getStartCredits() const
{
	return startCredits;
}

//------------------------------------------------------------------------------
void cGameSettings::setStartCredits (unsigned int value)
{
	std::swap (startCredits, value);
	if (startCredits != value) startCreditsChanged();
}

//------------------------------------------------------------------------------
eGameSettingsVictoryCondition cGameSettings::getVictoryCondition() const
{
	return victoryConditionType;
}

//------------------------------------------------------------------------------
void cGameSettings::setVictoryCondition (eGameSettingsVictoryCondition value)
{
	std::swap (victoryConditionType, value);
	if (victoryConditionType != value) victoryConditionTypeChanged();
}

//------------------------------------------------------------------------------
unsigned int cGameSettings::getVictoryTurns() const
{
	return victoryTurns;
}

//------------------------------------------------------------------------------
void cGameSettings::setVictoryTurns (unsigned int value)
{
	std::swap (victoryTurns, value);
	if (victoryTurns != value) victoryTurnsChanged();
}

//------------------------------------------------------------------------------
unsigned int cGameSettings::getVictoryPoints() const
{
	return victoryPoints;
}

//------------------------------------------------------------------------------
void cGameSettings::setVictoryPoints (unsigned int value)
{
	std::swap (victoryPoints, value);
	if (victoryPoints != value) victoryPointsChanged();
}

//------------------------------------------------------------------------------
const std::chrono::seconds& cGameSettings::getTurnEndDeadline() const
{
	return turnEndDeadline;
}

//------------------------------------------------------------------------------
void cGameSettings::setTurnEndDeadline (const std::chrono::seconds& value)
{
	const auto oldValue = turnEndDeadline;
	turnEndDeadline = value;
	if (oldValue != turnEndDeadline) turnEndDeadlineChanged();
}

//------------------------------------------------------------------------------
bool cGameSettings::isTurnEndDeadlineActive() const
{
	return turnEndDeadlineActive;
}

//------------------------------------------------------------------------------
void cGameSettings::setTurnEndDeadlineActive (bool value)
{
	std::swap (turnEndDeadlineActive, value);
	if (turnEndDeadlineActive != value) turnEndDeadlineActiveChanged();
}

//------------------------------------------------------------------------------
const std::chrono::seconds& cGameSettings::getTurnLimit() const
{
	return turnLimit;
}

//------------------------------------------------------------------------------
void cGameSettings::setTurnLimit (const std::chrono::seconds& value)
{
	const auto oldValue = turnLimit;
	turnLimit = value;
	if (oldValue != turnLimit) turnLimitChanged();
}

//------------------------------------------------------------------------------
bool cGameSettings::isTurnLimitActive() const
{
	return turnLimitActive;
}

//------------------------------------------------------------------------------
void cGameSettings::setTurnLimitActive (bool value)
{
	std::swap (turnLimitActive, value);
	if (turnLimitActive != value) turnLimitActiveChanged();
}

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
