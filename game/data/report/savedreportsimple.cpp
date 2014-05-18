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

#include "savedreportsimple.h"

#include "../../../netmessage.h"

//------------------------------------------------------------------------------
cSavedReportSimple::cSavedReportSimple (std::string text_) :
	text (std::move (text_))
{}


//------------------------------------------------------------------------------
cSavedReportSimple::cSavedReportSimple (cNetMessage& message)
{
	text = message.popString ();
}

//------------------------------------------------------------------------------
cSavedReportSimple::cSavedReportSimple (const tinyxml2::XMLElement& element)
{
	text = element.Attribute ("msg");
}

//------------------------------------------------------------------------------
void cSavedReportSimple::pushInto (cNetMessage& message) const
{
	message.pushString (text);

	cSavedReport::pushInto (message);
}

//------------------------------------------------------------------------------
void cSavedReportSimple::pushInto (tinyxml2::XMLElement& element) const
{
	element.SetAttribute ("msg", text.c_str ());

	cSavedReport::pushInto (element);
}

//------------------------------------------------------------------------------
eSavedReportType cSavedReportSimple::getType () const
{
	return eSavedReportType::Simple;
}

//------------------------------------------------------------------------------
std::string cSavedReportSimple::getMessage () const
{
	return text;
}

//------------------------------------------------------------------------------
bool cSavedReportSimple::isAlert () const
{
	return false;
}