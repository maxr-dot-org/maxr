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

#ifndef ui_graphical_game_hudH
#define ui_graphical_game_hudH

#include "ui/graphical/widget.h"
#include "maxrconfig.h"
#include "utility/signal/signal.h"
#include "utility/signal/signalconnectionmanager.h"

class cPushButton;
class cCheckBox;
class cSlider;
class cLabel;
class cTurnTimeClockWidget;
class cUnitVideoWidget;
class cUnitDetailsHud;
class cUnitRenameWidget;
class cUnit;
class cAnimationTimer;
class cPlayer;
class cTurnClock;
class cTurnTimeClock;
class cGameSettings;

class cHud : public cWidget
{
public:
	static const int panelLeftWidth    = 180;
	static const int panelRightWidth   = 12;
	static const int panelTotalWidth   = panelLeftWidth + panelRightWidth;
	static const int panelTopHeight    = 18;
	static const int panelBottomHeight = 14;
	static const int panelTotalHeight  = panelTopHeight + panelBottomHeight;

	cHud (std::shared_ptr<cAnimationTimer> animationTimer);

	void setPlayer (std::shared_ptr<const cPlayer> player);
	void setTurnClock (std::shared_ptr<const cTurnClock> turnClock);
	void setTurnTimeClock (std::shared_ptr<const cTurnTimeClock> turnTimeClock);
	void setGameSettings (std::shared_ptr<const cGameSettings> gameSettings);

	static AutoSurface generateSurface ();

	void setMinimalZoomFactor (float zoomFactor);
	float getZoomFactor () const;
	void increaseZoomFactor (double percent);
	void decreaseZoomFactor (double percent);

	void lockEndButton ();
	void unlockEndButton ();

	bool getSurveyActive () const;
	bool getHitsActive () const;
	bool getScanActive () const;
	bool getStatusActive () const;
	bool getAmmoActive () const;
	bool getGridActive () const;
	bool getColorActive () const;
	bool getRangeActive () const;
	bool getFogActive () const;
	bool getLockActive () const;

	bool getMiniMapZoomFactorActive () const;
	bool getMiniMapAttackUnitsOnly () const;

	void setCoordinatesText (const std::string& text);
	void setUnitNameText (const std::string& text);

	void resizeToResolution ();

	mutable cSignal<void ()> zoomChanged;

	mutable cSignal<void ()> surveyToggled;
	mutable cSignal<void ()> hitsToggled;
	mutable cSignal<void ()> scanToggled;
	mutable cSignal<void ()> statusToggled;
	mutable cSignal<void ()> ammoToggled;
	mutable cSignal<void ()> gridToggled;
	mutable cSignal<void ()> colorToggled;
	mutable cSignal<void ()> rangeToggled;
	mutable cSignal<void ()> fogToggled;
	mutable cSignal<void ()> lockToggled;

	mutable cSignal<void ()> centerClicked;
	mutable cSignal<void ()> helpClicked;

	mutable cSignal<void ()> reportsClicked;
	mutable cSignal<void ()> chatClicked;

	mutable cSignal<void ()> miniMapZoomFactorToggled;
	mutable cSignal<void ()> miniMapAttackUnitsOnlyToggled;

	mutable cSignal<void ()> endClicked;

	mutable cSignal<void ()> filesClicked;
	mutable cSignal<void ()> preferencesClicked;

	mutable cSignal<void ()> nextClicked;
	mutable cSignal<void ()> prevClicked;
	mutable cSignal<void ()> doneClicked;

	mutable cSignal<void (const cUnit&, const std::string&)> triggeredRenameUnit;

	void setActiveUnit (const cUnit* unit);

	virtual bool isAt (const cPosition& position) const MAXR_OVERRIDE_FUNCTION;

	virtual void draw () MAXR_OVERRIDE_FUNCTION;
protected:

private:
	AutoSurface surface;

	std::shared_ptr<const cPlayer> player;
	std::shared_ptr<const cTurnClock> turnClock;

	cSignalConnectionManager signalConnectionManager;
	cSignalConnectionManager turnClockSignalConnectionManager;

	cPushButton* endButton;

	cSlider* zoomSlider;

	cCheckBox* surveyButton;
	cCheckBox* hitsButton;
	cCheckBox* scanButton;
	cCheckBox* statusButton;
	cCheckBox* ammoButton;
	cCheckBox* gridButton;
	cCheckBox* colorButton;
	cCheckBox* rangeButton;
	cCheckBox* fogButton;
	cCheckBox* lockButton;

	cCheckBox* miniMapZoomFactorButton;
	cCheckBox* miniMapAttackUnitsOnlyButton;

	cLabel* coordsLabel;
	cLabel* unitNameLabel;
	cLabel* turnLabel;
	cTurnTimeClockWidget* turnTimeClockWidget;

	cUnitRenameWidget* unitRenameWidget;

	cUnitVideoWidget* unitVideo;

	cUnitDetailsHud* unitDetails;

	void handleZoomPlusClicked ();
	void handleZoomMinusClicked ();

	void handlePreferencesClicked ();
};


#endif // ui_graphical_game_hudH
