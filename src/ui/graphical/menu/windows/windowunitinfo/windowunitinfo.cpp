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

#include "ui/graphical/menu/windows/windowunitinfo/windowunitinfo.h"
#include "ui/graphical/menu/widgets/image.h"
#include "ui/graphical/menu/widgets/label.h"
#include "ui/graphical/menu/widgets/pushbutton.h"
#include "ui/graphical/menu/widgets/special/unitdetails.h"
#include "pcx.h"
#include "game/data/units/vehicle.h"
#include "game/data/units/building.h"

//------------------------------------------------------------------------------
cWindowUnitInfo::cWindowUnitInfo (const sUnitData& unitData_, const cPlayer& owner) :
	cWindow (LoadPCX (GFXOD_HELP), eWindowBackgrounds::Black),
	unitData(unitData_)
{
	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (328, 12), getPosition() + cPosition (328 + 157, 12 + 10)), lngPack.i18n ("Text~Title~Unitinfo"), FONT_LATIN_NORMAL, eAlignmentType::CenterHorizontal));

	auto infoImage = addChild (std::make_unique<cImage> (getPosition() + cPosition (11, 13)));

	auto infoLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition() + cPosition (344, 67), getPosition() + cPosition (344 + 279, 67 + 176)), "", FONT_LATIN_NORMAL, eAlignmentType::Left));
	infoLabel->setWordWrap (true);

	auto unitDetails = addChild (std::make_unique<cUnitDetails> (getPosition() + cPosition (16, 297)));

	auto okButton = addChild (std::make_unique<cPushButton> (getPosition() + cPosition (447, 452), ePushButtonType::Angular, lngPack.i18n ("Text~Others~Done"), FONT_LATIN_NORMAL));
	okButton->addClickShortcut (cKeySequence (cKeyCombination (eKeyModifierType::None, SDLK_RETURN)));
	signalConnectionManager.connect (okButton->clicked, [&]() { close(); });

	if (unitData.ID.isAVehicle())
	{
		const auto& uiData = *UnitsData.getVehicleUI (unitData.ID);

		infoImage->setImage (uiData.info.get());
	}
	else if (unitData.ID.isABuilding())
	{
		const auto& uiData = *UnitsData.getBuildingUI (unitData.ID);

		infoImage->setImage (uiData.info.get());
	}

	infoLabel->setText (unitData.ID.getUnitDataOriginalVersion()->description);

	unitDetails->setUnit (unitData.ID, owner, &unitData);
}
