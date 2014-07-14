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

#ifndef ui_graphical_menu_windows_windowstorage_windowstorageH
#define ui_graphical_menu_windows_windowstorage_windowstorageH

#include <array>

#include "ui/graphical/window.h"
#include "utility/signal/signalconnectionmanager.h"
#include "utility/signal/signal.h"

class cUnit;
class cVehicle;
class cTurnTimeClock;
class cPushButton;
class cLabel;
class cImage;
class cResourceBar;
class cUnitDetailsStored;

class cWindowStorage : public cWindow
{
public:
	cWindowStorage (const cUnit& unit, std::shared_ptr<const cTurnTimeClock> turnTimeClock);

	cSignal<void ()> activateAll;
	cSignal<void ()> reloadAll;
	cSignal<void ()> repairAll;
	cSignal<void ()> upgradeAll;

	cSignal<void (size_t index)> activate;
	cSignal<void (size_t index)> reload;
	cSignal<void (size_t index)> repair;
	cSignal<void (size_t index)> upgrade;
private:
	const cUnit& unit;

	cSignalConnectionManager signalConnectionManager;

	cSignalConnectionManager unitsSignalConnectionManager;

	cPushButton* upButton;
	cPushButton* downButton;

	cPushButton* activateAllButton;
	cPushButton* reloadAllButton;
	cPushButton* repairAllButton;
	cPushButton* upgradeAllButton;

	cLabel* metalBarAmountLabel;
	cResourceBar* metalBar;

	const bool canRepairReloadUpgrade;
	const bool canStorePlanes;
	const bool canStoreShips;

	const size_t columns;
	size_t page;

	static const int maxColumns = 3;
	static const int maxRows = 2;

	std::array<cPushButton*, maxColumns*maxRows> activateButtons;
	std::array<cPushButton*, maxColumns*maxRows> reloadButtons;
	std::array<cPushButton*, maxColumns*maxRows> repairButtons;
	std::array<cPushButton*, maxColumns*maxRows> upgradeButtons;

	std::array<cImage*, maxColumns*maxRows> unitImages;
	std::array<cLabel*, maxColumns*maxRows> unitNames;
	std::array<cUnitDetailsStored*, maxColumns*maxRows> unitDetails;

	void updateUnitsWidgets ();
	void updateUnitButtons (const cVehicle& unit, size_t positionIndex);
	void updateGlobalButtons ();
	void updateUpDownButtons ();

	void upClicked ();
	void downClicked ();

	void activateClicked (size_t index);
	void reloadClicked (size_t index);
	void repairClicked (size_t index);
	void upgradeClicked (size_t index);

	void activateAllClicked ();
	void reloadAllClicked ();
	void repairAllClicked ();
	void upgradeAllClicked ();

	void doneClicked ();

	void closeOnUnitDestruction ();
};

#endif // ui_graphical_menu_windows_windowstorage_windowstorageH
