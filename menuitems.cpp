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

#include <sstream>
#include <cmath>
#include <algorithm>
#include "menuitems.h"
#include "menus.h"
#include "settings.h"
#include "client.h"

namespace
{
	std::string plural(int n, const std::string &sing, const std::string &plu)
	{
		std::stringstream ss;
		ss << n << " ";
		ss << lngPack.i18n(n == 1 ? sing : plu);
		return ss.str();
	}
	
	Uint32 getPlayerColour(cPlayer *p)
	{
		return ((Uint32*)(p->color->pixels))[0];
	}
	
	void plot(SDL_Surface *s, int x, int y, Uint32 colour)
	{
		SDL_Rect rect = {x, y, 1, 1};
		SDL_FillRect(s, &rect, colour); 
	}
	
	void drawLine(SDL_Surface *s, int x0, int y0, int x1, int y1, Uint32 colour)
	{
		bool steep = abs(y1 - y0) > abs(x1 - x0);
		if(steep) {
			std::swap(x0, y0);
			std::swap(x1, y1);
		}
		if(x0 > x1) {
			std::swap(x0, x1);
			std::swap(y0, y1);
		}
		
		int dx = x1 - x0;
		int dy = abs(y1 - y0);
		int er = dx / 2;
		int ys = y0 < y1 ? 1 : -1;
		int y = y0;
		
		for(int x=x0; x<x1; x++)
		{
			if(steep) plot(s, y, x, colour);
			else plot(s, x, y, colour);
			er -= dy;
			if(er < 0) {
				y += ys;
				er += dx;
			}
		}
	}
	
	int extrapolateScore(const cPlayer *p, int turn)
	{
		const int now = Client->getTurn();
		
		if(turn <= now)
			return p->getScore(turn);
		else
			return p->getScore(now) + p->numEcos * (turn - now);
	}
}

cMenuItem::cMenuItem ( int x, int y )
{
	position.x = x;
	position.y = y;
	position.w = position.h = 0;

	isClicked = false;
	wasClicked = false;
	locked = false;
	disabled = false;

	active = false;

	click = NULL;
	release = NULL;
	hoverOn = NULL;
	hoverAway = NULL;
	moveMouseOver = NULL;
	wasKeyInput = NULL;

	clickSound = NULL;
	releaseSound = NULL;
}

bool cMenuItem::overItem( int x, int y ) const
{
	if ( x >= position.x && x < position.x+position.w &&
		y >= position.y && y < position.y+position.h )
	{
		return true;
	}
	return false;
}

void cMenuItem::clicked(void *parent)
{
	if ( locked ) return;
	isClicked = true;
	if ( preClicked() )
	{
		if ( clickSound ) PlayFX ( clickSound );
		if ( click ) click(parent);
	}
}

void cMenuItem::released(void *parent)
{
	if ( locked ) return;
	if ( preReleased() )
	{
		if ( releaseSound ) PlayFX ( releaseSound );
		if ( release ) release( parent );
		postReleased();
		isClicked = false;
		wasClicked = false;
	}
}

bool cMenuItem::preReleased()
{
	if ( isClicked || wasClicked ) return true;
	else return false;
}

void cMenuItem::hoveredOn(void *parent)
{
	if ( locked ) return;
	if ( wasClicked ) isClicked = true;
	if ( preHoveredOn() && hoverOn ) hoverOn(parent);
	wasClicked = false;
}

void cMenuItem::hoveredAway(void *parent)
{
	if ( locked ) return;
	if ( isClicked ) wasClicked = true;
	if ( preHoveredAway() && hoverAway ) hoverAway(parent);
	isClicked = false;
}

void cMenuItem::movedMouseOver( int lastMouseX, int lastMouseY, void *parent )
{
	if ( moveMouseOver ) moveMouseOver( parent );
}

void cMenuItem::somewhereReleased()
{
	if ( locked ) return;
	isClicked = false;
	wasClicked = false;
}

void cMenuItem::move ( int x, int y )
{
	position.x = x;
	position.y = y;
}

void cMenuItem::setLocked( bool locked_ )
{
	if ( preSetLocked( locked_ ) )
	{
		locked = locked_;
		isClicked = locked;
	}
}

void cMenuItem::setDisabled( bool disabled_ )
{
	disabled = disabled_;
}

void cMenuItem::setClickSound ( sSOUND *clickSound_ )
{
	clickSound = clickSound_;
}

void cMenuItem::setReleaseSound ( sSOUND *releaseSound_ )
{
	releaseSound = releaseSound_;
}

void cMenuItem::setClickedFunction ( void (*click_)(void *) )
{
	click = click_;
}

void cMenuItem::setReleasedFunction ( void (*release_)(void *) )
{
	release = release_;
}

void cMenuItem::setMovedOverFunction ( void (*moveMouseOver_)(void *) )
{
	moveMouseOver = moveMouseOver_;
}

void cMenuItem::setWasKeyInputFunction ( void (*wasKeyInput_)(void *) )
{
	wasKeyInput = wasKeyInput_;
}

bool cMenuItem::getIsClicked()
{
	return isClicked;
}

cMenuItemContainer::cMenuItemContainer( int x, int y ) : cMenuItem ( x, y )
{
	position.w = 0;
	position.h = 0;
}

void cMenuItemContainer::draw()
{
	for ( unsigned int i = 0; i < itemList.Size() ; i++ )
	{
		itemList[i]->draw();
	}
}

void cMenuItemContainer::clicked ( void *parent )
{
	preClicked();
	for ( unsigned int i = 0; i < itemList.Size() ; i++ )
	{
		if ( itemList[i]->overItem ( mouse->x, mouse->y ) ) itemList[i]->clicked( this );
	}
}

void cMenuItemContainer::released ( void *parent )
{
	for ( unsigned int i = 0; i < itemList.Size() ; i++ )
	{
		if ( itemList[i]->overItem ( mouse->x, mouse->y ) ) itemList[i]->released( this );
		else itemList[i]->somewhereReleased();
	}
}

void cMenuItemContainer::hoveredOn ( void *parent )
{
	for ( unsigned int i = 0; i < itemList.Size() ; i++ )
	{
		if ( itemList[i]->overItem( mouse->x, mouse->y ) ) itemList[i]->hoveredOn( this );
	}
}

void cMenuItemContainer::hoveredAway ( void *parent )
{
	for ( unsigned int i = 0; i < itemList.Size() ; i++ )
	{
		itemList[i]->hoveredAway( this );
	}
}

void cMenuItemContainer::movedMouseOver( int lastMouseX, int lastMouseY, void *parent )
{
	for ( unsigned int i = 0; i < itemList.Size() ; i++ )
	{
		if ( itemList[i]->overItem( lastMouseX, lastMouseY ) && !itemList[i]->overItem( mouse->x, mouse->y ) ) itemList[i]->hoveredAway( this );
		else if ( !itemList[i]->overItem( lastMouseX, lastMouseY ) && itemList[i]->overItem( mouse->x, mouse->y ) ) itemList[i]->hoveredOn( this );
		else if ( itemList[i]->overItem( lastMouseX, lastMouseY ) && itemList[i]->overItem( mouse->x, mouse->y ) ) itemList[i]->movedMouseOver( lastMouseX, lastMouseY, this );
	}
}

void cMenuItemContainer::somewhereReleased ()
{
	for ( unsigned int i = 0; i < itemList.Size() ; i++ )
	{
		itemList[i]->somewhereReleased();
	}
}

void cMenuItemContainer::addItem ( cMenuItem* item )
{
	itemList.Add( item );
	if ( item->position.x == 0 && item->position.y == 0 && item->position.w == 0 && item->position.h == 0 ) return;
	if ( item->position.x < position.x )
	{
		position.w = position.x+position.w-item->position.x;
		position.x = item->position.x;
	}
	if ( item->position.y < position.y )
	{
		position.w = position.y+position.h-item->position.y;
		position.y = item->position.y;
	}
	if ( item->position.x+item->position.w > position.x+position.w ) position.w = item->position.x+item->position.w-position.x;
	if ( item->position.y+item->position.h > position.y+position.h ) position.h = item->position.y+item->position.h-position.y;
}

void cMenuItemContainer::removeItem ( cMenuItem* item )
{
	for ( unsigned int i = 0; i < itemList.Size() ; i++ )
	{
		if ( item == itemList[i] )
		{
			itemList.Delete( i );
			break;
		}
	}
	// TODO: renew position
}


cMenuTimerBase::cMenuTimerBase(Uint32 intervall) :
	state(false)
{
	timerID = SDL_AddTimer(intervall, &sdlTimerCallback, this);
}

cMenuTimerBase::~cMenuTimerBase()
{
	SDL_RemoveTimer(timerID);
}

bool cMenuTimerBase::getState()
{
	if ( state )
	{
		state = false;
		return true;
	}
	return false;
}

Uint32 cMenuTimerBase::sdlTimerCallback(Uint32 intervall, void* param)
{
	cMenuTimerBase *timer = (cMenuTimerBase*) param;
	timer->state = true;

	return intervall;
}


cMenuImage::cMenuImage(int x, int y, SDL_Surface* image_) : cMenuItem(x, y)
{
	setImage ( image_ );
}

void cMenuImage::setImage(SDL_Surface *image_)
{
	if ( image_ != NULL )
	{
		image = SDL_CreateRGBSurface( OtherData.iSurface | SDL_SRCCOLORKEY, image_->w, image_->h, SettingsData.iColourDepth, 0, 0, 0, 0 );

		SDL_FillRect ( image, NULL, 0xFF00FF );
		SDL_SetColorKey ( image, SDL_SRCCOLORKEY, 0xFF00FF );

		SDL_BlitSurface ( image_, NULL, image, NULL );

		position.w = image->w;
		position.h = image->h;
	}
	else
	{
		image = NULL;
		position.w = position.h = 0;
	}
}

void cMenuImage::draw()
{
	if ( image )
	{
		SDL_BlitSurface ( image, NULL, buffer, &position );
	}
}

cMenuLabel::cMenuLabel(int x, int y, string text_, eUnicodeFontType fontType_) :
	cMenuItem(x, y),
	fontType(fontType_),
	flagCentered(false),
	flagBox(false)
{
	textPosition.x = position.x;
	textPosition.y = position.y;
	textPosition.w = textPosition.h = 0;
	setText ( text_ );
}

void cMenuLabel::setText( string text_ )
{
	text = text_;
	if ( !flagBox )
	{
		if ( text.length() > 0 )
		{
			position.w = textPosition.w = font->getTextWide ( text, fontType );
			position.h = textPosition.h = font->getFontHeight ( fontType );
		}
		else position.w = position.h = textPosition.w = textPosition.h = 0;
		setCentered ( flagCentered );
	}
}

void cMenuLabel::setFontType ( eUnicodeFontType fontType_ )
{
	fontType = fontType_;
}

void cMenuLabel::setCentered ( bool centered )
{
	flagCentered = centered;
	if ( flagCentered ) position.x = textPosition.x-textPosition.w/2;
	else position.x = textPosition.x;
}

void cMenuLabel::setBox( int width, int height )
{
	position.w = width;
	position.h = height;
	flagBox = true;
}

void cMenuLabel::draw()
{
	if ( flagCentered ) font->showTextCentered ( textPosition, text, fontType );
	else if ( flagBox ) font->showTextAsBlock ( position, text, fontType );
	else font->showText ( textPosition, text, fontType );
}

cMenuButton::cMenuButton(int x, int y, string text_, eButtonTypes buttonType_, eUnicodeFontType fontType_, sSOUND* clickSound_) :
	cMenuItem(x, y),
	text(text_),
	fontType(fontType_),
	buttonType(buttonType_)
{
	clickSound = clickSound_;
	renewButtonSurface();
}

void cMenuButton::renewButtonSurface()
{
	SDL_Rect src;
	switch ( buttonType )
	{
	default:
	case BUTTON_TYPE_STANDARD_BIG:
		src.x = 0;
		src.y = isClicked ? 29 : 0;
		position.w = src.w = 200;
		position.h = src.h = 29;
		break;
	case BUTTON_TYPE_STANDARD_SMALL:
		src.x = 0;
		src.y = isClicked ? 87 : 58;
		position.w = src.w = 150;
		position.h = src.h = 29;
		break;
	case BUTTON_TYPE_HUGE:
		src.x = isClicked ? 109 : 0;
		src.y = 116;
		position.w = src.w = 109;
		position.h = src.h = 40;
		break;
	case BUTTON_TYPE_ARROW_UP_BIG:
		src.x = isClicked ? 125 : 97;
		src.y = 157;
		position.w = src.w = 28;
		position.h = src.h = 29;
		break;
	case BUTTON_TYPE_ARROW_DOWN_BIG:
		src.x = isClicked ? 181 : 153;
		src.y = 157;
		position.w = src.w = 28;
		position.h = src.h = 29;
		break;
	case BUTTON_TYPE_ARROW_LEFT_BIG:
		src.x = isClicked ? 293 : 265;
		src.y = 157;
		position.w = src.w = 28;
		position.h = src.h = 29;
		break;
	case BUTTON_TYPE_ARROW_RIGHT_BIG:
		src.x = isClicked ? 237 : 209;
		src.y = 157;
		position.w = src.w = 28;
		position.h = src.h = 29;
		break;
	case BUTTON_TYPE_ARROW_UP_SMALL:
		position.w = src.w = 18;
		position.h = src.h = 17;
		src.x = isClicked ? 151+src.w : 151;
		src.y = 59;
		break;
	case BUTTON_TYPE_ARROW_DOWN_SMALL:
		position.w = src.w = 18;
		position.h = src.h = 17;
		src.x = isClicked ? 187+src.w : 187;
		src.y = 59;
		break;
	case BUTTON_TYPE_ARROW_LEFT_SMALL:
		position.w = src.w = 18;
		position.h = src.h = 17;
		src.x = isClicked ? 151+src.w : 151;
		src.y = 76;
		break;
	case BUTTON_TYPE_ARROW_RIGHT_SMALL:
		position.w = src.w = 18;
		position.h = src.h = 17;
		src.x = isClicked ? 187+src.w : 187;
		src.y = 76;
		break;
	case BUTTON_TYPE_ARROW_UP_BAR:
		position.w = src.w = 17;
		position.h = src.h = 17;
		src.x = isClicked ? 201+src.w : 201;
		src.y = 1;
		break;
	case BUTTON_TYPE_ARROW_DOWN_BAR:
		position.w = src.w = 17;
		position.h = src.h = 17;
		src.x = isClicked ? 201+src.w : 201;
		src.y = 18;
		break;
	case BUTTON_TYPE_ANGULAR:
		position.w = src.w = 78;
		position.h = src.h = 23;
		src.x = isClicked ? src.w : 0;
		src.y = 196;
		break;
	case BUTTON_TYPE_HUD_HELP:
		position.w = src.w = 26;
		position.h = src.h = 24;
		src.x = isClicked ? 366 : 268;
		src.y = isClicked ? 0 : 151;
		break;
	case BUTTON_TYPE_HUD_CENTER:
		position.w = src.w = 21;
		position.h = src.h = 22;
		src.x = isClicked ? 0 : 139;
		src.y = isClicked ? 21 : 149;
		break;
	case BUTTON_TYPE_HUD_REPORT:
		position.w = src.w = 49;
		position.h = src.h = 20;
		src.x = isClicked ? 210 : 245;
		src.y = isClicked ? 21 : 130;
		break;
	case BUTTON_TYPE_HUD_CHAT:
		position.w = src.w = 49;
		position.h = src.h = 20;
		src.x = isClicked ? 160 : 196;
		src.y = isClicked ? 21 : 129;
		break;
	case BUTTON_TYPE_HUD_NEXT:
		position.w = src.w = 39;
		position.h = src.h = 23;
		src.x = isClicked ? 288 : 158;
		src.y = isClicked ? 0 : 172;
		break;
	case BUTTON_TYPE_HUD_PREV:
		position.w = src.w = 38;
		position.h = src.h = 23;
		src.x = isClicked ? 327 : 198;
		src.y = isClicked ? 0 : 172;
		break;
	case BUTTON_TYPE_HUD_DONE:
		position.w = src.w = 26;
		position.h = src.h = 24;
		src.x = isClicked ? 262 : 132;
		src.y = isClicked ? 0 : 172;
		break;
	case BUTTON_TYPE_HUD_END:
		position.w = src.w = 70;
		position.h = src.h = 17;
		src.x = isClicked ? 22 : 0;
		src.y = isClicked ? 21 : 151;
		break;
	case BUTTON_TYPE_HUD_PREFERENCES:
		position.w = src.w = 67;
		position.h = src.h = 20;
		src.x = isClicked ? 195 : 0;
		src.y = isClicked ? 0 : 169;
		break;
	case BUTTON_TYPE_HUD_FILES:
		position.w = src.w = 67;
		position.h = src.h = 20;
		src.x = isClicked ? 93 : 71;
		src.y = isClicked ? 21 : 151;
		break;
	case BUTTON_TYPE_HUD_PLAY:
		position.w = src.w = 19;
		position.h = src.h = 18;
		src.x = isClicked ? 157 : 0;
		src.y = isClicked ? 0 : 132;
		break;
	case BUTTON_TYPE_HUD_STOP:
		position.w = src.w = 19;
		position.h = src.h = 19;
		src.x = isClicked ? 176 : 19;
		src.y = isClicked ? 0 : 132;
		break;
	}
	surface = SDL_CreateRGBSurface ( OtherData.iSurface | SDL_SRCCOLORKEY, src.w, src.h , SettingsData.iColourDepth, 0, 0, 0, 0 );
	SDL_SetColorKey ( surface, SDL_SRCCOLORKEY, 0xFF00FF );
	SDL_FillRect ( surface, NULL, 0xFF00FF );
	if ( buttonType >= BUTTON_TYPE_HUD_HELP && buttonType <= BUTTON_TYPE_HUD_STOP ) SDL_BlitSurface ( GraphicsData.gfx_hud_stuff, &src, surface, NULL );
	else SDL_BlitSurface ( GraphicsData.gfx_menu_stuff, &src, surface, NULL );

	text = font->shortenStringToSize ( text, position.w-getBordersSize(), fontType );
}

void cMenuButton::redraw()
{
	renewButtonSurface();
	draw();
	SHOW_SCREEN
	mouse->draw ( false, screen );
}

int cMenuButton::getTextYOffset()
{
	switch ( buttonType )
	{
	default:
	case BUTTON_TYPE_STANDARD_BIG:
	case BUTTON_TYPE_STANDARD_SMALL:
		return 7;
	case BUTTON_TYPE_HUGE:
		return 11;
	case BUTTON_TYPE_ARROW_UP_BIG:
	case BUTTON_TYPE_ARROW_DOWN_BIG:
	case BUTTON_TYPE_ARROW_LEFT_BIG:
	case BUTTON_TYPE_ARROW_RIGHT_BIG:
	case BUTTON_TYPE_ARROW_UP_SMALL:
	case BUTTON_TYPE_ARROW_DOWN_SMALL:
	case BUTTON_TYPE_ARROW_LEFT_SMALL:
	case BUTTON_TYPE_ARROW_RIGHT_SMALL:
	case BUTTON_TYPE_ARROW_UP_BAR:
	case BUTTON_TYPE_ARROW_DOWN_BAR:
		return -1;
	case BUTTON_TYPE_ANGULAR:
		if ( isClicked || locked ) return 5;
		else return 4;
	case BUTTON_TYPE_HUD_NEXT:
	case BUTTON_TYPE_HUD_PREV:
	case BUTTON_TYPE_HUD_DONE:
		return 9;
	case BUTTON_TYPE_HUD_END:
		if ( isClicked || locked ) return 3;
		else return 2;
	case BUTTON_TYPE_HUD_REPORT:
	case BUTTON_TYPE_HUD_CHAT:
	case BUTTON_TYPE_HUD_PREFERENCES:
	case BUTTON_TYPE_HUD_FILES:
		return 6;
	}
}

