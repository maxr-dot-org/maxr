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

#ifndef gui_menu_widgets_pushbuttonH
#define gui_menu_widgets_pushbuttonH

#include <string>

#include "../../../maxrconfig.h"
#include "clickablewidget.h"
#include "../../../autosurface.h"
#include "../../../utility/signal/signal.h"
#include "../../../unifonts.h"
#include "../../../sound.h"

enum class ePushButtonType
{
	StandardBig,
	StandardSmall,
	Huge,
	ArrowUpBig,
	ArrowDownBig,
	ArrowLeftBig,
	ArrowRightBig,
	ArrowUpSmall,
	ArrowDownSmall,
	ArrowLeftSmall,
	ArrowRightSmall,
	ArrowUpBar,
	ArrowDownBar,
	Angular,

	HudHelp,
	HudCenter,
	HudNext,
	HudPrev,
	HudDone,
	HudReport,
	HudChat,

	HudPreferences,
	HudFiles,
	HudEnd,
	HudPlay,
	HudStop,

	UnitContextMenu,

	Destroy,

	Invisible
};

class cPushButton : public cClickableWidget
{
public:
	explicit cPushButton (const cBox<cPosition>& area);
	cPushButton (const cPosition& position, ePushButtonType buttonType);
	cPushButton (const cPosition& position, ePushButtonType buttonType, cSoundChunk* clickSound);
	cPushButton (const cPosition& position, ePushButtonType buttonType, const std::string& text, eUnicodeFontType fontType = FONT_LATIN_BIG);
	cPushButton (const cPosition& position, ePushButtonType buttonType, cSoundChunk* clickSound, const std::string& text, eUnicodeFontType fontType = FONT_LATIN_BIG);

	virtual void draw () MAXR_OVERRIDE_FUNCTION;

	virtual bool handleMousePressed (cApplication& application, cMouse& mouse, eMouseButtonType button) MAXR_OVERRIDE_FUNCTION;
	virtual bool handleMouseReleased (cApplication& application, cMouse& mouse, eMouseButtonType button) MAXR_OVERRIDE_FUNCTION;

	void lock ();
	void unlock ();

	cSignal<void ()> clicked;
protected:
	virtual void setPressed (bool pressed) MAXR_OVERRIDE_FUNCTION;

	virtual bool handleClicked (cApplication& application, cMouse& mouse, eMouseButtonType button) MAXR_OVERRIDE_FUNCTION;

private:
	AutoSurface surface;
	ePushButtonType buttonType;

	eUnicodeFontType fontType;
	std::string text;

	cSoundChunk* clickSound;

	bool isLocked;

	void renewSurface ();

	int getBordersSize () const;
	int getTextYOffset () const;
};

#endif // gui_menu_widgets_pushbuttonH
