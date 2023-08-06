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

#include "gamegui.h"

#include "game/data/gui/gameguistate.h"
#include "game/data/map/map.h"
#include "game/data/player/player.h"
#include "game/data/report/savedreportchat.h"
#include "game/data/report/savedreportsimple.h"
#include "game/data/report/savedreportunit.h"
#include "game/data/report/special/savedreporthostcommand.h"
#include "game/data/units/building.h"
#include "game/data/units/vehicle.h"
#include "game/logic/client.h"
#include "game/logic/movejob.h"
#include "game/logic/turncounter.h"
#include "input/keyboard/keyboard.h"
#include "input/mouse/cursor/mousecursorsimple.h"
#include "input/mouse/mouse.h"
#include "resources/buildinguidata.h"
#include "resources/keys.h"
#include "resources/sound.h"
#include "resources/uidata.h"
#include "resources/vehicleuidata.h"
#include "ui/graphical/game/animations/animationtimer.h"
#include "ui/graphical/game/hud.h"
#include "ui/graphical/game/widgets/chatbox.h"
#include "ui/graphical/game/widgets/chatboxplayerlistviewitem.h"
#include "ui/graphical/game/widgets/debugoutputwidget.h"
#include "ui/graphical/game/widgets/gamemapwidget.h"
#include "ui/graphical/game/widgets/gamemessagelistview.h"
#include "ui/graphical/game/widgets/hudpanels.h"
#include "ui/graphical/game/widgets/minimapwidget.h"
#include "ui/graphical/game/widgets/unitcontextmenuwidget.h"
#include "ui/graphical/menu/widgets/special/lobbychatboxlistviewitem.h"
#include "ui/sound/effects/soundeffect.h"
#include "ui/sound/effects/soundeffectunit.h"
#include "ui/sound/effects/soundeffectvoice.h"
#include "ui/sound/game/unitreport.h"
#include "ui/sound/soundmanager.h"
#include "ui/translations.h"
#include "ui/widgets/application.h"
#include "ui/widgets/label.h"
#include "utility/indexiterator.h"
#include "utility/language.h"
#include "utility/log.h"
#include "utility/random.h"

#include <iomanip>
#include <iostream>
#include <sstream>

