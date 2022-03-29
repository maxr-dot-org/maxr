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

#include "savedreportssound.h"

#include "game/data/report/savedreport.h"
#include "game/data/report/savedreportchat.h"
#include "game/data/report/savedreportsimple.h"
#include "game/data/report/special/savedreporthostcommand.h"
#include "game/data/report/special/savedreportlostconnection.h"
#include "game/data/report/special/savedreportplayerdefeated.h"
#include "game/data/report/special/savedreportplayerendedturn.h"
#include "game/data/report/special/savedreportplayerleft.h"
#include "game/data/report/special/savedreportplayerwins.h"
#include "game/data/report/special/savedreportresourcechanged.h"
#include "game/data/report/special/savedreportturnstart.h"
#include "game/data/report/special/savedreportupgraded.h"
#include "game/data/report/unit/savedreportattacked.h"
#include "game/data/report/unit/savedreportattackingenemy.h"
#include "game/data/report/unit/savedreportcapturedbyenemy.h"
#include "game/data/report/unit/savedreportdestroyed.h"
#include "game/data/report/unit/savedreportdetected.h"
#include "game/data/report/unit/savedreportdisabled.h"
#include "game/data/report/unit/savedreportpathinterrupted.h"
#include "game/data/report/unit/savedreportsurveyoraiconfused.h"
#include "resources/sound.h"
#include "ui/sound/effects/soundeffectvoice.h"
#include "ui/sound/soundmanager.h"
#include "utility/random.h"

namespace
{

	//--------------------------------------------------------------------------
	void playReportSound (cSoundManager& soundManager, const cSavedReportAttacked&)
	{
		soundManager.playSound (std::make_unique<cSoundEffectVoice> (eSoundEffectType::VoiceAttackingUs, getRandom (VoiceData.VOIAttackingUs)));
	}

	//--------------------------------------------------------------------------
	void playReportSound (cSoundManager& soundManager, const cSavedReportAttackingEnemy&)
	{
		soundManager.playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceAttacking, getRandom (VoiceData.VOIAttackingEnemy)));
	}
	//--------------------------------------------------------------------------
	void playReportSound (cSoundManager& soundManager, const cSavedReportCapturedByEnemy&)
	{
		soundManager.playSound (std::make_unique<cSoundEffectVoice> (eSoundEffectType::VoiceStolenByEnemy, VoiceData.VOIUnitStolenByEnemy));
	}
	//--------------------------------------------------------------------------
	void playReportSound (cSoundManager& soundManager, const cSavedReportDestroyed&)
	{
		soundManager.playSound (std::make_unique<cSoundEffectVoice> (eSoundEffectType::VoiceDestroyed, getRandom (VoiceData.VOIDestroyedUs)));
	}
	//--------------------------------------------------------------------------
	void playReportSound (cSoundManager& soundManager, const cSavedReportDetected& report)
	{
		if (report.isSubmarine())
		{
			soundManager.playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceDetected, VoiceData.VOISubDetected));
		}
		else
		{
			soundManager.playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceDetected, getRandom (VoiceData.VOIDetected)));
		}
	}
	//--------------------------------------------------------------------------
	void playReportSound (cSoundManager& soundManager, const cSavedReportDisabled&)
	{
		soundManager.playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceDisabled, VoiceData.VOIUnitDisabled));
	}
	//--------------------------------------------------------------------------
	void playReportSound (cSoundManager& soundManager, const cSavedReportTurnStart& report)
	{
		if (!report.researchAreas.empty())
		{
			soundManager.playSound (std::make_unique<cSoundEffectVoice> (eSoundEffectType::VoiceTurnStartReport, VoiceData.VOIResearchComplete));
		}
		else
		{
			int relevantCount = 0;
			for (auto& unitReport : report.unitReports)
			{
				relevantCount += unitReport.count;
				if (relevantCount > 1) break;
			}

			if (relevantCount == 0)
			{
				soundManager.playSound (std::make_unique<cSoundEffectVoice> (eSoundEffectType::VoiceTurnStartReport, VoiceData.VOIStartNone));
			}
			else if (relevantCount == 1)
			{
				soundManager.playSound (std::make_unique<cSoundEffectVoice> (eSoundEffectType::VoiceTurnStartReport, VoiceData.VOIStartOne));
			}
			else if (relevantCount > 1)
			{
				soundManager.playSound (std::make_unique<cSoundEffectVoice> (eSoundEffectType::VoiceTurnStartReport, VoiceData.VOIStartMore));
			}
		}
	}

} // namespace

//------------------------------------------------------------------------------
void playSound (cSoundManager& soundManager, const cSavedReport& report)
{
	switch (report.getType())
	{
		case eSavedReportType::Attacked:
			playReportSound (soundManager, static_cast<const cSavedReportAttacked&> (report));
			break;
		case eSavedReportType::AttackingEnemy:
			playReportSound (soundManager, static_cast<const cSavedReportAttackingEnemy&> (report));
			break;
		case eSavedReportType::CapturedByEnemy:
			playReportSound (soundManager, static_cast<const cSavedReportCapturedByEnemy&> (report));
			break;
		case eSavedReportType::Destroyed:
			playReportSound (soundManager, static_cast<const cSavedReportDestroyed&> (report));
			break;
		case eSavedReportType::Detected:
			playReportSound (soundManager, static_cast<const cSavedReportDetected&> (report));
			break;
		case eSavedReportType::Disabled:
			playReportSound (soundManager, static_cast<const cSavedReportDisabled&> (report));
			break;
		case eSavedReportType::TurnStart:
			playReportSound (soundManager, static_cast<const cSavedReportTurnStart&> (report));
			break;
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
		case eSavedReportType::Producing_PositionBlocked:
		case eSavedReportType::Producing_InsufficientMaterial:
		case eSavedReportType::TurnWait:
		case eSavedReportType::TurnAutoMove:
		case eSavedReportType::Chat:
		case eSavedReportType::PathInterrupted:
		case eSavedReportType::SurveyorAiConfused:
		case eSavedReportType::HostCommand:
		case eSavedReportType::ResourceChanged:
		case eSavedReportType::PlayerEndedTurn:
		case eSavedReportType::LostConnection:
		case eSavedReportType::PlayerDefeated:
		case eSavedReportType::PlayerWins:
		case eSavedReportType::PlayerLeft:
		case eSavedReportType::Upgraded:
		{
			// Empty
			break;
		}
	}
}
