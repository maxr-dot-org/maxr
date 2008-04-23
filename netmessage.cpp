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

#include <string>
#include "netmessage.h"
#include "events.h"
#include "network.h"
#include "clientevents.h"
#include "serverevents.h"

cNetMessage::cNetMessage( char* c)
{
	iType = SDL_SwapLE16( *((Sint16*) c) );
	iLength = SDL_SwapLE16( *((Sint16*) (c + 2)) );
	iPlayerNr = c[4];

	if ( iLength > PACKAGE_LENGHT || iLength < 5 )
	{
		//invalid netMessage
		iLength = 5;
		iType = -1;
	}

	data = (char*) malloc( PACKAGE_LENGHT );
	memcpy( data, c, iLength );
}

cNetMessage::cNetMessage(int iType)
{
	this->iType = iType;
	data = (char*) malloc( PACKAGE_LENGHT );	// 0 - 1: reserviert für message typ
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
	//write iType to byte array
	*((Sint16*) data) = SDL_SwapLE16( (Sint16)iType);
	//write iLenght to byte array
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


void cNetMessage::pushChar( char c)
{
	data[iLength] = c;
	iLength ++;

	if ( iLength > PACKAGE_LENGHT ) cLog::write( "Warning: size of netMessage exceeds PACKAGE_LENGHT", cLog::eLOG_TYPE_NETWORK );
}

char cNetMessage::popChar()
{
	if ( iLength <= 5 ) 
	{
		cLog::write( "Warning: pop from empty netMessage", cLog::eLOG_TYPE_NETWORK );
		return 0;
	}
	iLength--;
	return data[iLength];
}

void cNetMessage::pushInt16( Sint16 i )
{
	*((Sint16*) (data + iLength)) = SDL_SwapLE16(i);
	iLength += 2;

	if ( iLength > PACKAGE_LENGHT ) cLog::write( "Warning: size of netMessage exceeds PACKAGE_LENGHT", cLog::eLOG_TYPE_NETWORK );
}

Sint16 cNetMessage::popInt16()
{
	if ( iLength <= 6 ) 
	{
		cLog::write( "Warning: pop from empty netMessage", cLog::eLOG_TYPE_NETWORK );
		return 0;
	}
	iLength -= 2;
	return SDL_SwapLE16( *((Sint16*) (data + iLength)) );
}

void cNetMessage::pushInt32( Sint32 i )
{
	*((Sint32*) (data + iLength)) = SDL_SwapLE32( i );
	iLength += 4;

	if ( iLength > PACKAGE_LENGHT ) cLog::write( "Warning: size of netMessage exceeds PACKAGE_LENGHT", cLog::eLOG_TYPE_NETWORK );
}

Sint32 cNetMessage::popInt32()
{
	if ( iLength <= 8 ) 
	{
		cLog::write( "Warning: pop from empty netMessage", cLog::eLOG_TYPE_NETWORK );
		return 0;
	}
	iLength -= 4;
	return SDL_SwapLE32( *((Sint32*) (data + iLength)) );
}

void cNetMessage::pushString( string s )
{
	int stringLength = (int) s.length() + 2;
	
	//first write a '\0'
	//in the netMessage both begin and end of the string are marked with a '\0'
	//so we are able to pop it correctly
	data[iLength] = 0;
	iLength++;
	stringLength--;

	const char* c = s.c_str();
	memcpy( data + iLength, c, stringLength );

	iLength += stringLength;

	if ( iLength > PACKAGE_LENGHT ) cLog::write( "Warning: size of netMessage exceeds PACKAGE_LENGHT", cLog::eLOG_TYPE_NETWORK );
}

string cNetMessage::popString()
{
	if ( data[iLength - 1] != '\0' )
	{
		cLog::write( "Warning: invalid popString() from netMessage", cLog::eLOG_TYPE_NETWORK );
		return string("");
	}

	//find begin of string
	iLength -= 2;
	while ( data[iLength] != '\0' )
	{
		if ( iLength <= 5 ) 
		{
			cLog::write( "Warning: pop string from netMessage failed, begin of string not found", cLog::eLOG_TYPE_NETWORK );
			return string("");
		}
		iLength--;
	}
	//(data + iLenght) points now to the leading '\0'

	return string( data + iLength + 1 );
}

void cNetMessage::pushBool( bool b )
{
	
	data[iLength] = b;
	
	iLength++;

	if ( iLength > PACKAGE_LENGHT ) cLog::write( "Warning: size of netMessage exceeds PACKAGE_LENGHT", cLog::eLOG_TYPE_NETWORK );
}

bool cNetMessage::popBool()
{
	if ( iLength <= 5 ) 
	{
		cLog::write( "Warning: pop from empty netMessage", cLog::eLOG_TYPE_NETWORK );
		return 0;
	}

	iLength--;

	if ( data[iLength] != 0 ) return 1;
	else return 0;

}

void cNetMessage::pushFloat( float f )
{

    float fnorm;
    int shift;
    Uint32 sign, exp, significand;
    unsigned significandbits = BITS - EXPBITS - 1; // -1 for sign bit

    if (f == 0.0)
	{
		pushInt32( 0 );
		return;
	}

    // check sign and begin normalization
    if (f < 0)
	{ 
		sign = 1;
		fnorm = -f;
	}
    else
	{ 
		sign = 0;
		fnorm = f;
	}

    // get the normalized form of f and track the exponent
    shift = 0;
    while (fnorm >= 2.0)
	{
		fnorm /= 2.0;
		shift++;
	}
    while (fnorm < 1.0)
	{
		fnorm *= 2.0;
		shift--;
	}
    fnorm -= 1.0;

    // calculate the binary form (non-float) of the significand data
    significand = fnorm * ((1LL<<significandbits) + 0.5f);

    // get the biased exponent
    exp = shift + ((1<<(EXPBITS-1)) - 1); // shift + bias

	//store result in netMessage
    pushInt32( (sign<<(BITS-1)) | (exp<<(BITS-EXPBITS-1)) | significand );

}

float cNetMessage::popFloat()
{
	float result;
    long long shift;
    unsigned bias;
    unsigned significandbits = BITS - EXPBITS - 1; // -1 for sign bit

	//get data from netMessage
	Uint32 i = popInt32();

    if (i == 0) return 0.0;

    // pull the significand
    result = ( i & ( (1LL<<significandbits) - 1)); // mask
    result /= ( 1LL<<significandbits ); // convert back to float
    result += 1.0f; // add the one back on

    // deal with the exponent
    bias = (1<<(EXPBITS-1)) - 1;
    shift = ((i>>significandbits)&((1LL<<EXPBITS)-1)) - bias;
    while (shift > 0)
	{
		result *= 2.0;
		shift--;
	}
    while (shift < 0)
	{
		result /= 2.0;
		shift++;
	}

    // sign it
    result *= (i>>(BITS-1))&1 ? -1.0: 1.0;

    return result;
}

string cNetMessage::getTypeAsString()
{
	//find the string representation of the message type for
	//should be updated when implementing a new message type
	switch (iType)
	{
	case GAME_EV_ADD_BUILDING:
		return string("GAME_EV_ADD_BUILDING");
	case GAME_EV_ADD_VEHICLE:
		return string("GAME_EV_ADD_VEHICLE");
	case GAME_EV_DEL_BUILDING:
		return string("GAME_EV_DEL_BUILDING");
	case GAME_EV_DEL_VEHICLE:
		return string("GAME_EV_DEL_VEHICLE");
	case GAME_EV_ADD_ENEM_BUILDING:
		return string("GAME_EV_ADD_ENEM_BUILDING");
	case GAME_EV_ADD_ENEM_VEHICLE:
		return string("GAME_EV_ADD_ENEM_VEHICLE");
	case GAME_EV_CHAT_SERVER:
		return string("GAME_EV_CHAT_SERVER");
	case GAME_EV_LOST_CONNECTION:
		return string("GAME_EV_LOST_CONNECTION");
	case GAME_EV_CHAT_CLIENT:
		return string("GAME_EV_CHAT_CLIENT");
	case GAME_EV_WANT_TO_END_TURN:
		return string("GAME_EV_WANT_TO_END_TURN");
	case GAME_EV_MAKE_TURNEND:
		return string("GAME_EV_MAKE_TURNEND");
	case GAME_EV_FINISHED_TURN:
		return string("GAME_EV_FINISHED_TURN");
	case GAME_EV_UNIT_DATA:
		return string("GAME_EV_UNIT_DATA");
	case GAME_EV_WANT_START_WORK:
		return("GAME_EV_WANT_START_WORK");
	case GAME_EV_WANT_STOP_WORK:
		return("GAME_EV_WANT_STOP_WORK");
	case GAME_EV_DO_START_WORK:
		return("GAME_EV_DO_START_WORK");
	case GAME_EV_DO_STOP_WORK:
		return("GAME_EV_DO_STOP_WORK");

	default:
		return iToStr( iType );
	}
}


string cNetMessage::getHexDump()
{
	string dump = "0x";

	serialize(); //updates the internal data

	for (int i = 0; i < iLength; i++)
	{
		unsigned char byte = data[i];
		unsigned char temp = byte/16;
		if ( temp < 10 ) dump += ('0' + temp);
		else dump += ( 'A' + temp - 10 );

		temp = byte%16;
		if ( temp < 10 ) dump += ('0' + temp);
		else dump += ( 'A' + temp - 10 );
	}

	return dump;
}