int cMenuButton::getBordersSize()
{
	switch ( buttonType )
	{
	case BUTTON_TYPE_HUD_DONE:
		return 0;
	default:
		return 12;
	}

}

bool cMenuButton::preClicked()
{
	redraw();
	return true;
}

bool cMenuButton::preReleased()
{
	if ( isClicked || wasClicked ) return true;
	return false;
}

void cMenuButton::postReleased()
{
	if ( !locked ) isClicked = false;
	redraw();
}

bool cMenuButton::preHoveredOn()
{
	if ( wasClicked ) preClicked();
	return true;
}

bool cMenuButton::preHoveredAway()
{
	if ( isClicked ) postReleased();
	return true;
}

void cMenuButton::draw()
{
	if ( surface )
	{
		SDL_BlitSurface ( surface, NULL, buffer, &position );

		if ( buttonType >= BUTTON_TYPE_HUD_NEXT && buttonType <= BUTTON_TYPE_HUD_FILES )
		{
			if ( isClicked ) font->showTextCentered ( position.x+position.w/2, position.y+getTextYOffset(), text, FONT_LATIN_SMALL_GREEN );
			else font->showTextCentered ( position.x+position.w/2, position.y+getTextYOffset()-1, text, FONT_LATIN_SMALL_RED );
			font->showTextCentered ( position.x+position.w/2-1, position.y+getTextYOffset()-1+(isClicked ? 1 : 0), text, FONT_LATIN_SMALL_WHITE );
		}
		else font->showTextCentered ( position.x+position.w/2, position.y+getTextYOffset(), text, fontType );
	}
}

bool cMenuButton::preSetLocked( bool locked_ )
{
	isClicked = locked_;
	renewButtonSurface();
	return true;
}

cMenuDestroyButton::cMenuDestroyButton(int x, int y, cMenu* menu) : 
	cMenuItem(x, y),
	opening(false),
	glassHeight(0)
{
	position.w = 59;
	position.h = 56;

	setLocked(true);

	timer = new cMenuTimer<cMenuDestroyButton> (*this, &cMenuDestroyButton::animationCallback, 10 );
	menu->addTimer(timer);
}

cMenuDestroyButton::~cMenuDestroyButton()
{
	delete timer;
}

bool cMenuDestroyButton::preClicked()
{
	draw();
	return (glassHeight>=56);
}

bool cMenuDestroyButton::preReleased()
{
	if ( isClicked || wasClicked ) return true;
	return false;
}

void cMenuDestroyButton::postReleased()
{
	if ( !locked ) isClicked = false;
	draw();
}

bool cMenuDestroyButton::preHoveredOn()
{
	if ( wasClicked ) preClicked();
	return true;
}

bool cMenuDestroyButton::preHoveredAway()
{
	if ( isClicked ) postReleased();
	return true;
}

bool cMenuDestroyButton::preSetLocked( bool locked_ )
{
	locked = locked_;
	return false;
}

void cMenuDestroyButton::draw()
{
	if ( isClicked )
	{
		SDL_Rect src = {6, 269, 59, 56};
		SDL_BlitSurface(GraphicsData.gfx_hud_stuff, &src, buffer, &position);
	}
	else
	{
		SDL_Rect src = {15, 13, 59, 56};
		SDL_BlitSurface(GraphicsData.gfx_destruction, &src, buffer, &position);
	}

	if ( glassHeight < 56 )
	{
		SDL_Rect src = { 0, glassHeight, 59, 56 - glassHeight };
		SDL_BlitSurface(GraphicsData.gfx_destruction_glas, &src, buffer, &position);
	}

	SHOW_SCREEN
	mouse->draw ( false, screen );
}

void cMenuDestroyButton::animationCallback()
{
	if ( opening && glassHeight < 56 )
	{
		glassHeight += 1;
		draw();
	
		if ( glassHeight >= 56 )
		{
			setLocked(false);
		}
	}
}

cMenuCheckButton::cMenuCheckButton(int x, int y, string text_, bool checked_, bool centered_, eCheckButtonTypes buttonType_, eCheckButtonTextOriantation textOrientation_, eUnicodeFontType fontType_, sSOUND *clickSound_) :
	cMenuItem(x, y),
	text(text_),
  fontType(fontType_),
  buttonType(buttonType_),
  textOrientation(textOrientation_),
	group(),
	centered(centered_),
	checked(checked_),
	textLimitWight(-1)
{
	clickSound = clickSound_;

	renewButtonSurface();
	if ( centered ) position.x -= position.w/2;
}

void cMenuCheckButton::renewButtonSurface()
{
	SDL_Rect src = {0,0,0,0};

	if ( buttonType >= CHECKBOX_HUD_INDEX_00 && buttonType <= CHECKBOX_HUD_INDEX_22 )
	{
		src.y = 44;
		position.w = src.w = 55;
		position.h = 0;
	}
	switch ( buttonType )
	{
	default:
	case RADIOBTN_TYPE_BTN_ROUND:
		position.w = src.w = 18;
		position.h = src.h = 17;
		src.x = checked ? 151+src.w : 151;
		src.y = 93;
		break;
	case RADIOBTN_TYPE_TEXT_ONLY:
		surface = NULL;
		position.w = font->getTextWide ( text, fontType );
		position.h = font->getFontHeight ( fontType );
		break;
	case CHECKBOX_TYPE_STANDARD:
		position.w = src.w = 18;
		position.h = src.h = 17;
		src.x = checked ? 187+src.w : 187;
		src.y = 93;
		break;
	case CHECKBOX_TYPE_TANK:
		position.w = src.w = 32;
		position.h = src.h = 31;
		src.x = checked ? src.w : 0;
		src.y = 219;
		break;
	case CHECKBOX_TYPE_PLANE:
		position.w = src.w = 32;
		position.h = src.h = 31;
		src.x = checked ? 32*2+src.w : 32*2;
		src.y = 219;
		break;
	case CHECKBOX_TYPE_SHIP:
		position.w = src.w = 32;
		position.h = src.h = 31;
		src.x = checked ? 32*4+src.w : 32*4;
		src.y = 219;
		break;
	case CHECKBOX_TYPE_BUILD:
		position.w = src.w = 32;
		position.h = src.h = 31;
		src.x = checked ? 32*6+src.w : 32*6;
		src.y = 219;
		break;
	case CHECKBOX_TYPE_TNT:
		position.w = src.w = 32;
		position.h = src.h = 31;
		src.x = checked ? 32*8+src.w : 32*8;
		src.y = 219;
		break;
	case RADIOBTN_TYPE_ANGULAR_BUTTON:
		position.w = src.w = 78;
		position.h = src.h = 23;
		src.x = checked ? src.w : 0;
		src.y = 196;
		textLimitWight = position.w-17;
		break;
	case CHECKBOX_HUD_INDEX_22:
		src.x += src.w;
	case CHECKBOX_HUD_INDEX_21:
		src.x += src.w;
	case CHECKBOX_HUD_INDEX_20:
		src.h = position.h = 18;
		src.y += 18+16;
		if ( !checked ) src.x += 167;
		break;
	case CHECKBOX_HUD_INDEX_12:
		src.x += src.w;
	case CHECKBOX_HUD_INDEX_11:
		src.x += src.w;
	case CHECKBOX_HUD_INDEX_10:
		src.h = position.h = 16;
		src.y += 18;
		if ( !checked ) src.x += 167;
		break;
	case CHECKBOX_HUD_INDEX_02:
		src.x += src.w;
	case CHECKBOX_HUD_INDEX_01:
		src.x += src.w;
	case CHECKBOX_HUD_INDEX_00:
		src.h = position.h = 18;
		if ( !checked ) src.x += 167;
		break;
	case CHECKBOX_HUD_LOCK:
		position.w = src.w = 21;
		position.h = src.h = 22;
		src.x = 397;
		src.y = checked ? 298 : 321;
		break;
	case CHECKBOX_HUD_TNT:
		position.w = src.w = 27;
		position.h = src.h = 28;
		src.x = checked ? 362 : 334;
		src.y = 24;
		break;
	case CHECKBOX_HUD_2X:
		position.w = src.w = 27;
		position.h = src.h = 28;
		src.x = checked ? 362 : 334;
		src.y = 53;
		break;
	}
	if ( src.w > 0 )
	{
		surface = SDL_CreateRGBSurface ( OtherData.iSurface, src.w, src.h , SettingsData.iColourDepth, 0, 0, 0, 0 );
		if ( buttonType >= CHECKBOX_HUD_INDEX_00 && buttonType <= CHECKBOX_HUD_2X ) SDL_BlitSurface ( GraphicsData.gfx_hud_stuff, &src, surface, NULL );
		else SDL_BlitSurface ( GraphicsData.gfx_menu_stuff, &src, surface, NULL );
	}

	if ( textLimitWight != -1 ) text = font->shortenStringToSize ( text, textLimitWight, fontType );
}

bool cMenuCheckButton::preClicked()
{
	if ( group )
	{
		checked = true;
		group->checkedButton ( this );
	}
	else
	{
		checked = !checked;
		renewButtonSurface();
	}
	draw();
	SHOW_SCREEN
	mouse->draw ( false, screen );
	return true;
}

void cMenuCheckButton::draw()
{
	SDL_Rect textDest = { -1, -1, -1, -1 };
	if ( surface )
	{
		switch ( textOrientation )
		{
		case TEXT_ORIENT_RIGHT:
			textDest.x = position.x+surface->w+2;
			textDest.y = position.y+(position.h/2)-(font->getFontHeight( fontType )/2);
			break;
		case TEXT_ORIENT_LEFT:
			textDest.x = position.x-(font->getTextWide ( text, fontType )+2);
			textDest.y = position.y+(position.h/2)-(font->getFontHeight( fontType )/2);
			break;
		}
	}

	switch ( buttonType )
	{
	default:
	case CHECKBOX_TYPE_STANDARD:
	case RADIOBTN_TYPE_BTN_ROUND:
		SDL_BlitSurface ( surface, NULL, buffer, &position );
		font->showText ( textDest.x , textDest.y, text, fontType );
		break;
	case CHECKBOX_TYPE_TANK:
	case CHECKBOX_TYPE_PLANE:
	case CHECKBOX_TYPE_SHIP:
	case CHECKBOX_TYPE_BUILD:
	case CHECKBOX_TYPE_TNT:
		SDL_BlitSurface ( surface, NULL, buffer, &position );
		break;
	case RADIOBTN_TYPE_TEXT_ONLY:
		font->showText ( position.x, position.y, text, fontType );
		if ( checked )
		{
#define SELECTION_COLOR 0xE3DACF
			SDL_Rect dest = { position.x+position.w+2, position.y-1, 1, position.h+2 };
			SDL_FillRect ( buffer, &dest, SELECTION_COLOR );
			dest.x -= position.w+4;
			SDL_FillRect ( buffer, &dest, SELECTION_COLOR );
			dest.h = 1;
			dest.w = position.w+5;
			SDL_FillRect ( buffer, &dest, SELECTION_COLOR );
			dest.y += position.h+2;
			SDL_FillRect ( buffer, &dest, SELECTION_COLOR );
		}
		break;
	case RADIOBTN_TYPE_ANGULAR_BUTTON:
		SDL_BlitSurface ( surface, NULL, buffer, &position );
		if ( checked ) font->showTextCentered ( position.x+position.w/2, position.y+5, text, fontType );
		else font->showTextCentered ( position.x+position.w/2, position.y+4, text, fontType );
		break;
	case CHECKBOX_HUD_INDEX_00:
	case CHECKBOX_HUD_INDEX_01:
	case CHECKBOX_HUD_INDEX_02:
		textDest.y = 7;
	case CHECKBOX_HUD_INDEX_10:
	case CHECKBOX_HUD_INDEX_11:
	case CHECKBOX_HUD_INDEX_12:
		if ( textDest.y != 7 ) textDest.y = 6;
	case CHECKBOX_HUD_INDEX_20:
	case CHECKBOX_HUD_INDEX_21:
	case CHECKBOX_HUD_INDEX_22:
		if ( textDest.y != 6 && textDest.y != 7 ) textDest.y = 5;
		SDL_BlitSurface ( surface, NULL, buffer, &position );
		if ( checked ) font->showTextCentered ( position.x+position.w/2, position.y+textDest.y, text, FONT_LATIN_SMALL_GREEN );
		else font->showTextCentered ( position.x+position.w/2, position.y+textDest.y-1, text, FONT_LATIN_SMALL_RED );
		font->showTextCentered ( position.x+position.w/2-1, position.y+textDest.y-1+(checked ? 1 : 0), text, FONT_LATIN_SMALL_WHITE );
		break;
	}
}

void cMenuCheckButton::setChecked ( bool checked_ )
{
	checked = checked_;
	if ( group ) group->checkedButton ( this );
	else renewButtonSurface();
}

bool cMenuCheckButton::isChecked()
{
	return checked;
}

void cMenuCheckButton::limitTextSize ( int w )
{
	textLimitWight = w;
	text = font->shortenStringToSize ( text, textLimitWight, fontType );
}

cMenuRadioGroup::~cMenuRadioGroup()
{
	while ( buttonList.Size() )
	{
		delete buttonList[0];
		buttonList.Delete ( 0 );
	}
}

void cMenuRadioGroup::draw()
{
	for ( unsigned int i = 0; i < buttonList.Size(); i++ )
	{
		buttonList[i]->draw();
	}
}

void cMenuRadioGroup::addButton( cMenuCheckButton* button )
{
	button->group = this;
	buttonList.Add ( button );
}

bool cMenuRadioGroup::buttonIsChecked ( int index )
{
	if ( index >= 0 && index < (int)buttonList.Size() && buttonList[index]->isChecked() )  return true;
	return false;
}

void cMenuRadioGroup::checkedButton( cMenuCheckButton* button )
{
	for ( unsigned int i = 0; i < buttonList.Size(); i++ )
	{
		if ( buttonList[i] != button ) buttonList[i]->checked = false;
		buttonList[i]->renewButtonSurface();
	}
}

bool cMenuRadioGroup::overItem( int x, int y ) const
{
	for ( unsigned int i = 0; i < buttonList.Size(); i++ )
	{
		if ( buttonList[i]->overItem ( x, y ) )
			return true;
	}
	return false;
}

void cMenuRadioGroup::clicked( void *parent )
{
	mouse->GetPos();
	for ( unsigned int i = 0; i < buttonList.Size(); i++ )
	{
		if ( buttonList[i]->overItem ( mouse->x, mouse->y ) )
		{
			buttonList[i]->clicked ( parent );
			//((cMenu*)parent)->draw();
			break;
		}
	}
	if ( click ) click(parent);
}

cMenuUnitListItem::cMenuUnitListItem(sID unitID_, cPlayer* owner_, sUnitUpgrade* upgrades_, eMenuUnitListDisplayTypes displayType_, cMenuUnitsList* parent, bool fixedResValue_) :
	cMenuItem (0, 0),
	displayType(displayType_),
	parentList(parent),
	unitID(unitID_),
	unitData(0),
	owner(owner_),
	upgrades(upgrades_),
	fixedResValue(fixedResValue_)
{
	init ();
}

cMenuUnitListItem::cMenuUnitListItem(sUnitData* unitData_, cPlayer* owner_, sUnitUpgrade* upgrades_, eMenuUnitListDisplayTypes displayType_, cMenuUnitsList* parent, bool fixedResValue_) :
	cMenuItem (0, 0),
	displayType(displayType_),
	parentList(parent),
	unitData(unitData_),
	owner(owner_),
	upgrades(upgrades_),
	fixedResValue(fixedResValue_)
{
	if (unitData != 0)
		unitID = unitData->ID;
	init ();
}

void cMenuUnitListItem::init ()
{
	if ( unitID.getVehicle() )
	{
		sVehicle *vehicle = unitID.getVehicle();
		scaleSurface ( vehicle->img_org[0], vehicle->img[0], vehicle->img_org[0]->w/2, vehicle->img_org[0]->h/2 );
		surface = SDL_CreateRGBSurface ( SDL_SRCCOLORKEY, vehicle->img[0]->w, vehicle->img[0]->h, SettingsData.iColourDepth, 0, 0, 0, 0 );
		SDL_SetColorKey ( surface, SDL_SRCCOLORKEY, 0xFF00FF );
		SDL_BlitSurface ( OtherData.colors[cl_grey], NULL, surface, NULL );
		SDL_BlitSurface ( vehicle->img[0], NULL, surface, NULL );
	}
	else if ( unitID.getBuilding() )
	{
		sBuilding *building = unitID.getBuilding();
		if ( building->data.isBig ) scaleSurface ( building->img_org, building->img, building->img_org->w/4, building->img_org->h/4 );
		else scaleSurface ( building->img_org, building->img, building->img_org->w/2, building->img_org->h/2 );
		surface = SDL_CreateRGBSurface ( SDL_SRCCOLORKEY, building->img->w, building->img->h, SettingsData.iColourDepth, 0, 0, 0, 0 );
		SDL_SetColorKey ( surface, SDL_SRCCOLORKEY, 0xFF00FF );
		if ( building->data.hasPlayerColor ) SDL_BlitSurface ( OtherData.colors[cl_grey], NULL, surface, NULL );
		else SDL_FillRect ( surface, NULL, 0xFF00FF );
		SDL_BlitSurface ( building->img, NULL, surface, NULL );
	}
	else surface = NULL;

	selected = false;
	marked = false;
	resValue = 0;
	minResValue = -1;
	fixed = false;
}

void cMenuUnitListItem::draw()
{
	SDL_Rect src = { 0, 0, 32, 32 };
	SDL_Rect dest = { position.x, position.y, 0, 0 };
	SDL_BlitSurface ( surface, &src, buffer, &dest );

	if ( selected )
	{
		dest.x-=4;
		dest.y-=4;
		dest.h=1;
		dest.w=8;
		SDL_FillRect ( buffer,&dest,0xE0E0E0 );
		dest.x+=30;
		SDL_FillRect ( buffer,&dest,0xE0E0E0 );
		dest.y+=38;
		SDL_FillRect ( buffer,&dest,0xE0E0E0 );
		dest.x-=30;
		SDL_FillRect ( buffer,&dest,0xE0E0E0 );
		dest.y=position.y-4;
		dest.w=1;
		dest.h=8;
		SDL_FillRect ( buffer,&dest,0xE0E0E0 );
		dest.x+=38;
		SDL_FillRect ( buffer,&dest,0xE0E0E0 );
		dest.y+=31;
		SDL_FillRect ( buffer,&dest,0xE0E0E0 );
		dest.x-=38;
		SDL_FillRect ( buffer,&dest,0xE0E0E0 );
	}
	dest.w = position.w-(32+4)-12;

	switch ( displayType )
	{
	case MUL_DIS_TYPE_NOEXTRA:
		drawName( false );
		break;
	case MUL_DIS_TYPE_COSTS:
		{
			drawName( false );

			dest.x = position.x+(32+4);
			dest.y = position.y+12;

			if ( unitID.getVehicle() ) font->showTextCentered( position.x+position.w-12, dest.y, iToStr(unitID.getUnitDataCurrentVersion(owner)->buildCosts), FONT_LATIN_SMALL_YELLOW );
		}
		break;
	default:
	case MUL_DIS_TYPE_CARGO:
		{
			int destY = drawName ( true );
			drawCargo ( destY );
		}
		break;
	}
}