//------------------------------------------------------------------------------
cGameGui::cGameGui (std::shared_ptr<const cStaticMap> staticMap_, std::shared_ptr<cSoundManager> soundManager_, std::shared_ptr<cAnimationTimer> animationTimer_, std::shared_ptr<const cFrameCounter> frameCounter) :
	cWindow (nullptr),
	animationTimer (std::move (animationTimer_)),
	soundManager (std::move (soundManager_)),
	staticMap (std::move (staticMap_)),
	mouseScrollDirection (0, 0),
	openPanelOnActivation (true)
{
	auto hudOwning = std::make_unique<cHud> (animationTimer);

	resize (hudOwning->getSize());

	gameMap = emplaceChild<cGameMapWidget> (cBox<cPosition> (cPosition (cHud::panelLeftWidth, cHud::panelTopHeight), getEndPosition() - cPosition (cHud::panelRightWidth, cHud::panelBottomHeight)), staticMap, animationTimer, soundManager, frameCounter);

	messageList = emplaceChild<cGameMessageListView> (cBox<cPosition> (cPosition (cHud::panelLeftWidth + 4, cHud::panelTopHeight + 7), cPosition (getEndPosition().x() - cHud::panelRightWidth - 4, cHud::panelTopHeight + 200)));

	hud = addChild (std::move (hudOwning));

	hud->setMinimalZoomFactor (gameMap->computeMinimalZoomFactor());

	chatBox = emplaceChild<cChatBox<cLobbyChatBoxListViewItem, cChatBoxPlayerListViewItem>> (cBox<cPosition> (cPosition (cHud::panelLeftWidth + 4, getEndPosition().y() - cHud::panelBottomHeight - 12 - 100), getEndPosition() - cPosition (cHud::panelRightWidth + 4, cHud::panelBottomHeight + 12)));

	miniMap = emplaceChild<cMiniMapWidget> (cBox<cPosition> (cPosition (15, 356), cPosition (15 + 112, 356 + 112)), staticMap);

	debugOutput = emplaceChild<cDebugOutputWidget> (cBox<cPosition> (cPosition (cHud::panelLeftWidth + 4, cHud::panelTopHeight + 7), cPosition (getEndPosition().x() - cHud::panelRightWidth - 8, getEndPosition().y() - cHud::panelBottomHeight - 8)));
	debugOutput->setGameMap (gameMap);
	debugOutput->disable();

	hudPanels = emplaceChild<cHudPanels> (getPosition(), getSize().y(), animationTimer);

	auto font = cUnicodeFont::font.get();
	primiaryInfoLabel = emplaceChild<cLabel> (cBox<cPosition> (cPosition (cHud::panelLeftWidth, 235), cPosition (getEndPosition().x() - cHud::panelRightWidth, 235 + font->getFontHeight (eUnicodeFontType::LatinBig))), "", eUnicodeFontType::LatinBig, toEnumFlag (eAlignmentType::CenterHorizontal) | eAlignmentType::Top);
	primiaryInfoLabel->disable();
	primiaryInfoLabel->hide();

	additionalInfoLabel = emplaceChild<cLabel> (cBox<cPosition> (cPosition (cHud::panelLeftWidth, 235 + font->getFontHeight (eUnicodeFontType::LatinBig)), cPosition (getEndPosition().x() - cHud::panelRightWidth, 235 + font->getFontHeight (eUnicodeFontType::LatinBig) + font->getFontHeight (eUnicodeFontType::LatinNormal))), "", eUnicodeFontType::LatinNormal, toEnumFlag (eAlignmentType::CenterHorizontal) | eAlignmentType::Top);
	additionalInfoLabel->disable();
	additionalInfoLabel->hide();

	signalConnectionManager.connect (hudPanels->opened, [this]() {
		hudPanels->disable();
		hudPanels->hide();
	});

	hud->activateShortcuts();
	gameMap->deactivateUnitCommandShortcuts();

	signalConnectionManager.connect (hud->zoomChanged, [this]() { gameMap->setZoomFactor (hud->getZoomFactor(), true); });

	signalConnectionManager.connect (hud->surveyToggled, [this]() { gameMap->setDrawSurvey (hud->getSurveyActive()); });
	signalConnectionManager.connect (hud->hitsToggled, [this]() { gameMap->setDrawHits (hud->getHitsActive()); });
	signalConnectionManager.connect (hud->scanToggled, [this]() { gameMap->setDrawScan (hud->getScanActive()); });
	signalConnectionManager.connect (hud->statusToggled, [this]() { gameMap->setDrawStatus (hud->getStatusActive()); });
	signalConnectionManager.connect (hud->ammoToggled, [this]() { gameMap->setDrawAmmo (hud->getAmmoActive()); });
	signalConnectionManager.connect (hud->gridToggled, [this]() { gameMap->setDrawGrid (hud->getGridActive()); });
	signalConnectionManager.connect (hud->colorToggled, [this]() { gameMap->setDrawColor (hud->getColorActive()); });
	signalConnectionManager.connect (hud->rangeToggled, [this]() { gameMap->setDrawRange (hud->getRangeActive()); });
	signalConnectionManager.connect (hud->fogToggled, [this]() { gameMap->setDrawFog (hud->getFogActive()); });
	signalConnectionManager.connect (hud->lockToggled, [this]() { gameMap->setLockActive (hud->getLockActive()); });

	signalConnectionManager.connect (hud->helpClicked, [this]() { gameMap->toggleHelpMode(); });
	signalConnectionManager.connect (hud->chatToggled, [this]() {
		if (hud->getChatActive())
		{
			chatBox->show();
			chatBox->enable();
		}
		else
		{
			chatBox->hide();
			chatBox->disable();
		}
	});
	hud->setChatActive (true);

	signalConnectionManager.connect (hud->miniMapZoomFactorToggled, [this]() { miniMap->setZoomFactor (hud->getMiniMapZoomFactorActive() ? 2 : 1); });
	signalConnectionManager.connect (hud->miniMapAttackUnitsOnlyToggled, [this]() { miniMap->setAttackUnitsUnly (hud->getMiniMapAttackUnitsOnly()); });

	signalConnectionManager.connect (gameMap->scrolled, [this]() { resetMiniMapViewWindow(); });
	signalConnectionManager.connect (gameMap->zoomFactorChanged, [this]() { resetMiniMapViewWindow(); });
	signalConnectionManager.connect (gameMap->tileUnderMouseChanged, [this] (const cPosition& tilePosition) {
		updateHudCoordinates (tilePosition);
		updateHudUnitName (tilePosition);
	});

	signalConnectionManager.connect (gameMap->mouseFocusReleased, [this]() {
		auto mouse = getActiveMouse();
		if (mouse)
		{
			if (hud->isAt (mouse->getPosition()))
				mouse->setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::Hand));
			else if (chatBox->isAt (mouse->getPosition()))
				mouse->setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::Hand));
			else
				gameMap->updateMouseCursor (*mouse);
		}
	});

	signalConnectionManager.connect (gameMap->getUnitSelection().mainSelectionChanged, [this]() {
		auto unit = gameMap->getUnitSelection().getSelectedUnit();
		hud->setActiveUnit (unit);
		updateSelectedUnitSound();
		connectSelectedUnit();

		if (unit == nullptr)
		{
			hud->activateShortcuts();
		}
		else
		{
			hud->deactivateShortcuts();
		}

		if (!player) return;

		if (unit && unit->getOwner() == player.get()) makeReport (*soundManager, getCurrentState(), *unit);
	});

	signalConnectionManager.connect (miniMap->focus, [this] (const cPosition& position) { gameMap->centerAt (position); });

	signalConnectionManager.connect (animationTimer->triggered10msCatchUp, [this]() {
		if (mouseScrollDirection != cPosition (0, 0))
		{
			gameMap->scroll (mouseScrollDirection);
		}
	});
	signalConnectionManager.connect (animationTimer->triggered400ms, [this]() {
		messageList->removeOldMessages();
	});

	terminated.connect ([this]() {
		stopSelectedUnitSound();
	});

	initShortcuts();

	signalConnectionManager.connect (Video.resolutionChanged, [this]() { handleResolutionChange(); });

	signalConnectionManager.connect (Video.screenShotTaken, [this] (const std::filesystem::path& path) {
		messageList->addMessage (lngPack.i18n ("Comp~Screenshot_Done", path.u8string()));
	});
}

