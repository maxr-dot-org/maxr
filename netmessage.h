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

#ifndef _net_message_h
#define _net_message_h

#include <SDL.h>
#include <string>
#include "main.h"

/**
* This class represents a message between server and client
* It provides some usefull interfaces to its containing data
*@author Eiko
*/
class cNetMessage
{
	char* data;		//the data of the netMessage

public:
	
	/** length of the containing data in bytes */
	int iLength;	
	/** should be set by the send function. Client->Host: message source, Host->Client: message destiantion */
	int iPlayerNr;	
	/** the type of the message */
	int iType;

	/** creates a new netMessage with iType
	* @author Eiko
	* @param iType the type of the message
	*/
	cNetMessage(int iType);

	/** creates a netMessage from a former serialized netMessage
	* @author Eiko
	* @param c the serial data from a netMessage
	*/
	cNetMessage( char* c );

	~cNetMessage();

	/** return a pointer to a platform independed serial representation of the NetMessage
	* Bytes 0 - 1: Type of the message, in little endian
	* Bytes 2 - 3: total Length of the message, little endian
	* Byte  4: the Playernumber
	* following Bytes: the pushed data
	* @author Eiko
	*/
	char* serialize();
	
	/** allocates a new SDL_Event containing the serial representation of the netMessage
	* @author Eiko
	* @return the SDL_Event
	*/
	SDL_Event* getGameEvent();

	/** pushes a char to the end of the netMessage
	* @author Eiko
	* @param c the char to push to the message
	*/
	void pushChar( char c);

	/** pops an char from the end of the netMessage
	* @author Eiko
	* @return the char poped from the message
	*/
	char popChar();

	/** pushes an Sint16 to the end of the netMessage
	* @author Eiko
	* @param i the Sint16 to push to the message
	*/
	void pushInt16( Sint16 i );

	/** pops a Sint16 from the end of the netMessage
	* @author Eiko
	* @return the Sint16 poped from the message
	*/
	Sint16 popInt16();

	/** pushes a Sint32 to the end of the netMessage
	* @author Eiko
	* @param i the Sint32 to push to the message
	*/
	void pushInt32( Sint32 i );

	/** pops a Sint 32 from the end of the netMessage
	* @author Eiko
	* @return the Sint32 poped from the message
	*/
	Sint32 popInt32 ();

	/** pushes a string to the end of the netMessage
	* @author Eiko
	* @param s the string to push to the message
	*/
	void pushString( string &s );

	/** pops a string from the end of the netMessage
	* @author Eiko
	* @return the string poped from the message
	*/
	string popString();

	/** pushes a bool to the end of the netMessage
	* @author Eiko
	* @param b the bool to push to the message
	*/
	void pushBool( bool b );

	/** pops a bool from the end of the netMessage
	* @author Eiko
	* @return the bool poped from the message
	*/
	bool popBool();

	#define BITS 32
	#define EXPBITS 8
	/** pushes a float to the end of the netMessage
	* @author Eiko
	* @param f the float to push to the message
	*/
	void pushFloat( float f );

	/** pops a float from the end of the netMessage
	* @author Eiko
	* @return the float poped from the message
	*/
	float popFloat();

	/** returns the string representation of iType
	* this is only for better readability of the netlog
	* @author Eiko
	*/
	string getTypeAsString();

	/** returns the serial hexadecimal representation of the netMessage
	* @author Eiko
	*/
	string getHexDump();
	
};

#endif //#ifndef _net_message_h
