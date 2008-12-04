/*
    MVEPlayer - Interplay MVE multimedia file player
    Copyright (C) 2008 Shaktazuki

	This file is part of MVEPlayer.

    MVEPlayer is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Shaktazuki
    shaktazuki at gmail dot com
*/

/*	MVEPlayer.c - Plays 8-bit Interplay MVE multimedia files.
	Author: Shaktazuki
	FINAL 1.1
*/
#include "MVEPlayer.h"
#include <assert.h>
#ifdef _MSC_VER
#include "sdl.h"
#else
#include <SDL.H>
#include <SDL/SDL.H>
#endif

/* NOAUDIO enables use of speed up / slow down functionality (f = faster, s = slower) */
//#define NOAUDIO
/* if you want fullscreen (stretched to fit 640x480), enable this */
#define FULLSCREEN

/* define (potentially) handled opcode types */
#define STOP_PLAYBACK			0x00
#define FETCH_NEXT_CHUNK		0x01
#define CREATE_TIMER			0x02
#define INIT_AUDIO_BUFFERS		0x03
#define START_AUDIO				0x04
#define INIT_VIDEO_BUFFERS		0x05
#define SEND_BUFFER_TO_DISPLAY	0x07
#define AUDIO_FRAME				0x08
#define INIT_VIDEO_MODE			0x0A
#define SET_PALETTE				0x0C
#define SET_DECODING_MAP		0x0F
#define VIDEO_DATA				0x11

/**************************/
/* internally used macros */
/**************************/

/* fetch LE words from a bytestream x */
#define LE16(x) (*(x) | ((*(x + 1)) << 8))

/******************************/
/* internally used structures */
/******************************/

typedef struct CHUNK
{
	Uint16 length;
	Uint16 type;
} chunk;

typedef struct OPCODE
{
	Uint16 length;
	Uint8 type;
	Uint8 version;
} opcode;

typedef struct BUFSTRUCT
{
	Uint32 length;
	Uint8 *data;
} buffer;

/********************/
/* global variables */
/********************/

/* flow control flags */
Uint8 QUITTING = 0, PAUSED = 0;

/* timer flags */
Uint8 TIMER_CREATED	= 0, TIMER_INIT = 0;

/* audio flags */
Uint8 AUDIO_PLAYING = 0, AUDIO_COMPRESSED = 0, AUDIO_STEREO = 0, SAMPLESIZE16 = 0;

/* screen buffers */
Uint8 *v_backbuf = NULL, *frame_hot = NULL, *frame_cold = NULL;

/* target time between frames */
float ms_per_frame = 0;

/***********************/
/* function prototypes */
/***********************/
#ifndef NOAUDIO
void MVEPlayerAudioCB(void *userdata, Uint8 *stream, Sint32 len);
void MVEPlayerDecodeAudio(buffer *input);
#endif
void MVEPlayerDecodeVideo(Uint16 wblocks, Uint16 hblocks, Uint8 *video_data, Uint8 *decoding_map);
void MVEPlayerEventHandler();

/********************************/
/* primary function entry point */
/********************************/

int MVEPlayer(const char *filename)
{	
	/*************************/
	/* variable declarations */
	/*************************/

	/* MVE validation tools */
	const char idstring[] = "Interplay MVE File\x1A\0\x1A\0\0\x1\x33\x11";
	const Uint8 idlen = 26;
	char fileTestString[26];

	/* timer vars */
	Uint32 timer_rate = 0, current_time = 0;
	Uint16 timer_subdivision = 0;
	float start_time = 0;

	/* audio variables */
#ifndef NOAUDIO
	SDL_AudioSpec *desired = NULL;
	Uint16 file_audio_flags = 0;
	buffer audio_buffer, audio_data_read, temp_audio_buffer;
#endif

	/* video variables */
	Uint32 screen_buffer_size = 0;
	Uint16 width_blocks = 0, height_blocks = 0, width = 0, height = 0;
	Uint8 *map = NULL, *video = NULL, *temp = NULL;
	SDL_Surface *screen = NULL, *frame_buf = NULL;
	SDL_Rect movie_screen;

	/* file handle */
	SDL_RWops *mve = NULL;

	/* data structures to handle file reads */
	chunk ch;
	opcode op;

	/* a counting var */
	Uint16 i = 0;

#ifndef NOAUDIO
	/* initialize audio buffers */
	audio_buffer.data = NULL;
	audio_buffer.length = 0;
	audio_data_read.data = NULL;
	audio_data_read.length = 0;
	temp_audio_buffer.data = NULL;
	temp_audio_buffer.length = 0;
#endif

	/******************************************/
	/* validate MVE, init SDL, and begin read */
	/******************************************/
	
	/* initialize file access structure */
	mve = SDL_RWFromFile(filename, "rb");

	/* confirm MVE file open */
	if(!mve) 
		return UNABLE_TO_OPEN_FILE;

	/* read the first 26 bytes */
	if(SDL_RWread(mve, fileTestString, sizeof(char), idlen) < 0) 
		return FILE_TOO_SHORT;

	/* validate MVE idstring */
	if(SDL_memcmp(fileTestString, idstring, idlen) != 0) 
		return NOT_MVE_FILE;
	
	/* open first chunk */
	if(!SDL_RWread(mve, &ch, sizeof(ch), 1)) 
		return CHUNK_READ_FAILED;

	/* read opcode (initial read) */
	if(!SDL_RWread(mve, &op, sizeof(op), 1)) 
		return OPCODE_READ_FAILED;

	/* are opcode parameters within known bounds? */
	if(op.type > 0x15 || op.version > 3) 
		return MVE_CORRUPT;

	/* initialize SDL */
#ifndef NOAUDIO
	if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_TIMER) < 0)
