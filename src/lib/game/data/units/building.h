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

#ifndef game_data_units_buildingH
#define game_data_units_buildingH

#include "game/data/miningresource.h"
#include "game/data/units/unit.h"
#include "game/data/units/unitdata.h"
#include "game/logic/upgradecalculator.h" // cResearch::eResearchArea
#include "utility/signal/signal.h"
#include "utility/signal/signalconnectionmanager.h"

#include <array>
#include <vector>

class cBase;
class cCrossPlattformRandom;
class cMap;
class cMapField;
class cModel;
class cPlayer;
class cSubBase;
class cVehicle;

//--------------------------------------------------------------------------
/** struct for the building order list */
//--------------------------------------------------------------------------
class cBuildListItem
{
public:
	cBuildListItem() = default;
	cBuildListItem (sID type, int remainingMetal);
	cBuildListItem (const cBuildListItem&) noexcept;
	cBuildListItem (cBuildListItem&&) noexcept;

	cBuildListItem& operator= (const cBuildListItem&);
	cBuildListItem& operator= (cBuildListItem&&) noexcept;

	const sID& getType() const;
	void setType (const sID& type);

	int getRemainingMetal() const;
	void setRemainingMetal (int value);

	uint32_t getChecksum (uint32_t crc) const;

	cSignal<void()> typeChanged;
	cSignal<void()> remainingMetalChanged;

	template <typename Archive>
	void serialize (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (type);
		archive & NVP (remainingMetal);
		// clang-format on
	}

private:
	sID type;
	int remainingMetal = 0;
};

//--------------------------------------------------------------------------
/** Class cBuilding for one building. */
//--------------------------------------------------------------------------
class cBuilding : public cUnit
{
public:
	friend class cDebugOutputWidget;

	explicit cBuilding (unsigned int ID); // used by serialization
	cBuilding (const cStaticUnitData* staticData, const cDynamicUnitData* data, cPlayer* Owner, unsigned int ID);
	virtual ~cBuilding();

	bool isAVehicle() const override { return false; }
	bool isABuilding() const override { return true; }
	bool isRubble() const { return rubbleValue > 0; }

	const sStaticBuildingData& getStaticData() const { return getStaticUnitData().buildingData; }

	bool getIsBig() const override;

	void refreshData();
	void updateNeighbours (const cMap& map);
	void CheckNeighbours (const cMap& Map);

	void startWork();
	void stopWork (bool forced = false);

	/** check whether a transfer to a unit on the field is possible */
	bool canTransferTo (const cPosition& position, const cMapView& map) const override;
	bool canTransferTo (const cUnit& unit) const override;
	void initMineResourceProd (const cMap& map);
	void calcTurboBuild (std::array<int, 3>& turboBuildRounds, std::array<int, 3>& turboBuildCosts, int vehicleCosts, int remainingMetal = -1) const;
	bool canExitTo (const cPosition& position, const cMap& map, const cStaticUnitData& unitData) const override;
	bool canExitTo (const cPosition& position, const cMapView& map, const cStaticUnitData& vehicleData) const override;
	bool canLoad (const cPosition& position, const cMapView& map, bool checkPosition = true) const;
	bool canLoad (const cVehicle* Vehicle, bool checkPosition = true) const override;
	bool canSupply (const cUnit* unit, eSupplyType supplyType) const override;

	bool isUnitWorking() const { return isWorking; }
	bool factoryHasJustFinishedBuilding() const;
	bool buildingCanBeStarted() const;
	bool buildingCanBeUpgraded() const;
	bool canBeStoppedViaUnitMenu() const override { return isUnitWorking(); }

	bool isBuildListEmpty() const;
	size_t getBuildListSize() const;
	const cBuildListItem& getBuildListItem (size_t index) const;
	cBuildListItem& getBuildListItem (size_t index);
	void setBuildList (std::vector<cBuildListItem> buildList);
	void addBuildListItem (cBuildListItem item);
	void removeBuildListItem (size_t index);

	int getBuildSpeed() const;
	int getMetalPerRound() const;
	bool getRepeatBuild() const;

