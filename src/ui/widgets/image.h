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

#ifndef ui_widgets_imageH
#define ui_widgets_imageH

#include "SDLutility/uniquesurface.h"
#include "resources/sound.h"
#include "ui/widgets/clickablewidget.h"
#include "utility/signal/signal.h"

class cImage : public cClickableWidget
{
public:
	cImage (const cPosition&, SDL_Surface* image = nullptr, cSoundChunk* clickSound = nullptr);

	void setImage (SDL_Surface* image);

	void draw (SDL_Surface& destination, const cBox<cPosition>& clipRect) override;
	bool isAt (const cPosition&) const override;

	cSignal<void()> clicked;

	void disableAtTransparent();
	void enableAtTransparent();

protected:
	bool handleClicked (cApplication&, cMouse&, eMouseButtonType) override;

private:
	UniqueSurface image;
	cSoundChunk* clickSound = nullptr;
	bool disabledAtTransparent = false;
};

#endif
