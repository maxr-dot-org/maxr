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

#include "network.h"
#include "events.h"
#include "server.h"

void cTCP::init()
{
	TCPMutex = SDL_CreateMutex();

	for ( int i = 0; i < MAX_CLIENTS; i++ )
	{
		Sockets[i] = NULL;
	}
	SocketSet = SDLNet_AllocSocketSet( MAX_CLIENTS );

	iLast_Socket = 0;
	sIP = "";
	iPort = 0;
	bDataLocked = false;
	bTCPLocked = false;
	bWaitForRead = false;
	bHost = false;

	bExit = false;
	TCPHandleThread = SDL_CreateThread( CallbackHandleNetworkThread, this );
}

void cTCP::kill()
{
	bExit = true;
	SDL_WaitThread ( TCPHandleThread, NULL );

	for ( int i = 0; i < iLast_Socket; i++ )
	{
		if ( Sockets[i]->iType != FREE_SOCKET )
		{
			SDLNet_TCP_Close ( Sockets[i]->socket );
		}
		free ( Sockets[i] );
	}

	SDL_DestroyMutex( TCPMutex );
	sendReadFinished();
	unlockData();
}


int cTCP::create()
{
	lockTCP();
	if( SDLNet_ResolveHost( &ipaddr, NULL, iPort ) == -1 ) { unlockTCP(); return -1; }
	unlockTCP();

	lockData();
	int iNum;
	if ( ( iNum = getFreeSocket() ) == -1 ) { unlockData(); return -1; }

	Sockets[iNum]->socket = SDLNet_TCP_Open ( &ipaddr );
	if ( !Sockets[iNum]->socket )
	{
		unlockData();
		deleteSocket ( iNum );
		return -1;
	}

	Sockets[iNum]->iType = SERVER_SOCKET;
	Sockets[iNum]->iState = STATE_NEW;
	clearBuffer ( &Sockets[iNum]->buffer );

	bHost = true; // is the host
	unlockData();
	return 0;
}

int cTCP::connect()
{
	lockTCP();
	if( SDLNet_ResolveHost( &ipaddr, sIP.c_str(), iPort ) == -1 ) { unlockTCP(); return -1; }
	unlockTCP();

	lockData();
	int iNum;
	if ( ( iNum = getFreeSocket() ) == -1 ) { unlockData(); return -1; }

	Sockets[iNum]->socket = SDLNet_TCP_Open ( &ipaddr );
	if ( !Sockets[iNum]->socket )
	{
		unlockData();
		deleteSocket ( iNum );
		return -1;
	}

	Sockets[iNum]->iType = CLIENT_SOCKET;
	Sockets[iNum]->iState = STATE_NEW;
	clearBuffer ( &Sockets[iNum]->buffer );

	bHost = false;	// is not the host
	unlockData();
	return 0;
}

int cTCP::sendTo( int iClientNumber, int iLenght, char *buffer )
{
	lockData();
	if ( iClientNumber >= 0 && iClientNumber < iLast_Socket && Sockets[iClientNumber]->iType == CLIENT_SOCKET && ( Sockets[iClientNumber]->iState == STATE_READY || Sockets[iClientNumber]->iState == STATE_NEW ) )
	{
		if ( iLenght >= PACKAGE_LENGHT-1 ) iLenght = PACKAGE_LENGHT-2;

		if ( iLenght > 0 )
		{
			char *sendBuffer = (char *) malloc ( iLenght+2 );
			sendBuffer[0] = (char)NETMESSAGE_CONTROLCHAR;
			sendBuffer[1] = (char)NETMESSAGE_STARTCHAR;
			memcpy ( &sendBuffer[2], buffer, iLenght );

			lockTCP();
			int iSendLenght = SDLNet_TCP_Send ( Sockets[iClientNumber]->socket, sendBuffer, iLenght+2 );
			unlockTCP();

			free ( sendBuffer );

			if ( iSendLenght != iLenght+2 )
			{
				Sockets[iClientNumber]->iState = STATE_DYING;
				void *data = malloc ( sizeof (Sint16) );
				((Sint16*)data)[0] = iClientNumber;
				pushEvent ( TCP_CLOSEEVENT, data, NULL );
				unlockData();
				return -1;
			}
		}
	}
	unlockData();
	return 0;
}

