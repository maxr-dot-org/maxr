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

#include "windowsingleplayer.h"
#include "windowgamesettings/gamesettings.h"
#include "windowgamesettings/windowgamesettings.h"
#include "windowmapselection/windowmapselection.h"
#include "windowclanselection/windowclanselection.h"
#include "windowlandingunitselection/windowlandingunitselection.h"
#include "../widgets/pushbutton.h"
#include "../../application.h"

#include "../../../main.h"
#include "../../../network.h"
#include "../../../player.h"
#include "../../../settings.h"
#include "../../../client.h"
#include "../../../server.h"

//------------------------------------------------------------------------------
cWindowSinglePlayer::cWindowSinglePlayer () :
	cWindowMain (lngPack.i18n ("Text~Others~Single_Player"))
{
	using namespace std::placeholders;

	const auto& menuPosition = getArea ().getMinCorner ();

	auto newGameButton = addChild (std::make_unique<cPushButton> (menuPosition + cPosition (390, 190), ePushButtonType::StandardBig, lngPack.i18n ("Text~Others~Game_New")));
	signalConnectionManager.connect (newGameButton->clicked, std::bind (&cWindowSinglePlayer::newGameClicked, this));

	auto loadGameButton = addChild (std::make_unique<cPushButton> (menuPosition + cPosition (390, 190 + buttonSpace), ePushButtonType::StandardBig, lngPack.i18n ("Text~Others~Game_Load")));
	signalConnectionManager.connect (loadGameButton->clicked, std::bind (&cWindowSinglePlayer::loadGameClicked, this));

	auto backButton = addChild (std::make_unique<cPushButton> (menuPosition + cPosition (415, 190 + buttonSpace * 6), ePushButtonType::StandardSmall, lngPack.i18n ("Text~Others~Back")));
	signalConnectionManager.connect (backButton->clicked, std::bind (&cWindowSinglePlayer::backClicked, this));
}

//------------------------------------------------------------------------------
cWindowSinglePlayer::~cWindowSinglePlayer ()
{}

//------------------------------------------------------------------------------
std::vector<std::pair<sID, int>> createInitialLandingUnitsList (const cPlayer& player, const cGameSettings& gameSettings)
{
	std::vector<std::pair<sID, int>> initialLandingUnits;

	if (gameSettings.getBridgeheadType () == eGameSettingsBridgeheadType::Mobile) return initialLandingUnits;

	const auto& constructorID = UnitsData.getConstructorID ();
	const auto& engineerID = UnitsData.getEngineerID ();
	const auto& surveyorID = UnitsData.getSurveyorID ();

	initialLandingUnits.push_back (std::make_pair (constructorID, 40));
	initialLandingUnits.push_back (std::make_pair (engineerID, 20));
	initialLandingUnits.push_back (std::make_pair (surveyorID, 0));

	if (player.getClan () == 7)
	{
		const int startCredits = gameSettings.getStartCredits ();

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

		for (int i = 0; i != numAddConstructors; ++i)
		{
			initialLandingUnits.push_back (std::make_pair (constructorID, 0));
		}
		for (int i = 0; i != numAddEngineers; ++i)
		{
			initialLandingUnits.push_back (std::make_pair (engineerID, 0));
		}
	}

	return initialLandingUnits;
}

//------------------------------------------------------------------------------
void cWindowSinglePlayer::newGameClicked ()
{
	if (!getActiveApplication ()) return;

	auto application = getActiveApplication ();

	auto windowGameSettings = getActiveApplication ()->show (std::make_shared<cWindowGameSettings> ());
	windowGameSettings->applySettings (cGameSettings ());

	windowGameSettings->done.connect ([=]()
	{
		auto windowMapSelection = application->show (std::make_shared<cWindowMapSelection> ());

		windowMapSelection->done.connect ([=]()
		{
			auto staticMap = std::make_shared<cStaticMap>();
			if (!windowMapSelection->loadSelectedMap (*staticMap))
			{
				// TODO: error dialog: could not load selected map!
				return;
			}

			auto player = std::make_shared<cPlayer> (sPlayer (cSettings::getInstance ().getPlayerName (), 0, 0));

			auto gameSettings = std::make_shared<cGameSettings> (windowGameSettings->getGameSettings ());

			if (gameSettings->getClansEnabled ())
			{
				auto windowClanSelection = application->show (std::make_shared<cWindowClanSelection> ());

				windowClanSelection->done.connect ([=]()
				{
					player->setClan (windowClanSelection->getSelectedClan ());

					auto initialLandingUnits = createInitialLandingUnitsList (*player, *gameSettings);

					auto windowLandingUnitSelection = application->show (std::make_shared<cWindowLandingUnitSelection> (*player, initialLandingUnits, gameSettings->getStartCredits ()));

					windowLandingUnitSelection->done.connect ([=]()
					{
						auto landingUnits = windowLandingUnitSelection->getLandingUnits ();
						auto unitUpgrades = windowLandingUnitSelection->getUnitUpgrades ();

						staticMap->clear ();

						// Get data from all windows and start the game

						// ...

						windowLandingUnitSelection->close ();
						windowClanSelection->close ();
						windowMapSelection->close ();
						windowGameSettings->close ();
					});
				});
			}
			else
			{
				auto initialLandingUnits = createInitialLandingUnitsList (*player, *gameSettings);

				auto windowLandingUnitSelection = application->show (std::make_shared<cWindowLandingUnitSelection> (*player, initialLandingUnits, gameSettings->getStartCredits ()));

				windowLandingUnitSelection->done.connect ([=]()
				{
					auto landingUnits = windowLandingUnitSelection->getLandingUnits ();
					auto unitUpgrades = windowLandingUnitSelection->getUnitUpgrades ();

					staticMap->clear ();

					// Get data from all windows and start the game

					// ...

					windowLandingUnitSelection->close ();
					windowMapSelection->close ();
					windowGameSettings->close ();
				});
			}
		});
	});
}

//------------------------------------------------------------------------------
void cWindowSinglePlayer::loadGameClicked ()
{
}

//------------------------------------------------------------------------------
void cWindowSinglePlayer::backClicked ()
{
	close ();
}