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
#include "main.h"
#include "video.h"
#include "unit.h"
#include "game/data/player/player.h"
#include "buildings.h"

//------------------------------------------------------------------------------
cUnitDetailsHud::cUnitDetailsHud (const cBox<cPosition>& area, bool drawLines_) :
	cWidget (area),
	drawLines (drawLines_),
	unit (nullptr),
	player (nullptr)
{
	const auto size = getSize ();
	if (size.y () < maxRows*rowHeight) resize (cPosition (getSize ().x (), maxRows*rowHeight));

	for (size_t i = 0; i < maxRows; ++i)
	{
		amountLabels[i] = addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (3, 2 + rowHeight * i), getPosition () + cPosition (3 + 35, 2 + rowHeight * i + rowHeight)), "", FONT_LATIN_SMALL_WHITE, toEnumFlag (eAlignmentType::CenterHorizontal) | eAlignmentType::Bottom));
		nameLabels[i] = addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (40, 2 + rowHeight * i), getPosition () + cPosition (40 + 40, 2 + rowHeight * i + rowHeight)), "", FONT_LATIN_SMALL_WHITE, toEnumFlag (eAlignmentType::Left) | eAlignmentType::Bottom));
	}

    surface = AutoSurface (SDL_CreateRGBSurface (0, size.x (), size.y (), Video.getColDepth (), 0, 0, 0, 0));

	SDL_FillRect (surface.get (), nullptr, 0xFF00FF);
	SDL_SetColorKey (surface.get (), SDL_TRUE, 0xFF00FF);
}

//------------------------------------------------------------------------------
void cUnitDetailsHud::setUnit (const cUnit* unit_, const cPlayer* player_)
{
	unit = unit_;
	player = player_;
}

//------------------------------------------------------------------------------
void cUnitDetailsHud::draw ()
{
	if (surface != nullptr)
	{
		reset ();

		SDL_Rect position = getArea ().toSdlRect ();
		SDL_BlitSurface (surface.get (), nullptr, cVideo::buffer, &position);
	}

	cWidget::draw ();
}

