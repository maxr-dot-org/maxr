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
#include "menu.h"
#include "log.h"

cTCP::cTCP ( bool server )
{
	this->bServer=server;
	iNum_clients=0;
	iMax_clients=8;
	iPlayerId=-1;
	SocketSet=NULL;
	sock_server=NULL;
	for ( int i=0;i<8;i++ )
	{
		sock_client[i]=NULL;
	}
	iStatus=STAT_CLOSED;
	UsedIDs = new sIDList();
	WaitOKList = new sList();
	NetMessageList = new sList();
	iMyID = 0;
	iNextMessageID = GenerateNewID();
	bReceiveThreadFinished = true;
	TCPReceiveThread = NULL;
	TCPResendThread = SDL_CreateThread ( CheckResends , NULL );
}

cTCP::~cTCP()
{
	TCPClose();
}


void cTCP::TCPClose()
{
	if ( TCPReceiveThread ) SDL_KillThread( TCPReceiveThread );
	if ( TCPResendThread ) SDL_KillThread( TCPResendThread );
	if ( SocketSet ) SDLNet_FreeSocketSet ( SocketSet );
	if ( sock_server ) SDLNet_TCP_Close ( sock_server );
	for ( int i=0;i<8;i++ )
	{
		if ( sock_client[i] ) SDLNet_TCP_Close ( sock_client[i] );
	}
	iStatus=STAT_CLOSED;
	TCPReceiveThread = NULL;
	TCPResendThread = NULL;
}

bool cTCP::TCPCreate()
{
	// Host
	if ( bServer )
	{
		if ( SDLNet_ResolveHost ( &addr,NULL,iPort ) <0 ) // No address for server
		{
			return false;
		}
	}
	// Client
	else
	{
		if ( SDLNet_ResolveHost ( &addr,sIp.c_str(),iPort ) <0 ) // Get server at address
		{
			return false;
		}
	}
	return true;
}

bool cTCP::TCPOpen ( void )
{
	TCPCreate();
	// Host
	if ( bServer )
	{
		sock_server = SDLNet_TCP_Open ( &addr ); // open socket
		if ( sock_server == NULL )
		{
			return false;
		}
		iStatus = STAT_OPENED;
		while ( iNum_clients < iMax_clients ) // Wait for more clients
		{
			if ( iNum_clients > 0 )
				TCPReceive(); // If there is minimal one client: look for messages
			else
				SDL_Delay ( 1000 ); // else wait a second
			sock_client[iNum_clients] = SDLNet_TCP_Accept ( sock_server ); // look for new client
			if ( sock_client[iNum_clients] != NULL ) // New client has connected
			{
				iNum_clients++;
				SocketSet = SDLNet_AllocSocketSet ( iNum_clients ); // Alloc socket-set for new client
				for ( int i=0;i<=iNum_clients;i++ )
				{
					SDLNet_TCP_AddSocket ( SocketSet, sock_client[i] ); // Add clients to the socket-set
				} 
				// Send the client his ID
				sNetBuffer *NetBuffer = new sNetBuffer();

				NetBuffer->iID = iNextMessageID;
				UsedIDs->Add ( iNextMessageID );
				iNextMessageID = GenerateNewID();
				NetBuffer->iTyp = BUFF_TYP_NEWID;
				NetBuffer->iTicks = SDL_GetTicks();
				NetBuffer->iDestClientNum = iNum_clients;
				NetBuffer->msg.lenght = ( int )iToStr( iNum_clients-1 ).length() + 1;
				strcpy(NetBuffer->msg.msg, iToStr( iNum_clients ).c_str() );

				SDLNet_TCP_Send ( sock_client[iNum_clients-1], NetBuffer, sizeof ( sNetBuffer ) );
				SDL_Delay ( 1 );

				string sTmp;
				sTmp = "(Host)Send NewID-Message: -ID " + iToStr(NetBuffer->iID) + " -Client " + iToStr( iNum_clients-1 ) + "-Message: \"" + NetBuffer->msg.msg + "\"";
				cLog::write(sTmp, LOG_TYPE_NETWORK);
				WaitOKList->Add(NetBuffer); // Add package to waitlist
			}
		}
		iStatus=STAT_CONNECTED;
	}
	// Client
	else
	{
		sock_server = SDLNet_TCP_Open ( &addr ); // open socket
		if ( sock_server == NULL )
		{
			return false;
		}
		TCPReceive(); // Wait for new id
		iStatus=STAT_CONNECTED;
	}
	return true;
}

