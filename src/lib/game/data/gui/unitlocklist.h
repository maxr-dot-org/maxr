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

#ifndef game_data_gui_unitlocklistH
#define game_data_gui_unitlocklistH

#include "utility/signal/signalconnectionmanager.h"

#include <utility>
#include <vector>

class cUnit;
class cMapFieldView;
class cPlayer;

class cUnitLockList
{
public:
	cUnitLockList() = default;

	void setPlayer (const cPlayer*);

	void toggleLockAt (const cMapFieldView&);

	size_t getLockedUnitsCount() const;
	const cUnit* getLockedUnit (size_t index) const;

	void unlockAll();

	void lockUnit (const cUnit&);

private:
	std::vector<std::pair<const cUnit*, cSignalConnectionManager>> lockedUnits;

	const cPlayer* player = nullptr;
};

#endif
