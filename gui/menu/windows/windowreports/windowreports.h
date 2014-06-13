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

#ifndef gui_menu_windows_windowreports_windowreportsH
#define gui_menu_windows_windowreports_windowreportsH

#include "../../../window.h"
#include "../../../../utility/signal/signal.h"
#include "../../../../utility/signal/signalconnectionmanager.h"

class cCheckBox;
class cPushButton;
class cFrame;
class cReportUnitListViewItem;
class cReportDisadvantagesListViewItem;
class cReportMessageListViewItem;
template<typename> class cListView;
template<typename, typename> class cPlot;

class cPlayer;
class cCasualtiesTracker;
class cUnit;
class cSavedReport;
class cTurnClock;
class cGameSettings;
struct sUnitData;

class cWindowReports : public cWindow
{
public:
	cWindowReports (std::vector<std::shared_ptr<const cPlayer>> players,
					std::shared_ptr<const cPlayer> localPlayer,
					std::shared_ptr<const cCasualtiesTracker> casualties,
					std::shared_ptr<const cTurnClock> turnClock,
					std::shared_ptr<const cGameSettings> gameSettings);

	cSignal<void (cUnit&)> unitClickedSecondTime;
	cSignal<void (const cSavedReport&)> reportClickedSecondTime;
private:
	cSignalConnectionManager signalConnectionManager;

	cCheckBox* unitsRadioButton;
	cCheckBox* disadvantagesRadioButton;
	cCheckBox* scoreRadioButton;
	cCheckBox* reportsRadioButton;

	cCheckBox* planesCheckBox;
	cCheckBox* groundCheckBox;
	cCheckBox* seaCheckBox;
	cCheckBox* stationaryCheckBox;

	cCheckBox* produceCheckBox;
	cCheckBox* fightCheckBox;
	cCheckBox* damagedCheckBox;
	cCheckBox* stealthCheckBox;

	cPushButton* upButton;
	cPushButton* downButton;

	cFrame* unitsFrame;
	cListView<cReportUnitListViewItem>* unitsList;

	cFrame* disadvantagesFrame;
	cListView<cReportDisadvantagesListViewItem>* disadvantagesList;

	cFrame* scoreFrame;
	cPlot<int, int>* scorePlot;

	cFrame* reportsFrame;
	cListView<cReportMessageListViewItem>* reportsList;

	std::vector<std::shared_ptr<const cPlayer>> players;

	std::shared_ptr<const cPlayer> localPlayer;

	std::shared_ptr<const cCasualtiesTracker> casualties;
	std::shared_ptr<const cTurnClock> turnClock;
	std::shared_ptr<const cGameSettings> gameSettings;

	bool unitListDirty;
	bool disadvantagesListDirty;
	bool reportsListDirty;

	bool checkFilter (const sUnitData& data) const;

	void handleFilterChanged ();

	void rebuildUnitList ();
	void rebuildDisadvantagesList ();
	void rebuildReportsList ();
	void initializeScorePlot ();

	void updateActiveFrame ();

	void upPressed ();
	void downPressed ();

	void handleUnitClicked (cReportUnitListViewItem& item);
	void handleReportClicked (cReportMessageListViewItem& item);
};

#endif // gui_menu_windows_windowreports_windowreportsH
