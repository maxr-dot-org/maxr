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

#include "ui/widgets/widget.h"
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
class cTurnCounter;
class cTurnTimeClock;
class cGameSettings;
class cUnitsData;

class cHud : public cWidget
{
public:
	static const int panelLeftWidth = 180;
	static const int panelRightWidth = 12;
	static const int panelTotalWidth = panelLeftWidth + panelRightWidth;
	static const int panelTopHeight = 18;
	static const int panelBottomHeight = 14;
	static const int panelTotalHeight = panelTopHeight + panelBottomHeight;

	cHud (std::shared_ptr<cAnimationTimer> animationTimer);

	void retranslate() override;

	void setPlayer (std::shared_ptr<const cPlayer>);
	void setTurnClock (std::shared_ptr<const cTurnCounter>);
	void setTurnTimeClock (std::shared_ptr<const cTurnTimeClock>);
	void setGameSettings (std::shared_ptr<const cGameSettings>);

	static AutoSurface generateSurface();

	void setMinimalZoomFactor (float zoomFactor);
	void setZoomFactor (float zoomFactor);
	float getZoomFactor() const;
	void increaseZoomFactor (double percent);
	void decreaseZoomFactor (double percent);

	void lockEndButton();
	void unlockEndButton();

	void setSurveyActive (bool value);
	bool getSurveyActive() const;

	void setHitsActive (bool value);
	bool getHitsActive() const;

	void setScanActive (bool value);
	bool getScanActive() const;

	void setStatusActive (bool value);
	bool getStatusActive() const;

	void setAmmoActive (bool value);
	bool getAmmoActive() const;

	void setGridActive (bool value);
	bool getGridActive() const;

	void setColorActive (bool value);
	bool getColorActive() const;

	void setRangeActive (bool value);
	bool getRangeActive() const;

	void setFogActive (bool value);
	bool getFogActive() const;

	void setLockActive (bool value);
	bool getLockActive() const;

	void setChatActive (bool value);
	bool getChatActive() const;

	void setMiniMapZoomFactorActive (bool value);
	bool getMiniMapZoomFactorActive() const;

	void setMiniMapAttackUnitsOnly (bool value);
	bool getMiniMapAttackUnitsOnly() const;

	void setCoordinatesText (const std::string& text);
	void setUnitNameText (const std::string& text);

	void startUnitVideo();
	void stopUnitVideo();
	bool isUnitVideoPlaying() const;

	void resizeToResolution();

	void activateShortcuts();
	void deactivateShortcuts();

	mutable cSignal<void()> zoomChanged;

	mutable cSignal<void()> surveyToggled;
	mutable cSignal<void()> hitsToggled;
	mutable cSignal<void()> scanToggled;
	mutable cSignal<void()> statusToggled;
	mutable cSignal<void()> ammoToggled;
	mutable cSignal<void()> gridToggled;
	mutable cSignal<void()> colorToggled;
	mutable cSignal<void()> rangeToggled;
	mutable cSignal<void()> fogToggled;
	mutable cSignal<void()> lockToggled;
	mutable cSignal<void()> chatToggled;

	mutable cSignal<void()> centerClicked;
	mutable cSignal<void()> helpClicked;

	mutable cSignal<void()> reportsClicked;

	mutable cSignal<void()> miniMapZoomFactorToggled;
	mutable cSignal<void()> miniMapAttackUnitsOnlyToggled;

	mutable cSignal<void()> endClicked;

	mutable cSignal<void()> filesClicked;
	mutable cSignal<void()> preferencesClicked;

	mutable cSignal<void()> nextClicked;
	mutable cSignal<void()> prevClicked;
	mutable cSignal<void()> doneClicked;

	mutable cSignal<void (const cUnit&, const std::string&)> triggeredRenameUnit;

	void setActiveUnit (const cUnit*);
	void setUnitsData (std::shared_ptr<const cUnitsData>);

	bool isAt (const cPosition&) const override;
	void draw (SDL_Surface& destination, const cBox<cPosition>& clipRect) override;

private:
	void handleZoomPlusClicked();
	void handleZoomMinusClicked();
	void handlePreferencesClicked();

private:
	AutoSurface surface;

	std::shared_ptr<const cPlayer> player;
	std::shared_ptr<const cTurnCounter> turnClock;
	std::shared_ptr<const cUnitsData> unitsData;

	cSignalConnectionManager signalConnectionManager;
	cSignalConnectionManager turnClockSignalConnectionManager;

	cPushButton* endButton = nullptr;
	cPushButton* preferencesButton = nullptr;
	cPushButton* filesButton = nullptr;

	cSlider* zoomSlider = nullptr;

	cCheckBox* surveyButton = nullptr;
	cCheckBox* hitsButton = nullptr;
	cCheckBox* scanButton = nullptr;
	cCheckBox* statusButton = nullptr;
	cCheckBox* ammoButton = nullptr;
	cCheckBox* gridButton = nullptr;
	cCheckBox* colorButton = nullptr;
	cCheckBox* rangeButton = nullptr;
	cCheckBox* fogButton = nullptr;
	cCheckBox* lockButton = nullptr;
	cCheckBox* chatButton = nullptr;

	cPushButton* reportsButton = nullptr;
	cPushButton* doneButton = nullptr;

	cShortcut* surveyShortcut = nullptr;
	cShortcut* hitsShortcut = nullptr;
	cShortcut* scanShortcut = nullptr;
	cShortcut* statusShortcut = nullptr;
	cShortcut* ammoShortcut = nullptr;
	cShortcut* gridShortcut = nullptr;
	cShortcut* colorShortcut = nullptr;
	cShortcut* rangeShortcut = nullptr;
	cShortcut* fogShortcut = nullptr;

	cCheckBox* miniMapZoomFactorButton = nullptr;
	cCheckBox* miniMapAttackUnitsOnlyButton = nullptr;

	cLabel* coordsLabel = nullptr;
	cLabel* unitNameLabel = nullptr;
	cLabel* turnLabel = nullptr;
	cTurnTimeClockWidget* turnTimeClockWidget = nullptr;

	cUnitRenameWidget* unitRenameWidget = nullptr;
	cUnitVideoWidget* unitVideo = nullptr;
	cUnitDetailsHud* unitDetails = nullptr;
};

#endif // ui_graphical_game_hudH
