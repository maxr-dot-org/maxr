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

#include <time.h>

#include "savegame.h"

#include "savegameinfo.h"
#include "model.h"
#include "player/player.h"
#include "gamesettings.h"
#include "maxrversion.h"
#include "utility/serialization/serialization.h"
#include "utility/serialization/xmlarchive.h"
#include "utility/files.h"
#include "utility/log.h"
#include "extendedtinyxml.h"
#include "protocol/netmessage.h"
#include "game/logic/server.h"
#include "utility/string/toString.h"
#include "game/logic/turntimeclock.h"

#define LOAD_ERROR(msg)                        \
	{                                          \
		Log.write(msg, cLog::eLOG_TYPE_ERROR); \
		info.gameName = "XML Error";           \
		return;                                \
	}

cSavegame::cSavegame() :
	saveingID(-1),
	loadedSlot(-1)
{}

int cSavegame::save(const cModel& model, int slot, const std::string& saveName)
{
#if 0 //---serialization test code---
	//write 1st xml archive
	tinyxml2::XMLDocument document1;
	document1.LinkEndChild(document1.NewElement("MAXR_SAVE_FILE"));
	cXmlArchiveIn archive1(*document1.RootElement());
	archive1 << NVP(model);
	document1.SaveFile("test1.xml");

	//write binary
	cBinaryArchiveIn archiveb = cBinaryArchiveIn();
	archiveb << model;

	//load binary
	cModel modelb;
	serialization::cPointerLoader pointerLoaderb(modelb);
	cBinaryArchiveOut archiveb2(archiveb.data(), archiveb.length(), &pointerLoaderb);
	archiveb2 >> modelb;

	//write 2nd xml archive
	tinyxml2::XMLDocument document2;
	document2.LinkEndChild(document2.NewElement("MAXR_SAVE_FILE"));
	cXmlArchiveIn archive2(*document2.RootElement());
	archive2 << serialization::makeNvp("model", modelb);
	document2.SaveFile("test2.xml");
	//both files should now be identical
#endif
	loadedSlot = -1;

	writeHeader(slot, saveName, model);

	cXmlArchiveIn archive(*xmlDocument.RootElement());
	archive << NVP(model);

	// add model crc
	archive.openNewChild("model");
	archive << serialization::makeNvp("modelcrc", model.getChecksum());
	archive.closeChild();

	loadedSlot = slot;
	makeDirectories(cSettings::getInstance().getSavesPath());
	tinyxml2::XMLError result = xmlDocument.SaveFile(getFileName(slot).c_str());
	if (result != tinyxml2::XML_NO_ERROR)
	{
		throw std::runtime_error(getXMLErrorMsg(xmlDocument));
	}
	saveingID++;
	return saveingID;
}

void cSavegame::saveGuiInfo(const cNetMessageGUISaveInfo& guiInfo)
{
	if (saveingID != guiInfo.savingID)
	{
		Log.write("Received GuiSaveInfo with wrong savingID", cLog::eLOG_TYPE_NET_WARNING);
		return;
	}

	cXmlArchiveIn archive(*xmlDocument.RootElement());
	archive.openNewChild("GuiInfo");

	archive << serialization::makeNvp("playerNr", guiInfo.playerNr);
	archive << serialization::makeNvp("guiState", guiInfo.guiState);
	archive << serialization::makeNvp("reportsNr", (int)guiInfo.reports->size());
	for (const auto &report : *guiInfo.reports)
	{
		archive << serialization::makeNvp("report", *report);
	}
	archive << serialization::makeNvp("savedPositions", guiInfo.savedPositions);
	archive << serialization::makeNvp("doneList", guiInfo.doneList);
	archive.closeChild();

	makeDirectories (cSettings::getInstance().getSavesPath());
	tinyxml2::XMLError result = xmlDocument.SaveFile(getFileName(loadedSlot).c_str());
	if (result != tinyxml2::XML_NO_ERROR)
	{
		Log.write("Writing savegame file failed: " + getXMLErrorMsg(xmlDocument), cLog::eLOG_TYPE_NET_WARNING);
	}
}

