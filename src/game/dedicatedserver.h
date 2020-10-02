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

#ifndef game_dedicatedServer_H
#define game_dedicatedServer_H

#include <string>
#include <vector>
#include <memory>

#include "game/connectionmanager.h"

class cDedicatedServerConfig;
class cServerGame;

//------------------------------------------------------------------------
/** cDedicatedServer class manages the server resources and handles command line input.
 *  @author Paul Grathwohl
 */
class cDedicatedServer
{
public:
	static cDedicatedServer& instance();

	void run();

protected:

	enum eHelpCommands
	{
		kHelpUnknownCommand,
		kNotImplementedYet,
		kHelpWrongArguments,
		kHelpHelp,
		kHelpGeneral,
		kHelpNewGame,
		kHelpLoadGame,
		kHelpSaveGame,
		kHelpStop,
		kHelpGames,
		kHelpMaps,
		kHelpSet,
		kHelpExit,
		kHelpPrintConfig
	};
	void printHelp (eHelpCommands) const;
	void printPrompt() const;
	void printConfiguration() const;
	void printGames() const;
	std::string getGamesString() const;
	void printMaps() const;
	std::string getAvailableMapsString() const;

	bool handleInput (const std::string& command);
	void setProperty (const std::string& property, const std::string& value);

	bool startServer (int saveGameNumber = -1);
	void startNewGame (std::shared_ptr<cConnectionManager>, int port);
	void loadSaveGame (std::shared_ptr<cConnectionManager>, int port, int saveGameNumber);
	void saveGame (int saveGameNumber);
	void stopGames();

	std::unique_ptr<cDedicatedServerConfig> configuration;

	std::vector<std::unique_ptr<cServerGame>> games;

private:
	cDedicatedServer();
	~cDedicatedServer();
};

#endif
