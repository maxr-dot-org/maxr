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

#include "savegame.h"

#include "defines.h"
#include "game/data/gamesettings.h"
#include "game/data/model.h"
#include "game/data/player/player.h"
#include "game/data/savegameinfo.h"
#include "game/logic/server.h"
#include "game/logic/turntimeclock.h"
#include "game/protocol/netmessage.h"
#include "game/serialization/serialization.h"
#include "game/serialization/xmlarchive.h"
#include "maxrversion.h"
#include "settings.h"
#include "utility/extendedtinyxml.h"
#include "utility/files.h"
#include "utility/log.h"
#include "utility/ranges.h"

#include <ctime>
#include <regex>

#define SAVE_FORMAT_VERSION ((std::string)"1.0")

//------------------------------------------------------------------------------
void cSavegame::save (const cModel& model, int slot, const std::string& saveName)
{
	loadedSlot = -1;

	writeHeader (slot, saveName, model);

	cXmlArchiveIn archive (*xmlDocument.RootElement());
	archive << NVP (model);

	// add model crc
	archive.openNewChild ("model");
	archive << serialization::makeNvp ("modelcrc", model.getChecksum());
	archive.closeChild();

	loadedSlot = slot;
	makeDirectories (cSettings::getInstance().getSavesPath());
	tinyxml2::XMLError result = xmlDocument.SaveFile (getFileName (slot).c_str());
	if (result != tinyxml2::XML_NO_ERROR)
	{
		throw std::runtime_error (getXMLErrorMsg (xmlDocument));
	}
	if (cSettings::getInstance().isDebug()) // Check Save/Load consistency
	{
		cModel model2;

		loadModel (model2, slot);

		if (model.getChecksum() != model2.getChecksum())
		{
			Log.write ("Checksum issue when saving", cLog::eLOG_TYPE_ERROR);
		}
	}
}

//------------------------------------------------------------------------------
void cSavegame::saveGuiInfo (const cNetMessageGUISaveInfo& guiInfo)
{
	cXmlArchiveIn archive (*xmlDocument.RootElement());
	archive.openNewChild ("GuiInfo");

	archive << serialization::makeNvp ("playerNr", guiInfo.playerNr);
	archive << serialization::makeNvp ("guiState", guiInfo.guiInfo);
	archive.closeChild();

	makeDirectories (cSettings::getInstance().getSavesPath());
	tinyxml2::XMLError result = xmlDocument.SaveFile (getFileName (loadedSlot).c_str());
	if (result != tinyxml2::XML_NO_ERROR)
	{
		Log.write ("Writing savegame file failed: " + getXMLErrorMsg (xmlDocument), cLog::eLOG_TYPE_NET_WARNING);
	}
}

//------------------------------------------------------------------------------
cSaveGameInfo cSavegame::loadSaveInfo (int slot)
{
	cSaveGameInfo info (slot);

	if (!loadDocument (slot))
	{
		info.gameName = "XML Error";
		return info;
	}

	if (!loadVersion (info.saveVersion))
	{
		info.gameName = "XML Error";
		return info;
	}

	try
	{
		cXmlArchiveOut archive (*xmlDocument.RootElement());
		archive.enterChild ("header");
		archive >> serialization::makeNvp ("gameVersion", info.gameVersion);
		archive >> serialization::makeNvp ("gameName", info.gameName);
		archive >> serialization::makeNvp ("type", info.type);
		archive >> serialization::makeNvp ("date", info.date);
		archive.leaveChild(); // header

		archive.enterChild ("model");
		archive.enterChild ("players");
		int numPlayers = 0;
		archive >> serialization::makeNvp ("length", numPlayers);
		info.players.resize (numPlayers);
		for (int i = 0; i < numPlayers; i++)
		{
			archive.enterChild ("item");
			sPlayerSettings player;
			int id;
			bool isDefeated;

			archive >> NVP (player);
			archive >> NVP (id);
			archive >> NVP (isDefeated);

			info.players[i] = cPlayerBasicData (player, id, isDefeated);

			archive.leaveChild(); // item
		}
		archive.leaveChild(); // players

		archive.enterChild ("map");
		archive.enterChild ("mapFile");
		archive >> serialization::makeNvp ("filename", info.mapName);
		archive >> serialization::makeNvp ("crc", info.mapCrc);
		archive.leaveChild(); // mapFile
		archive.leaveChild(); // map

		archive.enterChild ("turnCounter");
		archive >> serialization::makeNvp ("turn", info.turn);
		archive.leaveChild(); //turnCounter

		archive.leaveChild(); // model
	}
	catch (const std::runtime_error& e)
	{
		Log.write ("Error loading savegame file " + std::to_string (slot) + ": " + e.what(), cLog::eLOG_TYPE_ERROR);
		info.gameName = "XML Error";
		return info;
	}
	return info;
}

