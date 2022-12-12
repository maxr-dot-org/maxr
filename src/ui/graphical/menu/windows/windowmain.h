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

#ifndef ui_graphical_menu_windows_windowmainH
#define ui_graphical_menu_windows_windowmainH

#include "ui/widgets/window.h"
#include "utility/signal/signalconnectionmanager.h"

struct SDL_Surface;

class cImage;
class cLabel;

class cWindowMain : public cWindow
{
public:
	cWindowMain (const std::string& title);
	~cWindowMain();

	void retranslate() override;

protected:
	void setTitle (const std::string&);

protected:
	static const int buttonSpace = 35;

private:
	cLabel* titleLabel = nullptr;
	cLabel* creditLabel = nullptr;
	cImage* infoImage = nullptr;

	cSignalConnectionManager signalConnectionManager;

	void infoImageClicked();
	SDL_Surface* getRandomInfoImage();
};

#endif // ui_graphical_menu_windows_windowmainH
