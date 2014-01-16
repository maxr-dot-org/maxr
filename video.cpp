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
#include "unifonts.h"

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
/** Minimum video mode resultion we need */
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

	SDL_UpdateTexture (sdlTexture, NULL, screen->pixels, screen->pitch);
	SDL_RenderClear (sdlRenderer);
	SDL_RenderCopy (sdlRenderer, sdlTexture, NULL, NULL);
	SDL_RenderPresent (sdlRenderer);
}

int cVideo::applySettings()
{
	Log.write ("cVideo: Applying new video settings", cLog::eLOG_TYPE_DEBUG);

	SDL_SetWindowBordered (sdlWindow, SDL_TRUE);
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
	SDL_RenderSetLogicalSize (sdlRenderer, getResolutionX(), getResolutionY());
	if (buffer) SDL_FreeSurface (buffer);
	buffer = SDL_CreateRGBSurface (0, getResolutionX(), getResolutionY(), getColDepth(),
								   //0, 0, 0, 0);
								   0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	if (font != NULL) font->setTargetSurface (buffer);
	if (sdlTexture) SDL_DestroyTexture (sdlTexture);
	sdlTexture = SDL_CreateTexture (sdlRenderer,
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

	sdlWindow = SDL_CreateWindow (sVersion.c_str(),
								  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
								  getSplashW(), getSplashH(),
								  SDL_WINDOW_BORDERLESS | SDL_WINDOW_OPENGL);
	SDL_SetWindowIcon (sdlWindow, AutoSurface (SDL_LoadBMP (MAXR_ICON)));
	SDL_SetWindowFullscreen (sdlWindow, !getWindowMode());
	sdlRenderer = SDL_CreateRenderer (sdlWindow, -1, 0);

	screen = SDL_CreateRGBSurface (0, getSplashW(), getSplashH(), getColDepth(),
								   0, 0, 0, 0);
	buffer = SDL_CreateRGBSurface (0, getSplashW(), getSplashH(), getColDepth(),
								   0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	if (font != NULL) font->setTargetSurface (buffer);

	SDL_BlitSurface (splash, NULL, buffer, NULL);

	sdlTexture = SDL_CreateTexture (sdlRenderer,
									SDL_PIXELFORMAT_ARGB8888,
									SDL_TEXTUREACCESS_STREAMING,
									getSplashW(), getSplashH());
	// make the scaled rendering look smoother.
	SDL_SetHint (SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize (sdlRenderer, getSplashW(), getSplashH());

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
	const unsigned int modeCount = SDL_GetNumDisplayModes (displayIndex);
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

void blittPerSurfaceAlphaToAlphaChannel (SDL_Surface* src, SDL_Rect* srcrect, SDL_Surface* dst, SDL_Rect* dstrect)
{
	SDL_Rect temp1, temp2;

	if (!dst || !src) return;

	// check surface formats
	if (!dst->format->Amask || src->format->Amask) return;
	if (SDL_GetSurfaceAlphaMod (src, NULL) != 0) return;

	if (srcrect == NULL)
	{
		srcrect = &temp1;
		srcrect->x = 0;
		srcrect->y = 0;
		srcrect->h = src->h;
		srcrect->w = src->w;
	}

	if (dstrect == NULL)
	{
		dstrect = &temp2;
		dstrect->x = 0;
		dstrect->y = 0;
	}

	int width  = srcrect->w;
	int height = srcrect->h;

	//clip source rect to the source surface
	if (srcrect->x < 0)
	{
		width += srcrect->x;
		srcrect->x = 0;
	}
	if (srcrect->y < 0)
	{
		height += srcrect->y;
		srcrect->y = 0;
	}
	width  = std::min (src->w - srcrect->x, width);
	height = std::min (src->h - srcrect->y, height);

	// clip the dest rect to the destination clip rect
	if (dstrect->x < dst->clip_rect.x)
	{
		width -= dst->clip_rect.x - dstrect->x;
		srcrect->x += dst->clip_rect.x - dstrect->x;
		dstrect->x = dst->clip_rect.x;
	}
	if (dstrect->y < dst->clip_rect.y)
	{
		height -= dst->clip_rect.y - dstrect->y;
		srcrect->y += dst->clip_rect.y - dstrect->y;
		dstrect->y = dst->clip_rect.y;
	}
	width  = std::min (dst->clip_rect.x + dst->clip_rect.w - dstrect->x, width);
	height = std::min (dst->clip_rect.y + dst->clip_rect.h - dstrect->y, height);

	if (width <= 0 || height <= 0)
	{
		dstrect->w = 0;
		dstrect->h = 0;
		return;
	}

	if (SDL_MUSTLOCK (src)) SDL_LockSurface (src);
	if (SDL_MUSTLOCK (dst)) SDL_LockSurface (dst);

	//setup needed variables
	Uint8 srcAlpha = 0;
	SDL_GetSurfaceAlphaMod (src, &srcAlpha);
	const int srmask = src->format->Rmask;
	const int sgmask = src->format->Gmask;
	const int sbmask = src->format->Bmask;
	const int damask = dst->format->Amask;
	const int drmask = dst->format->Rmask;
	const int dgmask = dst->format->Gmask;
	const int dbmask = dst->format->Bmask;
	const int rshift = src->format->Rshift - dst->format->Rshift;
	const int gshift = src->format->Gshift - dst->format->Gshift;
	const int bshift = src->format->Bshift - dst->format->Bshift;
	const int ashift = dst->format->Ashift;
	Uint32 colorKey = 0;
	const bool useColorKey = SDL_GetColorKey (src, &colorKey) == 0;

	Uint32* dstPixel = static_cast<Uint32*> (dst->pixels) + dstrect->x + dstrect->y * dst->w;
	Uint32* srcPixel = static_cast<Uint32*> (src->pixels) + srcrect->x + srcrect->y * src->w;

	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			const Uint32 dcolor = *dstPixel;
			const Uint32 scolor = *srcPixel;

			if (useColorKey && scolor == colorKey)
			{
				dstPixel++;
				srcPixel++;
				continue;
			}

			Uint32 r = (scolor & srmask) >> rshift;
			Uint32 g = (scolor & sgmask) >> gshift;
			Uint32 b = (scolor & sbmask) >> bshift;
			Uint32 dalpha = (dcolor & damask) >> ashift;

			r = (((dcolor & drmask) >> 8) * (255 - srcAlpha) * dalpha) + r * srcAlpha;
			g = (((dcolor & dgmask) * (255 - srcAlpha) * dalpha) >> 8) + g * srcAlpha;
			b = (((dcolor & dbmask) * (255 - srcAlpha) * dalpha) >> 8) + b * srcAlpha;

			const Uint8 a = srcAlpha + dalpha - (srcAlpha * dalpha) / 255;

			if (a > 0)
			{
				r /= a;
				g /= a;
				b /= a;
			}

			r = r & drmask;
			g = g & dgmask;
			b = b & dbmask;

			*dstPixel = r | g | b | a << ashift;

			dstPixel++;
			srcPixel++;
		}

		dstPixel += dst->pitch / 4 - width;
		srcPixel += src->pitch / 4 - width;
	}

	if (SDL_MUSTLOCK (src)) SDL_UnlockSurface (src);
	if (SDL_MUSTLOCK (dst)) SDL_UnlockSurface (dst);
}

void blittAlphaSurface (SDL_Surface* src, SDL_Rect* srcrect, SDL_Surface* dst, SDL_Rect* dstrect)
{
	// TODO: [SDL2] special blitSurface seems useless.
	if (dst->format->Amask && SDL_GetSurfaceAlphaMod (src, NULL) == 0)
		blittPerSurfaceAlphaToAlphaChannel (src, srcrect, dst, dstrect);
	else
		SDL_BlitSurface (src, srcrect, dst, dstrect);
}

/**
 * draws one line of the source surface scaled to the destination surface.
 * @author alzi alias DoctorDeath
 * @param srcPixelData pointer to the first byte of the line in the pixeldate
 *        of the source surface.
 * @param srcWidth width of the line in the sourcesurface.
 * @param destPixelData pointer to the first byte where in the pixeldate
 *        of the source surface the line should be drawn.
 * @param destWidth Directory width of the line how it should be drawn
 *        to the destination surface.
 */
template<typename Type>
static void drawStetchedLine (Type* srcPixelData, int srcWidth, Type* destPixelData, int destWidth)
{
	int i = 0;
	int width = destWidth;
	Type pixel = 0;
	Type* srcEnd = srcPixelData + srcWidth;
	// go trough all pixel in this line
	while (srcPixelData < srcEnd)
	{
		pixel = *srcPixelData;
drawpixel:
		// copy the pixel
		*destPixelData++ = pixel;
		width--;
		i += srcWidth;
		if (!width) break;
		// draw pixel once more if necessary
		if (i < destWidth) goto drawpixel;
		// skip pixels when necessary
		do
		{
			i -= destWidth;
			srcPixelData++;
		}
		while (i >= destWidth);
	}
}

SDL_Surface* scaleSurface (SDL_Surface* scr, SDL_Surface* dest, int width, int height)
{
	if (width <= 0 || height <= 0 || !scr) return NULL;
	SDL_Surface* surface;

	// can not enlage an existing surface
	if (width > scr->w && dest) width = scr->w;
	if (height > scr->h && dest) height = scr->h;

	// generate new surface if necessary
	if (dest == NULL) surface = SDL_CreateRGBSurface (0, width, height, scr->format->BitsPerPixel, scr->format->Rmask, scr->format->Gmask, scr->format->Bmask, scr->format->Amask);
	else
	{
		// else set the size of the old one
		surface = dest;
		surface->w = width;
		surface->h = height;
	}

	if (SDL_MUSTLOCK (scr)) SDL_LockSurface (scr);
	if (SDL_MUSTLOCK (surface)) SDL_LockSurface (surface);

#if 0
	// just blit the surface when the new size is identic to the old one
	if (scr->w == width && scr->h == height)
	{
		SDL_BlitSurface (scr, NULL, surface, NULL);
		return surface;
	}
#endif
	// copy palette when necessary
	if (scr->format->BitsPerPixel == 8 && !dest)
	{
		for (int i = 0; i < 256; i++)
		{
			surface->format->palette->colors[i].r = scr->format->palette->colors[i].r;
			surface->format->palette->colors[i].g = scr->format->palette->colors[i].g;
			surface->format->palette->colors[i].b = scr->format->palette->colors[i].b;
		}
	}

	int srcRow = 0;
	int destRow = 0;
	int i = 0;
	// go trough all rows
	while (srcRow < scr->h)
	{
		Uint8* srcPixelData = static_cast<Uint8*> (scr->pixels) + (srcRow * scr->pitch);
		// draw the complete line
drawline:
		Uint8* destPixelData = static_cast<Uint8*> (surface->pixels) + (destRow * surface->pitch);

		// pay attention to different surface formats
		switch (scr->format->BytesPerPixel)
		{
			case 1:
				drawStetchedLine<Uint8> (srcPixelData, scr->w, destPixelData, surface->w);
				break;
			case 2:
				drawStetchedLine<Uint16> (reinterpret_cast<Uint16*> (srcPixelData), scr->w, (Uint16*) destPixelData, surface->w);
				break;
			case 3:
				// not yet supported
				break;
			case 4:
				drawStetchedLine<Uint32> (reinterpret_cast<Uint32*> (srcPixelData), scr->w, (Uint32*) destPixelData, surface->w);
				break;
		}
		destRow++;
		i += scr->h;
		// break when we have already finished
		if (destRow == surface->h) break;
		// draw the line once more when the destiniation surface has
		// a bigger height than the source surface
		if (i < surface->h) goto drawline;
		// skip lines in the source surface when the destination surface has
		// a smaller height than the source surface
		do
		{
			i -= surface->h;
			srcRow++;
		}
		while (i >= surface->h);
	}

	if (SDL_MUSTLOCK (scr)) SDL_UnlockSurface (scr);
	if (SDL_MUSTLOCK (surface)) SDL_UnlockSurface (surface);

	return surface;
}

static void line (int x1, int y1, int x2, int y2, unsigned int color, SDL_Surface* sf)
{
	if (x2 < x1)
	{
		std::swap (x1, x2);
		std::swap (y1, y2);
	}
	int dx = x2 - x1;
	int dy = y2 - y1;
	int dir = 1;
	if (dy < 0) {dy = -dy; dir = -1;}
	int error = 0;
	Uint32* ptr = static_cast<Uint32*> (sf->pixels);
	if (dx > dy)
	{
		for (; x1 != x2; x1++, error += dy)
		{
			if (error > dx) {error -= dx; y1 += dir;}
			if (x1 < sf->w && x1 >= 0 && y1 >= 0 && y1 < sf->h)
				ptr[x1 + y1 * sf->w] = color;
		}
		return;
	}
	for (; y1 != y2; y1 += dir, error += dx)
	{
		if (error > dy) {error -= dy; x1++;}
		if (x1 < sf->w && x1 >= 0 && y1 >= 0 && y1 < sf->h)
			ptr[x1 + y1 * sf->w] = color;
	}
}

// CreatePfeil ////////////////////////////////////////////////////////////////
// Erzeigt ein Pfeil-Surface:
SDL_Surface* CreatePfeil (int p1x, int p1y, int p2x, int p2y, int p3x, int p3y, unsigned int color, int size)
{
	SDL_Surface* sf;
	sf = SDL_CreateRGBSurface (0, size, size, Video.getColDepth(), 0, 0, 0, 0);
	SDL_SetColorKey (sf, SDL_TRUE, 0x00FF00FF);
	SDL_FillRect (sf, NULL, 0x00FF00FF);
	SDL_LockSurface (sf);

	const float fak = size / 64.0f;
	p1x = Round (p1x * fak);
	p1y = Round (p1y * fak);
	p2x = Round (p2x * fak);
	p2y = Round (p2y * fak);
	p3x = Round (p3x * fak);
	p3y = Round (p3y * fak);
	line (p1x, p1y, p2x, p2y, color, sf);
	line (p2x, p2y, p3x, p3y, color, sf);
	line (p3x, p3y, p1x, p1y, color, sf);

	SDL_UnlockSurface (sf);
	return sf;
}

static void setPixel (SDL_Surface* surface, int x, int y, int iColor)
{
	// check the surface size
	if (x < 0 || x >= surface->w || y < 0 || y >= surface->h) return;
	// check the clip rect
	if (x < surface->clip_rect.x || x >= surface->clip_rect.x + surface->clip_rect.w ||
		y < surface->clip_rect.y || y >= surface->clip_rect.y + surface->clip_rect.h) return;

	static_cast<Uint32*> (surface->pixels) [x + y * surface->w] = iColor;
}

void drawCircle (int iX, int iY, int iRadius, int iColor, SDL_Surface* surface)
{
	if (iX + iRadius < 0 || iX - iRadius > Video.getResolutionX() ||
		iY + iRadius < 0 || iY - iRadius > Video.getResolutionY()) return;
	SDL_LockSurface (surface);

	int d = 0;
	int xx = 0;
	int yy = iRadius;
	int bry = Round (0.70710678f * iRadius);
	while (yy > bry)
	{
		int da = d + (xx << 1) + 1;
		int db = da - (yy << 1) + 1;
		if (abs (da) < abs (db))
		{
			d = da;
			xx++;
		}
		else
		{
			d = db;
			xx++;
			yy--;
		}
		setPixel (surface, iX + xx, iY + yy, iColor);
		setPixel (surface, iX + yy, iY + xx, iColor);
		setPixel (surface, iX + yy, iY - xx, iColor);
		setPixel (surface, iX + xx, iY - yy, iColor);
		setPixel (surface, iX - xx, iY + yy, iColor);
		setPixel (surface, iX - yy, iY + xx, iColor);
		setPixel (surface, iX - yy, iY - xx, iColor);
		setPixel (surface, iX - xx, iY - yy, iColor);
	}
	SDL_UnlockSurface (surface);
}