bool cTCP::TCPSend ( int typ,const char *msg)
{
	if (iStatus == STAT_CLOSED) return false;
	sNetBuffer *NetBuffer = new sNetBuffer();
	int iPartLenght;
	int iPartNum = 1;
	int iMax_PartNum;
	// Send Message parts
	iMax_PartNum = ( int )(( float ) ( strlen(msg) + 1 ) / 256) + 1; // How many parts to send?
	while(iPartNum <= iMax_PartNum)
	{
		// Set the actual message part
		if(iPartNum < iMax_PartNum)
		{
			iPartLenght = 256;
		}
		else
		{
			iPartLenght = ( int ) ( strlen(msg) - ( ( iMax_PartNum - 1 ) * 255 ) + 1 );
		}
		NetBuffer->msg.typ = typ;
		NetBuffer->msg.lenght = iPartLenght;
		memcpy ( NetBuffer->msg.msg, msg,iPartLenght );
		NetBuffer->msg.msg[iPartLenght-1]='\0';

		// Set the buffer
		NetBuffer->iMax_parts = iMax_PartNum;
		NetBuffer->iPart = iPartNum;
		NetBuffer->iTyp = BUFF_TYP_DATA;

		// Host
		int i = 0;
		if ( bServer )
		{
			while ( sock_client[i] != NULL && i < iMax_clients )
			{
				NetBuffer->iID = iNextMessageID;
				UsedIDs->Add ( iNextMessageID );
				iNextMessageID = GenerateNewID();
				NetBuffer->iTicks = SDL_GetTicks();
				NetBuffer->iDestClientNum = i;
				SDLNet_TCP_Send ( sock_client[i], NetBuffer, sizeof ( sNetBuffer ) );
				string sTmp;
				sTmp = "(Host)Send Data-Message: -ID: "  + SplitMessageID( NetBuffer->iID ) + " -Client: " + iToStr( i ) + " -Message: \"" + NetBuffer->msg.msg + "\"" + " -iTyp: " + iToStr( NetBuffer->msg.typ );
				cLog::write(sTmp, LOG_TYPE_NETWORK);
				SDL_Delay ( 1 );
				i++;
			}
		}
		// Client
		else
		{
			NetBuffer->iID = iNextMessageID;
			UsedIDs->Add ( iNextMessageID );
			iNextMessageID = GenerateNewID();
			NetBuffer->iTicks = SDL_GetTicks();
			NetBuffer->iDestClientNum = -1;
			SDLNet_TCP_Send ( sock_server, NetBuffer, sizeof ( sNetBuffer ) );
			string sTmp;
			sTmp = "(Client)Send Data-Message: -ID: "  + SplitMessageID( NetBuffer->iID ) + " -Message: \"" + NetBuffer->msg.msg + "\"" + " -iTyp: " + iToStr( NetBuffer->msg.typ );
			cLog::write(sTmp, LOG_TYPE_NETWORK);
			SDL_Delay ( 1 );
		}
		// Add package to waitlist
		WaitOKList->Add(NetBuffer);

		iPartNum++;
	}
	return true;
}

