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

#ifndef ui_graphical_menu_widgets_comboboxH
#define ui_graphical_menu_widgets_comboboxH

#include <string>

#include "ui/graphical/widget.h"
#include "utility/autosurface.h"
#include "utility/signal/signal.h"
#include "utility/signal/signalconnectionmanager.h"

class cCheckBox;
class cLineEdit;
class cTextListViewItem;
template<typename> class cListView;

class cComboBox : public cWidget
{
public:
	explicit cComboBox (const cBox<cPosition>& area);

	void setMaxVisibleItems (size_t count);

	void addItem (std::string text);

	void removeItem (size_t index);

	size_t getItemsCount() const;
	const std::string& getItem (size_t index) const;

	void clearItems();

	const std::string& getSelectedText() const;
	void setSelectedIndex (size_t index);

	void draw (SDL_Surface& destination, const cBox<cPosition>& clipRect) override;
private:
	cSignalConnectionManager signalConnectionManager;

	AutoSurface listViewBackground;
	AutoSurface lineEditBackground;

	cListView<cTextListViewItem>* listView;
	cCheckBox* downButton;
	cLineEdit* lineEdit;

	size_t maxVisibleItems;

	void updateListViewSize();

	void updateLineEditBackground();
	void updateListViewBackground();
};

#endif // ui_graphical_menu_widgets_comboboxH