int cMenuUnitListItem::drawName( bool withNumber )
{
	SDL_Rect dest = { position.x+32+4, position.y+12, position.w-(32+4)-12, 0 };
	string name = ((string)unitID.getUnitDataCurrentVersion(owner)->name);
	eUnicodeFontType fontType = marked ? FONT_LATIN_SMALL_RED : FONT_LATIN_SMALL_WHITE;

	if ( withNumber )
	{
		// numerate the unit.
		int nrOfSameUnits = 1;
		// search the landing list for other units of the same type.
		for (int otherUnitIdx = 0; otherUnitIdx < (int)parentList->unitsList.Size(); otherUnitIdx++)
		{
			if ( !parentList->unitsList[otherUnitIdx]->unitID.getVehicle() ) continue;
			if ( parentList->unitsList[otherUnitIdx] == this ) break;
			if ( unitID == parentList->unitsList[otherUnitIdx]->unitID ) nrOfSameUnits++;
		}
		name += " " + iToStr ( nrOfSameUnits );
	}

	// display the name
	if ( font->getTextWide ( name, fontType ) > dest.w )
	{
		dest.y -= font->getFontHeight( fontType ) / 2;
		font->showTextAsBlock ( dest, name, fontType );
		dest.y += font->getFontHeight( fontType ) / 2;
	}
	else font->showText ( dest, name, fontType );

	return dest.y;
}

void cMenuUnitListItem::drawCargo( int destY )
{
	if ( !unitID.getVehicle() ) return;

	SDL_Rect dest = { position.x+32+4, destY, 0, 0 };

	if ( unitID.getUnitDataOriginalVersion()->storeResType != sUnitData::STORE_RES_NONE )
	{
		if ( unitID.getUnitDataOriginalVersion()->storeResType == sUnitData::STORE_RES_GOLD ) return; // don't allow buying gold

		if( resValue == 0 ) font->showText( dest.x, dest.y+10, "(empty)", FONT_LATIN_SMALL_WHITE );
		else if( resValue <= unitID.getUnitDataOriginalVersion()->storageResMax / 4 ) font->showText( dest.x, dest.y+10, " (" + iToStr(resValue) + "/" + iToStr(unitID.getUnitDataOriginalVersion()->storageResMax) + ")", FONT_LATIN_SMALL_RED );
		else if( resValue <= unitID.getUnitDataOriginalVersion()->storageResMax / 2 ) font->showText( dest.x, dest.y+10, " (" + iToStr(resValue) + "/" + iToStr(unitID.getUnitDataOriginalVersion()->storageResMax) + ")", FONT_LATIN_SMALL_YELLOW );
		else font->showText( dest.x, dest.y+10, " (" + iToStr(resValue) + "/" + iToStr(unitID.getUnitDataOriginalVersion()->storageResMax) + ")", FONT_LATIN_SMALL_GREEN );
	}
}

void cMenuUnitListItem::released( void *parent )
{
	if ( releaseSound ) PlayFX ( releaseSound );
	((cMenuUnitsList*)parent)->setSelection ( this );
	((cMenuUnitsList*)parent)->parentMenu->draw();
}

sID cMenuUnitListItem::getUnitID()
{
	return unitID;
}

sUnitData* cMenuUnitListItem::getUnitData()
{
	if ( !unitData ) return unitID.getUnitDataOriginalVersion();
	return unitData;
}

cPlayer *cMenuUnitListItem::getOwner()
{
	return owner;
}

int cMenuUnitListItem::getResValue()
{
	return resValue;
}

bool cMenuUnitListItem::getFixedResValue ()
{
	return fixedResValue;
}

void cMenuUnitListItem::setResValue( int resValue_, bool cargoCheck )
{
	if ( fixedResValue ) return;
	resValue = resValue_;
	if ( resValue < minResValue ) resValue = minResValue;
	if ( cargoCheck && resValue < 0 ) resValue = 0;
	if ( cargoCheck && resValue > unitID.getUnitDataOriginalVersion()->storageResMax ) resValue = unitID.getUnitDataOriginalVersion()->storageResMax;
}

void cMenuUnitListItem::setMinResValue ( int minResValue_ )
{
	minResValue = minResValue_;
	if ( resValue < minResValue ) resValue = minResValue;
}

void cMenuUnitListItem::setMarked ( bool marked_ )
{
	marked = marked_;
}

void cMenuUnitListItem::setFixedResValue ( bool fixedResValue_ )
{
	fixedResValue = fixedResValue_;
}


void cMenuUnitListItem::setFixed ( bool fixed_ )
{
	fixed = fixed_;
}

bool cMenuUnitListItem::getFixedStatus()
{
	return fixed;
}

sUnitUpgrade *cMenuUnitListItem::getUpgrades()
{
	return upgrades;
}


sUnitUpgrade *cMenuUnitListItem::getUpgrade( sUnitUpgrade::eUpgradeTypes type )
{
	if ( !upgrades ) return NULL;
	for ( int i = 0; i < 8; i++ )
	{
		if ( upgrades[i].type == type ) return &upgrades[i];
	}
	return NULL;
}

cMenuUnitsList::cMenuUnitsList( int x, int y, int w, int h, cHangarMenu *parent, eMenuUnitListDisplayTypes displayType_ ) : cMenuItem (x, y), parentMenu(parent), displayType(displayType_)
{
	resize ( w, h );

	doubleClicked = NULL;

	selectedUnit = NULL;
	offset = 0;
}

cMenuUnitsList::~cMenuUnitsList()
{
	while ( unitsList.Size() )
	{
		delete unitsList[0];
		unitsList.Delete( 0 );
	}
}

void cMenuUnitsList::draw()
{
	for ( int i = offset; i < offset+maxDisplayUnits; i++ )
	{
		if ( i >= (int)unitsList.Size() ) break;
		unitsList[i]->position.x = position.x+10;
		unitsList[i]->position.y = position.y+8+(i-offset)*34;
		unitsList[i]->draw();
	}
}

void cMenuUnitsList::released( void *parent )
{
	mouse->GetPos();
	for ( int i = offset; i < offset+maxDisplayUnits; i++ )
	{
		if ( i >= (int)unitsList.Size() ) break;
		if ( unitsList[i]->overItem ( mouse->x, mouse->y ) )
		{
			if ( selectedUnit == unitsList[i] )
			{
				if ( unitsList[i]->releaseSound ) PlayFX ( unitsList[i]->releaseSound );
				if ( !doubleClicked || !doubleClicked( this, parent ) ) unitsList[i]->released( this );
			}
			else unitsList[i]->released( this );
		}
	}
}

int cMenuUnitsList::getSize()
{
	return (int)unitsList.Size();
}

cMenuUnitListItem* cMenuUnitsList::getItem ( int index )
{
	if ( index >= 0 && index < (int)unitsList.Size() ) return unitsList[index];
	else return NULL;
}

cMenuUnitListItem* cMenuUnitsList::getSelectedUnit()
{
	return selectedUnit;
}

void cMenuUnitsList::resize ( int w, int h )
{
	position.w = w;
	position.h = h;

	maxDisplayUnits = (position.h-16)/34;
}

void cMenuUnitsList::setDoubleClickedFunction( bool (*doubleClicked_)(cMenuUnitsList *, void *parent ) )
{
	doubleClicked = doubleClicked_;
}

void cMenuUnitsList::scrollUp()
{
	if ( offset-maxDisplayUnits > 0 ) offset -= maxDisplayUnits;
	else offset = 0;
	parentMenu->draw();
}

void cMenuUnitsList::scrollDown()
{
	if ( offset+maxDisplayUnits < (int)unitsList.Size() )
	{
		offset += maxDisplayUnits;
		if ( offset+maxDisplayUnits > (int)unitsList.Size() && (int)unitsList.Size() > maxDisplayUnits )
		{
			offset = (int)unitsList.Size()-maxDisplayUnits;
		}
	}
	else return;
	parentMenu->draw();
}

void cMenuUnitsList::setSelection ( cMenuUnitListItem *selectedUnit_ )
{
	for ( unsigned int i = 0; i < unitsList.Size(); i++ )
	{
		if ( unitsList[i] == selectedUnit_ )
		{
			selectedUnit = selectedUnit_;
			selectedUnit->selected = true;
		}
		else unitsList[i]->selected = false;
	}
	parentMenu->setSelectedUnit ( selectedUnit );
}

void cMenuUnitsList::addUnit ( cMenuUnitListItem *unitItem, bool scroll )
{
	unitItem->setReleaseSound ( SoundData.SNDObjectMenu );
	unitItem->position.h = 32;
	unitItem->position.w = position.w-20;
	unitsList.Add ( unitItem );
	if ( selectedUnit ) selectedUnit->selected = false;
	selectedUnit = unitItem;
	selectedUnit->selected = true;
	if ( scroll && (int)unitsList.Size() > offset+maxDisplayUnits ) scrollDown();
}

cMenuUnitListItem *cMenuUnitsList::addUnit ( sUnitData *unitData, cPlayer *owner, sUnitUpgrade *upgrades, bool scroll, bool fixedCargo )
{
	cMenuUnitListItem *unitItem = new cMenuUnitListItem( unitData, owner, upgrades, displayType, this, fixedCargo );
	addUnit ( unitItem, scroll );
	return unitItem;
}

cMenuUnitListItem *cMenuUnitsList::addUnit ( sID unitID, cPlayer *owner, sUnitUpgrade *upgrades, bool scroll, bool fixedCargo )
{
	cMenuUnitListItem *unitItem = new cMenuUnitListItem( unitID, owner, upgrades, displayType, this, fixedCargo );
	addUnit ( unitItem, scroll );
	return unitItem;
}

void cMenuUnitsList::removeUnit ( cMenuUnitListItem *item )
{
	for ( unsigned int i = 0; i < unitsList.Size(); i++ )
	{
		if ( unitsList[i] == item )
		{
			cMenuUnitListItem *nextSelUnit = NULL;
			bool isInMenuSelected = false;
			if ( unitsList[i]->selected )
			{
				if ( i+1 < (int)unitsList.Size() ) nextSelUnit = unitsList[i+1];
				else if ( ((int)i)-1 >= 0 ) nextSelUnit = unitsList[i-1];
				if ( unitsList[i] == parentMenu->getSelectedUnit() ) isInMenuSelected = true;
			}
			delete unitsList[i];
			unitsList.Delete ( i );

			selectedUnit = nextSelUnit;
			if ( isInMenuSelected ) parentMenu->setSelectedUnit ( nextSelUnit );
			if ( selectedUnit ) selectedUnit->selected = true;
			if ( offset >= (int)unitsList.Size() ) scrollUp();
			break;
		}
	}
}

void cMenuUnitsList::clear()
{
	parentMenu->setSelectedUnit ( NULL );
	selectedUnit = NULL;
	while ( unitsList.Size() )
	{
		delete unitsList[0];
		unitsList.Delete ( 0 );
	}
	offset = 0;
}

void cMenuUnitsList::setDisplayType ( eMenuUnitListDisplayTypes displayType_ )
{
	displayType = displayType_;
}

void cUnitDataSymbolHandler::drawSymbols ( eUnitDataSymbols symType, int x, int y, int maxX, bool big, int value1, int value2 )
{
	SDL_Rect src;

	int toValue;

	if ( big )
	{
		src = getBigSymbolPosition ( symType );
		maxX -= src.w;

		if ( value2 < value1 ) maxX -= src.w + 3;
		toValue = value1;
	}
	else
	{
		src = getSmallSymbolPosition ( symType );
		toValue = value2;

		if ( symType == MENU_SYMBOLS_HITS )
		{
			if ( value1 <= value2 / 4 ) src.x += src.w*4;
			else if ( value1 <= value2 / 2 ) src.x += src.w*2;
		}
	}

	int offX = src.w;
	int step = 1;

	while ( offX*toValue > maxX )
	{
		offX--;

		if ( offX < 4 )
		{
			toValue /= 2;
			if ( big ) value2 /= 2;
			step *= 2;
			offX = src.w;
		}
	}

	SDL_Rect dest = {x, y, 0, 0};

	int oriSrcX = src.x;
	for ( int i = 0; i < toValue; i++ )
	{
		if ( big && i == value2 )
		{
			SDL_Rect mark;
			dest.x += src.w + 3;
			mark.x = dest.x - src.w / 2;
			mark.y = dest.y;
			mark.w = 1;
			mark.h = src.h;
			SDL_FillRect ( buffer, &mark, 0xFC0000 );
		}

		if ( !big && value1 <= 0 ) src.x = oriSrcX + src.w;

		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff, &src, buffer, &dest );

		dest.x += offX;
		value1 -= step;
	}
}

void cUnitDataSymbolHandler::drawNumber ( int x, int y, int value, int maximalValue )
{
	if ( value > maximalValue / 2 )	font->showTextCentered ( x, y, iToStr ( value ) + "/" + iToStr ( maximalValue ), FONT_LATIN_SMALL_GREEN, buffer );
	else if ( value > maximalValue / 4 ) font->showTextCentered ( x, y, iToStr ( value ) + "/" + iToStr ( maximalValue ), FONT_LATIN_SMALL_YELLOW, buffer );
	else font->showTextCentered ( x, y, iToStr ( value ) + "/" + iToStr ( maximalValue ), FONT_LATIN_SMALL_RED, buffer );
}

SDL_Rect cUnitDataSymbolHandler::getBigSymbolPosition ( eUnitDataSymbols symType )
{
	SDL_Rect src = {0, 109, 0, 0};

	switch ( symType )
	{
		case MENU_SYMBOLS_SPEED:
			src.x = 0;
			src.w = 11;
			src.h = 12;
			break;
		case MENU_SYMBOLS_HITS:
			src.x = 11;
			src.w = 7;
			src.h = 11;
			break;
		case MENU_SYMBOLS_AMMO:
			src.x = 18;
			src.w = 9;
			src.h = 14;
			break;
		case MENU_SYMBOLS_ATTACK:
			src.x = 27;
			src.w = 10;
			src.h = 14;
			break;
		case MENU_SYMBOLS_SHOTS:
			src.x = 37;
			src.w = 15;
			src.h = 7;
			break;
		case MENU_SYMBOLS_RANGE:
			src.x = 52;
			src.w = 13;
			src.h = 13;
			break;
		case MENU_SYMBOLS_ARMOR:
			src.x = 65;
			src.w = 11;
			src.h = 14;
			break;
		case MENU_SYMBOLS_SCAN:
			src.x = 76;
			src.w = 13;
			src.h = 13;
			break;
		case MENU_SYMBOLS_METAL:
			src.x = 89;
			src.w = 12;
			src.h = 15;
			break;
		case MENU_SYMBOLS_OIL:
			src.x = 101;
			src.w = 11;
			src.h = 12;
			break;
		case MENU_SYMBOLS_GOLD:
			src.x = 112;
			src.w = 13;
			src.h = 10;
			break;
		case MENU_SYMBOLS_ENERGY:
			src.x = 125;
			src.w = 13;
			src.h = 17;
			break;
		case MENU_SYMBOLS_HUMAN:
			src.x = 138;
			src.w = 12;
			src.h = 16;
			break;
	}
	return src;
}

SDL_Rect cUnitDataSymbolHandler::getSmallSymbolPosition ( eUnitDataSymbols symType )
{
	SDL_Rect src = {0, 98, 0, 0};
	switch ( symType )
	{
		case MENU_SYMBOLS_SPEED:
			src.x = 0;
			src.w = 7;
			src.h = 7;
			break;
		case MENU_SYMBOLS_HITS:
			src.x = 14;
			src.w = 6;
			src.h = 9;
			break;
		case MENU_SYMBOLS_AMMO:
			src.x = 50;
			src.w = 5;
			src.h = 7;
			break;
		case MENU_SYMBOLS_SHOTS:
			src.x = 88;
			src.w = 8;
			src.h = 4;
			break;
		case MENU_SYMBOLS_METAL:
			src.x = 60;
			src.w = 7;
			src.h = 10;
			break;
		case MENU_SYMBOLS_OIL:
			src.x = 104;
			src.w = 8;
			src.h = 9;
			break;
		case MENU_SYMBOLS_GOLD:
			src.x = 120;
			src.w = 9;
			src.h = 8;
			break;
		case MENU_SYMBOLS_ENERGY:
			src.x = 74;
			src.w = 7;
			src.h = 7;
			break;
		case MENU_SYMBOLS_HUMAN:
			src.x = 170;
			src.w = 8;
			src.h = 9;
			break;
		case MENU_SYMBOLS_TRANS_TANK:
			src.x = 138;
			src.w = 16;
			src.h = 8;
			break;
		case MENU_SYMBOLS_TRANS_AIR:
			src.x = 186;
			src.w = 21;
			src.h = 8;
			break;
	}
	return src;
}

cMenuUnitDetails::cMenuUnitDetails(int x, int y, bool drawLines_, cPlayer* owner_) :
	cMenuItem (x, y),
	owner(owner_),
	drawLines(drawLines_)
{
	position.w = 155;
	position.h = 25;
	vehicle = NULL;
	building = NULL;
}

