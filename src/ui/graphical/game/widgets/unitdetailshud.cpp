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

#include "ui/graphical/game/widgets/unitdetailshud.h"

#include "SDLutility/drawing.h"
#include "SDLutility/tosdl.h"
#include "game/data/gamesettings.h"
#include "game/data/player/player.h"
#include "game/data/units/building.h"
#include "game/data/units/unit.h"
#include "output/video/video.h"
#include "resources/uidata.h"
#include "ui/widgets/label.h"
#include "utility/language.h"

#include <cassert>

//------------------------------------------------------------------------------
cUnitDetailsHud::cUnitDetailsHud (const cBox<cPosition>& area, bool drawLines_) :
	cWidget (area),
	drawLines (drawLines_),
	unit (nullptr)
{
	const auto size = getSize();
	if (std::size_t (size.y()) < maxRows * rowHeight) resize (cPosition (getSize().x(), maxRows * rowHeight));

	for (size_t i = 0; i < maxRows; ++i)
	{
		amountLabels[i] = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (3, 2 + rowHeight * i), getPosition() + cPosition (3 + 35, 2 + rowHeight * i + rowHeight)), "", eUnicodeFontType::LatinSmallWhite, toEnumFlag (eAlignmentType::CenterHorizontal) | eAlignmentType::Bottom);
		nameLabels[i] = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (40, 2 + rowHeight * i), getPosition() + cPosition (40 + 40, 2 + rowHeight * i + rowHeight)), "", eUnicodeFontType::LatinSmallWhite, toEnumFlag (eAlignmentType::Left) | eAlignmentType::Bottom);
	}

	surface = AutoSurface (SDL_CreateRGBSurface (0, size.x(), size.y(), Video.getColDepth(), 0, 0, 0, 0));

	SDL_FillRect (surface.get(), nullptr, 0xFF00FF);
	SDL_SetColorKey (surface.get(), SDL_TRUE, 0xFF00FF);
}

//------------------------------------------------------------------------------
void cUnitDetailsHud::setUnit (const cUnit* unit_)
{
	unit = unit_;
}

//------------------------------------------------------------------------------
void cUnitDetailsHud::setPlayer (const cPlayer* player_)
{
	player = player_;
}

//------------------------------------------------------------------------------
void cUnitDetailsHud::setGameSettings (std::shared_ptr<const cGameSettings> gameSettings_)
{
	gameSettings = std::move (gameSettings_);
}

//------------------------------------------------------------------------------
void cUnitDetailsHud::draw (SDL_Surface& destination, const cBox<cPosition>& clipRect)
{
	if (surface != nullptr)
	{
		reset();

		blitClipped (*surface, getArea(), destination, clipRect);
	}

	cWidget::draw (destination, clipRect);
}

