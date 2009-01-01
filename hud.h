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
#ifndef hudH
#define hudH
#include "defines.h"
#include <SDL.h>

#include <string>


// Die Hud-Klasse ////////////////////////////////////////////////////////////
class cHud{
public:
  cHud();
  ~cHud();

  bool TNT,MinimapZoom,Nebel,Gitter,Scan,Reichweite,Munition,Treffer,Farben,Status,Studie,Lock;
  bool Praeferenzen;
  bool PlayFLC,PausePressed,PlayPressed;
  bool HelpPressed,ChatPressed,EndePressed;
  bool ErledigenPressed,NextPressed,PrevPressed;
  bool CenterPressed,DateiPressed,LogPressed;
  int Zoom,LastZoom;
  int OffX,OffY;
  bool LastOverEnde;
  int tmpSelectedUnitID;
  int minimapZoomFactor;
  int minimapOffsetX, minimapOffsetY;

  void SwitchTNT(bool set);
  void SwitchMinimapZoom(bool set);
  void SwitchNebel(bool set);
  void SwitchGitter(bool set);
  void SwitchScan(bool set);
  void SwitchReichweite(bool set);
  void SwitchMunition(bool set);
  void SwitchTreffer(bool set);
  void SwitchFarben(bool set);
  void SwitchStatus(bool set);
  void SwitchStudie(bool set);
  void SwitchLock(bool set);
  void DoZoom(int x,int y=274);
  void SetZoom(int zoom,int DestY=274);
  void DoScroll(int dir);
  void DoMinimapClick(int x,int y);
  void PraeferenzenButton(bool set);
  void PauseButton(bool set);
  void PlayButton(bool set);
  void HelpButton(bool set);
  void ChatButton(bool set);
  void LogButton(bool set);
  void EndeButton(bool set);
  void ErledigenButton(bool set);
  void NextButton(bool set);
  void PrevButton(bool set);
  void CenterButton(bool set);
  void DateiButton(bool set);
  void ScaleSurfaces(void);

  void CheckButtons(void);
  void CheckOneClick(void);
  void DoAllHud(void);
  void CheckScroll(bool pure=false);
  void CheckMouseOver(void);
  void ResetVideoMonitor(void);
  void ShowRunde(void);
  void showTurnTime( int iTime );

private:
	/** Wrapper for BlitButton using default Surfaces gfx_hud_stuff for source and gfx_hud for target using normal font
	*@author beko
	*@param scr SDL_Rect for position on source Surface
	*@param dest SDL_Rect for position to copy onto on sfDest
	*@param sText Text to apply on the image
	*@param bPressed button pressed
	*@return 0 on success
	*/
	int BlitButton(SDL_Rect scr, SDL_Rect dest, std::string sText, bool bPressed);

	/** Wrapper for BlitButton using default Surfaces gfx_hud_stuff for source and gfx_hud for target
	*@author beko
	*@param scr SDL_Rect for position on source Surface
	*@param dest SDL_Rect for position to copy onto on sfDest
	*@param sText Text to apply on the image
	*@param bPressed button pressed
	*@param bSmallFont Normal or small font
	*@return 0 on success
	*/
	int BlitButton(SDL_Rect scr, SDL_Rect dest, std::string sText, bool bPressed, bool bSmallFont);

	/** Draws an image onto given surface and attaches given text centered on it
	*@author beko
	*@param *sfSrc SDL_Surface source to get image from
	*@param scr SDL_Rect for position on source Surface
	*@param *sfDest SDL_Surface to copy sfSrc into
	*@param dest SDL_Rect for position to copy onto on sfDest
	*@param sText Text to apply on the image
	*@param bPressed button pressed
	*@param bSmallFont Normal or small font
	*@return 0 on success
	*/
 	int BlitButton(SDL_Surface *sfSrc, SDL_Rect scr, SDL_Surface *sfDest, SDL_Rect dest, std::string sText, bool bPressed, bool bSmallFont);

};

#endif
