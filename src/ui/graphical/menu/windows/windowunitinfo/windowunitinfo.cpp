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

#include "windowunitinfo.h"

#include "game/data/units/building.h"
#include "game/data/units/vehicle.h"
#include "resources/buildinguidata.h"
#include "resources/pcx.h"
#include "resources/uidata.h"
#include "resources/vehicleuidata.h"
#include "ui/graphical/menu/widgets/pushbutton.h"
#include "ui/graphical/menu/widgets/special/unitdetails.h"
#include "ui/translations.h"
#include "ui/uidefines.h"
#include "ui/widgets/image.h"
#include "ui/widgets/label.h"
#include "utility/language.h"

//------------------------------------------------------------------------------
cWindowUnitInfo::cWindowUnitInfo (const cDynamicUnitData& currentUnitData, const cPlayer* owner, const cUnitsData& unitsData) :
	cWindow (LoadPCX (GFXOD_HELP), eWindowBackgrounds::Black)
{
	titleLabel = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (328, 12), getPosition() + cPosition (328 + 157, 12 + 10)), lngPack.i18n ("Title~Unitinfo"), eUnicodeFontType::LatinNormal, eAlignmentType::CenterHorizontal);

	auto infoImage = emplaceChild<cImage> (getPosition() + cPosition (11, 13));

	auto infoLabel = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (344, 67), getPosition() + cPosition (344 + 279, 67 + 176)), "", eUnicodeFontType::LatinNormal, eAlignmentType::Left);
	infoLabel->setWordWrap (true);

	auto unitDetails = emplaceChild<cUnitDetails> (getPosition() + cPosition (16, 297));

	auto okButton = emplaceChild<cPushButton> (getPosition() + cPosition (447, 452), ePushButtonType::Angular, lngPack.i18n ("Others~Done"), eUnicodeFontType::LatinNormal);
	okButton->addClickShortcut (cKeySequence (cKeyCombination (eKeyModifierType::None, SDLK_RETURN)));
	signalConnectionManager.connect (okButton->clicked, [this]() { close(); });

	if (currentUnitData.getId().isAVehicle())
	{
		const auto& uiData = *UnitsUiData.getVehicleUI (currentUnitData.getId());

		infoImage->setImage (uiData.info.get());
	}
	else if (currentUnitData.getId().isABuilding())
	{
		const auto& uiData = *UnitsUiData.getBuildingUI (currentUnitData.getId());

		infoImage->setImage (uiData.info.get());
	}

	infoLabel->setText (getStaticUnitDescription (unitsData.getStaticUnitData (currentUnitData.getId())));

	unitDetails->setUnit (currentUnitData.getId(), owner, unitsData, &currentUnitData);
}

//------------------------------------------------------------------------------
void cWindowUnitInfo::retranslate()
{
	cWindow::retranslate();
	titleLabel->setText (lngPack.i18n ("Title~Unitinfo"));
	okButton->setText (lngPack.i18n ("Others~Done"));
}
