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
#ifndef networkH
#define networkH
#include <SDL_net.h>
#include <SDL_thread.h>
#include "defines.h"
#include "main.h"

#define STAT_OPENED		100
#define STAT_CLOSED		101
#define STAT_WAITING	102
#define STAT_CONNECTED	103

#define BUFF_TYP_DATA	200
#define BUFF_TYP_OK		201
#define BUFF_TYP_RESEND	202
#define BUFF_TYP_NEWID	203

enum MSG_TYPE
{
    // IDs der NET-Messages //////////////////////////////////////////////////////
    MSG_CHAT            =   1  , // Chatnachricht
    MSG_ADD_MOVEJOB      =  2  , // Einen Movejob hinzufügen
    MSG_MOVE_VEHICLE     =  3  , // Ein Fahrzeug umsetzen
    MSG_MOVE_TO          =  4  , // Ein Fahrzeug um ein Feld bewegen
    MSG_NO_PATH           = 5  , // Benachrichtigung über einen versperrten Pfad
    MSG_END_MOVE           = 6  , // Beendet die Bewegung eines Fahrzeugs
    MSG_CHANGE_VEH_NAME   = 7  , // Ändert den Namen eines Vehicles
    MSG_END_MOVE_FOR_NOW  = 8  , // Beendet die Bewegung eines Fahrzeugs für diese Runde
    MSG_CHANGE_PLAYER_NAME  = 9  , // Ändert den Namen eines Spielers
    MSG_ENDE_PRESSED      = 10 , // Benachrichtigung über einen Druck auf Ende
    MSG_MJOB_STOP        =  11 , // Wird vom Client übermittelt, wenn er manuell den MJob stoppt
    MSG_ADD_ATTACKJOB     = 12 , // Einen Attackjob einfügen
    MSG_DESTROY_OBJECT     = 13 , // Zerstört ein Objekt
    MSG_ERLEDIGEN        =  14 , // Ein MJob soll erledigt werden
    MSG_SAVED_SPEED      =  15 , // Teilt einem Client ein Speed Save mit
    MSG_CHANGE_BUI_NAME  =  16 , // Ändert den Namen eines Buildings
    MSG_START_BUILD      =  17 , // Startet ein Building zu bauen
    MSG_STOP_BUILD       =  18 , // Stopt ein Building zu bauen
    MSG_ADD_BUILDING     =  19 , // Fügt ein Gebäude ein
    MSG_START_BUILD_BIG  =  20 , // Startet den bau eines großen Gebäudes
    MSG_RESET_CONSTRUCTOR = 21 , // Setzt den Constructor nach beenden des Bauens neu
    MSG_START_CLEAR       = 22 , // Startet das Räumen eines Feldes
    MSG_STORE_VEHICLE     = 23 , // Läd ein Vehicle ein
    MSG_ACTIVATE_VEHICLE  = 24 , // Aktiviert ein geladenes Vehicle wieder
    MSG_START_WORK        = 25 , // Startet ein Gebäude
    MSG_STOP_WORK         = 26 , // Stoppt ein Gebäude
    MSG_ADD_VEHICLE       = 27 , // Erzeugt ein vehicle
    MSG_REPAIR            = 28 , // Repariert etwas
    MSG_RELOAD            = 29 , // Läd etwas nach
    MSG_WACHE             = 30 , // Ändert den Wachstatus eines Objektes
    MSG_CLEAR_MINE        = 31 , // Räumt eine Mine
    MSG_UPGRADE           = 32 , // Upgrade eines Spielers
    MSG_RESEARCH          = 33 , // Meldet eine abgeschlossene Forschung
    MSG_UPDATE_BUILDING   = 34 , // Meldet das Upgrade eines Gebäudes
    MSG_COMMANDO_MISTAKE  = 35 , // Berichtet über einen Fehler eines Commandos
    MSG_COMMANDO_SUCCESS  = 36 , // Meldet einen Erfolg eines Commandos
    MSG_START_SYNC        = 37 , // Fordert die Clients zum Synchronisieren auf
    MSG_SYNC_PLAYER       = 38 , // Sync Player
    MSG_SYNC_VEHICLE      = 39 , // Sync Vehicle
    MSG_SYNC_BUILDING     = 40 , // Sync Building
    MSG_UPDATE_STORED     = 41 , // Aktualisiert ein gespeichertes Vehicle
    MSG_REPORT_R_E_A      = 42 , // Berichtet über das Ende der RundenendeActions
    MSG_PING              = 43 , // Anforderung eines Pong
    MSG_PONG              = 44 , // Rückgabe des Pong
    MSG_PLAYER_DEFEAT     = 45 , // Medet die Niederlage eines Spielers
    MSG_HOST_DEFEAT       = 46 , // Meldet die Niederlage des Hostes
    MSG_PLAY_ROUNDS_NEXT  = 47 , // Nachricht, dass der nächste Spieler dran ist

