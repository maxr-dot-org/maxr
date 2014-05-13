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

#ifndef game_network_client_networkclientgameH
#define game_network_client_networkclientgameH

#include <memory>
#include <string>

#include "../../game.h"
#include "../../../maxrconfig.h"

class cClient;
class cTCP;

class cNetworkClientGame : public cGame
{
public:
	~cNetworkClientGame ();

	virtual void run () MAXR_OVERRIDE_FUNCTION;

	virtual void save (int saveNumber, const std::string& saveName) MAXR_OVERRIDE_FUNCTION;

	void setNetwork (std::shared_ptr<cTCP> network);
protected:
	std::shared_ptr<cTCP> network;

	std::unique_ptr<cClient> localClient;
};

#endif // game_network_client_networkclientgameH
