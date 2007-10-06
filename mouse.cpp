//////////////////////////////////////////////////////////////////////////////
// M.A.X. - mouse.cpp
//////////////////////////////////////////////////////////////////////////////
#include "mouse.h"
#include "main.h"

// Funktionen der Maus-Klasse ////////////////////////////////////////////////
cMouse::cMouse ( void )
{
	visible=false;
	MoveCallback=false;
	MoveCallbackEditor=false;
	back=NULL;
	cur=NULL;
	LastX=-100;
}

cMouse::~cMouse ( void )
{
	if ( back!=NULL )
	{
		SDL_FreeSurface ( back );
	}
}

// Malt die Maus, und wenn draw_back gesetzt ist, auch den alten Hintergrund:
void cMouse::draw ( bool draw_back,SDL_Surface *sf )
{
	SDL_Rect dest;
	if ( !visible||cur==NULL ) return;
	GetPos();
	dest.w=back->w;
	dest.h=back->h;
	// Den Hintergrund wiederherstellen:
	if ( draw_back&&LastX!=-100 )
	{
		dest.x=LastX;
		dest.y=LastY;
		SDL_BlitSurface ( back,NULL,sf,&dest );
		/*
		dest.x=LastX;
		dest.y=LastY;
		dest.w=back->w;
		dest.h=back->h;
		*/
		if ( SettingsData.bWindowMode ) SDL_UpdateRect ( sf,dest.x,dest.y,dest.h,dest.w );
	}
	// Den Hintergrund sichern, und die Maus malen:
	GetBack ( sf );
	dest.x=DrawX;
	dest.y=DrawY;
	LastX=DrawX;
	LastY=DrawY;

	SDL_BlitSurface ( cur,NULL,sf,&dest );
	if ( SettingsData.bWindowMode ) SDL_UpdateRect ( sf,dest.x,dest.y,dest.h,dest.w );
}

// Setzt den Cursor:
bool cMouse::SetCursor ( eCursor typ )
{
	switch ( typ )
	{
		case CHand:
			if ( cur==GraphicsData.gfx_Chand ) return false;
			cur=GraphicsData.gfx_Chand;
			break;
		case CNo:
			if ( cur==GraphicsData.gfx_Cno ) return false;
			cur=GraphicsData.gfx_Cno;
			break;
		case CSelect:
			if ( cur==GraphicsData.gfx_Cselect ) return false;
			cur=GraphicsData.gfx_Cselect;
			break;
		case CMove:
			if ( cur==GraphicsData.gfx_Cmove ) return false;
			cur=GraphicsData.gfx_Cmove;
			break;
		case CPfeil1:
			if ( cur==GraphicsData.gfx_Cpfeil1 ) return false;
			cur=GraphicsData.gfx_Cpfeil1;
			break;
		case CPfeil2:
			if ( cur==GraphicsData.gfx_Cpfeil2 ) return false;
			cur=GraphicsData.gfx_Cpfeil2;
			break;
		case CPfeil3:
			if ( cur==GraphicsData.gfx_Cpfeil3 ) return false;
			cur=GraphicsData.gfx_Cpfeil3;
			break;
		case CPfeil4:
			if ( cur==GraphicsData.gfx_Cpfeil4 ) return false;
			cur=GraphicsData.gfx_Cpfeil4;
			break;
		case CPfeil6:
			if ( cur==GraphicsData.gfx_Cpfeil6 ) return false;
			cur=GraphicsData.gfx_Cpfeil6;
			break;
		case CPfeil7:
			if ( cur==GraphicsData.gfx_Cpfeil7 ) return false;
			cur=GraphicsData.gfx_Cpfeil7;
			break;
		case CPfeil8:
			if ( cur==GraphicsData.gfx_Cpfeil8 ) return false;
			cur=GraphicsData.gfx_Cpfeil8;
			break;
		case CPfeil9:
			if ( cur==GraphicsData.gfx_Cpfeil9 ) return false;
			cur=GraphicsData.gfx_Cpfeil9;
			break;
		case CHelp:
			if ( cur==GraphicsData.gfx_Chelp ) return false;
			cur=GraphicsData.gfx_Chelp;
			break;
		case CAttack:
			if ( cur==GraphicsData.gfx_Cattack ) return false;
			cur=GraphicsData.gfx_Cattack;
			break;
		case CBand:
			if ( cur==GraphicsData.gfx_Cband ) return false;
			cur=GraphicsData.gfx_Cband;
			break;
		case CTransf:
			if ( cur==GraphicsData.gfx_Ctransf ) return false;
			cur=GraphicsData.gfx_Ctransf;
			break;
		case CLoad:
			if ( cur==GraphicsData.gfx_Cload ) return false;
			cur=GraphicsData.gfx_Cload;
			break;
		case CMuni:
			if ( cur==GraphicsData.gfx_Cmuni ) return false;
			cur=GraphicsData.gfx_Cmuni;
			break;
		case CRepair:
			if ( cur==GraphicsData.gfx_Crepair ) return false;
			cur=GraphicsData.gfx_Crepair;
			break;
		case CSteal:
			if ( cur==GraphicsData.gfx_Csteal ) return false;
			cur=GraphicsData.gfx_Csteal;
			break;
		case CDisable:
			if ( cur==GraphicsData.gfx_Cdisable ) return false;
			cur=GraphicsData.gfx_Cdisable;
			break;
		case CActivate:
			if ( cur==GraphicsData.gfx_Cactivate ) return false;
			cur=GraphicsData.gfx_Cactivate;
			break;
	}
	if ( back!=NULL )
	{
		SDL_Surface *tmp;
		tmp=SDL_CreateRGBSurface ( SDL_HWSURFACE|SDL_SRCCOLORKEY,cur->w,cur->h,32,0,0,0,0 );
		SDL_BlitSurface ( back,NULL,tmp,NULL );
		SDL_FreeSurface ( back );
		back=tmp;
	}
	else
	{
		back=SDL_CreateRGBSurface ( SDL_HWSURFACE|SDL_SRCCOLORKEY,cur->w,cur->h,32,0,0,0,0 );
	}
	return true;
}

