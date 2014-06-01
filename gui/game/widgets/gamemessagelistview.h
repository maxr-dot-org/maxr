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

#ifndef gui_game_widgets_gamemessagelistviewH
#define gui_game_widgets_gamemessagelistviewH

#include "../../widget.h"
#include "gamemessagelistviewitem.h"

template<typename>
class cListView;

class cGameMessageListView : public cWidget
{
public:
	explicit cGameMessageListView (const cBox<cPosition>& area);

	void addMessage (const std::string& message, eGameMessageListViewItemBackgroundColor backgroundColor = eGameMessageListViewItemBackgroundColor::DarkGray);

	void removeOldMessages ();

	virtual bool isAt (const cPosition& position) const MAXR_OVERRIDE_FUNCTION;

	virtual void handleResized (const cPosition& oldSize) MAXR_OVERRIDE_FUNCTION;
private:
	cListView<cGameMessageListViewItem>* listView;

	const std::chrono::seconds maximalDisplayTime;
};

#endif // gui_game_widgets_gamemessagelistviewH
