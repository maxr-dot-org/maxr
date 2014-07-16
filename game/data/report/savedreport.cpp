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

#include "netmessage.h"
#include "utility/tounderlyingtype.h"
#include "main.h"

//------------------------------------------------------------------------------
void cSavedReport::pushInto (cNetMessage& message) const
{
	message.pushInt32 (toUnderlyingType(getType()));
}

//------------------------------------------------------------------------------
void cSavedReport::pushInto (tinyxml2::XMLElement& element) const
{
	element.SetAttribute ("type", iToStr (toUnderlyingType (getType ())).c_str ());
}

//------------------------------------------------------------------------------
bool cSavedReport::hasUnitId () const
{
	return false;
}

//------------------------------------------------------------------------------
const sID& cSavedReport::getUnitId () const
{
	static sID dummy;
	return dummy;
}

//------------------------------------------------------------------------------
bool cSavedReport::hasPosition () const
{
	return false;
}

//------------------------------------------------------------------------------
const cPosition& cSavedReport::getPosition () const
{
	static cPosition dummy;
	return dummy;
}

//------------------------------------------------------------------------------
void cSavedReport::playSound (cSoundManager& soundManager) const
{}

//------------------------------------------------------------------------------
std::unique_ptr<cSavedReport> cSavedReport::createFrom (cNetMessage& message)
{
	auto type = (eSavedReportType)message.popInt32 ();

	switch (type)
	{
	case eSavedReportType::Chat:
		return std::make_unique<cSavedReportChat> (message);
	case eSavedReportType::Attacked:
		return std::make_unique<cSavedReportAttacked> (message);
	case eSavedReportType::AttackingEnemy:
		return std::make_unique<cSavedReportAttackingEnemy> (message);
	case eSavedReportType::CapturedByEnemy:
		return std::make_unique<cSavedReportCapturedByEnemy> (message);
	case eSavedReportType::Destroyed:
		return std::make_unique<cSavedReportDestroyed> (message);
	case eSavedReportType::Detected:
		return std::make_unique<cSavedReportDetected> (message);
	case eSavedReportType::Disabled:
		return std::make_unique<cSavedReportDisabled> (message);
	case eSavedReportType::PathInterrupted:
		return std::make_unique<cSavedReportPathInterrupted> (message);
	case eSavedReportType::SurveyorAiConfused:
		return std::make_unique<cSavedReportSurveyorAiConfused> (message);
	case eSavedReportType::SurveyorAiSenseless:
		return std::make_unique<cSavedReportSurveyorAiSenseless> (message);
	case eSavedReportType::HostCommand:
		return std::make_unique<cSavedReportHostCommand> (message);
	case eSavedReportType::ResourceChanged:
		return std::make_unique<cSavedReportResourceChanged> (message);
	case eSavedReportType::PlayerEndedTurn:
		return std::make_unique<cSavedReportPlayerEndedTurn> (message);
	case eSavedReportType::LostConnection:
		return std::make_unique<cSavedReportLostConnection> (message);
	case eSavedReportType::PlayerDefeated:
		return std::make_unique<cSavedReportPlayerDefeated> (message);
	case eSavedReportType::PlayerLeft:
		return std::make_unique<cSavedReportPlayerLeft> (message);
	case eSavedReportType::Upgraded:
		return std::make_unique<cSavedReportUpgraded> (message);
	case eSavedReportType::TurnStart:
		return std::make_unique<cSavedReportTurnStart> (message);
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
		return std::make_unique<cSavedReportSimple> (type);
	default:
		return nullptr;
	}
}

//------------------------------------------------------------------------------
std::unique_ptr<cSavedReport> cSavedReport::createFrom (const tinyxml2::XMLElement& element)
{
	auto type = (eSavedReportType)element.IntAttribute ("type");

	switch (type)
	{
	case eSavedReportType::Chat:
		return std::make_unique<cSavedReportChat> (element);
	case eSavedReportType::Attacked:
		return std::make_unique<cSavedReportAttacked> (element);
	case eSavedReportType::AttackingEnemy:
		return std::make_unique<cSavedReportAttackingEnemy> (element);
	case eSavedReportType::CapturedByEnemy:
		return std::make_unique<cSavedReportCapturedByEnemy> (element);
	case eSavedReportType::Destroyed:
		return std::make_unique<cSavedReportDestroyed> (element);
	case eSavedReportType::Detected:
		return std::make_unique<cSavedReportDetected> (element);
	case eSavedReportType::Disabled:
		return std::make_unique<cSavedReportDisabled> (element);
	case eSavedReportType::PathInterrupted:
		return std::make_unique<cSavedReportPathInterrupted> (element);
	case eSavedReportType::SurveyorAiConfused:
		return std::make_unique<cSavedReportSurveyorAiConfused> (element);
	case eSavedReportType::SurveyorAiSenseless:
		return std::make_unique<cSavedReportSurveyorAiSenseless> (element);
	case eSavedReportType::HostCommand:
		return std::make_unique<cSavedReportHostCommand> (element);
	case eSavedReportType::ResourceChanged:
		return std::make_unique<cSavedReportResourceChanged> (element);
	case eSavedReportType::PlayerEndedTurn:
		return std::make_unique<cSavedReportPlayerEndedTurn> (element);
	case eSavedReportType::LostConnection:
		return std::make_unique<cSavedReportLostConnection> (element);
	case eSavedReportType::PlayerDefeated:
		return std::make_unique<cSavedReportPlayerDefeated> (element);
	case eSavedReportType::PlayerLeft:
		return std::make_unique<cSavedReportPlayerLeft> (element);
	case eSavedReportType::Upgraded:
		return std::make_unique<cSavedReportUpgraded> (element);
	case eSavedReportType::TurnStart:
		return std::make_unique<cSavedReportTurnStart> (element);
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
		return std::make_unique<cSavedReportSimple> (type);
	default:
		return nullptr;
	}
}
