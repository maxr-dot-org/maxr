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

#include "savedreporttranslated.h"

#include "../../../netmessage.h"
#include "../../../language.h"
#include "../../../main.h" // lngPack

//------------------------------------------------------------------------------
cSavedReportTranslated::cSavedReportTranslated (std::string translationText_, bool isAlert) :
	translationText (std::move (translationText_)),
	alert (isAlert)
{}

//------------------------------------------------------------------------------
cSavedReportTranslated::cSavedReportTranslated (std::string translationText_, std::string insertText_, bool isAlert) :
	translationText (std::move (translationText_)),
	insertText (std::move (insertText_)),
	alert (isAlert)
{}

//------------------------------------------------------------------------------
cSavedReportTranslated::cSavedReportTranslated (cNetMessage& message)
{
	translationText = message.popString ();
	insertText = message.popString ();
	alert = message.popBool ();
}

//------------------------------------------------------------------------------
cSavedReportTranslated::cSavedReportTranslated (const tinyxml2::XMLElement& element)
{
	translationText = element.Attribute ("msg");
	insertText = element.Attribute ("insert");
	alert = element.BoolAttribute ("alert");
}

//------------------------------------------------------------------------------
void cSavedReportTranslated::pushInto (cNetMessage& message) const
{
	message.pushBool (alert);
	message.pushString (insertText);
	message.pushString (translationText);

	cSavedReport::pushInto (message);
}

//------------------------------------------------------------------------------
void cSavedReportTranslated::pushInto (tinyxml2::XMLElement& element) const
{
	element.SetAttribute ("msg", translationText.c_str ());
	element.SetAttribute ("insert", insertText.c_str ());
	element.SetAttribute ("alert", bToStr(alert).c_str ());

	cSavedReport::pushInto (element);
}

//------------------------------------------------------------------------------
eSavedReportType cSavedReportTranslated::getType () const
{
	return eSavedReportType::Translated;
}

//------------------------------------------------------------------------------
std::string cSavedReportTranslated::getMessage () const
{
	if (insertText.empty ()) return lngPack.i18n (translationText);
	else return lngPack.i18n (translationText, insertText);
}

//------------------------------------------------------------------------------
bool cSavedReportTranslated::isAlert () const
{
	return alert;
}