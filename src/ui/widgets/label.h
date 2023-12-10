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

#ifndef ui_widgets_labelH
#define ui_widgets_labelH

#include "output/video/unifonts.h"
#include "ui/widgets/alignment.h"
#include "ui/widgets/clickablewidget.h"

#include <string>
#include <vector>

class cLabel : public cClickableWidget
{
public:
	cLabel (const cBox<cPosition>& area, const std::string& text, eUnicodeFontType = eUnicodeFontType::LatinNormal, AlignmentFlags = toEnumFlag (eAlignmentType::Left) | eAlignmentType::Top);

	void setText (const std::string& text);
	const std::string& getText() const;

	void setFont (eUnicodeFontType);
	void setAlignment (AlignmentFlags);
	void setWordWrap (bool wordWrap);

	void resizeToTextHeight();

	void draw (SDL_Surface& destination, const cBox<cPosition>& clipRect) override;
	void handleResized (const cPosition& oldSize) override;

	cSignal<void()> clicked;

protected:
	bool handleClicked (cApplication&, cMouse&, eMouseButtonType) override;

private:
	void updateDisplayInformation();

private:
	std::string text;
	eUnicodeFontType fontType;
	AlignmentFlags alignment;
	bool wordWrap;

	std::vector<std::string> drawLines;

	UniqueSurface surface;
};

#endif
