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

#include "soundmanager.h"

#include "../../output/sound/sounddevice.h"
#include "../../output/sound/soundchannel.h"

//--------------------------------------------------------------------------
cSoundManager::cSoundManager () :
	loopSoundChannel (nullptr),
	muted (false),
	listenerPosition (0, 0),
	maxListeningDistance (20)
{}

//--------------------------------------------------------------------------
void cSoundManager::mute ()
{
	muted = true;
	stopSoundLoop ();
}

//--------------------------------------------------------------------------
void cSoundManager::unmute ()
{
	muted = false;
}

//--------------------------------------------------------------------------
void cSoundManager::setListenerPosition (const cPosition& listenerPosition_)
{
	listenerPosition = listenerPosition_;
}

//--------------------------------------------------------------------------
void cSoundManager::setMaxListeningDistance (int distance)
{
	maxListeningDistance = distance;
}

//--------------------------------------------------------------------------
void cSoundManager::playSound (const cSoundChunk& sound, bool disallowDuplicate)
{
	if (muted) return;

	if (disallowDuplicate && isAlreadyPlayingSound (sound)) return;

	cSoundDevice::getInstance ().getFreeSoundEffectChannel ().play (sound);
}

//--------------------------------------------------------------------------
void cSoundManager::playSound (const cSoundChunk& sound, const cPosition& soundPosition, bool disallowDuplicate)
{
	if (muted) return;

	if (disallowDuplicate && isAlreadyPlayingSound (sound)) return;

	auto& channel = cSoundDevice::getInstance ().getFreeSoundEffectChannel ();
	channel.play (sound);

	//const cPosition offset = soundPosition - listenerPosition;
	//const auto soundDistance = std::max<char> (offset.l2Norm () / maxListeningDistance * 255, 255);
	//const auto temp = (offset.x () == 0 && offset.x () == 0) ? 0 : std::atan2 (offset.y (), offset.x ());
	//const auto soundAngle = (temp > 0 ? temp : (2*M_PI + temp)) * 360 / (2*M_PI) + 90;

	//channel.setPosition (soundAngle, soundDistance);
}

//--------------------------------------------------------------------------
void cSoundManager::playVoice (const cSoundChunk& sound, bool disallowDuplicate)
{
	if (muted) return;

	if (disallowDuplicate && isAlreadyPlayingSound (sound)) return;

	cSoundDevice::getInstance ().getFreeVoiceChannel ().play (sound);
}

//--------------------------------------------------------------------------
void cSoundManager::startSoundLoop (const cSoundChunk& sound)
{
	if (muted) return;

	if (loopSoundChannel)
	{
		loopSoundChannel->stop ();
	}
	else
	{
		loopSoundChannel = &cSoundDevice::getInstance ().getFreeSoundEffectChannel ();
	}
	loopSoundChannel->play (sound, true);
}

//--------------------------------------------------------------------------
void cSoundManager::stopSoundLoop ()
{
	if (loopSoundChannel)
	{
		loopSoundChannel->stop ();
		loopSoundChannel = nullptr;
	}
}

//--------------------------------------------------------------------------
bool cSoundManager::isAlreadyPlayingSound (const cSoundChunk& sound)
{
	return false; // TODO: implement
}