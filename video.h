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

#ifndef videoH
#define videoH

#include "autosurface.h"
struct SDL_Texture;
struct SDL_Renderer;
struct SDL_Window;
struct SDL_Surface;

/**
 * cVideo class.
 * Stores videosettings (and can hopefully operate 'em too one day'):-)
 *
 * @author Bernd "beko" Kosmahl
 */
class cVideo
{
public:
	cVideo();
	~cVideo();

	void clearMemory();

	/**
	* Sets whether app should appear windowed or in fullscreen mode
	* @param bWindowMode pass true if app should work in windowed mode
	*                    pass false it app should start in fullscreen
	* @param bApply set to true if app should apply new windowMode too
	* @return 0 on success
	*/
	int setWindowMode (bool bWindowMode, bool bApply = false);

	/**
	* Get whether app should appear windowed or in fullscreen mode
	* @return  true if app should work in windowed mode
	*          false it app should start in fullscreen
	*/
	bool getWindowMode() const;

	/**
	* Set resolution/dimension of app window.
	* @param iWidth desired window width
	* @param iHeight desired window height
	* @param bApply  set to true if app should apply new resolution too
	* @return 0 on success
	*/
	int setResolution (int iWidth, int iHeight, bool bApply = false);

	/**
	* @deprecated for compat only - will be removed!
	* @return stored window width
	*/
	int getResolutionX() const;

	/**
	* @deprecated for compat only - will be removed!
	* @return stored window height
	*/
	int getResolutionY() const;

	/**
	* Sets colordepth
	* @param iDepth colordepth to set. e.g. 32 (bpp)
	* @return 0 on success
	*/
	int setColDepth (unsigned int iDepth);

	/**
	* Gets colordepth
	* @return colordepth
	*/
	int getColDepth() const;

	/**
	* @return Detected videomodes
	*/
	int getVideoSize() const;

	/**
	* @param iMode video mode num from video mode array
	* @return Videomode as string widthxheight.
	*         If iMode is unknown minimal needed videomode will be returned.
	*/
	std::string getVideoMode (unsigned int iMode) const;

	/**
	* Try to autodetect availavle video modes from SDL.
	*/
	void doDetection();

	/**
	* Check whether the provided mode is known to our video mode list
	* @param width Screenwidth to look for
	* @param height Screenheight to look for
	* @return iMode or -1 on unknown mode
	*/
	int validateMode (unsigned int iWidth, unsigned int iHeight) const;

	/**
	* @return Splash width
	*/
	int getSplashW() const;

	/**
	* @return Splash height
	*/
	int getSplashH() const;

	/**
	* @return Minimal needed screen resolution width
	*/
	int getMinW() const;

	/**
	* @return Minimal needed screen resolution height
	*/
	int getMinH() const;

	/**
	* Inits our buffers and draws the splashscreen
	*/
	void initSplash();

	/**
	* clears buffer (black)
	*/
	void clearBuffer();

	/**
	* Blits or updates buffer to screen depending on windowmode
	*/
	void draw();


	void takeScreenShot (const std::string& filename) const;

	void applyShadow (const SDL_Rect* rect);

	// Screenbuffers ///////////////////////////////////
	static SDL_Surface* buffer; // Der Bildschirm-Buffer

private:

	/**
	* Checks whether our minimal needed videomode has been autodetected
	* @return true if mininal video mode looks valid
	*/
	bool bHaveMinMode() const;

	/**
	* Applys current video settings
	* @return 0 on success
	*/
	int applySettings();
private:
	SDL_Window* sdlWindow;
	SDL_Renderer* sdlRenderer;
	SDL_Texture* sdlTexture;
};

extern cVideo Video;

/**
* Works like SDL_BlittSurface.
* But unlike SDL it respects the destination alpha channel of the surface.
* This function is only designed to blitt from a surface
* with per surface alpha value to a surface with alpha channel.
* A source color key is also supported.
*/
void blittPerSurfaceAlphaToAlphaChannel (SDL_Surface* src, SDL_Rect* srcrect, SDL_Surface* dst, SDL_Rect* dstrect);
/**
* Works like SDL_BlittSurface.
* This function choses the right blitt function to use for blitting.
*/
void blittAlphaSurface (SDL_Surface* src, SDL_Rect* srcrect, SDL_Surface* dst, SDL_Rect* dstrect);

/**
 * scale a surface to the overgiven sice.
 * The scaled surface will be drawn to the destination surface.
 * If the destiniation surface is NULL a new surface will be created.
 * @author alzi alias DoctorDeath
 * @param scr the surface to be scaled.
 * @param dest the surface that will receive the new pixel data.
 *        If it is NUll a new surface will be created.
 * @param width width of the new surface.
 * @param height height of the new surface.
 * @return returns the destination surface.
 */
SDL_Surface* scaleSurface (SDL_Surface* scr, SDL_Surface* dest, int width, int height);

/** this function checks, whether the surface has to be rescaled,
 * and scales it if necessary */
inline void CHECK_SCALING (SDL_Surface& surface, SDL_Surface& surface_org, float factor)
{
	if (!cSettings::getInstance().shouldDoPrescale() &&
		(surface.w != (int) (surface_org.w * factor) ||
		 surface.h != (int) (surface_org.h * (factor))))
		scaleSurface (&surface_org, &surface, (int) (surface_org.w * factor), (int) (surface_org.h * factor));
}

SDL_Surface* CreatePfeil (int p1x, int p1y, int p2x, int p2y, int p3x, int p3y, unsigned int color, int size);

/** Draws a circle on the surface */
void drawCircle (int iX, int iY, int iRadius, int iColor, SDL_Surface& surface);

#endif