bool cTCP::TCPReceive()
{
	sNetBuffer *NetBuffer = new sNetBuffer();
	// Host
	int i = 0;
	if ( bServer )
	{
		SDLNet_CheckSockets ( SocketSet, 1000 * 1 );
		while ( i < iNum_clients )
		{
			if ( SDLNet_SocketReady ( sock_client[i] ) )
			{
				SDLNet_TCP_Recv ( sock_client[i], NetBuffer, sizeof ( sNetBuffer ) );
				// is a new messages
				if ( NetBuffer->iTyp == BUFF_TYP_DATA )
				{
					string sTmp;
					sTmp = "(Host)Received Data-Message: -ID: "  + SplitMessageID( NetBuffer->iID ) + " -Client: " + iToStr( i ) + " -Message: \"" + NetBuffer->msg.msg + "\"" + " -iTyp: " + iToStr( NetBuffer->msg.typ );
					cLog::write(sTmp, LOG_TYPE_NETWORK);
					// Add message to list
					NetMessageList->Add( &NetBuffer->msg );
					// Send OK to Client
					SendOK( NetBuffer->iID, i );
				}
				// if an OK that messages has been reveived
				else if( NetBuffer->iTyp == BUFF_TYP_OK )
				{
					string sTmp;
					sTmp = "(Host)Received OK-Message: -ID: "  + SplitMessageID( NetBuffer->iID ) + " -Client: " + iToStr( i );
					cLog::write(sTmp, LOG_TYPE_NETWORK);
					// Delete Message ID from WaitList
					for (int k = 0; k < WaitOKList->iCount; k++)
					{
						if( ( ( sNetBuffer *) WaitOKList->Items[k] ) == NetBuffer )
						{
							WaitOKList->Delete(k);
							break;
						}
					}
					// Delete Message ID from used IDs
					for (int k = 0; k < UsedIDs->iCount; k++)
					{
						if( UsedIDs->iID[k] == NetBuffer->iID )
						{
							UsedIDs->Delete(k);
							break;
						}
					}
				}
				SDL_Delay ( 1 );
			}
			i++;
		}
	}
	// Client
	else
	{
		SDLNet_TCP_Recv ( sock_server, NetBuffer, sizeof ( sNetBuffer ) );
		if ( NetBuffer->iTyp == BUFF_TYP_DATA )
		{
			string sTmp;
			sTmp = "(Client)Received Data-Message: -ID: "  + SplitMessageID( NetBuffer->iID ) + " -Message: \"" + NetBuffer->msg.msg + "\"" + " -iTyp: " + iToStr( NetBuffer->msg.typ );
			cLog::write(sTmp, LOG_TYPE_NETWORK);
			// Add message to list
			NetMessageList->Add( &NetBuffer->msg );
			// Send OK to Server
			SendOK( NetBuffer->iID, -1 );
		}
		// if an OK that messages has been reveived
		else if( NetBuffer->iTyp == BUFF_TYP_OK )
		{
			string sTmp;
			sTmp = "(Client)Received OK-Message: -ID: "  + SplitMessageID( NetBuffer->iID );
			cLog::write(sTmp, LOG_TYPE_NETWORK);
			// Delete Message ID from WaitList
			for (int k = 0; k < WaitOKList->iCount; k++)
			{
				if( ( ( sNetBuffer *) WaitOKList->Items[k] ) == NetBuffer )
				{
					WaitOKList->Delete(k);
					break;
				}
			}
		}
		// if a newid has been received
		else if( NetBuffer->iTyp == BUFF_TYP_NEWID )
		{
			string sTmp;
			sTmp = "(Client)Received NewID-Message: -ID: "  + SplitMessageID( NetBuffer->iID ) + " -Message: \"" + NetBuffer->msg.msg;
			cLog::write(sTmp, LOG_TYPE_NETWORK);
			iMyID = atoi ( (char *) NetBuffer->msg.msg );
			SendOK( NetBuffer->iID, -1 );
		}
	}
	if ( !bServer || iStatus==STAT_CONNECTED )
		bReceiveThreadFinished = true;
	return true;
}

unsigned int cTCP::GenerateNewID()
{
	int iReturnID;
	int iHour, iMin, iSec, iMsec;
	iHour = SDL_GetTicks() / ( 60*60*1000 );
	iMin = SDL_GetTicks() / ( 60*1000 ) - iHour*60;
	iSec = SDL_GetTicks() / 1000 - iMin*60 - iHour*60*60;
	iMsec = SDL_GetTicks() - iSec*1000 - iMin*60*1000 - iHour*60*60*1000;

	char szTmp[13];	// Message-ID will allways be 12 characters long + null termination
	sprintf(szTmp, "%0.2d:%0.2d:%0.2d:%0.3d", iMyID, iMin, iSec, iMsec );
	cLog::write( (string)"ID generated: " + szTmp, LOG_TYPE_NETWORK); 

	iReturnID = iMyID * 10000000;
	iReturnID += iMin * 100000;
	iReturnID += iSec * 1000;
	iReturnID += iMsec;

	return iReturnID;
}

