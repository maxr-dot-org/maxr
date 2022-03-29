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

#ifndef ui_graphical_menu_widgets_special_reportmessagelistviewitemH
#define ui_graphical_menu_widgets_special_reportmessagelistviewitemH

#include "ui/graphical/menu/widgets/abstractlistviewitem.h"

class cImage;
class cModel;
class cSavedReport;

class cReportMessageListViewItem : public cAbstractListViewItem
{
public:
	explicit cReportMessageListViewItem (const cSavedReport&, const cModel&);

	void draw (SDL_Surface& destination, const cBox<cPosition>& clipRect) override;

	const cSavedReport& getReport() const;

protected:
	const cSavedReport& report;
};

#endif // ui_graphical_menu_widgets_special_reportmessagelistviewitemH
