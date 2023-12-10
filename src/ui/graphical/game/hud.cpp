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

#include "hud.h"

#include "SDLutility/tosdl.h"
#include "game/data/units/unit.h"
#include "game/logic/turncounter.h"
#include "game/logic/turntimeclock.h"
#include "output/video/video.h"
#include "resources/keys.h"
#include "resources/pcx.h"
#include "settings.h"
#include "ui/graphical/game/widgets/turntimeclockwidget.h"
#include "ui/graphical/game/widgets/unitdetailshud.h"
#include "ui/graphical/game/widgets/unitrenamewidget.h"
#include "ui/graphical/game/widgets/unitvideowidget.h"
#include "ui/graphical/menu/widgets/checkbox.h"
#include "ui/graphical/menu/widgets/pushbutton.h"
#include "ui/graphical/menu/widgets/slider.h"
#include "ui/widgets/label.h"
#include "ui/widgets/lineedit.h"
#include "utility/language.h"

//------------------------------------------------------------------------------
cHud::cHud (std::shared_ptr<cAnimationTimer> animationTimer) :
	player (nullptr),
	unitsData (nullptr)
{
	surface = generateSurface();
	resize (cPosition (surface->w, surface->h));

	endButton = emplaceChild<cPushButton> (cPosition (391, 4), ePushButtonType::HudEnd, lngPack.i18n ("Others~End"), eUnicodeFontType::LatinNormal);
	endButton->addClickShortcut (KeysList.keyEndTurn);
	signalConnectionManager.connect (endButton->clicked, [this]() { endClicked(); });

	preferencesButton = emplaceChild<cPushButton> (cPosition (86, 4), ePushButtonType::HudPreferences, lngPack.i18n ("Others~Settings"), eUnicodeFontType::LatinSmallWhite);
	signalConnectionManager.connect (preferencesButton->clicked, [this]() { preferencesClicked(); });
	filesButton = emplaceChild<cPushButton> (cPosition (17, 3), ePushButtonType::HudFiles, lngPack.i18n ("Others~Files"), eUnicodeFontType::LatinSmallWhite);
	signalConnectionManager.connect (filesButton->clicked, [this]() { filesClicked(); });

	surveyButton = emplaceChild<cCheckBox> (cPosition (2, 296), lngPack.i18n ("Others~Survey"), eUnicodeFontType::LatinSmallWhite, eCheckBoxTextAnchor::Left, eCheckBoxType::HudIndex_00, false, &SoundData.SNDHudSwitch);
	surveyShortcut = &surveyButton->addClickShortcut (KeysList.keySurvey);
	signalConnectionManager.connect (surveyButton->toggled, [this]() { surveyToggled(); });

	hitsButton = emplaceChild<cCheckBox> (cPosition (57, 296), lngPack.i18n ("Others~Hitpoints_7"), eUnicodeFontType::LatinSmallWhite, eCheckBoxTextAnchor::Left, eCheckBoxType::HudIndex_01, false, &SoundData.SNDHudSwitch);
	hitsShortcut = &hitsButton->addClickShortcut (KeysList.keyHitpoints);
	signalConnectionManager.connect (hitsButton->toggled, [this]() { hitsToggled(); });

	scanButton = emplaceChild<cCheckBox> (cPosition (112, 296), lngPack.i18n ("Others~Scan"), eUnicodeFontType::LatinSmallWhite, eCheckBoxTextAnchor::Left, eCheckBoxType::HudIndex_02, false, &SoundData.SNDHudSwitch);
	scanShortcut = &scanButton->addClickShortcut (KeysList.keyScan);
	signalConnectionManager.connect (scanButton->toggled, [this]() { scanToggled(); });

	statusButton = emplaceChild<cCheckBox> (cPosition (2, 296 + 18), lngPack.i18n ("Others~Status"), eUnicodeFontType::LatinSmallWhite, eCheckBoxTextAnchor::Left, eCheckBoxType::HudIndex_10, false, &SoundData.SNDHudSwitch);
	statusShortcut = &statusButton->addClickShortcut (KeysList.keyStatus);
	signalConnectionManager.connect (statusButton->toggled, [this]() { statusToggled(); });

	ammoButton = emplaceChild<cCheckBox> (cPosition (57, 296 + 18), lngPack.i18n ("Others~Ammo"), eUnicodeFontType::LatinSmallWhite, eCheckBoxTextAnchor::Left, eCheckBoxType::HudIndex_11, false, &SoundData.SNDHudSwitch);
	ammoShortcut = &ammoButton->addClickShortcut (KeysList.keyAmmo);
	signalConnectionManager.connect (ammoButton->toggled, [this]() { ammoToggled(); });

	gridButton = emplaceChild<cCheckBox> (cPosition (112, 296 + 18), lngPack.i18n ("Others~Grid"), eUnicodeFontType::LatinSmallWhite, eCheckBoxTextAnchor::Left, eCheckBoxType::HudIndex_12, false, &SoundData.SNDHudSwitch);
	gridShortcut = &gridButton->addClickShortcut (KeysList.keyGrid);
	signalConnectionManager.connect (gridButton->toggled, [this]() { gridToggled(); });

	colorButton = emplaceChild<cCheckBox> (cPosition (2, 296 + 18 + 16), lngPack.i18n ("Others~Color"), eUnicodeFontType::LatinSmallWhite, eCheckBoxTextAnchor::Left, eCheckBoxType::HudIndex_20, false, &SoundData.SNDHudSwitch);
	colorShortcut = &colorButton->addClickShortcut (KeysList.keyColors);
	signalConnectionManager.connect (colorButton->toggled, [this]() { colorToggled(); });

	rangeButton = emplaceChild<cCheckBox> (cPosition (57, 296 + 18 + 16), lngPack.i18n ("Others~Range"), eUnicodeFontType::LatinSmallWhite, eCheckBoxTextAnchor::Left, eCheckBoxType::HudIndex_21, false, &SoundData.SNDHudSwitch);
	rangeShortcut = &rangeButton->addClickShortcut (KeysList.keyRange);
	signalConnectionManager.connect (rangeButton->toggled, [this]() { rangeToggled(); });

	fogButton = emplaceChild<cCheckBox> (cPosition (112, 296 + 18 + 16), lngPack.i18n ("Others~Fog"), eUnicodeFontType::LatinSmallWhite, eCheckBoxTextAnchor::Left, eCheckBoxType::HudIndex_22, false, &SoundData.SNDHudSwitch);
	fogShortcut = &fogButton->addClickShortcut (KeysList.keyFog);
	signalConnectionManager.connect (fogButton->toggled, [this]() { fogToggled(); });

	lockButton = emplaceChild<cCheckBox> (cPosition (32, 227), eCheckBoxType::HudLock, false, &SoundData.SNDHudSwitch);
	signalConnectionManager.connect (lockButton->toggled, [this]() { lockToggled(); });

	miniMapAttackUnitsOnlyButton = emplaceChild<cCheckBox> (cPosition (136, 413), eCheckBoxType::HudTnt, false, &SoundData.SNDHudSwitch);
	signalConnectionManager.connect (miniMapAttackUnitsOnlyButton->toggled, [this]() { miniMapAttackUnitsOnlyToggled(); });
	miniMapZoomFactorButton = emplaceChild<cCheckBox> (cPosition (136, 387), eCheckBoxType::Hud2x, false, &SoundData.SNDHudSwitch);
	signalConnectionManager.connect (miniMapZoomFactorButton->toggled, [this]() { miniMapZoomFactorToggled(); });

	auto helpButton = emplaceChild<cPushButton> (cPosition (20, 250), ePushButtonType::HudHelp);
	signalConnectionManager.connect (helpButton->clicked, [this]() { helpClicked(); });
	auto centerButton = emplaceChild<cPushButton> (cPosition (4, 227), ePushButtonType::HudCenter);
	centerButton->addClickShortcut (KeysList.keyCenterUnit);
	signalConnectionManager.connect (centerButton->clicked, [this]() { centerClicked(); });

	reportsButton = emplaceChild<cPushButton> (cPosition (101, 252), ePushButtonType::HudReport, lngPack.i18n ("Others~Log"));
	signalConnectionManager.connect (reportsButton->clicked, [this]() { reportsClicked(); });
	chatButton = emplaceChild<cCheckBox> (cPosition (51, 252), lngPack.i18n ("Others~Chat"), eUnicodeFontType::LatinSmallWhite, eCheckBoxTextAnchor::Left, eCheckBoxType::HudChat);
	signalConnectionManager.connect (chatButton->toggled, [this]() { chatToggled(); });

	auto nextButton = emplaceChild<cPushButton> (cPosition (124, 227), ePushButtonType::HudNext, ">>");
	nextButton->addClickShortcut (KeysList.keyUnitNext);
	signalConnectionManager.connect (nextButton->clicked, [this]() { nextClicked(); });
	auto prevButton = emplaceChild<cPushButton> (cPosition (60, 227), ePushButtonType::HudPrev, "<<");
	prevButton->addClickShortcut (KeysList.keyUnitPrev);
	signalConnectionManager.connect (prevButton->clicked, [this]() { prevClicked(); });
	doneButton = emplaceChild<cPushButton> (cPosition (99, 227), ePushButtonType::HudDone, lngPack.i18n ("Others~Proceed_4"));
	doneButton->addClickShortcut (KeysList.keyUnitDone);
	signalConnectionManager.connect (doneButton->clicked, [this]() { doneClicked(); });

	coordsLabel = emplaceChild<cLabel> (cBox<cPosition> (cPosition (265, getEndPosition().y() - 18), cPosition (265 + 64, getEndPosition().y() - 18 + 10)), "", eUnicodeFontType::LatinNormal, eAlignmentType::CenterHorizontal);
	unitNameLabel = emplaceChild<cLabel> (cBox<cPosition> (cPosition (343, getEndPosition().y() - 18), cPosition (343 + 212, getEndPosition().y() - 18 + 10)), "", eUnicodeFontType::LatinNormal, eAlignmentType::CenterHorizontal);
	turnLabel = emplaceChild<cLabel> (cBox<cPosition> (cPosition (471, 7), cPosition (471 + 55, 7 + 10)), "", eUnicodeFontType::LatinNormal, eAlignmentType::CenterHorizontal);
	turnTimeClockWidget = emplaceChild<cTurnTimeClockWidget> (cBox<cPosition> (cPosition (537, 7), cPosition (537 + 55, 7 + 10)));

	zoomSlider = emplaceChild<cSlider> (cBox<cPosition> (cPosition (22, 275), cPosition (22 + 126, 275 + 15)), 0, 100, eOrientationType::Horizontal, eSliderHandleType::HudZoom, eSliderType::Invisible);
	signalConnectionManager.connect (zoomSlider->valueChanged, [this]() { zoomChanged(); });
	auto zoomPlusButton = emplaceChild<cPushButton> (cBox<cPosition> (cPosition (2, 275), cPosition (2 + 15, 275 + 15)));
	signalConnectionManager.connect (zoomPlusButton->clicked, [this]() { handleZoomPlusClicked(); });
	auto zoomMinusButton = emplaceChild<cPushButton> (cBox<cPosition> (cPosition (152, 275), cPosition (152 + 15, 275 + 15)));
	signalConnectionManager.connect (zoomMinusButton->clicked, [this]() { handleZoomMinusClicked(); });

	unitVideo = emplaceChild<cUnitVideoWidget> (cBox<cPosition> (cPosition (10, 29), cPosition (10 + 125, 29 + 125)), animationTimer);
	signalConnectionManager.connect (unitVideo->clicked, [this]() {
		if (unitVideo->hasAnimation())
		{
			unitVideo->toggle();
		}
	});
	auto playButton = emplaceChild<cPushButton> (cPosition (146, 123), ePushButtonType::HudPlay);
	signalConnectionManager.connect (playButton->clicked, [this]() { unitVideo->start(); });
	auto stopButton = emplaceChild<cPushButton> (cPosition (146, 143), ePushButtonType::HudStop);
	signalConnectionManager.connect (stopButton->clicked, [this]() { unitVideo->stop(); });

	unitDetails = emplaceChild<cUnitDetailsHud> (cBox<cPosition> (cPosition (8, 171), cPosition (8 + 155, 171 + 48)));

	unitRenameWidget = emplaceChild<cUnitRenameWidget> (cPosition (12, 30), 123);
	signalConnectionManager.connect (unitRenameWidget->unitRenameTriggered, [this]() {
		if (unitRenameWidget->getUnit())
		{
			triggeredRenameUnit (*unitRenameWidget->getUnit(), unitRenameWidget->getUnitName());
		}
	});
}

