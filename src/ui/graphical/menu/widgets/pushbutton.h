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

#ifndef ui_graphical_menu_widgets_pushbuttonH
#define ui_graphical_menu_widgets_pushbuttonH

#include "SDLutility/autosurface.h"
#include "output/video/unifonts.h"
#include "resources/sound.h"
#include "ui/widgets/clickablewidget.h"
#include "utility/signal/signal.h"

#include <string>

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

	Invisible,

	ArrowUpSmallModern,
	ArrowDownSmallModern,
	ArrowLeftSmallModern,
	ArrowRightSmallModern,
};

class cPushButton : public cClickableWidget
{
public:
	explicit cPushButton (const cBox<cPosition>& area);
	cPushButton (const cPosition&, ePushButtonType);
	cPushButton (const cPosition&, ePushButtonType, cSoundChunk* clickSound);
	cPushButton (const cPosition&, ePushButtonType, const std::string& text, eUnicodeFontType = eUnicodeFontType::LatinBig);
	cPushButton (const cPosition&, ePushButtonType, cSoundChunk* clickSound, const std::string& text, eUnicodeFontType = eUnicodeFontType::LatinBig);

	void draw (SDL_Surface& destination, const cBox<cPosition>& clipRect) override;
	bool handleMousePressed (cApplication&, cMouse&, eMouseButtonType) override;
	bool handleMouseReleased (cApplication&, cMouse&, eMouseButtonType) override;

	void setText (const std::string&);

	void lock();
	void unlock();

	static cPosition getButtonSize (ePushButtonType);

	cSignal<void()> clicked;

protected:
	void setPressed (bool pressed) override;
	bool handleClicked (cApplication&, cMouse&, eMouseButtonType) override;

private:
	UniqueSurface surface;
	ePushButtonType buttonType;

	eUnicodeFontType fontType;
	std::string text;

	cSoundChunk* clickSound;

	bool isLocked;

	void renewSurface();

	int getBordersSize() const;
	int getTextYOffset() const;
};

#endif // ui_graphical_menu_widgets_pushbuttonH
