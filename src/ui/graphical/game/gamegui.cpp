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

#include <sstream>
#include <iomanip>
#include <iostream>

#include "ui/graphical/game/gamegui.h"
#include "ui/graphical/game/gameguistate.h"
#include "ui/graphical/game/hud.h"
#include "ui/graphical/game/widgets/gamemapwidget.h"
#include "ui/graphical/game/widgets/minimapwidget.h"
#include "ui/graphical/game/widgets/unitcontextmenuwidget.h"
#include "ui/graphical/game/widgets/gamemessagelistview.h"
#include "ui/graphical/game/widgets/hudpanels.h"
#include "ui/graphical/game/widgets/chatbox.h"
#include "ui/graphical/game/widgets/debugoutputwidget.h"

#include "ui/graphical/game/animations/animationtimer.h"

#include "ui/sound/soundmanager.h"
#include "ui/sound/effects/soundeffect.h"
#include "ui/sound/effects/soundeffectvoice.h"
#include "ui/sound/effects/soundeffectunit.h"

#include "ui/graphical/application.h"
#include "ui/graphical/menu/widgets/label.h"

#include "keys.h"
#include "game/data/player/player.h"
#include "game/data/map/map.h"
#include "game/data/units/vehicle.h"
#include "game/data/units/building.h"
#include "sound.h"
#include "game/logic/client.h"
#include "game/logic/server.h"
#include "game/logic/clientevents.h"
#include "utility/log.h"
#include "netmessage.h"
#include "game/startup/game.h"
#include "input/mouse/mouse.h"
#include "input/keyboard/keyboard.h"
#include "input/mouse/cursor/mousecursorsimple.h"
#include "game/data/report/savedreportsimple.h"
#include "game/data/report/savedreportchat.h"
#include "game/data/report/savedreportunit.h"
#include "game/data/report/special/savedreporthostcommand.h"
#include "game/logic/turnclock.h"
#include "utility/random.h"
#include "utility/indexiterator.h"

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

	resize (hudOwning->getSize ());

	gameMap = addChild (std::make_unique<cGameMapWidget> (cBox<cPosition> (cPosition (cHud::panelLeftWidth, cHud::panelTopHeight), getEndPosition () - cPosition (cHud::panelRightWidth, cHud::panelBottomHeight)), staticMap, animationTimer, soundManager, frameCounter));

	messageList = addChild (std::make_unique<cGameMessageListView> (cBox<cPosition> (cPosition (cHud::panelLeftWidth + 4, cHud::panelTopHeight + 7), cPosition (getEndPosition ().x () - cHud::panelRightWidth - 4, cHud::panelTopHeight + 200))));

	hud = addChild (std::move (hudOwning));

	hud->setMinimalZoomFactor (gameMap->computeMinimalZoomFactor ());

    chatBox = addChild (std::make_unique<cChatBox> (cBox<cPosition> (cPosition (cHud::panelLeftWidth + 4, getEndPosition ().y () - cHud::panelBottomHeight - 12 - 100), getEndPosition () - cPosition (cHud::panelRightWidth + 4, cHud::panelBottomHeight + 12))));

	miniMap = addChild (std::make_unique<cMiniMapWidget> (cBox<cPosition> (cPosition (15, 356), cPosition (15 + 112, 356 + 112)), staticMap));

	debugOutput = addChild (std::make_unique<cDebugOutputWidget> (cBox<cPosition> (cPosition (cHud::panelLeftWidth + 4, cHud::panelTopHeight + 7), cPosition (getEndPosition ().x () - cHud::panelRightWidth - 8, getEndPosition ().y () - cHud::panelBottomHeight - 8))));
	debugOutput->setGameMap (gameMap);
	debugOutput->disable ();

	hudPanels = addChild (std::make_unique<cHudPanels> (getPosition (), getSize ().y (), animationTimer));

	primiaryInfoLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (cPosition (cHud::panelLeftWidth, 235), cPosition (getEndPosition ().x () - cHud::panelRightWidth, 235 + font->getFontHeight (FONT_LATIN_BIG))), "", FONT_LATIN_BIG, toEnumFlag (eAlignmentType::CenterHorizontal)  | eAlignmentType::Top));
	primiaryInfoLabel->disable ();
	primiaryInfoLabel->hide ();

	additionalInfoLabel = addChild (std::make_unique<cLabel> (cBox<cPosition> (cPosition (cHud::panelLeftWidth, 235 + font->getFontHeight (FONT_LATIN_BIG)), cPosition (getEndPosition ().x () - cHud::panelRightWidth, 235 + font->getFontHeight (FONT_LATIN_BIG) + font->getFontHeight (FONT_LATIN_NORMAL))), "", FONT_LATIN_NORMAL, toEnumFlag (eAlignmentType::CenterHorizontal)  | eAlignmentType::Top));
	additionalInfoLabel->disable ();
	additionalInfoLabel->hide ();

	signalConnectionManager.connect (hudPanels->opened, [&]()
	{
		hudPanels->disable ();
		hudPanels->hide ();
	});

	hud->activateShortcuts ();
	gameMap->deactivateUnitCommandShortcuts ();

	using namespace std::placeholders;

	signalConnectionManager.connect (hud->zoomChanged, [&](){ gameMap->setZoomFactor (hud->getZoomFactor (), true); });

	signalConnectionManager.connect (hud->surveyToggled, [&](){ gameMap->setDrawSurvey (hud->getSurveyActive ()); });
	signalConnectionManager.connect (hud->hitsToggled, [&](){ gameMap->setDrawHits (hud->getHitsActive ()); });
	signalConnectionManager.connect (hud->scanToggled, [&](){ gameMap->setDrawScan (hud->getScanActive ()); });
	signalConnectionManager.connect (hud->statusToggled, [&](){ gameMap->setDrawStatus (hud->getStatusActive ()); });
	signalConnectionManager.connect (hud->ammoToggled, [&](){ gameMap->setDrawAmmo (hud->getAmmoActive ()); });
	signalConnectionManager.connect (hud->gridToggled, [&](){ gameMap->setDrawGrid (hud->getGridActive ()); });
	signalConnectionManager.connect (hud->colorToggled, [&](){ gameMap->setDrawColor (hud->getColorActive ()); });
	signalConnectionManager.connect (hud->rangeToggled, [&](){ gameMap->setDrawRange (hud->getRangeActive ()); });
	signalConnectionManager.connect (hud->fogToggled, [&](){ gameMap->setDrawFog (hud->getFogActive ()); });
	signalConnectionManager.connect (hud->lockToggled, [&](){ gameMap->setLockActive (hud->getLockActive ()); });

	signalConnectionManager.connect (hud->helpClicked, [&](){ gameMap->toggleHelpMode (); });
	signalConnectionManager.connect (hud->chatToggled, [&]()
	{
		if (hud->getChatActive())
		{
			chatBox->show ();
			chatBox->enable ();
		}
		else
		{
			chatBox->hide ();
			chatBox->disable ();
		}
	});
	hud->setChatActive (true);

	signalConnectionManager.connect (hud->miniMapZoomFactorToggled, [&](){ miniMap->setZoomFactor (hud->getMiniMapZoomFactorActive () ? 2 : 1); });
	signalConnectionManager.connect (hud->miniMapAttackUnitsOnlyToggled, [&](){ miniMap->setAttackUnitsUnly (hud->getMiniMapAttackUnitsOnly ()); });

	signalConnectionManager.connect (gameMap->scrolled, std::bind(&cGameGui::resetMiniMapViewWindow, this));
	signalConnectionManager.connect (gameMap->zoomFactorChanged, std::bind (&cGameGui::resetMiniMapViewWindow, this));
	signalConnectionManager.connect (gameMap->tileUnderMouseChanged, std::bind (&cGameGui::updateHudCoordinates, this, _1));
	signalConnectionManager.connect (gameMap->tileUnderMouseChanged, std::bind (&cGameGui::updateHudUnitName, this, _1));

	signalConnectionManager.connect (gameMap->mouseFocusReleased, [this]()
	{
		auto mouse = getActiveMouse ();
		if (mouse)
		{
			if (hud->isAt (mouse->getPosition ())) mouse->setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::Hand));
			else if (chatBox->isAt (mouse->getPosition ())) mouse->setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::Hand));
			else gameMap->updateMouseCursor (*mouse);
		}
	});

	signalConnectionManager.connect (gameMap->getUnitSelection ().mainSelectionChanged, [&](){ hud->setActiveUnit (gameMap->getUnitSelection ().getSelectedUnit ()); });
	signalConnectionManager.connect (gameMap->getUnitSelection ().mainSelectionChanged, std::bind (&cGameGui::updateSelectedUnitIdleSound, this));
	signalConnectionManager.connect (gameMap->getUnitSelection ().mainSelectionChanged, std::bind (&cGameGui::connectSelectedUnit, this));
	signalConnectionManager.connect (gameMap->getUnitSelection ().mainSelectionChanged, [&]()
	{
		if (!player) return;

		auto unit = gameMap->getUnitSelection ().getSelectedUnit ();
		if (unit && unit->getOwner () == player.get ()) unit->makeReport (*soundManager);
	});
	signalConnectionManager.connect (gameMap->getUnitSelection ().mainSelectionChanged, [&]()
	{
		if (gameMap->getUnitSelection ().getSelectedUnit () == nullptr)
		{
			hud->activateShortcuts ();
		}
		else
		{
			hud->deactivateShortcuts ();
		}
	});

	signalConnectionManager.connect (miniMap->focus, [&](const cPosition& position){ gameMap->centerAt (position); });

	signalConnectionManager.connect (animationTimer->triggered10msCatchUp, [&]()
	{
		if (mouseScrollDirection != cPosition (0, 0))
		{
			gameMap->scroll (mouseScrollDirection);
		}
	});
	signalConnectionManager.connect (animationTimer->triggered400ms, [&]()
	{
		messageList->removeOldMessages ();
	});

	terminated.connect ([&]()
	{
		stopSelectedUnitSound ();
	});

	initShortcuts ();

	signalConnectionManager.connect (Video.resolutionChanged, std::bind (&cGameGui::handleResolutionChange, this));

	signalConnectionManager.connect (Video.screenShotTaken, [this](const std::string& path)
	{
		messageList->addMessage (lngPack.i18n ("Text~Comp~Screenshot_Done", path));
	});
}

