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
#include "menuevents.h"

cNetMessage::cNetMessage( char* c)
{
	if ( c[0] != START_CHAR ) Log.write ( "NetMessage has wrong start character", LOG_TYPE_NET_ERROR );

	iLength = SDL_SwapLE16( ((Sint16*)(c+1))[0] );
	iType = SDL_SwapLE16( ((Sint16*)(c+1))[1] );
	iPlayerNr = c[5];

	data = new char[iLength];
	data[0] = START_CHAR;
	memcpy ( data, c, iLength );
}

cNetMessage::cNetMessage(int iType)
{
	this->iType = iType;
	data = new char[PACKAGE_LENGTH];	// 0:	  reserved for startchar
												// 1 - 2: reserved for length
												// 3 - 4: reserved for message type
												// 5:	  reserved for playernumber
	data[0] = START_CHAR;
	iLength = 6;
}

cNetMessage::~cNetMessage()
{
	delete [] data;
}

char* cNetMessage::serialize()
{
	//set start character
	data[0] = START_CHAR;
	//write iLenght to byte array
	*((Sint16*) (data+1)) = SDL_SwapLE16( (Sint16)iLength);
	//write iType to byte array
	*((Sint16*) (data+3)) = SDL_SwapLE16( (Sint16)iType);
	//write iPlayernr to byte array
	data[5] = (char) iPlayerNr;

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
	if ( iLength > PACKAGE_LENGTH - 1 )
	{
		Log.write( "Size of netMessage exceeds MAX_MESSAGE_LENGTH", cLog::eLOG_TYPE_NET_ERROR );
		return;
	}

	data[iLength] = c;
	iLength ++;
}

char cNetMessage::popChar()
{
	if ( iLength <= 6 )
	{
		Log.write( "Pop from empty netMessage", cLog::eLOG_TYPE_NET_ERROR );
		return 0;
	}
	iLength--;
	return data[iLength];
}

void cNetMessage::pushInt16( Sint16 i )
{
	if ( iLength > PACKAGE_LENGTH - 2 )
	{
		Log.write( "Size of netMessage exceeds MAX_MESSAGE_LENGTH", cLog::eLOG_TYPE_NET_ERROR );
		return;
	}

	*((Sint16*) (data + iLength)) = SDL_SwapLE16(i);
	iLength += 2;
}

Sint16 cNetMessage::popInt16()
{
	if ( iLength <= 7 )
	{
		Log.write( "Pop from empty netMessage", cLog::eLOG_TYPE_NET_ERROR );
		return 0;
	}
	iLength -= 2;
	return SDL_SwapLE16( *((Sint16*) (data + iLength)) );
}

void cNetMessage::pushInt32( Sint32 i )
{
	if ( iLength > PACKAGE_LENGTH - 4 )
	{
		Log.write( "Size of netMessage exceeds MAX_MESSAGE_LENGTH", cLog::eLOG_TYPE_NET_ERROR );
		return;
	}

	*((Sint32*) (data + iLength)) = SDL_SwapLE32( i );
	iLength += 4;
}

Sint32 cNetMessage::popInt32()
{
	if ( iLength <= 9 )
	{
		Log.write( "Pop from empty netMessage", cLog::eLOG_TYPE_NET_ERROR );
		return 0;
	}
	iLength -= 4;
	return SDL_SwapLE32( *((Sint32*) (data + iLength)) );
}

void cNetMessage::pushString( string s )
{
	int stringLength = (int) s.length() + 2;

	if ( iLength > PACKAGE_LENGTH - stringLength )
	{
		Log.write( "Size of netMessage exceeds MAX_MESSAGE_LENGTH", cLog::eLOG_TYPE_NET_ERROR );
		return;
	}

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
		Log.write( "Invalid popString() from netMessage", cLog::eLOG_TYPE_NET_ERROR );
		return string("");
	}

	//find begin of string
	iLength -= 2;
	while ( data[iLength] != '\0' )
	{
		if ( iLength <= 6 )
		{
			Log.write( "Pop string from netMessage failed, begin of string not found", cLog::eLOG_TYPE_NET_ERROR );
			return string("");
		}
		iLength--;
	}
	//(data + iLenght) points now to the leading '\0'

	return string( data + iLength + 1 );
}

