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

#ifndef ui_graphical_game_widgets_unitvideowidgetH
#define ui_graphical_game_widgets_unitvideowidgetH

#include <memory>

#include "ui/graphical/widget.h"
#include "SDL_flic.h"

#include "utility/signal/signalconnectionmanager.h"

class cPosition;

template<typename T>
class cBox;

class cUnit;
class cImage;
class cAnimationTimer;

class cUnitVideoWidget : public cWidget
{
	typedef std::unique_ptr<FLI_Animation, void (*) (FLI_Animation*)> FliAnimationPointerType;
public:
	cUnitVideoWidget (const cBox<cPosition>& area, std::shared_ptr<cAnimationTimer> animationTimer);

	void start();
	void stop();

	bool isPlaying() const;

	void setUnit (const cUnit* unit);
private:
	cImage* currentFrameImage;
	FliAnimationPointerType fliAnimation;

	cSignalConnectionManager signalConnectionManager;

	bool playing;

	void nextFrame();
};

#endif // ui_graphical_game_widgets_unitvideowidgetH