#else
	if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_TIMER) < 0)
#endif
	{
		fprintf(stderr, "Could not initialize SDL: %s", SDL_GetError());
		return SDL_INIT_FAILURE;
	}
	atexit(SDL_Quit);
	
	/*************/
	/* main loop */
	/*************/
	while(!QUITTING)
	{
		/* poll for input from user */
		MVEPlayerEventHandler();

		if(PAUSED || QUITTING) 
			continue;

		if(TIMER_INIT)
		{
			start_time = (float)SDL_GetTicks();
			TIMER_INIT = 0;
		}

		/********************/
		/* opcode handling */
		/********************/
    	switch(op.type)
		{
		case STOP_PLAYBACK:

			QUITTING = !QUITTING;

			break;

		case FETCH_NEXT_CHUNK:

			if(!SDL_RWread(mve, &ch, sizeof(ch), 1)) 
				return CHUNK_READ_FAILED;

			break;

		case CREATE_TIMER:

			/* get timer rate (uint32) */
			timer_rate = SDL_ReadLE32(mve);

			/* get timer subdivision (uint16) */
			timer_subdivision = SDL_ReadLE16(mve);

			ms_per_frame = (float)(timer_rate * timer_subdivision / 1000.0);

			TIMER_CREATED = 1;
			TIMER_INIT = 1;

			break;
#ifndef NOAUDIO
		case INIT_AUDIO_BUFFERS:

			/* init sdl audio settings */
			desired = malloc(sizeof(SDL_AudioSpec));

			/* strip the unknown word out */
			SDL_ReadLE16(mve);

			/* get the file audio flags */
			file_audio_flags = SDL_ReadLE16(mve);

			/* get the sample rate (frequency) */
			desired->freq = SDL_ReadLE16(mve);

			/* fill the known audiospec values */
			desired->samples = 4096;
			desired->callback = MVEPlayerAudioCB;
			desired->userdata = &audio_buffer;

			/* fill values gotten from the file audio flags */
			desired->channels = (file_audio_flags & 0x01 ? 2 : 1);
			desired->format = (file_audio_flags & 0x02 ? AUDIO_S16 : AUDIO_U8);
			
			if(desired->channels == 2)
				AUDIO_STEREO = 1;

			if(desired->format == AUDIO_S16)
				SAMPLESIZE16 = 1;

			/* initialize the audio mode */
			if(SDL_OpenAudio(desired, NULL) < 0)
			{
				fprintf(stderr, "Couldn't open audio: %s", SDL_GetError());
				return COULD_NOT_OPEN_AUDIO;
			}				

			/* set the compressed flag if necessary */
			if(op.version == 1)
				AUDIO_COMPRESSED = (file_audio_flags & 0x04 ? 1 : 0);
			
			/* we're going to abort if is compressed and (not stereo or not AUDIO_S16) because we don't know what to do */
			/* besides, we've never seen such an MVE before... */
			if(AUDIO_COMPRESSED && (!AUDIO_STEREO || !SAMPLESIZE16))
				return CANNOT_PROCESS_AUDIO;

			/* get rid of rest of opcode data */
			if(op.version == 1)
				SDL_ReadLE32(mve);
			else
				SDL_ReadLE16(mve);

			break;

		case START_AUDIO:

			AUDIO_PLAYING = 1;
			if(SDL_GetAudioStatus() != SDL_AUDIO_PLAYING)
			{
				SDL_PauseAudio(0);
			}
			break;
#endif
		case INIT_VIDEO_BUFFERS:

			/* get the requested dimensions 
			note: this is in 8x8 pixel chunks so multiply by 8 to get requested screen pixels*/
			width_blocks = SDL_ReadLE16(mve);
			height_blocks = SDL_ReadLE16(mve);

			/* ditch the rest of the opcode data*/
			if(op.version > 0)
			{
				SDL_ReadLE16(mve);
				if(op.version > 1)
				{
					Uint16 truecolor = SDL_ReadLE16(mve);
					if(truecolor)
					{
						fprintf(stderr, "We're sorry.  MVEPlayer can only handle 8 bpp MVE files at this time.\n");
						return SIXTEEN_BIT_MVE;
					}
				}
			}

			/* if we have already allocated a buffer */
			if(frame_buf)
				SDL_FreeSurface(frame_buf);
				
			/* initialize video buffer to the actual movie dimensions */
			frame_buf = SDL_CreateRGBSurface(SDL_SWSURFACE, width_blocks << 3, height_blocks << 3, 8, 0, 0, 0, 0);

			/* init movie screen rect for fullscreen purposes */
			movie_screen.x = (screen->w - frame_buf->w) >> 1;
			movie_screen.y = (screen->h - frame_buf->h) >> 1;

			/* erase old video backbuffer */
			if(v_backbuf)
				free(v_backbuf);

			/* allocate memory for the backbuffers sufficient for the screen pixel buffer */
			screen_buffer_size = frame_buf->h * frame_buf->w;

			v_backbuf = calloc(1, screen_buffer_size << 1);
			assert(v_backbuf);

			frame_hot = v_backbuf;
			frame_cold = v_backbuf + screen_buffer_size;

			break;

		case SEND_BUFFER_TO_DISPLAY:

			/* write the decoded data to the frame_buf */
			memcpy(frame_buf->pixels, frame_hot, screen_buffer_size);

			/* send the buffer to the screen buffer */
			if(SDL_MUSTLOCK(screen))
				SDL_LockSurface(screen);
#ifdef FULLSCREEN
			SDL_SoftStretch(frame_buf, NULL, screen, NULL);
#else
			SDL_BlitSurface(frame_buf, NULL, screen, &movie_screen);
#endif

			if(SDL_MUSTLOCK(screen))
				SDL_UnlockSurface(screen);

			/* increment start time - this is now our target frame output time */
			start_time += ms_per_frame;

			/* what time is it now? */
			current_time = SDL_GetTicks();
			
			/* if there's time to next scheduled update, force delay to our target output time */
			if(current_time < start_time)
				SDL_Delay((Uint32)(start_time - current_time));

			/* update the screen */
			SDL_UpdateRect(screen,0,0,0,0);

			/* strip opcode data; don't know how to catch errors here */
			mve->seek(mve, op.length, SEEK_CUR);

			break;
#ifndef NOAUDIO
		case AUDIO_FRAME:

			/* strip the seq-index and stream-mask */
			SDL_ReadLE16(mve);
			SDL_ReadLE16(mve);

			/* lock SDL out of the audio_buffer; we'll release at the end */
			SDL_LockAudio();

			if(AUDIO_COMPRESSED)
			{
				/* send in the raw data via audio_data_read; it will be returned the same way */
				/* read in the compressed data (should be op.length - 4 [the 4 being seq-index and stream-mask]) */
				audio_data_read.length =  op.length - 4;
				audio_data_read.data = malloc(audio_data_read.length);
				assert(audio_data_read.data != NULL);

				if(SDL_RWread(mve, audio_data_read.data, audio_data_read.length, 1) < 0)
					return AUDIO_FRAME_READ_FAILURE;
				MVEPlayerDecodeAudio(&audio_data_read);
			}
			else
			{
				/* get the stream length */
				audio_data_read.length = SDL_ReadLE16(mve);

				/* allocate memory sufficient for the data read */
				audio_data_read.data = malloc(audio_data_read.length);
				assert(audio_data_read.data != NULL);

				/* get audio data */
				if(SDL_RWread(mve, audio_data_read.data, audio_data_read.length, 1) < 0)
					return AUDIO_FRAME_READ_FAILURE;
			}
			/* at this point, we have uncompressed audio in audio_data_read, along with appropriate lengths */

			/* create temp_audio_buffer the size of old audio buffer + new audio data */
			temp_audio_buffer.length = audio_data_read.length + audio_buffer.length;
			temp_audio_buffer.data = malloc(temp_audio_buffer.length);
			assert(temp_audio_buffer.data != NULL);

			/* copy old audio buffer to temp_audio_buffer */
			memcpy(temp_audio_buffer.data, audio_buffer.data, audio_buffer.length);

			/* append new data to temp_audio_buffer */
			memcpy(temp_audio_buffer.data + audio_buffer.length, audio_data_read.data, audio_data_read.length);

			/* free audio buffer */
			if(audio_buffer.data)
				free(audio_buffer.data);

			/* temp_audio_buffer has the requested data, and audio_buffer needs to have it. */
			audio_buffer.data = temp_audio_buffer.data;
			audio_buffer.length = temp_audio_buffer.length;

			/* let SDL have access to the audio_buffer */
			SDL_UnlockAudio();

			/* close off temp_audio_buffer */
			temp_audio_buffer.data = NULL;
			temp_audio_buffer.length = 0;

			/* close off audio_data_read */
			if(audio_data_read.data)
				free(audio_data_read.data);
			audio_data_read.data = NULL;
			audio_data_read.length = 0;

			break;
#endif
		case INIT_VIDEO_MODE:

			width = SDL_ReadLE16(mve);
			height = SDL_ReadLE16(mve);

			/* if we've been here before */
			if(screen)
				SDL_FreeSurface(screen);
#ifdef FULLSCREEN
			screen = SDL_SetVideoMode(640, 480, 8, SDL_SWSURFACE|SDL_FULLSCREEN);
#else
			screen = SDL_SetVideoMode(width, height, 8, SDL_SWSURFACE);
#endif
			SDL_WM_SetCaption(filename, NULL);

			/* strip unknown flag word */
			SDL_ReadLE16(mve);

			break;

		case SET_PALETTE:
			{
				Uint8 r = 0, g = 0, b = 0;
				Uint16 palette_start = 0, numPalColors = 0;

				/* get # of first entry to fill */
				palette_start = SDL_ReadLE16(mve);
				
				/* get number of entries in palette */
				numPalColors = SDL_ReadLE16(mve);

				/* fill the palette */
				for(i = 0; i < 256; i++)
				{
					if(i < palette_start)
					{
						r = g = b = 0;
					}
					else if(i >= numPalColors + palette_start)
					{
						r = g = b = 50;
					}
					else
					{
						SDL_RWread(mve, &r, sizeof(Uint8), 1);
						SDL_RWread(mve, &g, sizeof(Uint8), 1);
						SDL_RWread(mve, &b, sizeof(Uint8), 1);
						/* without this shift, it's too dark */
						r <<= 2;
						g <<= 2;
						b <<= 2;
					}
					frame_buf->format->palette->colors[i].r = r;
					frame_buf->format->palette->colors[i].g = g;
					frame_buf->format->palette->colors[i].b = b;
					frame_buf->format->palette->colors[i].unused =0;
				}
				
			/* install the palette in the screen surface */
			SDL_SetColors(screen, frame_buf->format->palette->colors, 0, 256);

			}

			break;

		case SET_DECODING_MAP:

			/* kill previous map */
			if(map)
				free(map);
			
			/* get new map */
			map = malloc(sizeof(Uint8) * op.length);
			assert(map != NULL);
			SDL_RWread(mve, map, op.length, 1);
			break;

		case VIDEO_DATA:

			/* free the previously allocated video data array */
			if(video)
				free(video);

			/* allocate enough memory for the video data array */
			video = malloc(op.length);
			assert(video != NULL);

			/* swap the frames so we draw next on the frame that was onscreen 1 frame ago */
			temp = frame_cold;
			frame_cold = frame_hot;
			frame_hot = temp;

			/* read the data */
			SDL_RWread(mve, video, op.length, 1);

			/* decode the video data and update the frame_hot pixel data */
			MVEPlayerDecodeVideo(width_blocks, height_blocks, video, map);

			break;

		default:

			/* are opcode parameters within known bounds? */
			if(op.type > 0x15 || op.version > 3) 
				return MVE_CORRUPT;

			/* strip opcode data; don't know how to catch errors here */
			mve->seek(mve, op.length, SEEK_CUR);

			break;
		}

		/************************/
		/* end opcode handling */
		/************************/
		
		/* read opcode (trailing read) */
		if(!SDL_RWread(mve, &op, sizeof(op), 1)) 
			return OPCODE_READ_FAILED;

	}
	/***********/
	/* cleanup */
	/***********/

