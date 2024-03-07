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

#include "ui/graphical/game/widgets/hudpanels.h"

#include "output/sound/soundchannel.h"
#include "output/sound/sounddevice.h"
#include "output/video/video.h"
#include "resources/sound.h"
#include "resources/uidata.h"
#include "ui/graphical/game/animations/animationtimer.h"

#include <algorithm>
#include <cassert>

//------------------------------------------------------------------------------
cHudPanels::cHudPanels (const cPosition& position, int height, std::shared_ptr<cAnimationTimer> animationTimer_, double percentClosed_) :
	cWidget (position),
	animationTimer (std::move (animationTimer_)),
	openStep (100. * 10 / (SoundData.SNDPanelOpen.empty() ? 800 : SoundData.SNDPanelOpen.getLength().count() * 0.95)),
	closeStep (100. * 10 / (SoundData.SNDPanelClose.empty() ? 800 : SoundData.SNDPanelClose.getLength().count() * 0.95)),
	percentClosed (percentClosed_)
{
	percentClosed = std::max (0., percentClosed);
	percentClosed = std::min (100., percentClosed);

	assert (animationTimer != nullptr);

	resize (cPosition (GraphicsData.gfx_panel_top->w, height));
}

//------------------------------------------------------------------------------
void cHudPanels::open()
{
	cSoundDevice::getInstance().playSoundEffect (SoundData.SNDPanelOpen);

	signalConnectionManager.connect (animationTimer->triggered10msCatchUp, [this]() { doOpenStep(); });
}

//------------------------------------------------------------------------------
void cHudPanels::close()
{
	cSoundDevice::getInstance().playSoundEffect (SoundData.SNDPanelClose);

	signalConnectionManager.connect (animationTimer->triggered10msCatchUp, [this]() { doCloseStep(); });
}

//------------------------------------------------------------------------------
void cHudPanels::draw (SDL_Surface& destination, const cBox<cPosition>&) /* override */
{
	SDL_Rect top = {getPosition().x(), getPosition().y() + (getSize().y() / 2) - GraphicsData.gfx_panel_top->h, GraphicsData.gfx_panel_top->w, GraphicsData.gfx_panel_top->h};
	SDL_Rect bottom = {getPosition().x(), getPosition().y() + (getSize().y() / 2), GraphicsData.gfx_panel_bottom->w, GraphicsData.gfx_panel_top->h};

	const auto offset = (int) ((getSize().y() / 2) * (100. - percentClosed) / 100);

	top.y -= offset;
	bottom.y += offset;

	SDL_BlitSurface (GraphicsData.gfx_panel_top.get(), nullptr, &destination, &top);
	SDL_BlitSurface (GraphicsData.gfx_panel_bottom.get(), nullptr, &destination, &bottom);
}

//------------------------------------------------------------------------------
void cHudPanels::doOpenStep()
{
	percentClosed -= openStep;

	if (percentClosed <= 0.)
	{
		percentClosed = std::max (0., percentClosed);
		signalConnectionManager.disconnectAll();
		signalConnectionManager.connect (animationTimer->triggeredFrame, [this]() {
			signalConnectionManager.disconnectAll();
			opened();
		});
	}
}

//------------------------------------------------------------------------------
void cHudPanels::doCloseStep()
{
	if (percentClosed >= 100.)
	{
		signalConnectionManager.disconnectAll();
		signalConnectionManager.connect (animationTimer->triggeredFrame, [this]() {
			signalConnectionManager.disconnectAll();
			closed();
		});
		return;
	}

	percentClosed += closeStep;

	if (percentClosed >= 100.)
	{
		percentClosed = std::min (100., percentClosed);
	}
}
