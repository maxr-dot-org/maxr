////////////////////////////////////////////////////////////////////////////////
//
//  File:   ExtendedTinyXml.cpp
//  Date:   07-10-01
//  Author: JCK
//
////////////////////////////////////////////////////////////////////////////////
//  Description:
//  Improves the TinyXML family by adding ExTiXmlNode. This class is a bid more
//  user-friendly.
//
//	Example for usage is added in "ExtendedTinyXml.h"
// 
////////////////////////////////////////////////////////////////////////////////


#include "ExtendedTinyXml.h"
#include "defines.h"
#include "log.h"

void debugToLog( std::string szMsg);
void debugToLog( void * pointer , const char * pszName);


ExTiXmlNode* ExTiXmlNode::XmlGetFirstNode( TiXmlDocument &rTiXmlDoc, const char * pszCurrent, ... )
{
	va_list pvaArg;
	va_start(pvaArg, pszCurrent);
	
	std::string szDebug;
	debugToLog(" 0 : XmlGetFirstNode : Start");


	TiXmlNode * pXmlNode;

	debugToLog( & rTiXmlDoc, " 1 : XmlGetFirstNode : rTiXmlDoc");

	if( rTiXmlDoc.Value() == NULL )
	{
		debugToLog(" 2 : XmlGetFirstNode : rTiXmlDoc fail");
		va_end( pvaArg );
		return NULL;
	}

	pXmlNode = rTiXmlDoc.RootElement();
	debugToLog( pXmlNode, " 3 : XmlGetFirstNode : pXmlNode");
	if( pXmlNode == NULL )
	{
		debugToLog(" 4 : XmlGetFirstNode : RootElement > pXmlNode fail");
		va_end( pvaArg );
		return NULL;
	}

//	szDebug = " 5 : XmlGetFirstNode : pszCurrent" + std::string( pszCurrent );
	debugToLog( szDebug );
	if( strcmp(pXmlNode->Value(), pszCurrent) != 0 )
	{
		debugToLog(" 6 : XmlGetFirstNode : pXmlNode != pszCurrent > fail");
		va_end( pvaArg );
		return NULL;
	}

	do
	{
		pszCurrent = va_arg(pvaArg, char * );
//		szDebug = " 7 : XmlGetFirstNode : pszCurrent" + std::string( pszCurrent );
		debugToLog( szDebug );
		if( pszCurrent != NULL )
		{
			debugToLog( pXmlNode, " 8 : XmlGetFirstNode : pXmlNode");
			pXmlNode = pXmlNode->FirstChild( pszCurrent );
			debugToLog( pXmlNode, " 9 : XmlGetFirstNode : pXmlNode");
			if( pXmlNode == NULL )
			{
				debugToLog("10 : XmlGetFirstNode : FirstChild > pXmlNode fail");
				va_end( pvaArg );
				return NULL;
			}
		}
	}while( pszCurrent != NULL );

	debugToLog( pXmlNode, "11 : XmlGetFirstNode : pXmlNode != 0 SUCCESS");
	debugToLog( (ExTiXmlNode *)pXmlNode, "12 : XmlGetFirstNode : pXmlNode cast Test");
	debugToLog("13 : XmlGetFirstNode : End !");

	return (ExTiXmlNode *)pXmlNode;
}

ExTiXmlNode* ExTiXmlNode::XmlGetNextNodeSibling()
{
	TiXmlNode * pXmlNode;
	if( this == NULL )
	{
		return NULL;
	}
	pXmlNode = this;

	pXmlNode = pXmlNode->NextSibling();

	return (ExTiXmlNode *)pXmlNode;
}


ExTiXmlNode * ExTiXmlNode::XmlReadNodeData( std::string &rstrData, XML_NODE_TYPE eType )
{
	bool bDataFoundPossible = true;
	TiXmlNode * pXmlNode = (TiXmlNode *)this;
	rstrData = "";

	if( this == NULL )
	{
		return NULL;
	}
	switch( eType )
	{
	case ExTiXmlNode::eXML_ATTRIBUTE :
			return NULL;
			break;
		case ExTiXmlNode::eXML_COMMENT :
			if ( pXmlNode->Type() == TiXmlNode::ELEMENT )
			{
				pXmlNode = pXmlNode->FirstChild();
			}
			if( pXmlNode == NULL )
			{
				return NULL;
			}
			do
			{
				if( pXmlNode->Type() == TiXmlNode::COMMENT )
				{
					rstrData =  pXmlNode->ToComment()->Value();
					bDataFoundPossible = false; // Get only the first data !
				}
				pXmlNode = pXmlNode->NextSibling();
				if( pXmlNode == NULL ) 
				{
					bDataFoundPossible = false; // Last sibling already checked
				}
			}while( bDataFoundPossible );
			break;
		case ExTiXmlNode::eXML_TEXT :
			if ( pXmlNode->Type() == TiXmlNode::ELEMENT )
			{
				pXmlNode = pXmlNode->FirstChild();
			}
			if( pXmlNode == NULL )
			{
				return NULL;
			}
			do
			{
				if( pXmlNode->Type() == TiXmlNode::TEXT )
				{
					rstrData =  pXmlNode->ToText()->Value();
					bDataFoundPossible = false; // Get only the first data !
				}
				pXmlNode = pXmlNode->NextSibling();
				if( pXmlNode == NULL ) 
				{
					bDataFoundPossible = false; // Last sibling already checked
				}
			}while( bDataFoundPossible );
			break;
		default :
		rstrData = "";
		return NULL;
	}
	return (ExTiXmlNode *) pXmlNode;
}


