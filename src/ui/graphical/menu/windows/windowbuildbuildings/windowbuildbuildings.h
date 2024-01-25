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

#ifndef ui_graphical_menu_windows_windowbuildbuildings_windowbuildbuildingsH
#define ui_graphical_menu_windows_windowbuildbuildings_windowbuildbuildingsH

#include "ui/graphical/menu/windows/windowhangar/windowhangar.h"

class cVehicle;
class cBuildSpeedHandlerWidget;
class cTurnTimeClock;
class cUnitsData;

class cWindowBuildBuildings : public cWindowHangar
{
public:
	cWindowBuildBuildings (const cVehicle& vehicle, std::shared_ptr<const cTurnTimeClock> turnTimeClock, std::shared_ptr<const cUnitsData> unitsData);

	void retranslate() override;

	const sID* getSelectedUnitId() const;
	int getSelectedBuildSpeed() const;

	cSignal<void()> donePath;

protected:
	void setActiveUnit (const sID& unitId) override;

private:
	cSignalConnectionManager signalConnectionManager;

	const cVehicle& vehicle;

	cBuildSpeedHandlerWidget* speedHandler = nullptr;
	cLabel* titleLabel = nullptr;
	cPushButton* pathButton = nullptr;

	void generateSelectionList (const cVehicle& vehicle, const cUnitsData& unitsData);

	void closeOnUnitDestruction();
};

#endif // ui_graphical_menu_windows_windowbuildbuildings_windowbuildbuildingsH
