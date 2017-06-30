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

#ifndef ui_sound_effects_soundeffectH
#define ui_sound_effects_soundeffectH

#include "ui/sound/effects/soundeffecttype.h"
#include "ui/sound/effects/soundchanneltype.h"
#include "ui/sound/effects/soundconflicthandlingtype.h"

#include "output/sound/soundchunk.h"

#include "utility/signal/signal.h"
#include "utility/signal/signalconnectionmanager.h"
#include "utility/thread/recursivemutex.h"
#include "utility/thread/mutex.h"

class cSoundChannel;
class cPosition;

class cSoundEffect
{
public:
	cSoundEffect (eSoundEffectType type, const cSoundChunk& sound);

	/**
	 * Destructor of the sound effect.
	 *
	 * The destructor should never be called while the sound is still playing.
	 *
	 * If you do so anyway you risk to produce a deadlock or race condition
	 * because the channel may stop by itself while the destructor is being
	 * executed.
	 */
	virtual ~cSoundEffect();

	virtual eSoundChannelType getChannelType() const;

	void play (cSoundChannel& channel, bool loop = false);
	void stop();

	void pause();
	void resume();

	cSoundChannel* getChannel() const;

	bool isInConflict (const cSoundEffect& other) const;

	bool hasConflictAtSameGameTimeOnly();

	unsigned int getMaxConcurrentConflictedCount() const;

	eSoundConflictHandlingType getSoundConflictHandlingType() const;

	virtual bool hasPosition() const;

	virtual const cPosition& getPosition() const;

	const cSoundChunk* getSound() const;

	cSignal<void ()> started;
	cSignal<void (), cRecursiveMutex> stopped;

	cSignal<void (), cRecursiveMutex> paused;
	cSignal<void (), cRecursiveMutex> resumed;

	cSignal<void ()> positionChanged;
private:
	cSignalConnectionManager signalConnectionManager;

	mutable cMutex channelMutex;

	eSoundEffectType type;
	const cSoundChunk* sound;

	cSoundChannel* channel;
};

#endif // ui_sound_effects_soundeffectH