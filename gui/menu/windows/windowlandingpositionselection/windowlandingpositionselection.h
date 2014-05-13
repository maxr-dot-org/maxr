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

#ifndef gui_menu_windows_windowlandingpositionselection_windowlandingpositionselectionH
#define gui_menu_windows_windowlandingpositionselection_windowlandingpositionselectionH

#include <memory>

#include "../../../window.h"
#include "../../../../utility/signal/signalconnectionmanager.h"
#include "../../../../utility/signal/signal.h"
#include "../../../../game/logic/landingpositionstate.h"

class cPushButton;
class cLabel;
class cPosition;
class cLandingPositionSelectionMap;
class cStaticMap;

struct sTerrain;


class cWindowLandingPositionSelection : public cWindow
{
public:
	cWindowLandingPositionSelection (std::shared_ptr<cStaticMap> map);
	~cWindowLandingPositionSelection ();

	const cPosition& getSelectedPosition () const;

	void applyReselectionState (eLandingPositionState state);

	void setInfoMessage (const std::string& message);

	void allowSelection ();
	void disallowSelection ();

	void lockBack ();
	void unlockBack ();

	cSignal<void (const cPosition&)> selectedPosition;

	virtual void handleActivated (cApplication& application) MAXR_OVERRIDE_FUNCTION;

	virtual bool handleMouseMoved (cApplication& application, cMouse& mouse, const cPosition& offset) MAXR_OVERRIDE_FUNCTION;
private:
	cSignalConnectionManager signalConnectionManager;

	bool selectionAllowed;

	bool firstActivate;

	cPushButton* backButton;
	cLabel* infoLabel;

	cLandingPositionSelectionMap* map;

	cPosition lastSelectedPosition;

	SDL_Surface* createHudSurface ();

	void backClicked ();
	void mapClicked (const cPosition& tilePosition);
};

#endif // gui_menu_windows_windowlandingpositionselection_windowlandingpositionselectionH
