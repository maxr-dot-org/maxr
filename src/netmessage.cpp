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

#include "netmessage.h"

#include "game/logic/clientevents.h"
#include "utility/log.h"
#include "main.h"
#include "menuevents.h"
#include "network.h"
#include "game/logic/serverevents.h"

using namespace std;

cNetMessage::cNetMessage (const char* c)
{
	if (c[0] != START_CHAR) Log.write ("NetMessage has wrong start character", cLog::eLOG_TYPE_NET_ERROR);
	// Use temporary variable to avoid gcc warning:
	// "dereferencing type-punned pointer will break strict-aliasing rules"
	const Sint16* data16 = reinterpret_cast<const Sint16*> (c + 1);
	iLength = SDL_SwapLE16 (data16[0]);
	iType = SDL_SwapLE16 (data16[1]);
	iPlayerNr = c[5];

	data[0] = START_CHAR;
	memcpy (data, c, iLength);
}

cNetMessage::cNetMessage (int iType)
{
	this->iType = iType;

	data[0] = START_CHAR;
	// 0:     reserved for startchar
	// 1 - 2: reserved for length
	// 3 - 4: reserved for message type
	// 5:     reserved for playernumber
	iLength = 6;
	iPlayerNr = -1;
}

char* cNetMessage::serialize()
{
	//set start character
	data[0] = START_CHAR;
	// Use temporary variable to avoid gcc warning:
	// "dereferencing type-punned pointer will break strict-aliasing rules"
	Sint16* data16 = reinterpret_cast<Sint16*> (data + 1);
	//write iLenght to byte array
	*data16 = SDL_SwapLE16 ((Sint16) iLength);
	//write iType to byte array
	* (data16 + 1) = SDL_SwapLE16 ((Sint16) iType);
	//write iPlayernr to byte array
	data[5] = (char) iPlayerNr;

	return data;
}

void cNetMessage::rewind()
{
	// Use temporary variable to avoid gcc warning:
	// "dereferencing type-punned pointer will break strict-aliasing rules"
	const Sint16* data16 = reinterpret_cast<Sint16*> (data + 1);
	iLength = SDL_SwapLE16 (*data16);
}

eNetMessageClass cNetMessage::getClass() const
{
	if (iType < FIRST_SERVER_MESSAGE)
		return NET_MSG_STATUS;
	else if (iType < FIRST_CLIENT_MESSAGE)
		return NET_MSG_SERVER;
	else if (iType < FIRST_MENU_MESSAGE)
		return NET_MSG_CLIENT;
	else
		return NET_MSG_MENU;
}


void cNetMessage::pushChar (char c)
{
	if (iLength > PACKAGE_LENGTH - 1)
	{
		Log.write ("Size of netMessage exceeds MAX_MESSAGE_LENGTH", cLog::eLOG_TYPE_NET_ERROR);
		return;
	}

	data[iLength] = c;
	iLength ++;
}

char cNetMessage::popChar()
{
	if (iLength <= 6)
	{
		Log.write ("Pop from empty netMessage", cLog::eLOG_TYPE_NET_ERROR);
		return 0;
	}
	iLength--;
	return data[iLength];
}

void cNetMessage::pushInt16 (Sint16 i)
{
	if (iLength > PACKAGE_LENGTH - 2)
	{
		Log.write ("Size of netMessage exceeds MAX_MESSAGE_LENGTH", cLog::eLOG_TYPE_NET_ERROR);
		return;
	}

	// Use temporary variable to avoid gcc warning:
	// "dereferencing type-punned pointer will break strict-aliasing rules"
	Sint16* data16 = reinterpret_cast<Sint16*> (data + iLength);
	*data16 = SDL_SwapLE16 (i);
	iLength += 2;
}

Sint16 cNetMessage::popInt16()
{
	if (iLength <= 7)
	{
		Log.write ("Pop from empty netMessage", cLog::eLOG_TYPE_NET_ERROR);
		return 0;
	}
	iLength -= 2;
	// Use temporary variable to avoid gcc warning:
	// "dereferencing type-punned pointer will break strict-aliasing rules"
	const Sint16* data16 = reinterpret_cast<Sint16*> (data + iLength);
	return SDL_SwapLE16 (*data16);
}

