//////////////////////////////////////////////////////////////////////////////
// M.A.X. - fonts.h
//////////////////////////////////////////////////////////////////////////////
#ifndef fontsH
#define fontsH
#include "defines.h"
#include "sdl.h"

// Die Farben des Small-Fonts ////////////////////////////////////////////////
enum eFontSmallColor{ClWhite,ClRed,ClGreen,ClYellow};

// Die Font-Klasse ///////////////////////////////////////////////////////////
class cFonts{
public:
  cFonts();
  ~cFonts();

  SDL_Rect *chars,*chars_small,*chars_big; // Rects der Zeichen.

  int Charset(char c);
  void OutText(char *str,int x,int y,SDL_Surface *sf);
  int CharsetBig(char c);
  void OutTextBig(char *str,int x,int y,SDL_Surface *sf);
  void OutTextBigCenter(char *str,int x,int y,SDL_Surface *sf);
  void OutTextBigCenterGold(char *str,int x,int y,SDL_Surface *sf);  
  int CharsetSmall(char c);
  void OutTextSmall(char *str,int x,int y,eFontSmallColor color,SDL_Surface *sf);
  void OutTextCenter(char *str,int x,int y,SDL_Surface *sf);
  int GetTextLen(char *str);
  int GetTextLenSmall(char *str);
  void OutTextSmallCenter(char *str,int x,int y,eFontSmallColor color,SDL_Surface *sf);
  void OutTextBlock(char *str,SDL_Rect block,SDL_Surface *sf);  
};

// Das Font-Objekt ///////////////////////////////////////////////////////////
EX cFonts *fonts;

#endif