void cMenuUnitDetails::draw()
{
	if ( drawLines )
	{
		SDL_Rect lineRect = { position.x+2, position.y+14, 153, 1 };
		SDL_FillRect ( buffer ,&lineRect, 0x743904 );
		lineRect.y += 12;
		SDL_FillRect ( buffer ,&lineRect, 0x743904 );
		lineRect.y += 12;
		SDL_FillRect ( buffer ,&lineRect, 0x743904 );
	}

	sUnitData *data;
	cPlayer *unitOwner;
	if ( vehicle )
	{
		data = &vehicle->data;
		unitOwner = vehicle->owner;
	}
	else if ( building )
	{
		data = &building->data;
		unitOwner = building->owner;
	}
	else return;

	// Die Hitpoints anzeigen:
	cUnitDataSymbolHandler::drawNumber ( position.x+23, position.y+6, data->hitpointsCur, data->hitpointsMax );
	font->showText ( position.x+47, position.y+6, lngPack.i18n ( "Text~Hud~Hitpoints" ), FONT_LATIN_SMALL_WHITE, buffer );
	cUnitDataSymbolHandler::drawSymbols ( cUnitDataSymbolHandler::MENU_SYMBOLS_HITS, position.x+80, position.y+3, 70, false, data->hitpointsCur, data->hitpointsMax );

	// Den Speed anzeigen:
	if ( data->speedMax > 0 )
	{
		cUnitDataSymbolHandler::drawNumber ( position.x+23, position.y+30, data->speedCur / 4, data->speedMax / 4 );
		font->showText ( position.x+47, position.y+30, lngPack.i18n ( "Text~Hud~Speed" ), FONT_LATIN_SMALL_WHITE, buffer );
		cUnitDataSymbolHandler::drawSymbols ( cUnitDataSymbolHandler::MENU_SYMBOLS_SPEED, position.x+80, position.y+28, 70, false, data->speedCur / 4, data->speedMax / 4 );
	}

	// additional values
	if(data->canScore)
	{
		int score = building->points;
		int tot = building->owner->getScore(Client->getTurn());
		int turnLim, scoreLim;
		Client->getVictoryConditions(&turnLim, &scoreLim);
		int lim = scoreLim ? scoreLim : tot;
		
		cUnitDataSymbolHandler::drawNumber ( position.x+23, position.y+18, score, score );
		font->showText ( position.x+47, position.y+18, lngPack.i18n ( "Text~Hud~Score" ), FONT_LATIN_SMALL_WHITE, buffer );
		cUnitDataSymbolHandler::drawSymbols ( cUnitDataSymbolHandler::MENU_SYMBOLS_HUMAN, position.x+80, position.y+16, 70, false, score, score);
		
		cUnitDataSymbolHandler::drawNumber ( position.x+23, position.y+30, tot, lim );
		font->showText ( position.x+47, position.y+30, lngPack.i18n ( "Text~Hud~Total" ), FONT_LATIN_SMALL_WHITE, buffer );
		cUnitDataSymbolHandler::drawSymbols ( cUnitDataSymbolHandler::MENU_SYMBOLS_HUMAN, position.x+80, position.y+28, 70, false, tot, lim);
	}
	else if ( ( data->storeResType != sUnitData::STORE_RES_NONE || data->storageUnitsMax > 0 ) && unitOwner == owner )
	{
		font->showText ( position.x+47, position.y+18, lngPack.i18n ( "Text~Hud~Cargo" ), FONT_LATIN_SMALL_WHITE, buffer );

		if ( data->storeResType > 0 )
		{
			if ( building )
			{
				cUnitDataSymbolHandler::drawNumber ( position.x+23, position.y+18, data->storageResCur, data->storageResMax );

				font->showText ( position.x+47, position.y+30, lngPack.i18n ( "Text~Hud~Total" ), FONT_LATIN_SMALL_WHITE, buffer );

				switch ( data->storeResType )
				{
				case sUnitData::STORE_RES_METAL:
					cUnitDataSymbolHandler::drawSymbols ( cUnitDataSymbolHandler::MENU_SYMBOLS_METAL, position.x+80, position.y+15, 70, false, data->storageResCur, data->storageResMax );
					cUnitDataSymbolHandler::drawNumber ( position.x+23, position.y+30, building->SubBase->Metal, building->SubBase->MaxMetal );
					cUnitDataSymbolHandler::drawSymbols ( cUnitDataSymbolHandler::MENU_SYMBOLS_METAL, position.x+80, position.y+27, 70, false, building->SubBase->Metal, building->SubBase->MaxMetal );
					break;
				case sUnitData::STORE_RES_OIL:
					cUnitDataSymbolHandler::drawSymbols ( cUnitDataSymbolHandler::MENU_SYMBOLS_OIL, position.x+80, position.y+15, 70, false, data->storageResCur, data->storageResMax );
					cUnitDataSymbolHandler::drawNumber ( position.x+23, position.y+30, building->SubBase->Oil, building->SubBase->MaxOil );
					cUnitDataSymbolHandler::drawSymbols ( cUnitDataSymbolHandler::MENU_SYMBOLS_OIL, position.x+80, position.y+27, 70, false, building->SubBase->Oil, building->SubBase->MaxOil );
					break;
				case sUnitData::STORE_RES_GOLD:
					cUnitDataSymbolHandler::drawSymbols ( cUnitDataSymbolHandler::MENU_SYMBOLS_GOLD, position.x+80, position.y+16, 70, false, data->storageResCur, data->storageResMax );
					cUnitDataSymbolHandler::drawNumber ( position.x+23, position.y+30, building->SubBase->Gold, building->SubBase->MaxGold );
					cUnitDataSymbolHandler::drawSymbols ( cUnitDataSymbolHandler::MENU_SYMBOLS_GOLD, position.x+80,  position.y+28, 70, false, building->SubBase->Gold, building->SubBase->MaxGold );
					break;
				}
			}
			else
			{
				cUnitDataSymbolHandler::drawNumber ( position.x+23, position.y+18, data->storageResCur, data->storageResMax );
				switch ( data->storeResType )
				{
				case sUnitData::STORE_RES_METAL:
					cUnitDataSymbolHandler::drawSymbols ( cUnitDataSymbolHandler::MENU_SYMBOLS_METAL, position.x+80, position.y+15, 70, false, data->storageResCur, data->storageResMax );
					break;
				case sUnitData::STORE_RES_OIL:
					cUnitDataSymbolHandler::drawSymbols ( cUnitDataSymbolHandler::MENU_SYMBOLS_OIL, position.x+80, position.y+15, 70, false, data->storageResCur, data->storageResMax );
					break;
				case sUnitData::STORE_RES_GOLD:
					cUnitDataSymbolHandler::drawSymbols ( cUnitDataSymbolHandler::MENU_SYMBOLS_GOLD, position.x+80, position.y+15, 70, false, data->storageResCur, data->storageResMax );
					break;
				}
			}
		}
		else
		{
			cUnitDataSymbolHandler::drawNumber ( position.x+23, position.y+18, data->storageUnitsCur, data->storageUnitsMax );

			switch ( data->storeUnitsImageType )
			{
			case sUnitData::STORE_UNIT_IMG_TANK:
			case sUnitData::STORE_UNIT_IMG_SHIP:
				cUnitDataSymbolHandler::drawSymbols ( cUnitDataSymbolHandler::MENU_SYMBOLS_TRANS_TANK, position.x+80, position.y+15, 70, false, data->storageUnitsCur, data->storageUnitsMax );
				break;
			case sUnitData::STORE_UNIT_IMG_PLANE:
				cUnitDataSymbolHandler::drawSymbols ( cUnitDataSymbolHandler::MENU_SYMBOLS_TRANS_AIR, position.x+80, position.y+15, 70, false, data->storageUnitsCur, data->storageUnitsMax );
				break;
			case sUnitData::STORE_UNIT_IMG_HUMAN:
				cUnitDataSymbolHandler::drawSymbols ( cUnitDataSymbolHandler::MENU_SYMBOLS_HUMAN, position.x+80, position.y+15, 70, false, data->storageUnitsCur, data->storageUnitsMax );
				break;
			}
		}
	}
	else if ( data->canAttack && !data->explodesOnContact )
	{
		if ( unitOwner == owner )
		{
			// Munition:
			cUnitDataSymbolHandler::drawNumber ( position.x+23, position.y+18, data->ammoCur, data->ammoMax );
			font->showText ( position.x+47, position.y+18, lngPack.i18n ( "Text~Hud~AmmoShort" ), FONT_LATIN_SMALL_WHITE, buffer );
			cUnitDataSymbolHandler::drawSymbols ( cUnitDataSymbolHandler::MENU_SYMBOLS_AMMO, position.x+80, position.y+16, 70, false, data->ammoCur, data->ammoMax );
		}

		// shots
		cUnitDataSymbolHandler::drawNumber ( position.x+23, position.y+41, data->shotsCur, data->shotsMax );
		font->showText ( position.x+47, position.y+41, lngPack.i18n ( "Text~Hud~Shots" ), FONT_LATIN_SMALL_WHITE, buffer );
		cUnitDataSymbolHandler::drawSymbols ( cUnitDataSymbolHandler::MENU_SYMBOLS_SHOTS, position.x+80, position.y+41, 70, false, data->shotsCur, data->shotsMax );
	}
	else if ( data->produceEnergy && building )
	{
		// EnergieProduktion:
		cUnitDataSymbolHandler::drawNumber ( position.x+23, position.y+18, ( building->IsWorking ? data->produceEnergy : 0 ), data->produceEnergy );
		font->showText ( position.x+47, position.y+18, lngPack.i18n ( "Text~Hud~Energy" ), FONT_LATIN_SMALL_WHITE, buffer );
		cUnitDataSymbolHandler::drawSymbols ( cUnitDataSymbolHandler::MENU_SYMBOLS_ENERGY, position.x+80, position.y+16, 70, false, ( building->IsWorking ? data->produceEnergy : 0 ), data->produceEnergy );

		if ( unitOwner == owner )
		{
			// Gesammt:
			font->showText ( position.x+47, position.y+30, lngPack.i18n ( "Text~Hud~Total" ), FONT_LATIN_SMALL_WHITE, buffer );
			cUnitDataSymbolHandler::drawNumber ( position.x+23, position.y+30, building->SubBase->EnergyProd, building->SubBase->MaxEnergyProd );
			cUnitDataSymbolHandler::drawSymbols ( cUnitDataSymbolHandler::MENU_SYMBOLS_ENERGY, position.x+80,  position.y+28, 70, false, building->SubBase->EnergyProd, building->SubBase->MaxEnergyProd );

			// Verbrauch:
			font->showText ( position.x+47,  position.y+41, lngPack.i18n ( "Text~Hud~Usage" ), FONT_LATIN_SMALL_WHITE, buffer );
			cUnitDataSymbolHandler::drawNumber ( position.x+23,  position.y+41, building->SubBase->EnergyNeed, building->SubBase->MaxEnergyNeed );
			cUnitDataSymbolHandler::drawSymbols ( cUnitDataSymbolHandler::MENU_SYMBOLS_ENERGY, position.x+80,  position.y+41, 70, false, building->SubBase->EnergyNeed, building->SubBase->MaxEnergyNeed );
		}
	}
	else if ( data->produceHumans && building )
	{
		// HumanProduktion:
		cUnitDataSymbolHandler::drawNumber ( position.x+23, position.y+18, data->produceHumans, data->produceHumans );
		font->showText ( position.x+47, position.y+18, lngPack.i18n ( "Text~Hud~Teams" ), FONT_LATIN_SMALL_WHITE, buffer );
		cUnitDataSymbolHandler::drawSymbols ( cUnitDataSymbolHandler::MENU_SYMBOLS_HUMAN, position.x+80, position.y+16, 70, false, data->produceHumans, data->produceHumans );

		if ( unitOwner == owner )
		{
			// Gesammt:
			font->showText ( position.x+47, position.y+30, lngPack.i18n ( "Text~Hud~Total" ), FONT_LATIN_SMALL_WHITE, buffer );
			cUnitDataSymbolHandler::drawNumber ( position.x+23, position.y+30, building->SubBase->HumanProd, building->SubBase->HumanProd );
			cUnitDataSymbolHandler::drawSymbols ( cUnitDataSymbolHandler::MENU_SYMBOLS_HUMAN, position.x+80,  position.y+28, 70, false, building->SubBase->HumanProd, building->SubBase->HumanProd );

			// Verbrauch:
			font->showText ( position.x+47,  position.y+41, lngPack.i18n ( "Text~Hud~Usage" ), FONT_LATIN_SMALL_WHITE, buffer );
			cUnitDataSymbolHandler::drawNumber ( position.x+23,  position.y+41, building->SubBase->HumanNeed, building->SubBase->MaxHumanNeed );
			cUnitDataSymbolHandler::drawSymbols ( cUnitDataSymbolHandler::MENU_SYMBOLS_HUMAN, position.x+80, position.y+39, 70, false, building->SubBase->HumanNeed, building->SubBase->MaxHumanNeed );
		}
	}
	else if ( data->needsHumans && building )
	{
		// HumanNeed:
		if ( building->IsWorking )
		{
			cUnitDataSymbolHandler::drawNumber ( position.x+23, position.y+18, data->needsHumans, data->needsHumans );
			font->showText ( position.x+47, position.y+18, lngPack.i18n ( "Text~Hud~Usage" ), FONT_LATIN_SMALL_WHITE, buffer );
			cUnitDataSymbolHandler::drawSymbols ( cUnitDataSymbolHandler::MENU_SYMBOLS_HUMAN, position.x+80, position.y+16, 70, false, data->needsHumans, data->needsHumans );
		}
		else
		{
			cUnitDataSymbolHandler::drawNumber ( position.x+23, position.y+18, 0, data->needsHumans );
			font->showText ( position.x+47, position.y+18, lngPack.i18n ( "Text~Hud~Usage" ), FONT_LATIN_SMALL_WHITE, buffer );
			cUnitDataSymbolHandler::drawSymbols ( cUnitDataSymbolHandler::MENU_SYMBOLS_HUMAN, position.x+80, position.y+16, 70, false, 0, data->needsHumans );
		}

		if ( unitOwner == owner )
		{
			// Gesammt:
			font->showText ( position.x+47, position.y+30, lngPack.i18n ( "Text~Hud~Total" ), FONT_LATIN_SMALL_WHITE, buffer );
			cUnitDataSymbolHandler::drawNumber ( position.x+23, position.y+30, building->SubBase->HumanNeed, building->SubBase->MaxHumanNeed );
			cUnitDataSymbolHandler::drawSymbols ( cUnitDataSymbolHandler::MENU_SYMBOLS_HUMAN, position.x+80,  position.y+28, 70, false, building->SubBase->HumanNeed, building->SubBase->MaxHumanNeed );
		}
	}
}

void cMenuUnitDetails::setOwner ( cPlayer *owner_ )
{
	owner = owner_;
}

void cMenuUnitDetails::setSelection( cVehicle *vehicle_, cBuilding *building_ )
{
	vehicle = vehicle_;
	building = building_;
}

cMenuUnitDetailsBig::cMenuUnitDetailsBig( int x, int y ) : cMenuItem (x,y)
{
	position.w = 246;
	position.h = 176;
	selectedUnit = NULL;
}

