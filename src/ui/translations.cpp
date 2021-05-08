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

#include "translations.h"

#include "game/data/gamesettings.h"
#include "game/data/player/clans.h"
#include "game/data/player/player.h"
#include "game/data/units/building.h"
#include "game/data/units/commandodata.h"
#include "game/data/units/vehicle.h"
#include "game/logic/endmoveaction.h"
#include "game/logic/movejob.h"
#include "output/video/unifonts.h"
#include "utility/language.h"
#include "utility/string/toString.h"

namespace
{
	//--------------------------------------------------------------------------
	std::string GetModificatorString (int original, int modified)
	{
		const int diff = modified - original;
		if (diff > 0)
			return " +" + iToStr (diff);
		else if (diff < 0)
			return " -" + iToStr (-diff);
		else // diff == 0
			return " =" + iToStr (modified);
	}

	//--------------------------------------------------------------------------
	std::string getRankString(const cCommandoData& commandoData)
	{
		const auto level = cCommandoData::getLevel (commandoData.getSuccessCount());
		const std::string suffix = (level > 0) ? " +" + std::to_string (level) : "";

		if (level < 1) return lngPack.i18n ("Text~Comp~CommandoRank_Greenhorn") + suffix;
		else if (level < 3) return lngPack.i18n ("Text~Comp~CommandoRank_Average") + suffix;
		else if (level < 6) return lngPack.i18n ("Text~Comp~CommandoRank_Veteran") + suffix;
		else if (level < 11) return lngPack.i18n ("Text~Comp~CommandoRank_Expert") + suffix;
		else if (level < 19) return lngPack.i18n ("Text~Comp~CommandoRank_Elite") + suffix;
		else return lngPack.i18n ("Text~Comp~CommandoRank_GrandMaster") + suffix;
	}

}

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
std::string getClanStatsDescription (const cClanUnitStat& clanUnitStat, const cUnitsData& originalData)
{
	const cDynamicUnitData* data = &originalData.getDynamicUnitData(clanUnitStat.getUnitId());

	if (data == nullptr) return "Unknown";

	std::string result = getStaticUnitName (originalData.getStaticUnitData (clanUnitStat.getUnitId())) + lngPack.i18n ("Text~Punctuation~Colon");
	const char* const commaSep = ", ";
	const char* sep = "";

	struct
	{
		const char* type;
		std::string text;
		int originalValue;
	} t[] =
	{
		// ToDo / Fixme if #756 fixed, use the non "_7" version of the text files
		{"Damage", lngPack.i18n ("Text~Others~Attack_7"), data->getDamage()},
		{"Range", lngPack.i18n ("Text~Others~Range"), data->getRange()},
		{"Armor", lngPack.i18n ("Text~Others~Armor_7"), data->getArmor()},
		{"Hitpoints", lngPack.i18n ("Text~Others~Hitpoints_7"), data->getHitpointsMax()},
		{"Scan", lngPack.i18n ("Text~Others~Scan_7"), data->getScan()},
		{"Speed", lngPack.i18n ("Text~Others~Speed_7"), data->getSpeedMax() / 4},
	};

	for (int i = 0; i != sizeof (t) / sizeof (*t); ++i)
	{
		if (clanUnitStat.hasModification (t[i].type) == false) continue;
		result += sep;
		result += t[i].text;
		result += GetModificatorString (t[i].originalValue, clanUnitStat.getModificationValue (t[i].type));
		sep = commaSep;
	}
	if (clanUnitStat.hasModification ("Built_Costs"))
	{
		result += sep;
		int nrTurns = clanUnitStat.getModificationValue ("Built_Costs");
		if (originalData.getStaticUnitData(data->getId()).vehicleData.isHuman == false) nrTurns /= clanUnitStat.getUnitId().isAVehicle() == 0 ? 2 : 3;

		result += iToStr (nrTurns) + " " + lngPack.i18n ("Text~Comp~Turns");
	}
	return result;
}

//------------------------------------------------------------------------------
std::vector<std::string> getClanStatsDescription (const cClan& clan, const cUnitsData& originalData)
{
	std::vector<std::string> result;
	for (int i = 0; i != clan.getNrUnitStats(); ++i)
	{
		const cClanUnitStat* stat = clan.getUnitStat (i);
		result.push_back (getClanStatsDescription(*stat, originalData));
	}
	return result;
}

