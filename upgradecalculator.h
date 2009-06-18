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

#ifndef upgradecalculatorH
#define upgradecalculatorH

#include <map>

class cResearch;

//-------------------------------------------------------------------------------
/**	A singleton class for calculating costs for upgrades and research and for 
	getting the	results of such upgrades and research.
	In M.A.X. research and gold upgrades have no direct influence on each other. 
	Their effects are simply added. 
	Example: 
	The first step for the armor 10 upgrade is 5 gold to get from armor 10 to 11.
	If you research armor till you have 50% you will have armor 15. But the first
	gold upgrade step will still cost 5 gold. And it will go up from 15 to 16.
	Other example:
	If you have gold-upgraded the speed of an awac from 18 to 32 and do now
	a speed research, then you will still get only an additional bonus of 1
	for the first research (and not 3). This is because the research benefit 
	is	always calculated on the basis of the start value and not of the current 
	value.
 
	To use this class, simply call: cUpdateCalculator::instance().theMethodINeed()
 
	@author Paul Grathwohl 
*/
//-------------------------------------------------------------------------------
class cUpgradeCalculator {
public:
	static cUpgradeCalculator& instance();

	enum UpgradeTypes {
		kHitpoints = 0,
		kArmor,
		kAmmo,
		kAttack,
		kSpeed,
		kShots,
		kRange,
		kScan,
		kCost
	};

	enum {
		kNoPriceAvailable = 0,
		kNoResearchAvailable = 66666
	};

	/** Calculates the price (gold) to upgrade from the given value.
	@param curValue the value the unit currently has (without boni by research!)
	@param orgValue the value the unit has as a base value
	@param upgradeType the area of the upgrade
	@param researchLevel the research level of the player that has to be taken into account
	@return the costs for this upgrade or kNoPriceAvailable if the values are unknown */
	int calcPrice(int curValue, int orgValue, int upgradeType, cResearch& researchLevel) const;

	/** Calculates the increase of a unit value, when an upgrade is bought.
	    Examples: If orgValue is 10, the increase will be 2. 
				  If orgValue is 28, the increase will be 5.
	    The increase is not growing, if the unit has already some upgrades! The only
		needed thing for the calculation is the value, at which the unit started.
		@param startValue the value, the unit has in it's base version
		@return the increase of the unit's value, when an upgrade is bought
	*/
	int calcIncreaseByUpgrade(int startValue) const;

	/** Calculates the price (gold) for upgrading a unit, that started with orgValue and has 
		currently curValue, to newValue.
	 @param orgValue the value the unit has as a base value
	 @param curValue the value the unit currently has
	 @param newValue the value the unit wants to reach
	 @upgradeType the area of the upgrade
	 @param researchLevel the research level of the player that has to be taken into account
	 @return the costs for this upgrade or kNoPriceAvailable if such an upgrade is impossible
	*/
	int getCostForUpgrade(int orgValue, int curValue, int newValue, int upgradeType, cResearch& researchLevel) const;
	
	/** Calculates the turns needed for one research center to reach the next level.
	 @param curResearchLevel the level this research area currently has (e.g. 20 for 20%) 
	 @param upgradeType the area of the upgrade
	 @return the turns needed to reach the next level with one research center or
			 kNoResearchAvailable if the passed values are out of range */
	int calcResearchTurns(int curResearchLevel, int upgradeType) const;

	/** Calculates the raw-material needed for upgrading a unit, that costs unitCost, to the current version.
		The costs in original M.A.X. are simply a fourth of the costs (rounded down) needed to build that unit.
		The costs do not depend on the quality of the upgrade (e.g. upgrading hitpoints from 18 to 20 costs the 
		same as upgrading the basic version of the unit to an ultra fat version with all values upgraded to a
		maximum).
	 @param unitCost the raw-material cost to build the unit that will be upgraded (e.g. 24 for a mine-building)
	 @return the raw-material needed to upgrade to the current version */
	int getMaterialCostForUpgrading(int unitCost) const;
	
	enum UnitTypes {
		kBuilding = 0, // Mines, Research Centers, Storage, Generators, ...
		kInfantry, // Infantry and Infiltrator
		kStandardUnit // all other, like Tank, Ground Attack Plane, Scanner, ...
	};

	/** Calculates the change of the given startValue, with the given researchLevel.
		This change is independent of the upgradeType, only kCost has a special handling,
		because it actually decreases the value (so you will get a negative value as 
		return value).
		@param startValue the value, the unit has in it's base version
		@param curResearchLevel the level for which you want to know the change (e.g. 10 for 10%)
		@param upgradeType optional, set it to kCost if you need to know the changes in cost
		@param unitType optional, needed for upgradeType kCost because the 
						behaviour changes for the unit types
		@return the change of the startValue (can be negative if kCost is the upgradeType) */
	int calcChangeByResearch(int startValue, int curResearchLevel, 
							 int upgradeType=-1, int unitType=kBuilding) const;

	/** Prints some upgrade values to the standard log on debug-level. Expand the implementation
		to test, if all works fine. */
	void printAllToLog() const;


	//-------------------------------------------
private:
	cUpgradeCalculator();

	typedef std::map<int, int> PriceMap;

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

	int lookupPrice(const PriceMap& prices, int value) const;
	void setupLookupTables();

	int getNearestPossibleCost(double realCost, int costDifference) const;

	void printToLog(const char* str, int value=-1000) const;

	bool setupDone;
};



//-------------------------------------------
/** Stores the current research state of a player. */
//-------------------------------------------
class cResearch
{
public:
	enum ResearchArea {
		kAttackResearch = 0,
		kShotsResearch,
		kRangeResearch,
		kArmorResearch,
		kHitpointsResearch,
		kSpeedResearch,
		kScanResearch,
		kCostResearch,
		kNrResearchAreas
	};

	cResearch (); ///< constructor
		
	/** Adds researchPoints to the current research points of the specified researchArea.
		\return true, if the next research level was reached */
	bool doResearch (int researchPoints, int researchArea);

	int getCurResearchLevel (int researchArea) const; ///< 0, 10, 20, 30, ...
	int getCurResearchPoints (int researchArea) const; ///< Number of research-center turns the player invested in an area 
	int getNeededResearchPoints (int researchArea) const;  ///< Number of research-center turns needed to reach the next level
	int getRemainingResearchPoints (int researchArea) const { return getNeededResearchPoints (researchArea) - getCurResearchPoints (researchArea); }

	int getRemainingTurns (int researchArea, int centersWorkingOn) const; ///< returns the needed number of turns to reach the next level with the given nr of research centers 

	void setCurResearchLevel (int researchLevel, int researchArea); ///< will also set the neededResearchPoints if necessary
	void setCurResearchPoints (int researchPoints, int researchArea); ///< if researchPoints >= neededResearchPoints, nothing will be done
	
	int getUpgradeCalculatorUpgradeType (int researchArea) const;
	int getResearchArea (int upgradeCalculatorType) const;
	
//-------------------------------------------
protected:
	void init (); ///< sets all research information to the initial values

	int curResearchLevel[kNrResearchAreas]; ///< 0, 10, 20, 30, ...
	int curResearchPoints[kNrResearchAreas]; ///< Numberr of research-center turns the player invested in an area
	int neededResearchPoints[kNrResearchAreas]; ///< Number of research-center turns needed to reach the next level (remainingResearchPoints == neededResearchPoints - curResearchPoints)
};


#endif // upgradecalculatorH
