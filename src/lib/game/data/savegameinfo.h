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

#include "player/playerbasicdata.h"
#include "utility/serialization/serialization.h"
#include "utility/version.h"

#include <string>
#include <vector>

/**
* The Types which are possible for a game
*/
enum class eGameType
{
	Single, // a singleplayer game
	Hotseat, // a hotseat multiplayer game
	TcpIp // a multiplayergame over TCP/IP
};

/**
 * this class contains all information about a save game, that are needed by the load/save and multiplayer menus.
*/
class cSaveGameInfo
{
public:
	cSaveGameInfo();
	explicit cSaveGameInfo (int number);

	// header
	cVersion saveVersion;
	std::string gameVersion;
	std::string gameName;
	/** the type of the save game (SIN, HOT, NET) */
	eGameType type;
	/** the date and time when this save game was saved */
	std::string date;

	// infos from model
	std::vector<cPlayerBasicData> players;
	std::filesystem::path mapFilename;
	uint32_t mapCrc = 0;
	uint32_t turn = 0;

	/** the slot number of the save game */
	int number = 0;

	template <typename Archive>
	void serialize (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (saveVersion);
		archive & NVP (gameVersion);
		archive & NVP (gameName);
		archive & NVP (type);
		archive & NVP (date);
		archive & NVP (players);
		archive & NVP (mapFilename);
		archive & NVP (mapCrc);
		archive & NVP (turn);
		archive & NVP (number);
		// clang-format on
	}
};

#endif // game_data_savegameinfoH
