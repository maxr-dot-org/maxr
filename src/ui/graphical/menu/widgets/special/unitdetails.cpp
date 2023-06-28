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

#include "ui/graphical/menu/widgets/special/unitdetails.h"

#include "SDLutility/tosdl.h"
#include "game/data/player/player.h"
#include "game/logic/upgradecalculator.h"
#include "output/video/video.h"
#include "resources/uidata.h"
#include "ui/widgets/label.h"
#include "utility/language.h"

//------------------------------------------------------------------------------
cUnitDetails::cUnitDetails (const cPosition& position) :
	cWidget (position),
	staticUnitData (nullptr),
	playerOriginalData (nullptr),
	playerCurrentData (nullptr),
	unitObjectCurrentData (nullptr),
	upgrades (nullptr)
{
	for (size_t i = 0; i < maxRows; ++i)
	{
		amountLabels[i] = emplaceChild<cLabel> (cBox<cPosition> (position + cPosition (5, 3 + rowHeight * i), position + cPosition (5 + 21, 3 + rowHeight * i + 10)), "", eUnicodeFontType::LatinNormal, eAlignmentType::Right);
		nameLabels[i] = emplaceChild<cLabel> (cBox<cPosition> (position + cPosition (30, 3 + rowHeight * i), position + cPosition (30 + 55, 3 + rowHeight * i + 10)), "", eUnicodeFontType::LatinNormal, eAlignmentType::Left);
	}

	const cPosition size (250, 170);
	surface = AutoSurface (SDL_CreateRGBSurface (0, size.x(), size.y(), Video.getColDepth(), 0, 0, 0, 0));

	SDL_FillRect (surface.get(), nullptr, 0xFF00FF);
	SDL_SetColorKey (surface.get(), SDL_TRUE, 0xFF00FF);

	resize (size);
}

//------------------------------------------------------------------------------
void cUnitDetails::draw (SDL_Surface& destination, const cBox<cPosition>& clipRect)
{
	if (surface != nullptr)
	{
		reset();

		SDL_Rect position = toSdlRect (getArea());
		SDL_BlitSurface (surface.get(), nullptr, &destination, &position);
	}

	cWidget::draw (destination, clipRect);
}

//------------------------------------------------------------------------------
const sID* cUnitDetails::getCurrentUnitId()
{
	if (playerOriginalData != nullptr)
	{
		return &unitId;
	}
	return nullptr;
}

//------------------------------------------------------------------------------
void cUnitDetails::setUnit (const sID& unitId_, const cPlayer* owner, const cUnitsData& unitsData, const cDynamicUnitData* unitObjectCurrentData_, const cUnitUpgrade* upgrades_)
{
	unitId = unitId_;

	staticUnitData = &unitsData.getStaticUnitData (unitId);
	playerOriginalData = &unitsData.getDynamicUnitData (unitId, owner ? owner->getClan() : -1);
	playerCurrentData = owner ? owner->getLastUnitData (unitId) : playerOriginalData;

	if (unitObjectCurrentData_ == nullptr)
	{
		unitObjectCurrentData = playerCurrentData;
	}
	else
	{
		unitObjectCurrentData = unitObjectCurrentData_;
	}

	upgrades = upgrades_;
}

//------------------------------------------------------------------------------
void cUnitDetails::setUpgrades (const cUnitUpgrade* upgrades_)
{
	upgrades = upgrades_;
}

//------------------------------------------------------------------------------
void cUnitDetails::drawRow (size_t index, eUnitDataSymbolType symbolType, int amount, const std::string& name, int value1, int value2)
{
	if (index >= maxRows) return;

	if (index != 0)
	{
		SDL_Rect dest = {0, int (rowHeight * index - 3), surface->w, 1};
		SDL_FillRect (surface.get(), &dest, 0xFFFC0000);
	}

	amountLabels[index]->show();
	nameLabels[index]->show();

	amountLabels[index]->setText (std::to_string (amount));
	nameLabels[index]->setText (name);
	drawBigSymbols (symbolType, cPosition (95, rowHeight * index), value1, value2);
}

