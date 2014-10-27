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

#ifndef ui_graphical_menu_widgets_special_protectionglassH
#define ui_graphical_menu_widgets_special_protectionglassH

#include "ui/graphical/menu/widgets/clickablewidget.h"
#include "utility/signal/signal.h"

class cPosition;
class cAnimationTimer;

class cProtectionGlass : public cWidget
{
public:
	explicit cProtectionGlass (const cPosition& position, std::shared_ptr<cAnimationTimer> animationTimer, double percentClosed = 100);

	void open ();

	cSignal<void ()> opened;

	virtual void draw (SDL_Surface& destination, const cBox<cPosition>& clipRect) MAXR_OVERRIDE_FUNCTION;;

private:
	cSignalConnectionManager signalConnectionManager;

	std::shared_ptr<cAnimationTimer> animationTimer;

	const double openStep;

	double percentClosed;

	void doOpenStep ();
};

#endif // ui_graphical_menu_widgets_special_protectionglassH
