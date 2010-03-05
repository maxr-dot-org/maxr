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

#include <SDL.h>
#include <string>

using namespace std;

struct sVidmode {
        unsigned int width; 
	unsigned int height;
	unsigned int mode;
};

/**
 * cVideo class. Stores videosettings (and can hopefully operate 'em too one day'):-)
 *
 * @author Bernd "beko" Kosmahl
 */
class cVideo
{
  public:
  cVideo();
  
  /**
  *@return Detected videomodes
  */
  int getVideoSize (void);
  
  /**
  *@param iMode video mode num from video mode array
  *@return Videomode as string widthxheight. If iMode is unknown minimal needed videomode will be returned.
  */
  std::string getVideoMode(int iMode);
  
  /**
  *Try to autodetect avail video modes from SDL. Might fail.
  *@return true on success
  */
  bool doDetection(void);
  
  /**
  *Checks whether our minimal needed videomode has been autodetected
  *@return true if mininal video mode looks valid
  */
  bool bHaveMinMode(void);
  
  /**
  *Check whether the provided mode is known to our video mode list
  *@param width Screenwidth to look for
  *@param height Screenheight to look for
  *@return iMode or -1 on unknown mode
  */
  int validateMode(int width, int height);
  
  /**
  *@return Splash width
  */
  int getSplashW(void);
  
  /**
  *@return Splash height
  */
  int getSplashH(void);
  
  /**
  *@return Minimal needed screen resolution width
  */
  int getMinW(void);
  
  /**
  *@return Minimal needed screen resolution height
  */
  int getMinH(void);
  
  
  private:

  /** Slashscreen width  */
  #define SPLASHWIDTH 500
  /** Slashscreen height  */
  #define SPLASHHEIGHT 420
  /**Minimum video mode resultion we need */
  #define MINWIDTH 640
  #define MINHEIGHT 480
  /**
  *@return Length of videomodes array
  */
  int getVideoNum (void);
  
  

};

extern cVideo Video;

#endif
