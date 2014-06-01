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

#include "savedreportunit.h"

#include "../../../netmessage.h"
#include "../../../unit.h"

//------------------------------------------------------------------------------
cSavedReportUnit::cSavedReportUnit (const cUnit& unit, std::string text_) :
	unitId (unit.data.ID),
	position (unit.getPosition()),
	text (std::move (text_))
{}


//------------------------------------------------------------------------------
cSavedReportUnit::cSavedReportUnit (cNetMessage& message)
{
	text = message.popString ();
	position = message.popPosition ();
	unitId = message.popID ();
}

//------------------------------------------------------------------------------
cSavedReportUnit::cSavedReportUnit (const tinyxml2::XMLElement& element)
{
	text = element.Attribute("msg");
	position.x () = element.IntAttribute ("xPos");
	position.y () = element.IntAttribute ("yPos");
	unitId.generate (element.Attribute ("id"));
}

//------------------------------------------------------------------------------
void cSavedReportUnit::pushInto (cNetMessage& message) const
{
	message.pushID (unitId);
	message.pushPosition (position);
	message.pushString (text);

	cSavedReport::pushInto (message);
}

//------------------------------------------------------------------------------
void cSavedReportUnit::pushInto (tinyxml2::XMLElement& element) const
{
	element.SetAttribute ("msg", text.c_str ());
	element.SetAttribute ("xPos", iToStr (position.x()).c_str ());
	element.SetAttribute ("yPos", iToStr (position.y()).c_str ());
	element.SetAttribute ("id", unitId.getText ().c_str ());

	cSavedReport::pushInto (element);
}

//------------------------------------------------------------------------------
const cPosition& cSavedReportUnit::getPosition () const
{
	return position;
}

//------------------------------------------------------------------------------
const sID& cSavedReportUnit::getUnitId () const
{
	return unitId;
}

//------------------------------------------------------------------------------
eSavedReportType cSavedReportUnit::getType () const
{
	return eSavedReportType::Unit;
}

//------------------------------------------------------------------------------
std::string cSavedReportUnit::getMessage () const
{
	return "[" + iToStr (position.x ()) + ", " + iToStr (position.y ()) + "] " + text;
}

//------------------------------------------------------------------------------
bool cSavedReportUnit::isAlert () const
{
	return false;
}