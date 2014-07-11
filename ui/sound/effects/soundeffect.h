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

#include "utility/signal/signal.h"
#include "utility/signal/signalconnectionmanager.h"
#include "output/sound/soundchunk.h"

class cSoundChannel;

enum class eSoundChannelType
{
	General,
	Voice
};

enum class eSoundConflictHandlingType
{
	StopOld,
	DiscardNew,
	PlayAnyway
};

enum class eSoundEffectType
{
	// Effects
	EffectReload,
	EffectRepair,
	EffectPlaceMine,
	EffectClearMine,
	EffectStartWork,
	EffectStopWork,
	EffectLoad,
	EffectActivate,
	EffectStartMove,
	EffectStopMove,
	EffectExplosion,
	EffectAbsorb,
	EffectAlert,

	// Voices
	VoiceNoPath,
	VoiceCommandoAction,
	VoiceReload,
	VoiceRepair,
	VoiceDisabled,
	VoiceStolenByEnemy,
	VoiceDetected,
	VoiceUnitStatus
};

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

	bool isInConflict (const cSoundEffect& other) const;

	unsigned int getMaxConcurrentConflictedCount () const;

	eSoundConflictHandlingType getSoundConflictHandlingType () const;

	cSignal<void ()> started;
	cSignal<void ()> stopped;

	cSignal<void ()> paused;
	cSignal<void ()> resumed;
private:
	cSignalConnectionManager signalConnectionManager;

	eSoundEffectType type;
	const cSoundChunk* sound;

	cSoundChannel* channel;
};

#endif // ui_sound_effects_soundeffectH