//------------------------------------------------------------------------------
void cHud::retranslate()
{
	cWidget::retranslate();

	endButton->setText (lngPack.i18n ("Others~End"));
	preferencesButton->setText (lngPack.i18n ("Others~Settings"));
	filesButton->setText (lngPack.i18n ("Others~Files"));
	surveyButton->setText (lngPack.i18n ("Others~Survey"));
	hitsButton->setText (lngPack.i18n ("Others~Hitpoints_7"));
	scanButton->setText (lngPack.i18n ("Others~Scan"));
	statusButton->setText (lngPack.i18n ("Others~Status"));
	ammoButton->setText (lngPack.i18n ("Others~Ammo"));
	gridButton->setText (lngPack.i18n ("Others~Grid"));
	colorButton->setText (lngPack.i18n ("Others~Color"));
	rangeButton->setText (lngPack.i18n ("Others~Range"));
	fogButton->setText (lngPack.i18n ("Others~Fog"));
	reportsButton->setText (lngPack.i18n ("Others~Log"));
	chatButton->setText (lngPack.i18n ("Others~Chat"));
	doneButton->setText (lngPack.i18n ("Others~Proceed_4"));
}

//------------------------------------------------------------------------------
void cHud::setPlayer (std::shared_ptr<const cPlayer> player_)
{
	player = std::move (player_);
	unitRenameWidget->setPlayer (player.get(), *unitsData);
	unitDetails->setPlayer (player.get());
}

