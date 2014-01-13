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

#include "main.h"
#include "video.h"

#include "autosurface.h"
#include "defines.h"
#include "log.h"
#include "mouse.h"
#include "pcx.h"

#include <algorithm>
#include <vector>

#include <SDL.h>

// TODO: [SDL2] move static into Video

static SDL_Window* sdlWindow;
static SDL_Renderer* sdlRenderer;
static SDL_Texture* sdlTexture;

cVideo Video;

/** Slashscreen width  */
#define SPLASHWIDTH 500
/** Slashscreen height  */
#define SPLASHHEIGHT 420
#define COLOURDEPTH 32
/**Minimum video mode resultion we need */
#define MINWIDTH 640
#define MINHEIGHT 480

struct sVidMode
{
	unsigned int width;
	unsigned int height;
};

static bool operator < (const sVidMode& lhs, const sVidMode& rhs)
{
	if (lhs.width != rhs.width) return lhs.width < rhs.width;
	return lhs.height < rhs.height;
}

static bool operator == (const sVidMode& lhs, const sVidMode& rhs)
{
	return lhs.width == rhs.width && lhs.height == rhs.height;
}

struct sVidData
{
	unsigned int width;
	unsigned int height;
	int iColDepth;
	bool bWindowMode;
};

/**
* Will store detected videomodes
*/
static std::vector<sVidMode> vVideoMode;

/**
* Stores our actual video data
*/
static sVidData videoData = { MINWIDTH, MINHEIGHT, 32, false };

int cVideo::setResolution (int iWidth, int iHeight, bool bApply)
{
	videoData.width = iWidth;
	videoData.height = iHeight;

	//validate only if we should apply because the resolution may be set during reading of settings and at this point does no SDL context exist yet
	if (bApply)
	{
		//detect what modes we have
		doDetection();
		// BEGIN SANITY CHECK SCREEN RES

		if (validateMode (iWidth, iHeight) >= 0)
		{
			Log.write ("cVideo:  => Found requested video mode " + iToStr (iWidth) + "x" + iToStr (iHeight) + " :)", cLog::eLOG_TYPE_INFO);
		}
		else
		{
			Log.write ("cVideo:  => Couldn't find requested video mode " + iToStr (iWidth) + "x" + iToStr (iHeight) + " :(", cLog::eLOG_TYPE_WARNING);
			if (bHaveMinMode())
			{
				Log.write ("cVideo:  => Edit your config and try default video mode " + iToStr (Video.getMinW()) + "x" + iToStr (Video.getMinH()) + " if I crash now!", cLog::eLOG_TYPE_WARNING);
			}
			else
			{
				Log.write ("cVideo:  => Couldn't even find my minimal video mode " + iToStr (Video.getMinW()) + "x" + iToStr (Video.getMinH()) + " - panic! ;(", cLog::eLOG_TYPE_WARNING);
			}
		}
		// END SANITY CHECK SCREEN RES

		return applySettings();
	}
	else
	{
		Log.write ("cVideo: Resolution set to " + iToStr (iWidth) + "x" + iToStr (iHeight) + " but was not applied yet", cLog::eLOG_TYPE_INFO);
	}

	return 0;
}

int cVideo::setColDepth (unsigned iDepth)
{
	// TODO: Implement other colourdepths beside 32 & add sanity checks.
	//       validate new color depth
	if (iDepth != 32)
	{
		Log.write ("cVideo: TODO: Implement other colourdepths beside 32. Desired " + iToStr (iDepth) + "bpp ignored.", cLog::eLOG_TYPE_WARNING);
		return -1;
	}
	else
	{
		// TODO: [SDL2] : sanitycheck
		videoData.iColDepth = iDepth;
	}
	return 0;
}

int cVideo::getColDepth() const
{
	return videoData.iColDepth;
}

int cVideo::setWindowMode (bool bWindowMode, bool bApply)
{
	videoData.bWindowMode = bWindowMode;
	Log.write ("cVideo: Window mode settings changed to " + std::string (getWindowMode() ? "windowmode" : "fullscreen"), cLog::eLOG_TYPE_DEBUG);

	if (bApply)
	{
		return applySettings();
	}
	return 0;
}

void cVideo::draw()
{
	// TODO: add sanity check to redraw function
	SDL_BlitSurface (buffer, NULL, screen, NULL);

	SDL_UpdateTexture(sdlTexture, NULL, screen->pixels, screen->pitch);
	SDL_RenderClear(sdlRenderer);
	SDL_RenderCopy(sdlRenderer, sdlTexture, NULL, NULL);
	SDL_RenderPresent(sdlRenderer);
}

