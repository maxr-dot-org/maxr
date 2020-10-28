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

#include "ui/graphical/menu/widgets/label.h"
#include "utility/string/toString.h"
#include "output/video/video.h"
#include "game/data/units/unit.h"
#include "game/data/player/player.h"
#include "game/data/units/building.h"
#include "game/data/gamesettings.h"
#include "utility/drawing.h"
#include "utility/language.h"

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
		amountLabels[i] = addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (3, 2 + rowHeight * i), getPosition() + cPosition (3 + 35, 2 + rowHeight * i + rowHeight)), "", FONT_LATIN_SMALL_WHITE, toEnumFlag (eAlignmentType::CenterHorizontal) | eAlignmentType::Bottom));
		nameLabels[i] = addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (40, 2 + rowHeight * i), getPosition() + cPosition (40 + 40, 2 + rowHeight * i + rowHeight)), "", FONT_LATIN_SMALL_WHITE, toEnumFlag (eAlignmentType::Left) | eAlignmentType::Bottom));
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

	drawRow (0, eUnitDataSymbolType::Hits, data.getHitpoints(), data.getHitpointsMax(), lngPack.i18n ("Text~Others~Hitpoints_7"));

	if (data.getSpeedMax() > 0) drawRow (2, eUnitDataSymbolType::Speed, data.getSpeed() / 4, data.getSpeedMax() / 4, lngPack.i18n ("Text~Others~Speed_7"));

	if (staticData.canScore)
	{
		assert (unit->isABuilding());  // currently only buildings can score
		const auto unitScore = static_cast<const cBuilding*> (unit)->points;
		const auto totalScore = unit->getOwner()->getScore();
		const auto goalScore = (gameSettings && gameSettings->getVictoryCondition() == eGameSettingsVictoryCondition::Points) ? gameSettings->getVictoryPoints() : totalScore;

		drawRow (1, eUnitDataSymbolType::Human, unitScore, unitScore, lngPack.i18n ("Text~Others~Score"));
		drawRow (2, eUnitDataSymbolType::Human, totalScore, goalScore, lngPack.i18n ("Text~Others~Total"));
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

			drawRow (1, symbolType, unit->getStoredResources(), staticData.storageResMax, lngPack.i18n ("Text~Others~Cargo_7"));

			if (unit->isABuilding())
			{
				const auto& building = static_cast<const cBuilding&> (*unit);
				const sMiningResource& stored = building.subBase->getResourcesStored();
				const sMiningResource& maxStored = building.subBase->getMaxResourcesStored();

				switch (staticData.storeResType)
				{
					case eResourceType::Metal:
						drawRow (2, symbolType, stored.metal, maxStored.metal, lngPack.i18n ("Text~Others~Total"));
						break;
					case eResourceType::Oil:
						drawRow (2, symbolType, stored.oil, maxStored.oil, lngPack.i18n ("Text~Others~Total"));
						break;
					case eResourceType::Gold:
						drawRow (2, symbolType, stored.gold, maxStored.gold, lngPack.i18n ("Text~Others~Total"));
						break;
					case eResourceType::None: break;
				}
			}
		}
		else if (staticData.storeUnitsImageType != cStaticUnitData::STORE_UNIT_IMG_NONE)
		{
			eUnitDataSymbolType symbolType;
			switch (staticData.storeUnitsImageType)
			{
				case cStaticUnitData::STORE_UNIT_IMG_TANK:
				case cStaticUnitData::STORE_UNIT_IMG_SHIP:
					symbolType = eUnitDataSymbolType::TransportTank;
					break;
				case cStaticUnitData::STORE_UNIT_IMG_PLANE:
					symbolType = eUnitDataSymbolType::TransportAir;
					break;
				case cStaticUnitData::STORE_UNIT_IMG_HUMAN:
					symbolType = eUnitDataSymbolType::Human;
					break;
				case cStaticUnitData::STORE_UNIT_IMG_NONE: break;
			}

			drawRow (1, symbolType, unit->storedUnits.size(), staticData.storageUnitsMax, lngPack.i18n ("Text~Others~Cargo_7"));
		}
	}
	else if (staticData.canAttack && !staticData.explodesOnContact)
	{
		if (unit->getOwner() == player) drawRow (1, eUnitDataSymbolType::Ammo, data.getAmmo(), data.getAmmoMax(), lngPack.i18n ("Text~Others~Ammo_7"));

		drawRow (3, eUnitDataSymbolType::Shots, data.getShots(), data.getShotsMax(), lngPack.i18n ("Text~Others~Shots_7"));
	}
	else if (staticData.produceEnergy && unit->isABuilding())
	{
		const auto& building = static_cast<const cBuilding&> (*unit);
		drawRow (1, eUnitDataSymbolType::Energy, (building.isUnitWorking() ? staticData.produceEnergy : 0), staticData.produceEnergy, lngPack.i18n ("Text~Others~Power"));

		if (unit->getOwner() == player)
		{
			drawRow (2, eUnitDataSymbolType::Energy, building.subBase->getEnergyProd(), building.subBase->getMaxEnergyProd(), lngPack.i18n ("Text~Others~Total"));
			drawRow (3, eUnitDataSymbolType::Energy, building.subBase->getEnergyNeed(), building.subBase->getMaxEnergyNeed(), lngPack.i18n ("Text~Others~Usage_7"));
		}
	}
	else if (staticData.produceHumans && unit->isABuilding())
	{
		const auto& building = static_cast<const cBuilding&> (*unit);
		drawRow (1, eUnitDataSymbolType::Human, staticData.produceHumans, staticData.produceHumans, lngPack.i18n ("Text~Others~Teams_7"));

		if (unit->getOwner() == player)
		{
			drawRow (2, eUnitDataSymbolType::Human, building.subBase->getHumanProd(), building.subBase->getHumanProd(), lngPack.i18n ("Text~Others~Total"));
			drawRow (3, eUnitDataSymbolType::Human, building.subBase->getHumanNeed(), building.subBase->getMaxHumanNeed(), lngPack.i18n ("Text~Others~Usage_7"));
		}
	}
	else if (staticData.needsHumans && unit->isABuilding())
	{
		const auto& building = static_cast<const cBuilding&> (*unit);
		if (building.isUnitWorking()) drawRow (1, eUnitDataSymbolType::Human, staticData.needsHumans, staticData.needsHumans, lngPack.i18n ("Text~Others~Usage_7"));
		else drawRow (1, eUnitDataSymbolType::Human, 0, staticData.needsHumans, lngPack.i18n ("Text~Others~Usage_7"));

		if (unit->getOwner() == player) drawRow (2, eUnitDataSymbolType::Human, building.subBase->getHumanNeed(), building.subBase->getMaxHumanNeed(), lngPack.i18n ("Text~Others~Total"));
	}
}

