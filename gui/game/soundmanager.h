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

#ifndef gui_game_soundmanagerH
#define gui_game_soundmanagerH

#include "../../main.h" // random()
#include "../../utility/position.h"

class cSoundChunk;
class cSoundChannel;

class cSoundManager
{
public:
	cSoundManager ();

	void mute ();
	void unmute ();

	void setListenerPosition (const cPosition& listenerPosition);
	void setMaxListeningDistance (int distance);

	void playSound (const cSoundChunk& sound, bool disallowDuplicate = true);
	void playSound (const cSoundChunk& sound, const cPosition& position, bool disallowDuplicate = true);

	void playVoice (const cSoundChunk& sound, bool disallowDuplicate = true);

	void startSoundLoop (const cSoundChunk& sound);
	void stopSoundLoop ();

private:
	cSoundChannel* loopSoundChannel;

	cPosition listenerPosition;
	int maxListeningDistance;

	bool muted;

	bool isAlreadyPlayingSound (const cSoundChunk& sound);
};

#endif // gui_game_soundmanagerH
