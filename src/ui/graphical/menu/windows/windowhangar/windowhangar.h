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

#ifndef ui_graphical_menu_windows_windowhangar_windowhangarH
#define ui_graphical_menu_windows_windowhangar_windowhangarH

#include "ui/widgets/window.h"
#include "utility/color.h"
#include "utility/signal/signal.h"
#include "utility/signal/signalconnectionmanager.h"

class cCheckBox;
class cImage;
class cLabel;
class cPlayer;
class cPushButton;
class cUnitDetails;
class cUnitListViewItemBuy;
class cUnitsData;
class cUnitUpgrade;

struct sID;

template <typename T>
class cListView;

class cWindowHangar : public cWindow
{
public:
	cWindowHangar (UniqueSurface, std::shared_ptr<const cUnitsData>, cRgbColor playerColor, int playerClan);
	cWindowHangar (UniqueSurface, std::shared_ptr<const cUnitsData>, const cPlayer&);
	~cWindowHangar();

	void retranslate() override;

	cSignal<void()> done;
	cSignal<void()> canceled;

protected:
	cUnitListViewItemBuy& addSelectionUnit (const sID& unitId);
	void setSelectedSelectionItem (const cUnitListViewItemBuy& item);

	void clearSelectionUnits();

	const cPlayer& getPlayer() const;

	void setActiveUpgrades (const cUnitUpgrade& unitUpgrades);

	virtual void setActiveUnit (const sID& unitId);

	const sID* getActiveUnit() const;

	std::shared_ptr<const cUnitsData> unitsData;

	// TODO: the following widgets should be private instead.
	// They are protect at the moment because some inheriting windows need to move/resize the widgets.
	cListView<cUnitListViewItemBuy>* selectionUnitList;

	cPushButton* okButton;
	cPushButton* backButton;
	cPushButton* selectionListUpButton;
	cPushButton* selectionListDownButton;

	cSignal<void (const cUnitListViewItemBuy&)> selectionUnitClickedSecondTime;

	cSignal<void (cUnitListViewItemBuy*)> selectionUnitSelectionChanged;

private:
	cSignalConnectionManager signalConnectionManager;

	std::unique_ptr<cPlayer> temporaryPlayer;
	const cPlayer& player;

	cImage* infoImage;
	cLabel* infoLabel;

	cUnitDetails* unitDetails;

	cCheckBox* infoTextCheckBox;

	void initialize();

	void infoCheckBoxToggled();

	void selectionUnitClicked (cUnitListViewItemBuy& unitItem);

	void okClicked();
	void backClicked();

	void handleSelectionChanged();
};

#endif // ui_graphical_menu_windows_windowhangar_windowhangarH