//------------------------------------------------------------------------------
void cUnitDetailsHud::reset()
{
	SDL_FillRect (surface.get(), nullptr, 0xFF00FF);
	SDL_SetColorKey (surface.get(), SDL_TRUE, 0xFF00FF);

	for (std::size_t i = 0; i < maxRows; ++i)
	{
		if (drawLines)
		{
			SDL_Rect lineRect = {2, int (14 + 12 * i), getSize().x(), 1};
			SDL_FillRect (surface.get(), &lineRect, 0xFF743904);
		}

		amountLabels[i]->hide();
		nameLabels[i]->hide();
	}

	if (unit == nullptr) return;

	const auto& data = unit->data;
	const auto& staticData = unit->getStaticUnitData();

	drawRow (0, eUnitDataSymbolType::Hits, data.getHitpoints(), data.getHitpointsMax(), lngPack.i18n ("Others~Hitpoints_7"));

	if (data.getSpeedMax() > 0) drawRow (2, eUnitDataSymbolType::Speed, data.getSpeed() / 4, data.getSpeedMax() / 4, lngPack.i18n ("Others~Speed_7"));

	if (staticData.buildingData.canScore && unit->getOwner())
	{
		assert (unit->isABuilding()); // currently only buildings can score
		const auto unitScore = static_cast<const cBuilding*> (unit)->points;
		const auto totalScore = unit->getOwner()->getScore();
		const auto goalScore = (gameSettings && gameSettings->victoryConditionType == eGameSettingsVictoryCondition::Points) ? gameSettings->victoryPoints : totalScore;

		drawRow (1, eUnitDataSymbolType::Human, unitScore, unitScore, lngPack.i18n ("Others~Score"));
		drawRow (2, eUnitDataSymbolType::Human, totalScore, goalScore, lngPack.i18n ("Others~Total"));
	}
	else if ((staticData.storeResType != eResourceType::None || staticData.storageUnitsMax > 0) && unit->getOwner() == player)
	{
		if (staticData.storageResMax > 0)
		{
			eUnitDataSymbolType symbolType;
			switch (staticData.storeResType)
			{
				case eResourceType::Metal:
					symbolType = eUnitDataSymbolType::Metal;
					break;
				case eResourceType::Oil:
					symbolType = eUnitDataSymbolType::Oil;
					break;
				case eResourceType::Gold:
					symbolType = eUnitDataSymbolType::Gold;
					break;
				case eResourceType::None: break;
			}

			drawRow (1, symbolType, unit->getStoredResources(), staticData.storageResMax, lngPack.i18n ("Others~Cargo_7"));

			if (unit->isABuilding())
			{
				const auto& building = static_cast<const cBuilding&> (*unit);
				if (!building.subBase) return;
				const sMiningResource& stored = building.subBase->getResourcesStored();
				const sMiningResource& maxStored = building.subBase->getMaxResourcesStored();

				switch (staticData.storeResType)
				{
					case eResourceType::Metal:
						drawRow (2, symbolType, stored.metal, maxStored.metal, lngPack.i18n ("Others~Total"));
						break;
					case eResourceType::Oil:
						drawRow (2, symbolType, stored.oil, maxStored.oil, lngPack.i18n ("Others~Total"));
						break;
					case eResourceType::Gold:
						drawRow (2, symbolType, stored.gold, maxStored.gold, lngPack.i18n ("Others~Total"));
						break;
					case eResourceType::None: break;
				}
			}
		}
		else if (staticData.storageUnitsImageType != eStorageUnitsImageType::None)
		{
			eUnitDataSymbolType symbolType;
			switch (staticData.storageUnitsImageType)
			{
				case eStorageUnitsImageType::Tank:
				case eStorageUnitsImageType::Ship:
					symbolType = eUnitDataSymbolType::TransportTank;
					break;
				case eStorageUnitsImageType::Plane:
					symbolType = eUnitDataSymbolType::TransportAir;
					break;
				case eStorageUnitsImageType::Human:
					symbolType = eUnitDataSymbolType::Human;
					break;
				case eStorageUnitsImageType::None: break;
			}

			drawRow (1, symbolType, unit->storedUnits.size(), staticData.storageUnitsMax, lngPack.i18n ("Others~Cargo_7"));
		}
	}
	else if (staticData.canAttack && (staticData.ID.isAVehicle() || !staticData.buildingData.explodesOnContact))
	{
		if (unit->getOwner() == player) drawRow (1, eUnitDataSymbolType::Ammo, data.getAmmo(), data.getAmmoMax(), lngPack.i18n ("Others~Ammo_7"));

		drawRow (3, eUnitDataSymbolType::Shots, data.getShots(), data.getShotsMax(), lngPack.i18n ("Others~Shots_7"));
	}
	else if (staticData.produceEnergy && unit->isABuilding())
	{
		const auto& building = static_cast<const cBuilding&> (*unit);
		drawRow (1, eUnitDataSymbolType::Energy, (building.isUnitWorking() ? staticData.produceEnergy : 0), staticData.produceEnergy, lngPack.i18n ("Others~Power"));

		if (unit->getOwner() == player)
		{
			drawRow (2, eUnitDataSymbolType::Energy, building.subBase->getEnergyProd(), building.subBase->getMaxEnergyProd(), lngPack.i18n ("Others~Total"));
			drawRow (3, eUnitDataSymbolType::Energy, building.subBase->getEnergyNeed(), building.subBase->getMaxEnergyNeed(), lngPack.i18n ("Others~Usage_7"));
		}
	}
	else if (staticData.produceHumans && unit->isABuilding())
	{
		const auto& building = static_cast<const cBuilding&> (*unit);
		drawRow (1, eUnitDataSymbolType::Human, staticData.produceHumans, staticData.produceHumans, lngPack.i18n ("Others~Teams_7"));

		if (unit->getOwner() == player)
		{
			drawRow (2, eUnitDataSymbolType::Human, building.subBase->getHumanProd(), building.subBase->getHumanProd(), lngPack.i18n ("Others~Total"));
			drawRow (3, eUnitDataSymbolType::Human, building.subBase->getHumanNeed(), building.subBase->getMaxHumanNeed(), lngPack.i18n ("Others~Usage_7"));
		}
	}
	else if (staticData.needsHumans && unit->isABuilding())
	{
		const auto& building = static_cast<const cBuilding&> (*unit);
		if (building.isUnitWorking())
			drawRow (1, eUnitDataSymbolType::Human, staticData.needsHumans, staticData.needsHumans, lngPack.i18n ("Others~Usage_7"));
		else
			drawRow (1, eUnitDataSymbolType::Human, 0, staticData.needsHumans, lngPack.i18n ("Others~Usage_7"));

		if (unit->getOwner() == player) drawRow (2, eUnitDataSymbolType::Human, building.subBase->getHumanNeed(), building.subBase->getMaxHumanNeed(), lngPack.i18n ("Others~Total"));
	}
}