string cTCP::SplitMessageID(unsigned int iID)
{
	string sResult;
	sResult = iToStr( iID );
	while( sResult.size() < 9 )
	{
		sResult.insert( 0,"0" );
	}
	sResult.insert( sResult.size()-3,":" );
	sResult.insert( sResult.size()-6,":" );
	sResult.insert( sResult.size()-9,":" );
	return sResult;
}

void cTCP::SendOK(unsigned int iID, int iClientNum)
{
	sNetBuffer *NetBuffer = new sNetBuffer();
	NetBuffer->iID = iID;
	NetBuffer->iMax_parts = 1;
	NetBuffer->iPart = 1;
	NetBuffer->iTyp = BUFF_TYP_OK;
	NetBuffer->iTicks = SDL_GetTicks();
	NetBuffer->iDestClientNum = iClientNum;

	// Client
	if(iClientNum == -1)
	{
		SDLNet_TCP_Send ( sock_server, NetBuffer, sizeof ( sNetBuffer ) );
		string sTmp;
		sTmp = "(Client)Send OK-Message: -ID: "  + SplitMessageID( NetBuffer->iID );
		cLog::write(sTmp, LOG_TYPE_NETWORK);
		SDL_Delay ( 1 );
	}
	// Host
	else
	{
		SDLNet_TCP_Send ( sock_client[iClientNum], NetBuffer, sizeof ( sNetBuffer ) );
		string sTmp;
		sTmp = "(Host)Send OK-Message: -ID: "  + SplitMessageID( NetBuffer->iID ) + " -Client: \"" + iToStr( iClientNum );
		cLog::write(sTmp, LOG_TYPE_NETWORK);
		SDL_Delay ( 1 );
	}
	// Delete Message ID from used IDs if this is server
	if( bServer ){
		for (int k = 0; k < UsedIDs->iCount; k++)
		{
			if( UsedIDs->iID[k] == iID )
			{
				UsedIDs->Delete(k);
				break;
			}
		}
	}
	return ;
}

void cTCP::SetTcpIpPort ( int iPort )
{
	this->iPort=iPort;
}

void cTCP::SetIp ( string sIp )
{
	this->sIp=sIp;
}

int cTCP::GetConnectionCount()
{
	return iNum_clients;
}

int Receive ( void * )
{
	MultiPlayer->network->bReceiveThreadFinished = false;
	MultiPlayer->network->TCPReceive();
	MultiPlayer->network->bReceiveThreadFinished = true;
	return 1;
}

int Open ( void * )
{
	MultiPlayer->network->bReceiveThreadFinished = false;
	if ( MultiPlayer->network->TCPOpen() )
	{
		MultiPlayer->network->bReceiveThreadFinished = true;
		return 1;
	}
	else
	{
		MultiPlayer->network->bReceiveThreadFinished = true;
		return 0;
	}
}

int CheckResends ( void * )
{
	SDL_Delay(1000);
	while ( 1 )
	{
		MultiPlayer->network->TCPCheckResends ();
	}
	return 1;
}

void cTCP::TCPCheckResends ()
{
	for(int i = 0; WaitOKList->iCount > i; i++) // Check all Waiting Buffers
	{
		if(WaitOKList->Items[i]) // sanity check
		{
			int iTime = ( ( sNetBuffer *) WaitOKList->Items[i])->iTicks; // Get the Time since this buffer was send the last time
			if( ( iTime - SDL_GetTicks() ) > 100 )
			{
				int iClient = ( ( sNetBuffer *) WaitOKList->Items[i])->iDestClientNum; // To which client should the buffer be send
				if(iClient != -1) // To a Client
				{
					SDLNet_TCP_Send ( sock_client[iClient],  WaitOKList->Items[i], sizeof ( sNetBuffer ) );
				}
				else // To the Host
				{
					SDLNet_TCP_Send ( sock_server,  WaitOKList->Items[i], sizeof ( sNetBuffer ) );
				}
				( ( sNetBuffer *) WaitOKList->Items[i])->iTicks = SDL_GetTicks(); // Set the new Time
				string sTmp;
				sTmp = "Resend buffer: -ID: " + iToStr( ( ( sNetBuffer *) WaitOKList->Items[i])->iID );
				cLog::write(sTmp, LOG_TYPE_NETWORK);
			}
			SDL_Delay ( 1 ) ;
		}
	}
	return ;
}
