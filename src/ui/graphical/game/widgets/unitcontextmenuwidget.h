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
#include "ui/graphical/widget.h"
#include "utility/signal/signal.h"

class cUnit;
class cPlayer;
class cMap;

class cUnitContextMenuWidget : public cWidget
{
public:
	cUnitContextMenuWidget();

	void setUnit (const cUnit* unit, eMouseModeType mouseInputMode, const cPlayer* player, const cMap* dynamicMap);
	const cUnit* getUnit();

	static bool unitHasAttackEntry (const cUnit* unit, const cPlayer* player, const cMap* dynamicMap);
	static bool unitHasBuildEntry (const cUnit* unit, const cPlayer* player, const cMap* dynamicMap);
	static bool unitHasDistributeEntry (const cUnit* unit, const cPlayer* player, const cMap* dynamicMap);
	static bool unitHasTransferEntry (const cUnit* unit, const cPlayer* player, const cMap* dynamicMap);
	static bool unitHasStartEntry (const cUnit* unit, const cPlayer* player, const cMap* dynamicMap);
	static bool unitHasAutoEntry (const cUnit* unit, const cPlayer* player, const cMap* dynamicMap);
	static bool unitHasStopEntry (const cUnit* unit, const cPlayer* player, const cMap* dynamicMap);
	static bool unitHasRemoveEntry (const cUnit* unit, const cPlayer* player, const cMap* dynamicMap);
	static bool unitHasManualFireEntry (const cUnit* unit, const cPlayer* player, const cMap* dynamicMap);
	static bool unitHasSentryEntry (const cUnit* unit, const cPlayer* player, const cMap* dynamicMap);
	static bool unitHasActivateEntry (const cUnit* unit, const cPlayer* player, const cMap* dynamicMap);
	static bool unitHasLoadEntry (const cUnit* unit, const cPlayer* player, const cMap* dynamicMap);
	static bool unitHasResearchEntry (const cUnit* unit, const cPlayer* player, const cMap* dynamicMap);
	static bool unitHasBuyEntry (const cUnit* unit, const cPlayer* player, const cMap* dynamicMap);
	static bool unitHasUpgradeThisEntry (const cUnit* unit, const cPlayer* player, const cMap* dynamicMap);
	static bool unitHasUpgradeAllEntry (const cUnit* unit, const cPlayer* player, const cMap* dynamicMap);
	static bool unitHasSelfDestroyEntry (const cUnit* unit, const cPlayer* player, const cMap* dynamicMap);
	static bool unitHasSupplyEntry (const cUnit* unit, const cPlayer* player, const cMap* dynamicMap);
	static bool unitHasRepairEntry (const cUnit* unit, const cPlayer* player, const cMap* dynamicMap);
	static bool unitHasLayMinesEntry (const cUnit* unit, const cPlayer* player, const cMap* dynamicMap);
	static bool unitHasCollectMinesEntry (const cUnit* unit, const cPlayer* player, const cMap* dynamicMap);
	static bool unitHasSabotageEntry (const cUnit* unit, const cPlayer* player, const cMap* dynamicMap);
	static bool unitHasStealEntry (const cUnit* unit, const cPlayer* player, const cMap* dynamicMap);
	static bool unitHasInfoEntry (const cUnit* unit, const cPlayer* player, const cMap* dynamicMap);
	static bool unitHasDoneEntry (const cUnit* unit, const cPlayer* player, const cMap* dynamicMap);

	cSignal<void ()> attackToggled;
	cSignal<void ()> buildClicked;
	cSignal<void ()> distributeClicked;
	cSignal<void ()> transferToggled;
	cSignal<void ()> startClicked;
	cSignal<void ()> autoToggled;
	cSignal<void ()> stopClicked;
	cSignal<void ()> removeClicked;
	cSignal<void ()> manualFireToggled;
	cSignal<void ()> sentryToggled;
	cSignal<void ()> activateClicked;
	cSignal<void ()> loadToggled;
	cSignal<void ()> researchClicked;
	cSignal<void ()> buyUpgradesClicked;
	cSignal<void ()> upgradeThisClicked;
	cSignal<void ()> upgradeAllClicked;
	cSignal<void ()> selfDestroyClicked;
	cSignal<void ()> supplyAmmoToggled;
	cSignal<void ()> repairToggled;
	cSignal<void ()> layMinesToggled;
	cSignal<void ()> collectMinesToggled;
	cSignal<void ()> sabotageToggled;
	cSignal<void ()> stealToggled;
	cSignal<void ()> infoClicked;
	cSignal<void ()> doneClicked;
private:
	const cUnit* unit;
};

#endif // ui_graphical_game_widgets_unitcontextmenuwidgetH