//------------------------------------------------------------------------------
void cUnitDetails::reset()
{
	if (unitObjectCurrentData == nullptr || playerCurrentData == nullptr || playerOriginalData == nullptr) return;

	const sUnitUpgrade* upgrade = nullptr;

	size_t rowIndex = 0;

	SDL_FillRect (surface.get(), nullptr, 0xFF00FF);
	SDL_SetColorKey (surface.get(), SDL_TRUE, 0xFF00FF);

	if (staticUnitData->canAttack)
	{
		// Damage:
		upgrade = upgrades ? upgrades->getUpgrade (sUnitUpgrade::eUpgradeType::Damage) : nullptr;
		drawRow (rowIndex++, eUnitDataSymbolType::Attack, upgrade ? upgrade->getCurValue() : unitObjectCurrentData->getDamage(), lngPack.i18n ("Others~Attack_7"), upgrade ? upgrade->getCurValue() : unitObjectCurrentData->getDamage(), playerOriginalData->getDamage());

		if (staticUnitData->ID.isAVehicle() || !staticUnitData->buildingData.explodesOnContact)
		{
			// Shots:
			upgrade = upgrades ? upgrades->getUpgrade (sUnitUpgrade::eUpgradeType::Shots) : nullptr;
			drawRow (rowIndex++, eUnitDataSymbolType::Shots, upgrade ? upgrade->getCurValue() : unitObjectCurrentData->getShotsMax(), lngPack.i18n ("Others~Shots_7"), upgrade ? upgrade->getCurValue() : unitObjectCurrentData->getShotsMax(), playerOriginalData->getShotsMax());

			// Range:
			upgrade = upgrades ? upgrades->getUpgrade (sUnitUpgrade::eUpgradeType::Range) : nullptr;
			drawRow (rowIndex++, eUnitDataSymbolType::Range, upgrade ? upgrade->getCurValue() : unitObjectCurrentData->getRange(), lngPack.i18n ("Others~Range_7"), upgrade ? upgrade->getCurValue() : unitObjectCurrentData->getRange(), playerOriginalData->getRange());

			// Ammo:
			upgrade = upgrades ? upgrades->getUpgrade (sUnitUpgrade::eUpgradeType::Ammo) : nullptr;
			drawRow (rowIndex++, eUnitDataSymbolType::Ammo, upgrade ? upgrade->getCurValue() : unitObjectCurrentData->getAmmoMax(), lngPack.i18n ("Others~Ammo_7"), upgrade ? upgrade->getCurValue() : unitObjectCurrentData->getAmmoMax(), playerOriginalData->getAmmoMax());
		}
	}

	eResourceType transport = staticUnitData->storeResType;

	if (transport != eResourceType::None)
	{
		eUnitDataSymbolType symbolType;
		switch (transport)
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
		drawRow (rowIndex++, symbolType, staticUnitData->storageResMax, lngPack.i18n ("Others~Cargo_7"), staticUnitData->storageResMax, staticUnitData->storageResMax);
	}

	if (staticUnitData->produceEnergy)
	{
		// Energy production:
		drawRow (rowIndex++, eUnitDataSymbolType::Energy, staticUnitData->produceEnergy, lngPack.i18n ("Others~Produce_7"), staticUnitData->produceEnergy, staticUnitData->produceEnergy);

		// Oil consumption:
		drawRow (rowIndex++, eUnitDataSymbolType::Oil, staticUnitData->needsOil, lngPack.i18n ("Others~Usage_7"), staticUnitData->needsOil, staticUnitData->needsOil);
	}

	if (staticUnitData->produceHumans)
	{
		// Human production:
		drawRow (rowIndex++, eUnitDataSymbolType::Human, staticUnitData->produceHumans, lngPack.i18n ("Others~Produce_7"), staticUnitData->produceHumans, staticUnitData->produceHumans);
	}

	// Armor:
	upgrade = upgrades ? upgrades->getUpgrade (sUnitUpgrade::eUpgradeType::Armor) : nullptr;
	drawRow (rowIndex++, eUnitDataSymbolType::Armor, upgrade ? upgrade->getCurValue() : unitObjectCurrentData->getArmor(), lngPack.i18n ("Others~Armor_7"), upgrade ? upgrade->getCurValue() : unitObjectCurrentData->getArmor(), playerOriginalData->getArmor());

	// Hit points:
	upgrade = upgrades ? upgrades->getUpgrade (sUnitUpgrade::eUpgradeType::Hits) : nullptr;
	drawRow (rowIndex++, eUnitDataSymbolType::Hits, upgrade ? upgrade->getCurValue() : unitObjectCurrentData->getHitpointsMax(), lngPack.i18n ("Others~Hitpoints_7"), upgrade ? upgrade->getCurValue() : unitObjectCurrentData->getHitpointsMax(), playerOriginalData->getHitpointsMax());

	// Scan:
	if (unitObjectCurrentData->getScan())
	{
		upgrade = upgrades ? upgrades->getUpgrade (sUnitUpgrade::eUpgradeType::Scan) : nullptr;
		drawRow (rowIndex++, eUnitDataSymbolType::Scan, upgrade ? upgrade->getCurValue() : unitObjectCurrentData->getScan(), lngPack.i18n ("Others~Scan_7"), upgrade ? upgrade->getCurValue() : unitObjectCurrentData->getScan(), playerOriginalData->getScan());
	}

	// Speed:
	if (unitObjectCurrentData->getSpeedMax())
	{
		upgrade = upgrades ? upgrades->getUpgrade (sUnitUpgrade::eUpgradeType::Speed) : nullptr;
		drawRow (rowIndex++, eUnitDataSymbolType::Speed, (upgrade ? upgrade->getCurValue() : unitObjectCurrentData->getSpeedMax()) / 4, lngPack.i18n ("Others~Speed_7"), (upgrade ? upgrade->getCurValue() : unitObjectCurrentData->getSpeedMax()) / 4, playerOriginalData->getSpeedMax() / 4);
	}

	// energy consumption:
	if (staticUnitData->needsEnergy)
	{
		drawRow (rowIndex++, eUnitDataSymbolType::Energy, staticUnitData->needsEnergy, lngPack.i18n ("Others~Usage_7"), staticUnitData->needsEnergy, staticUnitData->needsEnergy);
	}

	// humans needed:
	if (staticUnitData->needsHumans)
	{
		drawRow (rowIndex++, eUnitDataSymbolType::Human, staticUnitData->needsHumans, lngPack.i18n ("Others~Usage_7"), staticUnitData->needsHumans, staticUnitData->needsHumans);
	}

	// raw material consumption:
	if (staticUnitData->needsMetal)
	{
		drawRow (rowIndex++, eUnitDataSymbolType::Metal, staticUnitData->needsMetal, lngPack.i18n ("Others~Usage_7"), staticUnitData->needsMetal, staticUnitData->needsMetal);
	}

	// gold consumption:
	if (staticUnitData->buildingData.convertsGold)
	{
		drawRow (rowIndex++, eUnitDataSymbolType::Gold, staticUnitData->buildingData.convertsGold, lngPack.i18n ("Others~Usage_7"), staticUnitData->buildingData.convertsGold, staticUnitData->buildingData.convertsGold);
	}

	// Costs:
	// Do not use unit data but currentVersion data
	// since cost doesn't change unit version
	drawRow (rowIndex++, eUnitDataSymbolType::Metal, playerCurrentData->getBuildCost(), lngPack.i18n ("Others~Costs"), playerCurrentData->getBuildCost(), playerOriginalData->getBuildCost());

	while (rowIndex < maxRows)
	{
		amountLabels[rowIndex]->hide();
		nameLabels[rowIndex]->hide();
		++rowIndex;
	}
}

