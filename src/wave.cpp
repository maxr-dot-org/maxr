/***************************************************************************
 *              Resinstaller - installs missing GFX for MAXR               *
 *              This file is part of the resinstaller project              *
 *   Copyright (C) 2007, 2008 Eiko Oltmanns                                *
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

#include "wave.h"

#include "converter.h"
#include "file.h"
#include "ogg_encode.h"

#include <SDL.h>

cWaveFile::cWaveFile()
{
	buffer = nullptr;
	smplChunk.ListofSampleLoops = nullptr;
}

cWaveFile::~cWaveFile()
{
	//memory has to be deleted manually
	//because we can't determine here if it was allocated by SDL or malloc
}

int readSmplChunk (SDL_RWops* file, cWaveFile& waveFile)
{
	waveFile.smplChunk.ListofSampleLoops = nullptr;

	SDL_RWseek (file, 0, SEEK_SET);
	if (SDL_ReadLE32 (file) != RIFF)
	{
		return 0;
	}

	//ftm chunk length
	SDL_RWseek (file, 16, SEEK_SET);
	Uint32 length = SDL_ReadLE32 (file);

	//data chuck length
	SDL_RWseek (file, length + 4, SEEK_CUR);
	length = SDL_ReadLE32 (file);

	//read smpl chunk
	SDL_RWseek (file, length, SEEK_CUR);
	Uint32 chunkID = SDL_ReadLE32 (file);
	if (chunkID != SMPL)
		return 0;

	length = SDL_ReadLE32 (file);

	waveFile.smplChunk.Manufacturer = SDL_ReadLE32 (file);
	waveFile.smplChunk.Product = SDL_ReadLE32 (file);
	waveFile.smplChunk.SamplePeriod = SDL_ReadLE32 (file);
	waveFile.smplChunk.MIDIUnityNote = SDL_ReadLE32 (file);
	waveFile.smplChunk.MIDIPitchFraction = SDL_ReadLE32 (file);
	waveFile.smplChunk.SMPTEFormat = SDL_ReadLE32 (file);
	waveFile.smplChunk.SMPTEOffset = SDL_ReadLE32 (file);
	waveFile.smplChunk.NumSampleLoops = SDL_ReadLE32 (file);
	waveFile.smplChunk.SamplerData = SDL_ReadLE32 (file);
	waveFile.smplChunk.ListofSampleLoops = (SampleLoop*) malloc (sizeof (SampleLoop) * waveFile.smplChunk.NumSampleLoops);

	for (Uint32 i = 0; i < waveFile.smplChunk.NumSampleLoops; i++)
	{
		waveFile.smplChunk.ListofSampleLoops[i].CuePointID = SDL_ReadLE32 (file);
		waveFile.smplChunk.ListofSampleLoops[i].Type = SDL_ReadLE32 (file);
		waveFile.smplChunk.ListofSampleLoops[i].Start = SDL_ReadLE32 (file);
		waveFile.smplChunk.ListofSampleLoops[i].End = SDL_ReadLE32 (file);
		waveFile.smplChunk.ListofSampleLoops[i].Fraction = SDL_ReadLE32 (file);
		waveFile.smplChunk.ListofSampleLoops[i].PlayCount = SDL_ReadLE32 (file);
	}

	return 1;
}

int loadWAV (const std::filesystem::path& src, cWaveFile& waveFile)
{
	SDL_RWops* file;
	file = openFile (src, "rb");

	//load audio data and format spec
	if (SDL_LoadWAV_RW (file, 0, &waveFile.spec, &waveFile.buffer, &waveFile.length) == nullptr)
	{
		SDL_RWclose (file);
		throw InstallException ("File '" + src.string() + "' may be corrupted" + TEXT_FILE_LF);
	}
	//load smpl chunk
	readSmplChunk (file, waveFile);

	SDL_RWclose (file);
	return 1;
}

void saveWAV (const std::filesystem::path& dst, const cWaveFile& waveFile)
{
	int was_error = 0;
	Chunk chunk;
	Uint16 channel;
	Uint8* buf_pos;
	Uint16 bytespersample;

	const SDL_AudioSpec* spec = &waveFile.spec;
	Uint32 audio_len = waveFile.length;
	Uint8* audio_buf = waveFile.buffer;
	// FMT chunk
	WaveFMT* format = nullptr;

	//open destination file
	std::filesystem::create_directories (dst.parent_path());
	SDL_RWops* file = SDL_RWFromFile (dst.string().c_str(), "wb");
	if (file == nullptr)
	{
		throw InstallException (std::string ("Couldn't open file for writing") + TEXT_FILE_LF);
	}

	// Write the magic header
	if (SDL_WriteLE32 (file, RIFF) == -1)
	{
		throw InstallException (std::string ("Couldn't write to file") + TEXT_FILE_LF);
	}
	SDL_WriteLE32 (file, HEADER_SIZE + audio_len);
	SDL_WriteLE32 (file, WAVE);

	// Write the audio data format chunk
	chunk.magic = FMT;
	chunk.length = FMT_DATA_SIZE;
	chunk.data = (Uint8*) malloc (chunk.length);
	if (chunk.data == nullptr)
	{
		std::cout << "Out of memory\n";
		exit (-1);
	}
	format = (WaveFMT*) chunk.data;
	format->encoding = PCM_CODE;
	format->channels = (Uint16) spec->channels;
	format->frequency = (Uint32) spec->freq;
	switch (spec->format)
	{
		case AUDIO_U8:
		case AUDIO_S8:
			format->bitspersample = 8;
			break;
		default:
			format->bitspersample = 16;
			break;
	};
	bytespersample = format->bitspersample >> 3;
	format->byterate = format->frequency * format->channels * bytespersample;
	format->blockalign = bytespersample * format->channels;
	SDL_WriteLE32 (file, chunk.magic);
	SDL_WriteLE32 (file, chunk.length);
	SDL_WriteLE16 (file, format->encoding);
	SDL_WriteLE16 (file, format->channels);
	SDL_WriteLE32 (file, format->frequency);
	SDL_WriteLE32 (file, format->byterate);
	SDL_WriteLE16 (file, format->blockalign);
	SDL_WriteLE16 (file, format->bitspersample);

	// Write the audio data chunk
	SDL_WriteLE32 (file, DATA);
	SDL_WriteLE32 (file, audio_len);

	buf_pos = (Uint8*) audio_buf;
	while (buf_pos < audio_buf + audio_len)
	{
		for (channel = 0; channel < format->channels; ++channel)
		{
			switch (spec->format)
			{
				case AUDIO_U8:
				case AUDIO_S8:
					SDL_SetError ("8-bit WAV writing unsupported");
					was_error = 1;
					goto done;
					break;
				case AUDIO_U16LSB:
					SDL_WriteLE16 (file, SDL_SwapLE16 (*(Sint16*) buf_pos - TO_SINT16));
					break;
				case AUDIO_U16MSB:
					SDL_WriteLE16 (file, SDL_SwapBE16 (*(Sint16*) buf_pos - TO_SINT16));
					break;
				case AUDIO_S16LSB:
					SDL_WriteLE16 (file, SDL_SwapLE16 (*(Sint16*) buf_pos));
					break;
				case AUDIO_S16MSB:
					SDL_WriteLE16 (file, SDL_SwapBE16 (*(Sint16*) buf_pos));
					break;
			}
			buf_pos += bytespersample;
		}
	}

done:
	if (format != nullptr)
	{
		free (format);
	}

	SDL_RWclose (file);

	if (was_error)
	{
		throw InstallException (std::string ("Error while saving wav file") + TEXT_FILE_LF);
	}
}

void copyPartOfWAV (const std::filesystem::path& src, const std::filesystem::path& dst, Uint8 nr)
{
	cWaveFile waveFile;

	if (nr > 1)
	{
		return;
	}
	try
	{
		//load file from disk
		loadWAV (src, waveFile);

		//claculate absolute start/end positions
		//in the original MAX the smpl chunk of ATTACK5.WAV is missing
		//so we cut at fixed position, if smpl chunk is missing

		Uint8 bytesPerSample = (waveFile.spec.format & 0x00FF) / 8;

		Uint32 start, end;
		if (waveFile.smplChunk.ListofSampleLoops == 0)
		{
			if (nr == 0)
			{
				start = 0;
				end = 35001;
			}
			else if (nr == 1)
			{
				start = 35002;
				end = waveFile.length - 1;
			}
		}
		else
		{
			if (nr == 0)
			{
				start = 0;
				end = (waveFile.smplChunk.ListofSampleLoops[0].Start - 1) * bytesPerSample;
			}
			else if (nr == 1)
			{
				start = waveFile.smplChunk.ListofSampleLoops[0].Start * bytesPerSample;
				end = waveFile.smplChunk.ListofSampleLoops[0].End * bytesPerSample;
			}
		}

		//resize the wave buffer and copy the desired part
		waveFile.length = end - start;
		Uint8* new_buffer = (Uint8*) malloc (waveFile.length);
		if (new_buffer == nullptr)
		{
			std::cout << "Out of memory\n";
			exit (-1);
		}
		memcpy (new_buffer, waveFile.buffer + start, waveFile.length);
		SDL_FreeWAV (waveFile.buffer);
		waveFile.buffer = new_buffer;

		//save resized wave
		if (oggEncode)
		{
			encodeWAV (dst, waveFile);
		}
		else
		{
			saveWAV (dst, waveFile);
		}

		if (waveFile.buffer) free (waveFile.buffer);
		if (waveFile.smplChunk.ListofSampleLoops) free (waveFile.smplChunk.ListofSampleLoops);
	}
	END_INSTALL_FILE (dst.string())
}

void copyWAV (const std::filesystem::path& src, const std::filesystem::path& dst)
{
	if (oggEncode)
	{
		try
		{
			//load, encode and save file
			cWaveFile waveFile;
			loadWAV (src, waveFile);

			encodeWAV (dst, waveFile);

			if (waveFile.buffer) SDL_FreeWAV (waveFile.buffer);
			if (waveFile.smplChunk.ListofSampleLoops) free (waveFile.smplChunk.ListofSampleLoops);
		}
		END_INSTALL_FILE (dst.string())
	}
	else
	{
		//just copy file
		copyFile (src, dst);
	}
}
