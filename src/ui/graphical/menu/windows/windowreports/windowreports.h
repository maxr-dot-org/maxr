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

#ifndef ui_graphical_menu_windows_windowreports_windowreportsH
#define ui_graphical_menu_windows_windowreports_windowreportsH

#include "ui/widgets/window.h"
#include "utility/signal/signal.h"
#include "utility/signal/signalconnectionmanager.h"

class cCasualtiesTracker;
class cCheckBox;
class cFrame;
class cLabel;
class cModel;
class cPlayer;
class cPushButton;
class cReportDisadvantagesListViewItem;
class cReportUnitListViewItem;
class cReportMessageListViewItem;
class cSavedReport;
class cStaticUnitData;
class cTurnCounter;
class cTurnTimeClock;
class cUnit;

template <typename>
class cListView;
template <typename, typename>
class cPlot;

class cWindowReports : public cWindow
{
public:
	cWindowReports (const cModel&,
	                std::shared_ptr<const cPlayer> localPlayer,
	                const std::vector<std::unique_ptr<cSavedReport>>& reports);

	void retranslate() override;

	cSignal<void (cUnit&)> unitClickedSecondTime;
	cSignal<void (const cSavedReport&)> reportClickedSecondTime;

private:
	bool checkFilter (const cUnit& unit) const;
	bool checkFilter (const cStaticUnitData& data) const;

	void handleFilterChanged();

	void rebuildUnitList();
	void rebuildDisadvantagesList();
	void rebuildReportsList();
	void initializeScorePlot();

	void updateActiveFrame();

	void upPressed();
	void downPressed();

	void handleUnitClicked (cReportUnitListViewItem& item);
	void handleReportClicked (cReportMessageListViewItem& item);

private:
	cSignalConnectionManager signalConnectionManager;

	cLabel* includedLabel = nullptr;
	cPushButton* doneButton = nullptr;
	cLabel* limitedToLabel = nullptr;
	cLabel* victoryLabel = nullptr;
	std::vector<cLabel*> ecosphereLabels;

	cCheckBox* unitsRadioButton = nullptr;
	cCheckBox* disadvantagesRadioButton = nullptr;
	cCheckBox* scoreRadioButton = nullptr;
	cCheckBox* reportsRadioButton = nullptr;

	cCheckBox* planesCheckBox = nullptr;
	cCheckBox* groundCheckBox = nullptr;
	cCheckBox* seaCheckBox = nullptr;
	cCheckBox* stationaryCheckBox = nullptr;

	cCheckBox* produceCheckBox = nullptr;
	cCheckBox* fightCheckBox = nullptr;
	cCheckBox* damagedCheckBox = nullptr;
	cCheckBox* stealthCheckBox = nullptr;

	cPushButton* upButton = nullptr;
	cPushButton* downButton = nullptr;

	cFrame* unitsFrame = nullptr;
	cListView<cReportUnitListViewItem>* unitsList = nullptr;

	cFrame* disadvantagesFrame = nullptr;
	cListView<cReportDisadvantagesListViewItem>* disadvantagesList = nullptr;

	cFrame* scoreFrame = nullptr;
	cPlot<int, int>* scorePlot = nullptr;

	cFrame* reportsFrame = nullptr;
	cListView<cReportMessageListViewItem>* reportsList = nullptr;

	const cModel& model;

	std::shared_ptr<const cPlayer> localPlayer;

	//std::shared_ptr<const cTurnCounter> turnClock;
	const std::vector<std::unique_ptr<cSavedReport>>& reports;

	bool unitListDirty = true;
	bool disadvantagesListDirty = true;
	bool reportsListDirty = true;
};

#endif
