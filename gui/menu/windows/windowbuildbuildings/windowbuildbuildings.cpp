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

#include "windowbuildbuildings.h"
#include "../../widgets/label.h"
#include "../../widgets/pushbutton.h"
#include "../../widgets/listview.h"
#include "../../widgets/special/unitlistviewitembuy.h"
#include "../../widgets/special/buildspeedhandlerwidget.h"
#include "../../dialogs/dialogok.h"
#include "../../../application.h"
#include "../../../../pcx.h"
#include "../../../../vehicles.h"
#include "../../../../player.h"

//------------------------------------------------------------------------------
cWindowBuildBuildings::cWindowBuildBuildings (const cVehicle& vehicle_) :
	cWindowHangar (LoadPCX (GFXOD_BUILD_SCREEN), *vehicle_.owner),
	vehicle (vehicle_)
{

	addChild (std::make_unique<cLabel> (cBox<cPosition> (getPosition () + cPosition (328, 12), getPosition () + cPosition (328 + 157, 12 + 10)), lngPack.i18n ("Text~Title~Build_Vehicle"), FONT_LATIN_NORMAL, eAlignmentType::CenterHorizontal));

	speedHandler = addChild (std::make_unique<cBuildSpeedHandlerWidget> (getPosition () + cPosition (292, 345)));

	selectionUnitList->resize (cPosition(154,380));
	selectionUnitList->setItemDistance (cPosition (0, 2));

	selectionListUpButton->moveTo (getPosition () + cPosition (471, 440));
	selectionListDownButton->moveTo (getPosition () + cPosition (491, 440));

	backButton->moveTo (getPosition () + cPosition (300, 452));
	okButton->moveTo (getPosition () + cPosition (387, 452));

	if (vehicle.data.canBuildPath)
	{
		auto pathButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (338, 428), ePushButtonType::Angular, lngPack.i18n ("Text~Others~Path"), FONT_LATIN_NORMAL));
		signalConnectionManager.connect (pathButton->clicked, [&](){ donePath (); });
	}

	generateSelectionList (vehicle);

	signalConnectionManager.connect (selectionUnitClickedSecondTime, [&](const cUnitListViewItemBuy&){ done (); });

	signalConnectionManager.connect (vehicle.destroyed, std::bind (&cWindowBuildBuildings::closeOnUnitDestruction, this));
}

//------------------------------------------------------------------------------
const sID* cWindowBuildBuildings::getSelectedUnitId () const
{
	return getActiveUnit ();
}

//------------------------------------------------------------------------------
int cWindowBuildBuildings::getSelectedBuildSpeed () const
{
	return static_cast<int>(speedHandler->getBuildSpeedIndex ());
}

//------------------------------------------------------------------------------
void cWindowBuildBuildings::setActiveUnit (const sID& unitId)
{
	cWindowHangar::setActiveUnit (unitId);

	const auto& buildingData = *vehicle.owner->getUnitDataCurrentVersion (unitId);
	std::array<int, 3> turns;
	std::array<int, 3> costs;
	vehicle.calcTurboBuild (turns, costs, buildingData.buildCosts);

	speedHandler->setValues (turns, costs);
}

//------------------------------------------------------------------------------
void cWindowBuildBuildings::generateSelectionList (const cVehicle& vehicle)
{
	bool select = true;
	for (unsigned int i = 0; i < UnitsData.getNrBuildings (); ++i)
	{
		if (UnitsData.sbuildings[i].explodesOnContact) continue;

		if (vehicle.data.canBuild != UnitsData.sbuildings[i].buildAs) continue;

		auto& item = addSelectionUnit (UnitsData.sbuildings[i].ID);

		if (select)
		{
			setSelectedSelectionItem (item);
			select = false;
		}

		if (vehicle.data.storageResCur < vehicle.owner->BuildingData[i].buildCosts) item.markAsInsufficient();
	}
}

//------------------------------------------------------------------------------
void cWindowBuildBuildings::closeOnUnitDestruction ()
{
	close ();
	auto application = getActiveApplication ();
	if (application)
	{
		application->show (std::make_shared<cDialogOk> ("Unit destroyed!")); // TODO: translate
	}
}
