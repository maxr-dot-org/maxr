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

#include <functional>

#include "ui/graphical/menu/windows/windowsingleplayer.h"
#include "game/data/gamesettings.h"
#include "ui/graphical/menu/dialogs/dialogok.h"
#include "ui/graphical/menu/windows/windowgamesettings/windowgamesettings.h"
#include "ui/graphical/menu/windows/windowmapselection/windowmapselection.h"
#include "ui/graphical/menu/windows/windowclanselection/windowclanselection.h"
#include "ui/graphical/menu/windows/windowlandingunitselection/windowlandingunitselection.h"
#include "ui/graphical/menu/windows/windowlandingpositionselection/windowlandingpositionselection.h"
#include "ui/graphical/menu/windows/windowload/windowload.h"
#include "ui/graphical/menu/widgets/pushbutton.h"
#include "ui/graphical/application.h"
#include "ui/graphical/game/gamegui.h"
#include "game/startup/local/singleplayer/localsingleplayergamenew.h"
#include "game/startup/local/singleplayer/localsingleplayergamesaved.h"
#include "utility/language.h"
#include "utility/log.h"
#include "network.h"
#include "game/data/player/player.h"
#include "game/data/units/landingunit.h"
#include "settings.h"
#include "game/logic/client.h"
#include "game/logic/server.h"
#include "game/data/savegameinfo.h"

//------------------------------------------------------------------------------
cWindowSinglePlayer::cWindowSinglePlayer() :
	cWindowMain (lngPack.i18n ("Text~Others~Single_Player"))
{
	using namespace std::placeholders;

	auto newGameButton = addChild (std::make_unique<cPushButton> (getPosition() + cPosition (390, 190), ePushButtonType::StandardBig, lngPack.i18n ("Text~Others~Game_New")));
	signalConnectionManager.connect (newGameButton->clicked, std::bind (&cWindowSinglePlayer::newGameClicked, this));

	auto loadGameButton = addChild (std::make_unique<cPushButton> (getPosition() + cPosition (390, 190 + buttonSpace), ePushButtonType::StandardBig, lngPack.i18n ("Text~Others~Game_Load")));
	signalConnectionManager.connect (loadGameButton->clicked, std::bind (&cWindowSinglePlayer::loadGameClicked, this));

	auto backButton = addChild (std::make_unique<cPushButton> (getPosition() + cPosition (415, 190 + buttonSpace * 6), ePushButtonType::StandardSmall, lngPack.i18n ("Text~Others~Back")));
	signalConnectionManager.connect (backButton->clicked, std::bind (&cWindowSinglePlayer::backClicked, this));
}

//------------------------------------------------------------------------------
cWindowSinglePlayer::~cWindowSinglePlayer()
{}

// TODO: find nice place
//------------------------------------------------------------------------------
std::vector<std::pair<sID, int>> createInitialLandingUnitsList(int clan, const cGameSettings& gameSettings, const cUnitsData& unitsData)
{
	std::vector<std::pair<sID, int>> initialLandingUnits;

	if (gameSettings.getBridgeheadType() == eGameSettingsBridgeheadType::Mobile) return initialLandingUnits;

	const auto& constructorID = unitsData.getConstructorData().ID;
	const auto& engineerID = unitsData.getEngineerData().ID;
	const auto& surveyorID = unitsData.getSurveyorData().ID;

	initialLandingUnits.push_back (std::make_pair (constructorID, 40));
	initialLandingUnits.push_back (std::make_pair (engineerID, 20));
	initialLandingUnits.push_back (std::make_pair (surveyorID, 0));

	if (clan == 7)
	{
		const int startCredits = gameSettings.getStartCredits();

		size_t numAddConstructors = 0;
		size_t numAddEngineers = 0;

		if (startCredits < 100)
		{
			numAddEngineers = 1;
		}
		else if (startCredits < 150)
		{
			numAddEngineers = 1;
			numAddConstructors = 1;
		}
		else if (startCredits < 200)
		{
			numAddEngineers = 2;
			numAddConstructors = 1;
		}
		else if (startCredits < 300)
		{
			numAddEngineers = 2;
			numAddConstructors = 2;
		}
		else
		{
			numAddEngineers = 3;
			numAddConstructors = 2;
		}

		for (size_t i = 0; i != numAddConstructors; ++i)
		{
			initialLandingUnits.push_back (std::make_pair (constructorID, 0));
		}
		for (size_t i = 0; i != numAddEngineers; ++i)
		{
			initialLandingUnits.push_back (std::make_pair (engineerID, 0));
		}
	}

	return initialLandingUnits;
}

