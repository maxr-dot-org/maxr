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

#include "action.h"

#include "actionactivate.h"
#include "actionattack.h"
#include "actionbuyupgrades.h"
#include "actionchangebuildlist.h"
#include "actionchangemanualfire.h"
#include "actionchangeresearch.h"
#include "actionchangesentry.h"
#include "actionchangeunitname.h"
#include "actionclear.h"
#include "actionendturn.h"
#include "actionfinishbuild.h"
#include "actionload.h"
#include "actionminelayerstatus.h"
#include "actioninitnewgame.h"
#include "actionrepairreload.h"
#include "actionresourcedistribution.h"
#include "actionresumemove.h"
#include "actionselfdestroy.h"
#include "actionsetautomove.h"
#include "actionstartbuild.h"
#include "actionstartmove.h"
#include "actionstartturn.h"
#include "actionstartwork.h"
#include "actionstealdisable.h"
#include "actionstop.h"
#include "actiontransfer.h"
#include "actionupgradebuilding.h"
#include "actionupgradevehicle.h"
#include "utility/log.h"

#include <cassert>

const std::vector<std::pair<cAction::eActiontype, const char*>> serialization::sEnumStringMapping<cAction::eActiontype>::m =
{
	{cAction::eActiontype::ACTION_INIT_NEW_GAME, "ACTION_INIT_NEW_GAME"},
	{cAction::eActiontype::ACTION_START_WORK, "ACTION_START_WORK"},
	{cAction::eActiontype::ACTION_STOP, "ACTION_STOP"},
	{cAction::eActiontype::ACTION_TRANSFER, "ACTION_TRANSFER"},
	{cAction::eActiontype::ACTION_START_MOVE, "ACTION_START_MOVE"},
	{cAction::eActiontype::ACTION_RESUME_MOVE, "ACTION_RESUME_MOVE"},
	{cAction::eActiontype::ACTION_START_TURN, "ACTION_START_TURN"},
	{cAction::eActiontype::ACTION_END_TURN, "ACTION_END_TURN"},
	{cAction::eActiontype::ACTION_SELF_DESTROY, "ACTION_SELF_DESTROY"},
	{cAction::eActiontype::ACTION_ATTACK, "ACTION_ATTACK"},
	{cAction::eActiontype::ACTION_CHANGE_SENTRY, "ACTION_CHANGE_SENTRY"},
	{cAction::eActiontype::ACTION_CHANGE_MANUAL_FIRE, "ACTION_CHANGE_MANUAL_FIRE"},
	{cAction::eActiontype::ACTION_MINELAYER_STATUS, "ACTION_MINELAYER_STATUS"},
	{cAction::eActiontype::ACTION_START_BUILD, "ACTION_START_BUILD"},
	{cAction::eActiontype::ACTION_FINISH_BUILD, "ACTION_FINISH_BUILD"},
	{cAction::eActiontype::ACTION_CHANGE_BUILDLIST, "ACTION_CHANGE_BUILDLIST"},
	{cAction::eActiontype::ACTION_LOAD, "ACTION_LOAD"},
	{cAction::eActiontype::ACTION_ACTIVATE, "ACTION_ACTIVATE"},
	{cAction::eActiontype::ACTION_REPAIR_RELOAD, "ACTION_REPAIR_RELOAD"},
	{cAction::eActiontype::ACTION_RESOURCE_DISTRIBUTION, "ACTION_RESOURCE_DISTRIBUTION"},
	{cAction::eActiontype::ACTION_CLEAR, "ACTION_CLEAR"},
	{cAction::eActiontype::ACTION_STEAL_DISABLE, "ACTION_STEAL_DISABLE"},
	{cAction::eActiontype::ACTION_CHANGE_RESEARCH, "ACTION_CHANGE_RESEARCH"},
	{cAction::eActiontype::ACTION_CHANGE_UNIT_NAME, "ACTION_CHANGE_UNIT_NAME"},
	{cAction::eActiontype::ACTION_BUY_UPGRADES, "ACTION_BUY_UPGRADES"},
	{cAction::eActiontype::ACTION_UPGRADE_VEHICLE, "ACTION_UPGRADE_VEHICLE"},
	{cAction::eActiontype::ACTION_UPGRADE_BUILDING, "ACTION_UPGRADE_BUILDING"},
	{cAction::eActiontype::ACTION_SET_AUTO_MOVE, "ACTION_SET_AUTO_MOVE"}
};