	void setWorking (bool value);
	void setBuildSpeed (int value);
	void setMetalPerRound (int value);
	void setRepeatBuild (bool value);

	const sMiningResource& getMaxProd() const;

	void setResearchArea (cResearch::eResearchArea area);
	cResearch::eResearchArea getResearchArea() const;

	void setRubbleValue (int value, cCrossPlattformRandom& randomGenerator);
	int getRubbleValue() const;

	const cPosition& getDamageFXPoint() const;
	const cPosition& getDamageFXPoint2() const;

	uint32_t getChecksum (uint32_t crc) const override;

	mutable cSignal<void()> workingChanged;
	cSignal<void()> buildListChanged;
	cSignal<void()> buildListFirstItemDataChanged;
	cSignal<void()> researchAreaChanged;

	cSignal<void()> buildSpeedChanged;
	cSignal<void()> metalPerRoundChanged;
	cSignal<void()> repeatBuildChanged;

	template <typename Archive>
	static std::unique_ptr<cBuilding> createFrom (Archive& archive)
	{
		int id;
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (id);
		// clang-format on
		auto res = std::make_unique<cBuilding> (id);
		res->serialize (archive);
		return res;
	}

	template <typename Archive>
	void serialize (Archive& archive)
	{
		cUnit::serializeThis (archive); //serialize cUnit members

		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (rubbleTyp);
		archive & NVP (rubbleValue);
		archive & NVP (BaseN);
		archive & NVP (BaseE);
		archive & NVP (BaseS);
		archive & NVP (BaseW);
		archive & NVP (BaseBN);
		archive & NVP (BaseBE);
		archive & NVP (BaseBS);
		archive & NVP (BaseBW);
		archive & serialization::makeNvp ("maxMetalProd", maxProd.metal);
		archive & serialization::makeNvp ("maxOilProd", maxProd.oil);
		archive & serialization::makeNvp ("maxGoldProd", maxProd.gold);
		archive & serialization::makeNvp ("metalProd", prod.metal);
		archive & serialization::makeNvp ("oilProd", prod.oil);
		archive & serialization::makeNvp ("goldProd", prod.gold);
		archive & NVP (buildSpeed);
		archive & NVP (metalPerRound);
		archive & NVP (repeatBuild);
		archive & NVP (wasWorking);
		archive & NVP (points);
		archive & NVP (isWorking);
		archive & NVP (researchArea);
		archive & NVP (buildList);
		// clang-format on
	}

	void postLoad (cModel& model);

private:
	void connectFirstBuildListItem();
	void registerOwnerEvents();

private:
	cSignalConnectionManager buildListFirstItemSignalConnectionManager;
	cSignalConnectionManager ownerSignalConnectionManager;

public:
	mutable int effectAlpha = 0; // alpha value for the effect
	int rubbleTyp = 0; // type of the rubble graphic (when unit is rubble)

	bool BaseN = false, BaseE = false, BaseS = false, BaseW = false; // is the building connected in this direction?
	bool BaseBN = false, BaseBE = false, BaseBS = false, BaseBW = false; // is the building connected in this direction (only for big buildings)
	cSubBase* subBase = nullptr; // the subbase to which this building belongs
	sMiningResource prod; // production settings (from mine allocation menu)

	/** true if the building was has been working before it was disabled */
	bool wasWorking = false;
	int points = 0; // accumulated eco-sphere points
private:
	bool isWorking = false; // is the building currently working?

	int buildSpeed = 0;
	int metalPerRound = 0;
	bool repeatBuild = false;

	sMiningResource maxProd; // the maximum possible production of the building (resources under the building)

	int rubbleValue = 0; // number of resources in the rubble field

	cResearch::eResearchArea researchArea = cResearch::eResearchArea::AttackResearch; ///< if the building can research, this is the area the building last researched or is researching

	std::vector<cBuildListItem> buildList; // list with the units to be build by this factory

	// Gui stuff
	mutable std::optional<cPosition> DamageFXPoint; // the points, where smoke will be generated when the building is damaged
	mutable std::optional<cPosition> DamageFXPoint2;
};

#endif // game_data_units_buildingH
