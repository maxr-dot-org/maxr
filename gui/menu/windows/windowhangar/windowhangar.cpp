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
#include "../../../../main.h"
#include "../../../../vehicles.h"
#include "../../../../buildings.h"
#include "../../../../player.h"
#include "../../widgets/label.h"
#include "../../widgets/pushbutton.h"
#include "../../widgets/image.h"
#include "../../widgets/checkbox.h"
#include "../../widgets/listview.h"
#include "../../widgets/special/unitlistviewitembuy.h"
#include "../../widgets/special/unitdetails.h"

//------------------------------------------------------------------------------
cWindowHangar::cWindowHangar (SDL_Surface* surface, int playerColor, int playerClan) :
	cWindow (surface),
	temporaryPlayer (new cPlayer (sPlayer ("unnamed", playerColor, 0))),
	player (*temporaryPlayer)
{
	if (playerClan != -1) temporaryPlayer->setClan (playerClan);

	initialize ();
}

//------------------------------------------------------------------------------
cWindowHangar::cWindowHangar (SDL_Surface* surface, const cPlayer& player_) :
	cWindow (surface),
	player (player_)
{
	initialize ();
}

//------------------------------------------------------------------------------
void cWindowHangar::initialize ()
{
	infoImage = addChild (std::make_unique<cImage> (getPosition () + cPosition (11, 13)));

	infoLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (21, 23), getPosition () + cPosition (21 + 280, 23 + 220)), "", FONT_LATIN_NORMAL, eAlignmentType::Left));
	infoLabel->setWordWrap (true);

	infoTextCheckBox = addChild (std::make_unique<cCheckBox> (getPosition () + cPosition (291, 264), lngPack.i18n ("Text~Comp~Description"), FONT_LATIN_NORMAL, eCheckBoxTextAnchor::Left));
	infoTextCheckBox->setChecked (true);
	signalConnectionManager.connect (infoTextCheckBox->toggled, std::bind (&cWindowHangar::infoCheckBoxToggled, this));

	unitDetails = addChild (std::make_unique<cUnitDetails> (getPosition () + cPosition (16, 297)));


	using namespace std::placeholders;

	selectionUnitList = addChild (std::make_unique<cListView<cUnitListViewItemBuy>> (cBox<cPosition> (getPosition () + cPosition (477, 50), getPosition () + cPosition (477 + 154, 50 + 326))));
	signalConnectionManager.connect (selectionUnitList->itemClicked, std::bind (&cWindowHangar::selectionUnitClicked, this, _1));
	signalConnectionManager.connect (selectionUnitList->selectionChanged, std::bind (&cWindowHangar::handleSelectionChanged, this));

	selectionListUpButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (471, 387), ePushButtonType::ArrowUpSmall, SoundData.SNDObjectMenu));
	signalConnectionManager.connect (selectionListUpButton->clicked, std::bind (&cListView<cUnitListViewItemBuy>::pageUp, selectionUnitList));

	selectionListDownButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (491, 387), ePushButtonType::ArrowDownSmall, SoundData.SNDObjectMenu));
	signalConnectionManager.connect (selectionListDownButton->clicked, std::bind (&cListView<cUnitListViewItemBuy>::pageDown, selectionUnitList));


	okButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (447, 452), ePushButtonType::Angular, lngPack.i18n ("Text~Others~Done"), FONT_LATIN_NORMAL));
	signalConnectionManager.connect (okButton->clicked, std::bind (&cWindowHangar::okClicked, this));

	backButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (349, 452), ePushButtonType::Angular, lngPack.i18n ("Text~Others~Back"), FONT_LATIN_NORMAL));
	signalConnectionManager.connect (backButton->clicked, std::bind (&cWindowHangar::backClicked, this));
}

//------------------------------------------------------------------------------
cWindowHangar::~cWindowHangar ()
{}

//------------------------------------------------------------------------------
void cWindowHangar::okClicked ()
{
	done ();
}

//------------------------------------------------------------------------------
void cWindowHangar::backClicked ()
{
	close ();
}

//------------------------------------------------------------------------------
void cWindowHangar::infoCheckBoxToggled ()
{
	if (infoTextCheckBox->isChecked ()) infoLabel->show ();
	else infoLabel->hide ();
}

//------------------------------------------------------------------------------
void cWindowHangar::setActiveUnit (const sID& unitId)
{
	if (unitId.isAVehicle ())
	{
		const auto& uiData = *UnitsData.getVehicleUI (unitId);

		infoImage->setImage (uiData.info);
	}
	else if (unitId.isABuilding ())
	{
		const auto& uiData = *UnitsData.getBuildingUI (unitId);

		infoImage->setImage (uiData.info);
	}

	infoLabel->setText (unitId.getUnitDataOriginalVersion ()->description);

	unitDetails->setUnit (unitId, getPlayer());
}

//------------------------------------------------------------------------------
const sID* cWindowHangar::getActiveUnit () const
{
	return unitDetails->getCurrentUnitId ();
}

//------------------------------------------------------------------------------
void cWindowHangar::handleSelectionChanged ()
{
	auto selectedItem = selectionUnitList->getSelectedItem ();
	if (selectedItem != nullptr)
	{
		setActiveUnit (selectedItem->getUnitId ());
	}
	selectionUnitSelectionChanged (selectedItem);
}

//------------------------------------------------------------------------------
cUnitListViewItemBuy& cWindowHangar::addSelectionUnit (const sID& unitId)
{
	auto selectedItem = selectionUnitList->addItem (std::make_unique<cUnitListViewItemBuy> (selectionUnitList->getSize ().x () - 9, unitId, getPlayer()));
	return *selectedItem;
}

//------------------------------------------------------------------------------
void cWindowHangar::setSelectedSelectionItem (const cUnitListViewItemBuy& item)
{
	selectionUnitList->setSelectedItem (&item);
}

//------------------------------------------------------------------------------
void cWindowHangar::clearSelectionUnits ()
{
	selectionUnitList->clearItems ();
}

//------------------------------------------------------------------------------
const cPlayer& cWindowHangar::getPlayer () const
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
