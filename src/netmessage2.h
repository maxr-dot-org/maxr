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

#ifndef netmessage2H
#define netmessage2H

#include <memory>

#include "maxrconfig.h"
#include "utility/serialization/serialization.h"
#include "utility/serialization/textarchive.h"
#include "utility/serialization/binaryarchive.h"
#include "game/logic/gametimer.h"

class cModel;

class cNetMessage2
{
public:
	enum eNetMessageType {
		ACTION, /** the set of actions a client (AI or player) can trigger to influence the game */
		GAMETIME_SYNC_SERVER, /** sync message from server to clients */
		GAMETIME_SYNC_CLIENT, /** sync message from client to server */
		RANDOM_SEED,
		PLAYERSTATE,
		CHAT //TODO: action?
	};
	static std::unique_ptr<cNetMessage2> createFromBuffer(cBinaryArchiveOut& archive);
	
	virtual ~cNetMessage2() {}

	eNetMessageType getType() const { return type; };

	template <typename T>
	void serializeThis(T& archive)
	{ 
		archive & type; 
		archive & playerNr;
	}
	virtual void serialize(cBinaryArchiveIn& archive) { serializeThis(archive); } 
	virtual void serialize(cBinaryArchiveOut& archive) { serializeThis(archive); }
	virtual void serialize(cTextArchiveIn& archive) { serializeThis(archive); }

	int playerNr;

protected:
	cNetMessage2(eNetMessageType type) : type(type), playerNr(-1) {};
private:
	cNetMessage2(const cNetMessage2&) MAXR_DELETE_FUNCTION;
	cNetMessage2& operator=(const cNetMessage2&) MAXR_DELETE_FUNCTION;
	eNetMessageType type;
};

class cNetMessageChat : public cNetMessage2 
{
public:
	cNetMessageChat(std::string message) : cNetMessage2(CHAT), message(message) {};
	cNetMessageChat() : cNetMessage2(CHAT) {};

	template<typename T>
	void serializeThis(T& archive)
	{
		cNetMessage2::serialize(archive);
		archive & message;
	}
	virtual void serialize(cBinaryArchiveIn& archive) { serializeThis(archive); }
	virtual void serialize(cBinaryArchiveOut& archive) { serializeThis(archive); }
	virtual void serialize(cTextArchiveIn& archive) { serializeThis(archive); }

	std::string message;
};

class cNetMessageSyncServer : public cNetMessage2
{
public:
	cNetMessageSyncServer() : cNetMessage2(GAMETIME_SYNC_SERVER) {};

	template<typename T>
	void serializeThis(T& archive)
	{
		cNetMessage2::serialize(archive);
		archive & gameTime;
		archive & checksum;
		archive & ping;
	}
	virtual void serialize(cBinaryArchiveIn& archive) { serializeThis(archive); }
	virtual void serialize(cBinaryArchiveOut& archive) { serializeThis(archive); }
	virtual void serialize(cTextArchiveIn& archive) { serializeThis(archive); }

	unsigned int gameTime;
	unsigned int checksum;
	unsigned int ping;
};

class cNetMessageSyncClient : public cNetMessage2
{
public:
	cNetMessageSyncClient() : cNetMessage2(GAMETIME_SYNC_CLIENT) {};

	template<typename T>
	void serializeThis(T& archive)
	{
		cNetMessage2::serialize(archive);
		archive & gameTime;
		archive & crcOK;
		archive & timeBuffer;
		archive & ticksPerFrame;
		archive & queueSize;
		archive & eventCounter;
	}
	virtual void serialize(cBinaryArchiveIn& archive) { serializeThis(archive); }
	virtual void serialize(cBinaryArchiveOut& archive) { serializeThis(archive); }
	virtual void serialize(cTextArchiveIn& archive) { serializeThis(archive); }


	unsigned int gameTime;

	//send debug data to server
	bool crcOK;
	unsigned int timeBuffer;
	unsigned int ticksPerFrame;
	unsigned int queueSize;
	unsigned int eventCounter;
};

class cNetMessageRandomSeed : public cNetMessage2
{
public:
	cNetMessageRandomSeed(uint64_t seed) : 
		cNetMessage2(RANDOM_SEED),
		seed(seed) 
	{};

	template<typename T>
	void serializeThis(T& archive)
	{
		cNetMessage2::serialize(archive);
		archive & seed;
	}
	virtual void serialize(cBinaryArchiveIn& archive) { serializeThis(archive); }
	virtual void serialize(cBinaryArchiveOut& archive) { serializeThis(archive); }
	virtual void serialize(cTextArchiveIn& archive) { serializeThis(archive); }

	uint64_t seed;
};

#endif //netmessage2H
