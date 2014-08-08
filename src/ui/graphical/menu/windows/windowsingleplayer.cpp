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
#include "ui/graphical/menu/windows/windowscenario/windowscenario.h"
#include "ui/graphical/menu/windows/windowgamesettings/gamesettings.h"
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
#include "game/startup/local/scenario/localscenariogame.h"

#include "main.h"
#include "network.h"
#include "game/data/player/player.h"
#include "game/data/units/landingunit.h"
#include "settings.h"
#include "game/logic/client.h"
#include "game/logic/server.h"
#include "game/logic/clientevents.h"

#include "lua/lua.hpp"
#include "game/startup/local/scenario/luaposition.h"
#include "game/startup/local/scenario/luagame.h"
#include "game/startup/local/scenario/luaplayer.h"
#include "game/startup/local/scenario/luasettings.h"
#include "utility/files.h"
#include "utility/log.h"

//------------------------------------------------------------------------------
cWindowSinglePlayer::cWindowSinglePlayer () :
	cWindowMain (lngPack.i18n ("Text~Others~Single_Player"))
{
	using namespace std::placeholders;

	auto newGameButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (390, 190), ePushButtonType::StandardBig, lngPack.i18n ("Text~Others~Game_New")));
	signalConnectionManager.connect (newGameButton->clicked, std::bind (&cWindowSinglePlayer::newGameClicked, this));

    auto newScenarioButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (390, 190 + buttonSpace), ePushButtonType::StandardBig, lngPack.i18n ("Text~Others~Game_NewScenario")));
    signalConnectionManager.connect (newScenarioButton->clicked, std::bind (&cWindowSinglePlayer::newScenarioGameClicked, this));

    auto loadGameButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (390, 190 + 2 * buttonSpace), ePushButtonType::StandardBig, lngPack.i18n ("Text~Others~Game_Load")));
	signalConnectionManager.connect (loadGameButton->clicked, std::bind (&cWindowSinglePlayer::loadGameClicked, this));

	auto backButton = addChild (std::make_unique<cPushButton> (getPosition () + cPosition (415, 190 + buttonSpace * 6), ePushButtonType::StandardSmall, lngPack.i18n ("Text~Others~Back")));
	signalConnectionManager.connect (backButton->clicked, std::bind (&cWindowSinglePlayer::backClicked, this));
}

//------------------------------------------------------------------------------
cWindowSinglePlayer::~cWindowSinglePlayer ()
{}

