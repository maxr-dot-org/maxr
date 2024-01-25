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

#ifndef game_data_player_playerbasicdataH
#define game_data_player_playerbasicdataH

#include "game/data/player/playersettings.h"
#include "utility/color.h"
#include "utility/serialization/serialization.h"
#include "utility/signal/signal.h"

#include <string>

/**
 * a structure that includes all information needed in pre-game.
 */
class cPlayerBasicData
{
public:
	static cPlayerBasicData fromSettings();

	cPlayerBasicData() = default;
	cPlayerBasicData (const sPlayerSettings&, int nr, bool defeated);
	cPlayerBasicData (const cPlayerBasicData&);
	cPlayerBasicData& operator= (const cPlayerBasicData&);

	const std::string& getName() const;
	void setName (std::string);
	const cRgbColor& getColor() const { return player.color; }
	void setColor (cRgbColor);
	int getNr() const { return nr; }
	void setNr (int index);
	void setReady (bool ready);
	bool isReady() const { return ready; }
	void setDefeated (bool defeated);
	bool isDefeated() const { return defeated; }

	bool operator== (const cPlayerBasicData&) const;
	bool operator!= (const cPlayerBasicData& rhs) const { return !(*this == rhs); }

	template <typename Archive>
	void serialize (Archive& archive)
	{
		// clang-format off
		// See https://github.com/llvm/llvm-project/issues/44312
		archive & NVP (player);
		archive & NVP (nr);
		archive & NVP (ready);
		archive & NVP (defeated);
		// clang-format on
	}

	mutable cSignal<void()> nameChanged;
	mutable cSignal<void()> numberChanged;
	mutable cSignal<void()> colorChanged;
	mutable cSignal<void()> readyChanged;
	mutable cSignal<void()> isDefeatedChanged;

private:
	sPlayerSettings player;
	int nr = 0;
	bool ready = false;
	bool defeated = false;
};

//--------------------------------------------------------------------------
inline auto byPlayerNr (int playerNr)
{
	return [=] (const cPlayerBasicData& player) { return player.getNr() == playerNr; };
}

//--------------------------------------------------------------------------
inline auto byPlayerName (const std::string& name)
{
	return [=] (const cPlayerBasicData& player) { return player.getName() == name; };
}

#endif // game_data_player_playerbasicdataH
