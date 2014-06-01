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

#include "widgets/unitvideowidget.h"
#include "widgets/unitdetailshud.h"
#include "widgets/unitrenamewidget.h"

#include "../menu/widgets/pushbutton.h"
#include "../menu/widgets/checkbox.h"
#include "../menu/widgets/label.h"
#include "../menu/widgets/lineedit.h"
#include "../menu/widgets/slider.h"

#include "../../defines.h"
#include "../../settings.h"
#include "../../video.h"
#include "../../pcx.h"
#include "../../main.h"
#include "../../unit.h"
#include "../../keys.h"

//------------------------------------------------------------------------------
cHud::cHud (std::shared_ptr<cAnimationTimer> animationTimer) :
	player (nullptr)
{
	surface = generateSurface ();
	resize (cPosition (surface->w, surface->h));

	endButton = addChild (std::make_unique<cPushButton> (cPosition (391, 4), ePushButtonType::HudEnd, lngPack.i18n ("Text~Others~End"), FONT_LATIN_NORMAL));
	endButton->addClickShortcut (KeysList.keyEndTurn);
	signalConnectionManager.connect (endButton->clicked, [&](){ endClicked (); });

	auto preferencesButton = addChild (std::make_unique<cPushButton> (cPosition (86, 4), ePushButtonType::HudPreferences, lngPack.i18n ("Text~Others~Settings"), FONT_LATIN_SMALL_WHITE));
	signalConnectionManager.connect (preferencesButton->clicked, [&](){ preferencesClicked (); });
	auto filesButton = addChild (std::make_unique<cPushButton> (cPosition (17, 3), ePushButtonType::HudFiles, lngPack.i18n ("Text~Others~Files"), FONT_LATIN_SMALL_WHITE));
	signalConnectionManager.connect (filesButton->clicked, [&](){ filesClicked (); });

	surveyButton = addChild (std::make_unique<cCheckBox> (cPosition (2, 296), lngPack.i18n ("Text~Others~Survey"), FONT_LATIN_SMALL_WHITE, eCheckBoxTextAnchor::Left, eCheckBoxType::HudIndex_00, false, SoundData.SNDHudSwitch.get()));
	surveyButton->addClickShortcut (KeysList.keySurvey);
	signalConnectionManager.connect (surveyButton->toggled, [&](){ surveyToggled (); });
	
	hitsButton = addChild (std::make_unique<cCheckBox> (cPosition (57, 296), lngPack.i18n ("Text~Others~Hitpoints_7"), FONT_LATIN_SMALL_WHITE, eCheckBoxTextAnchor::Left, eCheckBoxType::HudIndex_01, false, SoundData.SNDHudSwitch.get ()));
	hitsButton->addClickShortcut (KeysList.keyHitpoints);
	signalConnectionManager.connect (hitsButton->toggled, [&](){ hitsToggled (); });
	
	scanButton = addChild (std::make_unique<cCheckBox> (cPosition (112, 296), lngPack.i18n ("Text~Others~Scan"), FONT_LATIN_SMALL_WHITE, eCheckBoxTextAnchor::Left, eCheckBoxType::HudIndex_02, false, SoundData.SNDHudSwitch.get ()));
	scanButton->addClickShortcut (KeysList.keyScan);
	signalConnectionManager.connect (scanButton->toggled, [&](){ scanToggled (); });
	
	statusButton = addChild (std::make_unique<cCheckBox> (cPosition (2, 296 + 18), lngPack.i18n ("Text~Others~Status"), FONT_LATIN_SMALL_WHITE, eCheckBoxTextAnchor::Left, eCheckBoxType::HudIndex_10, false, SoundData.SNDHudSwitch.get ()));
	statusButton->addClickShortcut (KeysList.keyStatus);
	signalConnectionManager.connect (statusButton->toggled, [&](){ statusToggled (); });
	
	ammoButton = addChild (std::make_unique<cCheckBox> (cPosition (57, 296 + 18), lngPack.i18n ("Text~Others~Ammo"), FONT_LATIN_SMALL_WHITE, eCheckBoxTextAnchor::Left, eCheckBoxType::HudIndex_11, false, SoundData.SNDHudSwitch.get ()));
	ammoButton->addClickShortcut (KeysList.keyAmmo);
	signalConnectionManager.connect (ammoButton->toggled, [&](){ ammoToggled (); });
	
	gridButton = addChild (std::make_unique<cCheckBox> (cPosition (112, 296 + 18), lngPack.i18n ("Text~Others~Grid"), FONT_LATIN_SMALL_WHITE, eCheckBoxTextAnchor::Left, eCheckBoxType::HudIndex_12, false, SoundData.SNDHudSwitch.get ()));
	gridButton->addClickShortcut (KeysList.keyGrid);
	signalConnectionManager.connect (gridButton->toggled, [&](){ gridToggled (); });
	
	colorButton = addChild (std::make_unique<cCheckBox> (cPosition (2, 296 + 18 + 16), lngPack.i18n ("Text~Others~Color"), FONT_LATIN_SMALL_WHITE, eCheckBoxTextAnchor::Left, eCheckBoxType::HudIndex_20, false, SoundData.SNDHudSwitch.get ()));
	colorButton->addClickShortcut (KeysList.keyColors);
	signalConnectionManager.connect (colorButton->toggled, [&](){ colorToggled (); });
	
	rangeButton = addChild (std::make_unique<cCheckBox> (cPosition (57, 296 + 18 + 16), lngPack.i18n ("Text~Others~Range"), FONT_LATIN_SMALL_WHITE, eCheckBoxTextAnchor::Left, eCheckBoxType::HudIndex_21, false, SoundData.SNDHudSwitch.get ()));
	rangeButton->addClickShortcut (KeysList.keyRange);
	signalConnectionManager.connect (rangeButton->toggled, [&](){ rangeToggled (); });
	
	fogButton = addChild (std::make_unique<cCheckBox> (cPosition (112, 296 + 18 + 16), lngPack.i18n ("Text~Others~Fog"), FONT_LATIN_SMALL_WHITE, eCheckBoxTextAnchor::Left, eCheckBoxType::HudIndex_21, false, SoundData.SNDHudSwitch.get ()));
	fogButton->addClickShortcut (KeysList.keyFog);
	signalConnectionManager.connect (fogButton->toggled, [&](){ fogToggled (); });

	auto lockButton = addChild (std::make_unique<cCheckBox> (cPosition (32, 227), eCheckBoxType::HudLock, false, SoundData.SNDHudSwitch.get ()));

	miniMapAttackUnitsOnlyButton = addChild (std::make_unique<cCheckBox> (cPosition (136, 413), eCheckBoxType::HudTnt, false, SoundData.SNDHudSwitch.get ()));
	signalConnectionManager.connect (miniMapAttackUnitsOnlyButton->toggled, [&](){ miniMapAttackUnitsOnlyToggled (); });
	miniMapZoomFactorButton = addChild (std::make_unique<cCheckBox> (cPosition (136, 387), eCheckBoxType::Hud2x, false, SoundData.SNDHudSwitch.get ()));
	signalConnectionManager.connect (miniMapZoomFactorButton->toggled, [&](){ miniMapZoomFactorToggled (); });

	//auto playersButton = addChild (std::make_unique<cCheckBox> (cPosition (136, 439), eCheckBoxType::HudPlayers, false, SoundData.SNDHudSwitch.get ()));

	auto helpButton = addChild (std::make_unique<cPushButton> (cPosition (20, 250), ePushButtonType::HudHelp));
	signalConnectionManager.connect (helpButton->clicked, [&](){ helpClicked (); });
	auto centerButton = addChild (std::make_unique<cPushButton> (cPosition (4, 227), ePushButtonType::HudCenter));
	signalConnectionManager.connect (centerButton->clicked, [&](){ centerClicked (); });

	auto reportsButton = addChild (std::make_unique<cPushButton> (cPosition (101, 252), ePushButtonType::HudReport, lngPack.i18n ("Text~Others~Log")));
	signalConnectionManager.connect (reportsButton->clicked, [&](){ reportsClicked (); });
    auto chatButton = addChild (std::make_unique<cPushButton> (cPosition (51, 252), ePushButtonType::HudChat, lngPack.i18n ("Text~Others~Chat")));
    signalConnectionManager.connect (chatButton->clicked, [&](){ chatClicked (); });

	auto nextButton = addChild (std::make_unique<cPushButton> (cPosition (124, 227), ePushButtonType::HudNext, ">>"));
	nextButton->addClickShortcut (KeysList.keyUnitNext);
	auto prevButton = addChild (std::make_unique<cPushButton> (cPosition (60, 227), ePushButtonType::HudPrev, "<<"));
	prevButton->addClickShortcut (KeysList.keyUnitPrev);
	auto doneButton = addChild (std::make_unique<cPushButton> (cPosition (99, 227), ePushButtonType::HudDone, lngPack.i18n ("Text~Others~Proceed")));
	doneButton->addClickShortcut (KeysList.keyUnitDone);

	coordsLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (cPosition (265, getEndPosition ().y () - 18), cPosition (265 + 64, getEndPosition ().y () - 18 + 10)), "", FONT_LATIN_NORMAL, eAlignmentType::CenterHorizontal));
	unitNameLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (cPosition (343, getEndPosition ().y () - 18), cPosition (343 + 212, getEndPosition ().y () - 18 + 10)), "", FONT_LATIN_NORMAL, eAlignmentType::CenterHorizontal));
	turnLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (cPosition (471, 7), cPosition (471 + 55, 7 + 10)), "", FONT_LATIN_NORMAL, eAlignmentType::CenterHorizontal));
	timeLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (cPosition (537, 7), cPosition (537 + 55, 7 + 10)), "Test4", FONT_LATIN_NORMAL, eAlignmentType::CenterHorizontal));

	zoomSlider = addChild (std::make_unique<cSlider> (cBox<cPosition> (cPosition (20, 275), cPosition (20 + 130, 275 + 15)), 0, 100, eOrientationType::Horizontal, eSliderHandleType::HudZoom, eSliderType::Invisible));
	signalConnectionManager.connect (zoomSlider->valueChanged, [&](){ zoomChanged (); });
	auto zoomPlusButton = addChild (std::make_unique<cPushButton> (cBox<cPosition> (cPosition (2, 275), cPosition (2 + 15, 275 + 15))));
	signalConnectionManager.connect (zoomPlusButton->clicked, std::bind (&cHud::handleZoomPlusClicked, this));
	auto zoomMinusButton = addChild (std::make_unique<cPushButton> (cBox<cPosition> (cPosition (152, 275), cPosition (152 + 15, 275 + 15))));
	signalConnectionManager.connect (zoomMinusButton->clicked, std::bind (&cHud::handleZoomMinusClicked, this));

	unitVideo = addChild (std::make_unique<cUnitVideoWidget> (cBox<cPosition> (cPosition (10, 29), cPosition (10 + 125, 29 + 125)), animationTimer));
	auto playButton = addChild (std::make_unique<cPushButton> (cPosition (146, 123), ePushButtonType::HudPlay));
	signalConnectionManager.connect (playButton->clicked, std::bind (&cUnitVideoWidget::start, unitVideo));
	auto stopButton = addChild (std::make_unique<cPushButton> (cPosition (146, 143), ePushButtonType::HudStop));
	signalConnectionManager.connect (stopButton->clicked, std::bind (&cUnitVideoWidget::stop, unitVideo));

	unitDetails = addChild (std::make_unique<cUnitDetailsHud> (cBox<cPosition> (cPosition (8, 171), cPosition (8 + 155, 171 + 48))));

	unitRenameWidget = addChild (std::make_unique<cUnitRenameWidget> (cPosition (12, 30), 123));
	signalConnectionManager.connect (unitRenameWidget->unitRenameTriggered, [&]()
	{
		if (unitRenameWidget->getUnit ())
		{
			triggeredRenameUnit (*unitRenameWidget->getUnit (), unitRenameWidget->getUnitName ());
		}
	});
}

