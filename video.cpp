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
#include "log.h"
#include "autosurface.h"
#include "defines.h"
#include "pcx.h"
#include <vector>


/**
*Will store either our detected videomodes or some generic video modes if auto-detection in doDetect() failed
*/
static vector<sVidMode> vVideoMode;

static bool bWasResized = false;

/**
 * Some possible video modes. Don't use external!'
 */
const static sVidMode videoModes[] =
{
    {  MINWIDTH,  MINHEIGHT,  0 },
    {  800,  600,  1 },
    {  960,  720,  2 },
    { 1024,  768,  3 },
    { 1024,  960,  4 },
    { 1152,  864,  5 },
    { 1280,  960,  6 },
    { 1280, 1024,  7 },
    { 1040, 1050,  8 },
    { 1600, 1200,  9 },
    { 2048, 1536, 10 }, //unusual resolutions start here
    { 1024,  480, 11 }, // Sony VAIO Pocketbook
    { 1152,  768, 12 }, // Apple TiBook
    { 1280,  854, 13 }, // Apple TiBook
    {  640,  400, 14 }, // generic 16:10 widescreen
    {  800,  500, 15 }, // as found modern
    { 1024,  640, 16 }, // notebooks
    { 1280,  800, 17 },
    { 1680, 1050, 18 },
    { 1920, 1200, 19 },
    { 1400, 1050, 20 }, // samsung x20
    { 1440,  900, 21 },
    { 1024,  600, 22 } // EEE PC
};

/**
*Stores our actual video data
*/
static sVidData videoData = { MINWIDTH, MINHEIGHT, 0, SDL_SWSURFACE, 32, false };

/**
*cvar for centered splash on screen
*/
static char cVideoPos[] = "SDL_VIDEO_CENTERED=1";

int cVideo::setResolution(int iWidth, int iHeight, bool bApply)
{
  //BEGIN SANITY CHECK SCREEN RES

  if(validateMode(iWidth, iHeight) >= 0)
  {
      Log.write("cVideo:  => Found requested video mode "+iToStr(iWidth)+"x"+iToStr(iHeight)+" :)", cLog::eLOG_TYPE_INFO);
  }
  else
  {
    Log.write("cVideo:  => Couldn't' find requested video mode "+iToStr(iWidth)+"x"+iToStr(iHeight)+" :(", cLog::eLOG_TYPE_WARNING);
    if(bHaveMinMode())
    {
      Log.write("cVideo:  => Edit your config and try default video mode "+iToStr(Video.getMinW())+"x"+iToStr(Video.getMinH())+" if I crash now!", cLog::eLOG_TYPE_WARNING);
    }
    else
    {
      Log.write("cVideo:  => Couldn''t even find my minimal video mode "+iToStr(Video.getMinW())+"x"+iToStr(Video.getMinH())+" - Panik! ;(", cLog::eLOG_TYPE_WARNING);    
    }
  }
  //END SANITY CHECK SCREEN RES
  
  videoData.width = iWidth;
  videoData.height = iHeight;
  
  if(bApply)
  {
    return applySettings();
  }
  return 0;
}

int cVideo::setColDepth(int iDepth)
{
  //TODO: Implement other colourdepths beside 32 & add sanity checks. validate new color depth
  if(iDepth != 32)
  {
    Log.write("cVideo: TODO: Implement other colourdepths beside 32. Desired "+iToStr(iDepth)+"bpp ignored.", cLog::eLOG_TYPE_WARNING);
    return -1;
  }
  else
  {
    const SDL_VideoInfo *vInfo = SDL_GetVideoInfo();
    Uint8 uBpp = vInfo->vfmt->BitsPerPixel;

    if(iDepth > (Uint32)uBpp)
    {
      Log.write("cVideo: Desired bpp is higher than the display ("+iToStr(uBpp)+"bpp) has!", cLog::eLOG_TYPE_WARNING);
    }
    videoData.iColDepth = iDepth;
  }
  return 0;
}

int cVideo::getColDepth(void)
{
  return videoData.iColDepth;
}

int cVideo::setWindowMode(bool bWindowMode, bool bApply)
{  
  videoData.bWindowMode = bWindowMode;
  Log.write("cVideo: Window mode settings changed to "+ string(Video.getWindowMode()?"windowmode":"fullscreen"), cLog::eLOG_TYPE_DEBUG);
  
  if(bApply)
  {
    return applySettings();
  }
  return 0;
}

void cVideo::draw(void)
{
    //TODO: add sanity check to redraw function
  SDL_BlitSurface(buffer,NULL,screen,NULL);
  
  if(getWindowMode())
  {
    SDL_UpdateRect(screen,0,0,0,0);
  }
  else
  {
    SDL_Flip(screen);
  }
}