cSaveGameInfo cSavegame::loadSaveInfo(int slot)
{
	cSaveGameInfo info(slot);

	if (!loadDocument(slot))
	{
		info.gameName = "XML Error";
		return info;
	}

	if (!loadVersion(info.saveVersion))
	{
		info.gameName = "XML Error";
		return info;
	}


	if (info.saveVersion < cVersion("1.0"))
	{
		loadLegacyHeader(info);
		return info;
	}

	try
	{
		cXmlArchiveOut archive(*xmlDocument.RootElement());
		archive.enterChild("header");
		archive >> serialization::makeNvp("gameVersion", info.gameVersion);
		archive >> serialization::makeNvp("gameName", info.gameName);
		archive >> serialization::makeNvp("type", info.type);
		archive >> serialization::makeNvp("date", info.date);
		archive.leaveChild(); // header

		archive.enterChild("model");
		int numPlayers;
		archive >> NVP(numPlayers);
		info.players.resize(numPlayers);
		for (int i = 0; i < numPlayers; i++)
		{
			archive.enterChild("player");
			std::string name;
			cPlayerColor color;
			int id;
			bool isDefeated;

			archive >> NVP(name);
			archive >> NVP(color);
			archive >> NVP(id);
			archive >> NVP(isDefeated);

			info.players[i] = cPlayerBasicData(name, color, id, isDefeated);

			archive.leaveChild(); // player
		}
		archive.enterChild("map");
		archive.enterChild("mapFile");
		archive >> serialization::makeNvp("filename", info.mapName);
		archive >> serialization::makeNvp("crc", info.mapCrc);
		archive.leaveChild(); // mapFile
		archive.leaveChild(); // map
		archive.leaveChild(); // model

		//TODO: load turn

	}
	catch (const std::runtime_error& e)
	{
		Log.write("Error loading savegame file " + iToStr(slot) + ": " + e.what(), cLog::eLOG_TYPE_ERROR);
		info.gameName = "XML Error";
		return info;
	}

	return info;
}

std::string cSavegame::getFileName(int slot)
{
	char numberstr[4];
	TIXML_SNPRINTF(numberstr, sizeof(numberstr), "%.3d", slot);
	return cSettings::getInstance().getSavesPath() + PATH_DELIMITER + "Save" + numberstr + ".xml";
}

void cSavegame::writeHeader(int slot, const std::string& saveName, const cModel &model)
{
	//init document
	loadedSlot = -1;
	xmlDocument.Clear();
	xmlDocument.LinkEndChild(xmlDocument.NewDeclaration());
	tinyxml2::XMLElement* rootnode = xmlDocument.NewElement("MAXR_SAVE_FILE");
	rootnode->SetAttribute("version", (SAVE_FORMAT_VERSION).c_str());
	xmlDocument.LinkEndChild(rootnode);

	//write header
	char timestr[21];
	time_t tTime = time(nullptr);
	tm* tmTime = localtime(&tTime);
	strftime(timestr, 21, "%d.%m.%y %H:%M", tmTime);

	eGameTypes type = GAME_TYPE_SINGLE;
	int humanPlayers = 0;
	for (auto player : model.getPlayerList())
	{
		if (player->isHuman())
			humanPlayers++;
	}
	if (humanPlayers > 1)
		type = GAME_TYPE_TCPIP;
	if (model.getGameSettings()->getGameType() == eGameSettingsGameType::HotSeat)
		type = GAME_TYPE_HOTSEAT;

	cXmlArchiveIn archive(*xmlDocument.RootElement());
	archive.openNewChild("header");
	archive << serialization::makeNvp("gameVersion", std::string(PACKAGE_VERSION  " "  PACKAGE_REV));
	archive << serialization::makeNvp("gameName", saveName);
	archive << serialization::makeNvp("type", type);
	archive << serialization::makeNvp("date", std::string(timestr));
	archive.closeChild();
}


void cSavegame::loadLegacyHeader(cSaveGameInfo& info)
{
	const tinyxml2::XMLElement* headerNode = xmlDocument.RootElement()->FirstChildElement("Header");
	if (headerNode == nullptr)
		LOAD_ERROR("Error loading savegame file " + iToStr(loadedSlot) + ": Node \"Header\" not found");

	//load name
	const tinyxml2::XMLElement* nameNode = headerNode->FirstChildElement("Name");
	if (nameNode == nullptr)
		LOAD_ERROR("Error loading savegame file " + iToStr(loadedSlot) + ": Subnode \"Name\" of Node \"Header\" not found");
	const char* str = nameNode->Attribute("string");
	if (str == nullptr)
		LOAD_ERROR("Error loading savegame file " + iToStr(loadedSlot) + ": Attribute \"String\" of Node \"Name\" not found");
	info.gameName = str;

	//load game type
	const tinyxml2::XMLElement* typeNode = headerNode->FirstChildElement("Type");
	if (typeNode == nullptr)
		LOAD_ERROR("Error loading savegame file " + iToStr(loadedSlot) + ": Subnode \"Type\" of Node \"Header\" not found");
	str = typeNode->Attribute("string");
	if (str == nullptr)
		LOAD_ERROR("Error loading savegame file " + iToStr(loadedSlot) + ": Attribute \"String\" of Node \"Type\" not found");
	std::string gameType = str;
	if (gameType == "IND")
		info.type = GAME_TYPE_SINGLE;
	else if (gameType == "HOT")
		info.type = GAME_TYPE_HOTSEAT;
	else if (gameType == "NET")
		info.type = GAME_TYPE_TCPIP;
	else
		LOAD_ERROR("Error loading savegame file " + iToStr(loadedSlot) + ": unknown game type");

	//load time
	const tinyxml2::XMLElement* timeNode = headerNode->FirstChildElement("Time");
	if (timeNode == nullptr)
		LOAD_ERROR("Error loading savegame file " + iToStr(loadedSlot) + ": Subnode \"Time\" of Node \"Header\" not found");
	str = timeNode->Attribute("string");
	if (str == nullptr)
		LOAD_ERROR("Error loading savegame file " + iToStr(loadedSlot) + ": Attribute \"String\" of Node \"Time\" not found");

	info.date = str;
}