//------------------------------------------------------------------------------
std::string cSavegame::getFileName (int slot)
{
	char numberstr[4];
	snprintf (numberstr, sizeof (numberstr), "%.3d", slot);
	return cSettings::getInstance().getSavesPath() + PATH_DELIMITER + "Save" + numberstr + ".xml";
}

//------------------------------------------------------------------------------
void cSavegame::writeHeader (int slot, const std::string& saveName, const cModel &model)
{
	//init document
	loadedSlot = -1;
	xmlDocument.Clear();
	xmlDocument.LinkEndChild (xmlDocument.NewDeclaration());
	tinyxml2::XMLElement* rootnode = xmlDocument.NewElement ("MAXR_SAVE_FILE");
	rootnode->SetAttribute ("version", (SAVE_FORMAT_VERSION).c_str());
	xmlDocument.LinkEndChild (rootnode);

	//write header
	char timestr[21];
	time_t tTime = time (nullptr);
	tm* tmTime = localtime (&tTime);
	strftime (timestr, 21, "%d.%m.%y %H:%M", tmTime);

	eGameType type = eGameType::Single;
	int humanPlayers = 0;
	for (auto player : model.getPlayerList())
	{
		if (player->isHuman())
			humanPlayers++;
	}
	if (humanPlayers > 1)
		type = eGameType::TcpIp;
	if (model.getGameSettings()->gameType == eGameSettingsGameType::HotSeat)
		type = eGameType::Hotseat;

	cXmlArchiveIn archive (*xmlDocument.RootElement());
	archive.openNewChild ("header");
	archive << serialization::makeNvp ("gameVersion", std::string (PACKAGE_VERSION  " "  PACKAGE_REV));
	archive << serialization::makeNvp ("gameName", saveName);
	archive << serialization::makeNvp ("type", type);
	archive << serialization::makeNvp ("date", std::string (timestr));
	archive.closeChild();
}

//------------------------------------------------------------------------------
void cSavegame::loadModel (cModel& model, int slot)
{
	if (!loadDocument (slot))
	{
		throw std::runtime_error ("Could not load savegame file " + std::to_string (slot));
	}

	cVersion saveVersion;
	if (!loadVersion (saveVersion))
	{
		throw std::runtime_error ("Could not load version info from savegame file " + std::to_string (slot));
	}

	if (saveVersion < cVersion (1, 0))
	{
		throw std::runtime_error ("Savegame version is not compatible. Versions < 1.0 are not supported.");
	}

	serialization::cPointerLoader loader (model);
	cXmlArchiveOut archive (*xmlDocument.RootElement(), &loader);
	archive >> NVP (model);

	// check crc
	uint32_t crcFromSave;
	archive.enterChild ("model");
	archive >> serialization::makeNvp ("modelcrc", crcFromSave);
	archive.leaveChild();
	Log.write (" Checksum from save file: " + std::to_string (crcFromSave), cLog::eLOG_TYPE_NET_DEBUG);

	uint32_t modelCrc = model.getChecksum();
	Log.write (" Checksum after loading model: " + std::to_string (modelCrc), cLog::eLOG_TYPE_NET_DEBUG);
	Log.write (" GameId: " + std::to_string (model.getGameId()), cLog::eLOG_TYPE_NET_DEBUG);

	if (crcFromSave != modelCrc)
	{
		Log.write (" Crc of loaded model does not match the saved crc!", cLog::eLOG_TYPE_NET_ERROR);
		//TODO: what to do in this case?
	}
}

