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
#include "fstcpip.h"
#include "menu.h"

// Funktionen der FailSave TCP/IP Klasse /////////////////////////////////////
cFSTcpIp::cFSTcpIp ( bool server )
{
	this->server=server;
	num_clients=0;
	max_clients=16;
	PlayerId=-1;
	SocketSet=NULL;
	sock_server=NULL;
	msg=new cNetMessage;
	for ( int i=0;i<8;i++ )
	{
		sock_client[i]=NULL;
	}
	status=STAT_CLOSED;
}

cFSTcpIp::~cFSTcpIp()
{
	FSTcpIpClose();
}


void cFSTcpIp::FSTcpIpClose()
{
	if ( SocketSet ) SDLNet_FreeSocketSet ( SocketSet );
	if ( sock_server ) SDLNet_TCP_Close ( sock_server );
	for ( int i=0;i<8;i++ )
	{
		if ( sock_client[i] ) SDLNet_TCP_Close ( sock_client[i] );
	}
	status=STAT_CLOSED;
}

bool cFSTcpIp::FSTcpIpCreate()
{
	// Server
	if ( server )
	{
		if ( SDLNet_ResolveHost ( &addr,NULL,port ) <0 )
		{
			return false;
		}
	}
	// Client
	else
	{
		if ( SDLNet_ResolveHost ( &addr,ip.c_str(),port ) <0 )
		{
			return false;
		}
	}
	return true;
}

bool cFSTcpIp::FSTcpIpOpen ( void )
{
	FSTcpIpCreate();
	// Server
	if ( server )
	{
		sock_server = SDLNet_TCP_Open ( &addr );
		if ( sock_server == NULL )
		{
			return false;
		}
		status=STAT_OPENED;
		while ( num_clients < max_clients )
		{
			if ( num_clients > 0 )
				FSTcpIpReceive();
			else
				SDL_Delay ( 1000 );
			sock_client[num_clients] = SDLNet_TCP_Accept ( sock_server );
			if ( sock_client[num_clients] != NULL )
			{
				num_clients++;
				SocketSet = SDLNet_AllocSocketSet ( num_clients );
				for ( int i=0;i<=num_clients;i++ )
					SDLNet_TCP_AddSocket ( SocketSet, sock_client[i] );
			}
		}
		status=STAT_CONNECTED;
	}
	// Client
	else
	{
		sock_server = SDLNet_TCP_Open ( &addr );
		if ( sock_server == NULL )
		{
			return false;
		}
		status=STAT_CONNECTED;
	}
	return true;
}

bool cFSTcpIp::FSTcpIpSend ( int typ,const void *msg,int len,int num )
{
	// Nachricht anpassen
	if ( len>1024 ) len = 1023;
	this->msg->typ=typ;
	this->msg->lenght=len;
	memcpy ( this->msg->msg, ( char * ) msg,len );
	this->msg->msg[len]='\0';
	this->msg->playerid=PlayerId;
	// Server
	int i = 0;
	if ( server )
		while ( sock_client[i] != NULL && i < max_clients )
		{
			if ( i != num )
				SDLNet_TCP_Send ( sock_client[i], this->msg, sizeof ( cNetMessage ) );
			i++;
		}
	else
		// Client
		SDLNet_TCP_Send ( sock_server, this->msg, sizeof ( cNetMessage ) );
	return true;
}

bool cFSTcpIp::FSTcpIpReceive()
{
	// Server
	int i = 0;
	if ( server )
	{
		SDLNet_CheckSockets ( SocketSet, 1000 * 1 );
		while ( i < num_clients )
		{
			if ( SDLNet_SocketReady ( sock_client[i] ) )
			{
				SDLNet_TCP_Recv ( sock_client[i], msg, sizeof ( cNetMessage ) );
				MultiPlayer->MessageList->AddNetMessage ( msg );
				//FSTcpIpSend(msg->typ,msg->msg,msg->lenght, i);
				SDL_Delay ( 5 );
			}
			i++;
		}
	}
	// Client
	else
	{
		SDLNet_TCP_Recv ( sock_server, msg, sizeof ( cNetMessage ) );
		MultiPlayer->MessageList->AddNetMessage ( msg );
	}
	if ( !server || status==STAT_CONNECTED )
		FSTcpIpReceiveThread=NULL;
	return true;
}

void cFSTcpIp::SetTcpIpPort ( int port )
{
	this->port=port;
}

void cFSTcpIp::SetIp ( string ip )
{
	this->ip=ip;
}

int cFSTcpIp::GetConnectionCount()
{
	return num_clients;
}

int Receive ( void * )
{
	MultiPlayer->fstcpip->FSTcpIpReceive();
	return 1;
}

int Open ( void * )
{
	if ( MultiPlayer->fstcpip->FSTcpIpOpen() )
		return 1;
	else
		return 0;
}
