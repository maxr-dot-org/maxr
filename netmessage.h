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
	

	//TODO: type float

};

#endif //#ifndef _net_message_h
