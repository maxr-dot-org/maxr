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

#include "game/data/report/unit/savedreportattacked.h"
#include "game/data/units/unit.h"
#include "ui/sound/soundmanager.h"
#include "ui/sound/effects/soundeffectvoice.h"
#include "resources/sound.h"
#include "utility/random.h"

//------------------------------------------------------------------------------
cSavedReportAttacked::cSavedReportAttacked (const cUnit& unit) :
	cSavedReportUnit (unit),
	unitName (unit.getDisplayName())
{}

//------------------------------------------------------------------------------
eSavedReportType cSavedReportAttacked::getType() const
{
	return eSavedReportType::Attacked;
}

//------------------------------------------------------------------------------
std::string cSavedReportAttacked::getText() const
{
	return unitName + " " + lngPack.i18n ("Text~Comp~Attacked");
}

//------------------------------------------------------------------------------
void cSavedReportAttacked::playSound (cSoundManager& soundManager) const
{
	soundManager.playSound (std::make_unique<cSoundEffectVoice> (eSoundEffectType::VoiceAttackingUs, getRandom (VoiceData.VOIAttackingUs)));
}
