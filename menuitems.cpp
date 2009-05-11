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
#include "menuitems.h"
#include "menus.h"

cMenuItem::cMenuItem ( int x, int y )
{
	position.x = x;
	position.y = y;
	position.w = position.h = 0;

	isClicked = false;
	wasClicked = false;
	locked = false;

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
	if ( x >= position.x && x <= position.x+position.w &&
		y >= position.y && y <= position.y+position.h )
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
		isClicked = false;
		wasClicked = false;
		if ( releaseSound ) PlayFX ( releaseSound );
		if ( release ) release(parent);
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


cMenuImage::cMenuImage ( int x, int y, SDL_Surface *image_ ) : cMenuItem( x, y ), image (NULL)
{
	setImage ( image_ );
}

cMenuImage::~cMenuImage()
{
	if ( image ) SDL_FreeSurface ( image );
}

void cMenuImage::setImage(SDL_Surface *image_)
{
	if ( image_ != image && image != NULL ) SDL_FreeSurface ( image );

	if ( image_ != NULL )
	{
		image = SDL_CreateRGBSurface( SDL_HWSURFACE | SDL_SRCCOLORKEY, image_->w, image_->h, SettingsData.iColourDepth, 0, 0, 0, 0 );

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

cMenuLabel::cMenuLabel ( int x, int y, string text_, eUnicodeFontType fontType_ ) : cMenuItem( x, y ), fontType( fontType_ )
{
	textPosition.x = position.x;
	textPosition.y = position.y;
	textPosition.w = textPosition.h = 0;
	setText ( text_ );
	flagCentered = false;
	flagBox = false;
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

void cMenuLabel::setCentered( bool centered )
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

cMenuButton::cMenuButton ( int x, int y, string text_, eButtonTypes buttonType_, eUnicodeFontType fontType_, sSOUND *clickSound_ ) : cMenuItem( x, y ), fontType( fontType_ ), buttonType( buttonType_ ), surface( NULL )
{
	clickSound = clickSound_;
	text = text_;
	renewButtonSurface();
}

cMenuButton::~cMenuButton()
{
	if ( surface ) SDL_FreeSurface ( surface );
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
	}
	if ( surface ) SDL_FreeSurface ( surface );
	surface = SDL_CreateRGBSurface ( SDL_HWSURFACE | SDL_SRCCOLORKEY, src.w, src.h , SettingsData.iColourDepth, 0, 0, 0, 0 );
	SDL_SetColorKey ( surface, SDL_SRCCOLORKEY, 0xFF00FF );
	SDL_FillRect ( surface, NULL, 0xFF00FF );
	SDL_BlitSurface ( GraphicsData.gfx_menu_stuff, &src, surface, NULL );
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
	case BUTTON_TYPE_ARROW_UP_SMALL:
	case BUTTON_TYPE_ARROW_DOWN_SMALL:
	case BUTTON_TYPE_ARROW_LEFT_SMALL:
	case BUTTON_TYPE_ARROW_RIGHT_SMALL:
	case BUTTON_TYPE_ARROW_UP_BAR:
	case BUTTON_TYPE_ARROW_DOWN_BAR:
		return -1;
	case BUTTON_TYPE_ANGULAR:
		if ( isClicked ) return 5;
		else return 4;
	}
}

bool cMenuButton::preClicked()
{
	redraw();
	return true;
}

bool cMenuButton::preReleased()
{
	if ( isClicked || wasClicked )
	{
		isClicked = false;
		redraw();
		return true;
	}
	return false;
}

bool cMenuButton::preHoveredOn()
{
	if ( wasClicked ) preClicked();
	return true;
}

bool cMenuButton::preHoveredAway()
{
	if ( isClicked ) preReleased();
	return true;
}

void cMenuButton::draw()
{
	if ( surface )
	{
		SDL_BlitSurface ( surface, NULL, buffer, &position );
		font->showTextCentered ( position.x+position.w/2, position.y+getTextYOffset(), text, fontType );
	}
}

bool cMenuButton::preSetLocked( bool locked_ )
{
	isClicked = locked_;
	renewButtonSurface();
	return true;
}

cMenuCheckButton::cMenuCheckButton ( int x, int y, string text_, bool checked_, bool centered_, eCheckButtonTypes buttonType_, eCheckButtonTextOriantation textOrientation_, eUnicodeFontType fontType_, sSOUND *clickSound_ )
: cMenuItem( x, y ),
  fontType( fontType_ ),
  buttonType( buttonType_ ),
  textOrientation ( textOrientation_ ),
  surface( NULL )
{
	clickSound = clickSound_;
	checked = checked_;
	centered = centered_;
	text = text_;
	group = NULL;
	renewButtonSurface();
	if ( centered ) position.x -= position.w/2;
}

void cMenuCheckButton::renewButtonSurface()
{
	SDL_Rect src = {0,0,0,0};
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
		if ( surface ) SDL_FreeSurface ( surface );
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
		break;
	}
	if ( src.w > 0 )
	{
		if ( surface ) SDL_FreeSurface ( surface );
		surface = SDL_CreateRGBSurface ( SDL_HWSURFACE, src.w, src.h , SettingsData.iColourDepth, 0, 0, 0, 0 );
		SDL_BlitSurface ( GraphicsData.gfx_menu_stuff, &src, surface, NULL );
	}
}

bool cMenuCheckButton::preClicked()
{
	switch ( buttonType )
	{
	default:
	case CHECKBOX_TYPE_STANDARD:
	case CHECKBOX_TYPE_TANK:
	case CHECKBOX_TYPE_PLANE:
	case CHECKBOX_TYPE_SHIP:
	case CHECKBOX_TYPE_BUILD:
	case CHECKBOX_TYPE_TNT:
		checked = !checked;
		renewButtonSurface();
		break;
	case RADIOBTN_TYPE_BTN_ROUND:
	case RADIOBTN_TYPE_TEXT_ONLY:
	case RADIOBTN_TYPE_ANGULAR_BUTTON:
		checked = true;
		if ( group ) group->checkedButton ( this );
		else renewButtonSurface();
		break;
	}
	draw();
	SHOW_SCREEN
	mouse->draw ( false, screen );
	return true;
}

void cMenuCheckButton::draw()
{
	SDL_Rect textDest;
	if ( surface )
	{
		switch ( this->textOrientation )
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

cMenuUnitListItem::cMenuUnitListItem( sID unitID_, cPlayer *owner_, sUnitUpgrade *upgrades_, eMenuUnitListDisplayTypes displayType_, cMenuUnitsList* parent, bool fixedResValue_ ) : cMenuItem (0,0), unitID(unitID_), owner(owner_), upgrades(upgrades_)
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
		if ( building->data.is_big ) scaleSurface ( building->img_org, building->img, building->img_org->w/4, building->img_org->h/4 );
		else scaleSurface ( building->img_org, building->img, building->img_org->w/2, building->img_org->h/2 );
		surface = SDL_CreateRGBSurface ( SDL_SRCCOLORKEY, building->img->w, building->img->h, SettingsData.iColourDepth, 0, 0, 0, 0 );
		SDL_SetColorKey ( surface, SDL_SRCCOLORKEY, 0xFF00FF );
		if ( !building->data.is_connector && !building->data.is_road ) SDL_BlitSurface ( OtherData.colors[cl_grey], NULL, surface, NULL );
		else SDL_FillRect ( surface, NULL, 0xFF00FF );
		SDL_BlitSurface ( building->img, NULL, surface, NULL );
	}
	else surface = NULL;

	selected = false;
	marked = false;
	resValue = 0;
	minResValue = -1;
	fixed = false;

	displayType = displayType_;
	fixedResValue = fixedResValue_;
	parentList = parent;
}

cMenuUnitListItem::~cMenuUnitListItem()
{
	if ( surface ) SDL_FreeSurface ( surface );
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

			if ( unitID.getVehicle() ) font->showTextCentered( position.x+position.w-12, dest.y, iToStr(unitID.getUnitData()->iBuilt_Costs), FONT_LATIN_SMALL_YELLOW );
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
	string name = ((string)unitID.getUnitData()->name);
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

	if ( unitID.getUnitData()->can_transport == TRANS_METAL || unitID.getUnitData()->can_transport == TRANS_OIL /*|| unitID.getUnitData()->can_transport == TRANS_GOLD*/ ) // don't allow buying gold
	{
		if( resValue == 0 ) font->showText( dest.x, dest.y+10, "(empty)", FONT_LATIN_SMALL_WHITE );
		else if( resValue <= unitID.getUnitData()->max_cargo / 4 ) font->showText( dest.x, dest.y+10, " (" + iToStr(resValue) + "/" + iToStr(unitID.getUnitData()->max_cargo) + ")", FONT_LATIN_SMALL_RED );
		else if( resValue <= unitID.getUnitData()->max_cargo / 2 ) font->showText( dest.x, dest.y+10, " (" + iToStr(resValue) + "/" + iToStr(unitID.getUnitData()->max_cargo) + ")", FONT_LATIN_SMALL_YELLOW );
		else font->showText( dest.x, dest.y+10, " (" + iToStr(resValue) + "/" + iToStr(unitID.getUnitData()->max_cargo) + ")", FONT_LATIN_SMALL_GREEN );
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
	if ( cargoCheck && resValue > unitID.getUnitData()->max_cargo ) resValue = unitID.getUnitData()->max_cargo;
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

cMenuUnitListItem *cMenuUnitsList::addUnit ( sID unitID, cPlayer *owner, sUnitUpgrade *upgrades, bool scroll, bool fixedCargo )
{
	cMenuUnitListItem *unitItem = new cMenuUnitListItem( unitID, owner, upgrades, displayType, this, fixedCargo );
	unitItem->setReleaseSound ( SoundData.SNDObjectMenu );
	unitItem->position.h = 32;
	unitItem->position.w = position.w-20;
	unitsList.Add ( unitItem );
	if ( selectedUnit ) selectedUnit->selected = false;
	selectedUnit = unitItem;
	selectedUnit->selected = true;
	if ( scroll && (int)unitsList.Size() > offset+maxDisplayUnits ) scrollDown();
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

cMenuUnitDetails::cMenuUnitDetails( int x, int y, cHangarMenu *parent ) : cMenuItem (x,y), parentMenu(parent)
{
	position.w = 246;
	position.h = 176;
	selectedUnit = NULL;
}

void cMenuUnitDetails::draw()
{
	if ( !selectedUnit ) return;
	sUnitData *data = selectedUnit->getUnitID().getUnitData ( selectedUnit->getOwner() );
	sUnitData *oriData = selectedUnit->getUnitID().getUnitData ();

#define DETAIL_COLUMN_1 dest.x+27
#define DETAIL_COLUMN_2 dest.x+42
#define DETAIL_COLUMN_3 dest.x+95
#define DETAIL_DOLINEBREAK dest.y = y + 14; SDL_FillRect ( buffer, &dest, 0xFC0000 ); y += 19;

	SDL_Rect dest = { position.x, position.y, position.w, 1 };
	int y;
	y = dest.y;

	if ( data->can_attack )
	{
		// Damage:
		font->showTextCentered ( DETAIL_COLUMN_1, y, iToStr ( data->damage ) );
		font->showText ( DETAIL_COLUMN_2, y, lngPack.i18n ( "Text~Vehicles~Damage" ) );
		drawBigSymbol ( MENU_SYMBOLS_BIG_ATTACK, DETAIL_COLUMN_3 , y - 3, 160, data->damage, oriData->damage );
		DETAIL_DOLINEBREAK

		if ( !data->is_expl_mine )
		{
			// Shots:
			font->showTextCentered ( DETAIL_COLUMN_1, y, iToStr ( data->max_shots ) );
			font->showText ( DETAIL_COLUMN_2, y, lngPack.i18n ( "Text~Vehicles~Shoots" ) );
			drawBigSymbol ( MENU_SYMBOLS_BIG_SHOTS, DETAIL_COLUMN_3, y + 2, 160, data->max_shots, oriData->max_shots );
			DETAIL_DOLINEBREAK

			// Range:
			font->showTextCentered ( DETAIL_COLUMN_1, y, iToStr ( data->range ) );
			font->showText ( DETAIL_COLUMN_2, y, lngPack.i18n ( "Text~Vehicles~Range" ) );
			drawBigSymbol ( MENU_SYMBOLS_BIG_RANGE, DETAIL_COLUMN_3, y - 2, 160, data->range, oriData->range );
			DETAIL_DOLINEBREAK

			// Ammo:
			font->showTextCentered ( DETAIL_COLUMN_1, y, iToStr ( data->max_ammo ) );
			font->showText ( DETAIL_COLUMN_2, y, lngPack.i18n ( "Text~Vehicles~Ammo" ) );
			drawBigSymbol ( MENU_SYMBOLS_BIG_AMMO, DETAIL_COLUMN_3, y - 2, 160, data->max_ammo, oriData->max_ammo );
			DETAIL_DOLINEBREAK
		}
	}

	int transport;
	if ( selectedUnit->getUnitID().getVehicle() ) transport = data->can_transport;
	else transport = data->can_load;

	if ( transport == TRANS_METAL || transport == TRANS_OIL || transport == TRANS_GOLD )
	{
		font->showTextCentered ( DETAIL_COLUMN_1, y, iToStr ( data->max_cargo ) );
		font->showText ( DETAIL_COLUMN_2, y, lngPack.i18n ( "Text~Vehicles~Cargo" ) );

		switch ( transport )
		{

			case TRANS_METAL:
				drawBigSymbol ( MENU_SYMBOLS_BIG_METAL, DETAIL_COLUMN_3 , y - 2, 160, data->max_cargo, oriData->max_cargo );
				break;

			case TRANS_OIL:
				drawBigSymbol ( MENU_SYMBOLS_BIG_OIL, DETAIL_COLUMN_3 , y - 2, 160, data->max_cargo, oriData->max_cargo );
				break;

			case TRANS_GOLD:
				drawBigSymbol ( MENU_SYMBOLS_BIG_GOLD, DETAIL_COLUMN_3 , y - 2, 160, data->max_cargo, oriData->max_cargo );
				break;
		}

		DETAIL_DOLINEBREAK
	}

	if ( data->energy_prod )
	{
		// Eneryproduction:
		font->showTextCentered ( DETAIL_COLUMN_1, y, iToStr ( data->energy_prod ) );
		font->showText ( DETAIL_COLUMN_2, y, lngPack.i18n ( "Text~Vehicles~Produce" ) );
		drawBigSymbol ( MENU_SYMBOLS_BIG_ENERGY, DETAIL_COLUMN_3, y - 2, 160, data->energy_prod, oriData->energy_prod );
		DETAIL_DOLINEBREAK

		// Oil consumption:
		font->showTextCentered ( DETAIL_COLUMN_1, y, iToStr ( data->oil_need ) );
		font->showText ( DETAIL_COLUMN_2, y, lngPack.i18n ( "Text~Vehicles~Usage" ) );
		drawBigSymbol ( MENU_SYMBOLS_BIG_OIL, DETAIL_COLUMN_3, y - 2, 160, data->oil_need, oriData->oil_need );
		DETAIL_DOLINEBREAK
	}

	if ( data->human_prod )
	{
		// Humanproduction:
		font->showText ( DETAIL_COLUMN_1, y, iToStr ( data->human_prod ) );
		font->showText ( DETAIL_COLUMN_2, y, lngPack.i18n ( "Text~Vehicles~Produce" ) );
		drawBigSymbol ( MENU_SYMBOLS_BIG_HUMAN, DETAIL_COLUMN_3, y - 2, 160, data->human_prod, oriData->human_prod );
		DETAIL_DOLINEBREAK
	}

	// Armor:
	font->showTextCentered ( DETAIL_COLUMN_1, y, iToStr ( data->armor ) );
	font->showText ( DETAIL_COLUMN_2, y, lngPack.i18n ( "Text~Vehicles~Armor" ) );
	drawBigSymbol ( MENU_SYMBOLS_BIG_ARMOR, DETAIL_COLUMN_3, y - 2, 160, data->armor, oriData->armor );
	DETAIL_DOLINEBREAK

	// Hitpoints:
	font->showTextCentered ( DETAIL_COLUMN_1, y, iToStr ( data->max_hit_points ) );
	font->showText ( DETAIL_COLUMN_2, y, lngPack.i18n ( "Text~Vehicles~Hitpoints" ) );
	drawBigSymbol ( MENU_SYMBOLS_BIG_HITS, DETAIL_COLUMN_3 , y - 1, 160, data->max_hit_points, oriData->max_hit_points );
	DETAIL_DOLINEBREAK

	// Scan:
	if ( data->scan )
	{
		font->showTextCentered ( DETAIL_COLUMN_1, y, iToStr ( data->scan ) );
		font->showText ( DETAIL_COLUMN_2, y, lngPack.i18n ( "Text~Vehicles~Scan" ) );
		drawBigSymbol ( MENU_SYMBOLS_BIG_SCAN, DETAIL_COLUMN_3 , y - 2, 160, data->scan, oriData->scan );
		DETAIL_DOLINEBREAK
	}

	// Speed:
	if ( data->max_speed )
	{
		font->showTextCentered ( DETAIL_COLUMN_1, y, iToStr ( data->max_speed / 4 ) ); //FIXME: might crash if e.g. max_speed = 3
		font->showText ( DETAIL_COLUMN_2, y, lngPack.i18n ( "Text~Vehicles~Speed" ) );
		drawBigSymbol ( MENU_SYMBOLS_BIG_SPEED, DETAIL_COLUMN_3 , y - 2, 160, data->max_speed / 4, oriData->max_speed / 4 );
		DETAIL_DOLINEBREAK
	}

	// energy consumption:
	if ( data->energy_need )
	{
		font->showTextCentered ( DETAIL_COLUMN_1, y, iToStr ( data->energy_need ) );
		font->showText ( DETAIL_COLUMN_2, y, lngPack.i18n ( "Text~Vehicles~Usage" ) );
		drawBigSymbol ( MENU_SYMBOLS_BIG_ENERGY, DETAIL_COLUMN_3, y - 2, 160, data->energy_need, oriData->energy_need );
		DETAIL_DOLINEBREAK
	}

	// humans needed:
	if ( data->human_need )
	{
		font->showTextCentered ( DETAIL_COLUMN_1, y, iToStr ( data->human_need ) );
		font->showText ( DETAIL_COLUMN_2, y, lngPack.i18n ( "Text~Vehicles~Produce" ) );
		drawBigSymbol ( MENU_SYMBOLS_BIG_HUMAN, DETAIL_COLUMN_3, y - 2, 160, data->human_need, oriData->human_need );
		DETAIL_DOLINEBREAK
	}

	// raw material consumption:
	if ( data->metal_need )
	{
		font->showTextCentered ( DETAIL_COLUMN_1, y, iToStr ( data->metal_need ) );
		font->showText ( DETAIL_COLUMN_2, y, lngPack.i18n ( "Text~Vehicles~Usage" ) );
		drawBigSymbol ( MENU_SYMBOLS_BIG_METAL, DETAIL_COLUMN_3, y - 2, 160, data->metal_need, oriData->metal_need );
		DETAIL_DOLINEBREAK
	}

	// gold consumption:
	if ( data->gold_need )
	{
		font->showTextCentered ( DETAIL_COLUMN_1, y, iToStr ( data->gold_need ) );
		font->showText ( DETAIL_COLUMN_2, y, lngPack.i18n ( "Text~Vehicles~Usage" ) );
		drawBigSymbol ( MENU_SYMBOLS_BIG_GOLD, DETAIL_COLUMN_3, y - 2, 160, data->gold_need, oriData->gold_need );
		DETAIL_DOLINEBREAK
	}
	
	// Costs:
	font->showTextCentered ( DETAIL_COLUMN_1, y, iToStr ( data->iBuilt_Costs ) );
	font->showText ( DETAIL_COLUMN_2, y, lngPack.i18n ( "Text~Vehicles~Costs" ) );
	drawBigSymbol ( MENU_SYMBOLS_BIG_METAL, DETAIL_COLUMN_3 , y - 2, 160, data->iBuilt_Costs, oriData->iBuilt_Costs );
}

void cMenuUnitDetails::drawBigSymbol ( eMenuSymbolsBig sym, int x, int y, int maxx, int value, int orgvalue )
{
	SDL_Rect src, dest;
	int offx;

	switch ( sym )
	{
		case MENU_SYMBOLS_BIG_SPEED:
			src.x = 0;
			src.y = 109;
			src.w = 11;
			src.h = 12;
			break;
		case MENU_SYMBOLS_BIG_HITS:
			src.x = 11;
			src.y = 109;
			src.w = 7;
			src.h = 11;
			break;
		case MENU_SYMBOLS_BIG_AMMO:
			src.x = 18;
			src.y = 109;
			src.w = 9;
			src.h = 14;
			break;
		case MENU_SYMBOLS_BIG_ATTACK:
			src.x = 27;
			src.y = 109;
			src.w = 10;
			src.h = 14;
			break;
		case MENU_SYMBOLS_BIG_SHOTS:
			src.x = 37;
			src.y = 109;
			src.w = 15;
			src.h = 7;
			break;
		case MENU_SYMBOLS_BIG_RANGE:
			src.x = 52;
			src.y = 109;
			src.w = 13;
			src.h = 13;
			break;
		case MENU_SYMBOLS_BIG_ARMOR:
			src.x = 65;
			src.y = 109;
			src.w = 11;
			src.h = 14;
			break;
		case MENU_SYMBOLS_BIG_SCAN:
			src.x = 76;
			src.y = 109;
			src.w = 13;
			src.h = 13;
			break;
		case MENU_SYMBOLS_BIG_METAL:
			src.x = 89;
			src.y = 109;
			src.w = 12;
			src.h = 15;
			break;
		case MENU_SYMBOLS_BIG_OIL:
			src.x = 101;
			src.y = 109;
			src.w = 11;
			src.h = 12;
			break;
		case MENU_SYMBOLS_BIG_GOLD:
			src.x = 112;
			src.y = 109;
			src.w = 13;
			src.h = 10;
			break;
		case MENU_SYMBOLS_BIG_ENERGY:
			src.x = 125;
			src.y = 109;
			src.w = 13;
			src.h = 17;
			break;
		case MENU_SYMBOLS_BIG_HUMAN:
			src.x = 138;
			src.y = 109;
			src.w = 12;
			src.h = 16;
			break;
	}

	maxx -= src.w;

	if ( orgvalue < value ) maxx -= src.w + 3;

	offx = src.w;

	while ( offx*value > maxx )
	{
		offx--;

		if ( offx < 4 )
		{
			value /= 2;
			orgvalue /= 2;
			offx = src.w;
		}
	}

	dest.x = x;
	dest.y = y;

	for ( int i = 0; i < value;i++ )
	{
		if ( i == orgvalue )
		{
			SDL_Rect mark;
			dest.x += src.w + 3;
			mark.x = dest.x - src.w / 2;
			mark.y = dest.y;
			mark.w = 1;
			mark.h = src.h;
			SDL_FillRect ( buffer, &mark, 0xFC0000 );
		}

		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff, &src, buffer, &dest );

		dest.x += offx;
	}
}

void cMenuUnitDetails::setSelection(cMenuUnitListItem *selectedUnit_)
{
	selectedUnit = selectedUnit_;
	//parentMenu->selectionChanged();
	draw();
}

cMenuMaterialBar::cMenuMaterialBar( int x, int y, int labelX, int labelY, int maxValue_, eMaterialBarTypes materialType_ ) : cMenuItem ( x, y )
{
	setReleaseSound ( SoundData.SNDObjectMenu );
	currentValue = maxValue = maxValue_;
	materialType = materialType_;
	position.w = 20;
	position.h = 115;
	valueLabel = new cMenuLabel ( labelX, labelY, iToStr( currentValue ) );
	valueLabel->setCentered ( true );
	generateSurface();
}

cMenuMaterialBar::~cMenuMaterialBar()
{
	if ( surface ) SDL_FreeSurface ( surface );
	delete valueLabel;
}

void cMenuMaterialBar::generateSurface()
{
	SDL_Rect src = { 114, 336, position.w, position.h };
	surface = SDL_CreateRGBSurface ( SDL_HWSURFACE, src.w, src.h , SettingsData.iColourDepth, 0, 0, 0, 0 );

	switch ( materialType )
	{
	default:
	case MAT_BAR_TYPE_METAL:
	case MAT_BAR_TYPE_OIL:
		src.x += src.w+1;
		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff, &src, surface, NULL );
		break;
	case MAT_BAR_TYPE_GOLD:
		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff, &src, surface, NULL );
		break;
	}
}

void cMenuMaterialBar::draw()
{
	if ( currentValue <= 0 && maxValue <= 0 ) return;
	int height = (int)((float)currentValue/maxValue*surface->h);
	SDL_Rect src = { 0, 0, surface->w, height };
	SDL_Rect dest = { position.x, position.y+(surface->h-height), surface->w, height };
	SDL_BlitSurface ( surface, &src, buffer, &dest );

	valueLabel->draw();
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

cMenuUpgradeHanlder::cMenuUpgradeHanlder ( int x, int y, cStartupHangarMenu *parent ) : cMenuItemContainer ( x, y ), parentMenu(parent)
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

cMenuUpgradeHanlder::~cMenuUpgradeHanlder()
{
	for ( int i = 0; i < 8; i++ )
	{
		delete decreaseButtons[i];
		delete increaseButtons[i];
		delete costsLabel[i];
	}
}


void cMenuUpgradeHanlder::buttonReleased( void* parent )
{
	cMenuUpgradeHanlder *This = (cMenuUpgradeHanlder *)parent;
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
			This->updateUnitValues( This->selection );

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

			This->updateUnitValues( This->selection );
			upgrades[i].purchased--;

			This->setSelection ( This->selection );
			This->parentMenu->draw();
		}
	}
}