void cNetMessage::pushBool( bool b )
{

	if ( iLength > PACKAGE_LENGTH - 1 )
	{
		Log.write( "Size of netMessage exceeds MAX_MESSAGE_LENGTH", cLog::eLOG_TYPE_NET_ERROR );
		return;
	}

	data[iLength] = b;
	iLength++;
}

bool cNetMessage::popBool()
{
	if ( iLength <= 6 )
	{
		Log.write( "Pop from empty netMessage", cLog::eLOG_TYPE_NET_ERROR );
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
    significand = Uint32(fnorm * ((1LL<<significandbits) + 0.5f));

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
    result = float( i & ( (1LL<<significandbits) - 1)); // mask
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
    result *= float((i>>(BITS-1))&1 ? -1.0: 1.0);

    return result;
}

string cNetMessage::getTypeAsString()
{
	//find the string representation of the message type for
	//should be updated when implementing a new message type
	switch (iType)
	{
	case MU_MSG_CHAT:
		return string("MU_MSG_CHAT");
	case MU_MSG_NEW_PLAYER:
		return string("MU_MSG_NEW_PLAYER");
	case MU_MSG_REQ_IDENTIFIKATION:
		return string("MU_MSG_REQ_IDENTIFIKATION");
	case MU_MSG_IDENTIFIKATION:
		return string("MU_MSG_IDENTIFIKATION");
	case MU_MSG_DEL_PLAYER:
		return string("MU_MSG_DEL_PLAYER");
	case MU_MSG_PLAYERLIST:
		return string("MU_MSG_PLAYERLIST");
	case MU_MSG_OPTINS:
		return string("MU_MSG_OPTINS");
	case MU_MSG_GO:
		return string("MU_MSG_GO");
	case MU_MSG_CLAN:
		return string("MU_MSG_CLAN");
	case MU_MSG_LANDING_VEHICLES:
		return string("MU_MSG_LANDING_VEHICLES");
	case MU_MSG_UPGRADES:
		return string("MU_MSG_UPGRADES");
	case MU_MSG_LANDING_COORDS:
		return string("MU_MSG_LANDING_COORDS");
	case MU_MSG_RESELECT_LANDING:
		return string("MU_MSG_RESELECT_LANDING");
	case MU_MSG_ALL_LANDED:
		return string("MU_MSG_ALL_LANDED");
	case MU_MSG_START_MAP_DOWNLOAD:
		return string("MU_MSG_START_MAP_DOWNLOAD");
	case MU_MSG_MAP_DOWNLOAD_DATA:
		return string("MU_MSG_MAP_DOWNLOAD_DATA");
	case MU_MSG_CANCELED_MAP_DOWNLOAD:
		return string("MU_MSG_CANCELED_MAP_DOWNLOAD");
	case MU_MSG_FINISHED_MAP_DOWNLOAD:
		return string("MU_MSG_FINISHED_MAP_DOWNLOAD");
	case MU_MSG_REQUEST_MAP:
		return string("MU_MSG_REQUEST_MAP");
	case GAME_EV_PLAYER_CLANS:
		return string("GAME_EV_PLAYER_CLANS");
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
	case GAME_EV_NEXT_MOVE:
		return string("GAME_EV_NEXT_MOVE");
	case GAME_EV_MOVE_JOB_SERVER:
		return string("GAME_EV_MOVE_JOB_SERVER");
	case GAME_EV_MOVE_JOB_CLIENT:
		return string("GAME_EV_MOVE_JOB_CLIENT");
	case GAME_EV_WANT_STOP_MOVE:
		return string("GAME_EV_WANT_STOP_MOVE");
	case GAME_EV_WANT_ATTACK:
		return string("GAME_EV_WANT_ATTACK");
	case GAME_EV_ATTACKJOB_LOCK_TARGET:
		return string("GAME_EV_ATTACKJOB_LOCK_TARGET");
	case GAME_EV_ATTACKJOB_FIRE:
		return string("GAME_EV_ATTACKJOB_FIRE");
	case GAME_EV_ATTACKJOB_FINISHED:
		return string("GAME_EV_ATTACKJOB_FINISHED");
	case GAME_EV_RESOURCES:
		return string("GAME_EV_RESOURCES");
	case GAME_EV_MINELAYERSTATUS:
		return string("GAME_EV_MINELAYERSTATUS");
	case GAME_EV_WANT_BUILD:
		return string("GAME_EV_WANT_BUILD");
	case GAME_EV_BUILD_ANSWER:
		return string("GAME_EV_BUILD_ANSWER");
	case GAME_EV_STOP_BUILD:
		return string("GAME_EV_STOP_BUILD");
	case GAME_EV_END_BUILDING:
		return string("GAME_EV_END_BUILDING");
	case GAME_EV_WANT_STOP_BUILDING:
		return string("GAME_EV_WANT_STOP_BUILDING");
	case GAME_EV_NEW_SUBBASE:
		return string("GAME_EV_NEW_SUBBASE");
	case GAME_EV_DELETE_SUBBASE:
		return string("GAME_EV_DELETE_SUBBASE");
	case GAME_EV_SUBBASE_BUILDINGS:
		return string("GAME_EV_SUBBASE_BUILDINGS");
	case GAME_EV_SUBBASE_VALUES:
		return string("GAME_EV_SUBBASE_VALUES");
	case GAME_EV_WANT_TRANSFER:
		return string("GAME_EV_WANT_TRANSFER");
	case GAME_EV_WANT_BUILDLIST:
		return string("GAME_EV_WANT_BUILDLIST");
	case GAME_EV_BUILDLIST:
		return string("GAME_EV_BUILDLIST");
	case GAME_EV_WANT_EXIT_FIN_VEH:
		return string("GAME_EV_WANT_EXIT_FIN_VEH");
	case GAME_EV_MINE_PRODUCE_VALUES:
		return string("GAME_EV_MINE_PRODUCE_VALUES");
	case GAME_EV_CHANGE_RESOURCES:
		return string("GAME_EV_CHANGE_RESOURCES");
	case GAME_EV_ATTACKJOB_IMPACT:
		return string("GAME_EV_ATTACKJOB_IMPACT");
	case GAME_EV_TURN_REPORT:
		return string("GAME_EV_TURN_REPORT");
	case GAME_EV_WANT_CHANGE_SENTRY:
		return string("GAME_EV_WANT_CHANGE_SENTRY");
	case GAME_EV_ADD_RUBBLE:
		return string("GAME_EV_ADD_RUBBLE");
	case GAME_EV_WANT_SUPPLY:
		return string("GAME_EV_WANT_SUPPLY");
	case GAME_EV_SUPPLY:
		return string("GAME_EV_SUPPLY");
	case GAME_EV_DETECTION_STATE:
		return string("GAME_EV_DETECTION_STATE");
	case GAME_EV_CLEAR_ANSWER:
		return string("GAME_EV_CLEAR_ANSWER");
	case GAME_EV_STOP_CLEARING:
		return string("GAME_EV_STOP_CLEARING");
	case GAME_EV_NOFOG:
		return string("GAME_EV_NOFOG");
	case GAME_EV_WANT_START_CLEAR:
		return string("GAME_EV_WANT_START_CLEAR");
	case GAME_EV_WANT_STOP_CLEAR:
		return string("GAME_EV_WANT_STOP_CLEAR");
	case GAME_EV_DEFEATED:
		return string("GAME_EV_DEFEATED");
	case GAME_EV_ABORT_WAITING:
		return string("GAME_EV_ABORT_WAITING");
	case GAME_EV_FREEZE:
		return string("GAME_EV_FREEZE");
	case GAME_EV_DEFREEZE:
		return string("GAME_EV_DEFREEZE");
	case GAME_EV_DEL_PLAYER:
		return string("GAME_EV_DEL_PLAYER");
	case GAME_EV_IDENTIFICATION:
		return string("GAME_EV_IDENTIFICATION");
	case GAME_EV_RECON_SUCESS:
		return string("GAME_EV_RECON_SUCESS");
	case GAME_EV_REQ_IDENT:
		return string("GAME_EV_REQ_IDENT");
	case GAME_EV_OK_RECONNECT:
		return string("GAME_EV_OK_RECONNECT");
	case GAME_EV_SPECIFIC_UNIT_DATA:
		return string("GAME_EV_SPECIFIC_UNIT_DATA");
	case GAME_EV_TURN:
		return string("GAME_EV_TURN");
	case GAME_EV_HUD_SETTINGS:
		return string("GAME_EV_HUD_SETTINGS");
	case GAME_EV_WANT_LOAD:
		return string("GAME_EV_WANT_LOAD");
	case GAME_EV_WANT_EXIT:
		return string("GAME_EV_WANT_EXIT");
	case GAME_EV_STORE_UNIT:
		return string("GAME_EV_STORE_UNIT");
	case GAME_EV_EXIT_UNIT:
		return string("GAME_EV_EXIT_UNIT");
	case GAME_EV_UNIT_UPGRADE_VALUES:
		return string("GAME_EV_UNIT_UPGRADE_VALUES");
	case GAME_EV_WANT_MARK_LOG:
		return string("GAME_EV_WANT_MARK_LOG");
	case GAME_EV_MARK_LOG:
		return string("GAME_EV_MARK_LOG");
	case GAME_EV_CREDITS_CHANGED:
		return string("GAME_EV_CREDITS_CHANGED");
	case GAME_EV_UPGRADED_BUILDINGS:
		return string("GAME_EV_UPGRADED_BUILDINGS");
	case GAME_EV_UPGRADED_VEHICLES:
		return string("GAME_EV_UPGRADED_VEHICLES");
	case GAME_EV_RESEARCH_SETTINGS:
		return string("GAME_EV_RESEARCH_SETTINGS");
	case GAME_EV_RESEARCH_LEVEL:
		return string("GAME_EV_RESEARCH_LEVEL");
	case GAME_EV_REFRESH_RESEARCH_COUNT:
		return string("GAME_EV_REFRESH_RESEARCH_COUNT");
	case GAME_EV_WANT_VEHICLE_UPGRADE:
		return string("GAME_EV_WANT_VEHICLE_UPGRADE");
	case GAME_EV_WANT_BUY_UPGRADES:
		return string("GAME_EV_WANT_BUY_UPGRADES");
	case GAME_EV_WANT_BUILDING_UPGRADE:
		return string("GAME_EV_WANT_BUILDING_UPGRADE");
	case GAME_EV_WANT_RESEARCH_CHANGE:
		return string("GAME_EV_WANT_RESEARCH_CHANGE");
	case GAME_EV_AUTOMOVE_STATUS:
		return string("GAME_EV_AUTOMOVE_STATUS");
	case GAME_EV_SET_AUTOMOVE:
		return string("GAME_EV_SET_AUTOMOVE");
	case GAME_EV_WANT_COM_ACTION:
		return string("GAME_EV_WANT_COM_ACTION");
	case GAME_EV_COMMANDO_ANSWER:
		return string("GAME_EV_COMMANDO_ANSWER");
	case GAME_EV_REQ_SAVE_INFO:
		return string("GAME_EV_REQ_SAVE_INFO");
	case GAME_EV_SAVED_REPORT:
		return string("GAME_EV_SAVED_REPORT");
	case GAME_EV_SAVE_HUD_INFO:
		return string("GAME_EV_SAVE_HUD_INFO");
	case GAME_EV_SAVE_REPORT_INFO:
		return string("GAME_EV_SAVE_REPORT_INFO");
	case GAME_EV_FIN_SEND_SAVE_INFO:
		return string("GAME_EV_FIN_SEND_SAVE_INFO");
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
