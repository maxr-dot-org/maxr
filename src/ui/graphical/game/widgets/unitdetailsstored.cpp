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

#include "ui/graphical/game/widgets/unitdetailsstored.h"

#include "SDLutility/tosdl.h"
#include "game/data/player/player.h"
#include "game/data/units/building.h"
#include "game/data/units/unit.h"
#include "output/video/video.h"
#include "ui/graphical/game/widgets/unitdetailshud.h"
#include "ui/translations.h"
#include "ui/widgets/label.h"
#include "utility/language.h"

//------------------------------------------------------------------------------
cUnitDetailsStored::cUnitDetailsStored (const cBox<cPosition>& area) :
	cWidget (area),
	unit (nullptr)
{
	const auto size = getSize();
	if (std::size_t (size.y()) < maxRows * rowHeight) resize (cPosition (getSize().x(), maxRows * rowHeight));

	for (size_t i = 0; i < maxRows; ++i)
	{
		amountLabels[i] = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (3, 2 + rowHeight * i), getPosition() + cPosition (3 + 30, 2 + rowHeight * i + rowHeight)), "", eUnicodeFontType::LatinSmallWhite, toEnumFlag (eAlignmentType::CenterHorizontal) | eAlignmentType::Bottom);
		nameLabels[i] = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (35, 2 + rowHeight * i), getPosition() + cPosition (35 + 30, 2 + rowHeight * i + rowHeight)), "", eUnicodeFontType::LatinSmallWhite, toEnumFlag (eAlignmentType::Left) | eAlignmentType::Bottom);
	}

	surface = UniqueSurface (SDL_CreateRGBSurface (0, size.x(), size.y(), Video.getColDepth(), 0, 0, 0, 0));

	SDL_FillRect (surface.get(), nullptr, 0xFF00FF);
	SDL_SetColorKey (surface.get(), SDL_TRUE, 0xFF00FF);
}

//------------------------------------------------------------------------------
void cUnitDetailsStored::setUnit (const cUnit* unit_)
{
	unit = unit_;

	reset();

	unitSignalConnectionManager.disconnectAll();

	if (unit)
	{
		unitSignalConnectionManager.connect (unit->data.hitpointsChanged, [this]() { reset(); });
		unitSignalConnectionManager.connect (unit->data.ammoChanged, [this]() { reset(); });
		unitSignalConnectionManager.connect (unit->data.hitpointsMaxChanged, [this]() { reset(); });
		unitSignalConnectionManager.connect (unit->data.ammoMaxChanged, [this]() { reset(); });
	}
}

//------------------------------------------------------------------------------
void cUnitDetailsStored::draw (SDL_Surface& destination, const cBox<cPosition>& clipRect)
{
	if (surface != nullptr)
	{
		SDL_Rect position = toSdlRect (getArea());
		SDL_BlitSurface (surface.get(), nullptr, &destination, &position);
	}

	cWidget::draw (destination, clipRect);
}

//------------------------------------------------------------------------------
void cUnitDetailsStored::reset()
{
	SDL_FillRect (surface.get(), nullptr, 0xFF00FF);
	SDL_SetColorKey (surface.get(), SDL_TRUE, 0xFF00FF);

	for (std::size_t i = 0; i < maxRows; ++i)
	{
		amountLabels[i]->hide();
		nameLabels[i]->hide();
	}

	if (unit == nullptr) return;

	const auto& data = unit->data;

	drawRow (0, eUnitDataSymbolType::Hits, data.getHitpoints(), data.getHitpointsMax(), lngPack.i18n ("Others~Hitpoints_7"));

	if (unit->getStaticUnitData().canAttack) drawRow (1, eUnitDataSymbolType::Ammo, data.getAmmo(), data.getAmmoMax(), lngPack.i18n ("Others~Ammo_7"));

	const cVehicle* vehicle = dynamic_cast<const cVehicle*> (unit);
	const auto storedResources = vehicle->getStoredResources();
	const auto storagedResMax = vehicle->getStaticUnitData().storageResMax;
	const auto title = toTranslatedString (unit->getStaticUnitData().storeResType);
	switch (unit->getStaticUnitData().storeResType)
	{
		case eResourceType::None: break;
		case eResourceType::Metal: drawRow (1, eUnitDataSymbolType::Metal, storedResources, storagedResMax, title); break;
		case eResourceType::Oil: drawRow (1, eUnitDataSymbolType::Oil, storedResources, storagedResMax, title); break;
		case eResourceType::Gold: drawRow (1, eUnitDataSymbolType::Gold, storedResources, storagedResMax, title); break;
	}
}

//------------------------------------------------------------------------------
void cUnitDetailsStored::drawRow (size_t index, eUnitDataSymbolType symbolType, int amount, int maximalAmount, const std::string& name)
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
	cUnitDetailsHud::drawSmallSymbols (surface.get(), rowHeight, symbolType, cPosition (65, 4 + rowHeight * index), amount, maximalAmount);
}
