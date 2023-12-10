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

#ifndef ui_graphical_game_widgets_gamemessagelistviewitemH
#define ui_graphical_game_widgets_gamemessagelistviewitemH

#include "SDLutility/autosurface.h"
#include "ui/graphical/menu/widgets/abstractlistviewitem.h"

#include <chrono>

class cLabel;

enum class eGameMessageListViewItemBackgroundColor
{
	DarkGray,
	LightGray,
	Red
};

class cGameMessageListViewItem : public cAbstractListViewItem
{
public:
	cGameMessageListViewItem (const std::string& message, eGameMessageListViewItemBackgroundColor backgroundColor);

	std::chrono::steady_clock::time_point getCreationTime() const;

	void draw (SDL_Surface& destination, const cBox<cPosition>& clipRect) override;
	void handleResized (const cPosition& oldSize) override;

private:
	cLabel* messageLabel = nullptr;

	eGameMessageListViewItemBackgroundColor backgroundColor;
	UniqueSurface background;

	const cPosition beginMargin;
	const cPosition endMargin;

	std::chrono::steady_clock::time_point creationTime;

	void createBackground();
};

#endif // ui_graphical_game_widgets_gamemessagelistviewitemH
