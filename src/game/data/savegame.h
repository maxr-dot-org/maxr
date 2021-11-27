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

	int save (const cModel& model, int slot, const std::string& saveName);
	void loadModel (cModel& model, int slot);

	void loadGuiInfo (const cServer* server, int slot, int playerNr = -1);
	void saveGuiInfo (const cNetMessageGUISaveInfo& guiInfo);
	int getLastUsedSaveSlot() const;
private:

	/**
	* Loads header information from old save file with version <1.0.
	* Only loading of header information is supported, to be able to display
	* these save files in the multiplayer menu. Loading game state from
	* savefiles <1.0 is not supported.
	*/
	void loadLegacyHeader (cSaveGameInfo& info);
	void writeHeader (int slot, const std::string& saveName, const cModel &model);
	bool loadDocument (int slot);
	bool loadVersion (cVersion& version);

	int saveingID = -1; //identifier number, to make sure the gui info from clients are written to the correct save file
	int loadedSlot = -1;

	tinyxml2::XMLDocument xmlDocument;
};

void fillSaveGames (std::size_t minIndex, std::size_t maxIndex, std::vector<cSaveGameInfo>&);

#endif //game_data_savegameH
