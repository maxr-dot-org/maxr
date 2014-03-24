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

#ifndef gui_game_widgets_gamemessagelistviewitemH
#define gui_game_widgets_gamemessagelistviewitemH

#include <chrono>

#include "../../menu/widgets/abstractlistviewitem.h"
#include "../../../autosurface.h"

class cLabel;

class cGameMessageListViewItem : public cAbstractListViewItem
{
public:
	cGameMessageListViewItem (int width, const std::string& message, bool alert);

	std::chrono::steady_clock::time_point getCreationTime () const;

	virtual void draw () MAXR_OVERRIDE_FUNCTION;
private:
	cLabel* messageLabel;
	
	AutoSurface redShadow;

	std::chrono::steady_clock::time_point creationTime;
};

#endif // gui_game_widgets_gamemessagelistviewitemH