//------------------------------------------------------------------------------
void cGameGui::setDynamicMap (std::shared_ptr<const cMap> dynamicMap_)
{
	dynamicMap = std::move(dynamicMap_);
	gameMap->setDynamicMap (dynamicMap);
	miniMap->setDynamicMap (dynamicMap);

	dynamicMapSignalConnectionManager.disconnectAll ();

	if (dynamicMap != nullptr)
	{
		dynamicMapSignalConnectionManager.connect (dynamicMap->addedUnit, [&](const cUnit& unit)
		{
			if (unit.data.ID == UnitsData.specialIDLandMine) soundManager->playSound (std::make_shared<cSoundEffectUnit> (eSoundEffectType::EffectPlaceMine, SoundData.SNDLandMinePlace, unit));
			else if (unit.data.ID == UnitsData.specialIDSeaMine) soundManager->playSound (std::make_shared<cSoundEffectUnit> (eSoundEffectType::EffectPlaceMine, SoundData.SNDSeaMinePlace, unit));
		});
		dynamicMapSignalConnectionManager.connect (dynamicMap->removedUnit, [&](const cUnit& unit)
		{
			if (unit.data.ID == UnitsData.specialIDLandMine) soundManager->playSound (std::make_shared<cSoundEffectUnit> (eSoundEffectType::EffectClearMine, SoundData.SNDLandMineClear, unit));
			else if (unit.data.ID == UnitsData.specialIDSeaMine) soundManager->playSound (std::make_shared<cSoundEffectUnit> (eSoundEffectType::EffectClearMine, SoundData.SNDSeaMineClear, unit));
		});
		//dynamicMapSignalConnectionManager.connect (dynamicMap->movedVehicle, [&](const cVehicle&){ });
	}
}

