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

#include "output/sound/soundchunk.h"

#include "utility/log.h"

//------------------------------------------------------------------------------
bool cSoundChunk::operator== (const cSoundChunk& other) const
{
	return sdlSound == other.sdlSound;
}

//--------------------------------------------------------------------------
void cSoundChunk::load (const std::filesystem::path& fileName)
{
	sdlSound = SaveSdlMixChunkPointer (Mix_LoadWAV (fileName.string().c_str()));
	if (sdlSound == nullptr)
	{
		Log.error ("Mix_LoadWAV returned nullptr on loading file '" + fileName.u8string() + "'. Reason: " + Mix_GetError());
	}
}

//--------------------------------------------------------------------------
bool cSoundChunk::empty() const
{
	return sdlSound == nullptr;
}

//--------------------------------------------------------------------------
std::chrono::milliseconds cSoundChunk::getLength() const
{
	if (!sdlSound) return std::chrono::milliseconds (0);

	int freq = 0;
	Uint16 fmt = 0;
	int chans = 0;

	if (!Mix_QuerySpec (&freq, &fmt, &chans)) return std::chrono::milliseconds (0);

	auto points = (sdlSound->alen / ((fmt & 0xFF) / 8));

	auto frames = (points / chans);

	return std::chrono::milliseconds ((frames * 1000) / freq);
}

//--------------------------------------------------------------------------
Mix_Chunk* cSoundChunk::getSdlSound() const
{
	return sdlSound.get();
}

//--------------------------------------------------------------------------
void cSoundChunk::SdlMixChunkDeleter::operator() (Mix_Chunk* chunk) const
{
	Mix_FreeChunk (chunk);
}