//------------------------------------------------------------------------------
void cGameGui::setMapView (std::shared_ptr<const cMapView> mapView_)
{
	miniMap->setMapView (mapView_);
	gameMap->setMapView (mapView_);

	mapView = mapView_;
}

//------------------------------------------------------------------------------
void cGameGui::setPlayer (std::shared_ptr<const cPlayer> player_)
{
	player = std::move (player_);
	gameMap->setPlayer (player);
	hud->setPlayer (player);

	messageList->clear();
}

//------------------------------------------------------------------------------
void cGameGui::setPlayers (std::vector<std::shared_ptr<const cPlayer>> players)
{
	chatBox->clearPlayers();
	for (const auto& player : players)
	{
		chatBox->addPlayerEntry (std::make_unique<cChatBoxPlayerListViewItem> (*player));
	}
}

//------------------------------------------------------------------------------
void cGameGui::setTurnClock (std::shared_ptr<const cTurnCounter> turnClock)
{
	hud->setTurnClock (turnClock);
}

//------------------------------------------------------------------------------
void cGameGui::setTurnTimeClock (std::shared_ptr<const cTurnTimeClock> turnTimeClock)
{
	hud->setTurnTimeClock (turnTimeClock);
}

//------------------------------------------------------------------------------
void cGameGui::setGameSettings (std::shared_ptr<const cGameSettings> gameSettings)
{
	hud->setGameSettings (gameSettings);
}

//------------------------------------------------------------------------------
void cGameGui::setUnitsData (std::shared_ptr<const cUnitsData> unitsData)
{
	gameMap->setUnitsData (unitsData);
	hud->setUnitsData (unitsData);
}
//------------------------------------------------------------------------------
cHud& cGameGui::getHud()
{
	return *hud;
}

//------------------------------------------------------------------------------
const cHud& cGameGui::getHud() const
{
	return *hud;
}

//------------------------------------------------------------------------------
cGameMapWidget& cGameGui::getGameMap()
{
	return *gameMap;
}

//------------------------------------------------------------------------------
const cGameMapWidget& cGameGui::getGameMap() const
{
	return *gameMap;
}

//------------------------------------------------------------------------------
cMiniMapWidget& cGameGui::getMiniMap()
{
	return *miniMap;
}

//------------------------------------------------------------------------------
const cMiniMapWidget& cGameGui::getMiniMap() const
{
	return *miniMap;
}

//------------------------------------------------------------------------------
cChatBox<cLobbyChatBoxListViewItem, cChatBoxPlayerListViewItem>& cGameGui::getChatBox()
{
	return *chatBox;
}

//------------------------------------------------------------------------------
const cChatBox<cLobbyChatBoxListViewItem, cChatBoxPlayerListViewItem>& cGameGui::getChatBox() const
{
	return *chatBox;
}

//------------------------------------------------------------------------------
cGameMessageListView& cGameGui::getGameMessageList()
{
	return *messageList;
}

//------------------------------------------------------------------------------
const cGameMessageListView& cGameGui::getGameMessageList() const
{
	return *messageList;
}

//------------------------------------------------------------------------------
cDebugOutputWidget& cGameGui::getDebugOutput()
{
	return *debugOutput;
}

//------------------------------------------------------------------------------
void cGameGui::setInfoTexts (const std::string& primiaryText, const std::string& additionalText)
{
	primiaryInfoLabel->setText (primiaryText);
	if (primiaryText.empty())
		primiaryInfoLabel->hide();
	else
		primiaryInfoLabel->show();

	additionalInfoLabel->setText (additionalText);
	if (additionalText.empty())
		additionalInfoLabel->hide();
	else
		additionalInfoLabel->show();
}

//------------------------------------------------------------------------------
void cGameGui::exit()
{
	panelSignalConnectionManager.disconnectAll();

	panelSignalConnectionManager.connect (hudPanels->closed, [this]() {
		close();
	});
	startClosePanel();
}

