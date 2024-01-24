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

#include "ui/widgets/widget.h"
#include "utility/signal/signalconnectionmanager.h"

#include <3rd/SDL_flic/SDL_flic.h>

template <typename T>
class cBox;

class cAnimationTimer;
class cImage;
class cPosition;
class cUnit;

class cUnitVideoWidget : public cWidget
{
public:
	explicit cUnitVideoWidget (const cBox<cPosition>& area);

	void bindConnections (cAnimationTimer&);

	void setUnit (const cUnit*);

	void start();
	void stop();

	bool isPlaying() const;
	bool hasAnimation() const;

private:
	void nextFrame();

private:
	struct FliAnimationCloser
	{
		void operator() (FLI_Animation* anim) const { FLI_Close (anim); }
	};
	using FliAnimationPointerType = std::unique_ptr<FLI_Animation, FliAnimationCloser>;

private:
	cSignal<void()> stateChanged;

	cImage* currentFrameImage = nullptr;
	FliAnimationPointerType fliAnimation;

	cSignalConnectionManager signalConnectionManager;

	bool playing = true;
};

#endif // ui_graphical_game_widgets_unitvideowidgetH
