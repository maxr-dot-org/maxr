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

#ifndef ui_graphical_menu_windows_windowresourcedistribution_windowresourcedistributionH
#define ui_graphical_menu_windows_windowresourcedistribution_windowresourcedistributionH

#include <array>

#include "ui/graphical/window.h"
#include "utility/signal/signalconnectionmanager.h"
#include "utility/signal/signal.h"
#include "game/data/base/base.h"

class cResourceBar;
class cLabel;
class cTurnTimeClock;

class cWindowResourceDistribution : public cWindow
{
public:
	cWindowResourceDistribution (const cBuilding& building, std::shared_ptr<const cTurnTimeClock> turnTimeClock);

	int getMetalProduction();
	int getOilProduction();
	int getGoldProduction();

	cSignal<void ()> done;
private:
	cSignalConnectionManager signalConnectionManager;

	std::unique_ptr<cSubBase> subBase;
	const cBuilding& building;

	std::array<cResourceBar*, 3> metalBars;
	std::array<cResourceBar*, 3> oilBars;
	std::array<cResourceBar*, 3> goldBars;

	std::array<cResourceBar*, 3> noneBars;

	std::array<cLabel*, 3> metalLabels;
	std::array<cLabel*, 3> oilLabels;
	std::array<cLabel*, 3> goldLabels;

	std::string secondBarText (int prod, int need);

	void setBarLabels();
	void setBarValues();

	void handleMetalChanged();
	void handleOilChanged();
	void handleGoldChanged();

	void closeOnUnitDestruction();
	void updateOnSubbaseDestruction();
};

#endif // ui_graphical_menu_windows_windowresourcedistribution_windowresourcedistributionH