//------------------------------------------------------------------------------
void cUnitDetails::drawBigSymbols (eUnitDataSymbolType symbolType, const cPosition& position, int value1, int value2)
{
	int maxX = 160;
	auto src = getBigSymbolPosition (symbolType);
	const auto srcSize = src.getSize();
	maxX -= srcSize.x();

	if (value2 != value1) maxX -= srcSize.x() + 3;
	if (value2 < value1) std::swap (value1, value2);
	int offX = srcSize.x();
	while (offX * value2 > maxX)
	{
		--offX;
		if (offX < 4)
		{
			value1 /= 2;
			value2 /= 2;
			offX = srcSize.x();
		}
	}
	SDL_Rect dest = {position.x(), position.y() + (rowHeight - 4 - srcSize.y()) / 2, 0, 0};

	for (int i = 0; i != value1; ++i)
	{
		auto srcRect = toSdlRect (src);
		SDL_BlitSurface (GraphicsData.gfx_hud_stuff.get(), &srcRect, surface.get(), &dest);

		dest.x += offX;
	}
	if (value1 == value2) return;

	dest.x += srcSize.x() + 3;
	SDL_Rect mark = {Sint16 (dest.x - srcSize.x() / 2), dest.y, 1, srcSize.y()};

	SDL_FillRect (surface.get(), &mark, 0xFFFC0000);

	if (symbolType == eUnitDataSymbolType::Metal)
	{
		src = getBigSymbolPosition (eUnitDataSymbolType::MetalEmpty);
	}
	for (int i = value1; i != value2; ++i)
	{
		auto srcRect = toSdlRect (src);
		SDL_BlitSurface (GraphicsData.gfx_hud_stuff.get(), &srcRect, surface.get(), &dest);

		dest.x += offX;
	}
}