#ifndef NOAUDIO
	/* stop audio */
	if(AUDIO_PLAYING)
	{
		SDL_PauseAudio(1);
		SDL_CloseAudio();
	}

	/* free audio buffers */
	if(audio_buffer.data)
		free(audio_buffer.data);

	if(audio_data_read.data)
		free(audio_data_read.data);

	if(temp_audio_buffer.data)
		free(temp_audio_buffer.data);

	/* free the audio_spec */
	if(desired)
		free(desired);
#endif

	/* free the decoding map */
	if(map)
		free(map);

	/* free the video data */
	if(video)
		free(video);

	/* free the video buffers */
	if(v_backbuf)
		free(v_backbuf);

	if(frame_buf)
		SDL_FreeSurface(frame_buf);

	if(screen)
		SDL_FreeSurface(screen);

	/* close the file handle */
	if(mve)
		SDL_RWclose(mve);

	/* seems to have worked. */
	return SUCCESS;
}

/******************************/
/* Audio callback entry point */
/******************************/
#ifndef NOAUDIO
void MVEPlayerAudioCB(void *userdata, Uint8 *stream, int len)
{
	Uint8 * temp;
	buffer *audio_buffer = (buffer *)userdata;

	if((Uint32)len > audio_buffer->length)
	{
		/* give SDL what we have */
		memcpy(stream, audio_buffer->data, audio_buffer->length);
		/* and now we have nothing */
		audio_buffer->length = 0;
	}
	else
	{
		/* copy len bytes to stream */
		memcpy(stream, audio_buffer->data, len);

		/* our buffer length is now whatever we started with minus len */
		audio_buffer->length -= len;

		/* now we need to store the data that wasn't read */
		temp = malloc(audio_buffer->length);
		assert(temp != NULL);

		/* copy from data address + len length bytes 
		(which should be the start of the data that wasn't copied to stream) */
		memcpy(temp, audio_buffer->data + len, audio_buffer->length);

		/* free the memory of the audio_buffer */
		if(audio_buffer->data)
			free(audio_buffer->data);

		/* assign the address of our stored data to the audio_buffer's data */
		audio_buffer->data = temp;
		
		/* close off temp */
		temp = NULL;
	}
}