void cMenuUpgradeHanlder::setSelection ( cMenuUnitListItem *selection_ )
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
	cPlayer *owner = selection->getOwner();
	for ( int i = 0; i < 8; i++ )
	{
		if ( upgrade[i].type != sUnitUpgrade::UPGRADE_TYPE_NONE ) costsLabel[i]->setText ( iToStr (upgrade[i].nextPrice) );
		else costsLabel[i]->setText ( "" );

		if ( upgrade[i].type != sUnitUpgrade::UPGRADE_TYPE_NONE && parentMenu->getCredits() >= upgrade[i].nextPrice ) increaseButtons[i]->setLocked ( false );
		else increaseButtons[i]->setLocked ( true );

		if ( upgrade[i].type != sUnitUpgrade::UPGRADE_TYPE_NONE && upgrade[i].purchased > 0 ) decreaseButtons[i]->setLocked ( false );
		else decreaseButtons[i]->setLocked ( true );
	}
}

cUpgradeCalculator::UpgradeTypes cMenuUpgradeHanlder::getUpgradeType( sUnitUpgrade upgrade )
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

void cMenuUpgradeHanlder::updateUnitValues ( cMenuUnitListItem *unit )
{
	for ( unsigned int i = 0; i < UnitsData.vehicle.Size()+UnitsData.building.Size(); i++ )
	{
		sUnitData *data;
		if ( i < UnitsData.vehicle.Size() ) data = &unit->getOwner()->VehicleData[i];
		else data = &unit->getOwner()->BuildingData[i-UnitsData.vehicle.Size()];

		if ( data->ID == unit->getUnitID() )
		{
			sUnitUpgrade *upgrades = unit->getUpgrades();
			for ( int j = 0; j < 8; j++ )
			{
				sUnitUpgrade *upgrades = unit->getUpgrades();
				if ( upgrades[j].purchased > 0 )
				{
					switch ( upgrades[j].type )
					{
						case sUnitUpgrade::UPGRADE_TYPE_DAMAGE:
							data->damage = upgrades[j].curValue;
							break;
						case sUnitUpgrade::UPGRADE_TYPE_SHOTS:
							data->max_shots = upgrades[j].curValue;
							break;
						case sUnitUpgrade::UPGRADE_TYPE_RANGE:
							data->range = upgrades[j].curValue;
							break;
						case sUnitUpgrade::UPGRADE_TYPE_AMMO:
							data->max_ammo = upgrades[j].curValue;
							break;
						case sUnitUpgrade::UPGRADE_TYPE_ARMOR:
							data->armor = upgrades[j].curValue;
							break;
						case sUnitUpgrade::UPGRADE_TYPE_HITS:
							data->max_hit_points = upgrades[j].curValue;
							break;
						case sUnitUpgrade::UPGRADE_TYPE_SCAN:
							data->scan = upgrades[j].curValue;
							break;
						case sUnitUpgrade::UPGRADE_TYPE_SPEED:
							data->max_speed = upgrades[j].curValue;
							break;
					}
				}
			}
			break;
		}
	}
}

