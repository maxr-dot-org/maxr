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

#include "dialogtransfer.h"

#include "dialogok.h"
#include "../widgets/label.h"
#include "../widgets/pushbutton.h"
#include "../widgets/image.h"
#include "../../application.h"
#include "../../../pcx.h"
#include "../../../main.h"
#include "../../../unit.h"
#include "../../../buildings.h"
#include "../../../vehicles.h"
#include "../../../map.h"
#include "../../../video.h"
#include "../../../base.h"

//------------------------------------------------------------------------------
cNewDialogTransfer::cNewDialogTransfer (const cUnit& sourceUnit, const cUnit& destinationUnit) :
	cWindow (LoadPCX (GFXOD_DIALOG_TRANSFER), eWindowBackgrounds::Alpha),
	resourceType (getCommonResourceType (sourceUnit, destinationUnit))
{
	transferLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (142, 48), getPosition () + cPosition (142+32, 48+15)), "", FONT_LATIN_BIG, eAlignmentType::CenterHorizontal));

	arrowImage = addChild (std::make_unique<cImage> (getPosition () + cPosition (140, 77)));

	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (20, 105), getPosition () + cPosition (40+80, 105+10)), sourceUnit.data.name, FONT_LATIN_SMALL_WHITE, eAlignmentType::CenterHorizontal));
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (190, 105), getPosition () + cPosition (210+80, 105+10)), destinationUnit.data.name, FONT_LATIN_SMALL_WHITE, eAlignmentType::CenterHorizontal));

	sourceUnitCargoLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (18, 60), getPosition () + cPosition (18+20, 60+10)), "", FONT_LATIN_SMALL_WHITE, eAlignmentType::CenterHorizontal));
	destinationUnitCargoLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (272, 60), getPosition () + cPosition (272+20, 60+10)), "", FONT_LATIN_SMALL_WHITE, eAlignmentType::CenterHorizontal));

	auto sourceUnitImage = addChild (std::make_unique<cImage> (getPosition () + cPosition (39, 26)));
	auto destinationUnitImage = addChild (std::make_unique<cImage> (getPosition () + cPosition (208, 26)));

	resourceBar = addChild (std::make_unique<cResourceBar> (cBox<cPosition> (getPosition () + cPosition (43, 159), getPosition () + cPosition (43+223, 159+16)), 0, 100, getResourceBarType (sourceUnit, destinationUnit), eOrientationType::Horizontal));
	signalConnectionManager.connect (resourceBar->valueChanged, std::bind (&cNewDialogTransfer::transferValueChanged, this));
	auto increaseButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (279, 159), ePushButtonType::ArrowRightSmall));
	signalConnectionManager.connect (increaseButton->clicked, [&](){ resourceBar->increase (1); });
	auto decreaseButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (17, 159), ePushButtonType::ArrowLeftSmall));
	signalConnectionManager.connect (decreaseButton->clicked, [&](){ resourceBar->decrease (1); });

	auto doneButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (159, 200), ePushButtonType::Angular, lngPack.i18n ("Text~Others~Done"), FONT_LATIN_NORMAL));
	doneButton->addClickShortcut (cKeySequence (cKeyCombination (eKeyModifierType::None, SDLK_RETURN)));
	signalConnectionManager.connect (doneButton->clicked, [&](){ done (); });

	auto cancelButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (71, 200), ePushButtonType::Angular, lngPack.i18n ("Text~Others~Cancel"), FONT_LATIN_NORMAL));
	cancelButton->addClickShortcut (cKeySequence (cKeyCombination (eKeyModifierType::None, SDLK_ESCAPE)));
	signalConnectionManager.connect (cancelButton->clicked, [&](){ close (); });

	initUnitImage (*sourceUnitImage, sourceUnit);
	initUnitImage (*destinationUnitImage, destinationUnit);

	initCargo (sourceCargo, sourceMaxCargo, sourceUnit, destinationUnit);
	initCargo (destinationCargo, destinationMaxCargo, destinationUnit, sourceUnit);

	resourceBar->setMinValue (0);
	resourceBar->setMaxValue (destinationMaxCargo);

	auto maxTransferToDestination = std::min (destinationCargo + sourceCargo, destinationMaxCargo) - destinationCargo;
	auto maxTransferToSource = std::min (sourceCargo + destinationCargo, sourceMaxCargo) - sourceCargo;

	resourceBar->setFixedMinValue (destinationCargo - maxTransferToSource);
	resourceBar->setFixedMaxValue (destinationCargo + maxTransferToDestination);

	resourceBar->setValue (destinationCargo + maxTransferToDestination);

	transferValueChanged ();

	signalConnectionManager.connect (sourceUnit.destroyed, std::bind (&cNewDialogTransfer::closeOnUnitDestruction, this));
	signalConnectionManager.connect (destinationUnit.destroyed, std::bind (&cNewDialogTransfer::closeOnUnitDestruction, this));
}

//------------------------------------------------------------------------------
sUnitData::eStorageResType cNewDialogTransfer::getCommonResourceType (const cUnit& sourceUnit, const cUnit& destinationUnit) const
{
	sUnitData::eStorageResType commonResourceType = sUnitData::STORE_RES_NONE;
	const auto sourceResource = sourceUnit.data.storeResType;
	const auto destinationResource = destinationUnit.data.storeResType;
	if (sourceResource == destinationResource)
	{
		commonResourceType = destinationResource;
	}
	else
	{
		if (sourceUnit.isAVehicle () && destinationUnit.isABuilding ()) commonResourceType = sourceResource;
		if (sourceUnit.isABuilding () && destinationUnit.isAVehicle ()) commonResourceType = destinationResource;
	}

	return commonResourceType;
}

