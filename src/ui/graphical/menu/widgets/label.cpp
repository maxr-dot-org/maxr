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

#include "ui/graphical/menu/widgets/label.h"
#include "utility/drawing.h"

//------------------------------------------------------------------------------
cLabel::cLabel (const cBox<cPosition>& area, const std::string& text_, eUnicodeFontType fontType_, AlignmentFlags alignment_) :
	cWidget (area),
	fontType (fontType_),
	alignment (alignment_),
	wordWrap (false)
{
	surface = AutoSurface (SDL_CreateRGBSurface (0, getSize ().x (), getSize ().y (), 32, 0, 0, 0, 0));
	SDL_FillRect (surface.get (), nullptr, 0xFF00FF);
	SDL_SetColorKey (surface.get (), SDL_TRUE, 0xFF00FF);

	setText (text_);
}

//------------------------------------------------------------------------------
void cLabel::setText (const std::string& text_)
{
	text = text_;

	// NOTE: do we really want to do this here?
	//       we replace the character sequence "\n" by the escape sequence '\n'
	//       because e.g. when reading translation strings, this is used to
	//       indicate line breaks.
	//       May move this directly to @ref cLanguage and make it more robust
	//       (i.e. really parsing escape sequence, so that "\\n" will result in
	//       "\n" and not in "\" followed by '\n')
	size_t pos = 0;
	while ((pos = text.find ("\\n", pos)) != std::string::npos)
	{
		text.replace (pos, 2, "\n");
		pos += 1;
	}

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
	std::swap(fontType, fontType_);
	if(fontType != fontType_) updateDisplayInformation ();
}

//------------------------------------------------------------------------------
void cLabel::setAlignment (AlignmentFlags alignment_)
{
	std::swap (alignment, alignment_);
	if (alignment != alignment_) updateDisplayInformation ();
}

//------------------------------------------------------------------------------
void cLabel::setWordWrap (bool wordWrap_)
{
	std::swap (wordWrap, wordWrap_);
	if (wordWrap != wordWrap_) updateDisplayInformation ();
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

		if (it == text.end() || font->isUtf8Space (&(*it)))
		{
			if (currentLineLength + currentWordLength >= maximalWidth || (it != text.end () && *it == '\n'))
			{
				if (currentLineLength + currentWordLength >= maximalWidth)
				{
					// Remove all leading white spaces
					while (nextWordBegin != it && font->isUtf8Space (&(*nextWordBegin)))
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
					while (nextWordBegin != it && font->isUtf8Space (&(*nextWordBegin)))
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

	SDL_FillRect (surface.get (), nullptr, 0xFF00FF);

	const auto height = font->getFontHeight (fontType) * drawLines.size ();

	int drawPositionY;
	if (alignment & eAlignmentType::Bottom)
	{
		drawPositionY = getSize ().y () - height;
	}
	else if (alignment & eAlignmentType::CenterVerical)
	{
		drawPositionY = getSize ().y () / 2 - height / 2;
	}
	else
	{
		drawPositionY = 0;
	}

	auto originalTargetSurface = font->getTargetSurface ();
	auto fontTargetSurfaceResetter = makeScopedOperation ([originalTargetSurface](){ font->setTargetSurface (originalTargetSurface); });
	font->setTargetSurface (surface.get ());
	for (size_t i = 0; i < drawLines.size (); ++i)
	{
		const auto& line = drawLines[i];

		const auto width = font->getTextWide (line, fontType);

		int drawPositionX;
		if (alignment & eAlignmentType::Right)
		{
			drawPositionX = getSize ().x () - width;
		}
		else if (alignment & eAlignmentType::CenterHorizontal)
		{
			drawPositionX = getSize ().x () / 2 - width / 2;
		}
		else
		{
			drawPositionX = 0;
		}

		font->showText (drawPositionX, drawPositionY, line, fontType);

		drawPositionY += font->getFontHeight (fontType);
	}
}

//------------------------------------------------------------------------------
void cLabel::draw (SDL_Surface& destination, const cBox<cPosition>& clipRect)
{
	if (surface && getArea ().intersects (clipRect))
	{
		blitClipped (*surface, getArea(), destination, clipRect);
	}

	cWidget::draw (destination, clipRect);
}

//------------------------------------------------------------------------------
void cLabel::handleResized (const cPosition& oldSize)
{
	cWidget::handleResized (oldSize);

	surface = AutoSurface (SDL_CreateRGBSurface (0, getSize ().x (), getSize ().y (), 32, 0, 0, 0, 0));
	SDL_FillRect (surface.get (), nullptr, 0xFF00FF);
	SDL_SetColorKey (surface.get (), SDL_TRUE, 0xFF00FF);

	updateDisplayInformation ();
}
