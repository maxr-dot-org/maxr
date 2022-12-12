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

#ifndef ui_graphical_menu_windows_windownetworklobby_windownetworklobbyH
#define ui_graphical_menu_windows_windownetworklobby_windownetworklobbyH

#include "game/data/savegameinfo.h"
#include "ui/widgets/window.h"
#include "utility/signal/signal.h"
#include "utility/signal/signalconnectionmanager.h"

#include <memory>

class cGameSettings;
class cImage;
class cLabel;
class cLineEdit;
class cLobbyChatBoxListViewItem;
class cLobbyClient;
class cLobbyPlayerListViewItem;
class cLobbyServer;
class cPlayerBasicData;
class cPushButton;
class cStaticMap;
template <typename T>
class cListView;

class cWindowNetworkLobby : public cWindow
{
public:
	explicit cWindowNetworkLobby (const std::string title, bool disableIp);

	void retranslate() override;

	void setTitle (const std::string&);

	void bindConnections (cLobbyClient&);
	void bindConnections (cLobbyServer&);

	void addChatEntry (const std::string& playerName, const std::string& message);
	void addInfoEntry (const std::string& message);

	void addPlayer (const std::shared_ptr<cPlayerBasicData>& player);
	void removePlayer (const cPlayerBasicData& player);
	void removePlayers();

	void setStaticMap (std::shared_ptr<cStaticMap> staticMap);
	void setGameSettings (std::unique_ptr<cGameSettings> gameSettings);
	void setSaveGame (const cSaveGameInfo&);

	void setMapDownloadPercent (int percent);
	void setMapDownloadCanceled();

	const std::shared_ptr<cStaticMap>& getStaticMap() const;
	const std::shared_ptr<cGameSettings>& getGameSettings() const;
	const cSaveGameInfo& getSaveGameInfo() const;
	const std::shared_ptr<cPlayerBasicData>& getLocalPlayer() const;
	std::vector<std::shared_ptr<cPlayerBasicData>> getPlayers() const;
	std::vector<cPlayerBasicData> getPlayersNotShared() const;
	std::shared_ptr<cPlayerBasicData> getPlayer (int i) const;

	unsigned short getPort() const;
	const std::string& getIp() const;

	const std::string& getChatMessage() const;

	void disablePortEdit();
	void disableIpEdit();
	void enablePortEdit();
	void enableIpEdit();

	void updatePlayerList (const cPlayerBasicData&, const std::vector<cPlayerBasicData>&);
	void updatePlayerListView();

	cSignal<void()> triggeredSelectMap;
	cSignal<void()> triggeredSelectSettings;
	cSignal<void()> triggeredSelectSaveGame;

	cSignal<void()> wantLocalPlayerReadyChange;
	cSignal<void()> triggeredChatMessage;

	cSignal<void()> triggeredStartGame;
	cSignal<void()> backClicked;

protected:
	cSignalConnectionManager signalConnectionManager;

	cLabel* titleLabel = nullptr;
	cPushButton* sendButton = nullptr;
	cImage* mapImage = nullptr;
	cLabel* mapNameLabel = nullptr;

	cLabel* settingsTextLabel = nullptr;

	cLineEdit* chatLineEdit = nullptr;
	cListView<cLobbyChatBoxListViewItem>* chatList = nullptr;

	cPushButton* mapButton = nullptr;
	cPushButton* settingsButton = nullptr;
	cPushButton* loadButton = nullptr;

	cListView<cLobbyPlayerListViewItem>* playersList = nullptr;

	cLabel* ipLabel = nullptr;
	cLabel* portLabel = nullptr;
	cLabel* playerNameLabel = nullptr;
	cLabel* colorLabel = nullptr;

	cLineEdit* ipLineEdit = nullptr;
	cLineEdit* portLineEdit = nullptr;
	cImage* restoreDefaultPortButton = nullptr;

	cPushButton* okButton = nullptr;
	cPushButton* backButton = nullptr;

protected:
	std::shared_ptr<cPlayerBasicData> localPlayer;
	std::shared_ptr<cStaticMap> staticMap;
	std::shared_ptr<cGameSettings> gameSettings;
	cSaveGameInfo saveGameInfo;

	void updateSettingsText();
	void updateMap();

	void triggerChatMessage (bool keepFocus);
};

#endif // ui_graphical_menu_windows_windownetworklobby_windownetworklobbyH
