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

#include "output/video/unifonts.h"
#include "ui/graphical/widget.h"

class cBuilding;
class cChatCommandExecutor;
class cClient;
class cGameMapWidget;
class cPosition;
class cServer;
class cVehicle;

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

	void setClient (const cClient*);
	void setServer (const cServer*);
	void setGameMap (const cGameMapWidget*);

	void initChatCommand (std::vector<std::unique_ptr<cChatCommandExecutor>>&);

	void draw (SDL_Surface& destination, const cBox<cPosition>& clipRect) override;

private:
	void setPrintPosition (cPosition);
	void print (const std::string&, eUnicodeFontType = FONT_LATIN_SMALL_WHITE);

	void trace();
	void traceVehicle (const cVehicle&, cPosition& drawPosition);
	void traceBuilding (const cBuilding&, cPosition& drawPosition);

	/** draw the content of the 'detectedByPlayer' lists over the units */
	void drawDetectedByPlayerList();
	/** draw the detection maps of all players */
	void drawDetectionMaps();

	void drawSentryMaps();

private:
	const cServer* server = nullptr;
	const cClient* client = nullptr;
	const cGameMapWidget* gameMap = nullptr;

	/** show infos about the running attackjobs */
	bool debugAjobs = false;
	/** show infos about the bases of the server. Only works on the host */
	bool debugBaseServer = false;
	/** show infos about the bases of the client */
	bool debugBaseClient = false;
	/** show infos about the sentries */
	bool debugSentry = false;
	/** show FX-infos */
	bool debugFX = false;
	/** show infos from the server about the unit under the mouse */
	bool debugTraceServer = false;
	/** show infos from the client about the unit under the mouse */
	bool debugTraceClient = false;
	/** show infos from the client about the unit under the mouse */
	bool debugPlayers = false;
	/** show drawing cache debug information */
	bool debugCache = false;
	bool debugSync = false;
	bool debugStealth = false;

	cPosition drawPosition;
};

#endif
