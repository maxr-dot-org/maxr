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
#include "../../../../utility/tounderlyingtype.h"
#include "../../../../main.h"

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
eGameSettingsResourceAmount gameSettingsResourceAmountFromString (const std::string& string)
{
	auto lowerCaseString = string;
	std::transform (lowerCaseString.begin (), lowerCaseString.end (), lowerCaseString.begin (), std::tolower);

	if (lowerCaseString.compare ("limited") == 0) return eGameSettingsResourceAmount::Limited;
	else if (lowerCaseString.compare ("normal") == 0) return eGameSettingsResourceAmount::Normal;
	else if (lowerCaseString.compare ("high") == 0) return eGameSettingsResourceAmount::High;
	else if (lowerCaseString.compare ("toomuch") == 0) return eGameSettingsResourceAmount::TooMuch;
	else throw std::runtime_error ("Invalid resource amount string '" + string + "'");
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
eGameSettingsResourceDensity gameSettingsResourceDensityFromString (const std::string& string)
{
	auto lowerCaseString = string;
	std::transform (lowerCaseString.begin (), lowerCaseString.end (), lowerCaseString.begin (), std::tolower);

	if (lowerCaseString.compare ("sparse") == 0) return eGameSettingsResourceDensity::Sparse;
	else if (lowerCaseString.compare ("normal") == 0) return eGameSettingsResourceDensity::Normal;
	else if (lowerCaseString.compare ("dense") == 0) return eGameSettingsResourceDensity::Dense;
	else if (lowerCaseString.compare ("toomuch") == 0) return eGameSettingsResourceDensity::TooMuch;
	else throw std::runtime_error ("Invalid resource density string '" + string + "'");
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
eGameSettingsBridgeheadType gameSettingsBridgeheadTypeFromString (const std::string& string)
{
	auto lowerCaseString = string;
	std::transform (lowerCaseString.begin (), lowerCaseString.end (), lowerCaseString.begin (), std::tolower);

	if (lowerCaseString.compare ("definite") == 0) return eGameSettingsBridgeheadType::Definite;
	else if (lowerCaseString.compare ("mobile") == 0) return eGameSettingsBridgeheadType::Mobile;
	else throw std::runtime_error ("Invalid bridgehead type string '" + string + "'");
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
eGameSettingsGameType gameSettingsGameTypeString (const std::string& string)
{
	auto lowerCaseString = string;
	std::transform (lowerCaseString.begin (), lowerCaseString.end (), lowerCaseString.begin (), std::tolower);

	if (lowerCaseString.compare ("simultaneous") == 0) return eGameSettingsGameType::Simultaneous;
	else if (lowerCaseString.compare ("turns") == 0) return eGameSettingsGameType::Turns;
	else if (lowerCaseString.compare ("hotseat") == 0) return eGameSettingsGameType::HotSeat;
	else throw std::runtime_error ("Invalid game type string '" + string + "'");
}

//------------------------------------------------------------------------------
std::string gameSettingsVictoryConditionToString (eGameSettingsVictoryCondition condition, bool translated)
{
	if (translated)
	{
		switch (condition)
		{
		case eGameSettingsVictoryCondition::Turns:
			return lngPack.i18n ("Text~Comp~Turns");;
		case eGameSettingsVictoryCondition::Points:
			return lngPack.i18n ("Text~Comp~Points");;
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
eGameSettingsVictoryCondition gameSettingsVictoryConditionFromString (const std::string& string)
{
	auto lowerCaseString = string;
	std::transform (lowerCaseString.begin (), lowerCaseString.end (), lowerCaseString.begin (), std::tolower);

	if (lowerCaseString.compare ("turns") == 0) return eGameSettingsVictoryCondition::Turns;
	else if (lowerCaseString.compare ("points") == 0) return eGameSettingsVictoryCondition::Points;
	else if (lowerCaseString.compare ("death") == 0) return eGameSettingsVictoryCondition::Death;
	else throw std::runtime_error ("Invalid victory condition string '" + string + "'");
}

//------------------------------------------------------------------------------
cGameSettings::cGameSettings () :
	metalAmount (eGameSettingsResourceAmount::Normal),
	oilAmount (eGameSettingsResourceAmount::Normal),
	goldAmount (eGameSettingsResourceAmount::Normal),
	resourceDensity (eGameSettingsResourceDensity::Normal),
	bridgeheadType (eGameSettingsBridgeheadType::Definite),
	gameType (eGameSettingsGameType::Simultaneous),
	clansEnabled (true),
	startCredits (defaultCreditsNormal),
	victoryConditionType (eGameSettingsVictoryCondition::Death),
	victoryTurns (400),
	vectoryPoints (400),
	turnDeadline (90)
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
unsigned int cGameSettings::getTurnDeadline () const
{
	return turnDeadline;
}

//------------------------------------------------------------------------------
void cGameSettings::setTurnDeadline (unsigned int value)
{
	turnDeadline = value;
}

//------------------------------------------------------------------------------
void cGameSettings::pushInto (cNetMessage& message) const
{
}

//------------------------------------------------------------------------------
void cGameSettings::popFrom (cNetMessage& message)
{
}