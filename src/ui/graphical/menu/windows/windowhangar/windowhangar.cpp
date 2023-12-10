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

#include "windowhangar.h"

#include "game/data/player/player.h"
#include "game/data/player/playerbasicdata.h"
#include "game/data/units/building.h"
#include "game/data/units/vehicle.h"
#include "resources/buildinguidata.h"
#include "resources/uidata.h"
#include "resources/vehicleuidata.h"
#include "ui/graphical/menu/widgets/checkbox.h"
#include "ui/graphical/menu/widgets/listview.h"
#include "ui/graphical/menu/widgets/pushbutton.h"
#include "ui/graphical/menu/widgets/special/unitdetails.h"
#include "ui/graphical/menu/widgets/special/unitlistviewitembuy.h"
#include "ui/translations.h"
#include "ui/widgets/image.h"
#include "ui/widgets/label.h"
#include "utility/language.h"

//------------------------------------------------------------------------------
cWindowHangar::cWindowHangar (UniqueSurface surface, std::shared_ptr<const cUnitsData> unitsData, cRgbColor playerColor, int playerClan) :
	cWindow (std::move (surface)),
	unitsData (unitsData),
	temporaryPlayer (std::make_unique<cPlayer> (cPlayerBasicData ({"unnamed", playerColor}, 0, false), *unitsData)),
	player (*temporaryPlayer)
{
	if (playerClan != -1) temporaryPlayer->setClan (playerClan, *unitsData);

	initialize();
}

//------------------------------------------------------------------------------
cWindowHangar::cWindowHangar (UniqueSurface surface, std::shared_ptr<const cUnitsData> unitsData, const cPlayer& player_) :
	cWindow (std::move (surface)),
	unitsData (unitsData),
	player (player_)
{
	initialize();
}

//------------------------------------------------------------------------------
void cWindowHangar::retranslate()
{
	cWindow::retranslate();

	infoTextCheckBox->setText (lngPack.i18n ("Comp~Description"));
	okButton->setText (lngPack.i18n ("Others~Done"));
	backButton->setText (lngPack.i18n ("Others~Back"));
}

//------------------------------------------------------------------------------
void cWindowHangar::initialize()
{
	infoImage = emplaceChild<cImage> (getPosition() + cPosition (11, 13));

	infoLabel = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (21, 23), getPosition() + cPosition (21 + 280, 23 + 220)), "", eUnicodeFontType::LatinNormal, eAlignmentType::Left);
	infoLabel->setWordWrap (true);

	infoTextCheckBox = emplaceChild<cCheckBox> (getPosition() + cPosition (291, 264), lngPack.i18n ("Comp~Description"), eUnicodeFontType::LatinNormal, eCheckBoxTextAnchor::Left);
	infoTextCheckBox->setChecked (true);
	signalConnectionManager.connect (infoTextCheckBox->toggled, [this]() { infoCheckBoxToggled(); });

	unitDetails = emplaceChild<cUnitDetails> (getPosition() + cPosition (16, 297));

	selectionUnitList = emplaceChild<cListView<cUnitListViewItemBuy>> (cBox<cPosition> (getPosition() + cPosition (477, 50), getPosition() + cPosition (477 + 154, 50 + 326)));
	selectionUnitList->setEndMargin (cPosition (2, 10));
	signalConnectionManager.connect (selectionUnitList->itemClicked, [this] (cUnitListViewItemBuy& unitItem) { selectionUnitClicked (unitItem); });
	signalConnectionManager.connect (selectionUnitList->selectionChanged, [this]() { handleSelectionChanged(); });

	selectionListUpButton = emplaceChild<cPushButton> (getPosition() + cPosition (471, 387), ePushButtonType::ArrowUpSmall, &SoundData.SNDObjectMenu);
	signalConnectionManager.connect (selectionListUpButton->clicked, [this]() { selectionUnitList->pageUp(); });

	selectionListDownButton = emplaceChild<cPushButton> (getPosition() + cPosition (491, 387), ePushButtonType::ArrowDownSmall, &SoundData.SNDObjectMenu);
	signalConnectionManager.connect (selectionListDownButton->clicked, [this]() { selectionUnitList->pageDown(); });

	okButton = emplaceChild<cPushButton> (getPosition() + cPosition (447, 452), ePushButtonType::Angular, lngPack.i18n ("Others~Done"), eUnicodeFontType::LatinNormal);
	okButton->addClickShortcut (cKeySequence (cKeyCombination (SDLK_RETURN)));
	signalConnectionManager.connect (okButton->clicked, [this]() { okClicked(); });

	backButton = emplaceChild<cPushButton> (getPosition() + cPosition (349, 452), ePushButtonType::Angular, lngPack.i18n ("Others~Back"), eUnicodeFontType::LatinNormal);
	backButton->addClickShortcut (cKeySequence (cKeyCombination (SDLK_ESCAPE)));
	signalConnectionManager.connect (backButton->clicked, [this]() { backClicked(); });
}

