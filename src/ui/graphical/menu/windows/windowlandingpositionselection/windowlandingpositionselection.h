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

#ifndef ui_graphical_menu_windows_windowlandingpositionselection_windowlandingpositionselectionH
#define ui_graphical_menu_windows_windowlandingpositionselection_windowlandingpositionselectionH

#include <memory>

#include "ui/graphical/window.h"
#include "utility/signal/signalconnectionmanager.h"
#include "utility/signal/signal.h"
#include "game/logic/landingpositionstate.h"

class cPushButton;
class cImage;
class cLabel;
class cPosition;
class cLandingPositionSelectionMap;
class cStaticMap;
class cAnimationTimer;
template<typename, typename> class cChatBox;
class cChatBoxLandingPlayerListViewItem;
class cLobbyChatBoxListViewItem;

struct sTerrain;


class cWindowLandingPositionSelection : public cWindow
{
public:
	cWindowLandingPositionSelection (std::shared_ptr<cStaticMap> staticMap, bool withChatBox);
	~cWindowLandingPositionSelection ();

	const cPosition& getSelectedPosition () const;

	void applyReselectionState (eLandingPositionState state);

	void setInfoMessage (const std::string& message);

	cChatBox<cLobbyChatBoxListViewItem, cChatBoxLandingPlayerListViewItem>* getChatBox ();

	void allowSelection ();
	void disallowSelection ();

	void lockBack ();
	void unlockBack ();

	cSignal<void (const cPosition&)> selectedPosition;
	cSignal<void ()> canceled;

	virtual void handleActivated (cApplication& application, bool firstTime) MAXR_OVERRIDE_FUNCTION;
	virtual void handleDeactivated (cApplication& application, bool removed) MAXR_OVERRIDE_FUNCTION;

	virtual bool handleMouseMoved (cApplication& application, cMouse& mouse, const cPosition& offset) MAXR_OVERRIDE_FUNCTION;

	mutable cSignal<void ()> opened;
	mutable cSignal<void ()> closed;
private:
	cSignalConnectionManager signalConnectionManager;
	cSignalConnectionManager circleAnimationConnectionManager;

	std::shared_ptr<cStaticMap> staticMap;

	std::shared_ptr<cAnimationTimer> animationTimer;

	bool selectionAllowed;

	eLandingPositionState reselectionState;

	cPushButton* backButton;
	cLabel* infoLabel;
	cImage* circlesImage;
	cChatBox<cLobbyChatBoxListViewItem, cChatBoxLandingPlayerListViewItem>* chatBox;

	float circleAnimationState;

	cLandingPositionSelectionMap* mapWidget;

	cPosition lastSelectedPosition;

	AutoSurface createHudSurface ();

	void backClicked ();
	void mapClicked (const cPosition& tilePosition);
	void updateLandingPositionCircles (const cPosition& tilePosition, float radiusFactor);

	void startCircleAnimation (const cPosition& tilePosition);
	void runCircleAnimation (const cPosition& tilePosition);
};

#endif // ui_graphical_menu_windows_windowlandingpositionselection_windowlandingpositionselectionH
