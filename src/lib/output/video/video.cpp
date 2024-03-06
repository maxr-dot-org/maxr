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

#include "video.h"

#include "input/keyboard/keyboard.h"
#include "output/video/unifonts.h"
#include "resources/pcx.h"
#include "resources/uidata.h"
#include "utility/log.h"
#include "utility/mathtools.h"
#include "utility/os.h"
#include "SDLutility/tosdl.h"
#include "SDLutility/uniquesurface.h"
#include "utility/narrow_cast.h"
#include "utility/thread/ismainthread.h"

#include <SDL.h>
#include <algorithm>
#include <cassert>
#include <ctime>
#include <functional>
#include <vector>

cVideo Video;

/*static*/ SDL_Surface* cVideo::buffer = nullptr; // the screen buffer

#define COLOURDEPTH 32
/** Minimum video mode resolution we need */
#define MINWIDTH 640
#define MINHEIGHT 480

//------------------------------------------------------------------------------
cVideo::cVideo() :
	resolutionX (MINWIDTH),
	resolutionY (MINHEIGHT),
	displayIndex (MINWIDTH),
	colorDepth (COLOURDEPTH)
{
	signalConnectionManager.connect (cKeyboard::getInstance().keyPressed, [this] (cKeyboard& keyboard, SDL_Keycode key) { keyPressed (keyboard, key); });
}

//------------------------------------------------------------------------------
cVideo::~cVideo()
{
	SDL_FreeSurface (cVideo::buffer);
	SDL_DestroyTexture (sdlTexture);
	SDL_DestroyRenderer (sdlRenderer);
	SDL_DestroyWindow (sdlWindow);
}

//------------------------------------------------------------------------------
void cVideo::clearMemory()
{
	SDL_FreeSurface (cVideo::buffer);
	SDL_DestroyTexture (sdlTexture);
	SDL_DestroyRenderer (sdlRenderer);
	SDL_DestroyWindow (sdlWindow);
	cVideo::buffer = nullptr;
	sdlTexture = nullptr;
	sdlRenderer = nullptr;
	sdlWindow = nullptr;
}

//------------------------------------------------------------------------------
void cVideo::init (const std::string& title, const std::filesystem::path& iconPath)
{
	sdlWindow = SDL_CreateWindow (title.c_str(),
	                              SDL_WINDOWPOS_CENTERED_DISPLAY (getDisplayIndex()),
	                              SDL_WINDOWPOS_CENTERED_DISPLAY (getDisplayIndex()),
	                              MINWIDTH,
	                              MINHEIGHT,
	                              SDL_WINDOW_BORDERLESS | SDL_WINDOW_OPENGL);

	{
		auto icon = UniqueSurface (SDL_LoadBMP (iconPath.u8string().c_str()));
		SDL_SetColorKey (icon.get(), 1, 0xFF00FF);
		SDL_SetWindowIcon (sdlWindow, icon.get());
	}

	sdlRenderer = SDL_CreateRenderer (sdlWindow, -1, 0);

	// make the scaled rendering look smoother.
	SDL_SetHint (SDL_HINT_RENDER_SCALE_QUALITY, "linear");

	detectResolutions();
}

