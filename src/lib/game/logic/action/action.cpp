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
#include "actioninitnewgame.h"
#include "actionload.h"
#include "actionminelayerstatus.h"
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

namespace serialization
{
	const std::vector<std::pair<cAction::eActiontype, const char*>>
	sEnumStringMapping<cAction::eActiontype>::m =
	{
		{cAction::eActiontype::InitNewGame, "InitNewGame"},
		{cAction::eActiontype::StartWork, "StartWork"},
		{cAction::eActiontype::Stop, "Stop"},
		{cAction::eActiontype::Transfer, "Transfer"},
		{cAction::eActiontype::StartMove, "StartMove"},
		{cAction::eActiontype::ResumeMove, "ResumeMove"},
		{cAction::eActiontype::StartTurn, "StartTurn"},
		{cAction::eActiontype::EndTurn, "EndTurn"},
		{cAction::eActiontype::SelfDestroy, "SelfDestroy"},
		{cAction::eActiontype::Attack, "Attack"},
		{cAction::eActiontype::ChangeSentry, "ChangeSentry"},
		{cAction::eActiontype::ChangeManualFire, "ChangeManualFire"},
		{cAction::eActiontype::MinelayerStatus, "MinelayerStatus"},
		{cAction::eActiontype::StartBuild, "StartBuild"},
		{cAction::eActiontype::FinishBuild, "FinishBuild"},
		{cAction::eActiontype::ChangeBuildlist, "ChangeBuildlist"},
		{cAction::eActiontype::Load, "Load"},
		{cAction::eActiontype::Activate, "Activate"},
		{cAction::eActiontype::RepairReload, "RepairReload"},
		{cAction::eActiontype::ResourceDistribution, "ResourceDistribution"},
		{cAction::eActiontype::Clear, "Clear"},
		{cAction::eActiontype::StealDisable, "StealDisable"},
		{cAction::eActiontype::ChangeResearch, "ChangeResearch"},
		{cAction::eActiontype::ChangeUnitName, "ChangeUnitName"},
		{cAction::eActiontype::BuyUpgrades, "BuyUpgrades"},
		{cAction::eActiontype::UpgradeVehicle, "UpgradeVehicle"},
		{cAction::eActiontype::UpgradeBuilding, "UpgradeBuilding"},
		{cAction::eActiontype::SetAutoMove, "SetAutoMove"}
	};
} // namespace serialization

std::unique_ptr<cAction> cAction::createFromBuffer (cBinaryArchiveIn& archive)
{
	eActiontype action;
	archive >> NVP (action);

	switch (action)
	{
		case eActiontype::InitNewGame: return std::make_unique<cActionInitNewGame> (archive);
		case eActiontype::StartWork: return std::make_unique<cActionStartWork> (archive);
		case eActiontype::Stop: return std::make_unique<cActionStop> (archive);
		case eActiontype::Transfer: return std::make_unique<cActionTransfer> (archive);
		case eActiontype::StartMove: return std::make_unique<cActionStartMove> (archive);
		case eActiontype::ResumeMove: return std::make_unique<cActionResumeMove> (archive);
		case eActiontype::StartTurn: return std::make_unique<cActionStartTurn> (archive);
		case eActiontype::EndTurn: return std::make_unique<cActionEndTurn> (archive);
		case eActiontype::SelfDestroy: return std::make_unique<cActionSelfDestroy> (archive);
		case eActiontype::Attack: return std::make_unique<cActionAttack> (archive);
		case eActiontype::ChangeSentry: return std::make_unique<cActionChangeSentry> (archive);
		case eActiontype::ChangeManualFire: return std::make_unique<cActionChangeManualFire> (archive);
		case eActiontype::MinelayerStatus: return std::make_unique<cActionMinelayerStatus> (archive);
		case eActiontype::StartBuild: return std::make_unique<cActionStartBuild> (archive);
		case eActiontype::FinishBuild: return std::make_unique<cActionFinishBuild> (archive);
		case eActiontype::ChangeBuildlist: return std::make_unique<cActionChangeBuildList> (archive);
		case eActiontype::Load: return std::make_unique<cActionLoad> (archive);
		case eActiontype::Activate: return std::make_unique<cActionActivate> (archive);
		case eActiontype::RepairReload: return std::make_unique<cActionRepairReload> (archive);
		case eActiontype::ResourceDistribution: return std::make_unique<cActionResourceDistribution> (archive);
		case eActiontype::Clear: return std::make_unique<cActionClear> (archive);
		case eActiontype::StealDisable: return std::make_unique<cActionStealDisable> (archive);
		case eActiontype::ChangeResearch: return std::make_unique<cActionChangeResearch> (archive);
		case eActiontype::ChangeUnitName: return std::make_unique<cActionChangeUnitName> (archive);
		case eActiontype::BuyUpgrades: return std::make_unique<cActionBuyUpgrades> (archive);
		case eActiontype::UpgradeVehicle: return std::make_unique<cActionUpgradeVehicle> (archive);
		case eActiontype::UpgradeBuilding: return std::make_unique<cActionUpgradeBuilding> (archive);
		case eActiontype::SetAutoMove: return std::make_unique<cActionSetAutoMove> (archive);
		default:
			throw std::runtime_error ("Unknown action type " + std::to_string (static_cast<int> (action)));
	}
}

//------------------------------------------------------------------------------
cAction::eActiontype cAction::getType() const
{
	return action;
}
