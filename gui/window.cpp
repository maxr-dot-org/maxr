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

#include "window.h"
#include "application.h"

#include "../input/mouse/mouse.h"
#include "../input/mouse/cursor/mousecursorsimple.h"
#include "../settings.h"
#include "../video.h"

//------------------------------------------------------------------------------
cWindow::cWindow (SDL_Surface* surface_, eWindowBackgrounds backgroundType_) :
	surface (nullptr),
	backgroundType (backgroundType_),
	activeApplication (nullptr),
	closing (false),
	hasBeenDrawnOnce (false)
{
	setSurface (surface_);
}

//------------------------------------------------------------------------------
bool cWindow::isClosing () const
{
	return closing;
}

//------------------------------------------------------------------------------
void cWindow::close ()
{
	closing = true;
}

//------------------------------------------------------------------------------
void cWindow::draw ()
{
	if (!hasBeenDrawnOnce)
	{
		switch (backgroundType)
		{
		case eWindowBackgrounds::Black:
			SDL_FillRect (cVideo::buffer, NULL, 0xFF000000);
			break;
		case eWindowBackgrounds::Alpha:
			// NOTE: this is not fully robust yet! It will not work if an
			// alpha-background-window will call another alpha-background-window.
			// Returning to an alpha-background-window from any other window
			// will not work as expected as well.
			if (cSettings::getInstance ().isAlphaEffects ())
			{
				Video.applyShadow (NULL);
			}
			break;
		case eWindowBackgrounds::Transparent:
			// do nothing here
			break;
		}
	}

	SDL_Rect position = getArea().toSdlRect ();
	if (surface != nullptr) SDL_BlitSurface (surface.get (), NULL, cVideo::buffer, &position);

	hasBeenDrawnOnce = true;

	cWidget::draw (); // draws all children
}

//------------------------------------------------------------------------------
void cWindow::handleActivated (cApplication& application)
{
	// one window should not be displayed in multiple applications at once
	// (we have only one application anyway...)
	assert (activeApplication == nullptr);

	hasBeenDrawnOnce = false;
	activeApplication = &application;

	auto mouse = activeApplication->getActiveMouse ();
	if (mouse)
	{
		auto defaultCursor = getDefaultCursor ();
		if (defaultCursor)
		{
			mouse->setCursor (std::move (defaultCursor));
		}
	}
}

//------------------------------------------------------------------------------
void cWindow::handleDeactivated (cApplication& application)
{
	if (activeApplication == &application)
	{
		activeApplication = nullptr;
	}
}

//------------------------------------------------------------------------------
void cWindow::handleRemoved (cApplication& application)
{
	assert (isClosing ());

	terminated ();
}

//------------------------------------------------------------------------------
cApplication* cWindow::getActiveApplication () const
{
	return activeApplication;
}

//------------------------------------------------------------------------------
std::unique_ptr<cMouseCursor> cWindow::getDefaultCursor () const
{
	return std::make_unique<cMouseCursorSimple> (eMouseCursorSimpleType::Hand);
}

//------------------------------------------------------------------------------
cMouse* cWindow::getActiveMouse () const
{
	return activeApplication ? activeApplication->getActiveMouse () : nullptr;
}

//------------------------------------------------------------------------------
cKeyboard* cWindow::getActiveKeyboard () const
{
	return activeApplication ? activeApplication->getActiveKeyboard () : nullptr;
}

//------------------------------------------------------------------------------
SDL_Surface* cWindow::getSurface ()
{
	return surface.get ();
}

//------------------------------------------------------------------------------
void cWindow::setSurface (SDL_Surface* surface_)
{
	surface = surface_;
	if (surface != nullptr)
	{
		resize (cPosition (surface->w, surface->h));
	}
}