//------------------------------------------------------------------------------
void cSavegame::loadGuiInfo (const cServer* server, int slot, int playerNr)
{
	if (!loadDocument (slot))
	{
		throw std::runtime_error ("Could not load savegame file " + std::to_string (slot));
	}

	tinyxml2::XMLElement* guiInfoElement = xmlDocument.RootElement()->FirstChildElement ("GuiInfo");
	while (guiInfoElement)
	{
		cXmlArchiveOut archive (*guiInfoElement);
		cNetMessageGUISaveInfo guiInfo (slot);

		serialization::makeNvp ("playerNr", guiInfo.playerNr);
		archive >> serialization::makeNvp ("guiState", guiInfo.guiInfo);

		if (guiInfo.playerNr == playerNr || playerNr == -1)
		{
			server->sendMessageToClients (guiInfo, guiInfo.playerNr);
		}

		guiInfoElement = guiInfoElement->NextSiblingElement ("GuiInfo");
	}
}

//------------------------------------------------------------------------------
int cSavegame::getLastUsedSaveSlot() const
{
	return loadedSlot;
}

//------------------------------------------------------------------------------
bool cSavegame::loadDocument (int slot)
{
	if (slot != loadedSlot)
	{
		xmlDocument.Clear();
		loadedSlot = -1;

		const std::string fileName = getFileName (slot);
		if (xmlDocument.LoadFile (fileName.c_str()) != tinyxml2::XML_NO_ERROR)
		{
			Log.write ("Error loading savegame file: " + fileName + ". Reason: " + getXMLErrorMsg (xmlDocument), cLog::eLOG_TYPE_ERROR);
			return false;
		}

		if (!xmlDocument.RootElement() || std::string ("MAXR_SAVE_FILE") != xmlDocument.RootElement()->Name())
		{
			Log.write ("Error loading savegame file: " + fileName + ": Rootnode \"MAXR_SAVE_FILE\" not found.", cLog::eLOG_TYPE_ERROR);
			return false;
		}
		loadedSlot = slot;
	}
	return true;
}

//------------------------------------------------------------------------------
bool cSavegame::loadVersion (cVersion& version)
{
	const char* versionString = xmlDocument.RootElement()->Attribute ("version");
	if (versionString == nullptr)
	{
		Log.write ("Error loading savegame file " + std::to_string (loadedSlot) + ": \"version\" attribute of node \"MAXR_SAVE_FILE\" not found.", cLog::eLOG_TYPE_ERROR);
		return false;
	}
	version.parseFromString (versionString);
	return true;
}

//------------------------------------------------------------------------------
void fillSaveGames (std::size_t minIndex, std::size_t maxIndex, std::vector<cSaveGameInfo>& saveGames)
{
	cSavegame savegame;
	const auto saveFileNames = getFilesOfDirectory (cSettings::getInstance().getSavesPath());
	const std::regex savename_regex{R"(Save(\d{3})\.xml)"};

	for (const auto& filename : saveFileNames)
	{
		std::smatch match;
		if (!std::regex_match (filename, match, savename_regex)) continue;
		const std::size_t number = atoi (match[1].str().c_str());

		if (number <= minIndex || number > maxIndex) continue;

		if (ranges::find_if (saveGames, [=](const cSaveGameInfo& save) { return std::size_t (save.number) == number; }) != saveGames.end()) continue;

		// read the information and add it to the saves list
		cSaveGameInfo saveInfo = savegame.loadSaveInfo (number);
		saveGames.push_back (saveInfo);
	}
}
