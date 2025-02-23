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

#include "uidata.h"

#include "game/data/units/building.h"
#include "game/data/units/vehicle.h"
#include "resources/buildinguidata.h"
#include "resources/vehicleuidata.h"

//------------------------------------------------------------------------------
// Globals

cGraphicsData GraphicsData;
cEffectsData EffectsData;
cResourceData ResourceData;
cUnitsUiData UnitsUiData;
cOtherData OtherData;

namespace
{
	constexpr SDL_Rect Rect_BigSymbol_Speed{0, 109, 11, 12};
	constexpr SDL_Rect Rect_BigSymbol_Hitpoints{11, 109, 7, 11};
	constexpr SDL_Rect Rect_BigSymbol_Ammo{18, 109, 9, 14};
	constexpr SDL_Rect Rect_BigSymbol_Attack{27, 109, 10, 14};
	constexpr SDL_Rect Rect_BigSymbol_Shots{37, 109, 15, 7};
	constexpr SDL_Rect Rect_BigSymbol_Range{52, 109, 13, 13};
	constexpr SDL_Rect Rect_BigSymbol_Armor{65, 109, 11, 14};
	constexpr SDL_Rect Rect_BigSymbol_Scan{76, 109, 13, 13};
	constexpr SDL_Rect Rect_BigSymbol_Metal{89, 109, 12, 15};
	constexpr SDL_Rect Rect_BigSymbol_Oil{101, 109, 11, 12};
	constexpr SDL_Rect Rect_BigSymbol_Gold{112, 109, 13, 10};
	constexpr SDL_Rect Rect_BigSymbol_Energy{125, 109, 13, 17};
	constexpr SDL_Rect Rect_BigSymbol_Human{138, 109, 12, 16};
	constexpr SDL_Rect Rect_BigSymbol_MetalEmpty{175, 109, 12, 15};
} // namespace

//------------------------------------------------------------------------------
sPartialSurface cGraphicsData::get_Slider (eSliderHandleType sliderHandleType) const
{
	switch (sliderHandleType)
	{
		case eSliderHandleType::Horizontal: return {gfx_menu_stuff.get(), {218, 35, 14, 17}};
		case eSliderHandleType::Vertical: return {gfx_menu_stuff.get(), {201, 35, 17, 14}};
		case eSliderHandleType::HudZoom: return {gfx_menu_stuff.get(), {314, 0, 25, 16}};
		case eSliderHandleType::ModernHorizontal: return {gfx_menu_stuff.get(), {241, 59, 8, 16}};
		case eSliderHandleType::ModernVertical: return {gfx_menu_stuff.get(), {224, 91, 16, 8}};
	}
	throw std::runtime_error ("Unknown enum eSliderHandleType: " + std::to_string (static_cast<int> (sliderHandleType)));
}

//------------------------------------------------------------------------------
SDL_Rect cGraphicsData::getSmallSymbolPosition (eUnitDataSymbolType symbolType)
{
	switch (symbolType)
	{
		case eUnitDataSymbolType::Speed: return getRect_SmallSymbol_Speed();
		case eUnitDataSymbolType::HitsGreen: return getRect_SmallSymbol_HitsGreen();
		case eUnitDataSymbolType::HitsOrange: return getRect_SmallSymbol_HitsOrange();
		case eUnitDataSymbolType::HitsRed: return getRect_SmallSymbol_HitsRed();
		case eUnitDataSymbolType::Ammo: return getRect_SmallSymbol_Ammo();
		case eUnitDataSymbolType::Shots: return getRect_SmallSymbol_Shots();
		case eUnitDataSymbolType::Metal: return getRect_SmallSymbol_Metal();
		case eUnitDataSymbolType::Oil: return getRect_SmallSymbol_Oil();
		case eUnitDataSymbolType::Gold: return getRect_SmallSymbol_Gold();
		case eUnitDataSymbolType::Energy: return getRect_SmallSymbol_Energy();
		case eUnitDataSymbolType::Human: return getRect_SmallSymbol_Human();
		case eUnitDataSymbolType::TransportTank: return getRect_SmallSymbol_TransportTank();
		case eUnitDataSymbolType::TransportAir: return getRect_SmallSymbol_TransportAir();
	}
	return {};
}

//------------------------------------------------------------------------------
sPartialSurface cGraphicsData::getBigSymbol (eUnitDataBigSymbolType symbolType) const
{
	switch (symbolType)
	{
		case eUnitDataBigSymbolType::Speed: return {gfx_hud_stuff.get(), Rect_BigSymbol_Speed};
		case eUnitDataBigSymbolType::Hits: return {gfx_hud_stuff.get(), Rect_BigSymbol_Hitpoints};
		case eUnitDataBigSymbolType::Ammo: return {gfx_hud_stuff.get(), Rect_BigSymbol_Ammo};
		case eUnitDataBigSymbolType::Attack: return {gfx_hud_stuff.get(), Rect_BigSymbol_Attack};
		case eUnitDataBigSymbolType::Shots: return {gfx_hud_stuff.get(), Rect_BigSymbol_Shots};
		case eUnitDataBigSymbolType::Range: return {gfx_hud_stuff.get(), Rect_BigSymbol_Range};
		case eUnitDataBigSymbolType::Armor: return {gfx_hud_stuff.get(), Rect_BigSymbol_Armor};
		case eUnitDataBigSymbolType::Scan: return {gfx_hud_stuff.get(), Rect_BigSymbol_Scan};
		case eUnitDataBigSymbolType::Metal: return {gfx_hud_stuff.get(), Rect_BigSymbol_Metal};
		case eUnitDataBigSymbolType::MetalEmpty: return {gfx_hud_stuff.get(), Rect_BigSymbol_MetalEmpty};
		case eUnitDataBigSymbolType::Oil: return {gfx_hud_stuff.get(), Rect_BigSymbol_Oil};
		case eUnitDataBigSymbolType::Gold: return {gfx_hud_stuff.get(), Rect_BigSymbol_Gold};
		case eUnitDataBigSymbolType::Energy: return {gfx_hud_stuff.get(), Rect_BigSymbol_Energy};
		case eUnitDataBigSymbolType::Human: return {gfx_hud_stuff.get(), Rect_BigSymbol_Human};
	}
	return {};
}

//------------------------------------------------------------------------------
cUnitsUiData::cUnitsUiData() :
	rubbleBig (std::make_unique<sBuildingUIData>()),
	rubbleSmall (std::make_unique<sBuildingUIData>())
{}

//------------------------------------------------------------------------------
cUnitsUiData::~cUnitsUiData()
{
}

//------------------------------------------------------------------------------
const sBuildingUIData* cUnitsUiData::getBuildingUI (sID id) const
{
	for (const auto& buildingUI : buildingUIs)
	{
		if (buildingUI.id == id)
			return &buildingUI;
	}
	return nullptr;
}

//------------------------------------------------------------------------------
const sBuildingUIData& cUnitsUiData::getBuildingUI (const cBuilding& building) const
{
	if (building.isRubble())
	{
		return building.getIsBig() ? *UnitsUiData.rubbleBig : *UnitsUiData.rubbleSmall;
	}
	return *getBuildingUI (building.getStaticUnitData().ID);
}

//------------------------------------------------------------------------------
const sVehicleUIData* cUnitsUiData::getVehicleUI (sID id) const
{
	for (const auto& vehicleUI : vehicleUIs)
	{
		if (vehicleUI.id == id)
			return &vehicleUI;
	}
	return nullptr;
}