//------------------------------------------------------------------------------
cWindowHangar::~cWindowHangar()
{}

//------------------------------------------------------------------------------
void cWindowHangar::okClicked()
{
	done();
}

//------------------------------------------------------------------------------
void cWindowHangar::backClicked()
{
	canceled();
}

//------------------------------------------------------------------------------
void cWindowHangar::infoCheckBoxToggled()
{
	if (infoTextCheckBox->isChecked())
		infoLabel->show();
	else
		infoLabel->hide();
}

//------------------------------------------------------------------------------
void cWindowHangar::setActiveUnit (const sID& unitId)
{
	if (unitId.isAVehicle())
	{
		const auto& uiData = *UnitsUiData.getVehicleUI (unitId);

		infoImage->setImage (uiData.info.get());
	}
	else if (unitId.isABuilding())
	{
		const auto& uiData = *UnitsUiData.getBuildingUI (unitId);

		infoImage->setImage (uiData.info.get());
	}

	infoLabel->setText (getStaticUnitDescription (unitsData->getStaticUnitData (unitId)));

	unitDetails->setUnit (unitId, &getPlayer(), *unitsData);
}

//------------------------------------------------------------------------------
const sID* cWindowHangar::getActiveUnit() const
{
	return unitDetails->getCurrentUnitId();
}

//------------------------------------------------------------------------------
void cWindowHangar::handleSelectionChanged()
{
	auto selectedItem = selectionUnitList->getSelectedItem();
	if (selectedItem != nullptr)
	{
		setActiveUnit (selectedItem->getUnitId());
	}
	selectionUnitSelectionChanged (selectedItem);
}

//------------------------------------------------------------------------------
cUnitListViewItemBuy& cWindowHangar::addSelectionUnit (const sID& unitId)
{
	auto selectedItem = selectionUnitList->addItem (std::make_unique<cUnitListViewItemBuy> (selectionUnitList->getSize().x() - 9, unitId, getPlayer(), *unitsData));
	return *selectedItem;
}

//------------------------------------------------------------------------------
void cWindowHangar::setSelectedSelectionItem (const cUnitListViewItemBuy& item)
{
	selectionUnitList->setSelectedItem (&item);
}

//------------------------------------------------------------------------------
void cWindowHangar::clearSelectionUnits()
{
	selectionUnitList->clearItems();
}

//------------------------------------------------------------------------------
const cPlayer& cWindowHangar::getPlayer() const
{
	return player;
}

//------------------------------------------------------------------------------
void cWindowHangar::setActiveUpgrades (const cUnitUpgrade& unitUpgrades)
{
	unitDetails->setUpgrades (&unitUpgrades);
}

//------------------------------------------------------------------------------
void cWindowHangar::selectionUnitClicked (cUnitListViewItemBuy& unitItem)
{
	if (&unitItem == selectionUnitList->getSelectedItem())
	{
		selectionUnitClickedSecondTime (unitItem);
	}
}
