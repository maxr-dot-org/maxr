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

#ifndef gui_menu_windows_windowmapselection_windowmapselectionH
#define gui_menu_windows_windowmapselection_windowmapselectionH

#include <array>
#include <vector>
#include <string>

#include "../../../window.h"
#include "../../../../utility/signal/signalconnectionmanager.h"
#include "../../../../utility/signal/signal.h"

class cImage;
class cLabel;
class cPushButton;
class cStaticMap;

class cWindowMapSelection : public cWindow
{
public:
	cWindowMapSelection ();
	~cWindowMapSelection ();

	cSignal<void ()> done;

	std::string getSelectedMapName () const;

	bool loadSelectedMap (cStaticMap& staticMap);
private:
	cSignalConnectionManager signalConnectionManager;

	static const size_t mapRows = 2;
	static const size_t mapColumns = 4;
	static const size_t mapCount = mapRows * mapColumns;

	std::array<cLabel*, mapCount> mapTitles;
	std::array<cImage*, mapCount> mapImages;

	cPushButton* upButton;
	cPushButton* downButton;

	cPushButton* okButton;

	std::vector<std::string> maps;
	int selectedMapIndex;
	unsigned int page;

	void mapClicked (const cImage* mapImage);

	void upClicked ();
	void downClicked ();

	void okClicked ();
	void backClicked ();

	void updateUpDownLocked ();

	void updateMaps ();
	void loadMaps ();
};

#endif // gui_menu_windows_windowmapselection_windowmapselectionH
