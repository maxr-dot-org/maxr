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

#ifndef wave_h
#define wave_h

#include "resinstaller.h"

#include <filesystem>

// WAVE files are little-endian
// a detailed description of the wav format can be found here:
// http://www.sonicspot.com/guide/wavefiles.html

/*******************************************/
/* Define values for Microsoft WAVE format */
/*******************************************/
#define RIFF 0x46464952 // "RIFF"
#define WAVE 0x45564157 // "WAVE"
#define FACT 0x74636166 // "fact"
#define LIST 0x5453494c // "LIST"
#define FMT 0x20746D66 // "fmt "
#define DATA 0x61746164 // "data"
#define SMPL 0x6C706D73 // "smpl"
#define PCM_CODE 0x0001
#define MS_ADPCM_CODE 0x0002
#define IMA_ADPCM_CODE 0x0011
#define WAVE_MONO 1
#define WAVE_STEREO 2
#define FMT_DATA_SIZE 16 // size of format chunk data
#define HEADER_SIZE 28 // header from "WAVE" onwards
#define TO_SINT16 0x8000 // converts Uint16 to Sint16

//the format chunk of the wav file
typedef struct WaveFMT
{
	Uint16 encoding;
	Uint16 channels; // 1 = mono, 2 = stereo
	Uint32 frequency; // One of 11025, 22050, or 44100 Hz
	Uint32 byterate; // Average bytes per second
	Uint16 blockalign; // Bytes per sample block
	Uint16 bitspersample; // One of 8, 12, 16, or 4 for ADPCM
} WaveFMT;

//The general chunk found in the WAVE file
typedef struct Chunk
{
	Uint32 magic;
	Uint32 length;
	Uint8* data; //Data includes magic and length
} Chunk;

//sample loop structure. defines one playback loop
typedef struct SampleLoop
{
	Uint32 CuePointID;
	Uint32 Type;
	Uint32 Start; //start of the sample loop in samples
	Uint32 End; //end of the sample loop in samples
	Uint32 Fraction;
	Uint32 PlayCount;
} SampleLoop;

//the smpl chunk. defines loops in the wave file
typedef struct SmplChunk
{
	Uint32 Manufacturer;
	Uint32 Product;
	Uint32 SamplePeriod;
	Uint32 MIDIUnityNote;
	Uint32 MIDIPitchFraction;
	Uint32 SMPTEFormat;
	Uint32 SMPTEOffset;
	Uint32 NumSampleLoops;
	Uint32 SamplerData;
	SampleLoop* ListofSampleLoops;
} SmplChunk;

class cWaveFile
{
public:
	cWaveFile();
	~cWaveFile();
	SDL_AudioSpec spec; //the audio spezification of the file
	Uint32 length; //length of the audio data in bytes
	Uint8* buffer; //the audio data
	SmplChunk smplChunk; //smpl Chunk, defines loops in the wave file
};

//reads an Smpl chunk if there is any in the wave file
int readSmplChunk (SDL_RWops* file, cWaveFile& waveFile);

//loads an wav file into the cWaveFile structure
int loadWAV (const std::filesystem::path& src, cWaveFile& waveFile);

//saves a cWaveFile structure to a physical wav file
void saveWAV (const std::filesystem::path& dst, const cWaveFile& waveFile);

//copys a part of the src file to the dst file
//the parts are defined by the wave files smpl chunk
//currently 0 means the part before the first SampleLoop
//and		1 means the first sample loop
//if oggEncode is set, this function encodes the file bevor saving it
//the extension .wav is then automatically replaced by .ogg
void copyPartOfWAV (const std::filesystem::path& src, const std::filesystem::path& dst, Uint8 nr);

//if oggEncode is set, this function loads encodes and saves a wave file
//the extension .wav is then automatically replaced by .ogg
//if oggEncode is 0, the wave file is just copied
void copyWAV (const std::filesystem::path& src, const std::filesystem::path& dst);

#endif //wave_h
