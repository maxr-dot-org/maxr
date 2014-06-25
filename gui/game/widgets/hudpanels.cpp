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

#include <algorithm>

#include "hudpanels.h"
#include "../../../sound.h"
#include "../../../video.h"
#include "../../../main.h"
#include "../../../output/sound/sounddevice.h"
#include "../../../output/sound/soundchannel.h"
#include "../temp/animationtimer.h"

//------------------------------------------------------------------------------
cHudPanels::cHudPanels (const cPosition& position, int height, std::shared_ptr<cAnimationTimer> animationTimer_, double percentClosed_) :
	cWidget (position),
	animationTimer (std::move (animationTimer_)),
    openStep (100. * 10 / (SoundData.SNDPanelOpen.empty () ? 800 : SoundData.SNDPanelOpen.getLength ().count () * 0.95)),
    closeStep (100. * 10 / (SoundData.SNDPanelClose.empty () ? 800 : SoundData.SNDPanelClose.getLength ().count () * 0.95)),
	percentClosed (percentClosed_)
{
	percentClosed = std::max (0., percentClosed);
	percentClosed = std::min (100., percentClosed);

	assert (animationTimer != nullptr);

	resize (cPosition(GraphicsData.gfx_panel_top->w, height));
}

//------------------------------------------------------------------------------
void cHudPanels::open ()
{
    cSoundDevice::getInstance ().getFreeSoundEffectChannel ().play (SoundData.SNDPanelOpen);

	signalConnectionManager.connect (animationTimer->triggered10ms, std::bind (&cHudPanels::doOpenStep, this));
}

//------------------------------------------------------------------------------
void cHudPanels::close ()
{
    cSoundDevice::getInstance ().getFreeSoundEffectChannel ().play (SoundData.SNDPanelClose);

	signalConnectionManager.connect (animationTimer->triggered10ms, std::bind (&cHudPanels::doCloseStep, this));
}

//------------------------------------------------------------------------------
void cHudPanels::draw ()
{
	SDL_Rect top = {getPosition ().x (), getPosition ().y () + (getSize ().y () / 2) - GraphicsData.gfx_panel_top->h, GraphicsData.gfx_panel_top->w, GraphicsData.gfx_panel_top->h};
	SDL_Rect bottom = {getPosition ().x (), getPosition ().y () + (getSize ().y () / 2), GraphicsData.gfx_panel_bottom->w, GraphicsData.gfx_panel_top->h};

	const auto offset = (int)((getSize ().y () / 2) * (100. - percentClosed) / 100);

	top.y -= offset;
	bottom.y += offset;

	SDL_BlitSurface (GraphicsData.gfx_panel_top.get (), NULL, cVideo::buffer, &top);
	SDL_BlitSurface (GraphicsData.gfx_panel_bottom.get (), NULL, cVideo::buffer, &bottom);
}

//------------------------------------------------------------------------------
void cHudPanels::doOpenStep ()
{
	percentClosed -= openStep;

	if (percentClosed <= 0.)
	{
		percentClosed = std::max (0., percentClosed);
		signalConnectionManager.disconnectAll ();
		opened ();
	}
}

//------------------------------------------------------------------------------
void cHudPanels::doCloseStep ()
{
    if (percentClosed >= 100.)
    {
        signalConnectionManager.disconnectAll ();
        closed ();
        return;
    }

	percentClosed += closeStep;

	if (percentClosed >= 100.)
	{
		percentClosed = std::min (100., percentClosed);
	}
}
