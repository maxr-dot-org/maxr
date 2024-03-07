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

#ifndef ui_graphical_game_widgets_unitcontextmenuwidgetH
#define ui_graphical_game_widgets_unitcontextmenuwidgetH

#include "ui/graphical/game/control/mousemode/mousemodetype.h"
#include "ui/widgets/widget.h"
#include "utility/signal/signal.h"

class cBuilding;
class cPlayer;
class cMapView;
class cUnit;
class cVehicle;

class cUnitContextMenuWidget : public cWidget
{
public:
	cUnitContextMenuWidget() = default;

	void setUnit (const cUnit*, eMouseModeType mouseInputMode, const cPlayer*, const cMapView*);
	const cUnit* getUnit() const;
	const cBuilding* getBuilding() const;
	const cVehicle* getVehicle() const;

	static bool unitHasAttackEntry (const cUnit*, const cPlayer*);
	static bool unitHasBuildEntry (const cUnit*, const cPlayer*);
	static bool unitHasDistributeEntry (const cBuilding*, const cPlayer*);
	static bool unitHasTransferEntry (const cUnit*, const cPlayer*);
	static bool unitHasStartEntry (const cBuilding*, const cPlayer*);
	static bool unitHasAutoEntry (const cVehicle*, const cPlayer*);
	static bool unitHasStopEntry (const cUnit*, const cPlayer*);
	static bool unitHasRemoveEntry (const cVehicle*, const cPlayer*, const cMapView*);
	static bool unitHasManualFireEntry (const cUnit*, const cPlayer*);
	static bool unitHasSentryEntry (const cUnit*, const cPlayer*);
	static bool unitHasActivateEntry (const cUnit*, const cPlayer*);
	static bool unitHasLoadEntry (const cUnit*, const cPlayer*);
	static bool unitHasEnterEntry (const cVehicle*, const cPlayer*);
	static bool unitHasResearchEntry (const cBuilding*, const cPlayer*);
	static bool unitHasBuyEntry (const cBuilding*, const cPlayer*);
	static bool unitHasUpgradeThisEntry (const cBuilding*, const cPlayer*);
	static bool unitHasUpgradeAllEntry (const cBuilding*, const cPlayer*);
	static bool unitHasSelfDestroyEntry (const cBuilding*, const cPlayer*);
	static bool unitHasSupplyEntry (const cVehicle*, const cPlayer*);
	static bool unitHasRepairEntry (const cVehicle*, const cPlayer*);
	static bool unitHasLayMinesEntry (const cVehicle*, const cPlayer*);
	static bool unitHasCollectMinesEntry (const cVehicle*, const cPlayer*);
	static bool unitHasSabotageEntry (const cVehicle*, const cPlayer*);
	static bool unitHasStealEntry (const cVehicle*, const cPlayer*);
	static bool unitHasInfoEntry (const cUnit*);
	static bool unitHasDoneEntry (const cUnit*);

	cSignal<void()> attackToggled;
	cSignal<void()> buildClicked;
	cSignal<void()> distributeClicked;
	cSignal<void()> transferToggled;
	cSignal<void()> startClicked;
	cSignal<void()> autoToggled;
	cSignal<void()> stopClicked;
	cSignal<void()> removeClicked;
	cSignal<void()> manualFireToggled;
	cSignal<void()> sentryToggled;
	cSignal<void()> activateClicked;
	cSignal<void()> loadToggled;
	cSignal<void()> enterToggled;
	cSignal<void()> researchClicked;
	cSignal<void()> buyUpgradesClicked;
	cSignal<void()> upgradeThisClicked;
	cSignal<void()> upgradeAllClicked;
	cSignal<void()> selfDestroyClicked;
	cSignal<void()> supplyAmmoToggled;
	cSignal<void()> repairToggled;
	cSignal<void()> layMinesToggled;
	cSignal<void()> collectMinesToggled;
	cSignal<void()> sabotageToggled;
	cSignal<void()> stealToggled;
	cSignal<void()> infoClicked;
	cSignal<void()> doneClicked;

private:
	const cUnit* unit = nullptr;
};

#endif // ui_graphical_game_widgets_unitcontextmenuwidgetH