int cVideo::applySettings()
{
	Log.write ("cVideo: Applying new video settings", cLog::eLOG_TYPE_DEBUG);

	SDL_SetWindowBordered(sdlWindow, SDL_TRUE);
	SDL_SetWindowSize (sdlWindow, getResolutionX(), getResolutionY());
	// TODO: [SDL2]: manage window mode correctly
	if (SDL_SetWindowFullscreen (sdlWindow, !getWindowMode()) == -1)
	{
		videoData.width = screen->w;
		videoData.height = screen->h;
		SDL_SetWindowSize (sdlWindow, getResolutionX(), getResolutionY());
		return -1;
	}

	if (screen) SDL_FreeSurface (screen);
	screen = SDL_CreateRGBSurface (0, getResolutionX(), getResolutionY(), getColDepth(),
								   0, 0, 0, 0);
								   //0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	SDL_RenderSetLogicalSize(sdlRenderer, getResolutionX(), getResolutionY());
	if (buffer) SDL_FreeSurface (buffer);
	buffer = SDL_CreateRGBSurface (0, getResolutionX(), getResolutionY(), getColDepth(),
								   //0, 0, 0, 0);
								   0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	if (sdlTexture) SDL_DestroyTexture (sdlTexture);
	sdlTexture = SDL_CreateTexture(sdlRenderer,
								   SDL_PIXELFORMAT_ARGB8888,
								   SDL_TEXTUREACCESS_STREAMING,
								   getResolutionX(), getResolutionY());
	draw();
	return 0;
}

void cVideo::clearBuffer()
{
	SDL_FillRect (buffer, NULL, SDL_MapRGB (buffer->format, 0, 0, 0));
}

/**
 * Shows splashscreen
 */
void cVideo::initSplash()
{
	AutoSurface splash (LoadPCX (SPLASH_BACKGROUND));

	std::string sVersion = PACKAGE_NAME " " PACKAGE_VERSION " " PACKAGE_REV " ";

	sdlWindow = SDL_CreateWindow(sVersion.c_str(),
								 SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
								 getSplashW(), getSplashH(),
								 SDL_WINDOW_BORDERLESS | SDL_WINDOW_OPENGL);
	SDL_SetWindowIcon (sdlWindow, AutoSurface (SDL_LoadBMP (MAXR_ICON)));
	SDL_SetWindowFullscreen (sdlWindow, !getWindowMode());
	sdlRenderer = SDL_CreateRenderer(sdlWindow, -1, 0);

	screen = SDL_CreateRGBSurface(0, getSplashW(), getSplashH(), getColDepth(),
								  0, 0, 0, 0);
	buffer = SDL_CreateRGBSurface(0, getSplashW(), getSplashH(), getColDepth(),
								  0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	SDL_BlitSurface (splash, NULL, buffer, NULL);

	sdlTexture = SDL_CreateTexture(sdlRenderer,
								   SDL_PIXELFORMAT_ARGB8888,
								   SDL_TEXTUREACCESS_STREAMING,
								   getSplashW(), getSplashH());
	// make the scaled rendering look smoother.
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(sdlRenderer, getSplashW(), getSplashH());

	draw();
}

bool cVideo::getWindowMode() const
{
	return videoData.bWindowMode;
}

int cVideo::getResolutionX() const
{
	return videoData.width;
}

int cVideo::getResolutionY() const
{
	return videoData.height;
}

std::string cVideo::getVideoMode (unsigned int iMode) const
{
	return iToStr (vVideoMode[iMode].width) + "x" + iToStr (vVideoMode[iMode].height);
}

void cVideo::doDetection()
{
	Log.write ("cVideo: Screen resolution detection started. Results may vary!", cLog::eLOG_TYPE_INFO);

	const int displayIndex = sdlWindow ? SDL_GetWindowDisplayIndex (sdlWindow) : 0;
	const unsigned int modeCount = SDL_GetNumDisplayModes(displayIndex);
	/* Print and store detected modes */
	for (unsigned int i = 0; i != modeCount; ++i)
	{
		SDL_DisplayMode mode;
		SDL_GetDisplayMode (displayIndex, i, &mode);

		// write detected video modes
		// (don't write modes *below* the minimum mode'
		if (mode.w >= MINWIDTH && mode.h >= MINHEIGHT)
		{
			const sVidMode tmp = { unsigned (mode.w), unsigned (mode.h) };
			vVideoMode.push_back (tmp);
		}
	}
	std::sort (vVideoMode.begin(), vVideoMode.end());
	vVideoMode.erase (std::unique (vVideoMode.begin(), vVideoMode.end()), vVideoMode.end());
	for (size_t i = 0; i != vVideoMode.size(); ++i)
	{
		sVidMode& mode = vVideoMode[i];
		Log.write ("cVideo: Offering detected video mode " + iToStr (i) + " (" + iToStr (mode.width) + "x" + iToStr (mode.height) + ")", cLog::eLOG_TYPE_INFO);
	}
}

bool cVideo::bHaveMinMode() const
{
	for (unsigned int i = 0; i < vVideoMode.size(); i++)
	{
		if (vVideoMode[i].width == MINWIDTH && vVideoMode[i].height == MINHEIGHT)
		{
			return true;
		}
	}

	Log.write ("cVideo: Minimal needed video mode (" + iToStr (MINWIDTH) + "x" + iToStr (MINHEIGHT) + ") not detected. Probably bad!", cLog::eLOG_TYPE_ERROR);
	return false;
}

int cVideo::validateMode (unsigned int iWidth, unsigned int iHeight) const
{
	for (unsigned int i = 0; i < vVideoMode.size(); i++)
	{
		if (vVideoMode[i].width == iWidth && vVideoMode[i].height == iHeight)
		{
			return i;
		}
	}
	Log.write ("cVideo: Configured video mode (" + iToStr (iWidth) + "x" + iToStr (iHeight) + ") not detected. Resume on own risk!", cLog::eLOG_TYPE_WARNING);
	return -1;
}

int cVideo::getVideoSize() const
{
	return vVideoMode.size();
}

int cVideo::getSplashW() const
{
	return SPLASHWIDTH;
}

int cVideo::getSplashH() const
{
	return SPLASHHEIGHT;
}

int cVideo::getMinW() const
{
	return MINWIDTH;
}

int cVideo::getMinH() const
{
	return MINHEIGHT;
}