int cTCP::send( int iLenght, char *buffer )
{
	int iReturnVal = 0;
	for ( int i = 0; i < getSocketCount(); i++ )
	{
		if ( sendTo ( i, iLenght, buffer ) == -1 )
		{
			iReturnVal = -1;
		}
	}
	return iReturnVal;
}

int cTCP::read( int iClientNumber, int iLenght, char *buffer )
{
	int iMinLenght = 0;

	lockData();
	if ( iClientNumber >= 0 && iClientNumber < iLast_Socket && Sockets[iClientNumber]->iType == CLIENT_SOCKET )
	{
		if ( iLenght > 0 )
		{
			iMinLenght = ( ( ( Sockets[iClientNumber]->buffer.iLenght ) < ( (unsigned int)iLenght ) ) ? ( Sockets[iClientNumber]->buffer.iLenght ) : ( (unsigned int)iLenght ) );
			memmove ( buffer, Sockets[iClientNumber]->buffer.data, iMinLenght );
			Sockets[iClientNumber]->buffer.iLenght -= iMinLenght;
			memmove ( Sockets[iClientNumber]->buffer.data, &Sockets[iClientNumber]->buffer.data[iMinLenght], Sockets[iClientNumber]->buffer.iLenght );
		}
	}
	unlockData();

	sendReadFinished();

	return iMinLenght;
}

int CallbackHandleNetworkThread( void *arg )
{
	cTCP *TCP = (cTCP *) arg;
	TCP->HandleNetworkThread();
	return 0;
}

