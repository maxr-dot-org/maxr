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

#ifndef ui_sound_soundmanagerH
#define ui_sound_soundmanagerH

#include <set>

#include "utility/position.h"
#include "utility/signal/signalconnectionmanager.h"
#include "utility/thread/mutex.h"

class cSoundEffect;
class cSoundChannel;

class cModel;

class cSoundManager
{
public:
	cSoundManager();
	~cSoundManager();

	void setModel (const cModel* model);

	void mute();
	void unmute();

	void setListenerPosition (const cPosition& listenerPosition);
	void setMaxListeningDistance (int distance);

	void playSound (std::shared_ptr<cSoundEffect> sound, bool loop = false);

	void stopAllSounds();
private:
	struct sStoredSound
	{
		sStoredSound (std::shared_ptr<cSoundEffect> sound_, unsigned int startGameTime_, bool active_);
		sStoredSound (sStoredSound&& other);
		sStoredSound& operator= (sStoredSound && other);

		bool operator< (const sStoredSound& other) const;

		std::shared_ptr<cSoundEffect> sound;
		unsigned int startGameTime;
		bool active;
		cSignalConnectionManager signalConnectionManager;

	private:
		sStoredSound (const sStoredSound& other) = delete;
		sStoredSound& operator= (const sStoredSound& other) = delete;
	};
	cSignalConnectionManager signalConnectionManager;

	const cModel* model;

	cRecursiveMutex playingSoundsMutex;
	std::vector<sStoredSound> playingSounds;

	cPosition listenerPosition;
	int maxListeningDistance;

	bool muted;

	cSoundChannel* getChannelForSound (cSoundEffect& sound);

	void finishedSound (cSoundEffect& sound);

	void updateSoundPosition (cSoundEffect& sound);
	void updateAllSoundPositions();
};

#endif // ui_sound_soundmanagerH
