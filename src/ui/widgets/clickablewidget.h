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

#ifndef ui_widgets_clickablewidgetH
#define ui_widgets_clickablewidgetH

#include "ui/widgets/widget.h"
#include "utility/signal/signalconnectionmanager.h"

#include <map>

class cKeySequence;
class cShortcut;

class cClickableWidget : public cWidget
{
public:
	cClickableWidget() = default;
	explicit cClickableWidget (const cPosition&);
	explicit cClickableWidget (const cBox<cPosition>& area);

	cShortcut& addClickShortcut (cKeySequence, eMouseButtonType = eMouseButtonType::Left);

	bool handleMouseMoved (cApplication&, cMouse&, const cPosition& offset) override;
	bool handleMousePressed (cApplication&, cMouse&, eMouseButtonType) override;
	bool handleMouseReleased (cApplication&, cMouse&, eMouseButtonType) override;
	void handleLooseMouseFocus (cApplication&) override;

	void setConsumeClick (bool consumeClick);

protected:
	virtual void setPressed (bool pressed);

	void finishMousePressed (cApplication&, cMouse&, eMouseButtonType);

	virtual bool handleClicked (cApplication&, cMouse&, eMouseButtonType) = 0;

	virtual bool acceptButton (eMouseButtonType) const;

private:
	bool& getStartedClickWithin (eMouseButtonType);

protected:
	bool isPressed = false;
	bool mouseWasOver = false;

	bool consumeClick = true;

private:
	cSignalConnectionManager signalConnectionManager;

	std::map<eMouseButtonType, bool> startedClickWithin;
};

#endif
