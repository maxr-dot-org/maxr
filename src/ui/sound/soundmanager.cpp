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

#include "ui/sound/soundmanager.h"

#include "game/data/model.h"
#include "output/sound/soundchannel.h"
#include "output/sound/sounddevice.h"
#include "settings.h"
#include "ui/sound/effects/soundeffect.h"
#include "utility/listhelpers.h"

//--------------------------------------------------------------------------
cSoundManager::sStoredSound::sStoredSound (std::shared_ptr<cSoundEffect> sound_, unsigned int startGameTime_, bool active_) :
	sound (std::move (sound_)),
	startGameTime (startGameTime_),
	active (active_)
{}

//--------------------------------------------------------------------------
bool cSoundManager::sStoredSound::operator< (const sStoredSound& other) const
{
	return startGameTime < other.startGameTime;
}

//--------------------------------------------------------------------------
cSoundManager::cSoundManager() :
	listenerPosition (0, 0),
	maxListeningDistance (20)
{}

//--------------------------------------------------------------------------
cSoundManager::~cSoundManager()
{
	signalConnectionManager.disconnectAll();
	std::unique_lock<std::recursive_mutex> playingSoundsLock (playingSoundsMutex);
	for (auto& storedSound : playingSounds)
	{
		storedSound.sound->stop();
	}
}

//--------------------------------------------------------------------------
void cSoundManager::setModel (const cModel* model_)
{
	model = model_;
}

//--------------------------------------------------------------------------
void cSoundManager::mute()
{
	muted = true;

	std::unique_lock<std::recursive_mutex> playingSoundsLock (playingSoundsMutex);
	for (auto& storedSound : playingSounds)
	{
		if (!storedSound.active) continue;

		auto* channel = storedSound.sound->getChannel();
		if (channel)
		{
			// Loop sounds are just muted so that they can be continued later on.
			// Normal effects are stopped entirely, so that we do not get cut sounds
			// on unmuting.
			if (channel->isLooping())
			{
				channel->mute();
			}
			else
			{
				storedSound.sound->stop();
			}
		}
	}
}

//--------------------------------------------------------------------------
void cSoundManager::unmute()
{
	muted = false;

	std::unique_lock<std::recursive_mutex> playingSoundsLock (playingSoundsMutex);
	for (auto& storedSound : playingSounds)
	{
		if (!storedSound.active) continue;

		auto channel = storedSound.sound->getChannel();
		if (channel)
		{
			channel->unmute();
		}
	}
}

//--------------------------------------------------------------------------
void cSoundManager::setListenerPosition (const cPosition& listenerPosition_)
{
	listenerPosition = listenerPosition_;
	updateAllSoundPositions();
}

//--------------------------------------------------------------------------
void cSoundManager::setMaxListeningDistance (int distance)
{
	maxListeningDistance = distance;
	updateAllSoundPositions();
}

//--------------------------------------------------------------------------
void cSoundManager::playSound (std::shared_ptr<cSoundEffect> sound, bool loop)
{
	if (!sound) return;

	if (muted && !loop) return;

	std::unique_lock<std::recursive_mutex> playingSoundsLock (playingSoundsMutex);

	EraseIf (playingSounds, [] (const sStoredSound& storedSound) { return !storedSound.active; });

	const unsigned int currentGameTime = model ? model->getGameTime() : 0;

	const auto soundConflictHandlingType = sound->getSoundConflictHandlingType();

	if (soundConflictHandlingType != eSoundConflictHandlingType::PlayAnyway)
	{
		const auto isInConflict = [&sound, currentGameTime] (const sStoredSound& storedSound) {
			return (!sound->hasConflictAtSameGameTimeOnly() || storedSound.startGameTime == currentGameTime) && sound->isInConflict (*storedSound.sound);
		};

		// count conflicts and erase sounds that are no longer active
		std::size_t conflicts = ranges::count_if (playingSounds, isInConflict);

		if (conflicts > sound->getMaxConcurrentConflictedCount())
		{
			switch (soundConflictHandlingType)
			{
				case eSoundConflictHandlingType::DiscardNew:
					return;
				case eSoundConflictHandlingType::StopOld:
				{
					// stop oldest sounds that are in conflict (list is sorted by start game time)
					for (auto& playingSound : playingSounds)
					{
						if (conflicts <= sound->getMaxConcurrentConflictedCount()) break;
						if (isInConflict (playingSound))
						{
							playingSound.sound->stop();
							--conflicts;
						}
					}
				}
				break;
				case eSoundConflictHandlingType::PlayAnyway: break;
			}
		}
	}

	// start new sound
	auto channel = getChannelForSound (*sound);

	if (channel == nullptr) return;

	sound->play (*channel, loop);

	sStoredSound playingSound (std::move (sound), currentGameTime, true);
	// Sound list is always sorted by start game time.
	auto it = std::lower_bound (playingSounds.begin(), playingSounds.end(), playingSound);
	it = playingSounds.insert (it, std::move (playingSound));
	auto& soundRef = *it->sound;
	signalConnectionManager.connect (soundRef.stopped, [this, &soundRef]() { finishedSound (soundRef); });

	updateSoundPosition (soundRef);
	it->signalConnectionManager.connect (soundRef.positionChanged, [this, &soundRef]() { updateSoundPosition (soundRef); });
}