//------------------------------------------------------------------------------
void cHud::setTurnClock (std::shared_ptr<const cTurnCounter> turnClock_)
{
	turnClock = std::move (turnClock_);

	turnClockSignalConnectionManager.disconnectAll();
	if (turnClock != nullptr)
	{
		turnLabel->setText (std::to_string (turnClock->getTurn()));
		turnClockSignalConnectionManager.connect (turnClock->turnChanged, [this]() {
			turnLabel->setText (std::to_string (turnClock->getTurn()));
		});
	}
}

//------------------------------------------------------------------------------
void cHud::setTurnTimeClock (std::shared_ptr<const cTurnTimeClock> turnTimeClock)
{
	turnTimeClockWidget->setTurnTimeClock (std::move (turnTimeClock));
}

//------------------------------------------------------------------------------
void cHud::setGameSettings (std::shared_ptr<const cGameSettings> gameSettings)
{
	unitDetails->setGameSettings (std::move (gameSettings));
}

//------------------------------------------------------------------------------
bool cHud::isAt (const cPosition& position) const
{
	cBox<cPosition> hole (cPosition (panelLeftWidth, panelTopHeight), getEndPosition() - cPosition (panelRightWidth, panelBottomHeight));

	if (hole.withinOrTouches (position))
	{
		// end button reaches into the hole
		return endButton->isAt (position);
	}
	return true;
}

