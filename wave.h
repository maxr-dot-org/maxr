#ifndef wave_h
#define wave_h

#include "resinstaller.h"

/* WAVE files are little-endian */

/*******************************************/
/* Define values for Microsoft WAVE format */
/*******************************************/
#define RIFF		0x46464952		/* "RIFF" */
#define WAVE		0x45564157		/* "WAVE" */
#define FACT		0x74636166		/* "fact" */
#define LIST		0x5453494c		/* "LIST" */
#define FMT		0x20746D66		/* "fmt " */
#define DATA		0x61746164		/* "data" */
#define PCM_CODE	0x0001
#define MS_ADPCM_CODE	0x0002
#define IMA_ADPCM_CODE	0x0011
#define WAVE_MONO	1
#define WAVE_STEREO	2
#define FMT_DATA_SIZE	16			/* size of format chunk data */
#define HEADER_SIZE	28			/* header from "WAVE" onwards */
#define TO_SINT16	0x8000			/* converts Uint16 to Sint16 */

/* Normally, these three chunks come consecutively in a WAVE file */
typedef struct WaveFMT {
/* Not saved in the chunk we read:
	Uint32	FMTchunk;
	Uint32	fmtlen;
*/
	Uint16	encoding;	
	Uint16	channels;		/* 1 = mono, 2 = stereo */
	Uint32	frequency;		/* One of 11025, 22050, or 44100 Hz */
	Uint32	byterate;		/* Average bytes per second */
	Uint16	blockalign;		/* Bytes per sample block */
	Uint16	bitspersample;		/* One of 8, 12, 16, or 4 for ADPCM */
} WaveFMT;

/* The general chunk found in the WAVE file */
typedef struct Chunk {
	Uint32 magic;
	Uint32 length;
	Uint8 *data;			/* Data includes magic and length */
} Chunk;

typedef struct WaveFile
{
	SDL_AudioSpec spec;	//the audio spezification of the file
	Uint32 length;		//length of the audio data in bytes
	Uint8 *buffer;		//the audio data
} WaveFile;

//saves a wav file previously loaded with SDL_LoadWave
int saveWAV( string dst, WaveFile& waveFile);

//copys the part between start and end (given in absolute position in wave buffer) from the file src to file dst
//an 0 as end is interpreted as end of file
int copyPartOfWAV( string src, string dst, float start, float end);

#endif //wave_h