void cMenuUnitDetailsBig::draw()
{
	if ( !selectedUnit ) return;
	sUnitData *data = selectedUnit->getUnitData ();
	if (data == 0)
		data = selectedUnit->getUnitID().getUnitDataCurrentVersion ( selectedUnit->getOwner() );
	sUnitData *oriData = selectedUnit->getUnitID().getUnitDataOriginalVersion (selectedUnit->getOwner ());

#define DETAIL_COLUMN_1 dest.x+27
#define DETAIL_COLUMN_2 dest.x+42
#define DETAIL_COLUMN_3 dest.x+95
#define DETAIL_DOLINEBREAK dest.y = y + 14; SDL_FillRect ( buffer, &dest, 0xFC0000 ); y += 19;

	SDL_Rect dest = { position.x, position.y, position.w, 1 };
	int y;
	y = dest.y;

	sUnitUpgrade *upgrade = NULL;

	if ( data->canAttack )
	{
		// Damage:
		upgrade = selectedUnit->getUpgrade ( sUnitUpgrade::UPGRADE_TYPE_DAMAGE );
		font->showTextCentered ( DETAIL_COLUMN_1, y, iToStr ( upgrade ? upgrade->curValue : data->damage ) );
		font->showText ( DETAIL_COLUMN_2, y, lngPack.i18n ( "Text~Vehicles~Damage" ) );
		cUnitDataSymbolHandler::drawSymbols ( cUnitDataSymbolHandler::MENU_SYMBOLS_ATTACK, DETAIL_COLUMN_3 , y - 3, 160, true, upgrade ? upgrade->curValue : data->damage, oriData->damage );
		DETAIL_DOLINEBREAK

		if ( !data->explodesOnContact )
		{
			// Shots:
			upgrade = selectedUnit->getUpgrade ( sUnitUpgrade::UPGRADE_TYPE_SHOTS );
			font->showTextCentered ( DETAIL_COLUMN_1, y, iToStr ( upgrade ? upgrade->curValue : data->shotsMax ) );
			font->showText ( DETAIL_COLUMN_2, y, lngPack.i18n ( "Text~Vehicles~Shoots" ) );
			cUnitDataSymbolHandler::drawSymbols ( cUnitDataSymbolHandler::MENU_SYMBOLS_SHOTS, DETAIL_COLUMN_3, y + 2, 160, true, upgrade ? upgrade->curValue : data->shotsMax, oriData->shotsMax );
			DETAIL_DOLINEBREAK

			// Range:
			upgrade = selectedUnit->getUpgrade ( sUnitUpgrade::UPGRADE_TYPE_RANGE );
			font->showTextCentered ( DETAIL_COLUMN_1, y, iToStr ( upgrade ? upgrade->curValue : data->range ) );
			font->showText ( DETAIL_COLUMN_2, y, lngPack.i18n ( "Text~Vehicles~Range" ) );
			cUnitDataSymbolHandler::drawSymbols ( cUnitDataSymbolHandler::MENU_SYMBOLS_RANGE, DETAIL_COLUMN_3, y - 2, 160, true, upgrade ? upgrade->curValue : data->range, oriData->range );
			DETAIL_DOLINEBREAK

			// Ammo:
			upgrade = selectedUnit->getUpgrade ( sUnitUpgrade::UPGRADE_TYPE_AMMO );
			font->showTextCentered ( DETAIL_COLUMN_1, y, iToStr ( upgrade ? upgrade->curValue : data->ammoMax ) );
			font->showText ( DETAIL_COLUMN_2, y, lngPack.i18n ( "Text~Vehicles~Ammo" ) );
			cUnitDataSymbolHandler::drawSymbols ( cUnitDataSymbolHandler::MENU_SYMBOLS_AMMO, DETAIL_COLUMN_3, y - 2, 160, true, upgrade ? upgrade->curValue : data->ammoMax, oriData->ammoMax );
			DETAIL_DOLINEBREAK
		}
	}

	sUnitData::eStorageResType transport;
	if ( selectedUnit->getUnitID().getVehicle() ) transport = data->storeResType;
	else transport = data->storeResType;

	if ( transport != sUnitData::STORE_RES_NONE )
	{
		font->showTextCentered ( DETAIL_COLUMN_1, y, iToStr ( data->storageResMax ) );
		font->showText ( DETAIL_COLUMN_2, y, lngPack.i18n ( "Text~Vehicles~Cargo" ) );

		switch ( transport )
		{
			case sUnitData::STORE_RES_METAL:
				cUnitDataSymbolHandler::drawSymbols ( cUnitDataSymbolHandler::MENU_SYMBOLS_METAL, DETAIL_COLUMN_3 , y - 2, 160, true, data->storageResMax, oriData->storageResMax );
				break;
			case sUnitData::STORE_RES_OIL:
				cUnitDataSymbolHandler::drawSymbols ( cUnitDataSymbolHandler::MENU_SYMBOLS_OIL, DETAIL_COLUMN_3 , y - 2, 160, true, data->storageResMax, oriData->storageResMax );
				break;
			case sUnitData::STORE_RES_GOLD:
				cUnitDataSymbolHandler::drawSymbols ( cUnitDataSymbolHandler::MENU_SYMBOLS_GOLD, DETAIL_COLUMN_3 , y - 2, 160, true, data->storageResMax, oriData->storageResMax );
				break;
		}

		DETAIL_DOLINEBREAK
	}

	if ( data->produceEnergy )
	{
		// Eneryproduction:
		font->showTextCentered ( DETAIL_COLUMN_1, y, iToStr ( data->produceEnergy ) );
		font->showText ( DETAIL_COLUMN_2, y, lngPack.i18n ( "Text~Vehicles~Produce" ) );
		cUnitDataSymbolHandler::drawSymbols ( cUnitDataSymbolHandler::MENU_SYMBOLS_ENERGY, DETAIL_COLUMN_3, y - 2, 160, true, data->produceEnergy, oriData->produceEnergy );
		DETAIL_DOLINEBREAK

		// Oil consumption:
		font->showTextCentered ( DETAIL_COLUMN_1, y, iToStr ( data->needsOil ) );
		font->showText ( DETAIL_COLUMN_2, y, lngPack.i18n ( "Text~Vehicles~Usage" ) );
		cUnitDataSymbolHandler::drawSymbols ( cUnitDataSymbolHandler::MENU_SYMBOLS_OIL, DETAIL_COLUMN_3, y - 2, 160, true, data->needsOil, oriData->needsOil );
		DETAIL_DOLINEBREAK
	}

	if ( data->produceHumans )
	{
		// Humanproduction:
		font->showText ( DETAIL_COLUMN_1, y, iToStr ( data->produceHumans ) );
		font->showText ( DETAIL_COLUMN_2, y, lngPack.i18n ( "Text~Vehicles~Produce" ) );
		cUnitDataSymbolHandler::drawSymbols ( cUnitDataSymbolHandler::MENU_SYMBOLS_HUMAN, DETAIL_COLUMN_3, y - 2, 160, true, data->produceHumans, oriData->produceHumans );
		DETAIL_DOLINEBREAK
	}

	// Armor:
	upgrade = selectedUnit->getUpgrade ( sUnitUpgrade::UPGRADE_TYPE_ARMOR );
	font->showTextCentered ( DETAIL_COLUMN_1, y, iToStr ( upgrade ? upgrade->curValue : data->armor ) );
	font->showText ( DETAIL_COLUMN_2, y, lngPack.i18n ( "Text~Vehicles~Armor" ) );
	cUnitDataSymbolHandler::drawSymbols ( cUnitDataSymbolHandler::MENU_SYMBOLS_ARMOR, DETAIL_COLUMN_3, y - 2, 160, true, upgrade ? upgrade->curValue : data->armor, oriData->armor );
	DETAIL_DOLINEBREAK

	// Hitpoints:
	upgrade = selectedUnit->getUpgrade ( sUnitUpgrade::UPGRADE_TYPE_HITS );
	font->showTextCentered ( DETAIL_COLUMN_1, y, iToStr ( upgrade ? upgrade->curValue : data->hitpointsMax ) );
	font->showText ( DETAIL_COLUMN_2, y, lngPack.i18n ( "Text~Vehicles~Hitpoints" ) );
	cUnitDataSymbolHandler::drawSymbols ( cUnitDataSymbolHandler::MENU_SYMBOLS_HITS, DETAIL_COLUMN_3 , y - 1, 160, true, upgrade ? upgrade->curValue : data->hitpointsMax, oriData->hitpointsMax );
	DETAIL_DOLINEBREAK

	// Scan:
	if ( data->scan )
	{
		upgrade = selectedUnit->getUpgrade ( sUnitUpgrade::UPGRADE_TYPE_SCAN );
		font->showTextCentered ( DETAIL_COLUMN_1, y, iToStr ( upgrade ? upgrade->curValue : data->scan ) );
		font->showText ( DETAIL_COLUMN_2, y, lngPack.i18n ( "Text~Vehicles~Scan" ) );
		cUnitDataSymbolHandler::drawSymbols ( cUnitDataSymbolHandler::MENU_SYMBOLS_SCAN, DETAIL_COLUMN_3 , y - 2, 160, true, upgrade ? upgrade->curValue : data->scan, oriData->scan );
		DETAIL_DOLINEBREAK
	}

	// Speed:
	if ( data->speedMax )
	{
		upgrade = selectedUnit->getUpgrade ( sUnitUpgrade::UPGRADE_TYPE_SPEED );
		font->showTextCentered ( DETAIL_COLUMN_1, y, iToStr ( (upgrade ? upgrade->curValue : data->speedMax) / 4 ) ); //FIXME: might crash if e.g. speedMax = 3
		font->showText ( DETAIL_COLUMN_2, y, lngPack.i18n ( "Text~Vehicles~Speed" ) );
		cUnitDataSymbolHandler::drawSymbols ( cUnitDataSymbolHandler::MENU_SYMBOLS_SPEED, DETAIL_COLUMN_3 , y - 2, 160, true, (upgrade ? upgrade->curValue : data->speedMax) / 4, oriData->speedMax / 4 );
		DETAIL_DOLINEBREAK
	}

	// energy consumption:
	if ( data->needsEnergy )
	{
		font->showTextCentered ( DETAIL_COLUMN_1, y, iToStr ( data->needsEnergy ) );
		font->showText ( DETAIL_COLUMN_2, y, lngPack.i18n ( "Text~Vehicles~Usage" ) );
		cUnitDataSymbolHandler::drawSymbols ( cUnitDataSymbolHandler::MENU_SYMBOLS_ENERGY, DETAIL_COLUMN_3, y - 2, 160, true, data->needsEnergy, oriData->needsEnergy );
		DETAIL_DOLINEBREAK
	}

	// humans needed:
	if ( data->needsHumans )
	{
		font->showTextCentered ( DETAIL_COLUMN_1, y, iToStr ( data->needsHumans ) );
		font->showText ( DETAIL_COLUMN_2, y, lngPack.i18n ( "Text~Vehicles~Usage" ) );
		cUnitDataSymbolHandler::drawSymbols ( cUnitDataSymbolHandler::MENU_SYMBOLS_HUMAN, DETAIL_COLUMN_3, y - 2, 160, true, data->needsHumans, oriData->needsHumans );
		DETAIL_DOLINEBREAK
	}

	// raw material consumption:
	if ( data->needsMetal )
	{
		font->showTextCentered ( DETAIL_COLUMN_1, y, iToStr ( data->needsMetal ) );
		font->showText ( DETAIL_COLUMN_2, y, lngPack.i18n ( "Text~Vehicles~Usage" ) );
		cUnitDataSymbolHandler::drawSymbols ( cUnitDataSymbolHandler::MENU_SYMBOLS_METAL, DETAIL_COLUMN_3, y - 2, 160, true, data->needsMetal, oriData->needsMetal );
		DETAIL_DOLINEBREAK
	}

	// gold consumption:
	if ( data->convertsGold )
	{
		font->showTextCentered ( DETAIL_COLUMN_1, y, iToStr ( data->convertsGold ) );
		font->showText ( DETAIL_COLUMN_2, y, lngPack.i18n ( "Text~Vehicles~Usage" ) );
		cUnitDataSymbolHandler::drawSymbols ( cUnitDataSymbolHandler::MENU_SYMBOLS_GOLD, DETAIL_COLUMN_3, y - 2, 160, true, data->convertsGold, oriData->convertsGold );
		DETAIL_DOLINEBREAK
	}

	// Costs:
	font->showTextCentered ( DETAIL_COLUMN_1, y, iToStr ( data->buildCosts ) );
	font->showText ( DETAIL_COLUMN_2, y, lngPack.i18n ( "Text~Vehicles~Costs" ) );
	cUnitDataSymbolHandler::drawSymbols ( cUnitDataSymbolHandler::MENU_SYMBOLS_METAL, DETAIL_COLUMN_3 , y - 2, 160, true, data->buildCosts, oriData->buildCosts );
}

void cMenuUnitDetailsBig::setSelection(cMenuUnitListItem *selectedUnit_)
{
	selectedUnit = selectedUnit_;
	draw();
}

cMenuMaterialBar::cMenuMaterialBar( int x, int y, int labelX, int labelY, int maxValue_, eMaterialBarTypes materialType_, bool inverted_, bool showLabel_ ) : cMenuItem ( x, y )
{
	setReleaseSound ( SoundData.SNDObjectMenu );
	currentValue = maxValue = maxValue_;
	inverted = inverted_;
	showLabel = showLabel_;
	valueLabel = new cMenuLabel ( labelX, labelY, iToStr( currentValue ) );
	valueLabel->setCentered ( true );

	setType ( materialType_ );
}

cMenuMaterialBar::~cMenuMaterialBar()
{
	delete valueLabel;
}

void cMenuMaterialBar::setType(eMaterialBarTypes materialType_)
 {
	if( surface && materialType == materialType_ ) return;

	materialType = materialType_;

	switch ( materialType )
	{
	default:
	case MAT_BAR_TYPE_METAL:
	case MAT_BAR_TYPE_OIL:
	case MAT_BAR_TYPE_GOLD:
		position.w = 20;
		position.h = 115;
		horizontal = false;
		break;
	case MAT_BAR_TYPE_METAL_HORI_BIG:
	case MAT_BAR_TYPE_OIL_HORI_BIG:
	case MAT_BAR_TYPE_GOLD_HORI_BIG:
	case MAT_BAR_TYPE_NONE_HORI_BIG:
		position.w = 240;
		position.h = 30;
		horizontal = true;
		break;
	case MAT_BAR_TYPE_METAL_HORI_SMALL:
	case MAT_BAR_TYPE_OIL_HORI_SMALL:
	case MAT_BAR_TYPE_GOLD_HORI_SMALL:
		position.w = 223;
		position.h = 16;
		horizontal = true;
		break;
	}

	generateSurface();
}

void cMenuMaterialBar::generateSurface()
{
	SDL_Rect src = { 114, 336, position.w, position.h };
	surface = SDL_CreateRGBSurface ( OtherData.iSurface | SDL_SRCCOLORKEY, src.w, src.h , SettingsData.iColourDepth, 0, 0, 0, 0 );
	SDL_SetColorKey ( surface, SDL_SRCCOLORKEY, 0xFF00FF );
	SDL_FillRect ( surface, NULL, 0xFF00FF );

	switch ( materialType )
	{
	case MAT_BAR_TYPE_METAL_HORI_BIG:
		src.x = 156;
		src.y = 339;
		break;
	case MAT_BAR_TYPE_OIL_HORI_BIG:
		src.x = 156;
		src.y = 369;
		break;
	case MAT_BAR_TYPE_GOLD_HORI_BIG:
		src.x = 156;
		src.y = 400;
		break;
	case MAT_BAR_TYPE_NONE_HORI_BIG:
		src.x = 156;
		src.y = 307;
		break;
	case MAT_BAR_TYPE_METAL_HORI_SMALL:
		src.x = 156;
		src.y = 256;
		break;
	case MAT_BAR_TYPE_OIL_HORI_SMALL:
		src.x = 156;
		src.y = 273;
		break;
	case MAT_BAR_TYPE_GOLD_HORI_SMALL:
		src.x = 156;
		src.y = 290;
		break;
	default:
	case MAT_BAR_TYPE_METAL:
		src.x = 114+src.w+1;
		src.y = 336;
		break;
	case MAT_BAR_TYPE_OIL:
		src.x = 400;
		src.y = 348;
		break;
	case MAT_BAR_TYPE_GOLD:
		src.x = 114;
		src.y = 336;
		break;
	}

	SDL_BlitSurface ( GraphicsData.gfx_hud_stuff, &src, surface, NULL );
}

void cMenuMaterialBar::draw()
{
	if ( currentValue <= 0 && maxValue <= 0 ) return;
	SDL_Rect src;
	src.h = horizontal ? surface->h : (int)((float)currentValue/maxValue*surface->h );
	src.w = horizontal ? (int)((float)currentValue/maxValue*surface->w ) : surface->w;
	src.x = horizontal ? surface->w-src.w : 0;
	src.y = 0;
	SDL_Rect dest;
	dest.h = dest.w = 0;
	dest.x = position.x;
	dest.y = position.y + ( horizontal ? 0 : surface->h-src.h );

	if ( inverted && horizontal )
	{
		dest.x += surface->w-src.w;
		src.x = 0;
	}

	SDL_BlitSurface ( surface, &src, buffer, &dest );

	if ( showLabel ) valueLabel->draw();
}

void cMenuMaterialBar::setMaximalValue( int maxValue_ )
{
	maxValue = maxValue_;
}

void cMenuMaterialBar::setCurrentValue( int currentValue_ )
{
	currentValue = currentValue_;
	valueLabel->setText( iToStr ( currentValue ) );
}

SDL_Rect cMenuMaterialBar::getPosition()
{
	return position;
}

cMenuUpgradeHandler::cMenuUpgradeHandler ( int x, int y, cUpgradeHangarMenu *parent ) : cMenuItemContainer ( x, y ), parentMenu(parent)
{
	for ( int i = 0; i < 8; i++ )
	{
		decreaseButtons[i] = new cMenuButton ( position.x , position.y+19*i, "", cMenuButton::BUTTON_TYPE_ARROW_LEFT_SMALL, FONT_LATIN_NORMAL, SoundData.SNDObjectMenu );
		decreaseButtons[i]->setLocked ( true );
		decreaseButtons[i]->setReleasedFunction ( &buttonReleased );
		addItem ( decreaseButtons[i] );

		increaseButtons[i] = new cMenuButton ( position.x+18, position.y+19*i, "", cMenuButton::BUTTON_TYPE_ARROW_RIGHT_SMALL, FONT_LATIN_NORMAL, SoundData.SNDObjectMenu );
		increaseButtons[i]->setLocked ( true );
		increaseButtons[i]->setReleasedFunction ( &buttonReleased );
		addItem ( increaseButtons[i] );

		costsLabel[i] = new cMenuLabel ( position.x+40, position.y+2+19*i );
		addItem ( costsLabel[i] );
	}

	selection = NULL;
}

cMenuUpgradeHandler::~cMenuUpgradeHandler()
{
	for ( int i = 0; i < 8; i++ )
	{
		delete decreaseButtons[i];
		delete increaseButtons[i];
		delete costsLabel[i];
	}
}


void cMenuUpgradeHandler::buttonReleased( void* parent )
{
	cMenuUpgradeHandler *This = (cMenuUpgradeHandler *)parent;
	if ( !This->selection ) return;

	sUnitUpgrade *upgrades = This->selection->getUpgrades();
	cPlayer *owner = This->selection->getOwner();

	for ( int i = 0; i < 8; i++ )
	{
		cUpgradeCalculator::UpgradeTypes upgradeType = This->getUpgradeType ( upgrades[i] );
		cUpgradeCalculator& uc = cUpgradeCalculator::instance();

		if ( This->increaseButtons[i]->overItem ( mouse->x, mouse->y ) )
		{
			This->parentMenu->setCredits (  This->parentMenu->getCredits()-upgrades[i].nextPrice );

			if ( upgradeType != cUpgradeCalculator::kSpeed)
			{
				upgrades[i].curValue += uc.calcIncreaseByUpgrade ( upgrades[i].startValue);
				upgrades[i].nextPrice = uc.calcPrice ( upgrades[i].curValue, upgrades[i].startValue, upgradeType, owner->researchLevel);
			}
			else
			{
				upgrades[i].curValue += 4 * uc.calcIncreaseByUpgrade ( upgrades[i].startValue / 4 );
				upgrades[i].nextPrice = uc.calcPrice ( upgrades[i].curValue / 4, upgrades[i].startValue / 4, upgradeType, owner->researchLevel );
			}

			upgrades[i].purchased++;

			This->setSelection ( This->selection );
			This->parentMenu->draw();
		}
		else if ( This->decreaseButtons[i]->overItem ( mouse->x, mouse->y ) )
		{
			if ( upgradeType != cUpgradeCalculator::kSpeed)
			{
				upgrades[i].curValue -= uc.calcIncreaseByUpgrade ( upgrades[i].startValue);
				upgrades[i].nextPrice = uc.calcPrice ( upgrades[i].curValue, upgrades[i].startValue, upgradeType, owner->researchLevel);
			}
			else
			{
				upgrades[i].curValue -= 4 * uc.calcIncreaseByUpgrade ( upgrades[i].startValue / 4 );
				upgrades[i].nextPrice = uc.calcPrice ( upgrades[i].curValue / 4, upgrades[i].startValue / 4, upgradeType, owner->researchLevel );
			}

			This->parentMenu->setCredits (  This->parentMenu->getCredits()+upgrades[i].nextPrice );

			upgrades[i].purchased--;

			This->setSelection ( This->selection );
			This->parentMenu->draw();
		}
	}
}

void cMenuUpgradeHandler::setSelection ( cMenuUnitListItem *selection_ )
{
	selection = selection_;
	if ( !selection )
	{
		for ( int i = 0; i < 8; i++ )
		{
			increaseButtons[i]->setLocked ( true );
			decreaseButtons[i]->setLocked ( true );
			costsLabel[i]->setText ( "" );
		}
		return;
	}
	sUnitUpgrade *upgrade = selection->getUpgrades();
	for ( int i = 0; i < 8; i++ )
	{
		if ( upgrade[i].type != sUnitUpgrade::UPGRADE_TYPE_NONE && upgrade[i].nextPrice != cUpgradeCalculator::kNoPriceAvailable )
			costsLabel[i]->setText ( iToStr (upgrade[i].nextPrice) );
		else
			costsLabel[i]->setText ( "" );

		if ( upgrade[i].type != sUnitUpgrade::UPGRADE_TYPE_NONE && parentMenu->getCredits() >= upgrade[i].nextPrice && upgrade[i].nextPrice != cUpgradeCalculator::kNoPriceAvailable )
			increaseButtons[i]->setLocked ( false );
		else
			increaseButtons[i]->setLocked ( true );

		if ( upgrade[i].type != sUnitUpgrade::UPGRADE_TYPE_NONE && upgrade[i].purchased > 0 )
			decreaseButtons[i]->setLocked ( false );
		else
			decreaseButtons[i]->setLocked ( true );
	}
}

cUpgradeCalculator::UpgradeTypes cMenuUpgradeHandler::getUpgradeType( sUnitUpgrade upgrade )
{
	switch ( upgrade.type )
	{
	case sUnitUpgrade::UPGRADE_TYPE_DAMAGE:
		return cUpgradeCalculator::kAttack;
	case sUnitUpgrade::UPGRADE_TYPE_SHOTS:
		return cUpgradeCalculator::kShots;
	case sUnitUpgrade::UPGRADE_TYPE_RANGE:
		return cUpgradeCalculator::kRange;
	case sUnitUpgrade::UPGRADE_TYPE_AMMO:
		return cUpgradeCalculator::kAmmo;
	case sUnitUpgrade::UPGRADE_TYPE_ARMOR:
		return cUpgradeCalculator::kArmor;
	case sUnitUpgrade::UPGRADE_TYPE_HITS:
		return cUpgradeCalculator::kHitpoints;
	case sUnitUpgrade::UPGRADE_TYPE_SCAN:
		return cUpgradeCalculator::kScan;
	case sUnitUpgrade::UPGRADE_TYPE_SPEED:
		return cUpgradeCalculator::kSpeed;
	}
	// default
	return cUpgradeCalculator::kAttack;
}

