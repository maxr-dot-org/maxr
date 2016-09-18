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
#include "game/data/report/savedreport.h"
#include "ui/graphical/game/gameguistate.h"

class cSavedReport;

// When changing this enum, also update function enumToString(eNetMessageType value)!
enum class eNetMessageType {
	ACTION, /** the set of actions a client (AI or player) can trigger to influence the game */
	GAMETIME_SYNC_SERVER, /** sync message from server to clients */
	GAMETIME_SYNC_CLIENT, /** sync message from client to server */
	RANDOM_SEED,
	PLAYERSTATE,
	CHAT,
	GUI_SAVE_INFO,
	REQUEST_GUI_SAVE_INFO,
	RESYNC_MODEL
};
std::string enumToString(eNetMessageType value);

//------------------------------------------------------------------------------
class cNetMessage2
{
public:

	static std::unique_ptr<cNetMessage2> createFromBuffer(const std::vector<unsigned char>& serialMessage);
	
	virtual ~cNetMessage2() {}

	eNetMessageType getType() const { return type; };
	std::unique_ptr<cNetMessage2> clone() const;

	virtual void serialize(cBinaryArchiveIn& archive) { serializeThis(archive); } 
	virtual void serialize(cTextArchiveIn& archive) { serializeThis(archive); }

	int playerNr;

protected:
	cNetMessage2(eNetMessageType type) : type(type), playerNr(-1) {};
private:
	template <typename T>
	void serializeThis(T& archive)
	{
		archive & type;
		archive & playerNr;
	}

	cNetMessage2(const cNetMessage2&) MAXR_DELETE_FUNCTION;
	cNetMessage2& operator=(const cNetMessage2&) MAXR_DELETE_FUNCTION;

	eNetMessageType type;
};

//------------------------------------------------------------------------------
class cNetMessageChat : public cNetMessage2 
{
public:
	cNetMessageChat(std::string message) : cNetMessage2(eNetMessageType::CHAT), message(message) {};
	cNetMessageChat() :	cNetMessage2(eNetMessageType::CHAT) {};

	cNetMessageChat(cBinaryArchiveOut& archive) :
		cNetMessage2(eNetMessageType::CHAT)
	{
		serializeThis(archive);
	}

	virtual void serialize(cBinaryArchiveIn& archive) { cNetMessage2::serialize(archive); serializeThis(archive); }
	virtual void serialize(cTextArchiveIn& archive)   { cNetMessage2::serialize(archive); serializeThis(archive); }

	std::string message;

private:
	template<typename T>
	void serializeThis(T& archive)
	{
		archive & message;
	}
};

//------------------------------------------------------------------------------
class cNetMessageSyncServer : public cNetMessage2
{
public:
	cNetMessageSyncServer() : cNetMessage2(eNetMessageType::GAMETIME_SYNC_SERVER) {};
	cNetMessageSyncServer(cBinaryArchiveOut& archive) :
		cNetMessage2(eNetMessageType::GAMETIME_SYNC_SERVER)
	{
		serializeThis(archive);
	}

	virtual void serialize(cBinaryArchiveIn& archive) { cNetMessage2::serialize(archive); serializeThis(archive); }
	virtual void serialize(cTextArchiveIn& archive)   { cNetMessage2::serialize(archive); serializeThis(archive); }

	unsigned int gameTime;
	unsigned int checksum;
	unsigned int ping;

private:
	template<typename T>
	void serializeThis(T& archive)
	{
		archive & gameTime;
		archive & checksum;
		archive & ping;
	}
};

//------------------------------------------------------------------------------
class cNetMessageSyncClient : public cNetMessage2
{
public:
	cNetMessageSyncClient() : cNetMessage2(eNetMessageType::GAMETIME_SYNC_CLIENT) {};
	cNetMessageSyncClient(cBinaryArchiveOut& archive) :
		cNetMessage2(eNetMessageType::GAMETIME_SYNC_CLIENT)
	{
		serializeThis(archive);
	}

	virtual void serialize(cBinaryArchiveIn& archive) { cNetMessage2::serialize(archive); serializeThis(archive); }
	virtual void serialize(cTextArchiveIn& archive)   { cNetMessage2::serialize(archive); serializeThis(archive); }

	unsigned int gameTime;

	//send debug data to server
	bool crcOK;
	unsigned int timeBuffer;
	unsigned int ticksPerFrame;
	unsigned int queueSize;
	unsigned int eventCounter;

private:
	template<typename T>
	void serializeThis(T& archive)
	{
		archive & gameTime;
		archive & crcOK;
		archive & timeBuffer;
		archive & ticksPerFrame;
		archive & queueSize;
		archive & eventCounter;
	}
};