//--------------------------------------------------------------------------
void cSoundManager::stopAllSounds()
{
	std::unique_lock<std::recursive_mutex> playingSoundsLock (playingSoundsMutex);
	for (auto& storedSound : playingSounds)
	{
		storedSound.sound->stop();
	}
}

//--------------------------------------------------------------------------
cSoundChannel* cSoundManager::getChannelForSound (cSoundEffect& sound)
{
	switch (sound.getChannelType())
	{
		default:
		case eSoundChannelType::General:
			return cSoundDevice::getInstance().getFreeSoundEffectChannel();
		case eSoundChannelType::Voice:
			return cSoundDevice::getInstance().getFreeVoiceChannel();
	}
}

//--------------------------------------------------------------------------
void cSoundManager::finishedSound (cSoundEffect& sound)
{
	std::unique_lock<std::recursive_mutex> playingSoundsLock (playingSoundsMutex);

	auto iter = ranges::find_if (playingSounds, [&sound] (const sStoredSound& entry) { return entry.sound.get() == &sound; });

	if (iter != playingSounds.end())
	{
		iter->active = false;
	}
}

//--------------------------------------------------------------------------
void cSoundManager::updateSoundPosition (cSoundEffect& sound)
{
	if (!cSettings::getInstance().is3DSound()) return;

	if (!sound.hasPosition()) return;

	auto channel = sound.getChannel();

	if (!channel) return;

	cFixedVector<double, 2> offset (sound.getPosition() - listenerPosition);

	const auto distance = offset.l2Norm();

	// Fade volume down when distance comes close to the maximum listening distance
	const auto soundDistance = static_cast<unsigned char> (std::min (static_cast<int> (distance / maxListeningDistance * std::numeric_limits<unsigned char>::max()), (int) std::numeric_limits<unsigned char>::max()));

	channel->setDistance (soundDistance);

	// compute panning
	double pan = 0;
	if (std::abs (distance) > std::numeric_limits<double>::epsilon() * std::abs (distance))
	{
		// y coordinate is taken into account as well
		offset /= distance;
		pan = offset[0] * std::cos (offset[1] * M_PI / 2);

		// if we are close to the position the panning will be reduced in effect.
		const auto distanceFactor = 1. - (1. / std::max (5. * distance / maxListeningDistance, 1.));
		pan *= distanceFactor;

		// just to make 100% sure that there are no overflows
		pan = std::clamp (pan, -1., -1.);
	}
	const auto right = std::min ((int) (std::numeric_limits<unsigned char>::max() * (pan + 1)), (int) std::numeric_limits<unsigned char>::max());
	const auto left = std::min ((int) (std::numeric_limits<unsigned char>::max() * (-1. * pan + 1)), (int) std::numeric_limits<unsigned char>::max());

	channel->setPanning (left, right);
}

//--------------------------------------------------------------------------
void cSoundManager::updateAllSoundPositions()
{
	for (auto& storedSound : playingSounds)
	{
		updateSoundPosition (*storedSound.sound);
	}
}