//------------------------------------------------------------------------------
cGameGuiState cGameGui::getCurrentState() const
{
	cGameGuiState state;
	state.mapPosition = gameMap->getMapCenterOffset();

	state.mapZoomFactor = hud->getZoomFactor();
	state.surveyActive = hud->getSurveyActive();
	state.hitsActive = hud->getHitsActive();
	state.scanActive = hud->getScanActive();
	state.statusActive = hud->getStatusActive();
	state.ammoActive = hud->getAmmoActive();
	state.gridActive = hud->getGridActive();
	state.colorActive = hud->getColorActive();
	state.rangeActive = hud->getRangeActive();
	state.fogActive = hud->getFogActive();
	state.lockActive = hud->getLockActive();
	state.miniMapZoomFactorActive = hud->getMiniMapZoomFactorActive();
	state.miniMapAttackUnitsOnly = hud->getMiniMapAttackUnitsOnly();
	state.unitVideoPlaying = hud->isUnitVideoPlaying();
	state.chatActive = hud->getChatActive();

	state.setSelectedUnits (gameMap->getUnitSelection());
	state.setLockedUnits (gameMap->getUnitLockList());

	state.currentTurnResearchAreasFinished = gameMap->currentTurnResearchAreasFinished;

	return state;
}

//------------------------------------------------------------------------------
void cGameGui::restoreState (const cGameGuiState& state)
{
	gameMap->centerAt (state.mapPosition);

	hud->setZoomFactor (state.mapZoomFactor);
	hud->setSurveyActive (state.surveyActive);
	hud->setHitsActive (state.hitsActive);
	hud->setScanActive (state.scanActive);
	hud->setStatusActive (state.statusActive);
	hud->setAmmoActive (state.ammoActive);
	hud->setGridActive (state.gridActive);
	hud->setColorActive (state.colorActive);
	hud->setRangeActive (state.rangeActive);
	hud->setFogActive (state.fogActive);
	hud->setLockActive (state.lockActive);
	hud->setMiniMapZoomFactorActive (state.miniMapZoomFactorActive);
	hud->setMiniMapAttackUnitsOnly (state.miniMapAttackUnitsOnly);
	hud->setChatActive (state.chatActive);
	if (state.unitVideoPlaying)
		hud->startUnitVideo();
	else
		hud->stopUnitVideo();

	gameMap->currentTurnResearchAreasFinished = state.currentTurnResearchAreasFinished;

	gameMap->getUnitSelection().deselectUnits();
	gameMap->getUnitLockList().unlockAll();

	auto selectedUnitIds = state.getSelectedUnitIds();
	auto lockedUnitIds = state.getLockedUnitIds();

	if (mapView)
	{
		// TODO: this may be very inefficient!
		//       we go over all map fields to find the units on the map that need to be selected.
		//       we may should think about a more efficient way to find specific units on the map
		for (auto i = makeIndexIterator (cPosition (0, 0), mapView->getSize()); i.hasMore(); i.next())
		{
			if (selectedUnitIds.empty() && lockedUnitIds.empty()) break;

			const auto& field = mapView->getField (*i);

			std::vector<cUnit*> fieldUnits = field.getUnits();

			for (auto* fieldUnit : fieldUnits)
			{
				for (auto k = selectedUnitIds.begin(); k != selectedUnitIds.end();)
				{
					if (fieldUnit->iID == *k)
					{
						gameMap->getUnitSelection().selectUnit (*fieldUnit, true);
						k = selectedUnitIds.erase (k);
					}
					else
					{
						++k;
					}
				}
				for (auto k = lockedUnitIds.begin(); k != lockedUnitIds.end();)
				{
					if (fieldUnit->iID == *k)
					{
						gameMap->getUnitLockList().lockUnit (*fieldUnit);
						k = lockedUnitIds.erase (k);
					}
					else
					{
						++k;
					}
				}
			}
		}
	}
}

//------------------------------------------------------------------------------
void cGameGui::updateHudCoordinates (const cPosition& tilePosition)
{
	std::stringstream coordsString;
	coordsString << std::setw (3) << std::setfill ('0') << tilePosition.x() << "-" << std::setw (3) << std::setfill ('0') << tilePosition.y();
	hud->setCoordinatesText (coordsString.str());
}

//------------------------------------------------------------------------------
void cGameGui::updateHudUnitName (const cPosition& tilePosition)
{
	std::string unitNameString;
	if (mapView && (!player || player->canSeeAt (tilePosition)))
	{
		const auto& field = mapView->getField (tilePosition);

		cUnit* unit = nullptr;
		if (field.getVehicle() != nullptr)
			unit = field.getVehicle();
		else if (field.getPlane() != nullptr)
			unit = field.getPlane();
		else if (field.getTopBuilding() != nullptr)
			unit = field.getTopBuilding();
		else if (field.getBaseBuilding() != nullptr && !field.getBaseBuilding()->isRubble())
			unit = field.getBaseBuilding();

		if (unit != nullptr && unit->getOwner())
		{
			// FIXME: string may be too long.
			unitNameString = getDisplayName (*unit) + " (" + unit->getOwner()->getName() + ")";
		}
	}
	hud->setUnitNameText (unitNameString);
}