//------------------------------------------------------------------------------
std::string getClanName (const cClan& clan)
{
	std::string name = lngPack.getClanName (clan.getClanID());
	if (name.empty()) return clan.getDefaultName();
	return name;
}

//------------------------------------------------------------------------------
std::string getClanDescription (const cClan& clan)
{
	auto description = lngPack.getClanDescription (clan.getClanID());
	if (description.empty()) return clan.getDefaultDescription();
	return description;
}

//------------------------------------------------------------------------------
std::string getStaticUnitName (const cStaticUnitData& unitData)
{
	std::string translatedName = lngPack.getUnitName (unitData.ID);
	if (!translatedName.empty())
		return translatedName;
	return unitData.getDefaultName();
}

//------------------------------------------------------------------------------
std::string getStaticUnitDescription (const cStaticUnitData& unitData)
{
	std::string translatedDescription = lngPack.getUnitDescription (unitData.ID);
	if (!translatedDescription.empty())
		return translatedDescription;
	return unitData.getDefaultDescription();
}
//------------------------------------------------------------------------------
std::string getName (const cUnit& unit)
{
	return unit.getCustomName().value_or (getStaticUnitName (unit.getStaticUnitData()));
}

//------------------------------------------------------------------------------
std::string getDisplayName (const cUnit& unit)
{
	return unit.getDisplayName (getStaticUnitName (unit.getStaticUnitData()));
}