int cVideo::applySettings(void)
{
  int oldX = MINWIDTH;
  int oldY = MINHEIGHT;
  int oldDepth = COLOURDEPTH;
  Uint32 oldSurface = SDL_SWSURFACE;
  
  if(screen != NULL)
  {
    oldX = screen->w;
    oldY = screen->h;
    oldDepth = screen->format->BitsPerPixel;
    oldSurface = screen->flags;
  }
  
  Log.write("cVideo: Applying new video settings", cLog::eLOG_TYPE_DEBUG);
  screen = SDL_SetVideoMode(getResolutionX(),getResolutionY(),getColDepth(),getSurfaceType()|(getWindowMode()?0:SDL_FULLSCREEN));
  
  if ( screen == NULL )
  {
    Log.write("cVideo:  => Failed. Reverting!", cLog::eLOG_TYPE_ERROR);
    Log.write(SDL_GetError(), cLog::eLOG_TYPE_ERROR);
    setResolution(oldX, oldY, false);
    setColDepth(oldDepth);
    //TODO: Don't know how to reset fullscreen mode proper from old values
    Log.write("cVideo: TODO: Don't know how to reset fullscreen mode proper from old values.", cLog::eLOG_TYPE_WARNING);
    screen = SDL_SetVideoMode(oldX,oldY,oldDepth,oldSurface);
    draw();
    return -1;
  }
  else
  {
    buffer = SDL_SetVideoMode(getResolutionX(),getResolutionY(),getColDepth(),getSurfaceType()|(getWindowMode()?0:SDL_FULLSCREEN));
  }
  //TODO: noticed problems: main menu, hud borders upper, right, lower need to redraw, alpha blending buffer still to small
  draw();
  bWasResized = true;
  return 0;
  
}

bool cVideo::wasResized(void)
{
  return bWasResized;
}

void cVideo::resetResized(void)
{
  bWasResized = false;
}

void cVideo::clearBuffer(void)
{
  SDL_FillRect ( buffer,NULL,SDL_MapRGB (buffer->format, 0, 0, 0) );
}

/**
 *Shows splashscreen
 */
void cVideo::initSplash(void)
{	
	if(putenv( cVideoPos)!=0) //set window to center of screen.
	{
		Log.write("cVideo: Couldn't export SDL_VIDEO_CENTERED", cLog::eLOG_TYPE_WARNING);
	}

	buffer = LoadPCX(SPLASH_BACKGROUND);
	SDL_WM_SetIcon(AutoSurface(SDL_LoadBMP(MAXR_ICON)), NULL);
	
	//BEGIN VERSION STRING
	string sVersion = PACKAGE_NAME; sVersion += " ";
	sVersion += PACKAGE_VERSION; sVersion += " ";
	sVersion += PACKAGE_REV; sVersion += " ";
	SDL_WM_SetCaption ( sVersion.c_str(), 0 );
	//END VERSION STRING
	
	screen=SDL_SetVideoMode ( getSplashW(), getSplashH(), getColDepth(), getSurfaceType()|SDL_NOFRAME );
	draw();
}

bool cVideo::getWindowMode(void)
{
  return videoData.bWindowMode;
}

int cVideo::getResolutionX(void)
{
  return videoData.width;
}

int cVideo::getResolutionY(void)
{
  return videoData.height;
}

void cVideo::setSurfaceType(Uint32 iSurfaceType)
{
  //TODO: set all surfaces to new surface type
  if(videoData.iSurfaceType != iSurfaceType)
  {
    Log.write("cVideo: Surface type "+getSurfaceName(getSurfaceType())+" overwritten with "+getSurfaceName(iSurfaceType), cLog::eLOG_TYPE_INFO);
  }
  videoData.iSurfaceType=iSurfaceType;
}

Uint32 cVideo::getSurfaceType(void)
{
   return videoData.iSurfaceType;
}

int cVideo::getVideoNum (void)
{
	return sizeof(videoModes)/sizeof(sVidMode);
}

string cVideo::getVideoMode(int iMode)
{
  string sTmp = iToStr(getMinW())+"x"+iToStr(getMinH()); //if no valid mode is given we return minimal video mode
  bool bFound = false;
  
  for(int i=0; i<vVideoMode.size(); i++)
  {
    if(vVideoMode[i].mode == iMode)
    {
      sTmp = iToStr(vVideoMode[i].width)+"x"+iToStr(vVideoMode[i].height);
      bFound = true;
      break;
    }
  }
  
  if(!bFound)
  {
     Log.write("cVideo: Video mode "+iToStr(iMode)+" not found. Returning default video mode "+sTmp, cLog::eLOG_TYPE_WARNING);
  }

  return sTmp;
}