//------------------------------------------------------------------------------
void cGameGui::connectSelectedUnit()
{
	selectedUnitConnectionManager.disconnectAll();

	const auto selectedUnit = gameMap->getUnitSelection().getSelectedUnit();

	if (!selectedUnit) return;

	selectedUnitConnectionManager.connect (selectedUnit->workingChanged, [selectedUnit, this]() {
		stopSelectedUnitSound();
		if (selectedUnit->data.getId().isABuilding())
		{
			const auto& building = static_cast<const cBuilding&> (*selectedUnit);
			auto& uiData = UnitsUiData.getBuildingUI (building);

			if (building.isUnitWorking())
				soundManager->playSound (std::make_shared<cSoundEffectUnit> (eSoundEffectType::EffectStartWork, uiData.Start, building));
			else
				soundManager->playSound (std::make_shared<cSoundEffectUnit> (eSoundEffectType::EffectStopWork, uiData.Stop, building));
		}
		updateSelectedUnitIdleSound();
	});

	selectedUnitConnectionManager.connect (selectedUnit->buildingChanged, [this]() {
		updateSelectedUnitIdleSound();
	});

	selectedUnitConnectionManager.connect (selectedUnit->clearingChanged, [this]() {
		updateSelectedUnitIdleSound();
	});

	if (selectedUnit->isAVehicle())
	{
		const auto selectedVehicle = static_cast<cVehicle*> (selectedUnit);
		selectedUnitConnectionManager.connect (selectedVehicle->movingChanged, [selectedVehicle, this]() {
			if (selectedVehicle == gameMap->getUnitSelection().getSelectedVehicle())
			{
				if (selectedVehicle->isUnitMoving())
				{
					updateSelectedUnitMoveSound (true);
				}
				else
				{
					if (mapView)
					{
						const auto building = mapView->getField (selectedVehicle->getPosition()).getBaseBuilding();
						bool water = mapView->isWater (selectedVehicle->getPosition());
						if (selectedVehicle->getStaticUnitData().factorGround > 0 && building && (building->getStaticUnitData().surfacePosition == eSurfacePosition::Base || building->getStaticUnitData().surfacePosition == eSurfacePosition::AboveBase || building->getStaticUnitData().surfacePosition == eSurfacePosition::AboveSea)) water = false;

						stopSelectedUnitSound();
						auto* uiData = UnitsUiData.getVehicleUI (selectedVehicle->getStaticUnitData().ID);

						if (water && selectedVehicle->getStaticUnitData().factorSea > 0)
							soundManager->playSound (std::make_shared<cSoundEffectUnit> (eSoundEffectType::EffectStopMove, uiData->StopWater, *selectedVehicle));
						else
							soundManager->playSound (std::make_shared<cSoundEffectUnit> (eSoundEffectType::EffectStopMove, uiData->Stop, *selectedVehicle));

						updateSelectedUnitIdleSound();
					}
				}
			}
		});
		selectedUnitConnectionManager.connect (selectedVehicle->positionChanged, [selectedVehicle, this]() {
			if (selectedVehicle->isUnitMoving() && selectedVehicle == gameMap->getUnitSelection().getSelectedVehicle())
			{
				if (!mapView) return;

				const auto building = mapView->getField (selectedVehicle->getPosition()).getBaseBuilding();
				bool water = mapView->isWater (selectedVehicle->getPosition());
				if (selectedVehicle->getStaticUnitData().factorGround > 0 && building && (building->getStaticUnitData().surfacePosition == eSurfacePosition::Base || building->getStaticUnitData().surfacePosition == eSurfacePosition::AboveBase || building->getStaticUnitData().surfacePosition == eSurfacePosition::AboveSea))
				{
					water = false;
				}
				auto* uiData = UnitsUiData.getVehicleUI (selectedVehicle->getStaticUnitData().ID);
				if ((water && *selectedUnitSoundLoop->getSound() == uiData->Drive) || (!water && *selectedUnitSoundLoop->getSound() == uiData->DriveWater))
				{
					updateSelectedUnitMoveSound (false);
				}
			}
		});
		selectedUnitConnectionManager.connect (selectedVehicle->moveJobBlocked, [selectedVehicle, this]() {
			if (selectedVehicle == gameMap->getUnitSelection().getSelectedVehicle())
			{
				soundManager->playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceNoPath, getRandom (VoiceData.VOINoPath)));
			}
		});
	}
}

