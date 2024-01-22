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

#include "SDLutility/drawing.h"

//------------------------------------------------------------------------------
cLabel::cLabel (const cBox<cPosition>& area, const std::string& text_, eUnicodeFontType fontType_, AlignmentFlags alignment_) :
	cClickableWidget (area),
	fontType (fontType_),
	alignment (alignment_)
{
	if (getSize().x() < 0 || getSize().y() < 0)
	{
		surface = nullptr;
	}
	else
	{
		surface = UniqueSurface (SDL_CreateRGBSurface (0, getSize().x(), getSize().y(), 32, 0, 0, 0, 0));
		SDL_FillRect (surface.get(), nullptr, 0xFF00FF);
		SDL_SetColorKey (surface.get(), SDL_TRUE, 0xFF00FF);
	}

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

	dirty = true;
}

//------------------------------------------------------------------------------
const std::string& cLabel::getText() const
{
	return text;
}

//------------------------------------------------------------------------------
void cLabel::setFont (eUnicodeFontType fontType_)
{
	std::swap (fontType, fontType_);
	if (fontType != fontType_) { dirty = true; }
}

//------------------------------------------------------------------------------
void cLabel::setAlignment (AlignmentFlags alignment_)
{
	std::swap (alignment, alignment_);
	if (alignment != alignment_) { dirty = true; }
}

//------------------------------------------------------------------------------
void cLabel::setWordWrap (bool wordWrap_)
{
	std::swap (wordWrap, wordWrap_);
	if (wordWrap != wordWrap_) { dirty = true; }
}

//------------------------------------------------------------------------------
void cLabel::resizeToTextHeight()
{
	if (dirty == true)
	{
		auto font = cUnicodeFont::font.get();
		drawLines = wordWrap ? font->breakText (text, getSize().x(), fontType) : std::vector{text};
	}
	const auto textHeight = drawLines.size() * cUnicodeFont::font->getFontHeight (fontType);
	resize (cPosition (getSize().x(), textHeight));
}

//------------------------------------------------------------------------------
void cLabel::updateDisplayInformation()
{
	if (surface == nullptr) return;

	auto font = cUnicodeFont::font.get();
	drawLines = wordWrap ? font->breakText (text, getSize().x(), fontType): std::vector{text};

	SDL_FillRect (surface.get(), nullptr, 0xFF00FF);

	const auto height = cUnicodeFont::font->getFontHeight (fontType) * drawLines.size();

	int drawPositionY;
	if (alignment & eAlignmentType::Bottom)
	{
		drawPositionY = getSize().y() - height;
	}
	else if (alignment & eAlignmentType::CenterVerical)
	{
		drawPositionY = getSize().y() / 2 - height / 2;
	}
	else
	{
		drawPositionY = 0;
	}

	auto originalTargetSurface = font->getTargetSurface();
	auto fontTargetSurfaceResetter = makeScopedOperation ([originalTargetSurface, font]() { font->setTargetSurface (originalTargetSurface); });
	font->setTargetSurface (surface.get());
	for (const auto& line : drawLines)
	{
		const auto width = font->getTextWide (line, fontType);

		int drawPositionX;
		if (alignment & eAlignmentType::Right)
		{
			drawPositionX = getSize().x() - width;
		}
		else if (alignment & eAlignmentType::CenterHorizontal)
		{
			drawPositionX = getSize().x() / 2 - width / 2;
		}
		else
		{
			drawPositionX = 0;
		}

		font->showText (drawPositionX, drawPositionY, line, fontType);

		drawPositionY += font->getFontHeight (fontType);
	}
	dirty = false;
}

//------------------------------------------------------------------------------
void cLabel::draw (SDL_Surface& destination, const cBox<cPosition>& clipRect)
{
	if (surface && getArea().intersects (clipRect))
	{
		if (dirty == true) { updateDisplayInformation(); }
		blitClipped (*surface, getArea(), destination, clipRect);
	}

	cWidget::draw (destination, clipRect);
}

//------------------------------------------------------------------------------
void cLabel::handleResized (const cPosition& oldSize)
{
	dirty = true;
	cWidget::handleResized (oldSize);

	if (getSize().x() < 0 || getSize().y() < 0)
	{
		surface = nullptr;
		return;
	}

	surface = UniqueSurface (SDL_CreateRGBSurface (0, getSize().x(), getSize().y(), 32, 0, 0, 0, 0));
	SDL_FillRect (surface.get(), nullptr, 0xFF00FF);
	SDL_SetColorKey (surface.get(), SDL_TRUE, 0xFF00FF);
}

//------------------------------------------------------------------------------
bool cLabel::handleClicked (cApplication& application, cMouse& mouse, eMouseButtonType button)
{
	clicked();

	return true;
}