void cTCP::HandleNetworkThread()
{
	while( !bExit )
	{
		SDLNet_CheckSockets ( SocketSet, 10 );

		lockData();

		// Wait until there is something to read
		while ( !bExit && bWaitForRead )
		{
			waitForRead();
		}
		// Check all Sockets
		for ( int i = 0; !bExit && i < iLast_Socket; i++ )
		{
			if ( Sockets[i]->iState == STATE_NEW )
			{
				lockTCP();
				if ( SDLNet_TCP_AddSocket ( SocketSet, Sockets[i]->socket ) != -1 )
				{
					Sockets[i]->iState = STATE_READY;
				}
				else
				{
					Sockets[i]->iState = STATE_DELETE;
				}
				unlockTCP();
			}
			else if ( Sockets[i]->iState == STATE_DELETE )
			{
				lockTCP();
				SDLNet_TCP_DelSocket ( SocketSet, Sockets[i]->socket );
				deleteSocket ( i );
				unlockTCP();
				i--;
				continue;
			}
			else if ( Sockets[i]->iType == SERVER_SOCKET && SDLNet_SocketReady ( Sockets[i]->socket ) )
			{
				lockTCP();
				TCPsocket socket = SDLNet_TCP_Accept ( Sockets[i]->socket );
				unlockTCP();

				if ( socket != NULL ) 
				{
					int iNum;
					if ( ( iNum = getFreeSocket() ) != -1 )
					{
						Sockets[iNum]->socket = socket;

						Sockets[iNum]->iType = CLIENT_SOCKET;
						Sockets[iNum]->iState = STATE_NEW;
						clearBuffer ( &Sockets[iNum]->buffer );
						void *data = malloc ( sizeof (Sint16) );
						((Sint16*)data)[0] = iNum;
						pushEvent( TCP_ACCEPTEVENT, data, NULL );
					}
				}
				else SDLNet_TCP_Close ( socket );
			}
			else if ( Sockets[i]->iType == CLIENT_SOCKET && Sockets[i]->iState == STATE_READY && SDLNet_SocketReady ( Sockets[i]->socket ) )
			{
				if ( Sockets[i]->buffer.iLenght >= PACKAGE_LENGHT )
				{
					bWaitForRead = true;
				}
				else
				{
					int iLenght;
					iLenght = SDLNet_TCP_Recv ( Sockets[i]->socket, &Sockets[i]->buffer.data[Sockets[i]->buffer.iLenght], PACKAGE_LENGHT-Sockets[i]->buffer.iLenght );

					// check whether everything has been read
					bool bAllRead = true;
					SDLNet_CheckSockets ( SocketSet, 10 );
					if ( SDLNet_SocketReady ( Sockets[i]->socket ) )
					{
						bAllRead = false;
					}

					static int iRestData = 0;
					int iMsgStartOffset = Sockets[i]->buffer.iLenght;
					if ( iLenght > 0 )
					{
						int iStartPos = iMsgStartOffset-iRestData;
						while ( ( iStartPos = findNextMessageStart ( iStartPos, Sockets[i]->buffer.data, iMsgStartOffset+iLenght ) ) != -1 )
						{
							int iEndPos = findNextMessageStart ( iStartPos+2, Sockets[i]->buffer.data, iMsgStartOffset+iLenght );
							if ( ( iEndPos != -1 || iStartPos == 0 ) || bAllRead )
							{
								if ( iEndPos == -1 ) iEndPos = iMsgStartOffset+iLenght;

								void *data = malloc ( sizeof (Sint16)*3 );
								((Sint16*)data)[0] = i;
								((Sint16*)data)[1] = ((Sint16*)(Sockets[i]->buffer.data+iStartPos+2))[0];
								((Sint16*)data)[2] = iEndPos-iStartPos;

								Sockets[i]->buffer.iLenght += iEndPos-iStartPos-iRestData;
								iRestData = 0;
								pushEvent ( TCP_RECEIVEEVENT, data, NULL );
								iStartPos = iEndPos;
							}
							else
							{
								if ( iRestData == 0 )
								{
									iRestData = iMsgStartOffset+iLenght-iStartPos;
									Sockets[i]->buffer.iLenght += iRestData;
								}
								else
								{
									iRestData += iLenght;
									Sockets[i]->buffer.iLenght += iLenght;
								}
								break;
							}
						}
					}
					else
					{
						void *data = malloc ( sizeof (Sint16)*2 );
						((Sint16*)data)[0] = i;

						Sockets[i]->iState = STATE_DYING;
						pushEvent ( TCP_CLOSEEVENT, data, NULL );
					}
				}
			}
		}
		unlockData();
	}
}

int cTCP::pushEvent( int iEventType, void *data1, void *data2 )
{
	SDL_Event* event = new SDL_Event;

	event->type = NETWORK_EVENT;
	event->user.code = iEventType;
	event->user.data1 = malloc ( PACKAGE_LENGHT );
	memcpy ( event->user.data1, data1, sizeof ( Sint16 ) * 3 );
	free ( data1 );
	event->user.data2 = data2;

	int iMessageLength = ((Sint16*)event->user.data1)[2];

	if ( bDataLocked )
	{
		unlockData();
		if ( ( Server && ( iEventType == TCP_CLOSEEVENT || iEventType == TCP_ACCEPTEVENT ) ) || ( iEventType == TCP_RECEIVEEVENT && SDL_SwapLE16 ( ((Sint16*)event->user.data1)[1] ) < FIRST_CLIENT_MESSAGE ) )
		{
			// Read data for server and add it to the event
			int iMinLenght, iClientNumber;
			iClientNumber = SDL_SwapLE16 ( ((Sint16*)event->user.data1)[0] );
			iMinLenght = ( ( ( Sockets[iClientNumber]->buffer.iLenght ) < (unsigned int)iMessageLength ) ? ( Sockets[iClientNumber]->buffer.iLenght ) : iMessageLength );
			memmove ( (char*)event->user.data1, Sockets[iClientNumber]->buffer.data, iMinLenght );
			Sockets[iClientNumber]->buffer.iLenght -= iMinLenght;
			memmove ( Sockets[iClientNumber]->buffer.data, &Sockets[iClientNumber]->buffer.data[iMinLenght], Sockets[iClientNumber]->buffer.iLenght );

			// Send the event to the server
			if ( Server != NULL ) Server->pushEvent ( event );
		}
		else
		{
			// Event for the client or message for the menu
			EventHandler->pushEvent ( event );
		}
		lockData();
	}
	else
	{
		cLog::write( "Fatal Error when trying to push event", cLog::eLOG_TYPE_NET_ERROR );
	}

	return 0;
}