    // Nur für das MP-Menü:
    MSG_SIGNING_IN       = 100 , // Anmeldenachricht eines Spielers
    MSG_YOUR_ID_IS       = 101 , // Teilt dem Client nach dem Anmelden seine ID mit
    MSG_MY_NAME_CHANGED  = 102 , // Benachrichtigung wenn ein Client seinen Namen/Farbe ändert
    MSG_PLAYER_LIST      = 103 , // Übertragng der Spielerliste
    MSG_OPTIONS          = 104 , // Übertragung der Optionen des Spiels
    MSG_WHO_ARE_YOU      = 105 , // Fordert den Client auf sich zu identifizieren
    MSG_CHECK_FOR_GO     = 106 , // Läßt überprüfen, ob alle Clients bereit sind
    MSG_READY_TO_GO      = 107 , // Meldet der Client wenn er bereit ist zum Starten
    MSG_NO_GO            = 108 , // Meldet der Client, wenn er nicht bereit ist
    MSG_LETS_GO          = 109 , // Gibt dem Client das Signal zum Start
    MSG_RESSOURCES       = 110 , // Überträgt die Ressourcenmap
    MSG_PLAYER_LANDING   = 111 , // Landedaten eines Players
    MSG_PLAYER_UPGRADES  = 112 , // Übeträgt alle Upgrades eines Players
    MSG_SAVEGAME_START   = 113 , // Start der Übertragung des Savegames
    MSG_SAVEGAME_PART    = 114  // Teil der Übertragung des Savegames
};

/**
* Message class with message, lenght and typ
*
* @author Albert "alzi" Ziegenhagel alias DoctorDeath
*/
class cNetMessage{
public:
	int typ;
	int lenght;
	char msg[256];
};

/**
* Buffer for incomming and outcomming packages
*
* @author Albert "alzi" Ziegenhagel alias DoctorDeath
*/
struct sNetBuffer{
	unsigned int iID;	// ID of this Message
	int iTyp;			// Typ of Buffer: see BUFF_TYP_XYZ
	int iPart;			// Partnumber of message
	int iMax_parts;		// Maximal number of parts
	int iTicks;			// Ticktime when this buffer has been send
	int iDestClientNum;	// Client to which this buffer should be send
	cNetMessage msg;	// Message for Data-Packages
};

struct sIDList{
	sIDList()
	{
		iCount = 0;
	}
	unsigned int iID[64];  // This isn't good but it hasn't worked with pointer. But normaly there shouldn't be more then 64 ids
	int iCount;

	void Add(unsigned int iSrcID)
	{
		if(iCount > 64) return; // Saves from bufferoverflow
		iID[iCount] = iSrcID;
		iCount++;
	}
	void Delete(int iIndex)
	{
		if(iIndex >= iCount) iIndex = iCount-1;
		iID[iIndex] = 0;
		iCount--;
		for (int i = iIndex; i <= iCount; i++)
			iID[i] = iID[i + 1];
	}
};

/**
* Failsafe TcpIp class.
*
* @author Albert "alzi" Ziegenhagel alias DoctorDeath
*/
class cTCP{
public:
	cTCP(bool server);
	~cTCP();
	SDL_Thread *TCPReceiveThread;	// Thread for message receiving
	sList *NetMessageList;				// List with all received messages

	bool bReceiveThreadFinished;		// Has the Receive-Thread finished?
	bool bServer;						// Is this a server?
	int iStatus;						// Status of connection
	int iMax_clients;					// Maximal clients that can connect
	int iMin_clients;					// Minimal clients needed to run stable

	void TCPCheckResends ();
	bool TCPOpen(void);
	bool TCPCreate(void);
	void TCPClose(void);
	bool TCPSend(int typ, const char *msg);
	bool TCPReceive(void);

	void SetTcpIpPort(int port);
	void SetIp(string ip);
	int GetConnectionCount(void);

private:
	unsigned int GenerateNewID();
	void SendOK(unsigned int iID, int iClientNum /* -1 For server*/ );
	SDL_Thread *TCPResendThread;	// Thread that looks for buffers which must be resend

	int iMyID;						// ID of this Client
	unsigned int iNextMessageID;	// ID of next Message
	sIDList *UsedIDs;				// A List with all currently used message IDs (server only)
	sList *WaitOKList;				// A List with all IDs of messages, the game is waiting for an Reseive-OK
	int iPlayerId;					// ID of this Player
	int iNum_clients;				// Number of current clients
	int iPort;						// Current port
	string sIp;						// Current ip or hostname
	IPaddress addr;					// Address for SDL_Net
	TCPsocket sock_server, sock_client[8];	// Sockets of Server (clients only) or for maximal 16 clients (server only)
	SDLNet_SocketSet SocketSet;		// The socket-set with all currently connected clients
};

/**
* Receive funktion for TCPReceiveThread.
*
* @author Albert "alzi" Ziegenhagel alias DoctorDeath
*/
int Receive(void *);
/**
* Open funktion for TCPReceiveThread (server only).
* In this funktion receiving of messages is integrated while the server is waiting for clients.
*
* @author Albert "alzi" Ziegenhagel alias DoctorDeath
*/
int Open(void *);
/**
* Looks for buffers which must be resend
*
* @author Albert "alzi" Ziegenhagel alias DoctorDeath
*/
int CheckResends(void *);

#endif
