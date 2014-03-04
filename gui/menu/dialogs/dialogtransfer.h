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

#ifndef gui_menu_dialogs_dialogtransferH
#define gui_menu_dialogs_dialogtransferH

#include "../../window.h"
#include "../../../utility/signal/signal.h"
#include "../../../utility/signal/signalconnectionmanager.h"
#include "../../../unit.h"
#include "../widgets/special/resourcebar.h"

class cUnit;
class cImage;
class cLabel;
class cResourceBar;

class cNewDialogTransfer : public cWindow
{
public:
	cNewDialogTransfer (const cUnit& sourceUnit, const cUnit& destinationUnit);

	int getTransferValue () const;

	sUnitData::eStorageResType getResourceType () const;

	cSignal<void ()> done;
private:
	cSignalConnectionManager signalConnectionManager;

	sUnitData::eStorageResType getCommonResourceType (const cUnit& sourceUnit, const cUnit& destinationUnit) const;
	eResourceBarType getResourceBarType (const cUnit& sourceUnit, const cUnit& destinationUnit) const;

	void initUnitImage (cImage& image, const cUnit& unit);
	void initCargo (int& cargo, int& maxCargo, const cUnit& sourceUnit, const cUnit& destinationUnit);

	cResourceBar* resourceBar;

	cLabel* transferLabel;

	cImage* arrowImage;

	cLabel* sourceUnitCargoLabel;
	cLabel* destinationUnitCargoLabel;

	const sUnitData::eStorageResType resourceType;

	int sourceCargo;
	int sourceMaxCargo;

	int destinationCargo;
	int destinationMaxCargo;

	void transferValueChanged ();
};

#endif // gui_menu_dialogs_dialogtransferH