//------------------------------------------------------------------------------
void cGameGui::setPlayer (std::shared_ptr<const cPlayer> player_)
{
	player = std::move(player_);
	gameMap->setPlayer (player);
	miniMap->setPlayer (player);
	hud->setPlayer (player);

	messageList->clear ();
}

//------------------------------------------------------------------------------
void cGameGui::setPlayers (std::vector<std::shared_ptr<const cPlayer>> players)
{
	chatBox->clearPlayers ();
	for (size_t i = 0; i < players.size (); ++i)
	{
		chatBox->addPlayer (*players[i]);
	}
}

//------------------------------------------------------------------------------
void cGameGui::setTurnClock (std::shared_ptr<const cTurnClock> turnClock)
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
cHud& cGameGui::getHud ()
{
	return *hud;
}

//------------------------------------------------------------------------------
const cHud& cGameGui::getHud () const
{
	return *hud;
}

//------------------------------------------------------------------------------
cGameMapWidget& cGameGui::getGameMap ()
{
	return *gameMap;
}

//------------------------------------------------------------------------------
const cGameMapWidget& cGameGui::getGameMap () const
{
	return *gameMap;
}

//------------------------------------------------------------------------------
cChatBox& cGameGui::getChatBox ()
{
	return *chatBox;
}

//------------------------------------------------------------------------------
const cChatBox& cGameGui::getChatBox () const
{
	return *chatBox;
}

//------------------------------------------------------------------------------
cGameMessageListView& cGameGui::getGameMessageList ()
{
	return *messageList;
}

//------------------------------------------------------------------------------
const cGameMessageListView& cGameGui::getGameMessageList () const
{
	return *messageList;
}

//------------------------------------------------------------------------------
cDebugOutputWidget& cGameGui::getDebugOutput ()
{
	return *debugOutput;
}

//------------------------------------------------------------------------------
void cGameGui::setInfoTexts (const std::string& primiaryText, const std::string& additionalText)
{
	primiaryInfoLabel->setText (primiaryText);
	if (primiaryText.empty ()) primiaryInfoLabel->hide ();
	else primiaryInfoLabel->show ();

	additionalInfoLabel->setText (additionalText);
	if (additionalText.empty ()) additionalInfoLabel->hide ();
	else additionalInfoLabel->show ();
}