bool cVideo::doDetection(void)
{
  Log.write("cVideo: Screen resolution detection started. Results may vary!", cLog::eLOG_TYPE_INFO);

  const SDL_VideoInfo *vInfo = SDL_GetVideoInfo();
  SDL_Rect** rDetectedModes;
  vVideoMode.clear();
  
  //detect us some video modes. detection works in fullscreen only. we try HW and SW surfaces
  rDetectedModes = SDL_ListModes(vInfo->vfmt, SDL_FULLSCREEN|getSurfaceType());
  
  if (rDetectedModes == (SDL_Rect**)0)
  {
    switchSurface(); //try SWSURFACE if HWSURFACE doesn't work and vice versa
    rDetectedModes = SDL_ListModes(vInfo->vfmt, SDL_FULLSCREEN|getSurfaceType()); //try with SWSURFACE
    if (rDetectedModes == (SDL_Rect**)0)
    {
      Log.write("cVideo: No video modes detected. Probably bad!", cLog::eLOG_TYPE_ERROR);
      vVideoMode.resize(getVideoNum());
      for(int i=0; i < vVideoMode.size(); i++) //write some default video modes
      {
	Log.write("cVideo: Offering default video mode "+iToStr(i)+" ("+ getVideoMode(i)+")", cLog::eLOG_TYPE_WARNING);
	vVideoMode.at(i) = videoModes[i];
      }
      return false;
    }
    else
    {
      Log.write("cVideo: Detected video modes with "+getSurfaceName(getSurfaceType())+" and "+iToStr(vInfo->vfmt->BitsPerPixel)+" bpp", cLog::eLOG_TYPE_INFO);
    }
  }
  else
  {
    Log.write("cVideo: Detected video modes with "+getSurfaceName(getSurfaceType())+" and "+iToStr(vInfo->vfmt->BitsPerPixel)+" bpp", cLog::eLOG_TYPE_INFO);
  }
  
    /* Print and store detected modes */  
  for (int i=0; rDetectedModes[i]; ++i)
  {
    //write detected video modes (don't write modes *below* the minimum mode'
    if(rDetectedModes[i]->w >= MINWIDTH && rDetectedModes[i]->h >= MINHEIGHT)
    {
      sVidMode tmp = {rDetectedModes[i]->w, rDetectedModes[i]->h, i};
      vVideoMode.push_back (tmp);
      Log.write("cVideo: Offering detected video mode "+iToStr(i)+" ("+ iToStr(rDetectedModes[i]->w)+"x"+iToStr(rDetectedModes[i]->h)+")", cLog::eLOG_TYPE_INFO);
    }
  }

  bHaveMinMode(); //sanity check on minimal needed screen resolution that we need at last
  
  return true;
}

bool cVideo::bHaveMinMode(void)
{
  for(int i=0; i<vVideoMode.size(); i++)
  {
    if(vVideoMode[i].width == MINWIDTH && vVideoMode[i].height == MINHEIGHT)
    {
      return true;
    }
  }
  
  Log.write("cVideo: Minimal needed video mode ("+ iToStr(MINWIDTH)+"x"+iToStr(MINHEIGHT)+") not detected. Probably bad!", cLog::eLOG_TYPE_ERROR);
  return false;
}

int cVideo::validateMode(int iWidth, int iHeight)
{
  for(int i=0; i<vVideoMode.size(); i++)
  {
    if(vVideoMode[i].width == iWidth && vVideoMode[i].height == iHeight)
    {
      return i;
    }
  }
  Log.write("cVideo: Provided video mode ("+ iToStr(iWidth)+"x"+iToStr(iHeight)+") not detected. Resume on own risk!", cLog::eLOG_TYPE_WARNING);
  return -1;
}

int cVideo::getVideoSize (void)
{
  return vVideoMode.size();
}

int cVideo::getSplashW(void)
{
  return SPLASHWIDTH;
}

int cVideo::getSplashH(void)
{
  return SPLASHHEIGHT;
}

int cVideo::getMinW(void)
{
  return MINWIDTH;
}

int cVideo::getMinH(void)
{
  return MINHEIGHT;
}

string cVideo::getSurfaceName (Uint32 iSurfaceType)
{
  if(iSurfaceType == SDL_SWSURFACE) return "SDL_SWSURFACE";
  else if(iSurfaceType == SDL_HWSURFACE) return "SDL_HWSURFACE";
  else return "UNKNOWN";
}

void cVideo::switchSurface(void)
{
  if(getSurfaceType() == SDL_SWSURFACE) setSurfaceType (SDL_HWSURFACE);
  else if(getSurfaceType() == SDL_HWSURFACE) setSurfaceType (SDL_SWSURFACE);
  else Log.write("cVideo: Can't' switch surface types. Unknown surface type "+ iToStr(getSurfaceType())+" detected.", cLog::eLOG_TYPE_ERROR);
}


cVideo Video;
