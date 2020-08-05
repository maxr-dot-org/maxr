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

#include "game/data/report/unit/savedreportdetected.h"

#include "game/data/units/unit.h"
#include "game/data/player/player.h"
#include "ui/sound/soundmanager.h"
#include "ui/sound/effects/soundeffectvoice.h"
#include "utility/language.h"
#include "utility/random.h"
#include "resources/sound.h"

//------------------------------------------------------------------------------
cSavedReportDetected::cSavedReportDetected (const cUnit& unit) :
	cSavedReportUnit (unit),
	unitName (unit.getDisplayName()),
	playerName (unit.getOwner()->getName()),
	submarine(unit.getStaticUnitData().isStealthOn & TERRAIN_SEA && unit.getStaticUnitData().canAttack)
{}

//------------------------------------------------------------------------------
eSavedReportType cSavedReportDetected::getType() const
{
	return eSavedReportType::Detected;
}

//------------------------------------------------------------------------------
std::string cSavedReportDetected::getText() const
{
	return unitName + " (" + playerName + ") " + lngPack.i18n ("Text~Comp~Detected");
}

//------------------------------------------------------------------------------
void cSavedReportDetected::playSound (cSoundManager& soundManager) const
{

	if (submarine)
	{
		soundManager.playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceDetected, VoiceData.VOISubDetected));
	}
	else
	{
		soundManager.playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceDetected, getRandom (VoiceData.VOIDetected)));
	}
}
