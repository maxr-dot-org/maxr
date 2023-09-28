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

#ifndef game_data_reports_savedreportchatH
#define game_data_reports_savedreportchatH

#include "game/data/report/savedreport.h"

class cPlayer;

class cSavedReportChat : public cSavedReport
{
public:
	cSavedReportChat (const cPlayer& player, std::string text);
	cSavedReportChat (std::string playerName, std::string text);
	template <typename Archive, ENABLE_ARCHIVE_IN>
	explicit cSavedReportChat (Archive& archive)
	{
		serializeThis (archive);
	}

	eSavedReportType getType() const override;

	bool isAlert() const override;

	void serialize (cBinaryArchiveIn& archive) override
	{
		cSavedReport::serialize (archive);
		serializeThis (archive);
	}
	void serialize (cJsonArchiveOut& archive) override
	{
		cSavedReport::serialize (archive);
		serializeThis (archive);
	}

	int getPlayerNumber() const;
	const std::string& getText() const;
	const std::string& getPlayerName() const;

private:
	template <typename Archive>
	void serializeThis (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (playerName);
		archive & NVP (playerNumber);
		archive & NVP (text);
		// clang-format on
	}

	std::string playerName;
	int playerNumber;
	std::string text;
};

#endif // game_data_reports_savedreportchatH