//------------------------------------------------------------------------------
void cHud::draw (SDL_Surface& destination, const cBox<cPosition>& clipRect)
{
	if (surface != nullptr)
	{
		auto position = toSdlRect (getArea());
		SDL_BlitSurface (surface.get(), nullptr, &destination, &position);
	}

	cWidget::draw (destination, clipRect);
}

//------------------------------------------------------------------------------
UniqueSurface cHud::generateSurface()
{
	UniqueSurface surface (SDL_CreateRGBSurface (0, Video.getResolutionX(), Video.getResolutionY(), Video.getColDepth(), 0, 0, 0, 0));

	SDL_FillRect (surface.get(), nullptr, 0x00FF00FF);
	SDL_SetColorKey (surface.get(), SDL_TRUE, 0x00FF00FF);

	const auto gfxPath = cSettings::getInstance().getGfxPath();
	{
		UniqueSurface tmpSurface (LoadPCX (gfxPath / "hud_left.pcx"));
		if (tmpSurface != nullptr)
		{
			SDL_BlitSurface (tmpSurface.get(), nullptr, surface.get(), nullptr);
		}
	}

	SDL_Rect dest;
	{
		UniqueSurface tmpSurface (LoadPCX (gfxPath / "hud_top.pcx"));
		if (tmpSurface != nullptr)
		{
			SDL_Rect src = {0, 0, Uint16 (tmpSurface->w), Uint16 (tmpSurface->h)};
			dest.x = panelLeftWidth;
			dest.y = 0;
			SDL_BlitSurface (tmpSurface.get(), &src, surface.get(), &dest);
			src.x = tmpSurface->w - 19;
			src.w = 18;
			src.h = panelTopHeight;
			dest.x = surface->w - panelTopHeight;
			SDL_BlitSurface (tmpSurface.get(), &src, surface.get(), &dest);
		}
	}

	{
		UniqueSurface tmpSurface (LoadPCX (gfxPath / "hud_right.pcx"));
		if (tmpSurface != nullptr)
		{
			SDL_Rect src = {0, 0, Uint16 (tmpSurface->w), Uint16 (tmpSurface->h)};
			dest.x = surface->w - panelRightWidth;
			dest.y = panelTopHeight;
			SDL_BlitSurface (tmpSurface.get(), &src, surface.get(), &dest);
		}
	}

	{
		UniqueSurface tmpSurface (LoadPCX (gfxPath / "hud_bottom.pcx"));
		if (tmpSurface != nullptr)
		{
			SDL_Rect src = {0, 0, Uint16 (tmpSurface->w), Uint16 (tmpSurface->h)};
			dest.x = panelLeftWidth;
			dest.y = surface->h - 24;
			SDL_BlitSurface (tmpSurface.get(), &src, surface.get(), &dest);
			src.x = tmpSurface->w - 41;
			src.w = 23;
			src.h = 24;
			dest.x = surface->w - 23;
			SDL_BlitSurface (tmpSurface.get(), &src, surface.get(), &dest);
			src.x = tmpSurface->w - 17;
			src.w = 16;
			src.h = 22;
			dest.x = panelLeftWidth - 16;
			dest.y = surface->h - 22;
			SDL_BlitSurface (tmpSurface.get(), &src, surface.get(), &dest);
		}
	}

	if (Video.getResolutionY() > 480)
	{
		UniqueSurface tmpSurface (LoadPCX (gfxPath / "logo.pcx"));
		if (tmpSurface != nullptr)
		{
			dest.x = 9;
			dest.y = Video.getResolutionY() - panelTotalHeight - 15;
			SDL_BlitSurface (tmpSurface.get(), nullptr, surface.get(), &dest);
		}
	}
	return surface;
}

