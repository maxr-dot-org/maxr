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

#include "unitreport.h"

#include "game/data/gui/gameguistate.h"
#include "game/data/player/player.h"
#include "game/data/units/building.h"
#include "game/data/units/vehicle.h"
#include "game/logic/movejob.h"
#include "resources/sound.h"
#include "ui/sound/effects/soundeffect.h"
#include "ui/sound/effects/soundeffectvoice.h"
#include "ui/sound/soundmanager.h"
#include "utility/listhelpers.h"
#include "utility/random.h"

namespace
{
	//------------------------------------------------------------------------------
	void makeReport (cSoundManager& soundManager, const cGameGuiState& gameGuiState, const cBuilding& building)
	{
		if (building.getStaticData().canResearch && building.isUnitWorking() && building.getOwner() && ranges::contains (gameGuiState.currentTurnResearchAreasFinished, building.getResearchArea()))
		{
			soundManager.playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceUnitStatus, VoiceData.VOIResearchComplete));
		}
	}

	//------------------------------------------------------------------------------
	void makeReport (cSoundManager& soundManager, const cGameGuiState&, const cVehicle& vehicle)
	{
		if (vehicle.isDisabled())
		{
			// Disabled:
			soundManager.playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceUnitStatus, getRandom (VoiceData.VOIUnitDisabledByEnemy)));
		}
		else if (vehicle.data.getHitpoints() > vehicle.data.getHitpointsMax() / 2)
		{
			// Status green
			if (vehicle.getMoveJob() && vehicle.getMoveJob()->getEndMoveAction().isAttacking())
			{
				soundManager.playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceUnitStatus, getRandom (VoiceData.VOIAttacking)));
			}
			else if (vehicle.isSurveyorAutoMoveActive())
			{
				soundManager.playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceUnitStatus, getRandom (VoiceData.VOISurveying)));
			}
			else if (vehicle.data.getSpeed() == 0)
			{
				// no more movement
				soundManager.playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceUnitStatus, VoiceData.VOINoSpeed));
			}
			else if (vehicle.isUnitBuildingABuilding())
			{
				// Beim bau:
				if (!vehicle.getBuildTurns())
				{
					// Bau beendet:
					soundManager.playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceUnitStatus, getRandom (VoiceData.VOIBuildDone)));
				}
			}
			else if (vehicle.isUnitClearing())
			{
				// removing dirt
				soundManager.playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceUnitStatus, VoiceData.VOIClearing));
			}
			else if (vehicle.getStaticUnitData().canAttack && vehicle.data.getAmmo() == 0)
			{
				// no ammo left
				soundManager.playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceUnitStatus, getRandom (VoiceData.VOIAmmoEmpty)));
			}
			else if (vehicle.getStaticUnitData().canAttack && vehicle.data.getAmmo() <= vehicle.data.getAmmoMax() / 4)
			{
				// red ammo-status but still ammo left
				soundManager.playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceUnitStatus, getRandom (VoiceData.VOIAmmoLow)));
			}
			else if (vehicle.isSentryActive())
			{
				// on sentry:
				soundManager.playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceUnitStatus, VoiceData.VOISentry));
			}
			else if (vehicle.isUnitClearingMines())
			{
				soundManager.playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceUnitStatus, getRandom (VoiceData.VOIClearingMines)));
			}
			else if (vehicle.isUnitLayingMines())
			{
				soundManager.playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceUnitStatus, VoiceData.VOILayingMines));
			}
			else
			{
				soundManager.playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceUnitStatus, getRandom (VoiceData.VOIOK)));
			}
		}
		else if (vehicle.data.getHitpoints() > vehicle.data.getHitpointsMax() / 4)
		{
			// Status yellow:
			soundManager.playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceUnitStatus, getRandom (VoiceData.VOIStatusYellow)));
		}
		else
		{
			// Status red:
			soundManager.playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceUnitStatus, getRandom (VoiceData.VOIStatusRed)));
		}
	}

} // namespace

//------------------------------------------------------------------------------
void makeReport (cSoundManager& soundManager, const cGameGuiState& gameGuiState, const cUnit& unit)
{
	if (auto* vehicle = dynamic_cast<const cVehicle*> (&unit)) makeReport (soundManager, gameGuiState, *vehicle);
	if (auto* building = dynamic_cast<const cBuilding*> (&unit)) makeReport (soundManager, gameGuiState, *building);
}
