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
#include "netmessage.h"
#include "ui/sound/soundmanager.h"
#include "ui/sound/effects/soundeffectvoice.h"
#include "sound.h"
#include "utility/random.h"

//------------------------------------------------------------------------------
cSavedReportDetected::cSavedReportDetected (const cUnit& unit) :
	cSavedReportUnit (unit),
	unitName (unit.getDisplayName ()),
	playerName (unit.getOwner ()->getName ())
{}

//------------------------------------------------------------------------------
cSavedReportDetected::cSavedReportDetected (cNetMessage& message) :
	cSavedReportUnit (message)
{
	unitName = message.popString ();
	playerName = message.popString ();
}

//------------------------------------------------------------------------------
cSavedReportDetected::cSavedReportDetected (const tinyxml2::XMLElement& element) :
	cSavedReportUnit (element)
{
	unitName = element.Attribute ("unitName");
	playerName = element.Attribute ("playerName");
}

//------------------------------------------------------------------------------
void cSavedReportDetected::pushInto (cNetMessage& message) const
{
	message.pushString (playerName);
	message.pushString (unitName);

	cSavedReportUnit::pushInto (message);
}

//------------------------------------------------------------------------------
void cSavedReportDetected::pushInto (tinyxml2::XMLElement& element) const
{
	element.SetAttribute ("unitName", unitName.c_str ());
	element.SetAttribute ("playerName", playerName.c_str ());

	cSavedReportUnit::pushInto (element);
}

//------------------------------------------------------------------------------
eSavedReportType cSavedReportDetected::getType () const
{
	return eSavedReportType::Detected;
}

//------------------------------------------------------------------------------
std::string cSavedReportDetected::getText () const
{
	return unitName + " (" + playerName + ") " + lngPack.i18n ("Text~Comp~Detected");
}

//------------------------------------------------------------------------------
void cSavedReportDetected::playSound (cSoundManager& soundManager) const
{
	const auto& unitData = *getUnitId ().getUnitDataOriginalVersion ();

	if (unitData.isStealthOn & TERRAIN_SEA && unitData.canAttack)
	{
		soundManager.playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceDetected, VoiceData.VOISubDetected));
	}
	else
	{
		soundManager.playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceDetected, getRandom (VoiceData.VOIDetected)));
	}
}