cMenuScroller::cMenuScroller ( int x, int y ) : cMenuItem ( x, y )
{
	position.w = 17;
	position.h = 14;

	SDL_Rect src = { 201, 35, position.w, position.h };
	surface = SDL_CreateRGBSurface( SDL_HWSURFACE, position.w, position.h, SettingsData.iColourDepth, 0, 0, 0, 0 );
	SDL_BlitSurface ( GraphicsData.gfx_menu_stuff, &src, surface, NULL );
}

cMenuScroller::~cMenuScroller()
{
	if ( surface ) SDL_FreeSurface ( surface );
}

void cMenuScroller::draw()
{
	SDL_BlitSurface ( surface, NULL, buffer, &position );
}

void cMenuScroller::move ( int y )
{
	position.y = y;
};

cMenuScrollBar::cMenuScrollBar ( int x, int y, int h, int pageSteps_, cMenu *parentMenu_, cMenuItem *parentItem_ ) : cMenuItemContainer ( x, y ), pageSteps(pageSteps_), parentMenu(parentMenu_), parentItem(parentItem_)
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
	scroller = new cMenuScroller ( position.x, position.y+17 );
	itemList.Add ( upButton );
}

cMenuScrollBar::~cMenuScrollBar()
{
	if ( surface ) SDL_FreeSurface ( surface );
	delete upButton;
	delete downButton;
	delete scroller;
}

