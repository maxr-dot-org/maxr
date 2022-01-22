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

#ifndef game_data_savegameH
#define game_data_savegameH

#include <string>
#include <vector>

#include <config/workaround/cpp17/optional.h>

#include <3rd/tinyxml2/tinyxml2.h>

class cModel;
class cSaveGameInfo;
class cVersion;
class cNetMessageGUISaveInfo;
class cServer;

class cSavegame
{
public:
	cSavegame() = default;

	static std::string getFileName (int slot);

	cSaveGameInfo loadSaveInfo (int slot);

	void save (const cModel& model, int slot, const std::string& saveName);
	void loadModel (cModel& model, int slot);

	void loadGuiInfo (const cServer* server, int slot, int playerNr = -1);
	void saveGuiInfo (const cNetMessageGUISaveInfo& guiInfo);

private:

	void writeHeader (tinyxml2::XMLDocument&, int slot, const std::string& saveName, const cModel &model);
	bool loadDocument (tinyxml2::XMLDocument&, int slot);
	std::optional<cVersion> loadVersion (const tinyxml2::XMLDocument&, int slot);
};

void fillSaveGames (std::size_t minIndex, std::size_t maxIndex, std::vector<cSaveGameInfo>&);

#endif //game_data_savegameH
