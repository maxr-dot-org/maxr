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

#ifndef game_logic_upgradecalculatorH
#define game_logic_upgradecalculatorH

#include "utility/serialization/nvp.h"
#include "utility/signal/signal.h"

#include <array>
#include <map>
#include <optional>

class cResearch;
class cUnitUpgrade;
class cDynamicUnitData;
class cStaticUnitData;

//------------------------------------------------------------------------------
/**
 * A singleton class for calculating costs for upgrades and research and
 * for getting the results of such upgrades and research.
 * In M.A.X. research and gold upgrades have no direct influence on each other.
 * Their effects are simply added.
 * Example:
 * The first step for the armor 10 upgrade is 5 gold
 * to get from armor 10 to 11.
 * If you research armor till you have 50% you will have armor 15.
 * But the first gold upgrade step will still cost 5 gold.
 * And it will go up from 15 to 16.
 * Other example:
 * If you have gold-upgraded the speed of an awac from 18 to 32 and
 * do now a speed research,
 * then you will still get only an additional bonus of 1
 * for the first research (and not 3).
 * This is because the research benefit is always calculated
 * on the basis of the start value and not of the current value.

 * To use this class, simply call:
 * cUpdateCalculator::instance().theMethodINeed()

 * @author Paul Grathwohl
 */
//------------------------------------------------------------------------------
class cUpgradeCalculator
{
public:
	static cUpgradeCalculator& instance();

	enum class eUpgradeType
	{
		Hitpoints = 0,
		Armor,
		Ammo,
		Attack,
		Speed,
		Shots,
		Range,
		Scan,
		Cost
	};

	/**
	 * Calculates the price (gold) to upgrade from the given value.
	 * @param curValue the value the unit currently has
	 *                 (without boni by research!)
	 * @param orgValue the value the unit has as a base value
	 * @param upgradeType the area of the upgrade
	 * @param researchLevel the research level of the player
	 *                      that has to be taken into account
	 * @return the costs for this upgrade if available
	 */
	std::optional<int> calcPrice (int curValue, int orgValue, eUpgradeType, const cResearch&) const;

	/**
	 * Calculates the increase of a unit value, when an upgrade is bought.
	 * Examples: If orgValue is 10, the increase will be 2.
	 * If orgValue is 28, the increase will be 5.
	 * The increase is not growing, if the unit has already some upgrades!
	 * The only needed thing for the calculation is the value,
	 * at which the unit started.
	 * @param startValue the value, the unit has in it's base version
	 * @return the increase of the unit's value, when an upgrade is bought
	*/
	int calcIncreaseByUpgrade (int startValue) const;

	/**
	 * Calculates the price (gold) for upgrading a unit,
	 * that started with orgValue and has currently curValue, to newValue.
	 * @param orgValue the value the unit has as a base value
	 * @param curValue the value the unit currently has
	 * @param newValue the value the unit wants to reach
	 * @upgradeType the area of the upgrade
	 * @param researchLevel the research level of the player
	 *                      that has to be taken into account
	 * @return the costs for this upgrade if available
	 */
	std::optional<int> getCostForUpgrade (int orgValue, int curValue, int newValue, eUpgradeType, const cResearch&) const;

	/**
	 * Calculates the turns needed for one research center
	 * to reach the next level.
	 * @param curResearchLevel the level this research area currently has
	 *                         (e.g. 20 for 20%)
	 * @param upgradeType the area of the upgrade
	 * @return the turns needed to reach the next level
	 *         with one research center
	 *         or std::nullopt if the passed values are out of range
	 */
	std::optional<int> calcResearchTurns (int curResearchLevel, eUpgradeType) const;

	/**
	 * Calculates the raw-material needed for upgrading a unit,
	 * that costs unitCost, to the current version.
	 * The costs in original M.A.X. are simply
	 * a fourth of the costs (rounded down) needed to build that unit.
	 * The costs do not depend on the quality of the upgrade
	 * (e.g. upgrading hitpoints from 18 to 20 costs the same as
	 * upgrading the basic version of the unit to an ultra fat version
	 * with all values upgraded to a maximum).
	 * @param unitCost the raw-material cost to build the unit
	 *                 that will be upgraded (e.g. 24 for a mine-building)
	 * @return the raw-material needed to upgrade to the current version
	 */
	int getMaterialCostForUpgrading (int unitCost) const;