//------------------------------------------------------------------------------
eResourceBarType cNewDialogTransfer::getResourceBarType (const cUnit& sourceUnit, const cUnit& destinationUnit) const
{
	switch (getCommonResourceType(sourceUnit, destinationUnit))
	{
	default:
	case sUnitData::STORE_RES_METAL:
		return eResourceBarType::MetalSlim;
	case sUnitData::STORE_RES_OIL:
		return eResourceBarType::OilSlim;
	case sUnitData::STORE_RES_GOLD:
		return eResourceBarType::GoldSlim;
	}
}

//------------------------------------------------------------------------------
void cNewDialogTransfer::initUnitImage (cImage& image, const cUnit& unit)
{
	const int unitImageWidth = 64;
	const int unitImageHeight = 64;

	const auto zoom = (float)unitImageWidth / (unit.data.isBig ? cStaticMap::tilePixelWidth*2 : cStaticMap::tilePixelWidth);

	AutoSurface unitImageSurface(SDL_CreateRGBSurface (0, unitImageWidth, unitImageHeight, Video.getColDepth (), 0, 0, 0, 0));
	SDL_FillRect (unitImageSurface.get (), NULL, 0xFF00FF);
	SDL_SetColorKey (unitImageSurface.get (), SDL_TRUE, 0xFF00FF);

	SDL_Rect dest = {0, 0, 0, 0};

	if (unit.isABuilding ())
	{
		const auto& building = static_cast<const cBuilding&>(unit);
		building.render (0, unitImageSurface.get (), dest, zoom, false, false);
	}
	else if (unit.isAVehicle ())
	{
		const auto& vehicle = static_cast<const cVehicle&>(unit);
		vehicle.render (nullptr, 0, nullptr, unitImageSurface.get (), dest, zoom, false);
	}

	image.setImage (unitImageSurface.get ());
}

//------------------------------------------------------------------------------
void cNewDialogTransfer::initCargo (int& cargo, int& maxCargo, const cUnit& unit1, const cUnit& unit2)
{
	if (unit1.isABuilding ())
	{
		const auto& building = static_cast<const cBuilding&>(unit1);

		if (unit2.isAVehicle ())
		{
			switch (unit2.data.storeResType)
			{
			default:
			case sUnitData::STORE_RES_METAL:
				maxCargo = building.SubBase->MaxMetal;
				cargo = building.SubBase->getMetal ();
				break;
			case sUnitData::STORE_RES_OIL:
				maxCargo = building.SubBase->MaxOil;
				cargo = building.SubBase->getOil ();
				break;
			case sUnitData::STORE_RES_GOLD:
				maxCargo = building.SubBase->MaxGold;
				cargo = building.SubBase->getGold ();
				break;
			}
		}
		else if (unit2.isABuilding ())
		{
			maxCargo = building.data.storageResMax;
			cargo = building.data.storageResCur;
		}
	}
	else if (unit1.isAVehicle ())
	{
		maxCargo = unit1.data.storageResMax;
		cargo = unit1.data.storageResCur;
	}
}

namespace {

// TODO: move function into a better file ?
void FlipSurfaceHorizontally (SDL_Surface* surface)
{
	assert (surface);
	if (SDL_MUSTLOCK (surface)) SDL_LockSurface (surface);

	// Assume surface format uses Uint32*
	// TODO: check surface format (or support more format).
	Uint32* p = static_cast<Uint32*> (surface->pixels);

	for (int h = 0; h != surface->h; ++h)
		for (int w = 0; w != surface->w / 2; ++w)
			std::swap (p[h * surface->w + w], p[(h + 1) * surface->w - w - 1]);

	if (SDL_MUSTLOCK (surface)) SDL_UnlockSurface (surface);
}
}


//------------------------------------------------------------------------------
int cNewDialogTransfer::getTransferValue () const
{
	return resourceBar->getValue () - destinationCargo;;
}

//------------------------------------------------------------------------------
sUnitData::eStorageResType cNewDialogTransfer::getResourceType () const
{
	return resourceType;
}

//------------------------------------------------------------------------------
void cNewDialogTransfer::transferValueChanged ()
{
	const auto transferValue = getTransferValue ();
	transferLabel->setText (iToStr (std::abs (transferValue)));

	sourceUnitCargoLabel->setText (iToStr (sourceCargo - transferValue));
	destinationUnitCargoLabel->setText (iToStr (destinationCargo + transferValue));

	if (transferValue >= 0) arrowImage->hide ();
	else
	{
		arrowImage->show ();
		// Set right to left arrow image.
		// little hack: flip part of the image that represent the arrow
		const unsigned int w = 40;
		const unsigned int h = 20;
		AutoSurface arrowSurface (SDL_CreateRGBSurface (0, w, h, Video.getColDepth (), 0, 0, 0, 0));
		const Sint16 x = arrowImage->getPosition ().x () - getPosition ().x (); // 140
		const Sint16 y = arrowImage->getPosition ().y () - getPosition ().y (); //  77
		SDL_Rect src = {x, y, w, h};
		SDL_BlitSurface (getSurface (), &src, arrowSurface.get (), NULL);
		FlipSurfaceHorizontally (arrowSurface.get ());

		arrowImage->setImage (arrowSurface.get ());
	}
}

//------------------------------------------------------------------------------
void cNewDialogTransfer::closeOnUnitDestruction ()
{
	close ();
	auto application = getActiveApplication ();
	if (application)
	{
		application->show (std::make_shared<cDialogOk> ("Unit destroyed!")); // TODO: translate
	}
}