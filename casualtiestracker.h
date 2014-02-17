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

#ifndef casualtiestracker_H
#define casualtiestracker_H

#include "main.h"
#include "notifications.h"
#include <vector>

class cNetMessage;
namespace tinyxml2
{
class XMLElement;
}

//-------------------------------------------------------------------------------
class cCasualtiesTracker : public cNotificationSender
{
public:
	cCasualtiesTracker() {}

	void initFromXML (tinyxml2::XMLElement* casualtiesNode);
	void storeToXML (tinyxml2::XMLElement* casualtiesNode) const;

	void logCasualty (sID unitType, int playerNr);
	int getCasualtiesOfUnitType (sID unitType, int playerNr);

	void updateCasualtiesFromNetMessage (cNetMessage* message);
	void prepareNetMessagesForClient (std::vector<cNetMessage*>& messages, int msgType);

	std::vector<sID> getUnitTypesWithLosses() const;

	//-------------------------------------------------------------------------------
private:
	struct Casualty
	{
		sID unitID;
		int numberOfLosses;
	};
	struct CasualtiesOfPlayer
	{
		std::vector<Casualty> casualties;
		int playerNr;
	};
	std::vector<CasualtiesOfPlayer> casualtiesPerPlayer;

	std::vector<Casualty>& getCasualtiesOfPlayer (int playerNr);
	void setCasualty (sID unitType, int numberOfLosses, int playerNr);

	void debugPrint();
};


#endif // casualtiestracker_H
