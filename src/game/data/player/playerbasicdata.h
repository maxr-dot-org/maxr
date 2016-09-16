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

#ifndef game_data_player_playerbasicdataH
#define game_data_player_playerbasicdataH

#include <string>

#include "game/data/player/playercolor.h"
#include "utility/signal/signal.h"

/**
 * a structure that includes all information needed in pre-game.
 */
class cPlayerBasicData
{
public:
	cPlayerBasicData();
	cPlayerBasicData (const std::string& name_, cPlayerColor color, int Nr_, int socketIndex_ = -1);
	cPlayerBasicData (const cPlayerBasicData& other);
	cPlayerBasicData& operator= (const cPlayerBasicData& other);

	const std::string& getName() const;
	void setName (std::string name);
	const cPlayerColor& getColor() const { return color; }
	void setColor (cPlayerColor color);
	int getNr() const;
	void setNr (int index);
	int getSocketIndex() const;
	void setSocketIndex (int index);
	void setLocal();
	bool isLocal() const;
	void onSocketIndexDisconnected (int socketIndex);
	void setReady (bool ready);
	bool isReady() const;

	uint32_t getChecksum(uint32_t crc) const;

	mutable cSignal<void ()> nameChanged;
	mutable cSignal<void ()> numberChanged;
	mutable cSignal<void ()> colorChanged;
	mutable cSignal<void ()> socketIndexChanged;
	mutable cSignal<void ()> readyChanged;

	template <typename T>
	void serialize(T& archive)
	{
		archive & NVP(name);
		archive & NVP(color);
		archive & NVP(Nr);
		//archive & NVP(socketIndex); //TODO: new socket number from multiplayer menu, when loading? Move socket to Client/Server
		archive & NVP(ready);
	}
private:
	std::string name;
	cPlayerColor color;
	int Nr;

	// Index in socket array of cServer::network
	// if MAX_CLIENTS it's the local connected player
	// -1 for unknown
	int socketIndex; //TODO: move to cPlayerConnectionManager
	bool ready;
};

#endif // game_data_player_playerbasicdataH
