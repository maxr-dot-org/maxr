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

#ifndef ui_translationsH
#define ui_translationsH

#include <string>
#include <vector>

class cBuilding;
class cClan;
class cClanUnitStat;
class cModel;
class cPlayer;
class cSavedReport;
class cStaticUnitData;
class cUnit;
class cUnitsData;
class cVecicle;

enum class eGameSettingsResourceAmount;
enum class eGameSettingsResourceDensity;
enum class eGameSettingsBridgeheadType;
enum class eGameSettingsGameType;
enum class eGameSettingsVictoryCondition;
enum class eResourceType;

std::string toTranslatedString (eGameSettingsResourceAmount);
std::string toTranslatedString (eGameSettingsResourceDensity);
std::string toTranslatedString (eGameSettingsBridgeheadType);
std::string toTranslatedString (eGameSettingsGameType);
std::string toTranslatedString (eGameSettingsVictoryCondition, int turn, int point);
std::string toTranslatedString (eResourceType);

std::string getClanStatsDescription (const cClanUnitStat&, const cUnitsData& originalData);
std::vector<std::string> getClanStatsDescription (const cClan&, const cUnitsData& originalData);

std::string getClanName (const cClan&);
std::string getClanDescription (const cClan&);

std::string getStaticUnitName (const cStaticUnitData&);
std::string getStaticUnitDescription (const cStaticUnitData&);

std::string getNamePrefix (const cUnit& unit);
std::string getName (const cUnit&);
std::string getDisplayName (const cUnit&);

std::string getStatusStr (const cBuilding&, const cPlayer* whoWantsToKnow, const cUnitsData&);
std::string getStatusStr (const cUnit&, const cPlayer* whoWantsToKnow, const cUnitsData&);
std::string getStatusStr (const cVecicle&, const cPlayer* whoWantsToKnow, const cUnitsData&);

std::string getMessage (const cSavedReport&, const cModel&);

#endif