ExTiXmlNode * ExTiXmlNode::XmlReadNodeData( std::string &rstrData, XML_NODE_TYPE eType,  const char * pszAttributeName )
{
	TiXmlNode * pXmlNode = (TiXmlNode *)this;
	TiXmlElement * pXmlElement;
	const char * pszTemp;

	rstrData = "";

	if( this == NULL )
	{
		return NULL;
	}
	switch( eType )
	{
		case ExTiXmlNode::eXML_ATTRIBUTE :
			if( pXmlNode->Type() != TiXmlNode::ELEMENT ) return NULL;
			pXmlElement = pXmlNode->ToElement();// FirstChildElement();
			if( pXmlElement == NULL )
			{
				return NULL;
			}
			pszTemp =  pXmlElement->Attribute( pszAttributeName );
			if( pszTemp == 0 )
			{
				return NULL;
			}else
			{
				rstrData = pszTemp;
			}
			break;
		case ExTiXmlNode::eXML_COMMENT :
			return NULL;
			break;
		case ExTiXmlNode::eXML_TEXT :
			return NULL;
			 break;
		default :
			return NULL;
	}
	return (ExTiXmlNode *)pXmlNode;
}

int ExTiXmlNode::CheckTimeStamp( std::string &rstrData )
{
	//JCK: Should be replaced by a faster and more secure function


	// Index   : 0123456789012345678
	// Example : 2007-09-30 13:04:00

	if( rstrData.length() != 19 ) return -1;
	if(	! isdigit( rstrData[0] )) return -1;
	if(	! isdigit( rstrData[1] )) return -1;
	if(	! isdigit( rstrData[2] )) return -1;
	if(	! isdigit( rstrData[3] )) return -1;
	if(	 rstrData[4] != '-' ) return -1;
	if(	! isdigit( rstrData[5] )) return -1;
	if(	! isdigit( rstrData[6] )) return -1;
	if(	 rstrData[7] != '-' ) return -1;
	if(	! isdigit( rstrData[8] )) return -1;
	if(	! isdigit( rstrData[9] )) return -1;
	if(	 rstrData[10] != ' ' ) return -1;
	if(	! isdigit( rstrData[11] )) return -1;
	if(	! isdigit( rstrData[12] )) return -1;
	if(	 rstrData[13] != ':' ) return -1;
	if(	! isdigit( rstrData[14] )) return -1;
	if(	! isdigit( rstrData[15] )) return -1;
	if(	 rstrData[16] != ':' ) return -1;
	if(	! isdigit( rstrData[17] )) return -1;
	if(	! isdigit( rstrData[18] )) return -1;

	std::string szTemp1 = rstrData;

	szTemp1.erase(16,1);
	szTemp1.erase(13,1);
	szTemp1.erase(10,1);
	szTemp1.erase( 7,1);
	szTemp1.erase( 4,1);

	std::string szTemp2 = MAX_BUILD_DATE;

	szTemp2.erase(16,1);
	szTemp2.erase(13,1);
	szTemp2.erase(10,1);
	szTemp2.erase( 7,1);
	szTemp2.erase( 4,1);

	if( szTemp1 < szTemp2 )
	{
		return 0; // XML is older than the game
	}
	if( szTemp1 > szTemp2 )
	{
		return 1; // XML is newer than the game
	}
	return 2;     // XML is matching the game
}

int ExTiXmlNode::XmlGetLastEditor( std::string &rstrData, ExTiXmlNode * pXmlAuthorNode )
{
	rstrData = "";

	// ToDo - JCK: Find the last editor of the XML file
	return 0;
}

bool ExTiXmlNode::XmlDataToBool( std::string &rstrData )
{
	// Default value = true !!!

	// is it a number ?
	if( rstrData.find_first_not_of(" 01234567890,.+-") == rstrData.npos )
	{
		if( atoi( rstrData.c_str() ) == 0 )
		{
			return false;
		}else
		{
			return true;
		}
	}else // no number ! only first letter is important !
	{
		std::string szTemp = rstrData;
		while(szTemp[0] == ' ')
		{
			szTemp.erase(0);
		}
		if( szTemp[0] == 'f' || szTemp[0] == 'F' || szTemp[0] == 'n' || szTemp[0] == 'N' )
		{
						return false;
		}else
		{
			return true;
		}
	}
}

void debugToLog( void * pointer , const char * pszName)
{
	char szMsg[256] = ""; //JCK
	sprintf(szMsg , "%s = %p", pszName, pointer);

	cLog::write( szMsg, cLog::eLOG_TYPE_DEBUG );
};

void debugToLog( std::string szMsg)
{
	cLog::write( szMsg.c_str(), cLog::eLOG_TYPE_DEBUG );
};