//------------------------------------------------------------------------------
void cGameGui::exit ()
{
	panelSignalConnectionManager.disconnectAll ();

	panelSignalConnectionManager.connect (hudPanels->closed, [&]()
	{
		close ();
	});
	startClosePanel ();
}

//------------------------------------------------------------------------------
cGameGuiState cGameGui::getCurrentState () const
{
	cGameGuiState state;
	state.setMapPosition (gameMap->getMapCenterOffset ());

	state.setMapZoomFactor (hud->getZoomFactor ());
	state.setSurveyActive (hud->getSurveyActive ());
	state.setHitsActive (hud->getHitsActive ());
	state.setScanActive (hud->getScanActive ());
	state.setStatusActive (hud->getStatusActive ());
	state.setAmmoActive (hud->getAmmoActive ());
	state.setGridActive (hud->getGridActive ());
	state.setColorActive (hud->getColorActive ());
	state.setRangeActive (hud->getRangeActive ());
	state.setFogActive (hud->getFogActive ());
	state.setLockActive (hud->getLockActive ());
	state.setMiniMapZoomFactorActive (hud->getMiniMapZoomFactorActive ());
	state.setMiniMapAttackUnitsOnly (hud->getMiniMapAttackUnitsOnly ());
	state.setUnitVideoPlaying (hud->isUnitVideoPlaying ());
	state.setChatActive (hud->getChatActive ());

	state.setSelectedUnits (gameMap->getUnitSelection ());
	state.setLockedUnits (gameMap->getUnitLockList ());

	return state;
}

