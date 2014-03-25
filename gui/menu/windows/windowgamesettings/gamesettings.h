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

#ifndef gui_menu_windows_windowgamesettings_gamesettingsH
#define gui_menu_windows_windowgamesettings_gamesettingsH

#include <string>

class cNetMessage;

enum class eGameSettingsResourceAmount
{
	Limited,
	Normal,
	High,
	TooMuch
};

enum class eGameSettingsResourceDensity
{
	Sparse,
	Normal,
	Dense,
	TooMuch
};

enum class eGameSettingsBridgeheadType
{
	Mobile,
	Definite,
};

enum class eGameSettingsGameType
{
	Simultaneous,
	Turns,
	HotSeat
};

enum class eGameSettingsVictoryCondition
{
	Turns,
	Points,
	Death
};

std::string gameSettingsResourceAmountToString (eGameSettingsResourceAmount amount);
eGameSettingsResourceAmount gameSettingsResourceAmountFromString (const std::string& string);

std::string gameSettingsResourceDensityToString (eGameSettingsResourceDensity density);
eGameSettingsResourceDensity gameSettingsResourceDensityFromString (const std::string& string);

std::string gameSettingsBridgeheadTypeToString (eGameSettingsBridgeheadType type);
eGameSettingsBridgeheadType gameSettingsBridgeheadTypeFromString (const std::string& string);

std::string gameSettingsGameTypeToString (eGameSettingsGameType type);
eGameSettingsGameType gameSettingsGameTypeString (const std::string& string);

std::string gameSettingsVictoryConditionToString (eGameSettingsVictoryCondition condition);
eGameSettingsVictoryCondition gameSettingsVictoryConditionFromString (const std::string& string);

class cGameSettings
{
public:
	static const int defaultCreditsNone    = 0;
	static const int defaultCreditsLow     = 50;
	static const int defaultCreditsLimited = 100;
	static const int defaultCreditsNormal  = 150;
	static const int defaultCreditsHigh    = 200;
	static const int defaultCreditsMore    = 250;

	cGameSettings ();

	eGameSettingsResourceAmount getMetalAmount () const;
	void setMetalAmount (eGameSettingsResourceAmount value);

	eGameSettingsResourceAmount getOilAmount () const;
	void setOilAmount (eGameSettingsResourceAmount value);

	eGameSettingsResourceAmount getGoldAmount () const;
	void setGoldAmount (eGameSettingsResourceAmount value);

	eGameSettingsResourceDensity getResourceDensity () const;
	void setResourceDensity (eGameSettingsResourceDensity value);

	eGameSettingsBridgeheadType getBridgeheadType () const;
	void setBridgeheadType (eGameSettingsBridgeheadType value);

	eGameSettingsGameType getGameType () const;
	void setGameType (eGameSettingsGameType value);

	bool getClansEnabled () const;
	void setClansEnabled (bool value);

	unsigned int getStartCredits () const;
	void setStartCredits (unsigned int value);

	eGameSettingsVictoryCondition getVictoryCondition () const;
	void setVictoryCondition (eGameSettingsVictoryCondition value);

	unsigned int getVictoryTurns () const;
	void setVictoryTurns (unsigned int value);

	unsigned int getVictoryPoints () const;
	void setVictoryPoints (unsigned int value);

	unsigned int getTurnDeadline () const;
	void setTurnDeadline (unsigned int value);

	void pushInto (cNetMessage& message) const;
	void popFrom (cNetMessage& message);
private:
	eGameSettingsResourceAmount metalAmount;
	eGameSettingsResourceAmount oilAmount;
	eGameSettingsResourceAmount goldAmount;

	eGameSettingsResourceDensity resourceDensity;

	eGameSettingsBridgeheadType bridgeheadType;

	eGameSettingsGameType gameType;

	bool clansEnabled;

	unsigned int startCredits;

	eGameSettingsVictoryCondition victoryConditionType;
	unsigned int victoryTurns;
	unsigned int vectoryPoints;

	unsigned int turnDeadline;
};

#endif // gui_menu_windows_windowgamesettings_gamesettingsH
