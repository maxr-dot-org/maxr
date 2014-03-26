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

#ifndef gui_menu_windows_windownetworklobby_windownetworklobbyH
#define gui_menu_windows_windownetworklobby_windownetworklobbyH

#include <memory>

#include "../../../window.h"
#include "../../../../utility/signal/signalconnectionmanager.h"

class cLabel;
class cImage;
class cLineEdit;
template<typename T> class cListView;
class cLobbyChatBoxListViewItem;
class cLobbyPlayerListViewItem;
class sPlayer;
class cStaticMap;
class cGameSettings;
class cTCP;

class cWindowNetworkLobby : public cWindow
{
public:
	explicit cWindowNetworkLobby (const std::string title, bool disableIp);

	void addChatEntry (const std::string& playerName, const std::string& message);
	void addInfoEntry (const std::string& message);

	void addPlayer (const std::shared_ptr<sPlayer>& player);

	cTCP& getNetwork ();

	const std::shared_ptr<cStaticMap>& getStaticMap () const;
	const std::shared_ptr<cGameSettings>& getGameSettings () const;
	int getSaveGameNumber () const;

	const std::shared_ptr<sPlayer>& getLocalPlayer () const;
	std::vector<std::shared_ptr<sPlayer>> getPlayers () const;
protected:
	void setStaticMap (std::shared_ptr<cStaticMap> staticMap);
	void setGameSettings (std::unique_ptr<cGameSettings> gameSettings);
	void setSaveGame (int saveGameNumber);

	unsigned short getPort () const;
	const std::string& getIp () const;

	void disablePortEdit ();
	void disableIpEdit ();

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

	std::shared_ptr<sPlayer> player;
	std::shared_ptr<cStaticMap> staticMap;
	std::shared_ptr<cGameSettings> gameSettings;

	std::shared_ptr<cTCP> network;

	int saveGameNumber;
	std::string saveGameName;
	std::string saveGamePlayers;

	void updateSettingsText ();
	void updateMap ();
	void updatePlayerColor ();

	void sendChatMessage (bool refocusChatLine);
};

#endif // gui_menu_windows_windownetworklobby_windownetworklobbyH
