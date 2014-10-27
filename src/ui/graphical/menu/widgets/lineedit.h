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

#ifndef ui_graphical_menu_widgets_lineeditH
#define ui_graphical_menu_widgets_lineeditH

#include <string>
#include <memory>
#include <chrono>

#include "maxrconfig.h"
#include "ui/graphical/menu/widgets/clickablewidget.h"
#include "ui/graphical/menu/widgets/tools/validatorstate.h"
#include "unifonts.h"
#include "utility/autosurface.h"
#include "utility/signal/signal.h"

class cValidator;

enum class eLineEditFrameType
{
	None,
	Box
};

class cLineEdit : public cClickableWidget
{
public:
	cLineEdit (const cBox<cPosition>& area, eLineEditFrameType frameType = eLineEditFrameType::None, eUnicodeFontType fontType = FONT_LATIN_NORMAL);
	~cLineEdit ();

	const std::string& getText ();
	void setText (std::string text);

	void setReadOnly (bool readOnly);
	void setValidator (std::unique_ptr<cValidator> validator);

	void finishEditing ();

	virtual void draw (SDL_Surface& destination, const cBox<cPosition>& clipRect) MAXR_OVERRIDE_FUNCTION;;

	virtual bool handleGetKeyFocus (cApplication& application) MAXR_OVERRIDE_FUNCTION;
	virtual void handleLooseKeyFocus (cApplication& application) MAXR_OVERRIDE_FUNCTION;

	virtual bool handleKeyPressed (cApplication& application, cKeyboard& keyboard, SDL_Keycode key) MAXR_OVERRIDE_FUNCTION;
	virtual bool handleTextEntered (cApplication& application, cKeyboard& keyboard, const char* text) MAXR_OVERRIDE_FUNCTION;

	cSignal<void ()> textSet;
	cSignal<void ()> escapePressed;
	cSignal<void ()> returnPressed;
	cSignal<void (eValidatorState)> editingFinished;
protected:

	virtual bool handleClicked (cApplication& application, cMouse& mouse, eMouseButtonType button) MAXR_OVERRIDE_FUNCTION;
private:
	const std::chrono::milliseconds cursorVisibleTime;
	const std::chrono::milliseconds cursorInvisibleTime;

	AutoSurface surface;

	std::string text;
	eUnicodeFontType fontType;

	eLineEditFrameType frameType;

	int cursorPos;
	int startOffset, endOffset;

	bool readOnly;
	std::unique_ptr<cValidator> validator;

	bool hasKeyFocus;

	bool showCursor;
	std::chrono::steady_clock::time_point lastCursorBlinkTime;

	void createBackground ();

	cPosition getTextDrawOffset () const;
	int getBorderSize () const;

	void resetTextPosition ();
	void doPosIncrease (int& value, int pos);
	void doPosDecrease (int& pos);
	void scrollLeft (bool changeCursor = true);
	void scrollRight ();
	void deleteLeft ();
	void deleteRight ();

	void finishEditingInternal ();
};

#endif // ui_graphical_menu_widgets_lineeditH