//------------------------------------------------------------------------------
void cWindowSinglePlayer::newGameClicked()
{
	if (!getActiveApplication()) return;

	auto application = getActiveApplication();

	auto game = std::make_shared<cLocalSingleplayerGameNew> ();

	//initialize copy of unitsData that will be used in game
	game->setUnitsData(std::make_shared<const cUnitsData>(UnitsDataGlobal));
	game->setClanData(std::make_shared<const cClanData>(ClanDataGlobal));

	auto windowGameSettings = getActiveApplication()->show (std::make_shared<cWindowGameSettings> ());
	windowGameSettings->applySettings (cGameSettings());

	windowGameSettings->done.connect ([ = ]()
	{
		auto gameSettings = std::make_shared<cGameSettings> (windowGameSettings->getGameSettings());
		game->setGameSettings (gameSettings);

		auto windowMapSelection = application->show (std::make_shared<cWindowMapSelection> ());

		windowMapSelection->done.connect ([ = ]()
		{
			auto staticMap = std::make_shared<cStaticMap>();
			if (!windowMapSelection->loadSelectedMap (*staticMap))
			{
				// TODO: error dialog: could not load selected map!
				return;
			}
			game->setStaticMap (staticMap);

			if (gameSettings->getClansEnabled())
			{
				auto windowClanSelection = application->show (std::make_shared<cWindowClanSelection> (game->getUnitsData(), game->getClanData()));

				signalConnectionManager.connect (windowClanSelection->canceled, [windowClanSelection]() { windowClanSelection->close(); });
				windowClanSelection->done.connect ([ = ]()
				{
					game->setPlayerClan (windowClanSelection->getSelectedClan());

					auto initialLandingUnits = createInitialLandingUnitsList (windowClanSelection->getSelectedClan(), *gameSettings, *game->getUnitsData());

					auto windowLandingUnitSelection = application->show (std::make_shared<cWindowLandingUnitSelection> (cPlayerColor(), windowClanSelection->getSelectedClan(), initialLandingUnits, gameSettings->getStartCredits(), game->getUnitsData()));

					signalConnectionManager.connect (windowLandingUnitSelection->canceled, [windowLandingUnitSelection]() { windowLandingUnitSelection->close(); });
					windowLandingUnitSelection->done.connect ([ = ]()
					{
						game->setLandingUnits (windowLandingUnitSelection->getLandingUnits());
						game->setUnitUpgrades (windowLandingUnitSelection->getUnitUpgrades());

						bool fixedBridgeHead = gameSettings->getBridgeheadType() == eGameSettingsBridgeheadType::Definite;
						auto landingUnits = windowLandingUnitSelection->getLandingUnits();
						auto unitsdata = game->getUnitsData();
						auto windowLandingPositionSelection = application->show (std::make_shared<cWindowLandingPositionSelection> (staticMap, fixedBridgeHead, landingUnits, unitsdata, false));

						signalConnectionManager.connect (windowLandingPositionSelection->canceled, [windowLandingPositionSelection]() { windowLandingPositionSelection->close(); });
						windowLandingPositionSelection->selectedPosition.connect ([ = ] (cPosition landingPosition)
						{
							game->setLandingPosition (landingPosition);

							game->start (*application);

							windowLandingPositionSelection->close();
							windowLandingUnitSelection->close();
							windowClanSelection->close();
							windowMapSelection->close();
							windowGameSettings->close();
						});
					});
				});
			}
			else
			{
				auto initialLandingUnits = createInitialLandingUnitsList (-1, *gameSettings, *game->getUnitsData());

				auto windowLandingUnitSelection = application->show (std::make_shared<cWindowLandingUnitSelection> (cPlayerColor(), -1, initialLandingUnits, gameSettings->getStartCredits(), game->getUnitsData()));

				signalConnectionManager.connect (windowLandingUnitSelection->canceled, [windowLandingUnitSelection]() { windowLandingUnitSelection->close(); });
				windowLandingUnitSelection->done.connect ([ = ]()
				{
					game->setLandingUnits (windowLandingUnitSelection->getLandingUnits());
					game->setUnitUpgrades (windowLandingUnitSelection->getUnitUpgrades());

					bool fixedBridgeHead = gameSettings->getBridgeheadType() == eGameSettingsBridgeheadType::Definite;
					auto landingUnits = windowLandingUnitSelection->getLandingUnits();
					auto unitsdata = game->getUnitsData();
					auto windowLandingPositionSelection = application->show(std::make_shared<cWindowLandingPositionSelection>(staticMap, fixedBridgeHead, landingUnits, unitsdata, false));

					signalConnectionManager.connect (windowLandingPositionSelection->canceled, [windowLandingPositionSelection]() { windowLandingPositionSelection->close(); });
					windowLandingPositionSelection->selectedPosition.connect ([ = ] (cPosition landingPosition)
					{
						game->setLandingPosition (landingPosition);

						game->start (*application);

						windowLandingPositionSelection->close();
						windowLandingUnitSelection->close();
						windowMapSelection->close();
						windowGameSettings->close();
					});
				});
			}
		});
	});
}

//------------------------------------------------------------------------------
void cWindowSinglePlayer::loadGameClicked()
{
	if (!getActiveApplication()) return;

	auto application = getActiveApplication();

	auto windowLoad = getActiveApplication()->show (std::make_shared<cWindowLoad> ());
	windowLoad->load.connect ([ = ] (const cSaveGameInfo& saveInfo)
	{
		auto game = std::make_shared<cLocalSingleplayerGameSaved> ();
		game->setSaveGameNumber (saveInfo.number);
		try
		{
			game->start(*application);
		}
		catch (const std::runtime_error& e)
		{
			Log.write("Could not start saved game.", cLog::eLOG_TYPE_ERROR);
			Log.write(e.what(), cLog::eLOG_TYPE_ERROR);
			application->show(std::make_shared<cDialogOk>(lngPack.i18n("Text~Error_Messages~ERROR_Save_Loading")));
			return;
		}


		windowLoad->close();
	});
}

//------------------------------------------------------------------------------
void cWindowSinglePlayer::backClicked()
{
	close();
}
