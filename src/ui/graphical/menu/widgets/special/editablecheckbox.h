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

#ifndef ui_graphical_menu_widgets_special_ediatblecheckboxH
#define ui_graphical_menu_widgets_special_ediatblecheckboxH

#include "ui/graphical/menu/widgets/checkbox.h"
#include "ui/graphical/menu/widgets/frame.h"
#include "ui/graphical/menu/widgets/label.h"
#include "ui/graphical/menu/widgets/lineedit.h"
#include "ui/graphical/menu/widgets/tools/validator.h"


class cEditableCheckBox : public cFrame
{
public:
	cEditableCheckBox (const cBox<cPosition>&, const std::string& prefix, const std::string& suffix, eUnicodeFontType fontType = FONT_LATIN_NORMAL);

	void setText (const std::string& text) { lineEdit->setText (text); }
	const std::string& getText() const { return lineEdit->getText(); }
	void setValidator (std::unique_ptr<cValidator> validator) { lineEdit->setValidator (std::move (validator)); }

	operator cCheckBox&() { return *checkBox; }

private:
	cSignalConnectionManager signalConnectionManager;
	cCheckBox* checkBox = nullptr;
	cLineEdit* lineEdit = nullptr;
	cLabel* suffixLabel = nullptr;
};

#endif
