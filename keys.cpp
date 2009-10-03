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
#include "files.h"
#include "extendedtinyxml.h"
#include "keys.h"
#include "log.h"
#include "settings.h"

// Funktionen ////////////////////////////////////////////////////////////////
int LoadKeys ()
{
	Log.write ( "Loading Keys", LOG_TYPE_INFO );
	TiXmlDocument KeysXml;
	ExTiXmlNode * pXmlNode = NULL;
	string sTmpString;

	if (!FileExists(KEYS_XML))
	{
		Log.write ( "generating new file", LOG_TYPE_WARNING );
		GenerateKeysXml();
	}
	if (!KeysXml.LoadFile(KEYS_XML))
	{
		Log.write ( "cannot load keys.xml\ngenerating new file", LOG_TYPE_WARNING );
		GenerateKeysXml();
	}

	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyExit", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyExit = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyExit from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyExit = GetKeyFromString ( "ESCAPE" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyJumpToAction", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyJumpToAction = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyJumpToAction from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyJumpToAction = GetKeyFromString ( "F1" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyEndTurn", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyEndTurn = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyEndTurn from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyEndTurn = GetKeyFromString ( "RETURN" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyChat", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyChat = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyChat from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyChat = GetKeyFromString ( "TAB" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyScroll8a", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyScroll8a = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyScroll8a from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyScroll8a = GetKeyFromString ( "UP" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyScroll8b", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyScroll8b = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyScroll8b from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyScroll8b = GetKeyFromString ( "KP8" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyScroll2a", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyScroll2a = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyScroll2a from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyScroll2a = GetKeyFromString ( "DOWN" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyScroll2b", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyScroll2b = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyScroll2b from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyScroll2b = GetKeyFromString ( "KP2" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyScroll6a", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyScroll6a = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyScroll6a from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyScroll6a = GetKeyFromString ( "RIGHT" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyScroll6b", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyScroll6b = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyScroll6b from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyScroll6b = GetKeyFromString ( "KP6" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyScroll4a", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyScroll4a = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyScroll4a from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyScroll4a = GetKeyFromString ( "LEFT" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyScroll4b", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyScroll4b = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyScroll4b from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyScroll4b = GetKeyFromString ( "KP4" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyScroll7", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyScroll7 = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyScroll7 from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyScroll7 = GetKeyFromString ( "KP7" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyScroll9", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyScroll9 = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyScroll9 from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyScroll9 = GetKeyFromString ( "KP9" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyScroll1", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyScroll1 = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyScroll1 from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyScroll1 = GetKeyFromString ( "KP1" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyScroll3", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyScroll3 = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyScroll3 from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyScroll3 = GetKeyFromString ( "KP3" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyZoomIna", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyZoomIna = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyZoomIna from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyZoomIna = GetKeyFromString ( "RIGHTBRACKET" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyZoomInb", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyZoomInb = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyZoomInb from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyZoomInb = GetKeyFromString ( "KP_PLUS" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyZoomOuta", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyZoomOuta = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyZoomOuta from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyZoomOuta = GetKeyFromString ( "SLASH" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyZoomOutb", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyZoomOutb = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyZoomOutb from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyZoomOutb = GetKeyFromString ( "KP_MINUS" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyFog", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyFog = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyFog from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyFog = GetKeyFromString ( "N" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyGrid", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyGrid = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyGrid from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyGrid = GetKeyFromString ( "G" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyScan", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyScan = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyScan from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyScan = GetKeyFromString ( "S" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyRange", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyRange = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyRange from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyRange = GetKeyFromString ( "R" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyAmmo", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyAmmo = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyAmmo from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyAmmo = GetKeyFromString ( "M" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyHitpoints", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyHitpoints = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyHitpoints from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyHitpoints = GetKeyFromString ( "T" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyColors", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyColors = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyColors from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyColors = GetKeyFromString ( "F" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyStatus", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyStatus = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyStatus from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyStatus = GetKeyFromString ( "P" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeySurvey", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeySurvey = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeySurvey from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeySurvey = GetKeyFromString ( "H" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyCalcPath", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyCalcPath = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyCalcPath from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyCalcPath = GetKeyFromString ( "LSHIFT" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyCenterUnit", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyCenterUnit = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyCalcPath from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyCenterUnit = GetKeyFromString ( "F" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyUnitMenuAttack", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyUnitMenuAttack = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyCalcPath from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyUnitMenuAttack = GetKeyFromString ( "A" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyUnitMenuBuild", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyUnitMenuBuild = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyCalcPath from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyUnitMenuBuild = GetKeyFromString ( "B" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyUnitMenuTransfer", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyUnitMenuTransfer = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyCalcPath from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyUnitMenuTransfer = GetKeyFromString ( "X" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyUnitMenuAutomove", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyUnitMenuAutomove = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyCalcPath from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyUnitMenuAutomove = GetKeyFromString ( "A" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyUnitMenuStart", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyUnitMenuStart = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyCalcPath from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyUnitMenuStart = GetKeyFromString ( "S" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyUnitMenuStop", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyUnitMenuStop = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyCalcPath from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyUnitMenuStop = GetKeyFromString ( "S" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyUnitMenuClear", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyUnitMenuClear = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyCalcPath from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyUnitMenuClear = GetKeyFromString ( "C" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyUnitMenuSentry", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyUnitMenuSentry = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyCalcPath from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyUnitMenuSentry = GetKeyFromString ( "S" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyUnitMenuActivate", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyUnitMenuActivate = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyCalcPath from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyUnitMenuActivate = GetKeyFromString ( "A" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyUnitMenuLoad", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyUnitMenuLoad = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyCalcPath from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyUnitMenuLoad = GetKeyFromString ( "L" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyUnitMenuReload", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyUnitMenuReload = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyCalcPath from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyUnitMenuReload = GetKeyFromString ( "R" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyUnitMenuRepair", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyUnitMenuRepair = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyCalcPath from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyUnitMenuRepair = GetKeyFromString ( "R" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyUnitMenuLayMine", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyUnitMenuLayMine = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyCalcPath from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyUnitMenuLayMine = GetKeyFromString ( "L" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyUnitMenuClearMine", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyUnitMenuClearMine = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyCalcPath from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyUnitMenuClearMine = GetKeyFromString ( "C" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyUnitMenuDisable", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyUnitMenuDisable = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyCalcPath from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyUnitMenuDisable = GetKeyFromString ( "D" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyUnitMenuSteal", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyUnitMenuSteal = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyCalcPath from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyUnitMenuSteal = GetKeyFromString ( "S" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyUnitMenuInfo", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyUnitMenuInfo = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyCalcPath from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyUnitMenuInfo = GetKeyFromString ( "H" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyUnitMenuDistribute", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyUnitMenuDistribute = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyCalcPath from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyUnitMenuDistribute = GetKeyFromString ( "D" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyUnitMenuResearch", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyUnitMenuResearch = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyCalcPath from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyUnitMenuResearch = GetKeyFromString ( "R" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyUnitMenuUpgrade", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyUnitMenuUpgrade = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyCalcPath from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyUnitMenuUpgrade = GetKeyFromString ( "U" );
	}
	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Keys","KeyUnitMenuDestroy", NULL);
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		KeysList.KeyUnitMenuDestroy = GetKeyFromString ( sTmpString );
	else
	{
		Log.write ( "Can't load KeyCalcPath from keys.xml: using default value", LOG_TYPE_WARNING );
		KeysList.KeyUnitMenuDestroy = GetKeyFromString ( "D" );
	}

	pXmlNode = pXmlNode->XmlGetFirstNode(KeysXml,"Controles","Mouse","MOUSE_STYLE", NULL);
	pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text");
	if ( sTmpString.compare ( "OLD_SCHOOL" ) == 0 )
		MouseStyle = OldSchool;
	else
		MouseStyle = Modern;

	Log.write ( "Done", LOG_TYPE_DEBUG );
	return 1;
}

// Liefert einen String mit dem Namen der Taste zurueck:
const char *GetKeyString ( SDLKey key )
{
	switch ( key )
	{
		case SDLK_UNKNOWN:return "UNKNOWN";
		case SDLK_BACKSPACE:return "BACKSPACE";
		case SDLK_TAB:return "TAB";
		case SDLK_CLEAR:return "CLEAR";
		case SDLK_RETURN:return "RETURN";
		case SDLK_PAUSE:return "PAUSE";
		case SDLK_ESCAPE:return "ESCAPE";
		case SDLK_SPACE:return "SPACE";
		case SDLK_EXCLAIM:return "EXCLAIM";
		case SDLK_QUOTEDBL:return "QUOTEDBL";
		case SDLK_HASH:return "HASH";
		case SDLK_DOLLAR:return "DOLLAR";
		case SDLK_AMPERSAND:return "AMPERSAND";
		case SDLK_QUOTE:return "QUOTE";
		case SDLK_LEFTPAREN:return "LEFTPAREN";
		case SDLK_RIGHTPAREN:return "RIGHTPAREN";
		case SDLK_ASTERISK:return "ASTERISK";
		case SDLK_PLUS:return "PLUS";
		case SDLK_COMMA:return "COMMA";
		case SDLK_MINUS:return "MINUS";
		case SDLK_PERIOD:return "PERIOD";
		case SDLK_SLASH:return "SLASH";
		case SDLK_0:return "0";
		case SDLK_1:return "1";
		case SDLK_2:return "2";
		case SDLK_3:return "3";
		case SDLK_4:return "4";
		case SDLK_5:return "5";
		case SDLK_6:return "6";
		case SDLK_7:return "7";
		case SDLK_8:return "8";
		case SDLK_9:return "9";
		case SDLK_COLON:return "COLON";
		case SDLK_SEMICOLON:return "SEMICOLON";
		case SDLK_LESS:return "LESS";
		case SDLK_EQUALS:return "EQUALS";
		case SDLK_GREATER:return "GREATER";
		case SDLK_QUESTION:return "QUESTION";
		case SDLK_AT:return "AT";
		case SDLK_LEFTBRACKET:return "LEFTBRACKET";
		case SDLK_BACKSLASH:return "BACKSLASH";
		case SDLK_RIGHTBRACKET:return "RIGHTBRACKET";
		case SDLK_CARET:return "CARET";
		case SDLK_UNDERSCORE:return "UNDERSCORE";
		case SDLK_BACKQUOTE:return "BACKQUOTE";
		case SDLK_a:return "A";
		case SDLK_b:return "B";
		case SDLK_c:return "C";
		case SDLK_d:return "D";
		case SDLK_e:return "E";
		case SDLK_f:return "F";
		case SDLK_g:return "G";
		case SDLK_h:return "H";
		case SDLK_i:return "I";
		case SDLK_j:return "J";
		case SDLK_k:return "K";
		case SDLK_l:return "L";
		case SDLK_m:return "M";
		case SDLK_n:return "N";
		case SDLK_o:return "O";
		case SDLK_p:return "P";
		case SDLK_q:return "Q";
		case SDLK_r:return "R";
		case SDLK_s:return "S";
		case SDLK_t:return "T";
		case SDLK_u:return "U";
		case SDLK_v:return "V";
		case SDLK_w:return "W";
		case SDLK_x:return "X";
		case SDLK_y:return "Y";
		case SDLK_z:return "Z";
		case SDLK_DELETE:return "DELETE";
		case SDLK_KP0:return "KP0";
		case SDLK_KP1:return "KP1";
		case SDLK_KP2:return "KP2";
		case SDLK_KP3:return "KP3";
		case SDLK_KP4:return "KP4";
		case SDLK_KP5:return "KP5";
		case SDLK_KP6:return "KP6";
		case SDLK_KP7:return "KP7";
		case SDLK_KP8:return "KP8";
		case SDLK_KP9:return "KP9";
		case SDLK_KP_PERIOD:return "KP_PERIOD";
		case SDLK_KP_DIVIDE:return "KP_DIVIDE";
		case SDLK_KP_MULTIPLY:return "KP_MULTIPLY";
		case SDLK_KP_MINUS:return "KP_MINUS";
		case SDLK_KP_PLUS:return "KP_PLUS";
		case SDLK_KP_ENTER:return "KP_ENTER";
		case SDLK_KP_EQUALS:return "KP_EQUALS";
		case SDLK_UP:return "UP";
		case SDLK_DOWN:return "DOWN";
		case SDLK_RIGHT:return "RIGHT";
		case SDLK_LEFT:return "LEFT";
		case SDLK_INSERT:return "INSERT";
		case SDLK_HOME:return "HOME";
		case SDLK_END:return "END";
		case SDLK_PAGEUP:return "PAGEUP";
		case SDLK_PAGEDOWN:return "PAGEDOWN";
		case SDLK_F1:return "F1";
		case SDLK_F2:return "F2";
		case SDLK_F3:return "F3";
		case SDLK_F4:return "F4";
		case SDLK_F5:return "F5";
		case SDLK_F6:return "F6";
		case SDLK_F7:return "F7";
		case SDLK_F8:return "F8";
		case SDLK_F9:return "F9";
		case SDLK_F10:return "F10";
		case SDLK_F11:return "F11";
		case SDLK_F12:return "F12";
		case SDLK_F13:return "F13";
		case SDLK_F14:return "F14";
		case SDLK_F15:return "F15";
		case SDLK_NUMLOCK:return "NUMLOCK";
		case SDLK_CAPSLOCK:return "CAPSLOCK";
		case SDLK_SCROLLOCK:return "SCROLLOCK";
		case SDLK_RSHIFT:return "RSHIFT";
		case SDLK_LSHIFT:return "LSHIFT";
		case SDLK_RCTRL:return "RCTRL";
		case SDLK_LCTRL:return "LCTRL";
		case SDLK_RALT:return "RALT";
		case SDLK_LALT:return "LALT";
		case SDLK_RMETA:return "RMETA";
		case SDLK_LMETA:return "LMETA";
		case SDLK_LSUPER:return "LSUPER";
		case SDLK_RSUPER:return "RSUPER";
		case SDLK_MODE:return "MODE";
		case SDLK_COMPOSE:return "COMPOSE";
		case SDLK_HELP:return "HELP";
		case SDLK_PRINT:return "PRINT";
		case SDLK_SYSREQ:return "SYSREQ";
		case SDLK_BREAK:return "BREAK";
		case SDLK_MENU:return "MENU";
		case SDLK_POWER:return "POWER";
		case SDLK_EURO:return "EURO";
		case SDLK_UNDO:return "UNDO";
		default: return "?";
	}
}

// Liefert den Code der Taste zurueck:
SDLKey GetKeyFromString ( string key )
{
	if ( !key.compare ( "UNKNOWN" ) ) return SDLK_UNKNOWN;
	if ( !key.compare ( "BACKSPACE" ) ) return SDLK_BACKSPACE;
	if ( !key.compare ( "TAB" ) ) return SDLK_TAB;
	if ( !key.compare ( "CLEAR" ) ) return SDLK_CLEAR;
	if ( !key.compare ( "RETURN" ) ) return SDLK_RETURN;
	if ( !key.compare ( "PAUSE" ) ) return SDLK_PAUSE;
	if ( !key.compare ( "ESCAPE" ) ) return SDLK_ESCAPE;
	if ( !key.compare ( "SPACE" ) ) return SDLK_SPACE;
	if ( !key.compare ( "EXCLAIM" ) ) return SDLK_EXCLAIM;
	if ( !key.compare ( "QUOTEDBL" ) ) return SDLK_QUOTEDBL;
	if ( !key.compare ( "HASH" ) ) return SDLK_HASH;
	if ( !key.compare ( "DOLLAR" ) ) return SDLK_DOLLAR;
	if ( !key.compare ( "AMPERSAND" ) ) return SDLK_AMPERSAND;
	if ( !key.compare ( "QUOTE" ) ) return SDLK_QUOTE;
	if ( !key.compare ( "LEFTPAREN" ) ) return SDLK_LEFTPAREN;
	if ( !key.compare ( "RIGHTPAREN" ) ) return SDLK_RIGHTPAREN;
	if ( !key.compare ( "ASTERISK" ) ) return SDLK_ASTERISK;
	if ( !key.compare ( "PLUS" ) ) return SDLK_PLUS;
	if ( !key.compare ( "COMMA" ) ) return SDLK_COMMA;
	if ( !key.compare ( "MINUS" ) ) return SDLK_MINUS;
	if ( !key.compare ( "PERIOD" ) ) return SDLK_PERIOD;
	if ( !key.compare ( "SLASH" ) ) return SDLK_SLASH;
	if ( !key.compare ( "0" ) ) return SDLK_0;
	if ( !key.compare ( "1" ) ) return SDLK_1;
	if ( !key.compare ( "2" ) ) return SDLK_2;
	if ( !key.compare ( "3" ) ) return SDLK_3;
	if ( !key.compare ( "4" ) ) return SDLK_4;
	if ( !key.compare ( "5" ) ) return SDLK_5;
	if ( !key.compare ( "6" ) ) return SDLK_6;
	if ( !key.compare ( "7" ) ) return SDLK_7;
	if ( !key.compare ( "8" ) ) return SDLK_8;
	if ( !key.compare ( "9" ) ) return SDLK_9;
	if ( !key.compare ( "COLON" ) ) return SDLK_COLON;
	if ( !key.compare ( "SEMICOLON" ) ) return SDLK_SEMICOLON;
	if ( !key.compare ( "LESS" ) ) return SDLK_LESS;
	if ( !key.compare ( "EQUALS" ) ) return SDLK_EQUALS;
	if ( !key.compare ( "GREATER" ) ) return SDLK_GREATER;
	if ( !key.compare ( "QUESTION" ) ) return SDLK_QUESTION;
	if ( !key.compare ( "AT" ) ) return SDLK_AT;
	if ( !key.compare ( "LEFTBRACKET" ) ) return SDLK_LEFTBRACKET;
	if ( !key.compare ( "BACKSLASH" ) ) return SDLK_BACKSLASH;
	if ( !key.compare ( "RIGHTBRACKET" ) ) return SDLK_RIGHTBRACKET;
	if ( !key.compare ( "CARET" ) ) return SDLK_CARET;
	if ( !key.compare ( "UNDERSCORE" ) ) return SDLK_UNDERSCORE;
	if ( !key.compare ( "BACKQUOTE" ) ) return SDLK_BACKQUOTE;
	if ( !key.compare ( "A" ) ) return SDLK_a;
	if ( !key.compare ( "B" ) ) return SDLK_b;
	if ( !key.compare ( "C" ) ) return SDLK_c;
	if ( !key.compare ( "D" ) ) return SDLK_d;
	if ( !key.compare ( "E" ) ) return SDLK_e;
	if ( !key.compare ( "F" ) ) return SDLK_f;
	if ( !key.compare ( "G" ) ) return SDLK_g;
	if ( !key.compare ( "H" ) ) return SDLK_h;
	if ( !key.compare ( "I" ) ) return SDLK_i;
	if ( !key.compare ( "J" ) ) return SDLK_j;
	if ( !key.compare ( "K" ) ) return SDLK_k;
	if ( !key.compare ( "L" ) ) return SDLK_l;
	if ( !key.compare ( "M" ) ) return SDLK_m;
	if ( !key.compare ( "N" ) ) return SDLK_n;
	if ( !key.compare ( "O" ) ) return SDLK_o;
	if ( !key.compare ( "P" ) ) return SDLK_p;
	if ( !key.compare ( "Q" ) ) return SDLK_q;
	if ( !key.compare ( "R" ) ) return SDLK_r;
	if ( !key.compare ( "S" ) ) return SDLK_s;
	if ( !key.compare ( "T" ) ) return SDLK_t;
	if ( !key.compare ( "U" ) ) return SDLK_u;
	if ( !key.compare ( "V" ) ) return SDLK_v;
	if ( !key.compare ( "W" ) ) return SDLK_w;
	if ( !key.compare ( "X" ) ) return SDLK_x;
	if ( !key.compare ( "Y" ) ) return SDLK_y;
	if ( !key.compare ( "Z" ) ) return SDLK_z;
	if ( !key.compare ( "DELETE" ) ) return SDLK_DELETE;
	if ( !key.compare ( "KP0" ) ) return SDLK_KP0;
	if ( !key.compare ( "KP1" ) ) return SDLK_KP1;
	if ( !key.compare ( "KP2" ) ) return SDLK_KP2;
	if ( !key.compare ( "KP3" ) ) return SDLK_KP3;
	if ( !key.compare ( "KP4" ) ) return SDLK_KP4;
	if ( !key.compare ( "KP5" ) ) return SDLK_KP5;
	if ( !key.compare ( "KP6" ) ) return SDLK_KP6;
	if ( !key.compare ( "KP7" ) ) return SDLK_KP7;
	if ( !key.compare ( "KP8" ) ) return SDLK_KP8;
	if ( !key.compare ( "KP9" ) ) return SDLK_KP9;
	if ( !key.compare ( "KP_PERIOD" ) ) return SDLK_KP_PERIOD;
	if ( !key.compare ( "KP_DIVIDE" ) ) return SDLK_KP_DIVIDE;
	if ( !key.compare ( "KP_MULTIPLY" ) ) return SDLK_KP_MULTIPLY;
	if ( !key.compare ( "KP_MINUS" ) ) return SDLK_KP_MINUS;
	if ( !key.compare ( "KP_PLUS" ) ) return SDLK_KP_PLUS;
	if ( !key.compare ( "KP_ENTER" ) ) return SDLK_KP_ENTER;
	if ( !key.compare ( "KP_EQUALS" ) ) return SDLK_KP_EQUALS;
	if ( !key.compare ( "UP" ) ) return SDLK_UP;
	if ( !key.compare ( "DOWN" ) ) return SDLK_DOWN;
	if ( !key.compare ( "RIGHT" ) ) return SDLK_RIGHT;
	if ( !key.compare ( "LEFT" ) ) return SDLK_LEFT;
	if ( !key.compare ( "INSERT" ) ) return SDLK_INSERT;
	if ( !key.compare ( "HOME" ) ) return SDLK_HOME;
	if ( !key.compare ( "END" ) ) return SDLK_END;
	if ( !key.compare ( "PAGEUP" ) ) return SDLK_PAGEUP;
	if ( !key.compare ( "PAGEDOWN" ) ) return SDLK_PAGEDOWN;
	if ( !key.compare ( "F1" ) ) return SDLK_F1;
	if ( !key.compare ( "F2" ) ) return SDLK_F2;
	if ( !key.compare ( "F3" ) ) return SDLK_F3;
	if ( !key.compare ( "F4" ) ) return SDLK_F4;
	if ( !key.compare ( "F5" ) ) return SDLK_F5;
	if ( !key.compare ( "F6" ) ) return SDLK_F6;
	if ( !key.compare ( "F7" ) ) return SDLK_F7;
	if ( !key.compare ( "F8" ) ) return SDLK_F8;
	if ( !key.compare ( "F9" ) ) return SDLK_F9;
	if ( !key.compare ( "F10" ) ) return SDLK_F10;
	if ( !key.compare ( "F11" ) ) return SDLK_F11;
	if ( !key.compare ( "F12" ) ) return SDLK_F12;
	if ( !key.compare ( "F13" ) ) return SDLK_F13;
	if ( !key.compare ( "F14" ) ) return SDLK_F14;
	if ( !key.compare ( "F15" ) ) return SDLK_F15;
	if ( !key.compare ( "NUMLOCK" ) ) return SDLK_NUMLOCK;
	if ( !key.compare ( "CAPSLOCK" ) ) return SDLK_CAPSLOCK;
	if ( !key.compare ( "SCROLLOCK" ) ) return SDLK_SCROLLOCK;
	if ( !key.compare ( "RSHIFT" ) ) return SDLK_RSHIFT;
	if ( !key.compare ( "LSHIFT" ) ) return SDLK_LSHIFT;
	if ( !key.compare ( "RCTRL" ) ) return SDLK_RCTRL;
	if ( !key.compare ( "LCTRL" ) ) return SDLK_LCTRL;
	if ( !key.compare ( "RALT" ) ) return SDLK_RALT;
	if ( !key.compare ( "LALT" ) ) return SDLK_LALT;
	if ( !key.compare ( "RMETA" ) ) return SDLK_RMETA;
	if ( !key.compare ( "LMETA" ) ) return SDLK_LMETA;
	if ( !key.compare ( "LSUPER" ) ) return SDLK_LSUPER;
	if ( !key.compare ( "RSUPER" ) ) return SDLK_RSUPER;
	if ( !key.compare ( "MODE" ) ) return SDLK_MODE;
	if ( !key.compare ( "COMPOSE" ) ) return SDLK_COMPOSE;
	if ( !key.compare ( "HELP" ) ) return SDLK_HELP;
	if ( !key.compare ( "PRINT" ) ) return SDLK_PRINT;
	if ( !key.compare ( "SYSREQ" ) ) return SDLK_SYSREQ;
	if ( !key.compare ( "BREAK" ) ) return SDLK_BREAK;
	if ( !key.compare ( "MENU" ) ) return SDLK_MENU;
	if ( !key.compare ( "POWER" ) ) return SDLK_POWER;
	if ( !key.compare ( "EURO" ) ) return SDLK_EURO;
	if ( !key.compare ( "UNDO" ) ) return SDLK_UNDO;
	return SDLK_UNKNOWN;
}

void GenerateKeysXml()
{
	//TODO: add generation of key xml
	Log.write("GenerateKeysXML not yet implemented", cLog::eLOG_TYPE_WARNING);
	return;
}