//------------------------------------------------------------------------------
/** Returns a string with the current state */
//------------------------------------------------------------------------------
std::string getStatusStr (const cBuilding& building, const cPlayer* whoWantsToKnow, const cUnitsData& unitsData)
{
	auto font = cUnicodeFont::font.get();
	if (building.isDisabled())
	{
		std::string sText;
		sText = lngPack.i18n ("Text~Comp~Disabled") + " (";
		sText += iToStr (building.getDisabledTurns()) + ")";
		return sText;
	}
	if (building.isUnitWorking() || building.factoryHasJustFinishedBuilding())
	{
		// Factory:
		if (!building.getStaticUnitData().canBuild.empty() && !building.isBuildListEmpty() && building.getOwner() == whoWantsToKnow)
		{
			const cBuildListItem& buildListItem = building.getBuildListItem (0);
			const std::string& unitName = getStaticUnitName (unitsData.getStaticUnitData (buildListItem.getType()));
			std::string sText;

			if (buildListItem.getRemainingMetal() > 0)
			{
				int iRound;

				iRound = (int) ceilf (buildListItem.getRemainingMetal() / (float)building.getMetalPerRound());
				sText = lngPack.i18n ("Text~Comp~Producing") + lngPack.i18n ("Text~Punctuation~Colon");
				sText += unitName + " (";
				sText += iToStr (iRound) + ")";

				if (font->getTextWide (sText, FONT_LATIN_SMALL_WHITE) > 126)
				{
					sText = lngPack.i18n ("Text~Comp~Producing") + lngPack.i18n("Text~Punctuation~Colon") + "\n";
					sText += unitName + " (";
					sText += iToStr (iRound) + ")";
				}

				return sText;
			}
			else //new unit is rdy + which kind of unit
			{
				sText = lngPack.i18n ("Text~Comp~Producing_Fin");
				sText += lngPack.i18n ("Text~Punctuation~Colon");
				sText += unitName;

				if (font->getTextWide (sText) > 126)
				{
					sText = lngPack.i18n ("Text~Comp~Producing_Fin") + lngPack.i18n("Text~Punctuation~Colon");
					sText += "\n";
					sText += unitName;
				}
				return sText;
			}
		}

		// Research Center
		if (building.getStaticData().canResearch && building.getOwner() == whoWantsToKnow && building.getOwner())
		{
			std::string sText = lngPack.i18n ("Text~Comp~Working") + "\n";
			for (int area = 0; area < cResearch::kNrResearchAreas; area++)
			{
				if (building.getOwner()->getResearchCentersWorkingOnArea ((cResearch::ResearchArea)area) > 0)
				{
					switch (area)
					{
						case cResearch::kAttackResearch: sText += lngPack.i18n ("Text~Others~Attack"); break;
						case cResearch::kShotsResearch: sText += lngPack.i18n ("Text~Others~Shots_7"); break;
						case cResearch::kRangeResearch: sText += lngPack.i18n ("Text~Others~Range"); break;
						case cResearch::kArmorResearch: sText += lngPack.i18n ("Text~Others~Armor_7"); break;
						case cResearch::kHitpointsResearch: sText += lngPack.i18n ("Text~Others~Hitpoints_7"); break;
						case cResearch::kSpeedResearch: sText += lngPack.i18n ("Text~Others~Speed"); break;
						case cResearch::kScanResearch: sText += lngPack.i18n ("Text~Others~Scan"); break;
						case cResearch::kCostResearch: sText += lngPack.i18n ("Text~Others~Costs"); break;
					}
					sText += lngPack.i18n ("Text~Punctuation~Colon") + iToStr (building.getOwner()->getResearchState().getRemainingTurns (area, building.getOwner()->getResearchCentersWorkingOnArea ((cResearch::ResearchArea)area))) + "\n";
				}
			}
			return sText;
		}

		// Goldraffinerie:
		if (building.getStaticData().convertsGold && building.getOwner() == whoWantsToKnow && building.getOwner())
		{
			std::string sText;
			sText = lngPack.i18n ("Text~Comp~Working") + "\n";
			sText += lngPack.i18n ("Text~Title~Credits") + lngPack.i18n ("Text~Punctuation~Colon");
			sText += iToStr (building.getOwner()->getCredits());
			return sText;
		}
		return lngPack.i18n ("Text~Comp~Working");
	}

	if (building.isAttacking())
		return lngPack.i18n ("Text~Comp~AttackingStatusStr");
	else if (building.isBeeingAttacked())
		return lngPack.i18n ("Text~Comp~IsBeeingAttacked");
	else if (building.isSentryActive())
		return lngPack.i18n ("Text~Comp~Sentry");
	else if (building.isManualFireActive())
		return lngPack.i18n ("Text~Comp~ReactionFireOff");

	//GoldRaf idle + gold-amount
	if (building.getStaticData().convertsGold && building.getOwner() == whoWantsToKnow && building.getOwner() && !building.isUnitWorking())
	{
		std::string sText;
		sText = lngPack.i18n("Text~Comp~Waits") + "\n";
		sText += lngPack.i18n("Text~Title~Credits") + lngPack.i18n("Text~Punctuation~Colon");
		sText += iToStr (building.getOwner()->getCredits());
		return sText;
	}

	//Research centre idle + projects
	// Research Center
	if (building.getStaticData().canResearch && building.getOwner() == whoWantsToKnow && building.getOwner() && !building.isUnitWorking())
	{
		std::string sText = lngPack.i18n("Text~Comp~Waits") + "\n";
		for (int area = 0; area < cResearch::kNrResearchAreas; area++)
		{
			if (building.getOwner()->getResearchCentersWorkingOnArea((cResearch::ResearchArea)area) > 0)
			{
				switch (area)
				{
				case cResearch::kAttackResearch: sText += lngPack.i18n("Text~Others~Attack"); break;
				case cResearch::kShotsResearch: sText += lngPack.i18n("Text~Others~Shots_7"); break;
				case cResearch::kRangeResearch: sText += lngPack.i18n("Text~Others~Range"); break;
				case cResearch::kArmorResearch: sText += lngPack.i18n("Text~Others~Armor_7"); break;
				case cResearch::kHitpointsResearch: sText += lngPack.i18n("Text~Others~Hitpoints_7"); break;
				case cResearch::kSpeedResearch: sText += lngPack.i18n("Text~Others~Speed"); break;
				case cResearch::kScanResearch: sText += lngPack.i18n("Text~Others~Scan"); break;
				case cResearch::kCostResearch: sText += lngPack.i18n("Text~Others~Costs"); break;
				}
				sText += lngPack.i18n ("Text~Punctuation~Colon") + iToStr (building.getOwner()->getResearchState().getRemainingTurns (area, building.getOwner()->getResearchCentersWorkingOnArea ((cResearch::ResearchArea)area))) + "\n";
			}
		}
		return sText;
	}

	return lngPack.i18n ("Text~Comp~Waits");
}

