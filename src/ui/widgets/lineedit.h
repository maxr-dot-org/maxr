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

#ifndef ui_widgets_lineeditH
#define ui_widgets_lineeditH

#include "SDLutility/uniquesurface.h"
#include "output/video/unifonts.h"
#include "ui/widgets/clickablewidget.h"
#include "ui/widgets/validators/validatorstate.h"
#include "utility/signal/signal.h"

#include <chrono>
#include <memory>
#include <string>

class cValidator;

enum class eLineEditFrameType
{
	None,
	Box
};

class cLineEdit : public cClickableWidget
{
public:
	cLineEdit (const cBox<cPosition>& area, eLineEditFrameType = eLineEditFrameType::None, eUnicodeFontType = eUnicodeFontType::LatinNormal);
	~cLineEdit();

	const std::string& getText();
	void setText (std::string text);

	void setReadOnly (bool readOnly);
	void setValidator (std::unique_ptr<cValidator>);

	void finishEditing();

	void draw (SDL_Surface& destination, const cBox<cPosition>& clipRect) override;
	bool handleGetKeyFocus (cApplication&) override;
	void handleLooseKeyFocus (cApplication&) override;
	bool handleKeyPressed (cApplication&, cKeyboard&, SDL_Keycode) override;
	void handleTextEntered (cApplication&, cKeyboard&, const char* text) override;

	cSignal<void()> clicked;
	cSignal<void()> textSet;
	cSignal<void()> escapePressed;
	cSignal<void()> returnPressed;
	cSignal<void (eValidatorState)> editingFinished;

protected:
	bool handleClicked (cApplication&, cMouse&, eMouseButtonType) override;

private:
	void createBackground();

	cPosition getTextDrawOffset() const;
	int getBorderSize() const;

	void resetTextPosition();
	void doPosIncrease (int& value, int pos);
	void doPosDecrease (int& pos);
	void scrollLeft (bool changeCursor = true);
	void scrollRight();
	void deleteLeft();
	void deleteRight();

	void finishEditingInternal();

private:
	const std::chrono::milliseconds cursorVisibleTime;
	const std::chrono::milliseconds cursorInvisibleTime;

	UniqueSurface surface;

	std::string text;
	eUnicodeFontType fontType;

	eLineEditFrameType frameType;

	int cursorPos = 0;
	int startOffset = 0;
	int endOffset = 0;

	bool readOnly = false;
	std::unique_ptr<cValidator> validator;

	bool hasKeyFocus = false;

	bool showCursor = false;
	std::chrono::steady_clock::time_point lastCursorBlinkTime;
};

#endif