//------------------------------------------------------------------------------
void cVideo::showSplashScreen (const std::filesystem::path& splashScreenPath)
{
	UniqueSurface splash (LoadPCX (splashScreenPath));

	SDL_SetWindowBordered (sdlWindow, SDL_FALSE);
	SDL_SetWindowSize (sdlWindow, splash->w, splash->h);
	SDL_SetWindowPosition (sdlWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
	SDL_SetWindowFullscreen (sdlWindow, 0);

	initializeBuffer (splash->w, splash->h);

	SDL_BlitSurface (splash.get(), nullptr, buffer, nullptr);

	draw();
}

//------------------------------------------------------------------------------
void cVideo::prepareGameScreen()
{
	SDL_SetWindowBordered (sdlWindow, SDL_TRUE);
	SDL_SetWindowSize (sdlWindow, getResolutionX(), getResolutionY());
	SDL_SetWindowPosition (sdlWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
	SDL_SetWindowFullscreen (sdlWindow, getWindowMode() ? 0 : SDL_WINDOW_FULLSCREEN);

	initializeBuffer (getResolutionX(), getResolutionY());

	draw();
}

//------------------------------------------------------------------------------
void cVideo::initializeBuffer (int width, int height)
{
	if (buffer) SDL_FreeSurface (buffer);
	buffer = SDL_CreateRGBSurface (0, width, height, getColDepth(), 0, 0, 0, 0);

	if (cUnicodeFont::font != nullptr)
		cUnicodeFont::font->setTargetSurface (buffer);

	if (sdlTexture) SDL_DestroyTexture (sdlTexture);
	sdlTexture = SDL_CreateTexture (sdlRenderer,
	                                SDL_PIXELFORMAT_ARGB8888,
	                                SDL_TEXTUREACCESS_STREAMING,
	                                width,
	                                height);

	SDL_RenderSetLogicalSize (sdlRenderer, width, height);
}

//------------------------------------------------------------------------------
void cVideo::setResolution (int iWidth, int iHeight, bool bApply)
{
	resolutionX = iWidth;
	resolutionY = iHeight;

	// validate only if we should apply
	// because the resolution may be set during reading of settings
	// and at this point does no SDL context exist yet
	if (bApply)
	{
		// BEGIN SANITY CHECK SCREEN RES

		if (validateResolution (iWidth, iHeight) >= 0)
		{
			Log.info ("cVideo:  => Found requested video mode " + std::to_string (iWidth) + "x" + std::to_string (iHeight) + " :)");
		}
		else
		{
			Log.warn ("cVideo:  => Couldn't find requested video mode " + std::to_string (iWidth) + "x" + std::to_string (iHeight) + " :(");
			if (haveMinMode())
			{
				Log.warn ("cVideo:  => Edit your config and try default video mode " + std::to_string (getMinW()) + "x" + std::to_string (getMinH()) + " if I crash now!");
			}
			else
			{
				Log.warn ("cVideo:  => Couldn't even find my minimal video mode " + std::to_string (getMinW()) + "x" + std::to_string (getMinH()) + " - panic! ;(");
			}
		}
		// END SANITY CHECK SCREEN RES

		applyResolution();

		resolutionChanged();
	}
	else
	{
		Log.info ("cVideo: Resolution set to " + std::to_string (iWidth) + "x" + std::to_string (iHeight) + " but was not applied yet");
	}
}

//------------------------------------------------------------------------------
void cVideo::setColDepth (unsigned int iDepth)
{
	// TODO: Implement other colourdepths beside 32 & add sanity checks.
	//       validate new color depth
	if (iDepth != 32)
	{
		Log.warn ("cVideo: TODO: Implement other colourdepths beside 32. Desired " + std::to_string (iDepth) + "bpp ignored.");
	}
	else
	{
		// TODO: [SDL2] : sanitycheck
		colorDepth = iDepth;
	}
}

//------------------------------------------------------------------------------
void cVideo::setDisplayIndex (int index)
{
	displayIndex = index;
	// TODO: apply the display index (reassign window to new display)
}

//------------------------------------------------------------------------------
void cVideo::setWindowMode (bool bWindowMode, bool bApply)
{
	windowMode = bWindowMode;
	Log.debug ("cVideo: Window mode settings changed to " + std::string (getWindowMode() ? "windowmode" : "fullscreen"));

	if (bApply)
	{
		applyWindowMode();
	}
}

//------------------------------------------------------------------------------
void cVideo::draw()
{
	// SDL2: Some SDL2 functions must be called from the main thread only
	// at least in some platform/configuration.
	// Check for all configurations.
	assert (is_main_thread());

	// TODO: add sanity check to redraw function

	SDL_UpdateTexture (sdlTexture, nullptr, buffer->pixels, buffer->pitch);
	SDL_RenderClear (sdlRenderer);
	SDL_RenderCopy (sdlRenderer, sdlTexture, nullptr, nullptr);
	SDL_RenderPresent (sdlRenderer);
}

//------------------------------------------------------------------------------
void cVideo::applyResolution()
{
	const auto windowFlags = SDL_GetWindowFlags (sdlWindow);
	const auto isFullscreen = (windowFlags & SDL_WINDOW_FULLSCREEN) != 0;

	if (isFullscreen) SDL_SetWindowFullscreen (sdlWindow, 0);

	SDL_SetWindowSize (sdlWindow, getResolutionX(), getResolutionY());

	applyWindowMode();

	initializeBuffer (getResolutionX(), getResolutionY());

	draw();
}

//------------------------------------------------------------------------------
void cVideo::applyWindowMode()
{
	const auto result = SDL_SetWindowFullscreen (sdlWindow, getWindowMode() ? 0 : SDL_WINDOW_FULLSCREEN);
	if (result == -1)
	{
		throw std::runtime_error (std::string ("Could not apply window mode: ") + SDL_GetError() + "'");
	}
}

//------------------------------------------------------------------------------
void cVideo::clearBuffer()
{
	SDL_FillRect (buffer, nullptr, toSdlColor (cRgbColor::black(), *buffer));
}

//------------------------------------------------------------------------------
void cVideo::detectResolutions()
{
	const auto numVideoDislplays = SDL_GetNumVideoDisplays();
	detectedResolutions.resize (numVideoDislplays);
	for (int displayIndex = 0; displayIndex < numVideoDislplays; ++displayIndex)
	{
		const auto numDisplayModes = SDL_GetNumDisplayModes (displayIndex);

		auto& resolutions = detectedResolutions[displayIndex];

		resolutions.clear();

		for (int displayModeIndex = 0; displayModeIndex < numDisplayModes; ++displayModeIndex)
		{
			SDL_DisplayMode mode;
			SDL_GetDisplayMode (displayIndex, displayModeIndex, &mode);

			// skip modes *below* the minimum mode
			if (mode.w < MINWIDTH || mode.h < MINHEIGHT) continue;

			resolutions.push_back (std::make_pair (mode.w, mode.h));
		}
		std::sort (resolutions.begin(), resolutions.end()); // lexicographic order
		resolutions.erase (std::unique (resolutions.begin(), resolutions.end()), resolutions.end()); // make sure there are no double entries

		for (size_t i = 0; i < resolutions.size(); ++i)
		{
			const auto& [w, h] = resolutions[i];
			Log.info ("cVideo: Display" + std::to_string (displayIndex) + " is offering detected video mode " + std::to_string (i) + " (" + std::to_string (w) + "x" + std::to_string (h) + ")");
		}
	}
}

//------------------------------------------------------------------------------
const std::vector<std::pair<int, int>>& cVideo::getDetectedResolutions() const
{
	const int displayIndex = sdlWindow ? SDL_GetWindowDisplayIndex (sdlWindow) : 0;

	return detectedResolutions[displayIndex];
}

//------------------------------------------------------------------------------
bool cVideo::haveMinMode() const
{
	if (ranges::any_of (getDetectedResolutions(), [] (const auto& resolution) { return resolution == std::pair<int, int>{MINWIDTH, MINHEIGHT}; }))
	{
		return true;
	}
	Log.error ("cVideo: Minimal needed video mode (" + std::to_string (MINWIDTH) + "x" + std::to_string (MINHEIGHT) + ") not detected. Probably bad!");
	return false;
}

//------------------------------------------------------------------------------
int cVideo::validateResolution (int width, int height) const
{
	const auto& resolutions = getDetectedResolutions();
	for (size_t i = 0; i < resolutions.size(); i++)
	{
		if (resolutions[i] == std::pair<int, int>{width, height})
		{
			return static_cast<int> (i);
		}
	}
	Log.warn ("cVideo: Configured video mode (" + std::to_string (width) + "x" + std::to_string (height) + ") not detected. Resume on own risk!");
	return -1;
}

//------------------------------------------------------------------------------
int cVideo::getMinW() const
{
	return MINWIDTH;
}

//------------------------------------------------------------------------------
int cVideo::getMinH() const
{
	return MINHEIGHT;
}

//------------------------------------------------------------------------------
void cVideo::takeScreenShot (const std::filesystem::path& filename) const
{
	SDL_SaveBMP (buffer, filename.u8string().c_str());
}

//------------------------------------------------------------------------------
void cVideo::keyPressed (cKeyboard& keyboard, SDL_Keycode key)
{
	if (keyboard.isAnyModifierActive (toEnumFlag (eKeyModifierType::AltLeft) | toEnumFlag (eKeyModifierType::AltRight)))
	{
		if (key == SDLK_RETURN)
		{
			setWindowMode (!getWindowMode(), true);
		}
		else if (key == SDLK_c)
		{
			const auto timestr = os::formattedNow ("screenie_%Y-%m-%d_%H%M%S_");
			std::filesystem::path screenshotfile;
			int counter = 0;
			const auto screenshotDir = cSettings::getInstance().getUserScreenshotsDir();
			do
			{
				counter += 1;
				screenshotfile = screenshotDir / (timestr + std::to_string (counter) + ".bmp");
			} while (std::filesystem::exists (screenshotfile));
			std::filesystem::create_directories (screenshotDir);
			Log.info ("Screenshot saved to " + screenshotfile.u8string());
			takeScreenShot (screenshotfile);

			screenShotTaken (screenshotfile);
		}
	}
}

//------------------------------------------------------------------------------
void cVideo::applyShadow (const SDL_Rect* rect, SDL_Surface& destination)
{
	const SDL_Rect fullscreen = {0, 0, getResolutionX(), getResolutionY()};
	if (rect == nullptr) rect = &fullscreen;
	SDL_Rect dest = {rect->x, rect->y, rect->w, rect->h};
	const UniqueRenderer renderer{SDL_CreateSoftwareRenderer (&destination)};
	const UniqueTexture texture{SDL_CreateTextureFromSurface (renderer.get(), GraphicsData.gfx_shadow.get())};
	SDL_RenderCopy (renderer.get(), texture.get(), nullptr, &dest);
}

//------------------------------------------------------------------------------
void blittPerSurfaceAlphaToAlphaChannel (SDL_Surface* src, SDL_Rect* srcrect, SDL_Surface* dst, SDL_Rect* dstrect)
{
	SDL_Rect temp1, temp2;

	if (!dst || !src) return;

	// check surface formats
	if (!dst->format->Amask || src->format->Amask) return;
	if (SDL_GetSurfaceAlphaMod (src, nullptr) != 0) return;

	if (srcrect == nullptr)
	{
		srcrect = &temp1;
		srcrect->x = 0;
		srcrect->y = 0;
		srcrect->h = src->h;
		srcrect->w = src->w;
	}

	if (dstrect == nullptr)
	{
		dstrect = &temp2;
		dstrect->x = 0;
		dstrect->y = 0;
	}

	int width = srcrect->w;
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
	width = std::min (src->w - srcrect->x, width);
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
	width = std::min (dst->clip_rect.x + dst->clip_rect.w - dstrect->x, width);
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

			const Uint8 a = narrow_cast<Uint8> (srcAlpha + dalpha - (srcAlpha * dalpha) / 255);

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

//------------------------------------------------------------------------------
void blittAlphaSurface (SDL_Surface* src, SDL_Rect* srcrect, SDL_Surface* dst, SDL_Rect* dstrect)
{
	// TODO: [SDL2] special blitSurface seems useless.
	if (dst->format->Amask && SDL_GetSurfaceAlphaMod (src, nullptr) == 0)
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
template <typename Type>
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
		} while (i >= destWidth);
	}
}

//------------------------------------------------------------------------------
SDL_Surface* scaleSurface (SDL_Surface* scr, SDL_Surface* dest, int width, int height)
{
	if (width <= 0 || height <= 0 || !scr) return nullptr;

	// can not enlage an existing surface
	if (width > scr->w && dest) width = scr->w;
	if (height > scr->h && dest) height = scr->h;

	SDL_Surface* surface = nullptr;
	// generate new surface if necessary
	if (dest == nullptr)
		surface = SDL_CreateRGBSurface (0, width, height, scr->format->BitsPerPixel, scr->format->Rmask, scr->format->Gmask, scr->format->Bmask, scr->format->Amask);
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
		SDL_BlitSurface (scr, nullptr, surface, nullptr);
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
		} while (i >= surface->h);
	}

	if (SDL_MUSTLOCK (scr)) SDL_UnlockSurface (scr);
	if (SDL_MUSTLOCK (surface)) SDL_UnlockSurface (surface);

	return surface;
}