// Liest den Hintergrund in das Back-Surface ein:
void cMouse::GetBack ( SDL_Surface *sf )
{
	SDL_Rect scr;
	GetPos();
	scr.x=DrawX;
	scr.y=DrawY;
	scr.w=back->w;
	scr.h=back->h;
	SDL_BlitSurface ( sf,&scr,back,NULL );
}

// Liest die aktuelle Mausposition aus:
void cMouse::GetPos ( void )
{
	SDL_GetMouseState ( &x,&y );

	// Cursor Offset bestimmen:
	if ( cur==GraphicsData.gfx_Cselect||cur==GraphicsData.gfx_Chelp||cur==GraphicsData.gfx_Cmove||cur==GraphicsData.gfx_Cno||cur==GraphicsData.gfx_Ctransf||cur==GraphicsData.gfx_Cband||cur==GraphicsData.gfx_Cload||cur==GraphicsData.gfx_Cmuni||cur==GraphicsData.gfx_Crepair )
	{
		DrawX=x-12;
		DrawY=y-12;
	}
	else if ( cur==GraphicsData.gfx_Cattack||cur==GraphicsData.gfx_Csteal||cur==GraphicsData.gfx_Cdisable )
	{
		DrawX=x-19;
		DrawY=y-16;
	}
	else
	{
		if ( x>SettingsData.iScreenW-cur->w )
		{
			x=SettingsData.iScreenW-cur->w;
			SDL_WarpMouse ( x,y );
		}
		if ( y>SettingsData.iScreenH-cur->h )
		{
			y=SettingsData.iScreenH-cur->h;
			SDL_WarpMouse ( x,y );
		}
		DrawX=x;
		DrawY=y;
	}
	if ( MoveCallback )
	{
		MouseMoveCallback ( false );
	}/*else if(MoveCallbackEditor){
	    MouseMoveCallbackEditor();
	  }*/
}

// Liefert die Koordinaten der Kachel unter der Maus:
void cMouse::GetKachel ( int *X,int *Y )
{
	cHud *hud;
	if ( x<180||y<18||x>180+ ( SettingsData.iScreenW-192 ) ||y>18+ ( SettingsData.iScreenH-32 ) ) {*X=-1;*Y=-1;return;}
	hud=game->hud;
	*X= (int)(( ( x-180 ) +hud->OffX/ ( 64.0/hud->Zoom ) ) /hud->Zoom);
	*Y= (int)(( ( y-18 ) +hud->OffY/ ( 64.0/hud->Zoom ) ) /hud->Zoom);
	if ( *X>=game->map->size ) *X=game->map->size-1;
	if ( *Y>=game->map->size ) *Y=game->map->size-1;
}

// Liefert den Offset der Kachel unter der Maus:
int cMouse::GetKachelOff ( void )
{
	cHud *hud;
	int ret;
	if ( x<180||y<18||x>180+ ( SettingsData.iScreenW-192 ) ||y>18+ ( SettingsData.iScreenH-32 ) ) return -1;
	hud=game->hud;
	ret= (int)(( ( x-180 ) +hud->OffX/ ( 64.0/hud->Zoom ) ) /hud->Zoom);
	ret+= ( ( int ) ( ( ( y-18 ) +hud->OffY/ ( 64.0/hud->Zoom ) ) /hud->Zoom ) ) *game->map->size;
	return ret;
}
