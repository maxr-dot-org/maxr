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
#ifndef menueventsH
#define menueventsH

#include "network.h"
#include "menus.h"

enum eMenuMessages
{
	MU_MSG_CHAT = FIRST_MENU_MESSAGE,	// simple text message
	MU_MSG_NEW_PLAYER,			// a new player has connected
	MU_MSG_REQ_IDENTIFIKATION,	// host requests a identifacation of this player
	MU_MSG_IDENTIFIKATION,		// player send his idenetification
	MU_MSG_DEL_PLAYER,			// a player should be deleted
	MU_MSG_PLAYERLIST,			// a list with all players and their data
	MU_MSG_OPTINS,				// all options selected by the host
	MU_MSG_GO,					// host wants to start the game
	MU_MSG_LANDING_VEHICLES,	// the list of purcased vehicles
	MU_MSG_UPGRADES,			// data of upgraded units
	GAME_EV_REQ_IDENT,			// a server of a running game requests an identification
	GAME_EV_OK_RECONNECT,		// a server gives his ok to the reconnect
	//messages for the landing selectiong menu:
	MU_MSG_LANDING_COORDS,		// the selected landing coords of a client
	MU_MSG_RESELECT_LANDING,	// informs a client that the player has to reselect the landing site
	MU_MSG_ALL_LANDED,			// all players have selcted there landing points and clients can start game
};

void sendMenuChatMessage ( string chatMsg, sMenuPlayer *player = NULL );

void sendRequestIdentification ( sMenuPlayer *player );

void sendPlayerList ( cList<sMenuPlayer*> *players );

void sendGameData ( cGameDataContainer *gameData, string saveGameString, sMenuPlayer *player = NULL );

void sendGo ();

void sendIdentification ( sMenuPlayer *player );

void sendLandingUnits ( cList<sLandingUnit> *landingList, int ownerNr );

void sendUnitUpgrades ( cPlayer *player );

void sendLandingCoords ( sClientLandData& c, int ownerNr );

void sendReselectLanding ( eLandingState state, int playerNr );

void sendAllLanded ();

void sendGameIdentification ( sMenuPlayer *player, int socket );

void sendReconnectionSuccess( int playerNr );

#endif // menueventsH