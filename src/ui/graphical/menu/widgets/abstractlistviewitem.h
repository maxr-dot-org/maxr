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

#ifndef ui_graphical_menu_widgets_abstractlistviewitemH
#define ui_graphical_menu_widgets_abstractlistviewitemH

#include "output/video/video.h"
#include "ui/widgets/application.h"
#include "ui/widgets/clickablewidget.h"
#include "utility/signal/signal.h"

class cAbstractListViewItem : public cWidget
{
	template <typename T>
	friend class cListView;

public:
	cAbstractListViewItem() = default;
	explicit cAbstractListViewItem (const cPosition& size);

	cSignal<void (const cPosition&)> resized;

	void handleResized (const cPosition& oldSize) override;

protected:
	void select();
	void deselect();

	bool isSelected() const;

private:
	bool selected = false;
};

#endif // ui_graphical_menu_widgets_abstractlistviewitemH
