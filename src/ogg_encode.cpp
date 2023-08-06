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

#include "ogg_encode.h"

#include "resinstaller.h"
#include "wave.h"

#include <SDL.h>
#include <string>
#include <vorbis/vorbisenc.h>

void encodeWAV (std::filesystem::path fileName, cWaveFile& waveFile)
{
	static unsigned char serialNr = 0;
	int ret;

	ogg_stream_state oggStream;
	ogg_page oggPage;
	ogg_packet oggPacket;

	vorbis_info vorbisInfo;
	vorbis_comment vorbisComment;
	vorbis_dsp_state vorbisDSPState;
	vorbis_block vorbisBlock;

	//if the file name ends with .wav it has to be replaced with .ogg
	std::string extension = fileName.extension().string();
	if (extension.compare (".wav") == 0 || extension.compare (".WAV") == 0)
	{
		fileName.replace_extension (".ogg");
	}

	std::filesystem::create_directories (fileName.parent_path());
	SDL_RWops* file = SDL_RWFromFile (fileName.u8string().c_str(), "wb");
	if (file == nullptr)
	{
		throw InstallException (std::string ("Couldn't open file for writing") + TEXT_FILE_LF);
	}

	//begin initialisations and encoder setup
	Uint8 bytesPerSample = (waveFile.spec.format & 0x00FF) / 8;

	//curently only 2 bps supported
	if (bytesPerSample != 2)
	{
		throw InstallException (std::string ("Encoding of wave files with ") + std::to_string (bytesPerSample) + " bytes per sample not supported" + TEXT_FILE_LF);
	}

	vorbis_info_init (&vorbisInfo);
	ret = vorbis_encode_init_vbr (&vorbisInfo, waveFile.spec.channels, waveFile.spec.freq, (float) VORBIS_QUALITY);
	if (ret != 0)
	{
		throw InstallException (std::string ("Couldn't initialize vorbis encoder") + TEXT_FILE_LF);
	}

	vorbis_comment_init (&vorbisComment);
	vorbis_analysis_init (&vorbisDSPState, &vorbisInfo);
	vorbis_block_init (&vorbisDSPState, &vorbisBlock);

	ogg_stream_init (&oggStream, serialNr);
	serialNr++;

	//build the 3 nessesary ogg headers
	ogg_packet header;
	ogg_packet header_comm;
	ogg_packet header_code;

	vorbis_analysis_headerout (&vorbisDSPState, &vorbisComment, &header, &header_comm, &header_code);
	ogg_stream_packetin (&oggStream, &header);
	ogg_stream_packetin (&oggStream, &header_comm);
	ogg_stream_packetin (&oggStream, &header_code);

	//flush the ogg stream
	while (ogg_stream_flush (&oggStream, &oggPage) != 0)
	{
		SDL_RWwrite (file, oggPage.header, oggPage.header_len, 1);
		SDL_RWwrite (file, oggPage.body, oggPage.body_len, 1);
	}

	//begin encoding
	int eos = 0;
	Uint32 dataPos = 0; //read position of the wave buffer in bytes
	while (!eos)
	{
		float** buffer = vorbis_analysis_buffer (&vorbisDSPState, READ);
		int i;
		for (i = 0; i < READ; i++)
		{
			//check end of data
			if (dataPos >= waveFile.length) break;

			for (int c = 0; c < waveFile.spec.channels; c++)
			{
				buffer[c][i] = (Sint16) ((waveFile.buffer[dataPos + (c * bytesPerSample) + 1] << 8) | (0x00FF & (Sint16) waveFile.buffer[dataPos + (c * bytesPerSample)])) / 32768.f;
			}

			dataPos += bytesPerSample * waveFile.spec.channels;
		}

		vorbis_analysis_wrote (&vorbisDSPState, i);

		while (vorbis_analysis_blockout (&vorbisDSPState, &vorbisBlock) == 1)
		{
			vorbis_analysis (&vorbisBlock, nullptr);
			vorbis_bitrate_addblock (&vorbisBlock);

			while (vorbis_bitrate_flushpacket (&vorbisDSPState, &oggPacket))
			{
				ogg_stream_packetin (&oggStream, &oggPacket);

				while (!eos)
				{
					int result = ogg_stream_pageout (&oggStream, &oggPage);
					if (result == 0) break;
					SDL_RWwrite (file, oggPage.header, 1, oggPage.header_len);
					SDL_RWwrite (file, oggPage.body, 1, oggPage.body_len);

					if (ogg_page_eos (&oggPage)) eos = 1;
				}
			}
		}
	}

	SDL_RWclose (file);

	// clean up
	ogg_stream_clear (&oggStream);
	vorbis_block_clear (&vorbisBlock);
	vorbis_dsp_clear (&vorbisDSPState);
	vorbis_comment_clear (&vorbisComment);
	vorbis_info_clear (&vorbisInfo);
}