//------------------------------------------------------------------------------
void cHud::setMinimalZoomFactor (float zoomFactor)
{
	zoomSlider->setMaxValue ((int) std::ceil (100. - 100. * zoomFactor));
}

//------------------------------------------------------------------------------
void cHud::setZoomFactor (float zoomFactor)
{
	zoomSlider->setValue (static_cast<int> (100. - zoomFactor * 100));
}

//------------------------------------------------------------------------------
float cHud::getZoomFactor() const
{
	return 1.f - (float) zoomSlider->getValue() / 100;
}

//------------------------------------------------------------------------------
void cHud::increaseZoomFactor (double percent)
{
	zoomSlider->increase ((int) ((zoomSlider->getMaxValue() - zoomSlider->getMinValue()) * percent));
}

//------------------------------------------------------------------------------
void cHud::decreaseZoomFactor (double percent)
{
	zoomSlider->decrease ((int) ((zoomSlider->getMaxValue() - zoomSlider->getMinValue()) * percent));
}

//------------------------------------------------------------------------------
void cHud::lockEndButton()
{
	endButton->lock();
}

//------------------------------------------------------------------------------
void cHud::unlockEndButton()
{
	endButton->unlock();
}

//------------------------------------------------------------------------------
void cHud::setSurveyActive (bool value)
{
	surveyButton->setChecked (value);
}

//------------------------------------------------------------------------------
bool cHud::getSurveyActive() const
{
	return surveyButton->isChecked();
}

//------------------------------------------------------------------------------
void cHud::setHitsActive (bool value)
{
	hitsButton->setChecked (value);
}

//------------------------------------------------------------------------------
bool cHud::getHitsActive() const
{
	return hitsButton->isChecked();
}

//------------------------------------------------------------------------------
void cHud::setScanActive (bool value)
{
	scanButton->setChecked (value);
}

//------------------------------------------------------------------------------
bool cHud::getScanActive() const
{
	return scanButton->isChecked();
}

//------------------------------------------------------------------------------
void cHud::setStatusActive (bool value)
{
	statusButton->setChecked (value);
}

//------------------------------------------------------------------------------
bool cHud::getStatusActive() const
{
	return statusButton->isChecked();
}

//------------------------------------------------------------------------------
void cHud::setAmmoActive (bool value)
{
	ammoButton->setChecked (value);
}

//------------------------------------------------------------------------------
bool cHud::getAmmoActive() const
{
	return ammoButton->isChecked();
}

//------------------------------------------------------------------------------
void cHud::setGridActive (bool value)
{
	gridButton->setChecked (value);
}

//------------------------------------------------------------------------------
bool cHud::getGridActive() const
{
	return gridButton->isChecked();
}