//------------------------------------------------------------------------------
bool cGameGui::handleMouseMoved (cApplication& application, cMouse& mouse, const cPosition& offset)
{
	const auto currentMousePosition = mouse.getPosition();
	const auto mouseLastPosition = currentMousePosition - offset;
	if (hud->isAt (currentMousePosition) && !hud->isAt (mouseLastPosition))
	{
		hud->setCoordinatesText ("");
		hud->setUnitNameText ("");
	}
	if (chatBox->isAt (currentMousePosition) && !chatBox->isAt (mouseLastPosition))
	{
		hud->setCoordinatesText ("");
		hud->setUnitNameText ("");
	}

	const int scrollFrameWidth = 5;

	mouseScrollDirection = 0;
	if (currentMousePosition.x() <= scrollFrameWidth)
		mouseScrollDirection.x() = -cSettings::getInstance().getScrollSpeed() / 4;
	else if (currentMousePosition.x() >= getEndPosition().x() - scrollFrameWidth)
		mouseScrollDirection.x() = +cSettings::getInstance().getScrollSpeed() / 4;
	if (currentMousePosition.y() <= scrollFrameWidth)
		mouseScrollDirection.y() = -cSettings::getInstance().getScrollSpeed() / 4;
	else if (currentMousePosition.y() >= getEndPosition().y() - scrollFrameWidth)
		mouseScrollDirection.y() = +cSettings::getInstance().getScrollSpeed() / 4;

	if (mouseScrollDirection.x() > 0 && mouseScrollDirection.y() == 0)
		mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::ArrowRight));
	else if (mouseScrollDirection.x() < 0 && mouseScrollDirection.y() == 0)
		mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::ArrowLeft));
	else if (mouseScrollDirection.x() == 0 && mouseScrollDirection.y() > 0)
		mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::ArrowDown));
	else if (mouseScrollDirection.x() == 0 && mouseScrollDirection.y() < 0)
		mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::ArrowUp));
	else if (mouseScrollDirection.x() > 0 && mouseScrollDirection.y() > 0)
		mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::ArrowRightDown));
	else if (mouseScrollDirection.x() > 0 && mouseScrollDirection.y() < 0)
		mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::ArrowRightUp));
	else if (mouseScrollDirection.x() < 0 && mouseScrollDirection.y() > 0)
		mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::ArrowLeftDown));
	else if (mouseScrollDirection.x() < 0 && mouseScrollDirection.y() < 0)
		mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::ArrowLeftUp));
	else if (hud->isAt (currentMousePosition))
		mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::Hand));
	else if (chatBox->isAt (currentMousePosition))
		mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::Hand));

	return cWindow::handleMouseMoved (application, mouse, offset);
}

//------------------------------------------------------------------------------
bool cGameGui::handleMouseWheelMoved (cApplication& application, cMouse& mouse, const cPosition& amount)
{
	const auto oldZoomFactor = gameMap->getZoomFactor();
	bool consumed = false;
	if (amount.y() > 0)
	{
		hud->decreaseZoomFactor (0.05);
		consumed = true;
	}
	else if (amount.y() < 0)
	{
		hud->increaseZoomFactor (0.05);
		consumed = true;
	}

	// scroll so that the zoom has been performed to center the mouse
	const auto newZoomFactor = gameMap->getZoomFactor();
	if (newZoomFactor != oldZoomFactor)
	{
		cPosition scrollOffset;

		const auto oldScreenPixelX = gameMap->getSize().x() / oldZoomFactor;
		const auto newScreenPixelX = gameMap->getSize().x() / newZoomFactor;
		scrollOffset.x() = (int) ((oldScreenPixelX - newScreenPixelX) * (mouse.getPosition().x() - gameMap->getPosition().x()) / gameMap->getSize().x() - (oldScreenPixelX - newScreenPixelX) / 2);

		const auto oldScreenPixelY = gameMap->getSize().y() / oldZoomFactor;
		const auto newScreenPixelY = gameMap->getSize().y() / newZoomFactor;
		scrollOffset.y() = (int) ((oldScreenPixelY - newScreenPixelY) * (mouse.getPosition().y() - gameMap->getPosition().y()) / gameMap->getSize().y() - (oldScreenPixelY - newScreenPixelY) / 2);

		gameMap->scroll (scrollOffset);
	}

	return consumed;
}

//------------------------------------------------------------------------------
void cGameGui::handleActivated (cApplication& application, bool firstTime)
{
	cWindow::handleActivated (application, firstTime);

	auto mouse = getActiveMouse();
	if (mouse)
	{
		if (hud->isAt (mouse->getPosition()))
			mouse->setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::Hand));
		else if (chatBox->isAt (mouse->getPosition()))
			mouse->setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::Hand));
		else
			gameMap->updateMouseCursor (*mouse);
	}

	if (openPanelOnActivation)
	{
		startOpenPanel();
		openPanelOnActivation = false;
	}
}

//------------------------------------------------------------------------------
void cGameGui::handleDeactivated (cApplication& application, bool removed)
{
	cWindow::handleDeactivated (application, removed);
}

