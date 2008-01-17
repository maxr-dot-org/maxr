#include <SDL.h>
#include "wave.h"



int saveWAV (string dst, WaveFile& waveFile)
{
	int was_error = 0;
	Chunk chunk;
	Uint16 channel;
	Uint8 *buf_pos;
	Uint16 bytespersample;

	SDL_AudioSpec* spec = &waveFile.spec;
	Uint32 audio_len = waveFile.length;
	Uint8* audio_buf = waveFile.buffer;
	/* FMT chunk */
	WaveFMT *format = NULL;

	//open destiantion file
	SDL_RWops* file = SDL_RWFromFile( dst.c_str(), "wb") ;
	if( file == NULL )
	{
		return 0;
	}
		
	/* Write the magic header */
	if ( SDL_WriteLE32(file, RIFF) == -1 )
	{
		return 0;
	}
	SDL_WriteLE32(file, HEADER_SIZE + audio_len);
	SDL_WriteLE32(file, WAVE);

	/* Write the audio data format chunk */
	chunk.magic = FMT;
	chunk.length = FMT_DATA_SIZE;
	chunk.data = (Uint8 *)malloc(chunk.length);
	if ( chunk.data == NULL )
	{
		return 0;
	}
	format = (WaveFMT *)chunk.data;
	format->encoding = PCM_CODE;
	format->channels = (Uint16)spec->channels;
	format->frequency = (Uint32)spec->freq;
	switch (spec->format) {
		case AUDIO_U8: case AUDIO_S8:
			format->bitspersample = 8;
			break;
		default:
			format->bitspersample = 16;
			break;
	};
	bytespersample = format->bitspersample >> 3;
	format->byterate = format->frequency * format->channels *
			bytespersample;
	format->blockalign = bytespersample * format->channels;
	SDL_WriteLE32(file, chunk.magic);
	SDL_WriteLE32(file, chunk.length);
	SDL_WriteLE16(file, format->encoding);
	SDL_WriteLE16(file, format->channels);
	SDL_WriteLE32(file, format->frequency);
	SDL_WriteLE32(file, format->byterate);
	SDL_WriteLE16(file, format->blockalign);
	SDL_WriteLE16(file, format->bitspersample);

	/* Write the audio data chunk */
	SDL_WriteLE32(file, DATA);
	SDL_WriteLE32(file, audio_len);

	buf_pos = (Uint8*) audio_buf;
	while (buf_pos < audio_buf + audio_len) {
		for (channel=0; channel < format->channels; ++channel) {
			switch (spec->format) {
				case AUDIO_U8:
				case AUDIO_S8:
					SDL_SetError("8-bit WAV writing unsupported");
					was_error = 1;
					goto done;
					break;
				case AUDIO_U16LSB:
					SDL_WriteLE16(file, SDL_SwapLE16(
						*(Sint16*)buf_pos - TO_SINT16));
					break;
				case AUDIO_U16MSB:
					SDL_WriteLE16(file, SDL_SwapBE16(
						*(Sint16*)buf_pos - TO_SINT16));
					break;
				case AUDIO_S16LSB:
					SDL_WriteLE16(file, SDL_SwapLE16(
						*(Sint16*)buf_pos));
					break;
				case AUDIO_S16MSB:
					SDL_WriteLE16(file, SDL_SwapBE16(
						*(Sint16*)buf_pos));
					break;
			}
			buf_pos += bytespersample;
		}
	}

done:
	if ( format != NULL ) {
		free(format);
	}

	SDL_RWclose( file );

	if ( was_error )
	{
		return 0;
	}

	return 1;
}


int copyPartOfWAV( string src, string dst, float start_ms, float end_ms)
{
	WaveFile waveFile;

	//load file from disk
	if( SDL_LoadWAV(src.c_str(), &waveFile.spec, &waveFile.buffer, &waveFile.length) == NULL)
	{
		return 0;
	}

	//claculate absolut start/end positions
	Uint8 bytesPerSample = (waveFile.spec.format & 0x00FF) / 8;
	Uint32 start, end;
	start = (int) waveFile.spec.freq * waveFile.spec.channels * bytesPerSample * start_ms / 1000;
	if ( end_ms == 0 )
	{
		end = waveFile.length;
	}
	else
	{
		end = (int) waveFile.spec.freq * waveFile.spec.channels * bytesPerSample * end_ms / 1000;
	}

	//check start and end positions
	if( start > end || end > waveFile.length )
	{
		return 0;
	}

	//resize the wave buffer to the desired part
	waveFile.length = end - start;
	Uint8* new_buffer = (Uint8*) malloc( waveFile.length );
	if ( new_buffer == NULL )
	{
		return 0;
	}
	memcpy( new_buffer, waveFile.buffer + start, waveFile.length );
	SDL_FreeWAV( waveFile.buffer );
	waveFile.buffer = new_buffer;
	
	//save resized wave
	if( saveWAV( dst, waveFile ) == 0 )
	{
		free ( waveFile.buffer );
		return 0;
	}

	free ( waveFile.buffer );
	return 1;
}
