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

#ifndef ui_graphical_menu_windows_windowload_windowloadH
#define ui_graphical_menu_windows_windowload_windowloadH

#include "ui/widgets/window.h"
#include "utility/signal/signal.h"
#include "utility/signal/signalconnectionmanager.h"

#include <array>
#include <functional>
#include <vector>

class cLabel;
class cPushButton;
class cSaveGameInfo;
class cSaveSlotWidget;
class cTurnTimeClock;

class cWindowLoad : public cWindow
{
public:
	explicit cWindowLoad (std::shared_ptr<const cTurnTimeClock> = nullptr, std::function<std::vector<cSaveGameInfo>()> = {});
	~cWindowLoad();

	void retranslate() override;

	void update();

	cSignal<void (const cSaveGameInfo&)> load;

protected:
	virtual void handleSlotClicked (size_t index);
	virtual void handleSlotDoubleClicked (size_t index);

	void selectSlot (size_t slotIndex, bool makeRenameable);

	int getSelectedSaveNumber() const;

	cSaveGameInfo* getSaveFile (int saveNumber);

	cSaveSlotWidget* getSaveSlotFromSaveNumber (size_t saveNumber);
	cSaveSlotWidget& getSaveSlot (size_t slotIndex);

private:
	void loadSaves();
	void updateSlots();

	void handleDownClicked();
	void handleUpClicked();

	void handleLoadClicked();

private:
	static const size_t rows = 5;
	static const size_t columns = 2;
	static const size_t maximalDisplayedSaves = 100;

	cSignalConnectionManager signalConnectionManager;

	cLabel* titleLabel = nullptr;
	cPushButton* loadButton = nullptr;
	cPushButton* backButton = nullptr;

	std::array<cSaveSlotWidget*, rows * columns> saveSlots;

	int page = 0;
	const int lastPage;

	int selectedSaveNumber = -1;
	std::string selectedOriginalName;

	std::function<std::vector<cSaveGameInfo>()> saveGamesGetter;
	std::vector<cSaveGameInfo> saveGames;
};

#endif // ui_graphical_menu_windows_windowload_windowloadH