//------------------------------------------------------------------------------
class cNetMessageRandomSeed : public cNetMessage2
{
public:
	cNetMessageRandomSeed(uint64_t seed) : 
		cNetMessage2(eNetMessageType::RANDOM_SEED),
		seed(seed) 
	{};
	cNetMessageRandomSeed(cBinaryArchiveOut& archive) :
		cNetMessage2(eNetMessageType::RANDOM_SEED)
	{
		serializeThis(archive);
	}

	virtual void serialize(cBinaryArchiveIn& archive) { cNetMessage2::serialize(archive); serializeThis(archive); }
	virtual void serialize(cTextArchiveIn& archive)   { cNetMessage2::serialize(archive); serializeThis(archive); }

	uint64_t seed;
private:
	template<typename T>
	void serializeThis(T& archive)
	{
		archive & seed;
	}
};

//------------------------------------------------------------------------------
class cNetMessageGUISaveInfo : public cNetMessage2
{
public:
	cNetMessageGUISaveInfo(int savingID) :
		cNetMessage2(eNetMessageType::GUI_SAVE_INFO),
		savingID(savingID),
		reports(nullptr)
	{};
	cNetMessageGUISaveInfo(cBinaryArchiveOut& archive) :
		cNetMessage2(eNetMessageType::GUI_SAVE_INFO)
	{
		loadThis(archive);
	}

	virtual void serialize(cBinaryArchiveIn& archive) { cNetMessage2::serialize(archive); saveThis(archive); }
	virtual void serialize(cTextArchiveIn& archive)   { cNetMessage2::serialize(archive); saveThis(archive); }

	std::shared_ptr<std::vector<std::unique_ptr<cSavedReport>>> reports;
	cGameGuiState guiState;
	std::array<std::pair<bool, cPosition>, 4> savedPositions;
	int savingID;
private:
	template<typename T>
	void loadThis(T& archive)
	{
		if (reports == nullptr)
			reports = std::make_shared<std::vector<std::unique_ptr<cSavedReport>>>();

		int size;
		archive >> size;
		reports->resize(size);
		for (auto& report : *reports)
		{
			report = cSavedReport::createFrom(archive);
		}
		archive >> guiState;
		archive >> savingID;
		archive >> savedPositions;
	}
	template<typename T>
	void saveThis(T& archive)
	{
		if (reports == nullptr)
			reports = std::make_shared<std::vector<std::unique_ptr<cSavedReport>>>();

		archive << (int)reports->size();
		for (auto& report : *reports)
		{
			archive << *report;
		}
		archive << guiState;
		archive << savingID;
		archive << savedPositions;
	}
};

//------------------------------------------------------------------------------
class cNetMessageRequestGUISaveInfo : public cNetMessage2
{
public:
	cNetMessageRequestGUISaveInfo(int savingID) :
		cNetMessage2(eNetMessageType::REQUEST_GUI_SAVE_INFO),
		savingID(savingID)
	{};
	cNetMessageRequestGUISaveInfo(cBinaryArchiveOut& archive) :
		cNetMessage2(eNetMessageType::REQUEST_GUI_SAVE_INFO)
	{
		serializeThis(archive);
	};

	virtual void serialize(cBinaryArchiveIn& archive) { cNetMessage2::serialize(archive); serializeThis(archive); }
	virtual void serialize(cTextArchiveIn& archive)   { cNetMessage2::serialize(archive); serializeThis(archive); }

	int savingID;
private:
	template<typename T>
	void serializeThis(T& archive)
	{
		archive & savingID;
	}
};

//------------------------------------------------------------------------------
class cNetMessageResyncModel : public cNetMessage2
{
public:
	cNetMessageResyncModel(const cModel& model);
	cNetMessageResyncModel(cBinaryArchiveOut& archive) :
		cNetMessage2(eNetMessageType::RESYNC_MODEL)
	{
		serializeThis(archive);
	};
	virtual void serialize(cBinaryArchiveIn& archive) { cNetMessage2::serialize(archive); serializeThis(archive); }
	virtual void serialize(cTextArchiveIn& archive)   { cNetMessage2::serialize(archive); serializeThis(archive); }

	void apply(cModel& model) const;
private:
	template<typename T>
	void serializeThis(T& archive)
	{
		archive & data;
	}

	std::vector<uint8_t> data;
};
#endif //netmessage2H