//------------------------------------------------------------------------------
void cUnitDetailsHud::drawRow (size_t index, eUnitDataSymbolType symbolType, int amount, int maximalAmount, const std::string& name)
{
	if (index >= maxRows) return;

	amountLabels[index]->show();
	nameLabels[index]->show();

	eUnicodeFontType fontType;
	if (amount > maximalAmount / 2) fontType = FONT_LATIN_SMALL_GREEN;
	else if (amount > maximalAmount / 4) fontType = FONT_LATIN_SMALL_YELLOW;
	else fontType = FONT_LATIN_SMALL_RED;

	amountLabels[index]->setFont (fontType);
	amountLabels[index]->setText (iToStr (amount) + "/" + iToStr (maximalAmount));

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

		auto srcRect = src.toSdlRect();
		SDL_BlitSurface (GraphicsData.gfx_hud_stuff.get(), &srcRect, destination, &dest);

		dest.x += offX;
		value1 -= step;
	}
}

//------------------------------------------------------------------------------
cBox<cPosition> cUnitDetailsHud::getSmallSymbolPosition (eUnitDataSymbolType symbolType)
{
	cPosition position (0, 98);
	cPosition size (0, 0);

	switch (symbolType)
	{
		case eUnitDataSymbolType::Speed:
			position.x() = 0;
			size.x() = 7;
			size.y() = 7;
			break;
		case eUnitDataSymbolType::Hits:
			position.x() = 14;
			size.x() = 6;
			size.y() = 9;
			break;
		case eUnitDataSymbolType::Ammo:
			position.x() = 50;
			size.x() = 5;
			size.y() = 7;
			break;
		case eUnitDataSymbolType::Shots:
			position.x() = 88;
			size.x() = 8;
			size.y() = 4;
			break;
		case eUnitDataSymbolType::Metal:
			position.x() = 60;
			size.x() = 7;
			size.y() = 10;
			break;
		case eUnitDataSymbolType::Oil:
			position.x() = 104;
			size.x() = 8;
			size.y() = 9;
			break;
		case eUnitDataSymbolType::Gold:
			position.x() = 120;
			size.x() = 9;
			size.y() = 8;
			break;
		case eUnitDataSymbolType::Energy:
			position.x() = 74;
			size.x() = 7;
			size.y() = 7;
			break;
		case eUnitDataSymbolType::Human:
			position.x() = 170;
			size.x() = 8;
			size.y() = 9;
			break;
		case eUnitDataSymbolType::TransportTank:
			position.x() = 138;
			size.x() = 16;
			size.y() = 8;
			break;
		case eUnitDataSymbolType::TransportAir:
			position.x() = 186;
			size.x() = 21;
			size.y() = 8;
			break;
		case eUnitDataSymbolType::Attack:
		case eUnitDataSymbolType::Range:
		case eUnitDataSymbolType::Armor:
		case eUnitDataSymbolType::Scan:
		case eUnitDataSymbolType::MetalEmpty:
			break;
	}

	return cBox<cPosition> (position, position + size - 1);
}
