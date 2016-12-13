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
#ifndef game_data_savegameinfoH
#define game_data_savegameinfoH

#include <string>
#include <vector>

#include "utility/version.h"
#include "utility/serialization/serialization.h"
#include "player/playerbasicdata.h"

/**
* The Types which are possible for a game
*/
enum eGameTypes
{
	GAME_TYPE_SINGLE,  // a singleplayer game
	GAME_TYPE_HOTSEAT, // a hotseat multiplayer game
	GAME_TYPE_TCPIP    // a multiplayergame over TCP/IP
};

/**
 * this class contains all information about a save game, that are needed by the load/save and multiplayer menus.
*/
class cSaveGameInfo
{
public:
	explicit cSaveGameInfo(int number);

	// header
	cVersion saveVersion;
	std::string gameVersion;
	std::string gameName;
	/** the type of the save game (SIN, HOT, NET) */
	eGameTypes type;
	/** the date and time when this save game was saved */
	std::string date;

	// infos from model
	std::vector<cPlayerBasicData> players;
	std::string mapName;
	uint32_t mapCrc;
	//int turn;

	/** the slot number of the save game */
	int number;

	template <typename T>
	void serialize(T& archive)
	{
		archive & saveVersion;
		archive & gameVersion;
		archive & gameName;
		archive & type;
		archive & date;
		archive & players;
		archive & mapName;
		archive & mapCrc;
		//archive & turn;
		archive & number;
	}
};

#endif // game_data_savegameinfoH
