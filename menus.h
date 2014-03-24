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
#ifndef menusH
#define menusH

#include <string>
#include "main.h"

class cNetMessage;

struct sLandingUnit
{
	sID unitID;
	int cargo;
};

enum eLandingState
{
	LANDING_STATE_UNKNOWN,      //initial state
	LANDING_POSITION_OK,        //there are no other players near the position
	LANDING_POSITION_WARNING,   //there are players within the warning distance
	LANDING_POSITION_TOO_CLOSE, //the position is too close to another player
	LANDING_POSITION_CONFIRMED  //warnings about nearby players will be ignored,
	//because the player has confirmed his position
};

std::string ToString (eLandingState state);

struct sClientLandData
{
	int iLandX, iLandY;
	int iLastLandX, iLastLandY;
	eLandingState landingState;
	bool receivedOK;

	sClientLandData() :
		iLandX (-1), iLandY (-1), iLastLandX (-1), iLastLandY (-1),
		landingState (LANDING_STATE_UNKNOWN), receivedOK (false)
	{}

	/** checks whether the landing positions are okay
	 *@author alzi
	 */
	static eLandingState checkLandingState (std::vector<sClientLandData>& landData, unsigned int playerNr);
};

enum eSettingResourceValue
{
	SETTING_RESVAL_LIMITED,
	SETTING_RESVAL_NORMAL,
	SETTING_RESVAL_HIGH,
	SETTING_RESVAL_TOOMUCH
};

enum eSettingResFrequency
{
	SETTING_RESFREQ_SPARSE,
	SETTING_RESFREQ_NORMAL,
	SETTING_RESFREQ_DENCE,
	SETTING_RESFREQ_TOOMUCH
};

enum eSettingsCredits
{
	SETTING_CREDITS_NONE = 0,
	SETTING_CREDITS_LOW = 50,
	SETTING_CREDITS_LIMITED = 100,
	SETTING_CREDITS_NORMAL = 150,
	SETTING_CREDITS_HIGH = 200,
	SETTING_CREDITS_MORE = 250
};

enum eSettingsBridgeHead
{
	SETTING_BRIDGEHEAD_MOBILE,
	SETTING_BRIDGEHEAD_DEFINITE
};

enum eSettingsAlienTech
{
	SETTING_ALIENTECH_ON,
	SETTING_ALIENTECH_OFF
};

enum eSettingsClans
{
	SETTING_CLANS_ON,
	SETTING_CLANS_OFF
};

enum eSettingsGameType
{
	SETTINGS_GAMETYPE_SIMU,
	SETTINGS_GAMETYPE_TURNS
};

enum eSettingsVictoryType
{
	SETTINGS_VICTORY_TURNS,
	SETTINGS_VICTORY_POINTS,
	SETTINGS_VICTORY_ANNIHILATION
};

enum eSettingsDuration
{
	SETTINGS_DUR_SHORT = 100,
	SETTINGS_DUR_MEDIUM = 200,
	SETTINGS_DUR_LONG = 400
};

enum ePlayerType
{
	PLAYERTYPE_HUMAN,
	PLAYERTYPE_NONE,
	PLAYERTYPE_PC
};



/**
 * A class that contains all settings for a new game.
 *@author alzi
 */
struct sSettings
{
	eSettingResourceValue metal, oil, gold;
	eSettingResFrequency resFrequency;
	unsigned int credits;
	eSettingsBridgeHead bridgeHead;
	eSettingsAlienTech alienTech;
	eSettingsClans clans;
	eSettingsGameType gameType;
	eSettingsVictoryType victoryType;
	int duration;
	/** deadline in seconds when the first player has finished his turn */
	int iTurnDeadline;
	bool hotseat;

	sSettings() :
		metal (SETTING_RESVAL_NORMAL), oil (SETTING_RESVAL_NORMAL), gold (SETTING_RESVAL_NORMAL),
		resFrequency (SETTING_RESFREQ_NORMAL), credits (SETTING_CREDITS_NORMAL),
		bridgeHead (SETTING_BRIDGEHEAD_DEFINITE), alienTech (SETTING_ALIENTECH_OFF),
		clans (SETTING_CLANS_ON), gameType (SETTINGS_GAMETYPE_SIMU),
		victoryType (SETTINGS_VICTORY_POINTS),
		duration (SETTINGS_DUR_MEDIUM), iTurnDeadline (90), hotseat (false) {}

	void pushInto (cNetMessage& message) const;
	void popFrom (cNetMessage& message);

	std::string getResValString (eSettingResourceValue type) const;
	std::string getResFreqString() const;
	std::string getVictoryConditionString() const;

	bool isTurnBasedGame() const { return gameType == SETTINGS_GAMETYPE_TURNS; }
};

#endif //menusH
