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

#ifndef ui_graphical_game_widgets_debugoutputidgetH
#define ui_graphical_game_widgets_debugoutputidgetH

#include "ui/graphical/widget.h"
#include "unifonts.h"

class cClient;
class cServer2;
class cPosition;
class cVehicle;
class cBuilding;
class cGameMapWidget;

/**
 * This class draws all the debug output on the screen.
 * It is a separate class,
 * so you can add a "friend class cDebugOutputWidget;" to the class,
 * which contains the data to display.
 * So there is no need to make members public
 * only to use them in the debug output.
 *@author eiko
 */
class cDebugOutputWidget : public cWidget
{
public:
	cDebugOutputWidget (const cBox<cPosition>& area);

	void setClient (const cClient* client);
	void setServer (const cServer2* server);
	void setGameMap (const cGameMapWidget* gameMap);

	void setDebugAjobs (bool value);
	void setDebugBaseServer (bool value);
	void setDebugBaseClient (bool value);
	void setDebugSentry (bool value);
	void setDebugFX (bool value);
	void setDebugTraceServer (bool value);
	void setDebugTraceClient (bool value);
	void setDebugPlayers (bool value);
	void setDebugCache (bool value);
	void setDebugSync (bool value);

	virtual void draw (SDL_Surface& destination, const cBox<cPosition>& clipRect) MAXR_OVERRIDE_FUNCTION;

private:
	const cServer2* server;
	const cClient* client;
	const cGameMapWidget* gameMap;

	/** show infos about the running attackjobs */
	bool debugAjobs;
	/** show infos about the bases of the server. Only works on the host */
	bool debugBaseServer;
	/** show infos about the bases of the client */
	bool debugBaseClient;
	/** show infos about the sentries */
	bool debugSentry;
	/** show FX-infos */
	bool debugFX;
	/** show infos from the server about the unit under the mouse */
	bool debugTraceServer;
	/** show infos from the client about the unit under the mouse */
	bool debugTraceClient;
	/** show infos from the client about the unit under the mouse */
	bool debugPlayers;
	/** show drawing cache debug information */
	bool debugCache;
	bool debugSync;

	cPosition drawPosition;

	void setPrintPosition(cPosition position);
	void print(const std::string& text, eUnicodeFontType font = FONT_LATIN_SMALL_WHITE);

	void trace();
	void traceVehicle (const cVehicle& vehicle, cPosition& drawPosition);
	void traceBuilding (const cBuilding& Building, cPosition& drawPosition);
};

#endif // ui_graphical_game_widgets_debugoutputidgetH