//------------------------------------------------------------------------------
cBox<cPosition> cUnitDetails::getBigSymbolPosition (eUnitDataSymbolType symbolType)
{
	cPosition position (0, 109);
	cPosition size (1, 1);

	switch (symbolType)
	{
		case eUnitDataSymbolType::Speed:
			position.x() = 0;
			size.x() = 11;
			size.y() = 12;
			break;
		case eUnitDataSymbolType::Hits:
			position.x() = 11;
			size.x() = 7;
			size.y() = 11;
			break;
		case eUnitDataSymbolType::Ammo:
			position.x() = 18;
			size.x() = 9;
			size.y() = 14;
			break;
		case eUnitDataSymbolType::Attack:
			position.x() = 27;
			size.x() = 10;
			size.y() = 14;
			break;
		case eUnitDataSymbolType::Shots:
			position.x() = 37;
			size.x() = 15;
			size.y() = 7;
			break;
		case eUnitDataSymbolType::Range:
			position.x() = 52;
			size.x() = 13;
			size.y() = 13;
			break;
		case eUnitDataSymbolType::Armor:
			position.x() = 65;
			size.x() = 11;
			size.y() = 14;
			break;
		case eUnitDataSymbolType::Scan:
			position.x() = 76;
			size.x() = 13;
			size.y() = 13;
			break;
		case eUnitDataSymbolType::Metal:
			position.x() = 89;
			size.x() = 12;
			size.y() = 15;
			break;
		case eUnitDataSymbolType::MetalEmpty:
			position.x() = 175;
			size.x() = 12;
			size.y() = 15;
			break;
		case eUnitDataSymbolType::Oil:
			position.x() = 101;
			size.x() = 11;
			size.y() = 12;
			break;
		case eUnitDataSymbolType::Gold:
			position.x() = 112;
			size.x() = 13;
			size.y() = 10;
			break;
		case eUnitDataSymbolType::Energy:
			position.x() = 125;
			size.x() = 13;
			size.y() = 17;
			break;
		case eUnitDataSymbolType::Human:
			position.x() = 138;
			size.x() = 12;
			size.y() = 16;
			break;
		case eUnitDataSymbolType::TransportTank:
		case eUnitDataSymbolType::TransportAir:
			break;
	}

	return cBox<cPosition> (position, position + size - 1);
}
