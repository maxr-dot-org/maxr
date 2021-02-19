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

#include "defines.h"
#include "game/data/miningresource.h"
#include "game/data/units/unitdata.h" // for sUnitData, sID
#include "game/data/units/unit.h"
#include "game/logic/upgradecalculator.h" // cResearch::ResearchArea
#include "resources/sound.h"
#include "resources/uidata.h"
#include "utility/signal/signal.h"
#include "utility/signal/signalconnectionmanager.h"

#include <SDL.h>

#include <array>
#include <vector>

class cBase;
class cPlayer;
class cVehicle;
class cMap;
class cMapField;
class cSubBase;
class cCrossPlattformRandom;

//--------------------------------------------------------------------------
/** struct for the images and sounds */
//--------------------------------------------------------------------------
struct sBuildingUIData
{
	sID id;

	bool hasClanLogos;
	bool hasDamageEffect;
	bool hasBetonUnderground;
	bool hasPlayerColor;
	bool hasOverlay;

	bool buildUpGraphic;
	bool powerOnGraphic;
	bool isAnimated;

	bool isConnectorGraphic;
	int hasFrames;


	AutoSurface img, img_org; // Surface of the building
	AutoSurface shw, shw_org; // Surfaces of the shadow
	AutoSurface eff, eff_org; // Surfaces of the effects
	AutoSurface video;  // video
	AutoSurface info;   // info image

	// Die Sounds:
	cSoundChunk Start;
	cSoundChunk Running;
	cSoundChunk Stop;
	cSoundChunk Attack;
	cSoundChunk Wait;

	sBuildingUIData();
	sBuildingUIData (sBuildingUIData&& other);
	sBuildingUIData& operator= (sBuildingUIData && other);
	void scaleSurfaces (float faktor);

private:
	sBuildingUIData (const sBuildingUIData& other) = delete;
	sBuildingUIData& operator= (const sBuildingUIData& other) = delete;
};

// enum for the upgrade symbols
#ifndef D_eSymbols
#define D_eSymbols
enum eSymbols {SSpeed, SHits, SAmmo, SMetal, SEnergy, SShots, SOil, SGold, STrans, SHuman, SAir};
enum eSymbolsBig {SBSpeed, SBHits, SBAmmo, SBAttack, SBShots, SBRange, SBArmor, SBScan, SBMetal, SBOil, SBGold, SBEnergy, SBHuman};
#endif

//--------------------------------------------------------------------------
/** struct for the building order list */
//--------------------------------------------------------------------------
class cBuildListItem
{
public:
	cBuildListItem();
	cBuildListItem (sID type, int remainingMetal);
	cBuildListItem (const cBuildListItem& other);
	cBuildListItem (cBuildListItem&& other);

	cBuildListItem& operator= (const cBuildListItem& other);
	cBuildListItem& operator= (cBuildListItem && other);

	const sID& getType() const;
	void setType (const sID& type);

	int getRemainingMetal() const;
	void setRemainingMetal (int value);

	uint32_t getChecksum(uint32_t crc) const;

	cSignal<void ()> typeChanged;
	cSignal<void ()> remainingMetalChanged;

	template<typename T>
	void serialize(T& archive)
	{
		archive & NVP(type);
		archive & NVP(remainingMetal);
	}
private:
	sID type;
	int remainingMetal;
};

//--------------------------------------------------------------------------
/** Class cBuilding for one building. */
//--------------------------------------------------------------------------
class cBuilding : public cUnit
{
public:
	friend class cDebugOutputWidget;

	cBuilding(const cStaticUnitData* staticData, const cDynamicUnitData* data, cPlayer* Owner, unsigned int ID);
	virtual ~cBuilding();

	bool isAVehicle() const override { return false; }
	bool isABuilding() const override { return true; }
	bool isRubble() const { return rubbleValue > 0; }


	int playStream();
	std::string getStatusStr (const cPlayer* player, const cUnitsData& unitsData) const override;

	/**
	* refreshes the shotsCur of this building
	*@author alzi alias DoctorDeath
	*@return 1 if there has been refreshed something, else 0.
	*/
	bool refreshData();
	void DrawSymbolBig (eSymbolsBig sym, int x, int y, int maxx, int value, int orgvalue, SDL_Surface* sf);
	void updateNeighbours (const cMap& map);
	void CheckNeighbours (const cMap& Map);

	void startWork ();
	void stopWork (bool forced = false);

	/** check whether a transfer to a unit on the field is possible */
	bool canTransferTo (const cPosition& position, const cMapView& map) const override;
	bool canTransferTo (const cUnit& unit) const override;
	void initMineRessourceProd (const cMap& map);
	void calcTurboBuild (std::array<int, 3>& turboBuildRounds, std::array<int, 3>& turboBuildCosts, int vehicleCosts, int remainingMetal = -1) const;
	bool canExitTo (const cPosition& position, const cMap& map, const cStaticUnitData& unitData) const override;
	bool canExitTo (const cPosition& position, const cMapView& map, const cStaticUnitData& vehicleData) const override;
	bool canLoad(const cPosition& position, const cMapView& map, bool checkPosition = true) const;
	bool canLoad (const cVehicle* Vehicle, bool checkPosition = true) const override;
	bool canSupply(const cUnit* unit, eSupplyType supplyType) const override;