void cMenuScrollBar::createSurface()
{
	SDL_Rect src = { 234, 1, 16, 48};
	SDL_Rect dest = { 0, 0, 0, 0 };
	surface = SDL_CreateRGBSurface( SDL_HWSURFACE, 16, position.h-28, SettingsData.iColourDepth, 0, 0, 0, 0 );
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
	scroller->position.y = position.y+17+offset*scrollerSteps;
	if ( scroller->position.y > position.y+position.h-17-14 ) scroller->position.y = position.y+position.h-17-14;
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

cMenuListBox::cMenuListBox ( int x, int y, int w, int h, int maxLines_, cMenu *parentMenu_ ) : cMenuItemContainer ( x, y ), maxLines(maxLines_), parentMenu(parentMenu_)
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
	while ( font->getTextWide( text.substr( startOffset, endOffset-startOffset ) ) > position.w-12 ) doPosDecrease ( endOffset );
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

	if ( font->getTextWide( text.substr( startOffset, text.length()-startOffset ) ) > position.w-12 )
	{
		endOffset = (int)text.length();
		while ( font->getTextWide( text.substr( startOffset, endOffset-startOffset ) ) > position.w-12 ) doPosDecrease ( endOffset );
	}
}

void cMenuLineEdit::scrollRight()
{
	// makes the cursor go right
	if ( cursorPos < (int)text.length() ) doPosIncrease ( cursorPos, cursorPos );
	while ( cursorPos > endOffset ) doPosIncrease ( endOffset, endOffset );
	while ( font->getTextWide( text.substr( startOffset, endOffset-startOffset ) ) > position.w-12 ) doPosIncrease ( startOffset, startOffset );
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

void cMenuLineEdit::handleKeyInput( SDL_keysym keysym, string ch, void *parent )
{
	if ( readOnly ) return;

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
				while ( font->getTextWide( text.substr( startOffset, endOffset-startOffset ) ) > position.w-12 ) doPosIncrease ( startOffset, startOffset );
			}
			else
			{
				if ( font->getTextWide( text.substr( startOffset, endOffset-startOffset ) ) > position.w-12 ) doPosDecrease ( endOffset );
				else doPosIncrease ( endOffset, cursorPos );
			}
			if ( wasKeyInput ) wasKeyInput ( parent );
		}
		break;
	}
	parentMenu->draw();
}

