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

/* Author: Paul Grathwohl */

#include "casualtiestracker.h"
#include "netmessage.h"
#include "tinyxml.h"

using namespace std;

//--------------------------------------------------------------------------
void cCasualtiesTracker::initFromXML (TiXmlElement* casualtiesNode)
{
	casualtiesPerPlayer.clear();

	TiXmlElement* playerNode = casualtiesNode->FirstChildElement ("PlayerCasualties");
	while (playerNode != 0)
	{
		int playerNr = 0;
		if (playerNode->Attribute ("PlayerNr", &playerNr) != 0)
		{
			TiXmlElement* casualtyNode = playerNode->FirstChildElement ("Casualty");
			while (casualtyNode != 0)
			{
				sID unitID;
				int losses;
				if (casualtyNode->Attribute ("ID_Fst", & (unitID.iFirstPart)) != 0
					&& casualtyNode->Attribute ("ID_Snd", & (unitID.iSecondPart)) != 0
					&& casualtyNode->Attribute ("Losses", & (losses)) != 0)
				{
					setCasualty (unitID, losses, playerNr);
				}
				casualtyNode = casualtyNode->NextSiblingElement ("Casualty");
			}
		}
		playerNode = playerNode->NextSiblingElement ("PlayerCasualties");
	}
}

//--------------------------------------------------------------------------
void cCasualtiesTracker::storeToXML (TiXmlElement* casualtiesNode) const
{
	// add sub elements for every player that contain all his casualties
	for (size_t i = 0; i < casualtiesPerPlayer.size(); i++)
	{
		const CasualtiesOfPlayer& casualtiesOfPlayer = casualtiesPerPlayer[i];

		TiXmlElement* playerNode = new TiXmlElement ("PlayerCasualties");
		casualtiesNode->LinkEndChild (playerNode);  // playerNode is now owned by casualtiesNode
		playerNode->SetAttribute ("PlayerNr", iToStr (casualtiesOfPlayer.playerNr).c_str());

		// add sub elements for each casualty of the current player
		for (size_t j = 0; j < casualtiesOfPlayer.casualties.size(); j++)
		{
			const Casualty& casualty = casualtiesOfPlayer.casualties[j];

			TiXmlElement* casualtyNode = new TiXmlElement ("Casualty");
			playerNode->LinkEndChild (casualtyNode);  // casualtyNode is now owned by playerNode
			casualtyNode->SetAttribute ("ID_Fst", iToStr (casualty.unitID.iFirstPart).c_str());
			casualtyNode->SetAttribute ("ID_Snd", iToStr (casualty.unitID.iSecondPart).c_str());
			casualtyNode->SetAttribute ("Losses", iToStr (casualty.numberOfLosses).c_str());
		}
	}
}

//--------------------------------------------------------------------------
void cCasualtiesTracker::logCasualty (sID unitType, int playerNr)
{
	setCasualty (unitType, getCasualtiesOfUnitType (unitType, playerNr) + 1, playerNr);
}

//--------------------------------------------------------------------------
void cCasualtiesTracker::setCasualty (sID unitType, int numberOfLosses, int playerNr)
{
	vector<Casualty>& casualties = getCasualtiesOfPlayer (playerNr);

	for (unsigned int i = 0; i < casualties.size(); i++)
	{
		if (unitType == casualties[i].unitID)
		{
			casualties[i].numberOfLosses = numberOfLosses;
			return;
		}
	}
	Casualty newCasualtyEntry;
	newCasualtyEntry.numberOfLosses = numberOfLosses;
	newCasualtyEntry.unitID = unitType;
	for (unsigned int i = 0; i < casualties.size(); i++)
	{
		if (unitType.iFirstPart < casualties[i].unitID.iFirstPart
			|| (unitType.iFirstPart == casualties[i].unitID.iFirstPart && unitType.iSecondPart < casualties[i].unitID.iSecondPart))
		{
			vector<Casualty>::iterator it = casualties.begin();
			casualties.insert (it + i, newCasualtyEntry);
			return;
		}
	}
	casualties.push_back (newCasualtyEntry);
}

//--------------------------------------------------------------------------
int cCasualtiesTracker::getCasualtiesOfUnitType (sID unitType, int playerNr)
{
	const vector<Casualty>& casualties = getCasualtiesOfPlayer (playerNr);
	for (unsigned int i = 0; i < casualties.size(); i++)
	{
		if (unitType == casualties[i].unitID)
			return casualties[i].numberOfLosses;
	}
	return 0;
}

//--------------------------------------------------------------------------
vector<sID> cCasualtiesTracker::getUnitTypesWithLosses() const
{
	vector<sID> result;

	for (unsigned int i = 0; i < casualtiesPerPlayer.size(); i++)
	{
		const vector<Casualty>& casualties = casualtiesPerPlayer[i].casualties;
		for (unsigned int entryIdx = 0; entryIdx < casualties.size(); entryIdx++)
		{
			const Casualty& casualty = casualties[entryIdx];
			bool containedInResult = false;
			for (unsigned int j = 0; j < result.size(); j++)
			{
				if (result[j] == casualty.unitID)
				{
					containedInResult = true;
					break;
				}
			}
			if (containedInResult == false)
			{
				bool inserted = false;
				for (unsigned int j = 0; j < result.size(); j++)
				{
					const int firstPart = casualty.unitID.iFirstPart;
					const int secondPart = casualty.unitID.iSecondPart;

					if ( (firstPart == 1 && result[j].iFirstPart == 0)   // 0: vehicle, 1: building; buildings should be inserted first
						 || (firstPart == result[j].iFirstPart && secondPart < result[j].iSecondPart))
					{
						result.insert (result.begin() + j, casualty.unitID);
						inserted = true;
						break;
					}
				}
				if (inserted == false)
					result.push_back (casualty.unitID);
			}
		}
	}
	return result;
}

