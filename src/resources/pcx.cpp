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

#include "pcx.h"

#include "output/video/video.h"
#include "utility/files.h"
#include "utility/log.h"

#include <algorithm>

/**
*  Class that uses a internal buffer to read from file.
*  It avoid some costly I/O calls.
*/
class cBufferedFile
{
public:
	cBufferedFile() = default;
	~cBufferedFile() { close(); }

	bool open (const char* filename, const char* mode)
	{
		this->file = SDL_RWFromFile (filename, mode);
		return this->file != nullptr;
	}

	void close()
	{
		if (this->file != nullptr)
		{
			SDL_RWclose (this->file);
			this->file = nullptr;
		}
	}

	void seek (int pos, int from)
	{
		SDL_RWseek (file, pos, from);
		start = end = 0;
	}

	void read (void* vbuffer, unsigned int size, unsigned int count)
	{
		char* buffer = reinterpret_cast<char*> (vbuffer);
		unsigned int pos = 0;
		const unsigned int totalSize = size * count;

		while (pos < totalSize)
		{
			this->fillBufferIfNeeded();
			const unsigned int copySize = std::min (end - start, totalSize - pos);

			memcpy (buffer + pos, this->internalBuffer + start, copySize);
			pos += copySize;
			start += copySize;
		}
	}

	Uint16 readLE16()
	{
		Uint16 res;

		read (&res, sizeof (res), 1);
		res = SDL_SwapLE16 (res);
		return res;
	}

private:
	void fillBufferIfNeeded()
	{
		if (start == end)
		{
			start = end = 0;
			end = SDL_RWread (file, internalBuffer, 1, bufferSize);
		}
	}

private:
	static const unsigned int bufferSize = 1024;
	SDL_RWops* file = nullptr;
	char internalBuffer[bufferSize];
	unsigned int start = 0;
	unsigned int end = 0;
};

AutoSurface LoadPCX (const std::string& name)
{
	// Open the file.
	if (!FileExists (name.c_str()))
	{
		// File not found, create empty surface.
		return AutoSurface (SDL_CreateRGBSurface (0, 100, 20, Video.getColDepth(), 0, 0, 0, 0));
	}

	cBufferedFile bufferedFile;
	if (!bufferedFile.open (name.c_str(), "rb"))
	{
		Log.write (SDL_GetError(), cLog::eLogType::Warning); // Image corrupted, create empty surface.
		return AutoSurface (SDL_CreateRGBSurface (0, 100, 20, Video.getColDepth(), 0, 0, 0, 0));
	}
	// Load the image.
	bufferedFile.seek (8, SEEK_SET);
	Uint16 const x = bufferedFile.readLE16() + 1;
	Uint16 const y = bufferedFile.readLE16() + 1;
	AutoSurface s (SDL_CreateRGBSurface (0, x, y, 32, 0, 0, 0, 0));
	if (!s)
	{
		Log.write (SDL_GetError(), cLog::eLogType::Error);
		return nullptr; //app will crash using this
	}
	SDL_SetColorKey (s.get(), SDL_TRUE, 0xFF00FF);

	Uint32* const buf = static_cast<Uint32*> (s->pixels);
	bufferedFile.seek (128, RW_SEEK_SET);
	int k = 0;
	int i = 0;
	do
	{
		char tmp[2];
		bufferedFile.read (tmp, 1, 1);
		unsigned char const temp = tmp[0];
		if (temp >= 0xC0)
		{
			int z = temp - 192;
			if (z + k > x)
				z = x - k;
			bufferedFile.read (tmp, 1, 1);
			unsigned char const temp = tmp[0];
			for (int j = 0; j < z; ++j)
			{
				buf[k + i * x] = temp;
				++k;
				if (k == x) break;
			}
		}
		else
		{
			buf[k + i * x] = temp;
			++k;
		}
		if (k == x)
		{
			k = 0;
			++i;
		}
	} while (i != y);

	// Convert from palette to true colour.
	int colors[256];
	bufferedFile.seek (-768, SEEK_END);
	for (int i = 0; i != 256; ++i)
	{
		Uint8 rgb[3];

		bufferedFile.read (rgb, sizeof (rgb), 1);
		colors[i] = SDL_MapRGB (s->format, rgb[0], rgb[1], rgb[2]);
	}
	for (int i = 0; i != x * y; ++i)
	{
		buf[i] = colors[buf[i]];
	}
	return s;
}