void cMenuLineEdit::setReturnPressedFunc( void (*returnPressed_)(void *) )
{
	returnPressed = returnPressed_;
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

	SDL_Surface *readySurface = SDL_CreateRGBSurface ( SDL_HWSURFACE | SDL_SRCCOLORKEY, src.w, src.h , SettingsData.iColourDepth, 0, 0, 0, 0 );
	SDL_SetColorKey ( readySurface, SDL_SRCCOLORKEY, 0xFF00FF );
	SDL_FillRect ( readySurface, NULL, 0xFF00FF );
	SDL_BlitSurface ( GraphicsData.gfx_player_ready, &src, readySurface, NULL );

	src.x -= 10;
	SDL_Surface *notReadySurface = SDL_CreateRGBSurface ( SDL_HWSURFACE | SDL_SRCCOLORKEY, src.w, src.h , SettingsData.iColourDepth, 0, 0, 0, 0 );
	SDL_SetColorKey ( notReadySurface, SDL_SRCCOLORKEY, 0xFF00FF );
	SDL_FillRect ( notReadySurface, NULL, 0xFF00FF );
	SDL_BlitSurface ( GraphicsData.gfx_player_ready, &src, notReadySurface, NULL );

	for ( int i = scrollBar->offset; i < scrollBar->offset+maxDrawPlayers; i++ )
	{
		if ( i < (int)players->Size() )
		{
			if ( (*players)[i]->ready ) playerReadys[i-scrollBar->offset]->setImage ( readySurface );
			else playerReadys[i-scrollBar->offset]->setImage ( notReadySurface );

			SDL_Surface *colorSurface = SDL_CreateRGBSurface ( SDL_HWSURFACE | SDL_SRCCOLORKEY, src.w, src.h , SettingsData.iColourDepth, 0, 0, 0, 0 );
			SDL_BlitSurface ( OtherData.colors[(*players)[i]->color], &src, colorSurface, NULL );
			playerColors[i-scrollBar->offset]->setImage ( colorSurface );
			SDL_FreeSurface ( colorSurface );

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
	SDL_FreeSurface ( readySurface );
	SDL_FreeSurface ( notReadySurface );
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
