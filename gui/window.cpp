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
			// fill the whole screen with black to prevent
			// old garbage from menus that don't support resolutions > 640x480
			SDL_FillRect (cVideo::buffer, NULL, 0xFF000000);
			break;
		case eWindowBackgrounds::Alpha:
			if (cSettings::getInstance ().isAlphaEffects ())
				Video.applyShadow (NULL);
			break;
		case eWindowBackgrounds::Transparent:
			// do nothing here
			break;
		}
	}

	SDL_Rect position = getArea().toSdlRect ();
	if (surface) SDL_BlitSurface (surface, NULL, cVideo::buffer, &position);

	hasBeenDrawnOnce = true;

	cWidget::draw (); // draws all children

	Video.draw ();
}

//------------------------------------------------------------------------------
void cWindow::handleActivated (cApplication& application)
{
	// one window should not be displayed in multiple applications at once
	// (we have only one application anyway...)
	assert (activeApplication == nullptr);

	hasBeenDrawnOnce = false;
	activeApplication = &application;
}

//------------------------------------------------------------------------------
void cWindow::handleDeactivated (cApplication& application)
{
	if (activeApplication == &application)
	{
		closing = false;
		activeApplication = nullptr;
	}
}

//------------------------------------------------------------------------------
cApplication* cWindow::getActiveApplication () const
{
	return activeApplication;
}

//------------------------------------------------------------------------------
void cWindow::setSurface (SDL_Surface* surface_)
{
	surface = surface_;
	resize (cPosition (surface->w, surface->h));
}
