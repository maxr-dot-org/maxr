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
#ifndef fontsH
#define fontsH

#include <string>

#include "defines.h"
#include "SDL.h"

// Die Farben des Small-Fonts ////////////////////////////////////////////////
enum eFontSmallColor{ClWhite,ClRed,ClGreen,ClYellow};

// Die Font-Klasse ///////////////////////////////////////////////////////////
class cFonts{
public:
  cFonts();
  ~cFonts();

  SDL_Rect *chars,*chars_small,*chars_big; // Rects der Zeichen.

  int Charset(char c);
   /** Wrapper for OutText
  */
  void OutText(std::string str,int x,int y,SDL_Surface *sf);
  void OutText(const char *str,int x,int y,SDL_Surface *sf);
  int CharsetBig(char c);
  /** Wrapper for OutTextBig
  */
  void OutTextBig(std::string str,int x,int y,SDL_Surface *sf);
  void OutTextBig(char *str,int x,int y,SDL_Surface *sf);
  /** Wrapper for OutTextBigCenter
  */
  void OutTextBigCenter(std::string str,int x,int y,SDL_Surface *sf);
  void OutTextBigCenter(char *str,int x,int y,SDL_Surface *sf);
  /** Wrapper for OutTextBigCenterGold
  */
  void OutTextBigCenterGold(std::string str,int x,int y,SDL_Surface *sf);  
  void OutTextBigCenterGold(char *str,int x,int y,SDL_Surface *sf);  
  int CharsetSmall(char c);
  /** Wrapper for OutTextSmall
  */
void OutTextSmall (std::string str,int x,int y,eFontSmallColor color,SDL_Surface *sf );
  void OutTextSmall(const char *str,int x,int y,eFontSmallColor color,SDL_Surface *sf);
  /** Wrapper for OutTextCenter
  */
  void OutTextCenter(std::string str,int x,int y,SDL_Surface *sf);
  void OutTextCenter(const char *str,int x,int y,SDL_Surface *sf);
  int GetTextLen(const char *str);
  int GetTextLenSmall(char *str);
  void OutTextSmallCenter(char *str,int x,int y,eFontSmallColor color,SDL_Surface *sf);
  void OutTextBlock(char *str,SDL_Rect block,SDL_Surface *sf);  
};

// Das Font-Objekt ///////////////////////////////////////////////////////////
EX cFonts *fonts;

#endif