//------------------------------------------------------------------------------
void cGameGui::restoreState (const cGameGuiState& state)
{
	gameMap->centerAt (state.getMapPosition ());

	hud->setZoomFactor (state.getMapZoomFactor ());
	hud->setSurveyActive (state.getSurveyActive ());
	hud->setHitsActive (state.getHitsActive ());
	hud->setScanActive (state.getScanActive ());
	hud->setStatusActive (state.getStatusActive ());
	hud->setAmmoActive (state.getAmmoActive ());
	hud->setGridActive (state.getGridActive ());
	hud->setColorActive (state.getColorActive ());
	hud->setRangeActive (state.getRangeActive ());
	hud->setFogActive (state.getFogActive ());
	hud->setLockActive (state.getLockActive ());
	hud->setMiniMapZoomFactorActive (state.getMiniMapZoomFactorActive ());
	hud->setMiniMapAttackUnitsOnly (state.getMiniMapAttackUnitsOnly ());
	hud->setChatActive (state.getChatActive ());
	if (state.getUnitVideoPlaying ()) hud->startUnitVideo ();
	else hud->stopUnitVideo ();

	gameMap->getUnitSelection ().deselectUnits ();
	gameMap->getUnitLockList ().unlockAll ();

	auto selectedUnitIds = state.getSelectedUnitIds ();
	auto lockedUnitIds = state.getLockedUnitIds ();

	if (dynamicMap)
	{
		// TODO: this may be very inefficient!
		//       we go over all map fields to find the units on the map that need to be selected.
		//       we may should think about a more efficient way to find specific units on the map
		std::vector<cUnit*> fieldUnits;
		for (auto i = makeIndexIterator (cPosition(0,0), dynamicMap->getSize()); i.hasMore (); i.next ())
		{
			if (selectedUnitIds.empty () && lockedUnitIds.empty ()) break;

			const auto& field = dynamicMap->getField (*i);

			field.getUnits (fieldUnits);

			for (size_t j = 0; j < fieldUnits.size (); ++j)
			{
				for (auto k = selectedUnitIds.begin(); k != selectedUnitIds.end (); )
				{
					if (fieldUnits[j]->iID == *k)
					{
						gameMap->getUnitSelection ().selectUnit (*fieldUnits[j], true);
						k = selectedUnitIds.erase (k);
					}
					else
					{
						++k;
					}
				}
				for (auto k = lockedUnitIds.begin (); k != lockedUnitIds.end ();)
				{
					if (fieldUnits[j]->iID == *k)
					{
						gameMap->getUnitLockList ().lockUnit (*fieldUnits[j]);
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
	coordsString << std::setw (3) << std::setfill ('0') << tilePosition.x () << "-" << std::setw (3) << std::setfill ('0') << tilePosition.y ();
	hud->setCoordinatesText (coordsString.str ());
}

//------------------------------------------------------------------------------
void cGameGui::updateHudUnitName (const cPosition& tilePosition)
{
	std::string unitNameString;
	if (dynamicMap && (!player || player->canSeeAt (tilePosition)))
	{
		const auto& field = dynamicMap->getField (tilePosition);

		cUnit* unit = nullptr;
		if (field.getVehicle () != nullptr) unit = field.getVehicle ();
		else if (field.getPlane () != nullptr) unit = field.getPlane ();
		else if (field.getTopBuilding () != nullptr) unit = field.getTopBuilding ();
		else if (field.getBaseBuilding () != nullptr && field.getBaseBuilding ()->getOwner ()) unit = field.getBaseBuilding ();

		if (unit != nullptr)
		{
			// FIXME: string may be to long.
			unitNameString = unit->getDisplayName () + " (" + unit->getOwner ()->getName () + ")";
		}
	}
	hud->setUnitNameText (unitNameString);
}

//------------------------------------------------------------------------------
void cGameGui::connectSelectedUnit ()
{
	selectedUnitConnectionManager.disconnectAll ();

	const auto selectedUnit = gameMap->getUnitSelection ().getSelectedUnit ();

	if (!selectedUnit) return;

	selectedUnitConnectionManager.connect (selectedUnit->workingChanged, [selectedUnit,this]()
	{
		stopSelectedUnitSound ();
		if (selectedUnit->data.ID.isABuilding ())
		{
			if (selectedUnit->isUnitWorking ()) soundManager->playSound (std::make_shared<cSoundEffectUnit> (eSoundEffectType::EffectStartWork, static_cast<const cBuilding&>(*selectedUnit).uiData->Start, *selectedUnit));
			else soundManager->playSound (std::make_shared<cSoundEffectUnit> (eSoundEffectType::EffectStopWork, static_cast<const cBuilding&>(*selectedUnit).uiData->Stop, *selectedUnit));
		}
		updateSelectedUnitIdleSound ();
	});


	selectedUnitConnectionManager.connect (selectedUnit->buildingChanged, [this]()
	{
		updateSelectedUnitIdleSound ();
	});

	selectedUnitConnectionManager.connect (selectedUnit->clearingChanged, [this]()
	{
		updateSelectedUnitIdleSound ();
	});

	if (selectedUnit->data.ID.isAVehicle ())
	{
		const auto selectedVehicle = static_cast<cVehicle*>(selectedUnit);
		selectedUnitConnectionManager.connect (selectedVehicle->clientMoveJobChanged, [selectedVehicle, this]()
		{
			connectMoveJob (*selectedVehicle);
		});
	}
}

//------------------------------------------------------------------------------
void cGameGui::connectMoveJob (const cVehicle& vehicle)
{
	moveJobSignalConnectionManager.disconnectAll ();

	if (&vehicle == gameMap->getUnitSelection ().getSelectedVehicle () && vehicle.getClientMoveJob ())
	{
		auto& moveJob = *vehicle.getClientMoveJob ();

		moveJobSignalConnectionManager.connect (moveJob.activated, [&](const cVehicle& vehicle)
		{
			if (&vehicle == gameMap->getUnitSelection ().getSelectedVehicle ())
			{
				updateSelectedUnitMoveSound ();
			}
		});

		moveJobSignalConnectionManager.connect (moveJob.stopped, [&](const cVehicle& vehicle)
		{
			if (&vehicle == gameMap->getUnitSelection ().getSelectedVehicle () && dynamicMap)
			{
				const auto building = dynamicMap->getField (vehicle.getPosition()).getBaseBuilding ();
				bool water = dynamicMap->isWater (vehicle.getPosition());
				if (vehicle.data.factorGround > 0 && building && (building->data.surfacePosition == sUnitData::SURFACE_POS_BASE || building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE || building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_SEA)) water = false;

				stopSelectedUnitSound ();
				if (water && vehicle.data.factorSea > 0) soundManager->playSound (std::make_shared<cSoundEffectUnit> (eSoundEffectType::EffectStopMove, vehicle.uiData->StopWater, vehicle));
				else soundManager->playSound (std::make_shared<cSoundEffectUnit> (eSoundEffectType::EffectStopMove, vehicle.uiData->Stop, vehicle));

				updateSelectedUnitIdleSound ();
			}
		});

		moveJobSignalConnectionManager.connect (moveJob.blocked, [&](const cVehicle& vehicle)
		{
			if (&vehicle == gameMap->getUnitSelection ().getSelectedVehicle ())
			{
				soundManager->playSound (std::make_shared<cSoundEffectVoice> (eSoundEffectType::VoiceNoPath, getRandom (VoiceData.VOINoPath)));
			}
		});

		moveJobSignalConnectionManager.connect (moveJob.moved, [&](const cVehicle& vehicle)
		{
			if (&vehicle == gameMap->getUnitSelection ().getSelectedVehicle ())
			{
				if (!vehicle.getClientMoveJob ()) return;

				bool wasWater = dynamicMap->isWater (vehicle.getClientMoveJob ()->Waypoints->position);
				bool water = dynamicMap->isWater (vehicle.getClientMoveJob ()->Waypoints->next->position);

				if (wasWater != water)
				{
					updateSelectedUnitMoveSound ();
				}
			}
		});
	}
}

//------------------------------------------------------------------------------
bool cGameGui::handleMouseMoved (cApplication& application, cMouse& mouse, const cPosition& offset)
{
	const auto currentMousePosition = mouse.getPosition ();
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
	if (currentMousePosition.x () <= scrollFrameWidth) mouseScrollDirection.x () = -cSettings::getInstance ().getScrollSpeed () / 4;
	else if (currentMousePosition.x () >= getEndPosition ().x () - scrollFrameWidth) mouseScrollDirection.x () = +cSettings::getInstance ().getScrollSpeed () / 4;
	if (currentMousePosition.y () <= scrollFrameWidth) mouseScrollDirection.y () = -cSettings::getInstance ().getScrollSpeed () / 4;
	else if (currentMousePosition.y () >= getEndPosition ().y () - scrollFrameWidth) mouseScrollDirection.y () = +cSettings::getInstance ().getScrollSpeed () / 4;

	if (mouseScrollDirection.x () > 0 &&  mouseScrollDirection.y () == 0) mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::ArrowRight));
	else if (mouseScrollDirection.x () < 0 &&  mouseScrollDirection.y () == 0) mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::ArrowLeft));
	else if (mouseScrollDirection.x () == 0 &&  mouseScrollDirection.y () > 0) mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::ArrowDown));
	else if (mouseScrollDirection.x () == 0 &&  mouseScrollDirection.y () < 0) mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::ArrowUp));
	else if (mouseScrollDirection.x () > 0 &&  mouseScrollDirection.y () > 0) mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::ArrowRightDown));
	else if (mouseScrollDirection.x () > 0 &&  mouseScrollDirection.y () < 0) mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::ArrowRightUp));
	else if (mouseScrollDirection.x () < 0 &&  mouseScrollDirection.y () > 0) mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::ArrowLeftDown));
	else if (mouseScrollDirection.x () < 0 &&  mouseScrollDirection.y () < 0) mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::ArrowLeftUp));
	else if (hud->isAt (currentMousePosition)) mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::Hand));
	else if (chatBox->isAt (currentMousePosition)) mouse.setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::Hand));

	return cWindow::handleMouseMoved (application, mouse, offset);
}

