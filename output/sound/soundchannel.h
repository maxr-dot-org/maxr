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

#ifndef output_sound_soundchannelH
#define output_sound_soundchannelH

#include "utility/signal/signal.h"
#include "utility/signal/signalconnectionmanager.h"

class cSoundChunk;
class cPosition;

class cSoundChannel
{
public:
	explicit cSoundChannel (int sdlChannelId);

	void play (const cSoundChunk& chunk, bool loop = false);

	void pause ();
	void resume ();

	void stop ();

	void mute ();
	void unmute ();

	bool isPlaying () const;
	bool isPlaying (const cSoundChunk& chunk) const;

	bool isLooping () const;

	bool isPaused () const;

	bool isMuted () const;

	void setVolume (int volume);

	void setPanning (unsigned char left, unsigned char right);
	void setDistance (unsigned char distance);
	void setPosition (short angle, unsigned char distance);

	int getSdlChannelId () const;

	cSignal<void ()> started;
	cSignal<void ()> stopped;

	cSignal<void ()> paused;
	cSignal<void ()> resumed;
private:
	int sdlChannelId;

	bool muted;
	int volume;

	bool looping;

	cSignalConnectionManager signalConnectionManager;

	static void channelFinishedCallback (int channelId);
	static cSignal<void (int)> channelFinished;
};

#endif // output_sound_soundchannelH
