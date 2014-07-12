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

class cSoundChannel;
class cPosition;

class cSoundEffect
{
public:
	cSoundEffect (eSoundEffectType type, const cSoundChunk& sound);
	virtual ~cSoundEffect ();

	virtual eSoundChannelType getChannelType () const;

	void play (cSoundChannel& channel, bool loop = false);
	void stop ();

	void pause ();
	void resume ();

	cSoundChannel* getChannel () const;

	bool isInConflict (const cSoundEffect& other) const;

	unsigned int getMaxConcurrentConflictedCount () const;

	eSoundConflictHandlingType getSoundConflictHandlingType () const;

	virtual bool hasPosition () const;

	virtual const cPosition& getPosition () const;

	cSignal<void ()> started;
	cSignal<void ()> stopped;

	cSignal<void ()> paused;
	cSignal<void ()> resumed;

	cSignal<void ()> positionChanged;
private:
	cSignalConnectionManager signalConnectionManager;

	eSoundEffectType type;
	const cSoundChunk* sound;

	cSoundChannel* channel;
};

#endif // ui_sound_effects_soundeffectH