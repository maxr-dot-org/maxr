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
#include "ui/graphical/menu/control/menuevents.h"
#include "network.h"
#include "game/logic/serverevents.h"

using namespace std;

#define START_CHAR 0xFF

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
	pushInt16 (id.secondPart);
	pushInt16 (id.firstPart);
}

sID cNetMessage::popID()
{
	sID id;

	id.firstPart = popInt16();
	id.secondPart = popInt16();
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
		case GAME_EV_DEL_BUILDING: return "GAME_EV_DEL_BUILDING";
		case GAME_EV_DEL_VEHICLE: return "GAME_EV_DEL_VEHICLE";
		case GAME_EV_UNIT_DATA: return "GAME_EV_UNIT_DATA";
		case GAME_EV_RESOURCES: return "GAME_EV_RESOURCES";
		case GAME_EV_SUBBASE_VALUES: return "GAME_EV_SUBBASE_VALUES";
		case GAME_EV_MINE_PRODUCE_VALUES: return "GAME_EV_MINE_PRODUCE_VALUES";
		case GAME_EV_CHANGE_RESOURCES: return "GAME_EV_CHANGE_RESOURCES";
		case GAME_EV_ADD_RUBBLE: return "GAME_EV_ADD_RUBBLE";
		case GAME_EV_WANT_SUPPLY: return "GAME_EV_WANT_SUPPLY";
		case GAME_EV_SUPPLY: return "GAME_EV_SUPPLY";
		case GAME_EV_CLEAR_ANSWER: return "GAME_EV_CLEAR_ANSWER";
		case GAME_EV_STOP_CLEARING: return "GAME_EV_STOP_CLEARING";
		case GAME_EV_NOFOG: return "GAME_EV_NOFOG";
		case GAME_EV_WANT_START_CLEAR: return "GAME_EV_WANT_START_CLEAR";
		case GAME_EV_WANT_STOP_CLEAR: return "GAME_EV_WANT_STOP_CLEAR";
		case GAME_EV_DEFEATED: return "GAME_EV_DEFEATED";
		case GAME_EV_ABORT_WAITING: return "GAME_EV_ABORT_WAITING";
		case GAME_EV_DEL_PLAYER: return "GAME_EV_DEL_PLAYER";
		case GAME_EV_WANT_KICK_PLAYER: return "GAME_EV_WANT_KICK_PLAYER";
		case GAME_EV_SPECIFIC_UNIT_DATA: return "GAME_EV_SPECIFIC_UNIT_DATA";
		case GAME_EV_HUD_SETTINGS: return "GAME_EV_HUD_SETTINGS";
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
		case GAME_EV_CASUALTIES_REPORT: return "GAME_EV_CASUALTIES_REPORT";
		case GAME_EV_REQUEST_CASUALTIES_REPORT: return "GAME_EV_REQUEST_CASUALTIES_REPORT";
		case GAME_EV_SCORE: return "GAME_EV_SCORE";
		case GAME_EV_NUM_ECOS: return "GAME_EV_NUM_ECOS";
		case GAME_EV_UNIT_SCORE: return "GAME_EV_UNIT_SCORE";
		case GAME_EV_REVEAL_MAP: return "GAME_EV_REVEAL_MAP";
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