//------------------------------------------------------------------------------
void cUnitDetailsHud::reset ()
{
	SDL_FillRect (surface.get (), nullptr, 0xFF00FF);
	SDL_SetColorKey (surface.get (), SDL_TRUE, 0xFF00FF);

	for (int i = 0; i < maxRows; ++i)
	{
		if (drawLines)
		{
			SDL_Rect lineRect = {2, 14 + 12*i, getSize ().x (), 1};
			SDL_FillRect (surface.get (), &lineRect, 0xFF743904);
		}

		amountLabels[i]->hide ();
		nameLabels[i]->hide ();
	}

	if (unit == nullptr) return;

	const auto& data = unit->data;

	drawRow (0, eUnitDataSymbolType::Hits, data.getHitpoints (), data.hitpointsMax, lngPack.i18n ("Text~Others~Hitpoints_7"));

	if (data.speedMax > 0) drawRow (2, eUnitDataSymbolType::Speed, data.speedCur / 4, data.speedMax / 4, lngPack.i18n ("Text~Others~Speed_7"));

	if (data.canScore)
	{
		assert (unit->data.ID.isABuilding ()); // currently only buildings can score
		const auto unitScore = static_cast<const cBuilding*>(unit)->points;
		const auto totalScore = unit->owner->getScore ();
		// FIXME: get game settings in here
		const auto goalScore = totalScore;// (gameSettings.victoryType == SETTINGS_VICTORY_POINTS) ? gameSetting.duration : tot;

		drawRow (1, eUnitDataSymbolType::Human, unitScore, unitScore, lngPack.i18n ("Text~Others~Score"));
		drawRow (2, eUnitDataSymbolType::Human, totalScore, goalScore, lngPack.i18n ("Text~Others~Total"));
	}
	else if ((data.storeResType != sUnitData::STORE_RES_NONE || data.storageUnitsMax > 0) && unit->owner == player)
	{
		if (data.storeResType > 0)
		{
			eUnitDataSymbolType symbolType;
			switch (data.storeResType)
			{
			case sUnitData::STORE_RES_METAL:
				symbolType = eUnitDataSymbolType::Metal;
				break;
			case sUnitData::STORE_RES_OIL:
				symbolType = eUnitDataSymbolType::Oil;
				break;
			case sUnitData::STORE_RES_GOLD:
				symbolType = eUnitDataSymbolType::Gold;
				break;
			}

			drawRow (1, symbolType, data.getStoredResources (), data.storageResMax, lngPack.i18n ("Text~Others~Cargo_7"));

			if (unit->data.ID.isABuilding ())
			{
				const auto& building = static_cast<const cBuilding&>(*unit);
				switch (data.storeResType)
				{
				case sUnitData::STORE_RES_METAL:
					drawRow (2, symbolType, building.SubBase->getMetal (), building.SubBase->MaxMetal, lngPack.i18n ("Text~Others~Total"));
					break;
				case sUnitData::STORE_RES_OIL:
					drawRow (2, symbolType, building.SubBase->getOil (), building.SubBase->MaxOil, lngPack.i18n ("Text~Others~Total"));
					break;
				case sUnitData::STORE_RES_GOLD:
					drawRow (2, symbolType, building.SubBase->getGold (), building.SubBase->MaxGold, lngPack.i18n ("Text~Others~Total"));
					break;
				}
			}
		}
		else if (data.storeUnitsImageType != sUnitData::STORE_UNIT_IMG_NONE)
		{
			eUnitDataSymbolType symbolType;
			switch (data.storeUnitsImageType)
			{
			case sUnitData::STORE_UNIT_IMG_TANK:
			case sUnitData::STORE_UNIT_IMG_SHIP:
				symbolType = eUnitDataSymbolType::TransportTank;
				break;
			case sUnitData::STORE_UNIT_IMG_PLANE:
				symbolType = eUnitDataSymbolType::TransportAir;
				break;
			case sUnitData::STORE_UNIT_IMG_HUMAN:
				symbolType = eUnitDataSymbolType::Human;
				break;
			}

			drawRow (1, symbolType, data.getStoredUnits (), data.storageUnitsMax, lngPack.i18n ("Text~Others~Cargo_7"));
		}
	}
	else if (data.canAttack && !data.explodesOnContact)
	{
		if (unit->owner == player) drawRow (1, eUnitDataSymbolType::Ammo, data.getAmmo (), data.ammoMax, lngPack.i18n ("Text~Others~Ammo_7"));

		drawRow (3, eUnitDataSymbolType::Shots, data.getShots (), data.shotsMax, lngPack.i18n ("Text~Others~Shots_7"));
	}
	else if (data.produceEnergy && unit->data.ID.isABuilding ())
	{
		const auto& building = static_cast<const cBuilding&>(*unit);
		drawRow (1, eUnitDataSymbolType::Energy, (building.isUnitWorking () ? data.produceEnergy : 0), data.produceEnergy, lngPack.i18n ("Text~Others~Power"));

		if (unit->owner == player)
		{
			drawRow (2, eUnitDataSymbolType::Energy, building.SubBase->EnergyProd, building.SubBase->MaxEnergyProd, lngPack.i18n ("Text~Others~Total"));
			drawRow (3, eUnitDataSymbolType::Energy, building.SubBase->EnergyNeed, building.SubBase->MaxEnergyNeed, lngPack.i18n ("Text~Others~Usage_7"));
		}
	}
	else if (data.produceHumans && unit->data.ID.isABuilding ())
	{
		const auto& building = static_cast<const cBuilding&>(*unit);
		drawRow (1, eUnitDataSymbolType::Human, data.produceHumans, data.produceHumans, lngPack.i18n ("Text~Others~Teams_7"));

		if (unit->owner == player)
		{
			drawRow (2, eUnitDataSymbolType::Human, building.SubBase->HumanProd, building.SubBase->HumanProd, lngPack.i18n ("Text~Others~Total"));
			drawRow (3, eUnitDataSymbolType::Human, building.SubBase->HumanNeed, building.SubBase->MaxHumanNeed, lngPack.i18n ("Text~Others~Usage_7"));
		}
	}
	else if (data.needsHumans && unit->data.ID.isABuilding ())
	{
		const auto& building = static_cast<const cBuilding&>(*unit);
		if (building.isUnitWorking ()) drawRow (1, eUnitDataSymbolType::Human, data.needsHumans, data.needsHumans, lngPack.i18n ("Text~Others~Usage_7"));
		else drawRow (1, eUnitDataSymbolType::Human, 0, data.needsHumans, lngPack.i18n ("Text~Others~Usage_7"));

		if (unit->owner == player) drawRow (2, eUnitDataSymbolType::Human, building.SubBase->HumanNeed, building.SubBase->MaxHumanNeed, lngPack.i18n ("Text~Others~Total"));
	}
}

