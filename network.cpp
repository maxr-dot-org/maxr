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

void cTCP::init()
{
	DataMutex = SDL_CreateMutex();
	TCPMutex = SDL_CreateMutex();
	WaitForRead = SDL_CreateCond();

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

	bExit = false;
	TCPHandleThread = SDL_CreateThread( CallbackHandleNetworkThread, this );
}

void cTCP::kill()
{
	bExit = true;
	SDL_CondSignal( WaitForRead );
	SDL_WaitThread ( TCPHandleThread, NULL );

	for ( int i = 0; i < iLast_Socket; i++ )
	{
		if ( Sockets[i]->iType != FREE_SOCKET )
		{
			SDLNet_TCP_Close ( Sockets[i]->socket );
		}
		free ( Sockets[i] );
	}

	SDL_DestroyMutex( DataMutex );
	SDL_DestroyMutex( TCPMutex );
	SDL_DestroyCond( WaitForRead );
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
	if ( !Sockets[iNum]->socket ) { unlockData(); return -1; }

	Sockets[iNum]->iType = SERVER_SOCKET;
	Sockets[iNum]->iState = STATE_NEW;
	clearBuffer ( &Sockets[iNum]->buffer );

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
	if ( !Sockets[iNum]->socket ) { unlockData(); return -1; }

	Sockets[iNum]->iType = CLIENT_SOCKET;
	Sockets[iNum]->iState = STATE_NEW;
	clearBuffer ( &Sockets[iNum]->buffer );

	unlockData();
	return 0;
}

int cTCP::sendTo( int iClientNumber, int iLenght, char *buffer )
{
	lockData();
	if ( iClientNumber >= 0 && iClientNumber < iLast_Socket && Sockets[iClientNumber]->iType == CLIENT_SOCKET && ( Sockets[iClientNumber]->iState == STATE_READY || Sockets[iClientNumber]->iState == STATE_NEW ) )
	{
		if ( iLenght > 0 )
		{
			lockTCP();
			int iSendLenght = SDLNet_TCP_Send ( Sockets[iClientNumber]->socket, buffer, iLenght );
			unlockTCP();
			if ( iSendLenght != iLenght )
			{
				Sockets[iClientNumber]->iState = STATE_DYING;
				void *data = malloc ( sizeof (Sint16) );
				((Sint16*)data)[0] = iClientNumber;
				sendEvent ( TCP_CLOSEEVENT, data, NULL );
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
	int iMinLenght;

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

	bWaitForRead = false;
	SDL_CondSignal ( WaitForRead );

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
			bDataLocked = false;
			SDL_CondWait ( WaitForRead, DataMutex );
			bDataLocked = true;
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
						sendEvent( TCP_ACCEPTEVENT, data, NULL );
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
					iLenght = SDLNet_TCP_Recv ( Sockets[i]->socket, Sockets[i]->buffer.data, PACKAGE_LENGHT - Sockets[i]->buffer.iLenght );

					void *data = malloc ( sizeof (Sint16) );
					((Sint16*)data)[0] = i;
					if ( iLenght > 0 )
					{
						int iOldLenght = Sockets[i]->buffer.iLenght;
						Sockets[i]->buffer.iLenght += iLenght;
						if ( iOldLenght == 0 )
						{
							sendEvent ( TCP_RECEIVEEVENT, data, NULL );
						}
					}
					else
					{
						Sockets[i]->iState = STATE_DYING;
						sendEvent ( TCP_CLOSEEVENT, data, NULL );
					}
				}
			}
		}
		unlockData();
	}
}

int cTCP::sendEvent( int iEventType, void *data1, void *data2 )
{
	SDL_Event event;

	event.type = NETWORK_EVENT;
	event.user.code = iEventType;
	event.user.data1 = data1;
	event.user.data2 = data2;

	if ( bDataLocked )
	{
		unlockData();
		EventHandler->pushEvent ( &event );
		lockData();
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
	lockData();
	SDLNet_TCP_Close( Sockets[iNum]->socket );
	for ( int i = iNum; i < iLast_Socket; i++ )
	{
		Sockets[i] = Sockets[i+1];
	}
	iLast_Socket--;
	unlockData();
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
	SDL_LockMutex ( DataMutex );
	bDataLocked = true;
}

void cTCP::unlockData()
{
	SDL_UnlockMutex ( DataMutex );
	bDataLocked = false;
}

void cTCP::sendWaitCondSignal()
{
	bWaitForRead = false;
	SDL_CondSignal( WaitForRead );
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

void cTCP::clearBuffer( sDataBuffer *buffer )
{
	buffer->iLenght = 0;
	memset ( buffer, 0, PACKAGE_LENGHT);
}

int cTCP::getFreeSocket()
{
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
