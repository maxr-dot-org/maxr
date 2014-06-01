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

#ifndef gui_game_hudH
#define gui_game_hudH

#include "../widget.h"
#include "../../maxrconfig.h"
#include "../../utility/signal/signal.h"
#include "../../utility/signal/signalconnectionmanager.h"

class cPushButton;
class cCheckBox;
class cSlider;
class cLabel;
class cUnitVideoWidget;
class cUnitDetailsHud;
class cUnitRenameWidget;
class cUnit;
class cAnimationTimer;
class cPlayer;

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

	static SDL_Surface* generateSurface ();

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

	bool getMiniMapZoomFactorActive () const;
	bool getMiniMapAttackUnitsOnly () const;

	void setTurnNumberText (const std::string& text);
	void setTurnTimeText (const std::string& text);
	void setCoordinatesText (const std::string& text);
	void setUnitNameText (const std::string& text);

	void resizeToResolution ();

	cSignal<void ()> zoomChanged;

	cSignal<void ()> surveyToggled;
	cSignal<void ()> hitsToggled;
	cSignal<void ()> scanToggled;
	cSignal<void ()> statusToggled;
	cSignal<void ()> ammoToggled;
	cSignal<void ()> gridToggled;
	cSignal<void ()> colorToggled;
	cSignal<void ()> rangeToggled;
	cSignal<void ()> fogToggled;

	cSignal<void ()> centerClicked;
	cSignal<void ()> helpClicked;

    cSignal<void ()> reportsClicked;
    cSignal<void ()> chatClicked;

	cSignal<void ()> miniMapZoomFactorToggled;
	cSignal<void ()> miniMapAttackUnitsOnlyToggled;

	cSignal<void ()> endClicked;

	cSignal<void ()> filesClicked;
	cSignal<void ()> preferencesClicked;

	cSignal<void (const cUnit&, const std::string&)> triggeredRenameUnit;

	void setActiveUnit (const cUnit* unit);

	virtual bool isAt (const cPosition& position) const MAXR_OVERRIDE_FUNCTION;

	virtual void draw () MAXR_OVERRIDE_FUNCTION;
protected:

private:
	AutoSurface surface;

	std::shared_ptr<const cPlayer> player;

	cSignalConnectionManager signalConnectionManager;

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

	cCheckBox* miniMapZoomFactorButton;
	cCheckBox* miniMapAttackUnitsOnlyButton;

	cLabel* coordsLabel;
	cLabel* unitNameLabel;
	cLabel* turnLabel;
	cLabel* timeLabel;

	cUnitRenameWidget* unitRenameWidget;

	cUnitVideoWidget* unitVideo;

	cUnitDetailsHud* unitDetails;

	void handleZoomPlusClicked ();
	void handleZoomMinusClicked ();

	void handlePreferencesClicked ();
};


#endif // gui_game_hudH
