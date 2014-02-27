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

#ifndef gui_game_gameguiH
#define gui_game_gameguiH

#include "../window.h"
#include "../../utility/signal/signalconnectionmanager.h"

class cHud;
class cGameMapWidget;
class cMiniMapWidget;
class cStaticMap;
class cMap;
class cPlayer;

class cNewGameGUI : public cWindow
{
public:
	cNewGameGUI (std::shared_ptr<const cStaticMap> staticMap);

	void setDynamicMap (const cMap* dynamicMap);
	void setPlayer (const cPlayer* player);

	virtual bool handleMouseMoved (cApplication& application, cMouse& mouse) MAXR_OVERRIDE_FUNCTION;
	virtual bool handleMouseWheelMoved (cApplication& application, cMouse& mouse, const cPosition& amount) MAXR_OVERRIDE_FUNCTION;

	virtual bool handleKeyPressed (cApplication& application, cKeyboard& keyboard, SDL_Keycode key) MAXR_OVERRIDE_FUNCTION;

	virtual void handleLooseMouseFocus (cApplication& application) MAXR_OVERRIDE_FUNCTION;
protected:

private:
	cSignalConnectionManager signalConnectionManager;

	std::shared_ptr<const cStaticMap> staticMap;
	const cMap* dynamicMap;
	const cPlayer* player;

	cHud* hud;
	cGameMapWidget* gameMap;
	cMiniMapWidget* miniMap;

	void resetMiniMapViewWindow ();

	void showFilesMenu ();
	void showPreferencesDialog ();

	void updateHudCoordinates (const cPosition& tilePosition);
	void updateHudUnitName (const cPosition& tilePosition);
	void handleTileClicked (const cPosition& tilePosition);
};

#endif // gui_game_gameguiH
