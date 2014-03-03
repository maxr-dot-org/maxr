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

#include "unitvideowidget.h"

#include "../temp/animationtimer.h"

#include "../../menu/widgets/image.h"

#include "../../../vehicles.h"
#include "../../../buildings.h"
#include "../../../files.h"

//------------------------------------------------------------------------------
cUnitVideoWidget::cUnitVideoWidget (const cBox<cPosition>& area, std::shared_ptr<cAnimationTimer> animationTimer) :
	cWidget (area),
	fliAnimation (nullptr, FLI_Close),
	playing (true)
{
	currentFrameImage = addChild (std::make_unique<cImage> (getPosition ()));

	signalConnectionManager.connect (animationTimer->triggered100ms, std::bind (&cUnitVideoWidget::nextFrame, this));
}

//------------------------------------------------------------------------------
void cUnitVideoWidget::start ()
{
	playing = true;
}

//------------------------------------------------------------------------------
void cUnitVideoWidget::stop ()
{
	playing = false;
}

//------------------------------------------------------------------------------
void cUnitVideoWidget::setUnit (const cUnit* unit)
{
	if (!unit)
	{
		fliAnimation = nullptr;
		currentFrameImage->setImage (nullptr);
	}
	else
	{
		if (unit->data.ID.isAVehicle ())
		{
			const auto& vehicle = *static_cast<const cVehicle*>(unit);
			if (FileExists (vehicle.uiData->FLCFile))
			{
				fliAnimation = FliAnimationPointerType (FLI_Open (SDL_RWFromFile (vehicle.uiData->FLCFile, "rb"), NULL), FLI_Close);
				FLI_Rewind (fliAnimation.get());
				FLI_NextFrame (fliAnimation.get ());
				currentFrameImage->setImage (fliAnimation->surface);
			}
			else
			{
				fliAnimation = nullptr;
				currentFrameImage->setImage (vehicle.uiData->storage);
			}
		}
		else if (unit->data.ID.isABuilding ())
		{
			const auto& building = *static_cast<const cBuilding*>(unit);

			fliAnimation = nullptr;
			currentFrameImage->setImage (building.uiData->video);
		}
	}
}

//------------------------------------------------------------------------------
void cUnitVideoWidget::nextFrame ()
{
	if (!playing) return;
	if (!fliAnimation) return;

	FLI_NextFrame (fliAnimation.get());
	currentFrameImage->setImage (fliAnimation->surface);
}