/* Interplay DPCM table */
Sint16 interplay_delta_table[256] =
{
         0,      1,      2,      3,      4,      5,      6,      7,      8,      9,     10,     11,     12,     13,     14,     15,
        16,     17,     18,     19,     20,     21,     22,     23,     24,     25,     26,     27,     28,     29,     30,     31,
        32,     33,     34,     35,     36,     37,     38,     39,     40,     41,     42,     43,     47,     51,     56,     61,
        66,     72,     79,     86,     94,    102,    112,    122,    133,    145,    158,    173,    189,    206,    225,    245,
       267,    292,    318,    348,    379,    414,    452,    493,    538,    587,    640,    699,    763,    832,    908,    991,
      1081,   1180,   1288,   1405,   1534,   1673,   1826,   1993,   2175,   2373,   2590,   2826,   3084,   3365,   3672,   4008,
      4373,   4772,   5208,   5683,   6202,   6767,   7385,   8059,   8794,   9597,  10472,  11428,  12471,  13609,  14851,  16206,
     17685,  19298,  21060,  22981,  25078,  27367,  29864,  32589, -29973, -26728, -23186, -19322, -15105, -10503,  -5481,     -1,
         1,      1,   5481,  10503,  15105,  19322,  23186,  26728,  29973, -32589, -29864, -27367, -25078, -22981, -21060, -19298,
    -17685, -16206, -14851, -13609, -12471, -11428, -10472,  -9597,  -8794,  -8059,  -7385,  -6767,  -6202,  -5683,  -5208,  -4772,
     -4373,  -4008,  -3672,  -3365,  -3084,  -2826,  -2590,  -2373,  -2175,  -1993,  -1826,  -1673,  -1534,  -1405,  -1288,  -1180,
     -1081,   -991,   -908,   -832,   -763,   -699,   -640,   -587,   -538,   -493,   -452,   -414,   -379,   -348,   -318,   -292,
      -267,   -245,   -225,   -206,   -189,   -173,   -158,   -145,   -133,   -122,   -112,   -102,    -94,    -86,    -79,    -72,
       -66,    -61,    -56,    -51,    -47,    -43,    -42,    -41,    -40,    -39,    -38,    -37,    -36,    -35,    -34,    -33,
       -32,    -31,    -30,    -29,    -28,    -27,    -26,    -25,    -24,    -23,    -22,    -21,    -20,    -19,    -18,    -17,
       -16,    -15,    -14,    -13,    -12,    -11,    -10,     -9,     -8,     -7,     -6,     -5,     -4,     -3,     -2,     -1
};

