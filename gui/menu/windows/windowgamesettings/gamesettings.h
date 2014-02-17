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

enum class eGameSettingsResourceAmount
{
	Low,
	Normal,
	Much,
	Most
};

enum class eGameSettingsResourceDensity
{
	Thin,
	Normal,
	Thick,
	Most
};

enum class eGameSettingsBridgeheadType
{
	Mobile,
	Definite,
};

enum class eGameSettingsGameType
{
	Turns,
	Simultaneous,
	HotSeat
};

enum class eGameSettingsVictoryCondition
{
	Turns,
	Points,
	Death
};

class cGameSettings
{
public:
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
};

#endif // gui_menu_windows_windowgamesettings_gamesettingsH