//------------------------------------------------------------------------------
bool cGameGui::handleMouseWheelMoved (cApplication& application, cMouse& mouse, const cPosition& amount)
{
	const auto oldZoomFactor = gameMap->getZoomFactor ();
	bool consumed = false;
	if (amount.y () > 0)
	{
		hud->decreaseZoomFactor (0.05);
		consumed = true;
	}
	else if (amount.y () < 0)
	{
		hud->increaseZoomFactor (0.05);
		consumed = true;
	}

	// scroll so that the zoom has been performed to center the mouse
	const auto newZoomFactor = gameMap->getZoomFactor ();
	if (newZoomFactor != oldZoomFactor)
	{
		cPosition scrollOffset;

		const auto oldScreenPixelX = gameMap->getSize ().x () / oldZoomFactor;
		const auto newScreenPixelX = gameMap->getSize ().x () / newZoomFactor;
		scrollOffset.x () = (int)((oldScreenPixelX - newScreenPixelX) * (mouse.getPosition ().x () - gameMap->getPosition ().x ()) / gameMap->getSize ().x () - (oldScreenPixelX - newScreenPixelX) / 2);

		const auto oldScreenPixelY = gameMap->getSize ().y () / oldZoomFactor;
		const auto newScreenPixelY = gameMap->getSize ().y () / newZoomFactor;
		scrollOffset.y () = (int)((oldScreenPixelY - newScreenPixelY) * (mouse.getPosition ().y () - gameMap->getPosition ().y ()) / gameMap->getSize ().y () - (oldScreenPixelY - newScreenPixelY) / 2);

		gameMap->scroll (scrollOffset);
	}

	return consumed;
}

//------------------------------------------------------------------------------
void cGameGui::handleActivated (cApplication& application, bool firstTime)
{
	cWindow::handleActivated (application, firstTime);

	auto mouse = getActiveMouse ();
	if (mouse)
	{
        if (hud->isAt (mouse->getPosition ())) mouse->setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::Hand));
        else if (chatBox->isAt (mouse->getPosition ())) mouse->setCursor (std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::Hand));
		else gameMap->updateMouseCursor (*mouse);
	}

	if (openPanelOnActivation)
	{
		startOpenPanel ();
		openPanelOnActivation = false;
	}
}

//------------------------------------------------------------------------------
void cGameGui::handleDeactivated (cApplication& application, bool removed)
{
	cWindow::handleDeactivated (application, removed);
}

//------------------------------------------------------------------------------
bool cGameGui::wantsCentered () const
{
	return false;
}

//------------------------------------------------------------------------------
std::unique_ptr<cMouseCursor> cGameGui::getDefaultCursor () const
{
	return nullptr;
}

//------------------------------------------------------------------------------
void cGameGui::startOpenPanel ()
{
	hudPanels->open ();
}