void cSavegame::loadModel(cModel& model, int slot)
{
	if (!loadDocument(slot))
	{
		throw std::runtime_error("Could not load savegame file " + iToStr(slot));
	}

	cVersion saveVersion;
	if (!loadVersion(saveVersion))
	{
		throw std::runtime_error("Could not load version info from savegame file " + iToStr(slot));
	}

	if (saveVersion < cVersion(1, 0))
	{
		throw std::runtime_error("Savegame version is not compatible. Versions < 1.0 are not supported.");
	}

	serialization::cPointerLoader loader(model);
	cXmlArchiveOut archive(*xmlDocument.RootElement(), &loader);
	archive >> NVP(model);

	// check crc
	uint32_t crcFromSave;
	archive.enterChild("model");
	archive >> serialization::makeNvp("modelcrc", crcFromSave);
	archive.leaveChild();
	Log.write(" Checksum from save file: " + toString(crcFromSave), cLog::eLOG_TYPE_NET_DEBUG);

	uint32_t modelCrc = model.getChecksum();
	Log.write(" Checksum after loading model: " + toString(modelCrc), cLog::eLOG_TYPE_NET_DEBUG);
	Log.write(" GameId: " + toString(model.getGameId()), cLog::eLOG_TYPE_NET_DEBUG);

	if (crcFromSave != modelCrc)
	{
		Log.write(" Crc of loaded model does not match the saved crc!", cLog::eLOG_TYPE_NET_ERROR);
		//TODO: what to do in this case?
	}
}

void cSavegame::loadGuiInfo(const cServer* server, int slot, int playerNr)
{
	if (!loadDocument(slot))
	{
		throw std::runtime_error("Could not load savegame file " + iToStr(slot));
	}

	tinyxml2::XMLElement* guiInfoElement = xmlDocument.RootElement()->FirstChildElement("GuiInfo");
	while (guiInfoElement)
	{
		cXmlArchiveOut archive(*guiInfoElement);
		cNetMessageGUISaveInfo guiInfo(slot);

		guiInfo.reports = std::make_shared<std::vector<std::unique_ptr<cSavedReport>>>();

		archive >> serialization::makeNvp("playerNr", guiInfo.playerNr);
		archive >> serialization::makeNvp("guiState", guiInfo.guiState);
		int size;
		archive >> serialization::makeNvp("reportsNr", size);
		guiInfo.reports->resize(size);
		for (auto &report : *guiInfo.reports)
		{
			report = cSavedReport::createFrom(archive, "report");
		}
		archive >> serialization::makeNvp("savedPositions", guiInfo.savedPositions);
		archive >> serialization::makeNvp("doneList", guiInfo.doneList);

		if (guiInfo.playerNr == playerNr || playerNr == -1)
		{
			server->sendMessageToClients(guiInfo, guiInfo.playerNr);
		}

		guiInfoElement = guiInfoElement->NextSiblingElement("GuiInfo");
	}
}

int cSavegame::getLastUsedSaveSlot() const
{
	return loadedSlot;
}

bool cSavegame::loadDocument(int slot)
{

	if (slot != loadedSlot)
	{
		xmlDocument.Clear();
		loadedSlot = -1;

		const std::string fileName = getFileName(slot);
		if (xmlDocument.LoadFile(fileName.c_str()) != tinyxml2::XML_NO_ERROR)
		{
			Log.write("Error loading savegame file: " + fileName + ". Reason: " + getXMLErrorMsg(xmlDocument), cLog::eLOG_TYPE_ERROR);
			return false;
		}

		if (!xmlDocument.RootElement() || std::string("MAXR_SAVE_FILE") != xmlDocument.RootElement()->Name())
		{
			Log.write("Error loading savegame file: " + fileName + ": Rootnode \"MAXR_SAVE_FILE\" not found.", cLog::eLOG_TYPE_ERROR);
			return false;
		}
		loadedSlot = slot;
	}

	return true;
}

bool cSavegame::loadVersion(cVersion& version)
{
	const char* versionString = xmlDocument.RootElement()->Attribute("version");
	if (versionString == nullptr)
	{
		Log.write("Error loading savegame file " + iToStr(loadedSlot) + ": \"version\" attribute of node \"MAXR_SAVE_FILE\" not found.", cLog::eLOG_TYPE_ERROR);
		return false;
	}

	version.parseFromString(versionString);

	return true;
}
