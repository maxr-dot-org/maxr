#include <string>
#include "netMessage.h"
#include "events.h"

cNetMessage::cNetMessage( char* c)
{
	iType = SDL_SwapLE16( *((Sint16*) c) );
	iLength = SDL_SwapLE16( *((Sint16*) (c + 2)) );
	iPlayerNr = c[4];

	data = (char*) malloc( iLength );
	memcpy( data, c, iLength );
}

cNetMessage::cNetMessage(int iType)
{
	this->iType = iType;
	data = (char*) malloc( 5 * sizeof(char) );  // 0 - 1: reserviert für message typ
												// 2 - 3: reserviert für length
												// 4:	  reserviert für Playernummer
	iLength = 5;
}

cNetMessage::~cNetMessage()
{
	free ( data );
}

char* cNetMessage::serialize()
{
	//write iLenght to byte array
	*((Sint16*) data) = SDL_SwapLE16( (Sint16)iType);
	//write iType to byte array
	*((Sint16*) (data + 2)) = SDL_SwapLE16( (Sint16)iLength);
	//write iPlayernr to byte array
	data[4] = (char) iPlayerNr;

	return data;
}

SDL_Event* cNetMessage::getGameEvent()
{
	SDL_Event* event = new SDL_Event;
	event->type = GAME_EVENT;
	event->user.data1 = malloc( iLength );
	memcpy( event->user.data1, serialize(), iLength );

	event->user.data2 = NULL;

	return event;
}

//Todo: check PACKAGE_LENGHT

void cNetMessage::pushChar( char c)
{
	data = (char*) realloc( data, iLength + 1 );
	data[iLength] = c;
	iLength ++;
}

char cNetMessage::popChar()
{
	iLength--;
	return data[iLength];
}

void cNetMessage::pushInt16( Sint16 i )
{
	data = (char*) realloc( data, iLength + 2 );
	*((Sint16*) (data + iLength)) = SDL_SwapLE16(i);
	iLength += 2;
}

Sint16 cNetMessage::popInt16()
{
	iLength -= 2;
	return SDL_SwapLE16( *((Sint16*) (data + iLength)) );
}

void cNetMessage::pushInt32( Sint32 i )
{
	data = (char*) realloc( data, iLength + 4 );
	*((Sint32*) (data + iLength)) = SDL_Swap32( i );
	iLength += 4;
}

Sint32 cNetMessage::popInt32()
{
	iLength -= 4;
	return SDL_SwapLE32( *((Sint32*) (data + iLength)) );
}

void cNetMessage::pushString( string &s )
{
	int stringLength = (int) s.length() + 2;

	//Todo: netlog warning about stringlength

	data = (char*) realloc( data, iLength + stringLength );
	
	//first write a '\0'
	//in the netMessage both begin and end of the string are marked with a '\0'
	//so we are able to pop it correctly
	data[iLength] = 0;
	iLength++;
	stringLength--;

	const char* c = s.c_str();
	memcpy( data + iLength, c, stringLength );

	iLength += stringLength;
}

string cNetMessage::popString()
{
	if ( data[iLength - 1] != '\0' )
	{
		//Todo: netlog error
		return NULL;
	}

	//find begin of string
	iLength -= 2;
	while ( data[iLength] != '\0' ) iLength--;
	//(data + iLenght) points now to the leading '\0'

	return string( data + iLength + 1 );
}