	enum class eUnitType
	{
		Building = 0, // Mines, Research Centers, Storage, Generators...
		Infantry, // Infantry and Infiltrator
		StandardUnit // all other, like Tank, Ground Attack Plane, Scanner...
	};

	/**
	 * Calculates the change of the given startValue,
	 * with the given researchLevel.
	 * This change is independent of the upgradeType,
	 * only kCost has a special handling,
	 * because it actually decreases the value
	 * (so you will get a negative value as return value).
	 * @param startValue the value, the unit has in it's base version
	 * @param curResearchLevel the level for which you want to know the change
	 *                         (e.g. 10 for 10%)
	 * @param upgradeType optional, set it to kCost
	 *                    if you need to know the changes in cost
	 * @param unitType optional, needed for upgradeType kCost because
	 *                           the behaviour changes for the unit types
	 * @return the change of the startValue
	 * (can be negative if kCost is the upgradeType)
	 */
	int calcChangeByResearch (int startValue, int curResearchLevel, std::optional<eUpgradeType> = std::nullopt, eUnitType = eUnitType::Building) const;

	/**
	 * Prints some upgrade values to the standard log on debug-level.
	 * Expand the implementation to test, if all works fine.
	 */
	void printAllToLog() const;

private:
	cUpgradeCalculator();

	using PriceMap = std::map<int, int>;

	PriceMap hitpointsArmorAmmo_2;
	PriceMap hitpointsArmorAmmo_4;
	PriceMap hitpointsArmorAmmo_6;
	PriceMap hitpointsArmorAmmo_7;
	PriceMap hitpointsArmorAmmo_8;
	PriceMap hitpointsArmorAmmo_9;
	PriceMap hitpointsArmorAmmo_10;
	PriceMap hitpointsArmorAmmo_12;
	PriceMap hitpointsArmorAmmo_14;
	PriceMap hitpointsArmorAmmo_16;
	PriceMap hitpointsArmorAmmo_18;
	PriceMap hitpointsArmorAmmo_20;
	PriceMap hitpointsArmorAmmo_24;
	PriceMap hitpointsArmorAmmo_26;
	PriceMap hitpointsArmorAmmo_28;
	PriceMap hitpointsArmorAmmo_32;
	PriceMap hitpointsArmorAmmo_36;
	PriceMap hitpointsArmorAmmo_40;
	PriceMap hitpointsArmorAmmo_56;

	PriceMap attackSpeed_5;
	PriceMap attackSpeed_6;
	PriceMap attackSpeed_7;
	PriceMap attackSpeed_8;
	PriceMap attackSpeed_9;
	PriceMap attackSpeed_10;
	PriceMap attackSpeed_11;
	PriceMap attackSpeed_12;
	PriceMap attackSpeed_14;
	PriceMap attackSpeed_15;
	PriceMap attackSpeed_16;
	PriceMap attackSpeed_17;
	PriceMap attackSpeed_18;
	PriceMap attackSpeed_20;
	PriceMap attackSpeed_22;
	PriceMap attackSpeed_24;
	PriceMap attackSpeed_28;
	PriceMap attackSpeed_30;
	PriceMap attackSpeed_36;

	PriceMap rangeScan_3;
	PriceMap rangeScan_4;
	PriceMap rangeScan_5;
	PriceMap rangeScan_6;
	PriceMap rangeScan_7;
	PriceMap rangeScan_8;
	PriceMap rangeScan_9;
	PriceMap rangeScan_10;
	PriceMap rangeScan_11;
	PriceMap rangeScan_12;
	PriceMap rangeScan_14;
	PriceMap rangeScan_16;
	PriceMap rangeScan_18;
	PriceMap rangeScan_20;
	PriceMap rangeScan_24;

	PriceMap shots_1;
	PriceMap shots_2;

	std::optional<int> lookupPrice (const PriceMap&, int value) const;

	int getNearestPossibleCost (float realCost, int costDifference) const;

	void printToLog (const char* str, int value = -1000) const;
};