cMenuScroller::cMenuScroller(int x, int y, eMenuScrollerTypes scrollerType_, cMenuItem* parent_, void (*movedCallback_)(void*)) :
	cMenuItem(x, y),
	parent(parent_),
	scrollerType(scrollerType_),
	movedCallback(movedCallback_)
{
	SDL_Rect src;
	src.y = 35;
	switch ( scrollerType )
	{
	case SCROLLER_TYPE_VERT:
		src.x = 201;
		src.w = 17;
		src.h = 14;
		break;
	case SCROLLER_TYPE_HORI:
		src.x = 218;
		src.w = 14;
		src.h = 17;
		break;
	case SCROLLER_TYPE_HUD_ZOOM:
		src.x = 132;
		src.y = 0;
		src.w = 25;
		src.h = 15;
		break;
	}
	position.w = src.w;
	position.h = src.h;
	mouseXOff = mouseYOff = 0;

	surface = SDL_CreateRGBSurface( OtherData.iSurface | SDL_SRCCOLORKEY, position.w, position.h, SettingsData.iColourDepth, 0, 0, 0, 0 );
	SDL_FillRect ( surface, NULL, 0xFF00FF );
	if ( scrollerType == SCROLLER_TYPE_HUD_ZOOM ) SDL_BlitSurface ( GraphicsData.gfx_hud_stuff, &src, surface, NULL );
	else SDL_BlitSurface ( GraphicsData.gfx_menu_stuff, &src, surface, NULL );
	SDL_SetColorKey ( surface, SDL_SRCCOLORKEY, 0xFF00FF );
}

void cMenuScroller::draw()
{
	SDL_BlitSurface ( surface, NULL, buffer, &position );
}

bool cMenuScroller::preClicked()
{
	mouseXOff = mouse->x - position.x;
	mouseYOff = mouse->y - position.y;
	return true;
}

void cMenuScroller::hoveredAway(void *parent)
{
	if ( locked ) return;
	if ( preHoveredAway() && hoverAway ) hoverAway(parent);
}

void cMenuScroller::movedMouseOver( int lastMouseX, int lastMouseY, void *parent )
{
	if ( !isClicked ) return;
	mouseMoved( false );
}

void cMenuScroller::somewhereMoved()
{
	if ( !isClicked ) return;
	mouseMoved( false );
}


void cMenuScroller::mouseMoved( bool center )
{
	switch ( scrollerType )
	{
	case SCROLLER_TYPE_VERT:
		position.y = mouse->y - (center ? (position.h/2) : mouseYOff);
		break;
	case SCROLLER_TYPE_HUD_ZOOM:
	case SCROLLER_TYPE_HORI:
		position.x = mouse->x - (center ? (position.w/2) : mouseXOff);
		break;
	}
	if ( movedCallback ) movedCallback( parent );
}

SDL_Rect cMenuScroller::getPosition()
{
	return position;
}

void cMenuScroller::move ( int value )
{
	switch ( scrollerType )
	{
	case SCROLLER_TYPE_VERT:
		position.y = value;
		break;
	case SCROLLER_TYPE_HUD_ZOOM:
	case SCROLLER_TYPE_HORI:
		position.x = value;
		break;
	}
}

cMenuScrollBar::cMenuScrollBar(int x, int y, int h, int pageSteps_, cMenu* parentMenu_, cMenuItem* parentItem_) :
	cMenuItemContainer(x, y),
	parentMenu(parentMenu_),
	parentItem(parentItem_),
	pageSteps(pageSteps_)
{
	maximalScroll = position.h = h;
	position.w = 17;
	offset = 0;
	scrollerSteps = 0;

	if ( position.h < 48 ) position.h = 48;
	createSurface();

	upButton = new cMenuButton ( position.x, position.y, "", cMenuButton::BUTTON_TYPE_ARROW_UP_BAR, FONT_LATIN_NORMAL, SoundData.SNDObjectMenu );
	upButton->setReleasedFunction( &upButtonReleased );
	itemList.Add ( upButton );
	downButton = new cMenuButton ( position.x, position.y+position.h-17, "", cMenuButton::BUTTON_TYPE_ARROW_DOWN_BAR, FONT_LATIN_NORMAL, SoundData.SNDObjectMenu );
	downButton->setReleasedFunction( &downButtonReleased );
	itemList.Add ( downButton );
	scroller = new cMenuScroller ( position.x, position.y+17, cMenuScroller::SCROLLER_TYPE_VERT, this );
	itemList.Add ( upButton );
}

cMenuScrollBar::~cMenuScrollBar()
{
	delete upButton;
	delete downButton;
	delete scroller;
}

void cMenuScrollBar::createSurface()
{
	SDL_Rect src = { 234, 1, 16, 48};
	SDL_Rect dest = { 0, 0, 0, 0 };
	surface = SDL_CreateRGBSurface( OtherData.iSurface, 16, position.h-28, SettingsData.iColourDepth, 0, 0, 0, 0 );
	do
	{
		if ( position.h-28-dest.y < 48 ) src.h = position.h-28-dest.x;
		SDL_BlitSurface ( GraphicsData.gfx_menu_stuff, &src, surface, &dest );
		dest.y += src.h;
	}
	while ( dest.y < position.h-28 );
}

void cMenuScrollBar::draw()
{
	SDL_Rect dest = position;
	dest.x++;
	dest.y += 14;
	SDL_BlitSurface ( surface, NULL, buffer, &dest );
	upButton->draw();
	downButton->draw();
	scroller->move ( position.y+17+offset*scrollerSteps );
	if ( scroller->getPosition().y > position.y+position.h-17-14 ) scroller->move ( position.y+position.h-17-14 );
	scroller->draw();
}

void cMenuScrollBar::setMaximalScroll ( int maximalScroll_ )
{
	if ( maximalScroll_ <= position.h-24 )
	{
		offset = 0;
		maximalOffset = 0;
		scrollerSteps = 0;
		maximalScroll = position.h;
		return;
	}
	maximalScroll = maximalScroll_;
	bool stayOnBottom = offset == maximalOffset;
	maximalOffset = (maximalScroll-(position.h-24))/pageSteps;
	maximalOffset++;
	if ( stayOnBottom ) offset = maximalOffset;
	scrollerSteps =  (position.h-17*2-14)/maximalOffset;
	scrollerSteps++;
}

void cMenuScrollBar::upButtonReleased( void* parent )
{
	cMenuScrollBar *This = ((cMenuScrollBar*)parent);
	if ( This->offset > 0 )
	{
		This->offset--;
		This->parentMenu->draw();
	}
}

void cMenuScrollBar::downButtonReleased( void* parent )
{
	cMenuScrollBar *This = ((cMenuScrollBar*)parent);
	if ( This->offset < This->maximalOffset )
	{
		This->offset++;
		This->parentMenu->draw();
	}
}

cMenuListBox::cMenuListBox(int x, int y, int w, int h, int maxLines_, cMenu* parentMenu_) :
	cMenuItemContainer(x, y),
	parentMenu(parentMenu_),
	maxLines(maxLines_)
{
	position.w = w;
	position.h = h;

	maxDrawLines = (position.h-24)/14;

	scrollBar = new cMenuScrollBar ( position.x+position.w-17, position.y, position.h, 14, parentMenu, this );
	itemList.Add ( scrollBar );
}

cMenuListBox::~cMenuListBox()
{
	delete scrollBar;
}

void cMenuListBox::draw()
{
	scrollBar->draw();

	for ( int i = scrollBar->offset; i < scrollBar->offset+maxDrawLines; i++ )
	{
		if ( i >= (int)lines.Size() ) break;
		font->showText ( position.x+12, position.y+12+14*(i-scrollBar->offset), lines[i] );
	}
}

void cMenuListBox::addLine ( string line )
{
	size_t pos = 0;
	size_t length;
	do
	{
		length = line.length()-pos;
		while ( font->getTextWide ( line.substr ( pos, length ) ) > position.w-24-17 ) length--;
		lines.Add ( line.substr ( pos, length ) );
		pos += length;
		if ( pos == line.length() ) break;
		line.insert ( pos, "	" );
	}
	while ( pos < line.length() );

	scrollBar->setMaximalScroll ( (int)lines.Size()*14 );
}

cMenuLineEdit::cMenuLineEdit ( int x, int y, int w, int h, cMenu *parentMenu_ ) : cMenuItem ( x, y ), parentMenu(parentMenu_)
{
	position.w = w;
	position.h = h;

	returnPressed = NULL;

	text = "";
	cursorPos = 0;

	readOnly = false;
	takeChars = takeNumerics = true;
	startOffset = endOffset = 0;
}

void cMenuLineEdit::draw()
{
	font->showText ( position.x+6, position.y+3, text.substr ( startOffset, endOffset-startOffset ) );
	if ( active && !readOnly ) font->showText ( position.x+6+font->getTextWide( text.substr( startOffset, cursorPos-startOffset ) ), position.y+3, "|" );
}

bool cMenuLineEdit::preClicked()
{
	if ( active )
	{
		int x = mouse->x - (position.x+6);
		int cursor = startOffset;
		while ( font->getTextWide( text.substr ( startOffset, cursor-startOffset ) ) < x )
		{
			doPosIncrease ( cursor, cursor );
			if ( cursor >= endOffset )
			{
				cursor = endOffset;
				break;
			}
		}
		cursorPos = cursor;
	}
	return true;
}

void cMenuLineEdit::setReadOnly ( bool readOnly_ )
{
	readOnly = readOnly_;
}

void cMenuLineEdit::setTaking ( bool takeChars_, bool takeNumerics_ )
{
	takeChars = takeChars_;
	takeNumerics = takeNumerics_;
}

void cMenuLineEdit::setText ( string text_ )
{
	text = text_;
	startOffset = 0;
	endOffset = (int)text.length();
	cursorPos = endOffset;
	while ( font->getTextWide( text.substr( startOffset, endOffset-startOffset ) ) > position.w-getBorderSize() ) doPosDecrease ( endOffset );
}

string cMenuLineEdit::getText ()
{
	return text;
}

void cMenuLineEdit::doPosIncrease( int &value, int pos )
{
	if ( pos < (int)text.length() )
	{
		unsigned char c = ((unsigned char*)(text.c_str()))[pos];
		if ( (c & 0xE0) == 0xE0 ) value += 3;
		else if ( (c & 0xC0) == 0xC0 ) value += 2;
		else  value += 1;
	}
}

void cMenuLineEdit::doPosDecrease( int &pos )
{
	if ( pos > 0 )
	{
		unsigned char c = ((unsigned char*)(text.c_str()))[pos-1];
		while ( ((c & 0xE0) != 0xE0) && ((c & 0xC0) != 0xC0) && ((c & 0x80) == 0x80) )
		{
			pos--;
			c = ((unsigned char*)(text.c_str()))[pos-1];
		}
		pos--;
	}
}

void cMenuLineEdit::scrollLeft( bool changeCursor )
{
	// makes the cursor go left
	if ( changeCursor && cursorPos > 0 ) doPosDecrease ( cursorPos );

	if ( cursorPos > 0 ) while ( cursorPos-1 < startOffset ) doPosDecrease ( startOffset );
	else while ( cursorPos < startOffset ) doPosDecrease ( startOffset );

	if ( font->getTextWide( text.substr( startOffset, text.length()-startOffset ) ) > position.w-getBorderSize() )
	{
		endOffset = (int)text.length();
		while ( font->getTextWide( text.substr( startOffset, endOffset-startOffset ) ) > position.w-getBorderSize() ) doPosDecrease ( endOffset );
	}
}

void cMenuLineEdit::scrollRight()
{
	// makes the cursor go right
	if ( cursorPos < (int)text.length() ) doPosIncrease ( cursorPos, cursorPos );
	while ( cursorPos > endOffset ) doPosIncrease ( endOffset, endOffset );
	while ( font->getTextWide( text.substr( startOffset, endOffset-startOffset ) ) > position.w-getBorderSize() ) doPosIncrease ( startOffset, startOffset );
}

void cMenuLineEdit::deleteLeft()
{
	// deletes the first character left from the cursor
	if ( cursorPos > 0 )
	{
		unsigned char c = ((unsigned char*)(text.c_str()))[cursorPos-1];
		while ( ((c & 0xE0) != 0xE0) && ((c & 0xC0) != 0xC0) && ((c & 0x80) == 0x80) )
		{
			text.erase ( cursorPos-1, 1 );
			cursorPos--;
			c = ((unsigned char*)(text.c_str()))[cursorPos-1];
		}
		text.erase ( cursorPos-1, 1 );
		cursorPos--;
		if ( endOffset > (int)text.length() ) endOffset = (int)text.length();
		scrollLeft( false );
	}
}

void cMenuLineEdit::deleteRight()
{
	// deletes the first character right from the cursor
	if ( cursorPos < (int)text.length() )
	{
		unsigned char c = ((unsigned char*)(text.c_str()))[cursorPos];
		if ( (c & 0xE0) == 0xE0 )text.erase ( cursorPos, 3 );
		else if ( (c & 0xC0) == 0xC0 )text.erase ( cursorPos, 2 );
		else text.erase ( cursorPos, 1 );
		if ( endOffset > (int)text.length() ) endOffset = (int)text.length();
	}
}

int cMenuLineEdit::getBorderSize()
{
	return 12;
}

bool cMenuLineEdit::handleKeyInput( SDL_keysym keysym, string ch, void *parent )
{
	if ( readOnly ) return false;

	switch ( keysym.sym )
	{
	case SDLK_RETURN:
		if ( returnPressed )
		{
			PlayFX ( SoundData.SNDHudButton );
			returnPressed ( parent );
		}
		break;
	case SDLK_LEFT:
		scrollLeft();
		break;
	case SDLK_RIGHT:
		scrollRight();
		break;
	case SDLK_BACKSPACE:
		deleteLeft();
		if ( wasKeyInput ) wasKeyInput ( parent );
		break;
	case SDLK_DELETE:
		deleteRight();
		if ( wasKeyInput ) wasKeyInput ( parent );
		break;
	default: // no special key - handle as normal character:
		if ( keysym.unicode >= 32 )
		{
			if ( keysym.unicode >= 48 && keysym.unicode <= 57 )
			{
				if ( !takeNumerics ) break;
			}
			else if ( !takeChars ) break;

			text.insert ( cursorPos, ch );
			if ( cursorPos < (int)text.length() ) doPosIncrease ( cursorPos, cursorPos );
			if ( cursorPos >= endOffset )
			{
				doPosIncrease ( endOffset, endOffset );
				while ( font->getTextWide( text.substr( startOffset, endOffset-startOffset ) ) > position.w-getBorderSize() ) doPosIncrease ( startOffset, startOffset );
			}
			else
			{
				if ( font->getTextWide( text.substr( startOffset, endOffset-startOffset ) ) > position.w-getBorderSize() ) doPosDecrease ( endOffset );
				else doPosIncrease ( endOffset, cursorPos );
			}
			if ( wasKeyInput ) wasKeyInput ( parent );
		}
		break;
	}
	parentMenu->draw();
	return true;
}

void cMenuLineEdit::setReturnPressedFunc( void (*returnPressed_)(void *) )
{
	returnPressed = returnPressed_;
}

cMenuChatBox::cMenuChatBox ( int x, int y, cMenu *parentMenu_ ) :
	cMenuLineEdit(x, y, SettingsData.iScreenW - HUD_TOTAL_WIDTH, 21, parentMenu_)
{
	generateSurface();
}

void cMenuChatBox::generateSurface()
{
	if ( SettingsData.iScreenW-HUD_TOTAL_WIDTH-20 < 60 ) return;

	surface = SDL_CreateRGBSurface( OtherData.iSurface, SettingsData.iScreenW-HUD_TOTAL_WIDTH-20, 48, SettingsData.iColourDepth, 0, 0, 0, 0 );
	SDL_FillRect ( surface, NULL, 0xFF00FF );
	SDL_SetColorKey ( surface, SDL_SRCCOLORKEY, 0xFF00FF );

	SDL_Rect src = { 0, 0, 30, 48 };
	SDL_Rect dest = { 0, 0, 0, 0 };

	// blit the beginning
	SDL_BlitSurface ( GraphicsData.gfx_hud_chatbox, &src, surface, &dest );
	// blit the end
	src.x = 40;
	dest.x = surface->w-30;
	SDL_BlitSurface ( GraphicsData.gfx_hud_chatbox, &src, surface, &dest );

	int restWidth = surface->w-60;
	src.x = 30;
	dest.x = 30;
	while ( restWidth > 0 )
	{
		src.w = 10;
		if ( restWidth < 10 ) src.w = restWidth;
		SDL_BlitSurface ( GraphicsData.gfx_hud_chatbox, &src, surface, &dest );
		dest.x += src.w;
		restWidth -= src.w;
	}
}

int cMenuChatBox::getBorderSize()
{
	return 38;
}

void cMenuChatBox::draw()
{
	if ( disabled ) return;

	if ( surface )
	{
		SDL_BlitSurface ( surface, NULL, buffer, &position );
	}

	font->showText ( position.x+28, position.y+5, text.substr ( startOffset, endOffset-startOffset ) );
	if ( active && !readOnly ) font->showText ( position.x+28+font->getTextWide( text.substr( startOffset, cursorPos-startOffset ) ), position.y+5, "|" );
}

cMenuPlayersBox::cMenuPlayersBox ( int x, int y, int w, int h, cNetworkMenu *parentMenu_ ) : cMenuItemContainer(x, y), parentMenu(parentMenu_)
{
	position.w = w;
	position.h = h;

	maxDrawPlayers = (position.h-24)/14;

	scrollBar = new cMenuScrollBar ( position.x+position.w-17, position.y, position.h, 14, parentMenu, this );
	itemList.Add ( scrollBar );

	for ( int i = 0; i < maxDrawPlayers; i++ )
	{
		cMenuImage *colorImage = new cMenuImage ( position.x+12, position.y+12+14*i );
		cMenuLabel *nameLabel = new cMenuLabel ( position.x+12+16, position.y+12+14*i );
		cMenuImage *readyImage = new cMenuImage ( position.x+position.w-17-15, position.y+12+14*i );
		playerColors.Add ( colorImage );
		playerNames.Add ( nameLabel );
		playerReadys.Add ( readyImage );
	}
}

cMenuPlayersBox::~cMenuPlayersBox()
{
	delete scrollBar;
	for ( int i = 0; i < maxDrawPlayers; i++ )
	{
		delete playerColors[i];
		delete playerNames[i];
		delete playerReadys[i];
	}
}

