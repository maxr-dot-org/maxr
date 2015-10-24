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

#include "game/data/report/savedreport.h"

#include "game/data/report/savedreportchat.h"
#include "game/data/report/savedreportsimple.h"

#include "game/data/report/unit/savedreportattacked.h"
#include "game/data/report/unit/savedreportattackingenemy.h"
#include "game/data/report/unit/savedreportcapturedbyenemy.h"
#include "game/data/report/unit/savedreportdestroyed.h"
#include "game/data/report/unit/savedreportdetected.h"
#include "game/data/report/unit/savedreportdisabled.h"
#include "game/data/report/unit/savedreportpathinterrupted.h"
#include "game/data/report/unit/savedreportsurveyoraiconfused.h"
#include "game/data/report/unit/savedreportsurveyoraisenseless.h"

#include "game/data/report/special/savedreporthostcommand.h"
#include "game/data/report/special/savedreportresourcechanged.h"
#include "game/data/report/special/savedreportlostconnection.h"
#include "game/data/report/special/savedreportplayerendedturn.h"
#include "game/data/report/special/savedreportplayerdefeated.h"
#include "game/data/report/special/savedreportplayerleft.h"
#include "game/data/report/special/savedreportupgraded.h"
#include "game/data/report/special/savedreportturnstart.h"

#include "main.h"

template <typename T>
static std::unique_ptr<cSavedReport> cSavedReport::createFromImpl (T& archive)
{
	eSavedReportType type;
	archive >> NVP(type);

	switch (type)
	{
	case eSavedReportType::Chat:
		return std::make_unique<cSavedReportChat>(archive);
	case eSavedReportType::Attacked:
		return std::make_unique<cSavedReportAttacked>(archive);
	case eSavedReportType::AttackingEnemy:
		return std::make_unique<cSavedReportAttackingEnemy>(archive);
	case eSavedReportType::CapturedByEnemy:
		return std::make_unique<cSavedReportCapturedByEnemy>(archive);
	case eSavedReportType::Destroyed:
		return std::make_unique<cSavedReportDestroyed>(archive);
	case eSavedReportType::Detected:
		return std::make_unique<cSavedReportDetected>(archive);
	case eSavedReportType::Disabled:
		return std::make_unique<cSavedReportDisabled>(archive);
	case eSavedReportType::PathInterrupted:
		return std::make_unique<cSavedReportPathInterrupted>(archive);
	case eSavedReportType::SurveyorAiConfused:
		return std::make_unique<cSavedReportSurveyorAiConfused>(archive);
	case eSavedReportType::SurveyorAiSenseless:
		return std::make_unique<cSavedReportSurveyorAiSenseless>(archive);
	case eSavedReportType::HostCommand:
		return std::make_unique<cSavedReportHostCommand>(archive);
	case eSavedReportType::ResourceChanged:
		return std::make_unique<cSavedReportResourceChanged>(archive);
	case eSavedReportType::PlayerEndedTurn:
		return std::make_unique<cSavedReportPlayerEndedTurn>(archive);
	case eSavedReportType::LostConnection:
		return std::make_unique<cSavedReportLostConnection>(archive);
	case eSavedReportType::PlayerDefeated:
		return std::make_unique<cSavedReportPlayerDefeated>(archive);
	case eSavedReportType::PlayerLeft:
		return std::make_unique<cSavedReportPlayerLeft>(archive);
	case eSavedReportType::Upgraded:
		return std::make_unique<cSavedReportUpgraded>(archive);
	case eSavedReportType::TurnStart:
		return std::make_unique<cSavedReportTurnStart>(archive);
	case eSavedReportType::MetalInsufficient:
	case eSavedReportType::FuelInsufficient:
	case eSavedReportType::GoldInsufficient:
	case eSavedReportType::EnergyInsufficient:
	case eSavedReportType::TeamInsufficient:
	case eSavedReportType::MetalLow:
	case eSavedReportType::FuelLow:
	case eSavedReportType::GoldLow:
	case eSavedReportType::EnergyLow:
	case eSavedReportType::TeamLow:
	case eSavedReportType::EnergyToLow:
	case eSavedReportType::EnergyIsNeeded:
	case eSavedReportType::BuildingDisabled:
	case eSavedReportType::ProducingError:
	case eSavedReportType::TurnWait:
	case eSavedReportType::TurnAutoMove:
		return std::make_unique<cSavedReportSimple>(type);
	default:
		//TODO: throw
		return nullptr;
	}
}

//------------------------------------------------------------------------------
std::unique_ptr<cSavedReport> cSavedReport::createFrom(cBinaryArchiveOut& archive)
{
	return createFromImpl(archive);
}

//------------------------------------------------------------------------------
std::unique_ptr<cSavedReport> cSavedReport::createFrom(cXmlArchiveOut& archive, const std::string& name)
{
	archive.enterChild(name);
	auto report = createFromImpl(archive);
	archive.leaveChild();
	return report;
}

//------------------------------------------------------------------------------
bool cSavedReport::hasUnitId() const
{
	return false;
}

//------------------------------------------------------------------------------
const sID& cSavedReport::getUnitId() const
{
	static sID dummy;
	return dummy;
}

//------------------------------------------------------------------------------
bool cSavedReport::hasPosition() const
{
	return false;
}

//------------------------------------------------------------------------------
const cPosition& cSavedReport::getPosition() const
{
	static cPosition dummy;
	return dummy;
}

//------------------------------------------------------------------------------
void cSavedReport::playSound (cSoundManager& soundManager) const
{}
