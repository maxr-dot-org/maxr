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
#include "utility/serializationarchive.h"

class cModel;

class cNetMessage2
{
public:
	enum eNetMessageType {
		ACTION, /** the set of actions a client (AI or player) can trigger to influence the game */
		SYNC,
		PLAYERSTATE,
		CHAT //TODO: action?
	};
	static std::unique_ptr<cNetMessage2> createFromBuffer(cArchiveOut& archive);
	
	virtual ~cNetMessage2() {}

	eNetMessageType getType() const { return type; };

	template <typename T>
	void serializeThis(T& archive)
	{ 
		archive & type; 
		archive & playerNr;
	}

	virtual void serialize(cArchiveIn& archive) { serializeThis(archive); }
	virtual void serialize(cArchiveOut& archive) { serializeThis(archive); }

	int playerNr;

protected:
	cNetMessage2(eNetMessageType type) : type(type), playerNr(-1) {};
private:
	cNetMessage2(const cNetMessage2&) MAXR_DELETE_FUNCTION;
	cNetMessage2& operator=(const cNetMessage2&)MAXR_DELETE_FUNCTION;
	eNetMessageType type;
};

//TODO: alles an die richtige stelle verschieben
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
	virtual void serialize(cArchiveIn& archive) { serializeThis(archive); }
	virtual void serialize(cArchiveOut& archive) { serializeThis(archive); }

	std::string message;
};



#endif //netmessage2H