void cMenuPlayersBox::draw()
{
	SDL_Rect src = { 10, 0, 10, 10 };

	AutoSurface readySurface(SDL_CreateRGBSurface(OtherData.iSurface | SDL_SRCCOLORKEY, src.w, src.h, SettingsData.iColourDepth, 0, 0, 0, 0));
	SDL_SetColorKey ( readySurface, SDL_SRCCOLORKEY, 0xFF00FF );
	SDL_FillRect ( readySurface, NULL, 0xFF00FF );
	SDL_BlitSurface ( GraphicsData.gfx_player_ready, &src, readySurface, NULL );

	src.x -= 10;
	AutoSurface notReadySurface(SDL_CreateRGBSurface(OtherData.iSurface | SDL_SRCCOLORKEY, src.w, src.h, SettingsData.iColourDepth, 0, 0, 0, 0));
	SDL_SetColorKey ( notReadySurface, SDL_SRCCOLORKEY, 0xFF00FF );
	SDL_FillRect ( notReadySurface, NULL, 0xFF00FF );
	SDL_BlitSurface ( GraphicsData.gfx_player_ready, &src, notReadySurface, NULL );

	for ( int i = scrollBar->offset; i < scrollBar->offset+maxDrawPlayers; i++ )
	{
		if ( i < (int)players->Size() )
		{
			if ( (*players)[i]->ready ) playerReadys[i-scrollBar->offset]->setImage ( readySurface );
			else playerReadys[i-scrollBar->offset]->setImage ( notReadySurface );

			AutoSurface colorSurface(SDL_CreateRGBSurface(OtherData.iSurface | SDL_SRCCOLORKEY, src.w, src.h, SettingsData.iColourDepth, 0, 0, 0, 0));
			SDL_BlitSurface ( OtherData.colors[(*players)[i]->color], &src, colorSurface, NULL );
			playerColors[i-scrollBar->offset]->setImage ( colorSurface );

			playerNames[i-scrollBar->offset]->setText ( (*players)[i]->name );
		}
		else
		{
			playerColors[i-scrollBar->offset]->setImage ( NULL );
			playerReadys[i-scrollBar->offset]->setImage ( NULL );
			playerNames[i-scrollBar->offset]->setText ( "" );
		}

		playerColors[i-scrollBar->offset]->draw();
		playerNames[i-scrollBar->offset]->draw();
		playerReadys[i-scrollBar->offset]->draw();
	}
	scrollBar->draw();
}

bool cMenuPlayersBox::preClicked()
{
	if ( mouse->x > position.x+position.w-17-15 && mouse->x < position.x+position.w-17-5 )
	{
		for ( int i = scrollBar->offset; i < scrollBar->offset+maxDrawPlayers; i++ )
		{
			if ( i >= (int)players->Size() ) break;
			if ( mouse->y > position.y+12+14*(i-scrollBar->offset) && mouse->y < position.y+12+14*(i-scrollBar->offset)+10 )
			{
				parentMenu->playerReadyClicked ( (*players)[i] );
			}
		}
	}
	return true;
}

void cMenuPlayersBox::setPlayers ( cList<sMenuPlayer*> *player_ )
{
	players = player_;

	scrollBar->setMaximalScroll ( (int)players->Size()*14 );
}

cMenuSaveSlot::cMenuSaveSlot( int x, int y, cMenu *parent ) : cMenuItem ( x, y )
{
	position.w = 203;
	position.h = 71;

	saveNumber = new cMenuLabel ( position.x+13, position.y+27, "", FONT_LATIN_BIG );
	saveTime = new cMenuLabel ( position.x+43, position.y+19 );
	saveType = new cMenuLabel ( position.x+156, position.y+19 );
	saveName = new cMenuLineEdit ( position.x+38, position.y+37, 154, 18, parent );
	saveName->setReadOnly ( true );
}

cMenuSaveSlot::~cMenuSaveSlot()
{
	delete saveNumber;
	delete saveType;
	delete saveTime;
	delete saveName;
}

void cMenuSaveSlot::draw()
{
	saveNumber->draw();
	saveType->draw();
	saveTime->draw();
	saveName->draw();
}

void cMenuSaveSlot::setSaveData ( sSaveFile saveFile, bool selected )
{
	saveNumber->setText ( iToStr ( saveFile.number ) );
	saveType->setText ( saveFile.type );
	saveTime->setText ( saveFile.time );
	saveName->setText ( saveFile.gamename );
	if ( selected ) saveNumber->setFontType ( FONT_LATIN_BIG_GOLD );
	else saveNumber->setFontType ( FONT_LATIN_BIG );
}

void cMenuSaveSlot::reset( int number, bool selected )
{
	saveNumber->setText ( iToStr ( number ) );
	saveType->setText ( "" );
	saveTime->setText ( "" );
	saveName->setText ( "" );
	if ( selected ) saveNumber->setFontType ( FONT_LATIN_BIG_GOLD );
	else saveNumber->setFontType ( FONT_LATIN_BIG );
}

cMenuLineEdit *cMenuSaveSlot::getNameEdit ()
{
	return saveName;
}

cMenuBuildSpeedHandler::cMenuBuildSpeedHandler( int x, int y ) : cMenuItemContainer ( x, y )
{
	speedGroup = new cMenuRadioGroup ();

	for ( int i = 0; i < 3; i++ )
	{
		int factor = i+1;
		if ( i == 2 ) factor = 4;
		turnsLabels[i] = new cMenuLabel ( position.x+97, position.y+25*i+5 );
		costsLabels[i] = new cMenuLabel ( position.x+137, position.y+25*i+5 );
		turnsLabels[i]->setCentered ( true );
		costsLabels[i]->setCentered ( true );
		speedButtons[i] =  new cMenuCheckButton ( position.x, position.y+25*i, lngPack.i18n ( "Text~Button~Build" ) + " x" + iToStr ( factor ), i == 0, false, cMenuCheckButton::RADIOBTN_TYPE_ANGULAR_BUTTON );
		speedGroup->addButton ( speedButtons[i] );
		addItem ( turnsLabels[i] );
		addItem ( costsLabels[i] );
	}
	addItem ( speedGroup );
	position.w = 77;
	position.h = 75;
}

cMenuBuildSpeedHandler::~cMenuBuildSpeedHandler()
{
	for ( int i = 0; i < 3; i++ )
	{
		delete turnsLabels[i];
		delete costsLabels[i];
	}
	delete speedGroup;
}

void cMenuBuildSpeedHandler::setValues ( int *turboBuildTurns, int *turboBuildCosts )
{
	turnsLabels[0]->setText ( iToStr ( turboBuildTurns[0] ) );
	costsLabels[0]->setText ( iToStr ( turboBuildCosts[0] ) );

	if ( turboBuildTurns[1] > 0 )
	{
		turnsLabels[1]->setText ( iToStr ( turboBuildTurns[1] ) );
		costsLabels[1]->setText ( iToStr ( turboBuildCosts[1] ) );
		speedButtons[1]->setLocked ( false );
	}
	else
	{
		turnsLabels[1]->setText ( "" );
		costsLabels[1]->setText ( "" );
		speedButtons[1]->setLocked ( true );
		if ( !speedGroup->buttonIsChecked ( 0 ) ) speedButtons[0]->setChecked ( true );
	}

	if ( turboBuildTurns[2] > 0 )
	{
		turnsLabels[2]->setText ( iToStr ( turboBuildTurns[2] ) );
		costsLabels[2]->setText ( iToStr ( turboBuildCosts[2] ) );
		speedButtons[2]->setLocked ( false );
	}
	else
	{
		turnsLabels[2]->setText ( "" );
		costsLabels[2]->setText ( "" );
		speedButtons[2]->setLocked ( true );
		if ( speedGroup->buttonIsChecked ( 2 ) ) speedButtons[1]->setChecked ( true );
	}
}

void cMenuBuildSpeedHandler::setBuildSpeed( int buildSpeed )
{
	if ( buildSpeed < 0 && buildSpeed >= 3 ) return;
	speedButtons[buildSpeed]->setChecked ( true );
}

int cMenuBuildSpeedHandler::getBuildSpeed()
{
	for ( int i = 0; i < 3; i++ )
	{
		if ( speedGroup->buttonIsChecked ( i ) ) return i;
	}
	return 0;
}

cMenuUpgradeFilter::cMenuUpgradeFilter( int x, int y, cHangarMenu *parentMenu_ ) : cMenuItemContainer ( x, y ), parentMenu(parentMenu_)
{
	checkButtonTank = new cMenuCheckButton ( position.x, position.y, "", true, false, cMenuCheckButton::CHECKBOX_TYPE_TANK );
	checkButtonTank->setClickedFunction ( &buttonChanged );
	addItem ( checkButtonTank );

	checkButtonPlane = new cMenuCheckButton ( position.x+33, position.y, "", false, false, cMenuCheckButton::CHECKBOX_TYPE_PLANE );
	checkButtonPlane->setClickedFunction ( &buttonChanged );
	addItem ( checkButtonPlane );

	checkButtonShip = new cMenuCheckButton ( position.x+33*2, position.y, "", false, false, cMenuCheckButton::CHECKBOX_TYPE_SHIP );
	checkButtonShip->setClickedFunction ( &buttonChanged );
	addItem ( checkButtonShip );

	checkButtonBuilding = new cMenuCheckButton ( position.x+33*3, position.y, "", false, false, cMenuCheckButton::CHECKBOX_TYPE_BUILD );
	checkButtonBuilding->setClickedFunction ( &buttonChanged );
	addItem ( checkButtonBuilding );

	checkButtonTNT = new cMenuCheckButton ( position.x+33*4, position.y, "", false, false, cMenuCheckButton::CHECKBOX_TYPE_TNT );
	checkButtonTNT->setClickedFunction ( &buttonChanged );
	addItem ( checkButtonTNT );
}

cMenuUpgradeFilter::~cMenuUpgradeFilter()
{
	delete checkButtonTank;
	delete checkButtonPlane;
	delete checkButtonShip;
	delete checkButtonBuilding;
	delete checkButtonTNT;
}

void cMenuUpgradeFilter::setTankChecked ( bool checked )
{
	checkButtonTank->setChecked ( checked );
}

void cMenuUpgradeFilter::setPlaneChecked ( bool checked )
{
	checkButtonPlane->setChecked ( checked );
}

void cMenuUpgradeFilter::setShipChecked ( bool checked )
{
	checkButtonShip->setChecked ( checked );
}

void cMenuUpgradeFilter::setBuildingChecked ( bool checked )
{
	checkButtonBuilding->setChecked ( checked );
}

void cMenuUpgradeFilter::setTNTChecked ( bool checked )
{
	checkButtonTNT->setChecked ( checked );
}

bool cMenuUpgradeFilter::TankIsChecked()
{
	return checkButtonTank->isChecked();
}

bool cMenuUpgradeFilter::PlaneIsChecked()
{
	return checkButtonPlane->isChecked();
}

bool cMenuUpgradeFilter::ShipIsChecked()
{
	return checkButtonShip->isChecked();
}

bool cMenuUpgradeFilter::BuildingIsChecked()
{
	return checkButtonBuilding->isChecked();
}

bool cMenuUpgradeFilter::TNTIsChecked()
{
	return checkButtonTNT->isChecked();
}

void cMenuUpgradeFilter::buttonChanged( void *parent )
{
	cMenuUpgradeFilter *filter = ((cMenuUpgradeFilter*)parent);
	filter->parentMenu->generateSelectionList();
	filter->parentMenu->draw();
}

cMenuStoredUnitDetails::cMenuStoredUnitDetails ( int x, int y, sUnitData *unitData_ ) : cMenuItem ( x, y ), unitData(unitData_)
{
	position.w = 135;
	position.h = 45;
}

void cMenuStoredUnitDetails::setUnitData ( sUnitData *unitData_ )
{
	unitData = unitData_;
}

void cMenuStoredUnitDetails::draw()
{
	if ( !unitData ) return;

	cUnitDataSymbolHandler::drawNumber ( position.x+16, position.y+12, unitData->hitpointsCur, unitData->hitpointsMax );
	font->showText ( position.x+30, position.y+12, lngPack.i18n ( "Text~Hud~Hitpoints" ), FONT_LATIN_SMALL_WHITE );
	cUnitDataSymbolHandler::drawSymbols ( cUnitDataSymbolHandler::MENU_SYMBOLS_HITS, position.x+63, position.y+12, 58, false, unitData->hitpointsCur, unitData->hitpointsMax );

	if ( unitData->canAttack )
	{
		cUnitDataSymbolHandler::drawNumber ( position.x+16, position.y+12+15, unitData->ammoCur, unitData->ammoMax );
		font->showText ( position.x+30, position.y+27, lngPack.i18n ( "Text~Hud~AmmoShort" ), FONT_LATIN_SMALL_WHITE );
		cUnitDataSymbolHandler::drawSymbols ( cUnitDataSymbolHandler::MENU_SYMBOLS_AMMO, position.x+63, position.y+27, 58, false, unitData->ammoCur, unitData->ammoMax );
	}
}

cMenuSlider::cMenuSlider(int x, int y, float minValue_, float maxValue_, cMenu* parent_, int wight, eSliderType type_, eSliderDirection direction_) :
	cMenuItem(x, y),
	type(type_),
	direction(direction_),
	parent(parent_)
{
	minValue = min ( minValue_, maxValue_ );
	maxValue = max ( minValue_, maxValue_ );
	curValue = minValue;

	switch ( type )
	{
	case SLIDER_TYPE_NORMAL:
		{
			SDL_Rect src = { 201, 53, 58, 3 };
			position.w = src.w;
			position.h = src.h;

	surface = SDL_CreateRGBSurface( OtherData.iSurface, src.w, src.h, SettingsData.iColourDepth, 0, 0, 0, 0 );
	SDL_BlitSurface ( GraphicsData.gfx_menu_stuff, &src, surface, NULL );

			scroller = new cMenuScroller ( x-7, y-7, cMenuScroller::SCROLLER_TYPE_HORI, this, &scrollerMoved );
		}
		break;
	case SLIDER_TYPE_HUD_ZOOM:
		position.w = wight;
		position.h = 14;

		surface = NULL;

		scroller = new cMenuScroller ( x, y, cMenuScroller::SCROLLER_TYPE_HUD_ZOOM, this, &scrollerMoved );
		break;
	}

	movedCallback = NULL;
}

cMenuSlider::~cMenuSlider()
{
	delete scroller;
}

void cMenuSlider::draw()
{
	if ( surface ) SDL_BlitSurface ( surface, NULL, buffer, &position );
	cMenuSlider::scroller->draw();
}

void cMenuSlider::setValue( float value )
{
	curValue = value;
	switch ( direction )
	{
	case SLIDER_DIR_LEFTMIN:
		scroller->move ( (int)( ( (minValue+curValue)*(position.w-(( type == SLIDER_TYPE_HUD_ZOOM ) ? scroller->getPosition().w : 0) ) ) / (maxValue-minValue) + position.x - (( type == SLIDER_TYPE_HUD_ZOOM ) ? 0 : 7) ) );
		break;
	case SLIDER_DIR_RIGHTMIN:
		scroller->move ( (int)( ( (maxValue-curValue)*(position.w-(( type == SLIDER_TYPE_HUD_ZOOM ) ? scroller->getPosition().w : 0) ) ) / (maxValue-minValue) + position.x - (( type == SLIDER_TYPE_HUD_ZOOM ) ? 0 : 7) ) );
		break;
	}
}

float cMenuSlider::getValue()
{
	return curValue;
}

void cMenuSlider::setMoveCallback ( void (*movedCallback_)(void *) )
{
	movedCallback = movedCallback_;
}

void cMenuSlider::scrollerMoved( void *parent_ )
{
	cMenuSlider *This = (cMenuSlider*)parent_;
	int pos = This->scroller->getPosition().x - This->position.x + (( This->type == SLIDER_TYPE_HUD_ZOOM ) ? 0 : 7);
	if ( pos < 0 )
	{
		pos = 0;
		This->scroller->move ( This->position.x-(( This->type == SLIDER_TYPE_HUD_ZOOM ) ? 0 : 7) );
	}
	if ( pos > This->position.w-(( This->type == SLIDER_TYPE_HUD_ZOOM ) ? This->scroller->getPosition().w : 0) )
	{
		pos = This->position.w-(( This->type == SLIDER_TYPE_HUD_ZOOM ) ? This->scroller->getPosition().w : 7);
		This->scroller->move ( This->position.x+This->position.w-(( This->type == SLIDER_TYPE_HUD_ZOOM ) ? This->scroller->getPosition().w : 7) );
	}
	switch ( This->direction )
	{
	case SLIDER_DIR_LEFTMIN:
		This->curValue = This->minValue + ((This->maxValue-This->minValue) / (float)(This->position.w-(( This->type == SLIDER_TYPE_HUD_ZOOM ) ? This->scroller->getPosition().w : 0))) * (float)pos;
		break;
	case SLIDER_DIR_RIGHTMIN:
		This->curValue = This->maxValue - ((This->maxValue-This->minValue) / (float)(This->position.w-(( This->type == SLIDER_TYPE_HUD_ZOOM ) ? This->scroller->getPosition().w : 0))) * (float)pos;
		break;
	}
	This->parent->draw();

	if ( This->movedCallback ) This->movedCallback ( This->parent );
}

bool cMenuSlider::preClicked()
{
	scroller->mouseMoved( true );
	return true;
}

cMenuScrollerHandler::cMenuScrollerHandler(int x, int y, int w, int maxValue_) : cMenuItem (x, y), maxValue(maxValue_)
{
	position.h = 17;
	position.w = w;

	scroller = new cMenuScroller ( position.x, position.y, cMenuScroller::SCROLLER_TYPE_HORI, this );

	currentValue = 0;
}

cMenuScrollerHandler::~cMenuScrollerHandler()
{
	delete scroller;
}

void cMenuScrollerHandler::draw()
{
	scroller->draw();
}

void cMenuScrollerHandler::setValue( int value )
{
	currentValue = value;
	int pos = position.x + (position.w-14) / maxValue * currentValue;
	scroller->move ( pos );
}

SDL_Rect cMenuScrollerHandler::getPosition()
{
	return position;
}

cMenuReportsScreen::cMenuReportsScreen( int x, int y, int w, int h, cPlayer *owner_, cReportsMenu *parentMenu_ ) :
	cMenuItem ( x, y ),
	owner ( owner_ ),
	parentMenu ( parentMenu_ )
{
	position.w = w;
	position.h = h;

	vehicles = owner->VehicleList;
	buildings = owner->BuildingList;
	index = 0;
	selected = -1;
	filterPlanes = filterGround = filterSea = filterBuilding = false;
	filterBuild = filterAttack = filterDamaged = filterStealth = false;

	maxItems = ((position.h-25) / 55);

	unitDetails = new cMenuUnitDetails*[maxItems];
	for ( int i = 0; i < maxItems; i++ )
	{
		unitDetails[i] = new cMenuUnitDetails( position.x+127, position.y+17+55*i, true, owner );
	}
	screenType = REP_SCR_TYPE_UNITS;
}

cMenuReportsScreen::~cMenuReportsScreen()
{
	for ( int i = 0; i < maxItems; i++ )
	{
		delete unitDetails[i];
	}
}

void cMenuReportsScreen::draw()
{
	switch ( screenType )
	{
	case REP_SCR_TYPE_UNITS:
		drawUnitsScreen();
		break;
	case REP_SCR_TYPE_DISADVA:
		drawDisadvantagesScreen();
		break;
	case REP_SCR_TYPE_SCORE:
		drawScoreScreen();
		break;
	case REP_SCR_TYPE_REPORTS:
		drawReportsScreen();
		break;
	}
}

