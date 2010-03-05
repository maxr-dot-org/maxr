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
#include <vector>


/**
*Will store either our detected videomodes or some generic video modes if auto-detection in doDetect() failed
*/
static vector<sVidmode> vVideoMode;

/**
 * Some possible video modes. Don't use external!'
 */
const static sVidmode videoModes[] =
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


int cVideo::getVideoNum (void)
{
	return sizeof(videoModes)/sizeof(sVidmode);
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
     Log.write("Video mode "+iToStr(iMode)+" not found. Returning default video mode "+sTmp, cLog::eLOG_TYPE_WARNING);
  }

  return sTmp;
  
}

bool cVideo::doDetection(void)
{
  Log.write("Screen resolution detection started. Results may vary!", cLog::eLOG_TYPE_DEBUG);

  const SDL_VideoInfo *vInfo = SDL_GetVideoInfo();
  SDL_Rect** rDetectedModes;
  vVideoMode.clear();
  
  //detect us some video modes. detection works in fullscreen only. we try HW and SW surfaces
  rDetectedModes = SDL_ListModes(vInfo->vfmt, SDL_FULLSCREEN|SDL_HWSURFACE); //try with HWSURFACE
  
  if (rDetectedModes == (SDL_Rect**)0)
  {
    rDetectedModes = SDL_ListModes(vInfo->vfmt, SDL_FULLSCREEN|SDL_SWSURFACE); //try with SWSURFACE
    if (rDetectedModes == (SDL_Rect**)0)
    {
      Log.write("No video modes detected. Probably bad!", cLog::eLOG_TYPE_ERROR);
      vVideoMode.resize(getVideoNum());
      for(int i=0; i < vVideoMode.size(); i++) //write some default video modes
      {
	Log.write("Offering default video mode "+iToStr(i)+" ("+ getVideoMode(i)+")", cLog::eLOG_TYPE_WARNING);
	vVideoMode.at(i) = videoModes[i];
      }
      return false;
    }
    else
    {
      OtherData.iSurface = SDL_SWSURFACE; //FIXME: dirty set of SURFACE type
      Log.write("Detected video modes with SDL_SWSURFACE and "+iToStr(vInfo->vfmt->BitsPerPixel)+" bpp", cLog::eLOG_TYPE_DEBUG);
      
    }
  }
  else
  {
    Log.write("Detected video modes with SDL_HWSURFACE and"+iToStr(vInfo->vfmt->BitsPerPixel)+" bpp", cLog::eLOG_TYPE_DEBUG);
    OtherData.iSurface = SDL_HWSURFACE; //FIXME: dirty set of SURFACE type
  }
  
    /* Print and store detected modes */  
  for (int i=0; rDetectedModes[i]; ++i)
  {
    //write detected video modes (don't write modes *below* the minimum mode'
    if(rDetectedModes[i]->w >= MINWIDTH && rDetectedModes[i]->h >= MINHEIGHT)
    {
      sVidmode tmp = {rDetectedModes[i]->w, rDetectedModes[i]->h, i};
      vVideoMode.push_back (tmp);
      Log.write("Offering detected video mode "+iToStr(i)+" ("+ iToStr(rDetectedModes[i]->w)+"x"+iToStr(rDetectedModes[i]->h)+")", cLog::eLOG_TYPE_DEBUG);
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
  
  Log.write("Minimal needed video mode ("+ iToStr(MINWIDTH)+"x"+iToStr(MINHEIGHT)+") not detected. Probably bad!", cLog::eLOG_TYPE_ERROR);
  return false;
}

int cVideo::validateMode(int width, int height)
{
  for(int i=0; i<vVideoMode.size(); i++)
  {
    if(vVideoMode[i].width == width && vVideoMode[i].height == height)
    {
      return i;
    }
  }
  Log.write("Provided video mode ("+ iToStr(width)+"x"+iToStr(height)+") not detected. Resume on own risk!", cLog::eLOG_TYPE_WARNING);
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
cVideo Video;
