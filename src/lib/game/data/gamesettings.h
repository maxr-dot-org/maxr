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

#ifndef ui_graphical_menu_windows_windowgamesettings_gamesettingsH
#define ui_graphical_menu_windows_windowgamesettings_gamesettingsH

#include "utility/serialization/serialization.h"

#include <chrono>

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

class cGameSettings
{
public:
	static const int defaultCreditsNone = 0;
	static const int defaultCreditsLow = 50;
	static const int defaultCreditsLimited = 100;
	static const int defaultCreditsNormal = 150;
	static const int defaultCreditsHigh = 200;
	static const int defaultCreditsMore = 250;

	static const unsigned int defaultVictoryTurnsOptions[3];
	static const unsigned int defaultVictoryPointsOptions[3];

	static const std::chrono::seconds defaultTurnLimitOption0;
	static const std::chrono::seconds defaultTurnLimitOption1;
	static const std::chrono::seconds defaultTurnLimitOption2;
	static const std::chrono::seconds defaultTurnLimitOption3;
	static const std::chrono::seconds defaultTurnLimitOption4;
	static const std::chrono::seconds defaultTurnLimitOption5;

	static const std::chrono::seconds defaultEndTurnDeadlineOption0;
	static const std::chrono::seconds defaultEndTurnDeadlineOption1;
	static const std::chrono::seconds defaultEndTurnDeadlineOption2;
	static const std::chrono::seconds defaultEndTurnDeadlineOption3;
	static const std::chrono::seconds defaultEndTurnDeadlineOption4;
	static const std::chrono::seconds defaultEndTurnDeadlineOption5;

	cGameSettings() = default;
	cGameSettings (const cGameSettings&) = default;

	cGameSettings& operator= (const cGameSettings&) = default;

	uint32_t getChecksum (uint32_t crc) const;

	template <ArchiveInOrOut Archive>
	void serialize (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (alienEnabled);
		archive & NVP (bridgeheadType);
		archive & NVP (clansEnabled);
		archive & NVP (gameType);
		archive & NVP (goldAmount);
		archive & NVP (metalAmount);
		archive & NVP (oilAmount);
		archive & NVP (resourceDensity);
		archive & NVP (startCredits);
		archive & NVP (turnEndDeadline);
		archive & NVP (turnEndDeadlineActive);
		archive & NVP (turnLimit);
		archive & NVP (turnLimitActive);
		archive & NVP (victoryConditionType);
		archive & NVP (victoryPoints);
		archive & NVP (victoryTurns);
		// clang-format on
	}

public:
	eGameSettingsResourceAmount metalAmount = eGameSettingsResourceAmount::Normal;
	eGameSettingsResourceAmount oilAmount = eGameSettingsResourceAmount::Normal;
	eGameSettingsResourceAmount goldAmount = eGameSettingsResourceAmount::Normal;

	eGameSettingsResourceDensity resourceDensity = eGameSettingsResourceDensity::Normal;

	eGameSettingsBridgeheadType bridgeheadType = eGameSettingsBridgeheadType::Definite;

	eGameSettingsGameType gameType = eGameSettingsGameType::Simultaneous;

	bool clansEnabled = true;

	bool alienEnabled = true;
	unsigned int startCredits = defaultCreditsNormal;

	eGameSettingsVictoryCondition victoryConditionType = eGameSettingsVictoryCondition::Death;
	unsigned int victoryTurns = defaultVictoryTurnsOptions[2];
	unsigned int victoryPoints = defaultVictoryPointsOptions[2];

	std::chrono::seconds turnEndDeadline = defaultEndTurnDeadlineOption5;
	bool turnEndDeadlineActive = true;

	std::chrono::seconds turnLimit = defaultTurnLimitOption5;
	bool turnLimitActive = true;
};

#endif // ui_graphical_menu_windows_windowgamesettings_gamesettingsH