	/**
	* draws the main image of the building onto the given surface
	*/
	void render (unsigned long long animationTime, SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, bool drawShadow, bool drawConcrete) const;
	void render_simple(SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, unsigned long long animationTime = 0, int alpha = 254) const;
	static void render_simple (SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, const sBuildingUIData& uiData, const cPlayer* owner, int frameNr = 0, int alpha = 254);

	bool isUnitWorking() const override { return isWorking; }
	bool factoryHasJustFinishedBuilding() const override;
	bool buildingCanBeStarted() const override;
	bool buildingCanBeUpgraded() const override;
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
	void setBuildSpeed(int value);
	void setMetalPerRound(int value);
	void setRepeatBuild(bool value);

	const sMiningResource& getMaxProd() const;

	void setResearchArea (cResearch::ResearchArea area);
	cResearch::ResearchArea getResearchArea() const;

	void setRubbleValue(int value, cCrossPlattformRandom& randomGenerator);
	int getRubbleValue() const;

	uint32_t getChecksum(uint32_t crc) const override;

	cSignal<void ()> buildListChanged;
	cSignal<void ()> buildListFirstItemDataChanged;
	cSignal<void ()> researchAreaChanged;

	cSignal<void()> buildSpeedChanged;
	cSignal<void()> metalPerRoundChanged;
	cSignal<void()> repeatBuildChanged;

	template <typename T>
	void serialize(T& archive)
	{
		cUnit::serializeThis(archive); //serialize cUnit members

		archive & NVP(rubbleTyp);
		archive & NVP(rubbleValue);
		archive & NVP(BaseN);
		archive & NVP(BaseE);
		archive & NVP(BaseS);
		archive & NVP(BaseW);
		archive & NVP(BaseBN);
		archive & NVP(BaseBE);
		archive & NVP(BaseBS);
		archive & NVP(BaseBW);
		archive & serialization::makeNvp("maxMetalProd", maxProd.metal);
		archive & serialization::makeNvp("maxOilProd", maxProd.oil);
		archive & serialization::makeNvp("maxGoldProd", maxProd.gold);
		archive & serialization::makeNvp("metalProd", prod.metal);
		archive & serialization::makeNvp("oilProd", prod.oil);
		archive & serialization::makeNvp("goldProd", prod.gold);
		archive & NVP(buildSpeed);
		archive & NVP(metalPerRound);
		archive & NVP(repeatBuild);
		archive & NVP(wasWorking);
		archive & NVP(points);
		archive & NVP(isWorking);
		archive & NVP(researchArea);
		archive & NVP(buildList);

		if (!archive.isWriter)
		{
			if (isRubble())
			{
				if (isBig)
				{
					uiData = UnitsUiData.rubbleBig;
					staticData = archive.getPointerLoader()->getBigRubbleData();
				}
				else
				{
					uiData = UnitsUiData.rubbleSmall;
					staticData = archive.getPointerLoader()->getSmallRubbleData();
				}
			}
			else
			{
				uiData = UnitsUiData.getBuildingUI(data.getId());
			}
			registerOwnerEvents();
			connectFirstBuildListItem();
		}
	}

private:
	cSignalConnectionManager buildListFirstItemSignalConnectionManager;
	cSignalConnectionManager ownerSignalConnectionManager;

	/**
	* draws the connectors onto the given surface
	*/
	void drawConnectors (SDL_Surface* surface, SDL_Rect dest, float zoomFactor, bool drawShadow) const;

	void render_rubble (SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor, bool drawShadow) const;
	void render_beton (SDL_Surface* surface, const SDL_Rect& dest, float zoomFactor) const;
	void connectFirstBuildListItem();

	void registerOwnerEvents();

public:
	const sBuildingUIData* uiData = nullptr;
	mutable int effectAlpha; // alpha value for the effect
	bool BaseN, BaseE, BaseS, BaseW; // is the building connected in this direction?
	bool BaseBN, BaseBE, BaseBS, BaseBW; // is the building connected in this direction (only for big buildings)
	cSubBase* subBase = nullptr;     // the subbase to which this building belongs
	sMiningResource prod;          // production settings (from mine allocation menu)

	int DamageFXPointX, DamageFXPointY, DamageFXPointX2, DamageFXPointY2; // the points, where smoke will be generated when the building is damaged
	/** true if the building was has been working before it was disabled */
	bool wasWorking;
	int points;     // accumulated eco-sphere points
private:
	bool isWorking;  // is the building currently working?

	int buildSpeed;
	int metalPerRound;
	bool repeatBuild;

	sMiningResource maxProd; // the maximum possible production of the building (resources under the building)

	int rubbleTyp;     // type of the rubble graphic (when unit is rubble)
	int rubbleValue;   // number of resources in the rubble field

	cResearch::ResearchArea researchArea; ///< if the building can research, this is the area the building last researched or is researching

	std::vector<cBuildListItem> buildList; // list with the units to be build by this factory
};

#endif // game_data_units_buildingH