// TODO: find nice place
//------------------------------------------------------------------------------
std::vector<std::pair<sID, int>> createInitialLandingUnitsList (int clan, const cGameSettings& gameSettings)
{
	std::vector<std::pair<sID, int>> initialLandingUnits;

	if (gameSettings.getBridgeheadType () == eGameSettingsBridgeheadType::Mobile) return initialLandingUnits;

	const auto& constructorID = UnitsData.getConstructorID ();
	const auto& engineerID = UnitsData.getEngineerID ();
	const auto& surveyorID = UnitsData.getSurveyorID ();

	initialLandingUnits.push_back (std::make_pair (constructorID, 40));
	initialLandingUnits.push_back (std::make_pair (engineerID, 20));
	initialLandingUnits.push_back (std::make_pair (surveyorID, 0));

	if (clan == 7)
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
void cWindowSinglePlayer::newGameClicked ()
{
	if (!getActiveApplication ()) return;

	auto application = getActiveApplication ();

	auto game = std::make_shared<cLocalSingleplayerGameNew> ();

	auto windowGameSettings = getActiveApplication ()->show (std::make_shared<cWindowGameSettings> ());
	windowGameSettings->applySettings (cGameSettings ());

	windowGameSettings->done.connect ([=]()
	{
		auto gameSettings = std::make_shared<cGameSettings> (windowGameSettings->getGameSettings ());
		game->setGameSettings (gameSettings);

		auto windowMapSelection = application->show (std::make_shared<cWindowMapSelection> ());

		windowMapSelection->done.connect ([=]()
		{
			auto staticMap = std::make_shared<cStaticMap>();
			if (!windowMapSelection->loadSelectedMap (*staticMap))
			{
				// TODO: error dialog: could not load selected map!
				return;
			}
			game->setStaticMap (staticMap);

			if (gameSettings->getClansEnabled ())
			{
				auto windowClanSelection = application->show (std::make_shared<cWindowClanSelection> ());

				signalConnectionManager.connect (windowClanSelection->canceled, [windowClanSelection]() { windowClanSelection->close (); });
				windowClanSelection->done.connect ([=]()
				{
					game->setPlayerClan (windowClanSelection->getSelectedClan ());

					auto initialLandingUnits = createInitialLandingUnitsList (windowClanSelection->getSelectedClan (), *gameSettings);

					auto windowLandingUnitSelection = application->show (std::make_shared<cWindowLandingUnitSelection> (cPlayerColor(), windowClanSelection->getSelectedClan (), initialLandingUnits, gameSettings->getStartCredits ()));

					signalConnectionManager.connect (windowLandingUnitSelection->canceled, [windowLandingUnitSelection]() { windowLandingUnitSelection->close (); });
					windowLandingUnitSelection->done.connect ([=]()
					{
						game->setLandingUnits (windowLandingUnitSelection->getLandingUnits ());
						game->setUnitUpgrades (windowLandingUnitSelection->getUnitUpgrades ());

						auto windowLandingPositionSelection = application->show (std::make_shared<cWindowLandingPositionSelection> (staticMap));

						signalConnectionManager.connect (windowLandingPositionSelection->canceled, [windowLandingPositionSelection]() { windowLandingPositionSelection->close (); });
						windowLandingPositionSelection->selectedPosition.connect ([=](cPosition landingPosition)
						{
							game->setLandingPosition (landingPosition);

							game->start (*application);

							windowLandingPositionSelection->close ();
							windowLandingUnitSelection->close ();
							windowClanSelection->close ();
							windowMapSelection->close ();
							windowGameSettings->close ();
						});
					});
				});
			}
			else
			{
				auto initialLandingUnits = createInitialLandingUnitsList (-1, *gameSettings);

				auto windowLandingUnitSelection = application->show (std::make_shared<cWindowLandingUnitSelection> (cPlayerColor(), -1, initialLandingUnits, gameSettings->getStartCredits ()));

				signalConnectionManager.connect (windowLandingUnitSelection->canceled, [windowLandingUnitSelection]() { windowLandingUnitSelection->close (); });
				windowLandingUnitSelection->done.connect ([=]()
				{
					game->setLandingUnits (windowLandingUnitSelection->getLandingUnits ());
					game->setUnitUpgrades (windowLandingUnitSelection->getUnitUpgrades ());

					auto windowLandingPositionSelection = application->show (std::make_shared<cWindowLandingPositionSelection> (staticMap));

					signalConnectionManager.connect (windowLandingPositionSelection->canceled, [windowLandingPositionSelection]() { windowLandingPositionSelection->close (); });
					windowLandingPositionSelection->selectedPosition.connect ([=](cPosition landingPosition)
					{
						game->setLandingPosition (landingPosition);

						game->start (*application);

						windowLandingPositionSelection->close ();
						windowLandingUnitSelection->close ();
						windowMapSelection->close ();
						windowGameSettings->close ();
					});
				});
			}
		});
	});
}

void cWindowSinglePlayer::newScenarioGameClicked()
{
    if (!getActiveApplication ()) return;
    auto application = getActiveApplication ();

    auto windowScenario = application->show (std::make_shared<cWindowScenario>());
    windowScenario->done.connect ([=]() {
        // Load scenario file
        std::string luaFilename = windowScenario->getSelectedScenario();
        std::string fullFilename = cSettings::getInstance().getScenariosPath() + PATH_DELIMITER + luaFilename;
        SDL_RWops* fpLuaFile = SDL_RWFromFile(fullFilename.c_str(), "rb");
        if (fpLuaFile == NULL)
        {
            // now try in the user's map directory
            std::string userMapsDir = getUserMapsDir();
            if (!userMapsDir.empty())
            {
                fullFilename = userMapsDir + luaFilename;
                fpLuaFile = SDL_RWFromFile (fullFilename.c_str(), "rb");
            }
        }
        if (fpLuaFile == NULL)
        {
            Log.write("Cannot load scenario file: \"" + luaFilename + "\"", cLog::eLOG_TYPE_ERROR);
            return;
        }
        Sint64 fileSize = SDL_RWsize(fpLuaFile);
        char* luaData = new char[fileSize];
        SDL_RWread(fpLuaFile, luaData, 1, fileSize);
        SDL_RWclose (fpLuaFile);

        // Create a lua context and run Lua interpreter
        lua_State* L = luaL_newstate();
        luaL_openlibs(L);
        Lunar<LuaPosition>::Register(L);
        Lunar<LuaGame>::Register(L);
        Lunar<LuaPlayer>::Register(L);
        Lunar<LuaSettings>::Register(L);

        // Create the game in a global variable "game"
        LuaGame* luaGame = new LuaGame(application);
        Lunar<LuaGame>::push(L, luaGame);               // luaGame has to be deleted from C++
        lua_setglobal(L, "game");

        // Load the Lua script
        int error = luaL_loadbuffer(L, luaData, fileSize, "luaScenario") || lua_pcall(L, 0, 0, 0);
        if (error) {
            Log.write("Cannot run scenario file, check lua syntax: \"" + luaFilename + "\"" +
                      "\nLua Error: \n" + lua_tostring(L, -1), cLog::eLOG_TYPE_ERROR);
            lua_pop(L, 1);
        }
        else {
            // Load scenario name from global variable of the lua script
            std::string scenarioName = "Unknown";
            lua_getglobal (L, "scenarioName");                              // Push global scenarioName var on the stack
            if (lua_isstring(L, 1)) scenarioName = lua_tostring(L, 1);      // Retreive the value
            lua_pop(L, 1);                                                  // Pop the value from the stack
            Log.write("Scenario loaded successfully : " + scenarioName, cLog::eLOG_TYPE_INFO);
        }

        // Close lua context
        lua_close(L);
        delete[] luaData;

        // TODO_M: when to delete luaGame ?
        // Maybe keep it living for more AI interaction during the game !

        windowScenario->close();
    });
}

//------------------------------------------------------------------------------
void cWindowSinglePlayer::loadGameClicked ()
{
    // TODO_M: Handle scenarios save & restore !
    // Maybe add a game type to be able to load correctly the game through the same button
    if (!getActiveApplication ()) return;

	auto application = getActiveApplication ();

	auto windowLoad = getActiveApplication ()->show (std::make_shared<cWindowLoad> ());
	windowLoad->load.connect ([=](int saveGameNumber)
	{
		auto game = std::make_shared<cLocalSingleplayerGameSaved> ();
		game->setSaveGameNumber (saveGameNumber);
		game->start (*application);

		windowLoad->close ();
	});
}

//------------------------------------------------------------------------------
void cWindowSinglePlayer::backClicked ()
{
	close ();
}
