////////////////////////////////////////////////////////////////////////////////
//
//  File:   extendedtinyxml.h
//  Date:   07-10-01
//  Author: JCK
//
////////////////////////////////////////////////////////////////////////////////
//  Description:
//  Improves the TinyXML family by adding ExTiXmlNode. This class is a bid more
//  user-friendly.
//
//	Example for usage is added at the end of this file.
// 
////////////////////////////////////////////////////////////////////////////////

#ifndef EXTENDEDTINYXML_H
#define EXTENDEDTINYXML_H

#include "tinyxml.h"
#include <stdarg.h>
#include <string>



class ExTiXmlNode : public TiXmlNode
{
	friend class TiXmlDocument;
	friend class TiXmlBase;
	friend class TiXmlNode;
	friend class TiXmlParsingData;
	friend class TiXmlElement;

public:
	enum XML_NODE_TYPE
	{
		eXML_DOCUMENT,
		eXML_ELEMENT,
		eXML_COMMENT,
		eXML_UNKNOWN,
		eXML_TEXT,
		eXML_DECLARATION,
		eXML_TYPECOUNT,
		eXML_ATTRIBUTE
	};

	/// Get the first Node with a matching path
	ExTiXmlNode* XmlGetFirstNode( TiXmlDocument & rTiXmlDoc, const char * pszCurrent, ... );

	/// Navigate to a sibling node.
	ExTiXmlNode* XmlGetNextNodeSibling();

	/// Retrieve data from a node
	ExTiXmlNode * XmlReadNodeData( std::string &rstrData, XML_NODE_TYPE eType );
	ExTiXmlNode * XmlReadNodeData( std::string &rstrData, XML_NODE_TYPE eType, const char * pszAttributeName );
	int XmlGetLastEditor( std::string &rstrData, ExTiXmlNode * pXmlAuthorNode );

	bool XmlDataToBool( std::string &rstrData );
	inline long XmlDataToLong( std::string &rstrData ){ return atol(rstrData.c_str()); };
	inline double XmlDataToDouble( std::string &rstrData ){ return atof(rstrData.c_str()); };
	int CheckTimeStamp( std::string &rstrData );


	// Overrides. Do not use them !
	void Print( FILE* cfile, int depth ) const {}
	const char* Parse(	const char* p, TiXmlParsingData* data, TiXmlEncoding encoding  ){}
	ExTiXmlNode* Clone() const {}
	bool Accept( TiXmlVisitor* visitor ) const {}
	#ifdef TIXML_USE_STL
		void StreamIn( std::istream* in, TIXML_STRING* tag ){}
	#endif

};


/*
int _tmain(int argc, _TCHAR* argv[])
{
	int iRet;
	std::string strResult;
	std::string strConcat;

	// Use a standard TinyXML Document. Do not use a Pointer!
	TiXmlDocument XmlDoc;

	// You have to use a pointer!
	ExTiXmlNode * pXmlNode;

	// Load a XML file using the standard TinyXML DocClass
	if( !XmlDoc.LoadFile( "Language GER.xml", TIXML_DEFAULT_ENCODING ))
	{
		return 0;
	}

	// Initialize the pointer
	pXmlNode = NULL;

	// Get the first node with a matching path.
	// NULL is the ending sign. Do not foget it!
	pXmlNode = pXmlNode->XmlGetFirstNode( XmlDoc, "MAX_Language_File", "Header","Author", NULL );
	//pXmlNode = pXmlNode->XmlGetFirstNode( XmlDoc, "MAX_Language_File", "Header","Author", "Editor", NULL );
	if( pXmlNode != NULL )
	{
		// Get the data of the attribute "name"
		if( pXmlNode->XmlReadNodeData( strResult, ExTiXmlNode::eXML_ATTRIBUTE, "name" ) == NULL )
		{
			; // Attribute not found
		}else
		{
			printf("%s\n", strResult.c_str());
		}
	}

	pXmlNode = pXmlNode->XmlGetFirstNode( XmlDoc, "MAX_Language_File", "Header", NULL );
	while( pXmlNode != NULL )
	{
		// Get the TEXT data within the node
		pXmlNode = pXmlNode->XmlReadNodeData( strResult, ExTiXmlNode::eXML_TEXT );
		strConcat += strResult;
		if( pXmlNode == NULL )
		{
			printf("%s\n",strConcat.c_str()); // No more text is found _OR_ the found text was the last within this level
		}
	}

	strConcat = "";
	pXmlNode = pXmlNode->XmlGetFirstNode( XmlDoc, "MAX_Language_File", "Header", NULL );
	while( pXmlNode != NULL )
	{
		// Get the COMMENT data within the node
		pXmlNode = pXmlNode->XmlReadNodeData( strResult, ExTiXmlNode::eXML_COMMENT );
		strConcat += strResult;
		if( pXmlNode == NULL )
		{
			printf("%s\n",strConcat.c_str()); // No more comments are found _OR_ the found comment was the last within this level
		}
	}

	pXmlNode = pXmlNode->XmlGetFirstNode( XmlDoc, "MAX_Language_File", "Header","Author", "Editor", NULL );
	if( pXmlNode != NULL )
	{
		// Get the data of the attribute "name"
		do
		{
			if( pXmlNode->XmlReadNodeData( strResult, ExTiXmlNode::eXML_ATTRIBUTE, "name" ) == NULL )
			{
				printf("%s\n",strResult.c_str()); // Attribute not found
			}else if( strResult != "Someone" )
			{
				printf("%s\n",strResult.c_str()); // Not the right value is found
			}else
			{
				printf("Someone was found!\n"); // Node with the correct path and attribute is found
			}
			pXmlNode = pXmlNode->XmlGetNextNodeSibling();
		}while( pXmlNode != NULL );
	}

	pXmlNode = pXmlNode->XmlGetFirstNode( XmlDoc, "MAX_Language_File", "Header", NULL );
	while( pXmlNode != NULL )
	{
		pXmlNode = pXmlNode->XmlGetNextNodeSibling();
		if( pXmlNode != NULL )
		{
			// Now let's use an inherited function
			iRet = pXmlNode->Type();
			switch( iRet )
			{
				case ExTiXmlNode::eXML_DOCUMENT:	printf("eXML_DOCUMENT\n") ;break;
				case ExTiXmlNode::eXML_ELEMENT:		printf("eXML_ELEMENT\n");break;
				case ExTiXmlNode::eXML_COMMENT:		printf("eXML_COMMENT\n");break;
				case ExTiXmlNode::eXML_UNKNOWN:		printf("eXML_UNKNOWN\n");break;
				case ExTiXmlNode::eXML_TEXT:		printf("eXML_TEXT\n");break;
				case ExTiXmlNode::eXML_DECLARATION: printf("eXML_DECLARATION\n");break;
				case ExTiXmlNode::eXML_TYPECOUNT:	printf("eXML_TYPECOUNT\n");break;
				case ExTiXmlNode::eXML_ATTRIBUTE:	printf("eXML_ATTRIBUTE\n");break;
				default:
					printf("default");break;
			}
		}
	}

	return 0;
}
*/

#endif