void cNetMessage::pushInt32 (int32_t i)
{
	if (iLength > PACKAGE_LENGTH - 4)
	{
		Log.write ("Size of netMessage exceeds MAX_MESSAGE_LENGTH", cLog::eLOG_TYPE_NET_ERROR);
		return;
	}
	// Use temporary variable to avoid gcc warning:
	// "dereferencing type-punned pointer will break strict-aliasing rules"
	int32_t* data32 = reinterpret_cast<int32_t*> (data + iLength);
	*data32 = SDL_SwapLE32 (i);
	iLength += 4;
}

int32_t cNetMessage::popInt32()
{
	if (iLength <= 9)
	{
		Log.write ("Pop from empty netMessage", cLog::eLOG_TYPE_NET_ERROR);
		return 0;
	}
	iLength -= 4;
	// Use temporary variable to avoid gcc warning:
	// "dereferencing type-punned pointer will break strict-aliasing rules"
	const int32_t* data32 = reinterpret_cast<int32_t*> (data + iLength);
	return SDL_SwapLE32 (*data32);
}

void cNetMessage::pushString (const string& s)
{
	int stringLength = (int) s.length() + 2;

	if (iLength > PACKAGE_LENGTH - stringLength)
	{
		Log.write ("Size of netMessage exceeds MAX_MESSAGE_LENGTH", cLog::eLOG_TYPE_NET_ERROR);
		return;
	}

	//first write a '\0'
	//in the netMessage both begin and end of the string are marked with a '\0'
	//so we are able to pop it correctly
	data[iLength] = 0;
	iLength++;
	stringLength--;

	const char* c = s.c_str();
	memcpy (data + iLength, c, stringLength);

	iLength += stringLength;
}

string cNetMessage::popString()
{
	if (data[iLength - 1] != '\0')
	{
		Log.write ("Invalid popString() from netMessage", cLog::eLOG_TYPE_NET_ERROR);
		return string ("");
	}

	//find begin of string
	iLength -= 2;
	while (data[iLength] != '\0')
	{
		if (iLength <= 6)
		{
			Log.write ("Pop string from netMessage failed, begin of string not found", cLog::eLOG_TYPE_NET_ERROR);
			return string ("");
		}
		iLength--;
	}
	//(data + iLenght) points now to the leading '\0'

	return string (data + iLength + 1);
}

void cNetMessage::pushBool (bool b)
{
	if (iLength > PACKAGE_LENGTH - 1)
	{
		Log.write ("Size of netMessage exceeds MAX_MESSAGE_LENGTH", cLog::eLOG_TYPE_NET_ERROR);
		return;
	}

	data[iLength] = b ? 1 : 0;
	iLength++;
}

bool cNetMessage::popBool()
{
	if (iLength <= 6)
	{
		Log.write ("Pop from empty netMessage", cLog::eLOG_TYPE_NET_ERROR);
		return 0;
	}

	iLength--;

	if (data[iLength] != 0) return true;
	else return false;
}

#define BITS 32
#define EXPBITS 8

void cNetMessage::pushFloat (float f)
{
	const unsigned significandbits = BITS - EXPBITS - 1; // -1 for sign bit

	if (f == 0.0f)
	{
		pushInt32 (0);
		return;
	}

	float fnorm;
	int sign;
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
	uint32_t shift = 0;
	while (fnorm >= 2.0f)
	{
		fnorm /= 2.0f;
		shift++;
	}
	while (fnorm < 1.0f)
	{
		fnorm *= 2.0f;
		shift--;
	}
	fnorm -= 1.0f;

	// calculate the binary form (non-float) of the significand data
	const uint32_t significand = uint32_t (fnorm * ((1LL << significandbits) + 0.5f));

	// get the biased exponent
	const uint32_t exp = shift + ((1 << (EXPBITS - 1)) - 1);   // shift + bias

	// store result in netMessage
	pushInt32 ((sign << (BITS - 1)) | (exp << (BITS - EXPBITS - 1)) | significand);
}