//------------------------------------------------------------------------------
void cGameGui::startClosePanel ()
{
	hudPanels->show ();
	hudPanels->enable ();

	hudPanels->close ();
}

//------------------------------------------------------------------------------
void cGameGui::resetMiniMapViewWindow ()
{
	miniMap->setViewWindow (gameMap->getDisplayedMapArea ());
}

//------------------------------------------------------------------------------
void cGameGui::updateSelectedUnitIdleSound ()
{
	auto selectedUnit = gameMap->getUnitSelection().getSelectedUnit ();
	if (selectedUnit == nullptr)
	{
		stopSelectedUnitSound ();
	}
	else if (selectedUnit->data.ID.isABuilding ())
	{
		const auto& building = static_cast<const cBuilding&>(*selectedUnit);
		if (building.isUnitWorking ())
		{
			startSelectedUnitSound (building, building.uiData->Running);
		}
		else
		{
			startSelectedUnitSound (building, building.uiData->Wait);
		}
	}
	else if (selectedUnit->data.ID.isAVehicle ())
	{
		const auto& vehicle = static_cast<const cVehicle&>(*selectedUnit);

		const cBuilding* building = dynamicMap ? dynamicMap->getField (vehicle.getPosition()).getBaseBuilding () : nullptr;
		bool water = staticMap->isWater (vehicle.getPosition());
		if (vehicle.data.factorGround > 0 && building && (building->data.surfacePosition == sUnitData::SURFACE_POS_BASE || building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE || building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_SEA)) water = false;

		if (vehicle.isUnitBuildingABuilding () && (vehicle.getBuildTurns () || player.get () != vehicle.getOwner ()))
		{
			startSelectedUnitSound (vehicle, SoundData.SNDBuilding);
		}
		else if (vehicle.isUnitClearing ())
		{
			startSelectedUnitSound (vehicle, SoundData.SNDClearing);
		}
		else if (water && vehicle.data.factorSea > 0)
		{
			startSelectedUnitSound (vehicle, vehicle.uiData->WaitWater);
		}
		else
		{
			startSelectedUnitSound (vehicle, vehicle.uiData->Wait);
		}
	}
}

//------------------------------------------------------------------------------
void cGameGui::updateSelectedUnitMoveSound ()
{
	auto selectedVehicle = gameMap->getUnitSelection ().getSelectedVehicle ();
	if (!selectedVehicle) return;
	if (!dynamicMap) return;

	const auto& vehicle = *selectedVehicle;

	const auto building = dynamicMap->getField (vehicle.getPosition()).getBaseBuilding ();
	bool water = dynamicMap->isWater (vehicle.getPosition());
	if (vehicle.data.factorGround > 0 && building && (building->data.surfacePosition == sUnitData::SURFACE_POS_BASE || building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_BASE || building->data.surfacePosition == sUnitData::SURFACE_POS_ABOVE_SEA)) water = false;
	stopSelectedUnitSound ();

	if (water && vehicle.data.factorSea != 0) soundManager->playSound (std::make_shared<cSoundEffectUnit> (eSoundEffectType::EffectStartMove, vehicle.uiData->StartWater, vehicle));
	else soundManager->playSound (std::make_shared<cSoundEffectUnit> (eSoundEffectType::EffectStartMove, vehicle.uiData->Start, vehicle));

	if (water && vehicle.data.factorSea != 0)
		startSelectedUnitSound (vehicle, vehicle.uiData->DriveWater);
	else
		startSelectedUnitSound (vehicle, vehicle.uiData->Drive);
}

//------------------------------------------------------------------------------
void cGameGui::startSelectedUnitSound (const cUnit& unit, const cSoundChunk& sound)
{
	stopSelectedUnitSound ();
	selectedUnitSoundLoop = std::make_shared<cSoundEffectUnit> (eSoundEffectType::EffectUnitSound, sound, unit);
	soundManager->playSound (selectedUnitSoundLoop, true);
}

//------------------------------------------------------------------------------
void cGameGui::stopSelectedUnitSound ()
{
	if (selectedUnitSoundLoop)
	{
		selectedUnitSoundLoop->stop ();
		selectedUnitSoundLoop = nullptr;
	}
}

