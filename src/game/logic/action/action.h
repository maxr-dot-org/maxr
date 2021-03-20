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

#include "protocol/netmessage.h"

class cAction : public cNetMessageT<eNetMessageType::ACTION>
{
public:
	// When changing this enum, also update function enumToString(eActiontype value)!
	enum class eActiontype {
		ACTION_INIT_NEW_GAME,
		ACTION_START_WORK,
		ACTION_STOP,
		ACTION_TRANSFER,
		ACTION_START_MOVE,
		ACTION_RESUME_MOVE,
		ACTION_START_TURN,
		ACTION_END_TURN,
		ACTION_SELF_DESTROY,
		ACTION_ATTACK,
		ACTION_CHANGE_SENTRY,
		ACTION_CHANGE_MANUAL_FIRE,
		ACTION_MINELAYER_STATUS,
		ACTION_START_BUILD,
		ACTION_FINISH_BUILD,
		ACTION_CHANGE_BUILDLIST,
		ACTION_LOAD,
		ACTION_ACTIVATE,
		ACTION_REPAIR_RELOAD,
		ACTION_RESOURCE_DISTRIBUTION,
		ACTION_CLEAR,
		ACTION_STEAL_DISABLE,
		ACTION_CHANGE_RESEARCH,
		ACTION_CHANGE_UNIT_NAME,
		ACTION_BUY_UPGRADES,
		ACTION_UPGRADE_VEHICLE,
		ACTION_UPGRADE_BUILDING,
		ACTION_SET_AUTO_MOVE
	};
	static std::unique_ptr<cAction> createFromBuffer(cBinaryArchiveOut& archive);

	eActiontype getType() const;

	void serialize(cBinaryArchiveIn& archive) override { cNetMessage::serialize(archive); serializeThis(archive); }
	void serialize(cTextArchiveIn& archive) override { cNetMessage::serialize(archive); serializeThis(archive); }

	//Note: this function handles incoming data from network. Make every possible sanity check!
	virtual void execute(cModel& model) const = 0;
protected:
	cAction (eActiontype type) : type(type){}
private:
	template<typename T>
	void serializeThis(T& archive)
	{
		archive & type;
	}

	cAction(const cAction&) = delete;
	cAction& operator=(const cAction&) = delete;

	eActiontype type;
};

std::string enumToString(cAction::eActiontype value);

//------------------------------------------------------------------------------
template <cAction::eActiontype ActionType>
class cActionT : public cAction
{
public:
	cActionT() : cAction (ActionType) {}
};

#endif