float cNetMessage::popFloat()
{
	const unsigned significandbits = BITS - EXPBITS - 1; // -1 for sign bit

	// get data from netMessage
	uint32_t i = popInt32();

	if (i == 0) return 0.0f;

	// pull the significand
	float result (i & ((1LL << significandbits) - 1));     // mask
	result /= (1LL << significandbits);   // convert back to float
	result += 1.0f; // add the one back on

	// deal with the exponent
	unsigned bias = (1 << (EXPBITS - 1)) - 1;
	long long shift = ((i >> significandbits) & ((1LL << EXPBITS) - 1)) - bias;
	while (shift > 0)
	{
		result *= 2.0f;
		shift--;
	}
	while (shift < 0)
	{
		result /= 2.0f;
		shift++;
	}

	// sign it
	result *= ((i >> (BITS - 1)) & 1) ? -1.0f : 1.0f;

	return result;
}

void cNetMessage::pushID (const sID& id)
{
	pushInt16 (id.iSecondPart);
	pushInt16 (id.iFirstPart);
}

sID cNetMessage::popID()
{
	sID id;

	id.iFirstPart = popInt16();
	id.iSecondPart = popInt16();
	return id;
}

void cNetMessage::pushPosition (const cPosition& position)
{
	pushInt32 (position.x());
	pushInt32 (position.y());
}

cPosition cNetMessage::popPosition()
{
	cPosition position;
	position.y() = popInt32();
	position.x() = popInt32();
	return position;
}

void cNetMessage::pushColor (const cRgbColor& color)
{
	pushInt16 (color.r);
	pushInt16 (color.g);
	pushInt16 (color.b);
}

cRgbColor cNetMessage::popColor()
{
	cRgbColor color;
	color.b = popInt16();
	color.g = popInt16();
	color.r = popInt16();
	return color;
}

