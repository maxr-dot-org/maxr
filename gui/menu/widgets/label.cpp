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

#include "label.h"

//------------------------------------------------------------------------------
cLabel::cLabel (const cBox<cPosition>& area, const std::string& text_, eUnicodeFontType fontType_, AlignmentFlags alignment_) :
	cWidget (area),
	text (text_),
	fontType (fontType_),
	alignment (alignment_),
	wordWrap (false)
{}

//------------------------------------------------------------------------------
void cLabel::setText (const std::string& text_)
{
	text = text_;
}

//------------------------------------------------------------------------------
void cLabel::setFont (eUnicodeFontType fontType_)
{
	fontType = fontType_;
}

//------------------------------------------------------------------------------
void cLabel::setAlignment (AlignmentFlags alignment_)
{
	alignment = alignment_;
}

//------------------------------------------------------------------------------
void cLabel::setWordWrap (bool wordWrap_)
{
	wordWrap = wordWrap_;
}

//------------------------------------------------------------------------------
void cLabel::draw ()
{
	// TODO: support all combinations of alignment and word wrap
	if (alignment & eAlignmentType::CenterHorizontal)
	{
		auto x = getArea ().getMinCorner ().x () + (getArea ().getMaxCorner ().x () - getArea ().getMinCorner ().x ()) / 2;
		auto y = getArea ().getMinCorner ().y ();
		font->showTextCentered (x, y, text, fontType);
	}
	else if (wordWrap) font->showTextAsBlock (getArea ().toSdlRect (), text, fontType);
	else font->showText (getArea ().toSdlRect (), text, fontType);

	cWidget::draw ();
}
