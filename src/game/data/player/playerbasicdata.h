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

#include <string>

#include "game/data/player/playersettings.h"
#include "game/serialization/serialization.h"
#include "utility/color.h"
#include "utility/signal/signal.h"

/**
 * a structure that includes all information needed in pre-game.
 */
class cPlayerBasicData
{
public:
	static cPlayerBasicData fromSettings();

	cPlayerBasicData();
	cPlayerBasicData (const sPlayerSettings&, int nr, bool defeated);
	cPlayerBasicData (const cPlayerBasicData&);
	cPlayerBasicData& operator= (const cPlayerBasicData&);

	const std::string& getName() const;
	void setName (std::string);
	const cRgbColor& getColor() const { return player.color; }
	void setColor (cRgbColor);
	int getNr() const;
	void setNr (int index);
	void setReady (bool ready);
	bool isReady() const;
	void setDefeated (bool defeated);
	bool isDefeated() const;

	bool operator == (const cPlayerBasicData&) const;
	bool operator != (const cPlayerBasicData& rhs) const { return !(*this == rhs); }

	mutable cSignal<void()> nameChanged;
	mutable cSignal<void()> numberChanged;
	mutable cSignal<void()> colorChanged;
	mutable cSignal<void()> readyChanged;
	mutable cSignal<void()> isDefeatedChanged;

	template <typename Archive>
	void serialize (Archive& archive)
	{
		archive & NVP (player);
		archive & NVP (nr);
		archive & NVP (ready);
		archive & NVP (defeated);
	}
private:
	sPlayerSettings player;
	int nr;
	bool ready;
	bool defeated;
};

//--------------------------------------------------------------------------
inline auto byPlayerNr (int playerNr)
{
	return [=](const cPlayerBasicData& player){ return player.getNr() == playerNr; };
}

//--------------------------------------------------------------------------
inline auto byPlayerName (const std::string& name)
{
	return [=](const cPlayerBasicData& player){ return player.getName() == name; };
}

#endif // game_data_player_playerbasicdataH