//------------------------------------------------------------------------------
static void line (int x1, int y1, int x2, int y2, unsigned int color, SDL_Surface& sf)
{
	if (x2 < x1)
	{
		std::swap (x1, x2);
		std::swap (y1, y2);
	}
	int dx = x2 - x1;
	int dy = y2 - y1;
	int dir = 1;
	if (dy < 0)
	{
		dy = -dy;
		dir = -1;
	}
	int error = 0;
	Uint32* ptr = static_cast<Uint32*> (sf.pixels);
	if (dx > dy)
	{
		for (; x1 != x2; x1++, error += dy)
		{
			if (error > dx)
			{
				error -= dx;
				y1 += dir;
			}
			if (x1 < sf.w && x1 >= 0 && y1 >= 0 && y1 < sf.h)
				ptr[x1 + y1 * sf.w] = color;
		}
		return;
	}
	for (; y1 != y2; y1 += dir, error += dx)
	{
		if (error > dy)
		{
			error -= dy;
			x1++;
		}
		if (x1 < sf.w && x1 >= 0 && y1 >= 0 && y1 < sf.h)
			ptr[x1 + y1 * sf.w] = color;
	}
}

//------------------------------------------------------------------------------
// CreatePfeil ////////////////////////////////////////////////////////////////
// Erzeigt ein Pfeil-Surface:
UniqueSurface CreatePfeil (int p1x, int p1y, int p2x, int p2y, int p3x, int p3y, unsigned int color, int size)
{
	UniqueSurface sf (SDL_CreateRGBSurface (0, size, size, Video.getColDepth(), 0, 0, 0, 0));
	SDL_SetColorKey (sf.get(), SDL_TRUE, 0x00FF00FF);
	SDL_FillRect (sf.get(), nullptr, 0x00FF00FF);
	SDL_LockSurface (sf.get());

	const float fak = size / 64.0f;
	p1x = Round (p1x * fak);
	p1y = Round (p1y * fak);
	p2x = Round (p2x * fak);
	p2y = Round (p2y * fak);
	p3x = Round (p3x * fak);
	p3y = Round (p3y * fak);
	line (p1x, p1y, p2x, p2y, color, *sf);
	line (p2x, p2y, p3x, p3y, color, *sf);
	line (p3x, p3y, p1x, p1y, color, *sf);

	SDL_UnlockSurface (sf.get());
	return sf;
}

