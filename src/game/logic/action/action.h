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

#ifndef game_logic_actionH
#define game_logic_actionH

#include "netmessage2.h"

class cAction : public cNetMessage2
{
public:
	// When changing this enum, also update function enumToString(eActiontype value)!
	enum class eActiontype {
		ACTION_INIT_NEW_GAME,
		ACTION_START_WORK,
		ACTION_STOP_WORK,
		ACTION_TRANSFER,
		ACTION_START_MOVE,
		ACTION_STOP_MOVE,
		ACTION_RESUME_MOVE,
		ACTION_END_TURN,
		ACTION_SELF_DESTROY,
		ACTION_ATTACK,
		ACTION_CHANGE_SENTRY,
		ACTION_CHANGE_MANUAL_FIRE,
		ACTION_MINELAYER_STATUS
	};
	static std::unique_ptr<cAction> createFromBuffer(cBinaryArchiveOut& archive);

	eActiontype getType() const;

	virtual void serialize(cBinaryArchiveIn& archive) { cNetMessage2::serialize(archive); serializeThis(archive); }
	virtual void serialize(cTextArchiveIn& archive)   { cNetMessage2::serialize(archive); serializeThis(archive); }

	//Note: this function handles incoming data from network. Make every possible sanity check!
	virtual void execute(cModel& model) const = 0;
protected:
	cAction(eActiontype type) : cNetMessage2(eNetMessageType::ACTION), type(type){};
private:
	template<typename T>
	void serializeThis(T& archive)
	{
		archive & type;
	}

	cAction(const cAction&) MAXR_DELETE_FUNCTION;
	cAction& operator=(const cAction&)MAXR_DELETE_FUNCTION;

	eActiontype type;
};

std::string enumToString(cAction::eActiontype value);


#endif
