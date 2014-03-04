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
{
	updateDisplayInformation ();
}

//------------------------------------------------------------------------------
void cLabel::setText (const std::string& text_)
{
	text = text_;
	updateDisplayInformation ();
}

//------------------------------------------------------------------------------
const std::string& cLabel::getText () const
{
	return text;
}

//------------------------------------------------------------------------------
void cLabel::setFont (eUnicodeFontType fontType_)
{
	fontType = fontType_;
	updateDisplayInformation ();
}

//------------------------------------------------------------------------------
void cLabel::setAlignment (AlignmentFlags alignment_)
{
	alignment = alignment_;
	updateDisplayInformation ();
}

//------------------------------------------------------------------------------
void cLabel::setWordWrap (bool wordWrap_)
{
	wordWrap = wordWrap_;
	updateDisplayInformation ();
}

//------------------------------------------------------------------------------
void cLabel::resizeToTextHeight ()
{
	const auto textHeight = drawLines.size () * font->getFontHeight (fontType);
	resize (cPosition (getSize ().x (), textHeight));
}

//------------------------------------------------------------------------------
void cLabel::breakText (const std::string& text, std::vector<std::string>& lines, int maximalWidth, eUnicodeFontType fontType) const
{
	// NOTE: better would be not to copy each line into the vector
	//       but use something like "string_view". We could simulate this by using
	//       a pair of iterators (like a range) but non of other methods would support such a
	//       rand and therefore the construction of a new string object would be necessary anyway.

	int currentLineLength = 0;
	int currentWordLength = 0;

	lines.push_back ("");

	auto it = text.begin ();
	auto nextWordBegin = it;
	while (true)
	{
		auto& currentLine = lines.back ();

		if (it == text.end() || std::isspace (*it))
		{
			if (currentLineLength + currentWordLength >= maximalWidth || (it != text.end () && *it == '\n'))
			{
				if (currentLineLength + currentWordLength >= maximalWidth)
				{
					// Remove all leading white spaces
					while (nextWordBegin != it && std::isspace (*nextWordBegin))
					{
						int increase;
						auto unicodeCharacter = font->encodeUTF8Char (&(*nextWordBegin), increase);
						currentWordLength -= font->getUnicodeCharacterWidth (unicodeCharacter, fontType);
						nextWordBegin += increase;
					}

					// TODO: may break the word when the single word is to long for the line
					// put the word into the next line
					lines.push_back (std::string (nextWordBegin, it));
					currentLineLength = currentWordLength;
				}
				else
				{
					currentLine.append (nextWordBegin, it);
					lines.push_back ("");
					currentLineLength = 0;
				}
			}
			else
			{
				if (currentLine.empty ())
				{
					// Remove all leading white spaces if we are at the beginning of a new line
					while (nextWordBegin != it && std::isspace (*nextWordBegin))
					{
						int increase;
						auto unicodeCharacter = font->encodeUTF8Char (&(*nextWordBegin), increase);
						currentWordLength -= font->getUnicodeCharacterWidth (unicodeCharacter, fontType);
						nextWordBegin += increase;
					}
				}
				currentLine.append (nextWordBegin, it);
				currentLineLength += currentWordLength;
			}

			if (it == text.end ()) break;

			nextWordBegin = it;
			currentWordLength = 0;
		}

		int increase;
		auto unicodeCharacter = font->encodeUTF8Char (&(*it), increase);
		currentWordLength += font->getUnicodeCharacterWidth (unicodeCharacter, fontType);

		it += increase;
	}
}

//------------------------------------------------------------------------------
void cLabel::updateDisplayInformation ()
{
	drawLines.clear ();

	if (wordWrap)
	{
		breakText (text, drawLines, getSize ().x (), fontType);
	}
	else
	{
		drawLines.push_back (text);
	}
}

//------------------------------------------------------------------------------
void cLabel::draw ()
{
	const auto height = font->getFontHeight (fontType) * drawLines.size ();

	int drawPositionY;
	if (alignment & eAlignmentType::Bottom)
	{
		drawPositionY = getEndPosition ().y () - height;
	}
	else if (alignment & eAlignmentType::CenterVerical)
	{
		drawPositionY = getPosition ().y () + getSize ().y () / 2 - height / 2;
	}
	else
	{
		drawPositionY = getPosition ().y ();
	}

	for (size_t i = 0; i < drawLines.size (); ++i)
	{
		const auto& line = drawLines[i];

		const auto width = font->getTextWide (line, fontType);

		int drawPositionX;
		if (alignment & eAlignmentType::Right)
		{
			drawPositionX = getEndPosition ().x () - width;
		}
		else if (alignment & eAlignmentType::CenterHorizontal)
		{
			drawPositionX = getPosition ().x () + getSize ().x () / 2 - width / 2;
		}
		else
		{
			drawPositionX = getPosition ().x ();
		}

		font->showText (drawPositionX, drawPositionY, line, fontType);

		drawPositionY += font->getFontHeight (fontType);
	}

	cWidget::draw ();
}
