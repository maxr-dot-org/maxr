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

#ifndef ui_graphical_menu_windows_windowplayerselection_windowplayerselectionH
#define ui_graphical_menu_windows_windowplayerselection_windowplayerselectionH

#include "SDLutility/uniquesurface.h"
#include "ui/widgets/window.h"
#include "utility/signal/signal.h"
#include "utility/signal/signalconnectionmanager.h"

#include <array>

class cImage;
class cLabel;
class cPushButton;

enum class ePlayerType
{
	Human,
	Ai,
	None
};

class cWindowPlayerSelection : public cWindow
{
	const static size_t maxPlayers = 4;

public:
	cWindowPlayerSelection();

	void retranslate() override;

	const std::array<ePlayerType, maxPlayers>& getPlayerTypes() const;

	cSignal<void()> done;

private:
	cSignalConnectionManager signalConnectionManager;

	cLabel* titleLabel = nullptr;
	cLabel* teamLabel = nullptr;
	cLabel* humanLabel = nullptr;
	cLabel* computerLabel = nullptr;
	cLabel* nobodyLabel = nullptr;

	cPushButton* okButton = nullptr;
	cPushButton* backButton = nullptr;

	std::array<ePlayerType, maxPlayers> playerTypes;

	std::array<cImage*, maxPlayers> humanPlayerImages;
	std::array<cImage*, maxPlayers> aiPlayerImages;
	std::array<cImage*, maxPlayers> noPlayerImages;

	UniqueSurface humanPlayerSurface;
	UniqueSurface aiPlayerSurface;
	UniqueSurface noPlayerSurface;
	UniqueSurface dummySurface;

	void setPlayerType (size_t playerIndex, ePlayerType playerType);
};

#endif // ui_graphical_menu_windows_windowplayerselection_windowplayerselectionH
