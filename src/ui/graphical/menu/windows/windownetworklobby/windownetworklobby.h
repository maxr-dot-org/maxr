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

#include <memory>

#include "ui/graphical/window.h"
#include "utility/signal/signal.h"
#include "utility/signal/signalconnectionmanager.h"

class cLabel;
class cImage;
class cLineEdit;
template<typename T> class cListView;
class cLobbyChatBoxListViewItem;
class cLobbyPlayerListViewItem;
class cPlayerBasicData;
class cStaticMap;
class cGameSettings;
class cTCP;

class cWindowNetworkLobby : public cWindow
{
public:
	explicit cWindowNetworkLobby (const std::string title, bool disableIp);

	void addChatEntry (const std::string& playerName, const std::string& message);
	void addInfoEntry (const std::string& message);

	void addPlayer (const std::shared_ptr<cPlayerBasicData>& player);
	void removePlayer (const cPlayerBasicData& player);
	void removeNonLocalPlayers();

	void setStaticMap (std::shared_ptr<cStaticMap> staticMap);
	void setGameSettings (std::unique_ptr<cGameSettings> gameSettings);
	void setSaveGame (int saveGameNumber);

	void setMapDownloadPercent (int percent);
	void setMapDownloadCanceled();

	const std::shared_ptr<cStaticMap>& getStaticMap() const;
	const std::shared_ptr<cGameSettings>& getGameSettings() const;
	const std::vector<cPlayerBasicData>& getSaveGamePlayers() const;
	std::string getSaveGameName() const;

	const std::shared_ptr<cPlayerBasicData>& getLocalPlayer() const;
	std::vector<std::shared_ptr<cPlayerBasicData>> getPlayers() const;
	std::vector<cPlayerBasicData> getPlayersNotShared() const;

	unsigned short getPort() const;
	const std::string& getIp() const;

	const std::string& getChatMessage() const;

	void disablePortEdit();
	void disableIpEdit();

	cSignal<void ()> backClicked;
	cSignal<void ()> wantLocalPlayerReadyChange;
	cSignal<void ()> triggeredChatMessage;
	cSignal<void ()> staticMapChanged;
	cSignal<void ()> gameSettingsChanged;
	cSignal<void ()> saveGameChanged;
private:
	cSignalConnectionManager signalConnectionManager;

	cImage* mapImage;
	cLabel* mapNameLabel;

	cLabel* settingsTextLabel;

	cLineEdit* chatLineEdit;
	cListView<cLobbyChatBoxListViewItem>* chatList;

	cListView<cLobbyPlayerListViewItem>* playersList;

	cLineEdit* ipLineEdit;
	cLineEdit* portLineEdit;

	cImage* colorImage;
protected:
	std::shared_ptr<cPlayerBasicData> localPlayer;
	std::shared_ptr<cStaticMap> staticMap;
	std::shared_ptr<cGameSettings> gameSettings;

	std::string saveGameName;
	std::vector<cPlayerBasicData> saveGamePlayers;

	void updateSettingsText();
	void updateMap();
	void updatePlayerColor();

	void triggerChatMessage (bool keepFocus);
};

#endif // ui_graphical_menu_windows_windownetworklobby_windownetworklobbyH
