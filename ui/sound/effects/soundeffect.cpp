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

#include "ui/sound/effects/soundeffect.h"
#include "output/sound/soundchannel.h"

//--------------------------------------------------------------------------
cSoundEffect::cSoundEffect (eSoundEffectType type_, const cSoundChunk& sound_) :
	type (type_),
	sound (&sound_),
	channel (nullptr)
{}

//--------------------------------------------------------------------------
cSoundEffect::~cSoundEffect ()
{
	signalConnectionManager.clear ();
	if (channel)
	{
		channel->stop ();
	}
}

//--------------------------------------------------------------------------
eSoundChannelType cSoundEffect::getChannelType () const
{
	return eSoundChannelType::General;
}

//--------------------------------------------------------------------------
void cSoundEffect::play (cSoundChannel& channel_, bool loop)
{
	stop ();

	channel = &channel_;

	channel->play (*sound, loop);

	signalConnectionManager.connect (channel->stopped, [this]()
	{
		signalConnectionManager.disconnectAll ();
		channel = nullptr;
		stopped ();
	});
	signalConnectionManager.connect (channel->paused, [this](){ paused (); });
	signalConnectionManager.connect (channel->resumed, [this](){ resumed (); });

	started ();
}

//--------------------------------------------------------------------------
void cSoundEffect::stop ()
{
	signalConnectionManager.disconnectAll ();
	if (channel)
	{
		channel->stop ();
		channel = nullptr;

		stopped ();
	}
}

//--------------------------------------------------------------------------
void cSoundEffect::pause ()
{
	if (channel)
	{
		channel->pause ();

		paused ();
	}
}

//--------------------------------------------------------------------------
void cSoundEffect::resume ()
{
	if (channel)
	{
		channel->resume ();

		resumed ();
	}
}

//--------------------------------------------------------------------------
bool cSoundEffect::isInConflict (const cSoundEffect& other) const
{
	return type == other.type;
}

//--------------------------------------------------------------------------
unsigned int cSoundEffect::getMaxConcurrentConflictedCount () const
{
	switch (type)
	{
	default:
	case eSoundEffectType::EffectReload:
	case eSoundEffectType::EffectRepair:
	case eSoundEffectType::EffectPlaceMine:
	case eSoundEffectType::EffectClearMine:
	case eSoundEffectType::EffectStartWork:
	case eSoundEffectType::EffectStopWork:
	case eSoundEffectType::EffectLoad:
	case eSoundEffectType::EffectActivate:
	case eSoundEffectType::EffectStartMove:
	case eSoundEffectType::EffectStopMove:
	case eSoundEffectType::EffectAlert:
	case eSoundEffectType::VoiceNoPath:
	case eSoundEffectType::VoiceCommandoAction:
	case eSoundEffectType::VoiceReload:
	case eSoundEffectType::VoiceRepair:
	case eSoundEffectType::VoiceDisabled:
	case eSoundEffectType::VoiceStolenByEnemy:
	case eSoundEffectType::VoiceDetected:
		return 0;
	case eSoundEffectType::VoiceUnitStatus:
		return 1;
	case eSoundEffectType::EffectExplosion:
	case eSoundEffectType::EffectAbsorb:
		return std::numeric_limits<unsigned int>::max();
	}
}

//--------------------------------------------------------------------------
eSoundConflictHandlingType cSoundEffect::getSoundConflictHandlingType () const
{
	switch (type)
	{
	default:
	case eSoundEffectType::EffectReload:
	case eSoundEffectType::EffectRepair:
	case eSoundEffectType::EffectPlaceMine:
	case eSoundEffectType::EffectClearMine:
	case eSoundEffectType::EffectStartWork:
	case eSoundEffectType::EffectStopWork:
	case eSoundEffectType::EffectLoad:
	case eSoundEffectType::EffectActivate:
	case eSoundEffectType::EffectStartMove:
	case eSoundEffectType::EffectStopMove:
	case eSoundEffectType::EffectAlert:
	case eSoundEffectType::VoiceNoPath:
	case eSoundEffectType::VoiceCommandoAction:
	case eSoundEffectType::VoiceReload:
	case eSoundEffectType::VoiceRepair:
	case eSoundEffectType::VoiceDisabled:
	case eSoundEffectType::VoiceStolenByEnemy:
	case eSoundEffectType::VoiceDetected:
		return eSoundConflictHandlingType::DiscardNew;
	case eSoundEffectType::VoiceUnitStatus:
		return eSoundConflictHandlingType::StopOld;
	case eSoundEffectType::EffectExplosion:
	case eSoundEffectType::EffectAbsorb:
		return eSoundConflictHandlingType::PlayAnyway;
	}
}