//------------------------------------------------------------------------------
bool cGameGui::wantsCentered() const
{
	return false;
}

//------------------------------------------------------------------------------
std::unique_ptr<cMouseCursor> cGameGui::getDefaultCursor() const
{
	return nullptr;
}

//------------------------------------------------------------------------------
void cGameGui::startOpenPanel()
{
	hudPanels->open();
}

//------------------------------------------------------------------------------
void cGameGui::startClosePanel()
{
	hudPanels->show();
	hudPanels->enable();

	hudPanels->close();
}

//------------------------------------------------------------------------------
void cGameGui::resetMiniMapViewWindow()
{
	miniMap->setViewWindow (gameMap->getDisplayedMapArea());
}

//------------------------------------------------------------------------------
void cGameGui::updateSelectedUnitSound()
{
	auto selectedUnit = gameMap->getUnitSelection().getSelectedVehicle();
	if (selectedUnit && selectedUnit->isUnitMoving())
	{
		updateSelectedUnitMoveSound (false);
	}
	else
	{
		updateSelectedUnitIdleSound();
	}
}

//------------------------------------------------------------------------------
void cGameGui::updateSelectedUnitIdleSound()
{
	auto selectedUnit = gameMap->getUnitSelection().getSelectedUnit();
	if (selectedUnit == nullptr)
	{
		stopSelectedUnitSound();
	}
	else if (selectedUnit->isABuilding())
	{
		const auto& building = static_cast<const cBuilding&> (*selectedUnit);
		auto& uiData = UnitsUiData.getBuildingUI (building);

		if (building.isUnitWorking())
		{
			startSelectedUnitSound (building, uiData.Running);
		}
		else
		{
			startSelectedUnitSound (building, uiData.Wait);
		}
	}
	else
	{
		const auto& vehicle = static_cast<const cVehicle&> (*selectedUnit);
		auto* uiData = UnitsUiData.getVehicleUI (vehicle.getStaticUnitData().ID);

		const cBuilding* building = mapView ? mapView->getField (vehicle.getPosition()).getBaseBuilding() : nullptr;
		bool water = staticMap->isWater (vehicle.getPosition());
		if (vehicle.getStaticUnitData().factorGround > 0 && building && (building->getStaticUnitData().surfacePosition == eSurfacePosition::Base || building->getStaticUnitData().surfacePosition == eSurfacePosition::AboveBase || building->getStaticUnitData().surfacePosition == eSurfacePosition::AboveSea)) water = false;

		if (vehicle.isUnitBuildingABuilding() && (vehicle.getBuildTurns() || player.get() != vehicle.getOwner()))
		{
			startSelectedUnitSound (vehicle, SoundData.SNDBuilding);
		}
		else if (vehicle.isUnitClearing())
		{
			startSelectedUnitSound (vehicle, SoundData.SNDClearing);
		}
		else if (water && vehicle.getStaticUnitData().factorSea > 0)
		{
			startSelectedUnitSound (vehicle, uiData->WaitWater);
		}
		else
		{
			startSelectedUnitSound (vehicle, uiData->Wait);
		}
	}
}

//------------------------------------------------------------------------------
void cGameGui::updateSelectedUnitMoveSound (bool startedNew)
{
	auto selectedVehicle = gameMap->getUnitSelection().getSelectedVehicle();
	if (!selectedVehicle) return;
	if (!mapView) return;

	const auto& vehicle = *selectedVehicle;
	auto* uiData = UnitsUiData.getVehicleUI (vehicle.getStaticUnitData().ID);

	const auto building = mapView->getField (vehicle.getPosition()).getBaseBuilding();
	bool water = mapView->isWater (vehicle.getPosition());
	if (vehicle.getStaticUnitData().factorGround > 0 && building && (building->getStaticUnitData().surfacePosition == eSurfacePosition::Base || building->getStaticUnitData().surfacePosition == eSurfacePosition::AboveBase || building->getStaticUnitData().surfacePosition == eSurfacePosition::AboveSea)) water = false;
	stopSelectedUnitSound();

	if (startedNew)
	{
		if (water && vehicle.getStaticUnitData().factorSea != 0)
			soundManager->playSound (std::make_shared<cSoundEffectUnit> (eSoundEffectType::EffectStartMove, uiData->StartWater, vehicle));
		else
			soundManager->playSound (std::make_shared<cSoundEffectUnit> (eSoundEffectType::EffectStartMove, uiData->Start, vehicle));
	}

	if (water && vehicle.getStaticUnitData().factorSea != 0)
		startSelectedUnitSound (vehicle, uiData->DriveWater);
	else
		startSelectedUnitSound (vehicle, uiData->Drive);
}