/*****************************/
/* audio decoder entry point */
/*****************************/
void MVEPlayerDecodeAudio(buffer * in)
{
	/* this only works for stereo dpcm at present */

	Sint16 sample[2];
	Uint16 in_pos = 0, out_pos = 0;
	Uint8 channel = 0;
	buffer out;

	/* in->data includes initial stream-len word */
	/* uncompressed streamlen is stream-len bytes */
	out.length = LE16(in->data);
	in_pos +=2;
	out.data = malloc(out.length);
	assert(out.data != NULL);

	/* each byte in the input buffer after the first four (two words: initial L and R values)
	will be expanded to fill a word in the return buffer after decompression. 
	The initial two words will be stored in return buffer as-is. */
	sample[0] = LE16(in->data + in_pos);
	in_pos += 2;
	*(Sint16 *)(out.data + out_pos) = sample[0];
	out_pos += 2;
	sample[1] = LE16(in->data + in_pos);
	in_pos += 2;
	*(Sint16 *)(out.data + out_pos) = sample[1];
	out_pos += 2;

	while(in_pos < in->length)
	{
		sample[channel] += interplay_delta_table[*(in->data + in_pos++)];
		*(Sint16 *)(out.data + out_pos) = sample[channel];
		out_pos += 2;
		channel = !channel;
	}

	/* free the original buffer*/
	if(in->data)
		free(in->data);

	/* assign the incoming buffer the outgoing data for return */
	in->data = out.data;
	in->length = out.length;
}
#endif