//------------------------------------------------------------------------------
void cUnitDetailsHud::drawRow (size_t index, eUnitDataSymbolType symbolType, int amount, int maximalAmount, const std::string& name)
{
	if (index >= maxRows) return;

	amountLabels[index]->show ();
	nameLabels[index]->show ();

	eUnicodeFontType fontType;
	if (amount > maximalAmount / 2) fontType = FONT_LATIN_SMALL_GREEN;
	else if (amount > maximalAmount / 4) fontType = FONT_LATIN_SMALL_YELLOW;
	else fontType = FONT_LATIN_SMALL_RED;

	amountLabels[index]->setFont (fontType);
	amountLabels[index]->setText (iToStr (amount) + "/" + iToStr (maximalAmount));

	nameLabels[index]->setText (name);
	drawSmallSymbols (surface.get (), rowHeight, symbolType, cPosition (80, rowHeight * index), amount, maximalAmount);
}

//------------------------------------------------------------------------------
void cUnitDetailsHud::drawSmallSymbols (SDL_Surface* destination, int rowHeight, eUnitDataSymbolType symbolType, const cPosition& position, int value1, int value2)
{
	const int maxX = destination->w - position.x () - 5;
	auto src = getSmallSymbolPosition (symbolType);
	const cPosition srcSize = src.getSize();
	int toValue = value2;

	if (symbolType == eUnitDataSymbolType::Hits)
	{
		if (value1 <= value2 / 4) // red
		{
			src.getMinCorner ().x () += srcSize.x () * 4;
			src.getMaxCorner ().x () += srcSize.x () * 4;
		}
		else if (value1 <= value2 / 2) // orange
		{
			src.getMinCorner ().x () += srcSize.x () * 2;
			src.getMaxCorner ().x () += srcSize.x () * 2;
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
			offX = srcSize.x ();
		}
	}

	SDL_Rect dest = {position.x (), position.y ()+2 + (rowHeight - srcSize.y ()) / 2, 0, 0};

	const auto oriSrcMinX = src.getMinCorner ().x ();
	const auto oriSrcMaxX = src.getMaxCorner ().x ();
	for (int i = 0; i < toValue; i++)
	{
		if (value1 <= 0)
		{
			src.getMinCorner ().x () = oriSrcMinX + srcSize.x ();
			src.getMaxCorner ().x () = oriSrcMaxX + srcSize.x ();
		}

		auto srcRect = src.toSdlRect ();
		SDL_BlitSurface (GraphicsData.gfx_hud_stuff.get (), &srcRect, destination, &dest);

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
		position.x () = 0;
		size.x () = 7;
		size.y () = 7;
		break;
	case eUnitDataSymbolType::Hits:
		position.x () = 14;
		size.x () = 6;
		size.y () = 9;
		break;
	case eUnitDataSymbolType::Ammo:
		position.x () = 50;
		size.x () = 5;
		size.y () = 7;
		break;
	case eUnitDataSymbolType::Shots:
		position.x () = 88;
		size.x () = 8;
		size.y () = 4;
		break;
	case eUnitDataSymbolType::Metal:
		position.x () = 60;
		size.x () = 7;
		size.y () = 10;
		break;
	case eUnitDataSymbolType::Oil:
		position.x () = 104;
		size.x () = 8;
		size.y () = 9;
		break;
	case eUnitDataSymbolType::Gold:
		position.x () = 120;
		size.x () = 9;
		size.y () = 8;
		break;
	case eUnitDataSymbolType::Energy:
		position.x () = 74;
		size.x () = 7;
		size.y () = 7;
		break;
	case eUnitDataSymbolType::Human:
		position.x () = 170;
		size.x () = 8;
		size.y () = 9;
		break;
	case eUnitDataSymbolType::TransportTank:
		position.x () = 138;
		size.x () = 16;
		size.y () = 8;
		break;
	case eUnitDataSymbolType::TransportAir:
		position.x () = 138;
		size.x () = 21;
		size.y () = 8;
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