void cTCP::close( int iClientNumber )
{
	lockData();
	if ( iClientNumber >= 0 && iClientNumber < iLast_Socket && ( Sockets[iClientNumber]->iType == CLIENT_SOCKET || Sockets[iClientNumber]->iType == SERVER_SOCKET ) )
	{
		Sockets[iClientNumber]->iState = STATE_DELETE;
	}
	unlockData();
}

void cTCP::deleteSocket( int iNum )
{
	SDLNet_TCP_Close( Sockets[iNum]->socket );
	for ( int i = iNum; i < iLast_Socket; i++ )
	{
		Sockets[i] = Sockets[i+1];
	}
	iLast_Socket--;
}

void cTCP::lockTCP()
{
	SDL_LockMutex ( TCPMutex );
	bTCPLocked = true;
}

void cTCP::unlockTCP()
{
	SDL_UnlockMutex ( TCPMutex );
	bTCPLocked = false;
}

void cTCP::lockData()
{
	while ( bDataLocked )
	{
		SDL_Delay ( 10 );
	}
	bDataLocked = true;
}

void cTCP::unlockData()
{
	bDataLocked = false;
}

void cTCP::sendReadFinished()
{
	bWaitForRead = false;
}

void cTCP::waitForRead()
{
	unlockData();
	while ( bWaitForRead )
	{
		SDL_Delay( 10 );
	}
	lockData();
}

void cTCP::setPort( int iPort )
{
	this->iPort = iPort;
}

void cTCP::setIP ( string sIP )
{
	this->sIP = sIP;
}

int cTCP::getSocketCount()
{
	return iLast_Socket;
}

int cTCP::getConnectionStatus()
{
	if ( iLast_Socket  > 0 ) return 1;
	return 0;
}
bool cTCP::isHost()
{
	return bHost;
}

void cTCP::clearBuffer( sDataBuffer *buffer )
{
	buffer->iLenght = 0;
	memset ( buffer, 0, PACKAGE_LENGHT);
}

int cTCP::getFreeSocket()
{
	if ( iLast_Socket == MAX_CLIENTS ) return -1;
	int iNum;
	for( iNum = 0; iNum < iLast_Socket; iNum++ )
	{
		if ( Sockets[iNum]->iType == FREE_SOCKET )
		{
			break;
		}
	}
	if ( iNum == iLast_Socket )
	{
		Sockets[iNum] = (sSocket *) malloc ( sizeof ( sSocket ) );
		Sockets[iNum]->iType = FREE_SOCKET;
		iLast_Socket++;
	}
	else if ( iNum > iLast_Socket ) return -1;

	return iNum;
}

int cTCP::findNextMessageStart ( int iStartPos, char *data, int iLength )
{
	for ( int iPos = iStartPos; iPos < iLength; iPos++ )
	{
		if ( data[iPos] == (char)NETMESSAGE_CONTROLCHAR && data[iPos+1] == (char)NETMESSAGE_STARTCHAR ) return iPos;
	}
	return -1;
}