//------------------------------------------------------------------------------
void cUnitDetailsHud::drawRow (size_t index, eUnitDataSymbolType symbolType, int amount, int maximalAmount, const std::string& name)
{
	if (index >= maxRows) return;

	amountLabels[index]->show();
	nameLabels[index]->show();

	eUnicodeFontType fontType;
	if (amount > maximalAmount / 2)
		fontType = eUnicodeFontType::LatinSmallGreen;
	else if (amount > maximalAmount / 4)
		fontType = eUnicodeFontType::LatinSmallYellow;
	else
		fontType = eUnicodeFontType::LatinSmallRed;

	amountLabels[index]->setFont (fontType);
	amountLabels[index]->setText (std::to_string (amount) + "/" + std::to_string (maximalAmount));

	nameLabels[index]->setText (name);
	drawSmallSymbols (surface.get(), rowHeight, symbolType, cPosition (80, rowHeight * index), amount, maximalAmount);
}

//------------------------------------------------------------------------------
void cUnitDetailsHud::drawSmallSymbols (SDL_Surface* destination, int rowHeight, eUnitDataSymbolType symbolType, const cPosition& position, int value1, int value2)
{
	const int maxX = destination->w - position.x() - 5;
	auto src = getSmallSymbolPosition (symbolType);
	const cPosition srcSize = src.getSize();
	int toValue = value2;

	if (symbolType == eUnitDataSymbolType::Hits)
	{
		if (value1 <= value2 / 4) // red
		{
			src.getMinCorner().x() += srcSize.x() * 4;
			src.getMaxCorner().x() += srcSize.x() * 4;
		}
		else if (value1 <= value2 / 2) // orange
		{
			src.getMinCorner().x() += srcSize.x() * 2;
			src.getMaxCorner().x() += srcSize.x() * 2;
		}
	}
	int offX = srcSize.x();
	int step = 1;

	while (offX * toValue > maxX)
	{
		offX--;

		if (offX < 4)
		{
			toValue /= 2;
			step *= 2;
			offX = srcSize.x();
		}
	}

	SDL_Rect dest = {position.x(), position.y() + 2 + (rowHeight - srcSize.y()) / 2, 0, 0};

	const auto oriSrcMinX = src.getMinCorner().x();
	const auto oriSrcMaxX = src.getMaxCorner().x();
	for (int i = 0; i < toValue; i++)
	{
		if (value1 <= 0)
		{
			src.getMinCorner().x() = oriSrcMinX + srcSize.x();
			src.getMaxCorner().x() = oriSrcMaxX + srcSize.x();
		}

		auto srcRect = toSdlRect (src);
		SDL_BlitSurface (GraphicsData.gfx_hud_stuff.get(), &srcRect, destination, &dest);

		dest.x += offX;
		value1 -= step;
	}
}

//------------------------------------------------------------------------------
cBox<cPosition> cUnitDetailsHud::getSmallSymbolPosition (eUnitDataSymbolType symbolType)
{
	SDL_Rect rect{};

	switch (symbolType)
	{
		case eUnitDataSymbolType::Speed: rect = cGraphicsData::getRect_SmallSymbol_Speed(); break;
		case eUnitDataSymbolType::Hits: rect = cGraphicsData::getRect_SmallSymbol_Hits(); break;
		case eUnitDataSymbolType::Ammo: rect = cGraphicsData::getRect_SmallSymbol_Ammo(); break;
		case eUnitDataSymbolType::Shots: rect = cGraphicsData::getRect_SmallSymbol_Shots(); break;
		case eUnitDataSymbolType::Metal: rect = cGraphicsData::getRect_SmallSymbol_Metal(); break;
		case eUnitDataSymbolType::Oil: rect = cGraphicsData::getRect_SmallSymbol_Oil(); break;
		case eUnitDataSymbolType::Gold: rect = cGraphicsData::getRect_SmallSymbol_Gold(); break;
		case eUnitDataSymbolType::Energy: rect = cGraphicsData::getRect_SmallSymbol_Energy(); break;
		case eUnitDataSymbolType::Human: rect = cGraphicsData::getRect_SmallSymbol_Human(); break;
		case eUnitDataSymbolType::TransportTank: rect = cGraphicsData::getRect_SmallSymbol_TransportTank(); break;
		case eUnitDataSymbolType::TransportAir: rect = cGraphicsData::getRect_SmallSymbol_TransportAir(); break;

		case eUnitDataSymbolType::Attack:
		case eUnitDataSymbolType::Range:
		case eUnitDataSymbolType::Armor:
		case eUnitDataSymbolType::Scan:
		case eUnitDataSymbolType::MetalEmpty:
			break;
	}
	const cPosition position{rect.x, rect.y};
	const cPosition size{rect.w, rect.h};
	return cBox<cPosition> (position, position + size - 1);
}