//------------------------------------------------------------------------------
void cHud::setPlayer (std::shared_ptr<const cPlayer> player_)
{
	player = std::move(player_);
	unitRenameWidget->setPlayer (player.get());
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
void cHud::draw ()
{
	if (surface != nullptr)
	{
		auto position = getArea ().toSdlRect ();
		//SDL_Rect rect = {0, getEndPosition ().y () - panelBottomHeight * 2, getSize ().x (), panelBottomHeight * 2};
		SDL_BlitSurface (surface.get (), nullptr, cVideo::buffer, &position);
	}

	cWidget::draw ();
}

//------------------------------------------------------------------------------
SDL_Surface* cHud::generateSurface ()
{
	SDL_Surface* surface = SDL_CreateRGBSurface (0, Video.getResolutionX (), Video.getResolutionY (), Video.getColDepth (), 0, 0, 0, 0);

	SDL_FillRect (surface, NULL, 0x00FF00FF);
	SDL_SetColorKey (surface, SDL_TRUE, 0x00FF00FF);

	const std::string gfxPath = cSettings::getInstance ().getGfxPath () + PATH_DELIMITER;
	{
		AutoSurface tmpSurface (LoadPCX (gfxPath + "hud_left.pcx"));
		if (tmpSurface != nullptr)
		{
			SDL_BlitSurface (tmpSurface.get (), NULL, surface, NULL);
		}
	}

	SDL_Rect dest;
	{
		AutoSurface tmpSurface (LoadPCX (gfxPath + "hud_top.pcx"));
		if (tmpSurface != nullptr)
		{
			SDL_Rect src = {0, 0, Uint16 (tmpSurface->w), Uint16 (tmpSurface->h)};
			dest.x = panelLeftWidth;
			dest.y = 0;
			SDL_BlitSurface (tmpSurface.get (), &src, surface, &dest);
			src.x = 1275;
			src.w = 18;
			src.h = panelTopHeight;
			dest.x = surface->w - panelTopHeight;
			SDL_BlitSurface (tmpSurface.get (), &src, surface, &dest);
		}
	}

	{
		AutoSurface tmpSurface (LoadPCX (gfxPath + "hud_right.pcx"));
		if (tmpSurface != nullptr)
		{
			SDL_Rect src = {0, 0, Uint16 (tmpSurface->w), Uint16 (tmpSurface->h)};
			dest.x = surface->w - panelRightWidth;
			dest.y = panelTopHeight;
			SDL_BlitSurface (tmpSurface.get (), &src, surface, &dest);
		}
	}

	{
		AutoSurface tmpSurface (LoadPCX (gfxPath + "hud_bottom.pcx"));
		if (tmpSurface != nullptr)
		{
			SDL_Rect src = {0, 0, Uint16 (tmpSurface->w), Uint16 (tmpSurface->h)};
			dest.x = panelLeftWidth;
			dest.y = surface->h - 24;
			SDL_BlitSurface (tmpSurface.get (), &src, surface, &dest);
			src.x = 1275;
			src.w = 23;
			src.h = 24;
			dest.x = surface->w - 23;
			SDL_BlitSurface (tmpSurface.get (), &src, surface, &dest);
			src.x = 1299;
			src.w = 16;
			src.h = 22;
			dest.x = panelLeftWidth - 16;
			dest.y = surface->h - 22;
			SDL_BlitSurface (tmpSurface.get (), &src, surface, &dest);
		}
	}

	if (Video.getResolutionY () > 480)
	{
		AutoSurface tmpSurface (LoadPCX (gfxPath + "logo.pcx"));
		if (tmpSurface != nullptr)
		{
			dest.x = 9;
			dest.y = Video.getResolutionY () - panelTotalHeight - 15;
			SDL_BlitSurface (tmpSurface.get (), NULL, surface, &dest);
		}
	}
	return surface;
}

void cHud::setMinimalZoomFactor (float zoomFactor)
{
	zoomSlider->setMaxValue ((int)std::ceil(100. - 100. * zoomFactor));
}

//------------------------------------------------------------------------------
float cHud::getZoomFactor () const
{
	return 1.f - (float)zoomSlider->getValue () / 100;
}

//------------------------------------------------------------------------------
void cHud::increaseZoomFactor (double percent)
{
	zoomSlider->increase ((int)((zoomSlider->getMaxValue () - zoomSlider->getMinValue ()) * percent));
}

//------------------------------------------------------------------------------
void cHud::decreaseZoomFactor (double percent)
{
	zoomSlider->decrease ((int)((zoomSlider->getMaxValue () - zoomSlider->getMinValue ()) * percent));
}

//------------------------------------------------------------------------------
void cHud::lockEndButton ()
{
	endButton->lock ();
}

//------------------------------------------------------------------------------
void cHud::unlockEndButton ()
{
	endButton->unlock ();
}

//------------------------------------------------------------------------------
bool cHud::getSurveyActive () const
{
	return surveyButton->isChecked ();
}

//------------------------------------------------------------------------------
bool cHud::getHitsActive () const
{
	return hitsButton->isChecked ();
}

//------------------------------------------------------------------------------
bool cHud::getScanActive () const
{
	return scanButton->isChecked ();
}

//------------------------------------------------------------------------------
bool cHud::getStatusActive () const
{
	return statusButton->isChecked ();
}

//------------------------------------------------------------------------------
bool cHud::getAmmoActive () const
{
	return ammoButton->isChecked ();
}

//------------------------------------------------------------------------------
bool cHud::getGridActive () const
{
	return gridButton->isChecked ();
}

//------------------------------------------------------------------------------
bool cHud::getColorActive () const
{
	return colorButton->isChecked ();
}

//------------------------------------------------------------------------------
bool cHud::getRangeActive () const
{
	return rangeButton->isChecked ();
}

//------------------------------------------------------------------------------
bool cHud::getFogActive () const
{
	return fogButton->isChecked ();
}

//------------------------------------------------------------------------------
bool cHud::getMiniMapZoomFactorActive () const
{
	return miniMapZoomFactorButton->isChecked ();
}

//------------------------------------------------------------------------------
bool cHud::getMiniMapAttackUnitsOnly () const
{
	return miniMapAttackUnitsOnlyButton->isChecked ();
}

//------------------------------------------------------------------------------
void cHud::setTurnNumberText (const std::string& text)
{
	turnLabel->setText (text);
}

//------------------------------------------------------------------------------
void cHud::setTurnTimeText (const std::string& text)
{
	timeLabel->setText (text);
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
void cHud::resizeToResolution ()
{
	surface = generateSurface ();
	resize (cPosition (surface->w, surface->h));

	coordsLabel->moveTo (cPosition (265, getEndPosition ().y () - 18));
	unitNameLabel->moveTo (cPosition (343, getEndPosition ().y () - 18));
}

//------------------------------------------------------------------------------
void cHud::handleZoomPlusClicked ()
{
	zoomSlider->decrease ((zoomSlider->getMaxValue () - zoomSlider->getMinValue ()) / 6);
}

//------------------------------------------------------------------------------
void cHud::handleZoomMinusClicked ()
{
	zoomSlider->increase ((zoomSlider->getMaxValue () - zoomSlider->getMinValue ()) / 6);
}

//------------------------------------------------------------------------------
void cHud::setActiveUnit (const cUnit* unit)
{
	unitRenameWidget->setUnit (unit);
	unitVideo->setUnit (unit);
	unitDetails->setUnit (unit, player.get());
}