//------------------------------------------------------------------------------
/** Returns a string with the current state */
//------------------------------------------------------------------------------
std::string getStatusStr (const cVehicle& vehicle, const cPlayer* player, const cUnitsData& unitsData)
{
	auto font = cUnicodeFont::font.get();
	if (vehicle.isDisabled())
	{
		std::string sText;
		sText = lngPack.i18n ("Text~Comp~Disabled") + " (";
		sText += iToStr (vehicle.getDisabledTurns()) + ")";
		return sText;
	}
	else if (vehicle.isSurveyorAutoMoveActive())
		return lngPack.i18n ("Text~Comp~Surveying");
	else if (vehicle.isUnitBuildingABuilding())
	{
		if (vehicle.getOwner() != player)
			return lngPack.i18n ("Text~Comp~Producing");
		else
		{
			std::string sText;
			if (vehicle.getBuildTurns())
			{
				sText = lngPack.i18n ("Text~Comp~Producing");
				sText += lngPack.i18n ("Text~Punctuation~Colon");
				sText += getStaticUnitName (unitsData.getStaticUnitData (vehicle.getBuildingType())) + " (";
				sText += iToStr (vehicle.getBuildTurns());
				sText += ")";

				if (font->getTextWide (sText) > 126)
				{
					sText = lngPack.i18n ("Text~Comp~Producing") + lngPack.i18n("Text~Punctuation~Colon");
					sText += "\n";
					sText += getStaticUnitName (unitsData.getStaticUnitData (vehicle.getBuildingType())) + " (";
					sText += iToStr (vehicle.getBuildTurns());
					sText += ")";
				}
				return sText;
			}
			else //small building is rdy + activate after engineere moves away
			{
				sText = lngPack.i18n ("Text~Comp~Producing_Fin");
				sText += lngPack.i18n ("Text~Punctuation~Colon");
				sText += getStaticUnitName (unitsData.getStaticUnitData (vehicle.getBuildingType()));

				if (font->getTextWide (sText) > 126)
				{
					sText = lngPack.i18n ("Text~Comp~Producing_Fin") + lngPack.i18n("Text~Punctuation~Colon");
					sText += "\n";
					sText += getStaticUnitName (unitsData.getStaticUnitData (vehicle.getBuildingType()));
				}
				return sText;
			}
		}
	}
	else if (vehicle.isUnitClearingMines())
		return lngPack.i18n ("Text~Comp~Clearing_Mine");
	else if (vehicle.isUnitLayingMines())
		return lngPack.i18n ("Text~Comp~Laying");
	else if (vehicle.isUnitClearing())
	{
		if (vehicle.getClearingTurns())
		{
			std::string sText;
			sText = lngPack.i18n ("Text~Comp~Clearing") + " (";
			sText += iToStr (vehicle.getClearingTurns()) + ")";
			return sText;
		}
		else
			return lngPack.i18n ("Text~Comp~Clearing_Fin");
	}
	// generate other infos for normal non-unit-related-events and infiltrators
	std::string sTmp;
	{
		if (vehicle.getMoveJob() && vehicle.getMoveJob()->getEndMoveAction().getType() == EMAT_ATTACK)
			sTmp = lngPack.i18n ("Text~Comp~MovingToAttack");
		else if (vehicle.getMoveJob())
			sTmp = lngPack.i18n ("Text~Comp~Moving");
		else if (vehicle.isAttacking())
			sTmp = lngPack.i18n ("Text~Comp~AttackingStatusStr");
		else if (vehicle.isBeeingAttacked())
			sTmp = lngPack.i18n ("Text~Comp~IsBeeingAttacked");
		else if (vehicle.isManualFireActive())
			sTmp = lngPack.i18n ("Text~Comp~ReactionFireOff");
		else if (vehicle.isSentryActive())
			sTmp = lngPack.i18n ("Text~Comp~Sentry");
		else sTmp = lngPack.i18n ("Text~Comp~Waits");

		// extra info only for infiltrators
		// TODO should it be original behavior (as it is now) or
		// don't display CommandRank for enemy (could also be a bug in original...?)
		if ((vehicle.getStaticData().canCapture || vehicle.getStaticData().canDisable) /* && vehicle.owner == gameGUI.getClient()->getActivePlayer()*/)
		{
			sTmp += "\n";
			sTmp += getRankString (vehicle.getCommandoData());
		}

		return sTmp;
	}

	return lngPack.i18n ("Text~Comp~Waits");
}

//------------------------------------------------------------------------------
std::string getStatusStr (const cUnit& unit, const cPlayer* whoWantsToKnow, const cUnitsData& unitData)
{
	if (const auto* vehicle = dynamic_cast<const cVehicle*> (&unit)) return getStatusStr (*vehicle, whoWantsToKnow, unitData);
	if (const auto* building = dynamic_cast<const cBuilding*> (&unit)) return getStatusStr (*building, whoWantsToKnow, unitData);
	return {};
}
