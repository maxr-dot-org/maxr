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

#ifndef ui_graphical_game_gameguiH
#define ui_graphical_game_gameguiH

#include <array>

#include "ui/graphical/window.h"
#include "utility/signal/signal.h"
#include "utility/signal/signalconnectionmanager.h"
#include "resources/sound.h"
#include "game/logic/upgradecalculator.h"
#include "ui/graphical/game/unitselection.h"

class cHud;
class cGameMapWidget;
class cMiniMapWidget;
class cGameMessageListView;
template<typename, typename> class cChatBox;
class cLobbyChatBoxListViewItem;
class cChatBoxPlayerListViewItem;
class cLabel;
class cDebugOutputWidget;
class cStaticMap;
class cMap;
class cMapView;
class cPlayer;
class cUnit;
class cVehicle;
class cBuilding;
class cAnimationTimer;
class cClient;
class cHudPanels;
class cSavedReport;
class cTurnCounter;
class cTurnTimeClock;
class cGameSettings;
class cSoundManager;
class cSoundEffect;
class cGameGuiState;
struct sID;
class cBuildListItem;
class cFrameCounter;
class cUnitsData;

class cGameGui : public cWindow
{
public:
	cGameGui (std::shared_ptr<const cStaticMap> staticMap, std::shared_ptr<cSoundManager> soundManager, std::shared_ptr<cAnimationTimer> animationTimer, std::shared_ptr<const cFrameCounter> frameCounter);

	void setMapView(std::shared_ptr<const cMapView> mapView);
	void setPlayer (std::shared_ptr<const cPlayer> player);
	void setPlayers (std::vector<std::shared_ptr<const cPlayer>> players);
	void setTurnClock (std::shared_ptr<const cTurnCounter> turnClock);
	void setTurnTimeClock (std::shared_ptr<const cTurnTimeClock> turnTimeClock);
	void setGameSettings (std::shared_ptr<const cGameSettings> gameSettings);
	void setUnitsData(std::shared_ptr<const cUnitsData> unitsData);

	cHud& getHud();
	const cHud& getHud() const;

	cGameMapWidget& getGameMap();
	const cGameMapWidget& getGameMap() const;

	cMiniMapWidget& getMiniMap();
	const cMiniMapWidget& getMiniMap() const;

	cChatBox<cLobbyChatBoxListViewItem, cChatBoxPlayerListViewItem>& getChatBox();
	const cChatBox<cLobbyChatBoxListViewItem, cChatBoxPlayerListViewItem>& getChatBox() const;

	cGameMessageListView& getGameMessageList();
	const cGameMessageListView& getGameMessageList() const;

	cDebugOutputWidget& getDebugOutput();

	void setInfoTexts (const std::string& primiaryText, const std::string& additionalText);

	void exit();

	cGameGuiState getCurrentState() const;
	void restoreState (const cGameGuiState& state);

	virtual bool handleMouseMoved (cApplication& application, cMouse& mouse, const cPosition& offset) MAXR_OVERRIDE_FUNCTION;
	virtual bool handleMouseWheelMoved (cApplication& application, cMouse& mouse, const cPosition& amount) MAXR_OVERRIDE_FUNCTION;

	virtual void handleActivated (cApplication& application, bool firstTime) MAXR_OVERRIDE_FUNCTION;
	virtual void handleDeactivated (cApplication& application, bool removed) MAXR_OVERRIDE_FUNCTION;

	virtual bool wantsCentered() const MAXR_OVERRIDE_FUNCTION;
protected:

	virtual std::unique_ptr<cMouseCursor> getDefaultCursor() const MAXR_OVERRIDE_FUNCTION;
private:
	cSignalConnectionManager signalConnectionManager;
	cSignalConnectionManager panelSignalConnectionManager;
	cSignalConnectionManager selectedUnitConnectionManager;

	std::shared_ptr<cAnimationTimer> animationTimer;
	std::shared_ptr<cSoundManager> soundManager;

	std::shared_ptr<const cStaticMap> staticMap;
	std::shared_ptr<const cMapView> mapView;
	std::shared_ptr<const cPlayer> player;

	cHud* hud;
	cHudPanels* hudPanels;
	cGameMapWidget* gameMap;
	cMiniMapWidget* miniMap;
	cGameMessageListView* messageList;
	cChatBox<cLobbyChatBoxListViewItem, cChatBoxPlayerListViewItem>* chatBox;
	cDebugOutputWidget* debugOutput;

	cLabel* primiaryInfoLabel;
	cLabel* additionalInfoLabel;

	cPosition mouseScrollDirection;

	std::shared_ptr<cSoundEffect> selectedUnitSoundLoop;

	bool openPanelOnActivation;

	void startOpenPanel();
	void startClosePanel();

	void resetMiniMapViewWindow();

	void updateHudCoordinates (const cPosition& tilePosition);
	void updateHudUnitName (const cPosition& tilePosition);

	void connectSelectedUnit();

	void initShortcuts();

	void handleResolutionChange();

	void updateSelectedUnitSound();
	void updateSelectedUnitIdleSound();
	void updateSelectedUnitMoveSound (bool startedNew);

	void startSelectedUnitSound (const cUnit& unit, const cSoundChunk& sound);
	void stopSelectedUnitSound();
};

#endif // ui_graphical_game_gameguiH