/**************************************/
/* Video decoding routine entry point */
/**************************************/
void MVEPlayerDecodeVideo(Uint16 wblocks, Uint16 hblocks, Uint8 * pData, Uint8 * pMap)
{
	Uint8 encoding = 0, *map = NULL, *data = NULL, *new_frame = NULL, *current = NULL, *temp = NULL;
	Uint16 pitch = 0;
	Sint32 x, y, i, j;

	map = pMap;

	/* there are 14 undocumented bytes at head of video stream */
	data = pData + 14;

	pitch = wblocks << 3;

	/* for each row (of 8x8 pixel blocks) */
    for(y = 0; y < hblocks; y++) 
	{
		/* for each column (of 8x8 pixel blocks) */
		for (x = 0; x < wblocks; x++) 
		{
			/* assign the frame buffer pointers to the addresses of the screen 
			buffers so we can do pointer arithmetic on them such as incrementing. 
			current is the frame currently onscreen; new_frame is the frame under construction. */

			new_frame = frame_hot;
			current = frame_cold;

			/* increment pixel pointer past the rendered area on the screen
			y = number of rows, wblocks = number of blocks per row, 8 * 8 = pixel area of the block; 
			y * wblocks * 8 * 8 = all the complete rows of 8x8 pixel blocks rendered.
			x * 8 = upper left pixel of the 8x8 block in this new row to be rendered at this time. 
			I felt like replacing the multiplications throughout this function with shift operations. */

			new_frame += (y * wblocks << 6) + (x << 3);
			current += (y * wblocks << 6) + (x << 3);
				
			/* get the decoding map info for this block; low 4 bits if x is even, high 4 bits if x is odd */
			encoding = (*map >> ((x & 1) << 2)) & 0x0f;

			/* if x is odd, increment the decoding map */
			map += (x & 1);

			/* process the block */
			switch(encoding)
			{
			case 0x0:

				for(j = 0; j < 8; j++)
				{
					memcpy(new_frame + j * pitch, current + j * pitch, 8);
				}

				break;

			case 0x1:
				
				break;

			case 0x2:

				temp = new_frame;
				if(*data < 56)
				{
					i = 8 + (*data % 7);
					j =      *data / 7;
				}
				else
				{
					i = -14 + ((*data - 56) % 29);
					j =   8 + ((*data - 56) / 29);
				}
				data++;

				temp += j * pitch + i;

				for(j = 0; j < 8; j++)
				{
					memcpy(new_frame + j * pitch, temp + j * pitch, 8);
				}

				break;

			case 0x3:

				temp = new_frame;
				if(*data < 56)
				{
					i = -(8 + (*data % 7));
					j = -     (*data / 7);
				}
				else
				{
					i = -(-14 + ((*data - 56) % 29));
					j = -(  8 + ((*data - 56) / 29));
				}
				data++;

				temp += j * pitch + i;

				for(j = 0; j < 8; j++)
				{
					memcpy(new_frame + j * pitch, temp + j * pitch, 8);
				}

				break;

			case 0x4:

				i = -8 + (*data & 0xF);
				j = -8 + (*data >> 4 );
				data++;

				current += j * pitch + i;

				for(j = 0; j < 8; j++)
				{
					memcpy(new_frame + j * pitch, current + j * pitch, 8);
				}

				break;

			case 0x5:

				current += (Sint8)data[1] * pitch + (Sint8)data[0];
				data += 2;

				for(j = 0; j < 8; j++)
				{
					memcpy(new_frame + j * pitch, current + j * pitch, 8);
				}

				break;

			case 0x6:

				x += 2;
				if(x >= wblocks)
				{
					y++;
					x %= wblocks;
				}
				break;

			case 0x7:
				{
					Uint8 p[2], b[8], mask;

					p[0] = *data++;
					p[1] = *data++;

					if(p[0] <= p[1])
					{
						for(i = 0; i < 8; i++)
						{
							b[i] = *data++;
						}
						for(j = 0; j < 8; j++)
						{
							for(i = 0, mask = 1; i < 8; i++, mask <<= 1)
							{
								new_frame[i] = p[!!(b[j] & mask)];
							}
							new_frame += pitch;
						}
					}
					else
					{
						b[0] = *data++;
						b[1] = *data++;

						for(j = 0; j < 4; j++)
						{
							if(!(j % 2))
								mask = 0x01;
							for(i = 0; i < 4; i++, mask <<= 1)
							{
								new_frame[i << 1] = 
									new_frame[(i << 1) + 1] =
									new_frame[(i << 1) + pitch] = 
									new_frame[(i << 1) + 1 + pitch] = p[!!(b[j >> 1] & mask)];
							}
							new_frame += pitch << 1;
						}
					}
				} /* end case block */

				break;

			case 0x8:
				{
					Uint8 p[8], b[8], mask, k;
					if(data[0] <= data[1])
					{
						for(j = 0; j < 4; j++)
						{
							for(i = 0; i < 2; i++)
							{
								p[i + (j << 1)] = *data++;
							}
							for(i = 0; i < 2; i++)
							{
								b[i + (j << 1)] = *data++;
							}
						}

						for(k = 0; k < 4; k++)
						{
							if(k == 2)
							{
								new_frame -= pitch << 3;
								new_frame += 4;
							}
							for(j = 0; j < 4; j++)
							{
								if(!(j & 1))
									mask = 1;
								for(i = 0; i < 4; i++)
								{
									new_frame[i] = p[!!(b[(j >> 1) + (k << 1)] & mask) + (k << 1)];
									mask <<= 1;
								} /* for i */
								new_frame += pitch;
							} /* for j */
						} /* for k */
					} /* if p0 <= p1 */
					else
					{
						for(j = 0; j < 2; j++)
						{
							for(i = 0; i < 2; i++)
							{
								p[(j << 1) + i] = *data++;
							}
							for(i = 0; i < 4; i++)
							{
								b[(j << 2) + i] = *data++;
							}
						}
						if(p[2] > p[3])
						{
							/* horizontally split block */
							for(j = 0; j < 8; j++)
							{
								for(i = 0, mask = 1; i < 8; i++, mask <<= 1)
								{
									new_frame[i] = p[!!(b[j] & mask) + (j > 3 ? 2 : 0)];
								}
								new_frame += pitch;
							}
						}
						else
						{
							/* vertically split block */
							for(k = 0; k < 2; k++)
							{
								if(k == 1)
								{
									new_frame -= pitch << 3;
									new_frame += 4;
								}
								for(j = 0; j < 4; j++)
								{
									mask = 1;
									for(i = 0; i < 8; i++)
									{
										new_frame[i % 4] = p[!!(b[j + (k << 2)] & mask) + (k << 1)];
										mask <<= 1;
										if(i == 3)
											new_frame += pitch;
									}
									new_frame += pitch;
								}
							}
						}
					} /* end if/else block */					
				} /* end case block */

				break;

			case 0x9:
				{
					Uint8 p[4], b[16];
					for(i = 0; i < 4; i++)
					{
						p[i] = *data++;
					}
					if(p[0] <= p[1] && p[2] <= p[3])
					{
						Uint16 mask;
						for(i = 0; i < 16; i++)
						{
							b[i] = *data++;
						}
						for(j = 0; j < 8; j++)
						{
							for(i = 0, mask = 3; i < 8; i++, mask <<= 2)
							{
								new_frame[i] = p[(mask & (b[(j << 1) + 1] << 8 | b[j << 1])) >> (i << 1)];
							}
							new_frame += pitch;
						}
					}
					if(p[0] <= p[1] && p[2] > p[3])
					{
						Uint8 mask;
						for(i = 0; i < 4; i++)
						{
							b[i] = *data++;
						}
						for(j = 0; j < 8; j++)
						{
							mask = 3;
							for(i = 0; i < 8; i++)
							{
								new_frame[i] = p[(b[j >> 1] & mask) >> ((i >> 1) << 1)];
								if(i & 1)
									mask <<= 2;
							}
							new_frame += pitch;
						}
					}
					if(p[0] > p[1] && p[2] <= p[3])
					{
						Uint8 mask;
						for(i = 0; i < 8; i++)
						{
							b[i] = *data++;
						}
						for(j = 0; j < 8; j++)
						{
							mask = 3;
							for(i = 0; i < 8; i++)
							{
								new_frame[i] = p[(b[j] & mask) >> ((i >> 1) << 1)];
								if(i & 1)
									mask <<= 2;
							}
							new_frame += pitch;
						}
					}
					if(p[0] > p[1] && p[2] > p[3])
					{
						Uint16 mask;
						for(i = 0; i < 8; i++)
						{
							b[i] = *data++;
						}
						for(j = 0; j < 4; j++)
						{
							mask = 3;
							for(i = 0; i < 8; i++)
							{
								new_frame[i] =
									new_frame[i + pitch] = p[((b[(j << 1) + 1] << 8 | b[j << 1]) & mask) >> (i << 1)];
								mask <<= 2;
							}
							new_frame += pitch << 1;
						}
					}
				}

				break;

			case 0xA:
				{
					Uint8 p[16], b[16], mask, k;
					if(data[0] <= data[1])
					{
						for(i = 0; i < 4; i++)
						{
							for(j = 0; j < 4; j++)
							{
								p[(i << 2) + j] = *data++;
							}
							for(j = 0; j < 4; j++)
							{
								b[(i << 2) + j] = *data++;
							}
						}
						for(k = 0; k < 4; k++)
						{
							for(j = 0; j < 4; j++)
							{
								for(i = 0, mask = 3; i < 4; i++, mask <<= 2)
								{
									new_frame[i] = p[((b[(k << 2) + j] & mask) >> (i << 1)) + (k << 2)];
								}
								new_frame += pitch;
							}
							if(k == 1)
							{
								new_frame -= pitch << 3;
								new_frame += 4;
							}
						}
					}
					else
					{
						for(i = 0; i < 2; i++)
						{
							for(j = 0; j < 4; j++)
							{
								p[(i << 2) + j] = *data++;
							}
							for(j = 0; j < 8; j++)
							{
								b[(i << 3) + j] = *data++;
							}
						}
						if(p[4] <= p[5])
						{
							for(k = 0; k < 2; k++)
							{
								for(j = 0; j < 8; j++)
								{
									for(i = 0, mask = 3; i < 4; i++, mask <<= 2)
									{
										new_frame[i] = p[((b[j + (k << 3)] & mask) >> (i << 1)) + (k << 2)];
									}
									new_frame += pitch;
								}
								if(k == 0)
								{
									new_frame -= pitch << 3;
									new_frame += 4;
								}
							}
						}
						else
						{
							for(k = 0; k < 2; k++)
							{
								for(j = 0; j < 4; j++)
								{
									for(i = 0, mask = 3; i < 8; i++, mask <<= 2)
									{
										if(i == 4)
											mask = 3;
										new_frame[i] = p[((b[(j << 1) + (i > 3 ? 1 : 0) + (k << 3)] & mask) 
											>> ((i % 4) << 1)) + (k << 2)];
									}
									new_frame += pitch;
								}
							}
						}
					}
				}

				break;

			case 0xB:

				for(j = 0; j < 8; j++)
				{
					for(i = 0; i < 8; i++)
					{
						new_frame[i] = *data++;
					}
					new_frame += pitch;
				}

				break;

			case 0xC:
				{
					Uint8 p[16];
					for(i = 0; i < 16; i++)
					{
						p[i] = *data++;
					}
					for(j = 0; j < 8; j++)
					{
						for(i = 0; i < 8; i++)
						{
							new_frame[i] = p[(i >> 1) + ((j >> 1) << 2)];
						}
						new_frame += pitch;
					}
				}

				break;

			case 0xD:
				{
					Uint8 p[4];
					for(i = 0; i < 4; i++)
					{
						p[i] = *data++;
					}
					for(j = 0; j < 8; j++)
					{
						for(i = 0; i < 8; i++)
						{
							new_frame[i] = p[((j > 3 ? 1 : 0) << 1) + (i > 3 ? 1 : 0)];
						}
						new_frame += pitch;
					}
				}

				break;

			case 0xE:

				for(j = 0; j < 8; j++) 
				{
					memset(new_frame + j * pitch, *data, 8);
				}
				data++;

				break;

			case 0xF:

				for(j = 0; j < 8; j++)
				{
					for(i = 0; i < 8; i++)
						new_frame[i] = data[((i + j) & 1)];
					new_frame += pitch;
				}
				data += 2;

				break;
			} /* end switch(encoding) */
		} /* end for(x) */
	} /* end for(y) */
}

