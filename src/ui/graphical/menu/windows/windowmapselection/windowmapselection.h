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

#ifndef ui_graphical_menu_windows_windowmapselection_windowmapselectionH
#define ui_graphical_menu_windows_windowmapselection_windowmapselectionH

#include "ui/widgets/window.h"
#include "utility/signal/signal.h"
#include "utility/signal/signalconnectionmanager.h"

#include <array>
#include <filesystem>
#include <vector>

class cImage;
class cLabel;
class cPushButton;
class cStaticMap;

class cWindowMapSelection : public cWindow
{
public:
	cWindowMapSelection();

	void retranslate() override;

	cSignal<void (const std::filesystem::path& mapFilename)> done;

private:
	void mapClicked (int imageIndex);

	void upClicked();
	void downClicked();

	void okClicked();
	void backClicked();

	void updateUpDownLocked();

	void updateMaps();
	void loadMaps();

private:
	cSignalConnectionManager signalConnectionManager;

	static const size_t mapRows = 2;
	static const size_t mapColumns = 4;
	static const size_t mapCount = mapRows * mapColumns;

	std::array<cLabel*, mapCount> mapTitles;
	std::array<cImage*, mapCount> mapImages;

	cLabel* titleLabel = nullptr;
	cPushButton* upButton = nullptr;
	cPushButton* downButton = nullptr;
	cPushButton* okButton = nullptr;
	cPushButton* backButton = nullptr;

	std::vector<std::filesystem::path> maps;
	std::optional<std::size_t> selectedMapIndex;
	unsigned int page = 0;
};

#endif // ui_graphical_menu_windows_windowmapselection_windowmapselectionH
