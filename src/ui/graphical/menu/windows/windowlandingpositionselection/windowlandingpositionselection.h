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

#include "game/logic/landingpositionstate.h"
#include "ui/widgets/window.h"
#include "utility/signal/signal.h"
#include "utility/signal/signalconnectionmanager.h"

#include <memory>

class cAnimationTimer;
class cChatBoxLandingPlayerListViewItem;
class cCheckBox;
class cImage;
class cLabel;
class cLandingPositionSelectionMap;
class cLobbyChatBoxListViewItem;
class cPlayerLandingStatus;
class cPosition;
class cPushButton;
class cStaticMap;
class cUnitsData;

struct sTerrain;
struct sLandingUnit;

template <typename, typename>
class cChatBox;

class cWindowLandingPositionSelection : public cWindow
{
public:
	cWindowLandingPositionSelection (std::shared_ptr<cStaticMap>, bool fixedBridgeHead, const std::vector<sLandingUnit>&, std::shared_ptr<const cUnitsData>, bool withChatBox);
	~cWindowLandingPositionSelection();

	void retranslate() override;

	void applyReselectionState (eLandingPositionState);

	void setInfoMessage (const std::string&);

	void addChatPlayerEntry (const cPlayerLandingStatus&);
	void removeChatPlayerEntry (int playerId);
	void addChatEntry (const std::string& playerName, const std::string& message);

	void allowSelection();
	void disallowSelection();

	void lockBack();
	void unlockBack();

	void handleActivated (cApplication&, bool firstTime) override;
	void handleDeactivated (cApplication&, bool removed) override;
	bool handleMouseMoved (cApplication&, cMouse&, const cPosition& offset) override;

	cSignal<void (const std::string&)> onCommandEntered;
	cSignal<void (const cPosition&)> selectedPosition;
	cSignal<void()> canceled;

	cSignal<void()> opened;
	cSignal<void()> closed;

private:
	UniqueSurface createHudSurface();

	void backClicked();
	void mapClicked (const cPosition& tilePosition);
	void updateLandingPositionCircles (const cPosition& tilePosition, float radiusFactor);

	void startCircleAnimation (const cPosition& tilePosition);
	void runCircleAnimation (const cPosition& tilePosition);
	cSignalConnectionManager signalConnectionManager;
	cSignalConnectionManager circleAnimationConnectionManager;

private:
	std::shared_ptr<cStaticMap> staticMap;
	std::shared_ptr<cAnimationTimer> animationTimer;
	bool selectionAllowed = true;
	eLandingPositionState reselectionState = eLandingPositionState::Unknown;

	cCheckBox* toggleChatBoxButton = nullptr;
	cPushButton* backButton = nullptr;
	cLabel* infoLabel = nullptr;
	cImage* circlesImage = nullptr;
	cChatBox<cLobbyChatBoxListViewItem, cChatBoxLandingPlayerListViewItem>* chatBox = nullptr;

	float circleAnimationState = 0.f;

	cLandingPositionSelectionMap* mapWidget = nullptr;

	cPosition lastSelectedPosition;
};

#endif // ui_graphical_menu_windows_windowlandingpositionselection_windowlandingpositionselectionH