//------------------------------------------------------------------------------
void cGameGui::startSelectedUnitSound (const cUnit& unit, const cSoundChunk& sound)
{
	stopSelectedUnitSound();
	selectedUnitSoundLoop = std::make_shared<cSoundEffectUnit> (eSoundEffectType::EffectUnitSound, sound, unit);
	soundManager->playSound (selectedUnitSoundLoop, true);
}

//------------------------------------------------------------------------------
void cGameGui::stopSelectedUnitSound()
{
	if (selectedUnitSoundLoop)
	{
		selectedUnitSoundLoop->stop();
		selectedUnitSoundLoop = nullptr;
	}
}

//------------------------------------------------------------------------------
template <typename Action>
void cGameGui::addShortcut (cKeySequence key, Action action)
{
	auto shortcut = std::make_unique<cShortcut> (key);
	signalConnectionManager.connect (shortcut->triggered, action);

	cWidget::addShortcut (std::move (shortcut));
}

//------------------------------------------------------------------------------
void cGameGui::initShortcuts()
{
	addShortcut (KeysList.keyScroll1, [this]() { gameMap->scroll (cPosition (-cSettings::getInstance().getScrollSpeed(), +cSettings::getInstance().getScrollSpeed())); });
	addShortcut (KeysList.keyScroll3, [this]() { gameMap->scroll (cPosition (+cSettings::getInstance().getScrollSpeed(), +cSettings::getInstance().getScrollSpeed())); });
	addShortcut (KeysList.keyScroll7, [this]() { gameMap->scroll (cPosition (-cSettings::getInstance().getScrollSpeed(), -cSettings::getInstance().getScrollSpeed())); });
	addShortcut (KeysList.keyScroll9, [this]() { gameMap->scroll (cPosition (+cSettings::getInstance().getScrollSpeed(), -cSettings::getInstance().getScrollSpeed())); });

	auto scrollDown = [this]() { gameMap->scroll (cPosition (0, +cSettings::getInstance().getScrollSpeed())); };
	addShortcut (KeysList.keyScroll2a, scrollDown);
	addShortcut (KeysList.keyScroll2b, scrollDown);

	auto scrollLeft = [this]() { gameMap->scroll (cPosition (-cSettings::getInstance().getScrollSpeed(), 0)); };
	addShortcut (KeysList.keyScroll4a, scrollLeft);
	addShortcut (KeysList.keyScroll4b, scrollLeft);

	auto scrollRight = [this]() { gameMap->scroll (cPosition (+cSettings::getInstance().getScrollSpeed(), 0)); };
	addShortcut (KeysList.keyScroll6a, scrollRight);
	addShortcut (KeysList.keyScroll6b, scrollRight);

	auto scrollUp = [this]() { gameMap->scroll (cPosition (0, -cSettings::getInstance().getScrollSpeed())); };
	addShortcut (KeysList.keyScroll8b, scrollUp);
	addShortcut (KeysList.keyScroll8a, scrollUp);

	addShortcut (KeysList.keyChat, [this]() {
		hud->setChatActive (true);
		chatBox->focus();
	});
}

//------------------------------------------------------------------------------
void cGameGui::handleResolutionChange()
{
	hud->resizeToResolution();

	resize (hud->getSize());

	auto font = cUnicodeFont::font.get();

	// TODO: remove duplication of widget areas with the ones during initialization

	hudPanels->resize (cPosition (hudPanels->getSize().x(), getSize().y()));

	gameMap->resize (getSize() - cPosition (cHud::panelTotalWidth, cHud::panelTopHeight));

	messageList->setArea (cBox<cPosition> (cPosition (cHud::panelLeftWidth + 4, cHud::panelTopHeight + 7), cPosition (getEndPosition().x() - cHud::panelRightWidth - 4, cHud::panelTopHeight + 200)));

	chatBox->setArea (cBox<cPosition> (cPosition (cHud::panelLeftWidth + 4, getEndPosition().y() - cHud::panelBottomHeight - 12 - 100), getEndPosition() - cPosition (cHud::panelRightWidth + 4, cHud::panelBottomHeight + 12)));

	primiaryInfoLabel->setArea (cBox<cPosition> (cPosition (cHud::panelLeftWidth, 235), cPosition (getEndPosition().x() - cHud::panelRightWidth, 235 + font->getFontHeight (eUnicodeFontType::LatinBig))));
	additionalInfoLabel->setArea (cBox<cPosition> (cPosition (cHud::panelLeftWidth, 235 + font->getFontHeight (eUnicodeFontType::LatinBig)), cPosition (getEndPosition().x() - cHud::panelRightWidth, 235 + font->getFontHeight (eUnicodeFontType::LatinBig) + font->getFontHeight (eUnicodeFontType::LatinNormal))));

	debugOutput->setArea (cBox<cPosition> (cPosition (cHud::panelLeftWidth + 4, cHud::panelTopHeight + 7), cPosition (getEndPosition().x() - cHud::panelRightWidth - 8, getEndPosition().y() - cHud::panelBottomHeight - 8)));
}
