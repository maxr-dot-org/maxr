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
#ifndef dialogH
#define dialogH
#include "defines.h"
#include "main.h"

void ShowDialog(string text,bool bPurgeHud,string path,int SaveLoad=-1);

//void ShowDialogList(cList *list,int offset);

/** Shows localized Yes/No dialog
 * @param text Text to show on button
 * @return true on Yes<br>false on No
 */
bool ShowYesNo(string text);
int ShowNumberInput(string text, int iMaxValue, int iDefaultValue);
 /**
 * Shows dialogbox with localized OK button
 * @param sText Text to show on button
 * @param bPurgeHud Clean hud (infoscreens) and black out gamewindow
 */
void ShowOK(string sText,bool bPurgeHud=false);
/** Shows selfdestruction dialog with safe feature. User has to click twice to blow something up
 * @author beko
 * @return true on blast<br>false on cancel
 */
bool showSelfdestruction(void);
 /** Shows licence infobox refering to hardcoded GPL-notation and warranty information
 * @author beko
 */
void showLicence();
 /** Shows localized preferences dialog
 * @author beko
 */
void showPreferences(void);
 /**
 *
 */
void drawDialogArrow(SDL_Surface *surface, SDL_Rect *dest, int type);
/**
 * Draws a sliderbar with a slider
 * @author beko
 * @param *sfDialog SDL_Surface of dialog for proper redrawing background
 * @param offx
 * @param offy
 * @param value 0 - 255 for sliderposition
 * @param *surface SDL_Surface to draw on
 */
void drawSlider(SDL_Surface *sfDialog, int offx,int offy,int value, SDL_Surface *surface);
/**
 * Draws a checkbox
 * @author beko
 * @param offx
 * @param offy
 * @param set clickstatus
 * @param *surface SDL_Surface to draw on
 */
void drawCheckbox(int offx,int offy,bool set, SDL_Surface *surface);
/** Draws a button<br>
 * Size is 77x23
 * @author beko
 * @param sText Text displayed centered on button
 * @param bPressed clickstatus
 * @param x x position
 * @param y y position
 * @param *surface SDL_Surface to draw on
 */
void drawButton(std::string sText, bool bPressed, int x, int y, SDL_Surface *surface);


enum ARROW_TYPE
{
	ARROW_TYPE_UP = 0,
	ARROW_TYPE_DOWN = 1,
};

#endif