std::unique_ptr<cAction> cAction::createFromBuffer (cBinaryArchiveOut& archive)
{
	eActiontype type;
	archive >> type;

	switch (type)
	{
	case eActiontype::ACTION_INIT_NEW_GAME:
		return std::make_unique<cActionInitNewGame> (archive);
	case eActiontype::ACTION_START_WORK:
		return std::make_unique<cActionStartWork> (archive);
	case eActiontype::ACTION_STOP:
		return std::make_unique<cActionStop> (archive);
	case eActiontype::ACTION_TRANSFER:
		return std::make_unique<cActionTransfer> (archive);
	case eActiontype::ACTION_START_MOVE:
		return std::make_unique<cActionStartMove> (archive);
	case eActiontype::ACTION_RESUME_MOVE:
		return std::make_unique<cActionResumeMove> (archive);
	case eActiontype::ACTION_START_TURN:
		return std::make_unique<cActionStartTurn> (archive);
	case eActiontype::ACTION_END_TURN:
		return std::make_unique<cActionEndTurn> (archive);
	case eActiontype::ACTION_SELF_DESTROY:
		return std::make_unique<cActionSelfDestroy> (archive);
	case eActiontype::ACTION_ATTACK:
		return std::make_unique<cActionAttack> (archive);
	case eActiontype::ACTION_CHANGE_SENTRY:
		return std::make_unique<cActionChangeSentry> (archive);
	case eActiontype::ACTION_CHANGE_MANUAL_FIRE:
		return std::make_unique<cActionChangeManualFire> (archive);
	case eActiontype::ACTION_MINELAYER_STATUS:
		return std::make_unique<cActionMinelayerStatus> (archive);
	case eActiontype::ACTION_START_BUILD:
		return std::make_unique<cActionStartBuild> (archive);
	case eActiontype::ACTION_FINISH_BUILD:
		return std::make_unique<cActionFinishBuild> (archive);
	case eActiontype::ACTION_CHANGE_BUILDLIST:
		return std::make_unique<cActionChangeBuildList> (archive);
	case eActiontype::ACTION_LOAD:
		return std::make_unique<cActionLoad> (archive);
	case eActiontype::ACTION_ACTIVATE:
		return std::make_unique<cActionActivate> (archive);
	case eActiontype::ACTION_REPAIR_RELOAD:
		return std::make_unique<cActionRepairReload> (archive);
	case eActiontype::ACTION_RESOURCE_DISTRIBUTION:
		return std::make_unique<cActionResourceDistribution> (archive);
	case eActiontype::ACTION_CLEAR:
		return std::make_unique<cActionClear> (archive);
	case eActiontype::ACTION_STEAL_DISABLE:
		return std::make_unique<cActionStealDisable> (archive);
	case eActiontype::ACTION_CHANGE_RESEARCH:
		return std::make_unique<cActionChangeResearch> (archive);
	case eActiontype::ACTION_CHANGE_UNIT_NAME:
		return std::make_unique<cActionChangeUnitName> (archive);
	case eActiontype::ACTION_BUY_UPGRADES:
		return std::make_unique<cActionBuyUpgrades> (archive);
	case eActiontype::ACTION_UPGRADE_VEHICLE:
		return std::make_unique<cActionUpgradeVehicle> (archive);
	case eActiontype::ACTION_UPGRADE_BUILDING:
		return std::make_unique<cActionUpgradeBuilding> (archive);
	case eActiontype::ACTION_SET_AUTO_MOVE:
		return std::make_unique<cActionSetAutoMove> (archive);
	default:
		throw std::runtime_error ("Unknown action type " + std::to_string (static_cast<int> (type)));
		return nullptr;
	}
}

//------------------------------------------------------------------------------
cAction::eActiontype cAction::getType() const
{
	return type;
}