void cMenuReportsScreen::drawUnitsScreen()
{
	goThroughUnits ( true );

	if ( selected >= index*maxItems && selected < (index+1)*maxItems )
	{
		int selIndex = selected-index*maxItems;
		SDL_Rect selDest = { position.x+13, position.y+26+55*selIndex, 8, 1 };

		SDL_FillRect ( buffer, &selDest, 0xE0E0E0 );
		selDest.x += 30;
		SDL_FillRect ( buffer, &selDest ,0xE0E0E0 );
		selDest.y += 38;
		SDL_FillRect ( buffer, &selDest, 0xE0E0E0 );
		selDest.x -= 30;
		SDL_FillRect ( buffer, &selDest, 0xE0E0E0 );
		selDest.y = position.y+26+55*selIndex;
		selDest.w = 1;
		selDest.h = 8;
		SDL_FillRect ( buffer, &selDest, 0xE0E0E0 );
		selDest.x += 38;
		SDL_FillRect ( buffer, &selDest, 0xE0E0E0 );
		selDest.y += 31;
		SDL_FillRect ( buffer, &selDest, 0xE0E0E0 );
		selDest.x -= 38;
		SDL_FillRect ( buffer, &selDest, 0xE0E0E0 );
	}
}

void cMenuReportsScreen::drawDisadvantagesScreen()
{
	font->showText ( position.x+17, position.y+30, lngPack.i18n( "Text~Error_Messages~INFO_Not_Implemented" ) );
}

void cMenuReportsScreen::drawScoreScreen()
{
	int turnLimit, scoreLimit;
	Client->getVictoryConditions(&turnLimit, &scoreLimit);
	{
		std::stringstream ss;
		if(turnLimit) 
		{
			ss << lngPack.i18n("Text~Comp~GameEndsAt") << " " <<
			plural(turnLimit, "Text~Comp~Turn", "Text~Comp~Turns");
		}
		else if(scoreLimit)
		{
			ss << lngPack.i18n("Text~Comp~GameEndsAt") << " " <<
			plural(scoreLimit, "Text~Comp~Point", "Text~Comp~Points");
		}
		else 
			ss << lngPack.i18n("Text~Comp~NoLimit");
		
		font->showText(position.x+25, position.y+20, ss.str());
	}
	
	for (unsigned n=0, y=36; n < Client->PlayerList->Size(); n++, y+=16)
	{
		cPlayer *p = (*Client->PlayerList)[n];
		int score = p->getScore(Client->getTurn());
		int ecos = p->numEcos;
		
		SDL_Rect r = {position.x + 24, position.y + y + 3, 8, 8};
		SDL_FillRect(buffer, &r, getPlayerColour(p));
		
		std::stringstream ss;
		ss << p->name << ": " 
			<< plural(score, "Text~Comp~Point", "Text~Comp~Points") << ", "
			<< plural(ecos, "Text~Comp~EcoSphere", "Text~Comp~EcoSpheres");
		
		font->showText(position.x+42, position.y+y, ss.str());
	}
	
	drawScoreGraph();
}

void cMenuReportsScreen::drawScoreGraph()
{
	const Uint32 axis_colour = SDL_MapRGB(buffer->format, 164, 164, 164);
	const Uint32 limit_colour = SDL_MapRGB(buffer->format, 128, 128, 128);
	
	const int px = position.x;
	const int py = position.y;
	
	const int w = 400;
	const int h = 300;
	
	const int x0 = px +  40, x1 = x0 + w;
	const int y0 = py + 130, y1 = y0 + h;
	
	/*
		Calculate time axis
	*/
	const int pix_per_turn = 5;
	
	const int now = Client->getTurn();
	
	const int num_turns = w / pix_per_turn;
	int max_turns = now + 40;
	int min_turns = max_turns - num_turns;
	
	if(min_turns < 1)
	{
		const int over = 1 - min_turns;
		min_turns += over;
		max_turns += over;
	}
	
	const int now_x = x0 + (now - min_turns) * pix_per_turn;
	
	/*
		Calculate points axis
	*/
	int highest_score = 0;
	int lowest_score = 0x7FFFFFFF; 
	for(unsigned n=0; n < Client->PlayerList->Size(); n++)
	{
		for(int turn = min_turns; turn < max_turns; turn++)
		{
			cPlayer *p = (*Client->PlayerList)[n];
			int score = extrapolateScore(p, turn);
			if(score > highest_score)
				highest_score = score;
			if(score < lowest_score)
				lowest_score = score;
		}
	}
	
	const int max_points = highest_score;
	const int min_points = lowest_score;
	const int num_points = max_points - min_points;
	
	const float default_pix_per_point = 5;
	float pix_per_point;
	if(num_points) {
		pix_per_point = (float) h / num_points;
		if(pix_per_point > default_pix_per_point)
			pix_per_point = default_pix_per_point;
	}
	else
		pix_per_point = default_pix_per_point;
	
	/*
		Draw Limits
	*/
	drawLine(buffer, now_x, y0, now_x, y1, limit_colour);
	
	int turn_lim, points_lim;
	Client->getVictoryConditions(&turn_lim, &points_lim);
	
	if(turn_lim && turn_lim > min_turns && turn_lim < max_turns) 
	{
		int x = x0 + (turn_lim - min_turns) * pix_per_turn;
		
		drawLine(buffer, x, y0, x, y1, limit_colour); 
		font->showTextCentered(x, y1 + 8, iToStr(turn_lim), FONT_LATIN_SMALL_WHITE);
	}
	if(points_lim && points_lim > min_points && points_lim < max_points)
	{
		int y = y1 - (points_lim - min_points) * pix_per_point;
		
		drawLine(buffer, x0, y, x1, y, limit_colour);
		font->showText(x0 - 16, y - 3, iToStr(points_lim), FONT_LATIN_SMALL_WHITE);
	}
	
	/*
		Draw Labels
	*/
	int my = y1 - (max_points - min_points) * pix_per_point;
	
	font->showTextCentered(x0,    y1 + 8, iToStr(min_turns), FONT_LATIN_SMALL_WHITE);
	font->showTextCentered(now_x, y1 + 8, iToStr(now),       FONT_LATIN_SMALL_WHITE);
	font->showTextCentered(x1,    y1 + 8, iToStr(max_turns), FONT_LATIN_SMALL_WHITE);
	
	font->showText(x0 - 16, y1 - 3, iToStr(min_points), FONT_LATIN_SMALL_WHITE);
	font->showText(x0 - 16, my - 3, iToStr(max_points), FONT_LATIN_SMALL_WHITE);
	
	/*
		Draw Score Lines
	*/
	for(unsigned n=0, y=40; n < Client->PlayerList->Size(); n++, y+=25)
	{
		cPlayer *p = (*Client->PlayerList)[n];
		Uint32 player_colour = getPlayerColour(p);
		
		int lx, ly;
		
		for(int turn = min_turns; turn < max_turns; turn++)
		{
			int points = extrapolateScore(p, turn);
			
			int x = x0 + pix_per_turn * (turn - min_turns);
			int y = y1 - pix_per_point * (points - min_points);
			
			if(turn != min_turns)
				drawLine(buffer, lx, ly, x, y, player_colour);
			
			lx = x;
			ly = y;
		}
	}
	
	/*
		Draw Axes
	*/
	drawLine(buffer, x0, y0, x0, y1, axis_colour);
	drawLine(buffer, x0, y1, x1, y1, axis_colour);
}

void cMenuReportsScreen::drawReportsScreen()
{
	SDL_Rect textDest = { position.x+54, position.y+25, 410, 30 };
	for ( unsigned int i = (index)*maxItems; i < owner->savedReportsList.Size(); i++ )
	{
		if ( (int)i >= (index+1)*maxItems ) break;
		sSavedReportMessage &savedReport = owner->savedReportsList[i];

		switch ( savedReport.type )
		{
		case sSavedReportMessage::REPORT_TYPE_COMP:
			// TODO: draw computer symbol
			break;
		case sSavedReportMessage::REPORT_TYPE_UNIT:
			{
				SDL_Surface *orgSurface;
				SDL_Rect src = { 0, 0, 32, 32 };
				SDL_Rect dest = { position.x+17, position.y+30+(i-(index)*maxItems)*55, 0, 0 };

				if ( savedReport.unitID.getVehicle() ) orgSurface = savedReport.unitID.getVehicle()->img_org[0];
				else if ( savedReport.unitID.getBuilding() ) orgSurface = savedReport.unitID.getBuilding()->img_org;
				else break;

				AutoSurface surface(generateUnitSurface(orgSurface, *savedReport.unitID.getUnitDataOriginalVersion()));
				SDL_BlitSurface ( surface, &src, buffer, &dest );
			}
			break;
		case sSavedReportMessage::REPORT_TYPE_CHAT:
			// TODO: draw player color;
			break;
		}

		font->showTextAsBlock ( textDest, savedReport.message );
		textDest.y += 55;

		if ( selected == (int)i )
		{
			int selIndex = selected-index*maxItems;
			SDL_Rect selDest = { position.x+15, position.y+23+55*selIndex, 1, 53 };

			SDL_FillRect ( buffer, &selDest, 0xE0E0E0 );
			selDest.x += 450;
			SDL_FillRect ( buffer, &selDest ,0xE0E0E0 );
			selDest.x -= 450;
			selDest.w = 450;
			selDest.h = 1;
			SDL_FillRect ( buffer, &selDest, 0xE0E0E0 );
			selDest.y += 53;
			SDL_FillRect ( buffer, &selDest, 0xE0E0E0 );
		}
	}
}

bool cMenuReportsScreen::checkFilter ( sUnitData &data, bool checkInclude )
{
	if ( checkInclude )
	{
		if ( data.factorAir > 0 && !filterPlanes ) return false;
		if ( data.factorGround > 0 && ( data.factorSea == 0 || !filterSea ) && !filterGround ) return false;
		if ( data.factorSea > 0 && ( data.factorGround == 0 || !filterGround ) && !filterSea ) return false;
	}

	if ( data.canBuild.empty() && filterBuild ) return false;
	if ( !data.canAttack && filterAttack ) return false;
	if ( data.hitpointsCur >= data.hitpointsMax && filterDamaged ) return false;
	if ( !data.isStealthOn && filterStealth ) return false;

	if ( data.surfacePosition != sUnitData::SURFACE_POS_GROUND ) return false;

	return true;
}


bool cMenuReportsScreen::goThroughUnits ( bool draw, int *count_, cVehicle **vehicle, cBuilding **building )
{
	bool deleteCount = false;
	int minCount = (index)*maxItems;
	int maxCount = (index+1)*maxItems;
	if ( !count_ )
	{
		count_ = new int;
		deleteCount = true;
	}
	int &count = (*count_);
	count = 0;

	SDL_Rect src = { 0, 0, 32, 32 };
	SDL_Rect dest = { position.x+17, position.y+30, 0, 0 };
	SDL_Rect nameDest = { position.x+54, position.y+25, 75, 30 };

	cVehicle *nextVehicle = vehicles;
	while ( nextVehicle && count < maxCount )
	{
		bool inFilter = checkFilter ( nextVehicle->data, true );
		if ( !inFilter || count < minCount )
		{
			nextVehicle = nextVehicle->next;
			if ( inFilter ) count++;
			continue;
		}
		if ( draw )
		{
			{ AutoSurface surface(generateUnitSurface(nextVehicle->typ->img_org[0], nextVehicle->data));
				SDL_BlitSurface(surface, &src, buffer, &dest);
			}

			font->showTextAsBlock ( nameDest, nextVehicle->name );
			unitDetails[count-minCount]->setSelection ( nextVehicle, NULL );

			font->showText ( position.x+291, position.y+35+56*(count-minCount), iToStr ( nextVehicle->PosX ) + "," + iToStr ( nextVehicle->PosY ) );
			font->showText ( position.x+343, position.y+35+56*(count-minCount), nextVehicle->getStatusStr() );
			dest.y += 55; nameDest.y += 55;
		}
		if ( vehicle && count == selected ) (*vehicle) = nextVehicle;
		count++;
		nextVehicle = nextVehicle->next;
	}

	cBuilding *nextBuilding = buildings;
	if ( filterBuilding )
	{
		while ( nextBuilding && count < maxCount )
		{
			bool inFilter = checkFilter ( nextBuilding->data, false );
			if ( !inFilter || count < minCount )
			{
				nextBuilding = nextBuilding->next;
				if ( inFilter ) count++;
				continue;
			}
			if ( draw )
			{
				{ AutoSurface surface(generateUnitSurface(nextBuilding->typ->img_org, nextBuilding->data));
					SDL_BlitSurface(surface, &src, buffer, &dest);
				}

				font->showTextAsBlock ( nameDest, nextBuilding->name );
				unitDetails[count-minCount]->setSelection ( NULL, nextBuilding );

				font->showText ( position.x+291, position.y+35+56*(count-minCount), iToStr ( nextBuilding->PosX ) + "," + iToStr ( nextBuilding->PosY ) );
				font->showText ( position.x+343, position.y+35+56*(count-minCount), nextBuilding->getStatusStr() );

				dest.y += 55; nameDest.y += 55;
			}
			if ( building && count == selected ) (*building) = nextBuilding;
			count++;
			nextBuilding = nextBuilding->next;
		}
	}

	for ( int i = 0; i < count-minCount; i++ )
	{
		unitDetails[i]->draw();
	}

	if ( count == maxCount && ( nextVehicle || nextBuilding ) )
	{
		if ( deleteCount ) delete count_;
		return true;
	}
	if ( deleteCount ) delete count_;
	return false;
}

void cMenuReportsScreen::setIncludeFilter(bool filterPlanes_, bool filterGround_, bool filterSea_, bool filterBuilding_)
{
	if ( screenType != REP_SCR_TYPE_UNITS ) return;

	filterPlanes = filterPlanes_;
	filterGround = filterGround_;
	filterSea = filterSea_;
	filterBuilding = filterBuilding_;

	index = 0;
	selected = -1;
	parentMenu->scrollCallback ( index > 0, goThroughUnits ( false ) );
};

void cMenuReportsScreen::setBorderedFilter(bool filterBuild_, bool filterAttack_, bool filterDamaged_, bool filterStealth_)
{
	if ( screenType != REP_SCR_TYPE_UNITS ) return;

	filterBuild = filterBuild_;
	filterAttack = filterAttack_;
	filterDamaged = filterDamaged_;
	filterStealth = filterStealth_;

	index = 0;
	selected = -1;
	parentMenu->scrollCallback ( index > 0, goThroughUnits ( false ) );
}

void cMenuReportsScreen::setType ( bool unitsChecked, bool disadvaChecked, bool scoreChecked, bool reportsChecked )
{
	if ( unitsChecked ) screenType = REP_SCR_TYPE_UNITS;
	else if ( disadvaChecked ) screenType = REP_SCR_TYPE_DISADVA;
	else if ( scoreChecked ) screenType = REP_SCR_TYPE_SCORE;
	else if ( reportsChecked ) screenType = REP_SCR_TYPE_REPORTS;

	index = 0;
	selected = -1;

	// we use scrollUp to update the scroll buttons
	scrollUp();
}

SDL_Surface *cMenuReportsScreen::generateUnitSurface(SDL_Surface *oriSurface, sUnitData &data )
{
	int factor = 2;
	if ( data.isBig ) factor = 4;

	SDL_Surface *surface = SDL_CreateRGBSurface ( SDL_SRCCOLORKEY, oriSurface->w/factor, oriSurface->h/factor, SettingsData.iColourDepth, 0, 0, 0, 0 );
	AutoSurface tmpSurface(SDL_CreateRGBSurface(SDL_SRCCOLORKEY, oriSurface->w/factor, oriSurface->h/factor, SettingsData.iColourDepth, 0, 0, 0, 0));

	scaleSurface ( oriSurface, tmpSurface, tmpSurface->w, tmpSurface->h );

	SDL_SetColorKey ( surface, SDL_SRCCOLORKEY, 0xFF00FF );
	if ( data.hasPlayerColor ) SDL_BlitSurface ( OtherData.colors[cl_grey], NULL, surface, NULL );
	else SDL_FillRect ( surface, NULL, 0xFF00FF );

	SDL_BlitSurface ( tmpSurface, NULL, surface, NULL );

	return surface;
}

void cMenuReportsScreen::scrollDown()
{
	switch ( screenType )
	{
	case REP_SCR_TYPE_UNITS:
		if ( goThroughUnits ( false ) ) index++;
		parentMenu->scrollCallback ( index > 0, goThroughUnits ( false ) );
		break;
	case REP_SCR_TYPE_DISADVA:
		parentMenu->scrollCallback ( false, false );
		break;
	case REP_SCR_TYPE_SCORE:
		parentMenu->scrollCallback ( false, false );
		break;
	case REP_SCR_TYPE_REPORTS:
		if ( (index+1)*maxItems < (int)owner->savedReportsList.Size() ) index++;
		parentMenu->scrollCallback ( index > 0, (index+1)*maxItems < (int)owner->savedReportsList.Size() );
		break;
	}
}

void cMenuReportsScreen::scrollUp()
{
	if ( index > 0 ) index--;
	switch ( screenType )
	{
	case REP_SCR_TYPE_UNITS:
		parentMenu->scrollCallback ( index > 0, goThroughUnits( false ) );
		break;
	case REP_SCR_TYPE_DISADVA:
		parentMenu->scrollCallback ( false, false );
		break;
	case REP_SCR_TYPE_SCORE:
		parentMenu->scrollCallback ( false, false );
		break;
	case REP_SCR_TYPE_REPORTS:
		parentMenu->scrollCallback ( index > 0, (index+1)*maxItems < (int)owner->savedReportsList.Size() );
		break;
	}
}

void cMenuReportsScreen::released( void *parent )
{
	int clickedIndex = Round ( (mouse->y-position.x-17)/55.0 )+index*maxItems;
	if ( clickedIndex >= (index+1)*maxItems ) clickedIndex = (index+1)*maxItems-1;

	switch ( screenType )
	{
	case REP_SCR_TYPE_UNITS:
		{
			int maxDisplayedUnits;
			cVehicle *vehicle = NULL;
			cBuilding *building = NULL;
			goThroughUnits ( false, &maxDisplayedUnits, &vehicle, &building );
			if ( clickedIndex == selected )
			{
				parentMenu->doubleClicked ( vehicle, building );
				return;
			}
			if ( clickedIndex >= maxDisplayedUnits ) return;
		}
		break;
	case REP_SCR_TYPE_DISADVA:
		break;
	case REP_SCR_TYPE_SCORE:
		break;
	case REP_SCR_TYPE_REPORTS:
		if ( clickedIndex > (int)owner->savedReportsList.Size() ) return;
		if ( clickedIndex == selected )
		{
			sSavedReportMessage &savedReport = owner->savedReportsList[clickedIndex];
			parentMenu->close();
			Client->addMessage ( savedReport.message );
			if ( savedReport.type == sSavedReportMessage::REPORT_TYPE_UNIT )
			{
				int offX = savedReport.xPos * 64 - ( ( int ) ( ( ( float ) (SettingsData.iScreenW - 192) / (2 * Client->gameGUI.getTileSize() ) ) * 64 ) ) + 32;
				int offY = savedReport.yPos * 64 - ( ( int ) ( ( ( float ) (SettingsData.iScreenH - 32 ) / (2 * Client->gameGUI.getTileSize() ) ) * 64 ) ) + 32;
				Client->gameGUI.setOffsetPosition ( offX, offY );
			}
			return;
		}
		break;
	}

	selected = clickedIndex;
	parentMenu->draw();
}