//-------------------------------------------
/** Stores the current research state of a player. */
//-------------------------------------------
class cResearch
{
public:
	static constexpr std::size_t kNrResearchAreas = 8;
	enum class eResearchArea
	{
		AttackResearch = 0,
		ShotsResearch,
		RangeResearch,
		ArmorResearch,
		HitpointsResearch,
		SpeedResearch,
		ScanResearch,
		CostResearch
	};

	cResearch();

	/**
	 * Adds researchPoints to the current research points of
	 * the specified researchArea.
	 * @return true, if the next research level was reached
	 */
	bool doResearch (int researchPoints, eResearchArea);

	int getCurResearchLevel (eResearchArea) const; ///< 0, 10, 20, 30, ...

	/// returns the needed number of turns to reach the next level
	/// with the given nr of research centers
	int getRemainingTurns (eResearchArea, int centersWorkingOn) const;

	cUpgradeCalculator::eUpgradeType getUpgradeCalculatorUpgradeType (eResearchArea) const;
	std::optional<cResearch::eResearchArea> getResearchArea (cUpgradeCalculator::eUpgradeType) const;

	uint32_t getChecksum (uint32_t crc) const;

	cSignal<void (eResearchArea)> currentResearchPointsChanged;
	cSignal<void (eResearchArea)> neededResearchPointsChanged;

	template <typename Archive>
	void serialize (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (curResearchLevel);
		archive & NVP (curResearchPoints);
		archive & NVP (neededResearchPoints);
		// clang-format on
	}
	//-------------------------------------------
private:
	std::array<int, kNrResearchAreas> curResearchLevel; ///< 0, 10, 20, 30, ...
	/// Number of research-center turns the player invested in an area
	std::array<int, kNrResearchAreas> curResearchPoints;
	/// Number of research-center turns needed to reach the next level
	// (remainingResearchPoints == neededResearchPoints - curResearchPoints)
	std::array<std::optional<int>, kNrResearchAreas> neededResearchPoints;
};

/**
 * A struct that contains information about the upgrades of a unit.
 *@author alzi
 */
struct sUnitUpgrade
{
	sUnitUpgrade() = default;

	int purchase (const cResearch&);
	int cancelPurchase (const cResearch&);
	int computedPurchasedCount (const cResearch&);

	/** The different values of a unit that can be upgraded */
	enum class eUpgradeType
	{
		Damage,
		Shots,
		Range,
		Ammo,
		Armor,
		Hits,
		Scan,
		Speed,
		None
	};

	int getCurValue() const { return curValue; }
	eUpgradeType getType() const { return type; }
	std::optional<int> getNextPrice() const { return nextPrice; }
	int getPurchased() const { return purchased; }

	template <typename Archive>
	void serialize (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (nextPrice);
		archive & NVP (purchased);
		archive & NVP (curValue);
		archive & NVP (startValue);
		archive & NVP (type);
		// clang-format on
	}

private:
	friend class cUnitUpgrade;
	/** what will the next upgrade cost */
	std::optional<int> nextPrice = 0;
	/** how many upgrades of this type has the player purchased */
	int purchased = 0;
	/** what is the current value */
	int curValue = -1;
	/** the value that this unit would have without all upgrades */
	int startValue = 0;
	/** the type of the upgrade */
	eUpgradeType type = eUpgradeType::None;
};

class cUnitUpgrade
{
public:
	void init (const cDynamicUnitData& origData, const cDynamicUnitData& curData, const cStaticUnitData&, const cResearch&);
	sUnitUpgrade* getUpgrade (sUnitUpgrade::eUpgradeType);
	const sUnitUpgrade* getUpgrade (sUnitUpgrade::eUpgradeType) const;

	int computedPurchasedCount (const cResearch&);
	bool hasBeenPurchased() const;
	int getValueOrDefault (sUnitUpgrade::eUpgradeType, int defaultValue) const;
	void updateUnitData (cDynamicUnitData&) const;
	int calcTotalCosts (const cDynamicUnitData& originalData, const cDynamicUnitData& currentData, const cResearch&) const;

	template <typename Archive>
	void serialize (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (upgrades);
		// clang-format on
	}

public:
	std::array<sUnitUpgrade, 8> upgrades;
};

#endif // game_logic_upgradecalculatorH