//--------------------------------------------------------------------------
void cCasualtiesTracker::debugPrint()
{
	for (unsigned int i = 0; i < casualtiesPerPlayer.size(); i++)
	{
		Log.write ("Casualties of Player: " + iToStr (casualtiesPerPlayer[i].playerNr), cLog::eLOG_TYPE_DEBUG);

		const vector<Casualty>& casualties = casualtiesPerPlayer[i].casualties;
		for (unsigned int entryIdx = 0; entryIdx < casualties.size(); entryIdx++)
		{
			const sUnitData* unitData = casualties[entryIdx].unitID.getUnitDataOriginalVersion();
			if (unitData != NULL)
				Log.write ("  " + unitData->name + ": " + iToStr (casualties[entryIdx].numberOfLosses), cLog::eLOG_TYPE_DEBUG);
			else
				Log.write ("Invalid Casualty: Can't get unitData from sID", cLog::eLOG_TYPE_DEBUG);
		}
	}
}

//--------------------------------------------------------------------------
void cCasualtiesTracker::updateCasualtiesFromNetMessage (cNetMessage* message)
{
	if (message == NULL)
		return;
	const int dataSetsInMessage = message->popInt16();
	for (int dataSet = 0; dataSet < dataSetsInMessage; dataSet++)
	{
		const int playerNr = message->popInt16();
		const int nrCasualtyReports = message->popInt16();
		for (int i = 0; i < nrCasualtyReports; i++)
		{
			sID unitType;
			unitType.iFirstPart = message->popInt16();
			unitType.iSecondPart = message->popInt16();
			const int numberLosses = message->popInt32();
			setCasualty (unitType, numberLosses, playerNr);
		}
	}

	notifyListeners ("casualties tracker updated");
}

//--------------------------------------------------------------------------
void cCasualtiesTracker::prepareNetMessagesForClient (cList<cNetMessage*>& messages, int msgType)
{
	cNetMessage* message = 0;
	int entriesInMessageForPlayer = 0;
	int dataSetsInMessage = 0;
	for (unsigned int i = 0; i < casualtiesPerPlayer.size(); i++)
	{
		if (entriesInMessageForPlayer > 0)
			dataSetsInMessage++;
		const int currentPlayer = casualtiesPerPlayer[i].playerNr;
		entriesInMessageForPlayer = 0;
		vector<Casualty>& casualties = casualtiesPerPlayer[i].casualties;
		for (unsigned int entryIdx = 0; entryIdx < casualties.size(); entryIdx++)
		{
			if (message == 0)
			{
				message = new cNetMessage (msgType);
				entriesInMessageForPlayer = 0;
				dataSetsInMessage = 1;
			}

			message->pushInt32 (casualties[entryIdx].numberOfLosses);
			message->pushInt16 (casualties[entryIdx].unitID.iSecondPart);
			message->pushInt16 (casualties[entryIdx].unitID.iFirstPart);
			entriesInMessageForPlayer++;

			if (message->iLength + 4 + 4 + 8 > PACKAGE_LENGTH)
			{
				message->pushInt16 (entriesInMessageForPlayer);
				message->pushInt16 (currentPlayer);
				message->pushInt16 (dataSetsInMessage);
				entriesInMessageForPlayer = 0;
				messages.Add (message);
				message = 0;
			}
		}
		if (message != 0 && entriesInMessageForPlayer > 0)
		{
			message->pushInt16 (entriesInMessageForPlayer);
			message->pushInt16 (currentPlayer);
		}
	}
	if (message != 0)
	{
		message->pushInt16 (dataSetsInMessage);
		messages.Add (message);
	}
}

//--------------------------------------------------------------------------
vector<cCasualtiesTracker::Casualty>& cCasualtiesTracker::getCasualtiesOfPlayer (int playerNr)
{
	for (unsigned int i = 0; i < casualtiesPerPlayer.size(); i++)
	{
		if (casualtiesPerPlayer[i].playerNr == playerNr)
			return casualtiesPerPlayer[i].casualties;
	}

	CasualtiesOfPlayer newCasualtiesOfPlayer;
	newCasualtiesOfPlayer.playerNr = playerNr;
	for (unsigned int i = 0; i < casualtiesPerPlayer.size(); i++)
	{
		if (playerNr < casualtiesPerPlayer[i].playerNr)
		{
			vector<CasualtiesOfPlayer>::iterator it = casualtiesPerPlayer.begin();
			casualtiesPerPlayer.insert (it + i, newCasualtiesOfPlayer);
			return casualtiesPerPlayer[i].casualties;
		}
	}
	casualtiesPerPlayer.push_back (newCasualtiesOfPlayer);
	return casualtiesPerPlayer[casualtiesPerPlayer.size() - 1].casualties;
}