/*************************************/
/* event polling routine entry point */
/*************************************/
void MVEPlayerEventHandler()
{
	/* event vars for determining pause, quit, or speed commands from user; declared static 
	so they retain their values between calls to MVEPlayerEventHandler (else nothing happens) */

	static Uint8 keyin = 0;
	static SDL_Event event;

	while(SDL_PollEvent(&event))
	{
		switch(event.type)
		{
		case SDL_KEYDOWN:
			keyin = event.key.keysym.sym;
			break;
		case SDL_KEYUP:
			if(event.key.keysym.sym == keyin)
			{
				switch(event.key.keysym.sym)
				{
#ifdef NOAUDIO
				case SDLK_s:
					printf("slower\n");
					TIMER_INIT = 1;
					ms_per_frame *= 2;
					break;
				case SDLK_f:
					printf("faster\n");
					TIMER_INIT = 1;
					ms_per_frame /= 2;
					break;
#endif
				case SDLK_q:
				case SDLK_ESCAPE:
				case SDLK_RETURN:
					QUITTING = !QUITTING;
					break;
				case SDLK_SPACE:
					PAUSED = !PAUSED;
					if(PAUSED)
					{
						if(AUDIO_PLAYING) 
							SDL_PauseAudio(1);
						if(TIMER_CREATED)
							TIMER_INIT = 1;
					}
					else if(AUDIO_PLAYING) 
						SDL_PauseAudio(0);						
					break;
				/* ToggleFullScreen does not work on my system so I cannot test it */
				/*case SDLK_f:
					SDL_WM_ToggleFullScreen(screen);
					break;*/
				default:
					break;
				}
				keyin = 0;
			}
			break;
		case SDL_QUIT:
			QUITTING = !QUITTING;
			break;
		default:
			break;
		}
	}
}