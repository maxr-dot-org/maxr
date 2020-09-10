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

#ifndef ui_graphical_menu_widgets_labelH
#define ui_graphical_menu_widgets_labelH

#include <string>
#include <vector>

#include "maxrconfig.h"
#include "ui/graphical/menu/widgets/clickablewidget.h"
#include "ui/graphical/alignment.h"
#include "utility/unifonts.h"

class cLabel : public cClickableWidget
{
public:
	cLabel (const cBox<cPosition>& area, const std::string& text, eUnicodeFontType fontType_ = FONT_LATIN_NORMAL, AlignmentFlags alignment = toEnumFlag (eAlignmentType::Left)  | eAlignmentType::Top);

	void setText (const std::string& text);
	const std::string& getText() const;

	void setFont (eUnicodeFontType fontType);
	void setAlignment (AlignmentFlags alignment);
	void setWordWrap (bool wordWrap);

	void resizeToTextHeight();

	void draw (SDL_Surface& destination, const cBox<cPosition>& clipRect) MAXR_OVERRIDE_FUNCTION;
	void handleResized (const cPosition& oldSize) MAXR_OVERRIDE_FUNCTION;

	cSignal<void ()> clicked;
protected:

	bool handleClicked (cApplication& application, cMouse& mouse, eMouseButtonType button) MAXR_OVERRIDE_FUNCTION;
private:
	std::string text;
	eUnicodeFontType fontType;
	AlignmentFlags alignment;
	bool wordWrap;

	std::vector<std::string> drawLines;

	AutoSurface surface;

	void updateDisplayInformation();

	// TODO: may move to some other place
	void breakText (const std::string& text, std::vector<std::string>& lines, int maximalWidth, eUnicodeFontType fontType) const;
};

#endif // ui_graphical_menu_widgets_labelH
