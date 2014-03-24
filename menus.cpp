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

#include <cassert>

#include "menus.h"
#include "netmessage.h"

//------------------------------------------------------------------------------
void sSettings::pushInto (cNetMessage& message) const
{
	message.pushBool (hotseat);
	message.pushInt16 (iTurnDeadline);
	message.pushInt16 (duration);
	message.pushChar (victoryType);
	message.pushChar (gameType);
	message.pushChar (clans);
	message.pushChar (alienTech);
	message.pushChar (bridgeHead);
	message.pushInt16 (credits);
	message.pushChar (resFrequency);
	message.pushChar (gold);
	message.pushChar (oil);
	message.pushChar (metal);
}

//------------------------------------------------------------------------------
void sSettings::popFrom (cNetMessage& message)
{
	metal = (eSettingResourceValue) message.popChar();
	oil = (eSettingResourceValue) message.popChar();
	gold = (eSettingResourceValue) message.popChar();
	resFrequency = (eSettingResFrequency) message.popChar();
	credits = message.popInt16();
	bridgeHead = (eSettingsBridgeHead) message.popChar();
	alienTech = (eSettingsAlienTech) message.popChar();
	clans = (eSettingsClans) message.popChar();
	gameType = (eSettingsGameType) message.popChar();
	victoryType = (eSettingsVictoryType) message.popChar();
	duration = message.popInt16();
	iTurnDeadline = message.popInt16();
	hotseat = message.popBool();
}

//------------------------------------------------------------------------------
std::string sSettings::getResValString (eSettingResourceValue type) const
{
	switch (type)
	{
		case SETTING_RESVAL_LIMITED: return lngPack.i18n ("Text~Option~Limited");
		case SETTING_RESVAL_NORMAL: return lngPack.i18n ("Text~Option~Normal");
		case SETTING_RESVAL_HIGH: return lngPack.i18n ("Text~Option~High");
		case SETTING_RESVAL_TOOMUCH: return lngPack.i18n ("Text~Option~TooMuch");
	}
	return "";
}

//------------------------------------------------------------------------------
std::string sSettings::getResFreqString () const
{
	switch (resFrequency)
	{
		case SETTING_RESFREQ_SPARSE: return lngPack.i18n ("Text~Option~Sparse");
		case SETTING_RESFREQ_NORMAL: return lngPack.i18n ("Text~Option~Normal");
		case SETTING_RESFREQ_DENCE: return lngPack.i18n ("Text~Option~Dense");
		case SETTING_RESFREQ_TOOMUCH: return lngPack.i18n ("Text~Option~TooMuch");
	}
	return "";
}

std::string ToString (eLandingState state)
{
	switch (state)
	{
		case LANDING_POSITION_TOO_CLOSE: return "LANDING_POSITION_TOO_CLOSE";
		case LANDING_POSITION_WARNING: return "LANDING_POSITION_WARNING";
		case LANDING_POSITION_OK: return "LANDING_POSITION_OK";
		case LANDING_POSITION_CONFIRMED: return "LANDING_POSITION_COMFIRMED";
		case LANDING_STATE_UNKNOWN: return "LANDING_STATE_UNKNOWN";
	}
	assert (0);
	return "unknown";
}

//------------------------------------------------------------------------------
/*static*/ eLandingState sClientLandData::checkLandingState (std::vector<sClientLandData>& landData, unsigned int playerNr)
{
	int posX = landData[playerNr].iLandX;
	int posY = landData[playerNr].iLandY;
	int lastPosX = landData[playerNr].iLastLandX;
	int lastPosY = landData[playerNr].iLastLandY;
	bool bPositionTooClose = false;
	bool bPositionWarning = false;

	// check distances to all other players
	for (size_t i = 0; i != landData.size(); ++i)
	{
		const sClientLandData& c = landData[i];
		if (c.receivedOK == false) continue;
		if (i == playerNr) continue;

		const int sqDistance = Square (c.iLandX - posX) + Square (c.iLandY - posY);

		if (sqDistance < Square (LANDING_DISTANCE_TOO_CLOSE))
		{
			bPositionTooClose = true;
		}
		if (sqDistance < Square (LANDING_DISTANCE_WARNING))
		{
			bPositionWarning = true;
		}
	}

	// now set the new landing state,
	// depending on the last state, the last position, the current position,
	//  bPositionTooClose and bPositionWarning
	eLandingState lastState = landData[playerNr].landingState;
	eLandingState newState = LANDING_STATE_UNKNOWN;

	if (bPositionTooClose)
	{
		newState = LANDING_POSITION_TOO_CLOSE;
	}
	else if (bPositionWarning)
	{
		if (lastState == LANDING_POSITION_WARNING)
		{
			const int sqDelta = Square (posX - lastPosX) + Square (posY - lastPosY);
			if (sqDelta <= Square (LANDING_DISTANCE_TOO_CLOSE))
			{
				// the player has chosen the same position after a warning
				// so further warnings will be ignored
				newState = LANDING_POSITION_CONFIRMED;
			}
			else
			{
				newState = LANDING_POSITION_WARNING;
			}
		}
		else if (lastState == LANDING_POSITION_CONFIRMED)
		{
			// player is in state LANDING_POSITION_CONFIRMED,
			// so ignore the warning
			newState = LANDING_POSITION_CONFIRMED;
		}
		else
		{
			newState = LANDING_POSITION_WARNING;
		}
	}
	else
	{
		if (lastState == LANDING_POSITION_CONFIRMED)
		{
			newState = LANDING_POSITION_CONFIRMED;
		}
		else
		{
			newState = LANDING_POSITION_OK;
		}
	}

	landData[playerNr].landingState = newState;
	return newState;
}

//------------------------------------------------------------------------------
std::string sSettings::getVictoryConditionString() const
{
	std::string r = iToStr (duration) + " ";

	switch (victoryType)
	{
		case SETTINGS_VICTORY_TURNS: r += lngPack.i18n ("Text~Comp~Turns");  break;
		case SETTINGS_VICTORY_POINTS: r += lngPack.i18n ("Text~Comp~Points"); break;
		case SETTINGS_VICTORY_ANNIHILATION: return lngPack.i18n ("Text~Comp~NoLimit");
	}
	return r;
}