//------------------------------------------------------------------------------
static void setPixel (SDL_Surface& surface, int x, int y, int iColor)
{
	// check the surface size
	if (x < 0 || x >= surface.w || y < 0 || y >= surface.h) return;
	// check the clip rect
	if (x < surface.clip_rect.x || x >= surface.clip_rect.x + surface.clip_rect.w || y < surface.clip_rect.y || y >= surface.clip_rect.y + surface.clip_rect.h) return;

	static_cast<Uint32*> (surface.pixels)[x + y * surface.w] = iColor;
}

//------------------------------------------------------------------------------
void drawCircle (int iX, int iY, int iRadius, int iColor, SDL_Surface& surface)
{
	if (iX + iRadius < 0 || iX - iRadius > Video.getResolutionX() || iY + iRadius < 0 || iY - iRadius > Video.getResolutionY()) return;
	SDL_LockSurface (&surface);

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
	SDL_UnlockSurface (&surface);
}

//------------------------------------------------------------------------------
void applySettings (cVideo& video, const sVideoSettings& videoSettings)
{
	auto resolution = videoSettings.resolution.value_or (cPosition (video.getMinW(), video.getMinH()));
	video.setResolution (resolution.x(), resolution.y(), false);
	video.setColDepth (videoSettings.colourDepth);
	video.setWindowMode (videoSettings.windowMode);
	video.setDisplayIndex (videoSettings.displayIndex);
}