//------------------------------------------------------------------------------
void cGameGui::initShortcuts ()
{
	auto scroll1Shortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyScroll1));
	signalConnectionManager.connect (scroll1Shortcut->triggered, std::bind (&cGameMapWidget::scroll, gameMap, cPosition (-cSettings::getInstance ().getScrollSpeed (), +cSettings::getInstance ().getScrollSpeed ())));

	auto scroll3Shortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyScroll3));
	signalConnectionManager.connect (scroll3Shortcut->triggered, std::bind (&cGameMapWidget::scroll, gameMap, cPosition (+cSettings::getInstance ().getScrollSpeed (), +cSettings::getInstance ().getScrollSpeed ())));

	auto scroll7Shortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyScroll7));
	signalConnectionManager.connect (scroll7Shortcut->triggered, std::bind (&cGameMapWidget::scroll, gameMap, cPosition (-cSettings::getInstance ().getScrollSpeed (), -cSettings::getInstance ().getScrollSpeed ())));

	auto scroll9Shortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyScroll9));
	signalConnectionManager.connect (scroll9Shortcut->triggered, std::bind (&cGameMapWidget::scroll, gameMap, cPosition (+cSettings::getInstance ().getScrollSpeed (), -cSettings::getInstance ().getScrollSpeed ())));


	auto scroll2aShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyScroll2a));
	signalConnectionManager.connect (scroll2aShortcut->triggered, std::bind (&cGameMapWidget::scroll, gameMap, cPosition (0, +cSettings::getInstance ().getScrollSpeed ())));
	auto scroll2bShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyScroll2b));
	signalConnectionManager.connect (scroll2bShortcut->triggered, std::bind (&cGameMapWidget::scroll, gameMap, cPosition (0, +cSettings::getInstance ().getScrollSpeed ())));

	auto scroll4aShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyScroll4a));
	signalConnectionManager.connect (scroll4aShortcut->triggered, std::bind (&cGameMapWidget::scroll, gameMap, cPosition (-cSettings::getInstance ().getScrollSpeed (), 0)));
	auto scroll4bShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyScroll4b));
	signalConnectionManager.connect (scroll4bShortcut->triggered, std::bind (&cGameMapWidget::scroll, gameMap, cPosition (-cSettings::getInstance ().getScrollSpeed (), 0)));

	auto scroll6aShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyScroll6a));
	signalConnectionManager.connect (scroll6aShortcut->triggered, std::bind (&cGameMapWidget::scroll, gameMap, cPosition (+cSettings::getInstance ().getScrollSpeed (), 0)));
	auto scroll6bShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyScroll6b));
	signalConnectionManager.connect (scroll6bShortcut->triggered, std::bind (&cGameMapWidget::scroll, gameMap, cPosition (+cSettings::getInstance ().getScrollSpeed (), 0)));

	auto scroll8aShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyScroll8a));
	signalConnectionManager.connect (scroll8aShortcut->triggered, std::bind (&cGameMapWidget::scroll, gameMap, cPosition (0, -cSettings::getInstance ().getScrollSpeed ())));
	auto scroll8bShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyScroll8b));
	signalConnectionManager.connect (scroll8bShortcut->triggered, std::bind (&cGameMapWidget::scroll, gameMap, cPosition (0, -cSettings::getInstance ().getScrollSpeed ())));


	auto chatShortcut = addShortcut (std::make_unique<cShortcut> (KeysList.keyChat));
	signalConnectionManager.connect (chatShortcut->triggered, [&]()
	{
		hud->setChatActive (true);
		chatBox->focus ();
	});
}

//------------------------------------------------------------------------------
void cGameGui::handleResolutionChange ()
{
	hud->resizeToResolution ();

	resize (hud->getSize ());

	// TODO: remove duplication of widget areas with the ones during initialization

	hudPanels->resize (cPosition (hudPanels->getSize ().x (), getSize ().y ()));

	gameMap->resize (getSize () - cPosition (cHud::panelTotalWidth, cHud::panelTopHeight));

	messageList->setArea (cBox<cPosition> (cPosition (cHud::panelLeftWidth + 4, cHud::panelTopHeight + 7), cPosition (getEndPosition ().x () - cHud::panelRightWidth - 4, cHud::panelTopHeight + 200)));

	chatBox->setArea (cBox<cPosition> (cPosition (cHud::panelLeftWidth + 4, getEndPosition ().y () - cHud::panelBottomHeight - 12 - 100), getEndPosition () - cPosition (cHud::panelRightWidth + 4, cHud::panelBottomHeight + 12)));

	primiaryInfoLabel->setArea(cBox<cPosition> (cPosition (cHud::panelLeftWidth, 235), cPosition (getEndPosition ().x () - cHud::panelRightWidth, 235 + font->getFontHeight (FONT_LATIN_BIG))));
	additionalInfoLabel->setArea (cBox<cPosition> (cPosition (cHud::panelLeftWidth, 235 + font->getFontHeight (FONT_LATIN_BIG)), cPosition (getEndPosition ().x () - cHud::panelRightWidth, 235 + font->getFontHeight (FONT_LATIN_BIG) + font->getFontHeight (FONT_LATIN_NORMAL))));

	debugOutput->setArea (cBox<cPosition> (cPosition (cHud::panelLeftWidth + 4, cHud::panelTopHeight + 7), cPosition (getEndPosition ().x () - cHud::panelRightWidth - 8, getEndPosition ().y () - cHud::panelBottomHeight - 8)));
}
