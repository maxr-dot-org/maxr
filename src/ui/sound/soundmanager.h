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

#include "utility/position.h"
#include "utility/signal/signalconnectionmanager.h"

#include <mutex>
#include <set>

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
	cSoundChannel* getChannelForSound (cSoundEffect& sound);

	void finishedSound (cSoundEffect& sound);

	void updateSoundPosition (cSoundEffect& sound);
	void updateAllSoundPositions();

private:
	struct sStoredSound
	{
		sStoredSound (std::shared_ptr<cSoundEffect> sound_, unsigned int startGameTime_, bool active_);
		sStoredSound (const sStoredSound&) = delete;
		sStoredSound& operator= (const sStoredSound&) = delete;
		sStoredSound (sStoredSound&&) = default;
		sStoredSound& operator= (sStoredSound&&) = default;

		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/61911
		bool operator< (const sStoredSound&) const;
		// clang-format on

		std::shared_ptr<cSoundEffect> sound;
		unsigned int startGameTime;
		bool active;
		cSignalConnectionManager signalConnectionManager;
	};

private:
	cSignalConnectionManager signalConnectionManager;

	const cModel* model = nullptr;

	std::recursive_mutex playingSoundsMutex;
	std::vector<sStoredSound> playingSounds;

	cPosition listenerPosition;
	int maxListeningDistance;

	bool muted = false;
};

#endif // ui_sound_soundmanagerH