//------------------------------------------------------------------------------
void cHud::setColorActive (bool value)
{
	colorButton->setChecked (value);
}

//------------------------------------------------------------------------------
bool cHud::getColorActive() const
{
	return colorButton->isChecked();
}

//------------------------------------------------------------------------------
void cHud::setRangeActive (bool value)
{
	rangeButton->setChecked (value);
}

//------------------------------------------------------------------------------
bool cHud::getRangeActive() const
{
	return rangeButton->isChecked();
}

//------------------------------------------------------------------------------
void cHud::setFogActive (bool value)
{
	fogButton->setChecked (value);
}

//------------------------------------------------------------------------------
bool cHud::getFogActive() const
{
	return fogButton->isChecked();
}

//------------------------------------------------------------------------------
void cHud::setLockActive (bool value)
{
	lockButton->setChecked (value);
}

//------------------------------------------------------------------------------
bool cHud::getLockActive() const
{
	return lockButton->isChecked();
}

//------------------------------------------------------------------------------
void cHud::setChatActive (bool value)
{
	chatButton->setChecked (value);
}

//------------------------------------------------------------------------------
bool cHud::getChatActive() const
{
	return chatButton->isChecked();
}

//------------------------------------------------------------------------------
void cHud::setMiniMapZoomFactorActive (bool value)
{
	miniMapZoomFactorButton->setChecked (value);
}

//------------------------------------------------------------------------------
bool cHud::getMiniMapZoomFactorActive() const
{
	return miniMapZoomFactorButton->isChecked();
}

//------------------------------------------------------------------------------
void cHud::setMiniMapAttackUnitsOnly (bool value)
{
	miniMapAttackUnitsOnlyButton->setChecked (value);
}

//------------------------------------------------------------------------------
bool cHud::getMiniMapAttackUnitsOnly() const
{
	return miniMapAttackUnitsOnlyButton->isChecked();
}

//------------------------------------------------------------------------------
void cHud::setCoordinatesText (const std::string& text)
{
	coordsLabel->setText (text);
}

//------------------------------------------------------------------------------
void cHud::setUnitNameText (const std::string& text)
{
	unitNameLabel->setText (text);
}

//------------------------------------------------------------------------------
void cHud::startUnitVideo()
{
	unitVideo->start();
}

//------------------------------------------------------------------------------
void cHud::stopUnitVideo()
{
	unitVideo->stop();
}

//------------------------------------------------------------------------------
bool cHud::isUnitVideoPlaying() const
{
	return unitVideo->isPlaying();
}

//------------------------------------------------------------------------------
void cHud::resizeToResolution()
{
	surface = generateSurface();
	resize (cPosition (surface->w, surface->h));

	coordsLabel->moveTo (cPosition (265, getEndPosition().y() - 18));
	unitNameLabel->moveTo (cPosition (343, getEndPosition().y() - 18));
}

//------------------------------------------------------------------------------
void cHud::activateShortcuts()
{
	surveyShortcut->activate();
	hitsShortcut->activate();
	scanShortcut->activate();
	statusShortcut->activate();
	ammoShortcut->activate();
	gridShortcut->activate();
	colorShortcut->activate();
	rangeShortcut->activate();
	fogShortcut->activate();
}

//------------------------------------------------------------------------------
void cHud::deactivateShortcuts()
{
	surveyShortcut->deactivate();
	hitsShortcut->deactivate();
	scanShortcut->deactivate();
	statusShortcut->deactivate();
	ammoShortcut->deactivate();
	gridShortcut->deactivate();
	colorShortcut->deactivate();
	rangeShortcut->deactivate();
	fogShortcut->deactivate();
}

//------------------------------------------------------------------------------
void cHud::handleZoomPlusClicked()
{
	zoomSlider->decrease ((zoomSlider->getMaxValue() - zoomSlider->getMinValue()) / 6);
}

//------------------------------------------------------------------------------
void cHud::handleZoomMinusClicked()
{
	zoomSlider->increase ((zoomSlider->getMaxValue() - zoomSlider->getMinValue()) / 6);
}

void cHud::setUnitsData (std::shared_ptr<const cUnitsData> unitsData_)
{
	unitsData = unitsData_;
}

//------------------------------------------------------------------------------
void cHud::setActiveUnit (const cUnit* unit)
{
	unitRenameWidget->setUnit (unit, *unitsData);
	unitVideo->setUnit (unit);
	unitDetails->setUnit (unit);
}
