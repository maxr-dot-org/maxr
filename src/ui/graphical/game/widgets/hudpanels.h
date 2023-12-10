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

#ifndef ui_graphical_game_widgets_hudpanelsH
#define ui_graphical_game_widgets_hudpanelsH

#include "SDLutility/uniquesurface.h"
#include "ui/widgets/widget.h"
#include "utility/signal/signal.h"
#include "utility/signal/signalconnectionmanager.h"

#include <memory>

class cAnimationTimer;

class cHudPanels : public cWidget
{
public:
	cHudPanels (const cPosition&, int height, std::shared_ptr<cAnimationTimer>, double percentClosed = 100);

	void open();
	void close();

	cSignal<void()> opened;
	cSignal<void()> closed;

	void draw (SDL_Surface& destination, const cBox<cPosition>& clipRect) override;

private:
	cSignalConnectionManager signalConnectionManager;

	std::shared_ptr<cAnimationTimer> animationTimer;

	const double openStep;
	const double closeStep;

	double percentClosed;

	void doOpenStep();
	void doCloseStep();
};

#endif // ui_graphical_game_widgets_hudpanelsH
