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

#include "windowmain.h"

#include "game/data/units/building.h"
#include "game/data/units/vehicle.h"
#include "maxrversion.h"
#include "resources/buildinguidata.h"
#include "resources/pcx.h"
#include "resources/uidata.h"
#include "resources/vehicleuidata.h"
#include "ui/uidefines.h"
#include "ui/widgets/image.h"
#include "ui/widgets/label.h"
#include "utility/language.h"
#include "utility/random.h"

#include <functional>

//------------------------------------------------------------------------------
cWindowMain::cWindowMain (const std::string& title) :
	cWindow (LoadPCX (GFXOD_MAIN))
{
	titleLabel = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (0, 147), getPosition() + cPosition (getArea().getMaxCorner().x(), 157)), title, eUnicodeFontType::LatinNormal, eAlignmentType::CenterHorizontal);

	creditLabel = emplaceChild<cLabel> (cBox<cPosition> (getPosition() + cPosition (0, 465), getPosition() + cPosition (getArea().getMaxCorner().x(), 475)), lngPack.i18n ("Main~Credits_Reloaded") + " " + PACKAGE_VERSION, eUnicodeFontType::LatinNormal, eAlignmentType::CenterHorizontal);

	infoImage = emplaceChild<cImage> (getPosition() + cPosition (16, 182), getRandomInfoImage(), &SoundData.SNDHudButton);
	signalConnectionManager.connect (infoImage->clicked, [this]() { infoImageClicked(); });
}

//------------------------------------------------------------------------------
cWindowMain::~cWindowMain()
{}

//------------------------------------------------------------------------------
void cWindowMain::retranslate()
{
	creditLabel->setText (lngPack.i18n ("Main~Credits_Reloaded") + " " + PACKAGE_VERSION);
}

//------------------------------------------------------------------------------
void cWindowMain::setTitle (const std::string& title)
{
	titleLabel->setText (title);
}

//------------------------------------------------------------------------------
void cWindowMain::infoImageClicked()
{
	infoImage->setImage (getRandomInfoImage());
}

//------------------------------------------------------------------------------
SDL_Surface* cWindowMain::getRandomInfoImage()
{
	int const showBuilding = random (3);
	// I want 3 possible random numbers since a chance of 50:50 is boring
	// (and vehicles are way more cool so I prefer them to be shown) -- beko
	static int lastUnitShow = -1;
	int unitShow = -1;
	SDL_Surface* surface = nullptr;

	if (showBuilding == 1 && UnitsUiData.buildingUIs.size() > 0)
	{
		// that's a 33% chance that we show a building on 1
		do
		{
			unitShow = random (UnitsUiData.buildingUIs.size() - 1);
			// make sure we don't show same unit twice
		} while (unitShow == lastUnitShow && UnitsUiData.buildingUIs.size() > 1);
		surface = UnitsUiData.buildingUIs[unitShow].info.get();
	}
	else if (UnitsUiData.vehicleUIs.size() > 0)
	{
		// and a 66% chance to show a vehicle on 0 or 2
		do
		{
			unitShow = random (UnitsUiData.vehicleUIs.size() - 1);
			// make sure we don't show same unit twice
		} while (unitShow == lastUnitShow && UnitsUiData.vehicleUIs.size() > 1);
		surface = UnitsUiData.vehicleUIs[unitShow].info.get();
	}
	else
		surface = nullptr;
	lastUnitShow = unitShow; //store shown unit
	return surface;
}