string cNetMessage::getTypeAsString() const
{
	//find the string representation of the message type for
	//should be updated when implementing a new message type
	switch (iType)
	{
		case TCP_CLOSE: return "TCP_CLOSE";
		case TCP_ACCEPT: return "TCP_ACCEPT";
		case MU_MSG_CHAT: return "MU_MSG_CHAT";
		case MU_MSG_REQ_IDENTIFIKATION: return "MU_MSG_REQ_IDENTIFIKATION";
		case MU_MSG_PLAYER_NUMBER: return "MU_MSG_PLAYER_NUMBER";
		case MU_MSG_IDENTIFIKATION: return "MU_MSG_IDENTIFIKATION";
		case MU_MSG_PLAYERLIST: return "MU_MSG_PLAYERLIST";
		case MU_MSG_OPTINS: return "MU_MSG_OPTINS";
		case MU_MSG_GO: return "MU_MSG_GO";
		case MU_MSG_CLAN: return "MU_MSG_CLAN";
		case MU_MSG_LANDING_VEHICLES: return "MU_MSG_LANDING_VEHICLES";
		case MU_MSG_UPGRADES: return "MU_MSG_UPGRADES";
		case MU_MSG_LANDING_COORDS: return "MU_MSG_LANDING_COORDS";
		case MU_MSG_READY_TO_START: return "MU_MSG_READY_TO_START";
		case MU_MSG_LANDING_STATE: return "MU_MSG_LANDING_STATE";
		case MU_MSG_ALL_LANDED: return "MU_MSG_ALL_LANDED";
		case MU_MSG_LANDING_POSITION: return "MU_MSG_LANDING_POSITION";
		case MU_MSG_IN_LANDING_POSITION_SELECTION_STATUS: return "MU_MSG_IN_LANDING_POSITION_SELECTION_STATUS";
		case MU_MSG_PLAYER_HAS_SELECTED_LANDING_POSITION: return "MU_MSG_PLAYER_HAS_SELECTED_LANDING_POSITION";
		case MU_MSG_PLAYER_HAS_ABORTED_GAME_PREPARATION: return "MU_MSG_PLAYER_HAS_ABORTED_GAME_PREPARATION";
		case MU_MSG_START_MAP_DOWNLOAD: return "MU_MSG_START_MAP_DOWNLOAD";
		case MU_MSG_MAP_DOWNLOAD_DATA: return "MU_MSG_MAP_DOWNLOAD_DATA";
		case MU_MSG_CANCELED_MAP_DOWNLOAD: return "MU_MSG_CANCELED_MAP_DOWNLOAD";
		case MU_MSG_FINISHED_MAP_DOWNLOAD: return "MU_MSG_FINISHED_MAP_DOWNLOAD";
		case MU_MSG_REQUEST_MAP: return "MU_MSG_REQUEST_MAP";
		case GAME_EV_PLAYER_CLANS: return "GAME_EV_PLAYER_CLANS";
		case GAME_EV_ADD_BUILDING: return "GAME_EV_ADD_BUILDING";
		case GAME_EV_ADD_VEHICLE: return "GAME_EV_ADD_VEHICLE";
		case GAME_EV_DEL_BUILDING: return "GAME_EV_DEL_BUILDING";
		case GAME_EV_DEL_VEHICLE: return "GAME_EV_DEL_VEHICLE";
		case GAME_EV_ADD_ENEM_BUILDING: return "GAME_EV_ADD_ENEM_BUILDING";
		case GAME_EV_ADD_ENEM_VEHICLE: return "GAME_EV_ADD_ENEM_VEHICLE";
		case GAME_EV_CHAT_CLIENT: return "GAME_EV_CHAT_CLIENT";
		case GAME_EV_WANT_TO_END_TURN: return "GAME_EV_WANT_TO_END_TURN";
		case GAME_EV_MAKE_TURNEND: return "GAME_EV_MAKE_TURNEND";
		case GAME_EV_FINISHED_TURN: return "GAME_EV_FINISHED_TURN";
		case GAME_EV_TURN_START_TIME: return "GAME_EV_TURN_START_TIME";
		case GAME_EV_TURN_END_DEADLINE_START_TIME: return "GAME_EV_TURN_END_DEADLINE_START_TIME";
		case GAME_EV_UNIT_DATA: return "GAME_EV_UNIT_DATA";
		case GAME_EV_WANT_START_WORK: return "GAME_EV_WANT_START_WORK";
		case GAME_EV_WANT_STOP_WORK: return "GAME_EV_WANT_STOP_WORK";
		case GAME_EV_DO_START_WORK: return "GAME_EV_DO_START_WORK";
		case GAME_EV_DO_STOP_WORK: return "GAME_EV_DO_STOP_WORK";
		case GAME_EV_NEXT_MOVE: return "GAME_EV_NEXT_MOVE";
		case GAME_EV_MOVE_JOB_SERVER: return "GAME_EV_MOVE_JOB_SERVER";
		case GAME_EV_MOVE_JOB_CLIENT: return "GAME_EV_MOVE_JOB_CLIENT";
		case GAME_EV_WANT_STOP_MOVE: return "GAME_EV_WANT_STOP_MOVE";
		case GAME_EV_WANT_ATTACK: return "GAME_EV_WANT_ATTACK";
		case GAME_EV_ATTACKJOB: return "GAME_EV_ATTACKJOB";
		case GAME_EV_RESOURCES: return "GAME_EV_RESOURCES";
		case GAME_EV_MINELAYERSTATUS: return "GAME_EV_MINELAYERSTATUS";
		case GAME_EV_WANT_BUILD: return "GAME_EV_WANT_BUILD";
		case GAME_EV_BUILD_ANSWER: return "GAME_EV_BUILD_ANSWER";
		case GAME_EV_STOP_BUILD: return "GAME_EV_STOP_BUILD";
		case GAME_EV_END_BUILDING: return "GAME_EV_END_BUILDING";
		case GAME_EV_WANT_STOP_BUILDING: return "GAME_EV_WANT_STOP_BUILDING";
		case GAME_EV_SUBBASE_VALUES: return "GAME_EV_SUBBASE_VALUES";
		case GAME_EV_WANT_TRANSFER: return "GAME_EV_WANT_TRANSFER";
		case GAME_EV_WANT_BUILDLIST: return "GAME_EV_WANT_BUILDLIST";
		case GAME_EV_BUILDLIST: return "GAME_EV_BUILDLIST";
		case GAME_EV_WANT_EXIT_FIN_VEH: return "GAME_EV_WANT_EXIT_FIN_VEH";
		case GAME_EV_MINE_PRODUCE_VALUES: return "GAME_EV_MINE_PRODUCE_VALUES";
		case GAME_EV_CHANGE_RESOURCES: return "GAME_EV_CHANGE_RESOURCES";
		case GAME_EV_WANT_CHANGE_MANUAL_FIRE: return "GAME_EV_WANT_CHANGE_MANUAL_FIRE";
		case GAME_EV_WANT_CHANGE_SENTRY: return "GAME_EV_WANT_CHANGE_SENTRY";
		case GAME_EV_ADD_RUBBLE: return "GAME_EV_ADD_RUBBLE";
		case GAME_EV_WANT_SUPPLY: return "GAME_EV_WANT_SUPPLY";
		case GAME_EV_SUPPLY: return "GAME_EV_SUPPLY";
		case GAME_EV_DETECTION_STATE: return "GAME_EV_DETECTION_STATE";
		case GAME_EV_CLEAR_ANSWER: return "GAME_EV_CLEAR_ANSWER";
		case GAME_EV_STOP_CLEARING: return "GAME_EV_STOP_CLEARING";
		case GAME_EV_NOFOG: return "GAME_EV_NOFOG";
		case GAME_EV_WANT_START_CLEAR: return "GAME_EV_WANT_START_CLEAR";
		case GAME_EV_WANT_STOP_CLEAR: return "GAME_EV_WANT_STOP_CLEAR";
		case GAME_EV_DEFEATED: return "GAME_EV_DEFEATED";
		case GAME_EV_ABORT_WAITING: return "GAME_EV_ABORT_WAITING";
		case GAME_EV_FREEZE: return "GAME_EV_FREEZE";
		case GAME_EV_UNFREEZE: return "GAME_EV_UNFREEZE";
		case GAME_EV_DEL_PLAYER: return "GAME_EV_DEL_PLAYER";
		case GAME_EV_IDENTIFICATION: return "GAME_EV_IDENTIFICATION";
		case GAME_EV_RECON_SUCCESS: return "GAME_EV_RECON_SUCCESS";
		case GAME_EV_REQ_RECON_IDENT: return "GAME_EV_REQ_RECON_IDENT";
		case GAME_EV_WANT_KICK_PLAYER: return "GAME_EV_WANT_KICK_PLAYER";
		case GAME_EV_RECONNECT_ANSWER: return "GAME_EV_RECONNECT_ANSWER";
		case GAME_EV_SPECIFIC_UNIT_DATA: return "GAME_EV_SPECIFIC_UNIT_DATA";
		case GAME_EV_TURN: return "GAME_EV_TURN";
		case GAME_EV_HUD_SETTINGS: return "GAME_EV_HUD_SETTINGS";
		case GAME_EV_WANT_LOAD: return "GAME_EV_WANT_LOAD";
		case GAME_EV_WANT_EXIT: return "GAME_EV_WANT_EXIT";
		case GAME_EV_STORE_UNIT: return "GAME_EV_STORE_UNIT";
		case GAME_EV_EXIT_UNIT: return "GAME_EV_EXIT_UNIT";
		case GAME_EV_UNIT_UPGRADE_VALUES: return "GAME_EV_UNIT_UPGRADE_VALUES";
		case GAME_EV_WANT_MARK_LOG: return "GAME_EV_WANT_MARK_LOG";
		case GAME_EV_MARK_LOG: return "GAME_EV_MARK_LOG";
		case GAME_EV_CREDITS_CHANGED: return "GAME_EV_CREDITS_CHANGED";
		case GAME_EV_UPGRADED_BUILDINGS: return "GAME_EV_UPGRADED_BUILDINGS";
		case GAME_EV_UPGRADED_VEHICLES: return "GAME_EV_UPGRADED_VEHICLES";
		case GAME_EV_RESEARCH_SETTINGS: return "GAME_EV_RESEARCH_SETTINGS";
		case GAME_EV_RESEARCH_LEVEL: return "GAME_EV_RESEARCH_LEVEL";
		case GAME_EV_FINISHED_RESEARCH_AREAS: return "GAME_EV_FINISHED_RESEARCH_AREAS";
		case GAME_EV_REFRESH_RESEARCH_COUNT: return "GAME_EV_REFRESH_RESEARCH_COUNT";
		case GAME_EV_WANT_VEHICLE_UPGRADE: return "GAME_EV_WANT_VEHICLE_UPGRADE";
		case GAME_EV_WANT_BUY_UPGRADES: return "GAME_EV_WANT_BUY_UPGRADES";
		case GAME_EV_WANT_BUILDING_UPGRADE: return "GAME_EV_WANT_BUILDING_UPGRADE";
		case GAME_EV_WANT_RESEARCH_CHANGE: return "GAME_EV_WANT_RESEARCH_CHANGE";
		case GAME_EV_AUTOMOVE_STATUS: return "GAME_EV_AUTOMOVE_STATUS";
		case GAME_EV_SET_AUTOMOVE: return "GAME_EV_SET_AUTOMOVE";
		case GAME_EV_WANT_COM_ACTION: return "GAME_EV_WANT_COM_ACTION";
		case GAME_EV_COMMANDO_ANSWER: return "GAME_EV_COMMANDO_ANSWER";
		case GAME_EV_REQ_SAVE_INFO: return "GAME_EV_REQ_SAVE_INFO";
		case GAME_EV_SAVED_REPORT: return "GAME_EV_SAVED_REPORT";
		case GAME_EV_CASUALTIES_REPORT: return "GAME_EV_CASUALTIES_REPORT";
		case GAME_EV_SAVE_HUD_INFO: return "GAME_EV_SAVE_HUD_INFO";
		case GAME_EV_SAVE_REPORT_INFO: return "GAME_EV_SAVE_REPORT_INFO";
		case GAME_EV_FIN_SEND_SAVE_INFO: return "GAME_EV_FIN_SEND_SAVE_INFO";
		case GAME_EV_REQUEST_CASUALTIES_REPORT: return "GAME_EV_REQUEST_CASUALTIES_REPORT";
		case GAME_EV_REQUEST_RESYNC: return "GAME_EV_REQUEST_RESYNC";
		case GAME_EV_DELETE_EVERYTHING: return "GAME_EV_DELETE_EVERYTHING";
		case GAME_EV_SCORE: return "GAME_EV_SCORE";
		case GAME_EV_NUM_ECOS: return "GAME_EV_NUM_ECOS";
		case GAME_EV_UNIT_SCORE: return "GAME_EV_UNIT_SCORE";
		case GAME_EV_GAME_SETTINGS: return "GAME_EV_GAME_SETTINGS";
		case GAME_EV_WAIT_FOR: return "GAME_EV_WAIT_FOR";
		case GAME_EV_END_MOVE_ACTION_SERVER: return "GAME_EV_END_MOVE_ACTION_SERVER";
		case GAME_EV_END_MOVE_ACTION: return "GAME_EV_END_MOVE_ACTION";
		case GAME_EV_REVEAL_MAP: return "GAME_EV_REVEAL_MAP";
		case NET_GAME_TIME_SERVER: return "NET_GAME_TIME_SERVER";
		case NET_GAME_TIME_CLIENT: return "NET_GAME_TIME_CLIENT";
		case GAME_EV_MOVEJOB_RESUME: return "GAME_EV_MOVEJOB_RESUME";
		default: return iToStr (iType);
	}
}

string cNetMessage::getHexDump()
{
	string dump = "0x";

	serialize(); //updates the internal data

	const char hexChars[] = "0123456789ABCDEF";
	for (int i = 0; i < iLength; i++)
	{
		const unsigned char byte = data[i];
		const unsigned char high = (byte >> 4) & 0x0F;
		const unsigned char low = byte & 0x0F;

		dump += hexChars[high];
		dump += hexChars[low];
	}
	return dump;
}
