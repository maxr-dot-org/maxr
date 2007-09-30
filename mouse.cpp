//////////////////////////////////////////////////////////////////////////////
// M.A.X. - mouse.cpp
//////////////////////////////////////////////////////////////////////////////
#include "mouse.h"
#include "main.h"

// Funktionen der Maus-Klasse ////////////////////////////////////////////////
cMouse::cMouse(void){
  visible=false;
  MoveCallback=false;
  MoveCallbackEditor=false;
  back=NULL;
  cur=NULL;
  LastX=-100;
}

cMouse::~cMouse(void){
  if(back!=NULL){
    SDL_FreeSurface(back);
  }
}

// Malt die Maus, und wenn draw_back gesetzt ist, auch den alten Hintergrund:
void cMouse::draw(bool draw_back,SDL_Surface *sf){
	SDL_Rect dest;
	if(!visible||cur==NULL)return;
	GetPos();
	dest.w=back->w;
	dest.h=back->h; 
	// Den Hintergrund wiederherstellen:
	if(draw_back&&LastX!=-100)
	{
		dest.x=LastX;
		dest.y=LastY;
		SDL_BlitSurface(back,NULL,sf,&dest);
		/*
		dest.x=LastX;
		dest.y=LastY;
		dest.w=back->w;
		dest.h=back->h;
		*/
		if(WindowMode)SDL_UpdateRect(sf,dest.x,dest.y,dest.h,dest.w);
	}
	// Den Hintergrund sichern, und die Maus malen:
	GetBack(sf);
	dest.x=DrawX;
	dest.y=DrawY;
	LastX=DrawX;
	LastY=DrawY;

	SDL_BlitSurface(cur,NULL,sf,&dest);
	if(WindowMode)SDL_UpdateRect(sf,dest.x,dest.y,dest.h,dest.w);
}

// Setzt den Cursor:
bool cMouse::SetCursor(eCursor typ){
  switch(typ){
    case CHand:
      if(cur==gfx_Chand)return false;
      cur=gfx_Chand;
      break;
    case CNo:
      if(cur==gfx_Cno)return false;
      cur=gfx_Cno;
      break;
    case CSelect:
      if(cur==gfx_Cselect)return false;
      cur=gfx_Cselect;
      break;
    case CMove:
      if(cur==gfx_Cmove)return false;
      cur=gfx_Cmove;
      break;
    case CPfeil1:
      if(cur==gfx_Cpfeil1)return false;
      cur=gfx_Cpfeil1;
      break;
    case CPfeil2:
      if(cur==gfx_Cpfeil2)return false;
      cur=gfx_Cpfeil2;
      break;
    case CPfeil3:
      if(cur==gfx_Cpfeil3)return false;
      cur=gfx_Cpfeil3;
      break;
    case CPfeil4:
      if(cur==gfx_Cpfeil4)return false;
      cur=gfx_Cpfeil4;
      break;
    case CPfeil6:
      if(cur==gfx_Cpfeil6)return false;
      cur=gfx_Cpfeil6;
      break;
    case CPfeil7:
      if(cur==gfx_Cpfeil7)return false;
      cur=gfx_Cpfeil7;
      break;
    case CPfeil8:
      if(cur==gfx_Cpfeil8)return false;
      cur=gfx_Cpfeil8;
      break;
    case CPfeil9:
      if(cur==gfx_Cpfeil9)return false;
      cur=gfx_Cpfeil9;
      break;
    case CHelp:
      if(cur==gfx_Chelp)return false;
      cur=gfx_Chelp;
      break;
    case CAttack:
      if(cur==gfx_Cattack)return false;
      cur=gfx_Cattack;
      break;
    case CBand:
      if(cur==gfx_Cband)return false;
      cur=gfx_Cband;
      break;
    case CTransf:
      if(cur==gfx_Ctransf)return false;
      cur=gfx_Ctransf;
      break;
    case CLoad:
      if(cur==gfx_Cload)return false;
      cur=gfx_Cload;
      break;
    case CMuni:
      if(cur==gfx_Cmuni)return false;
      cur=gfx_Cmuni;
      break;
    case CRepair:
      if(cur==gfx_Crepair)return false;
      cur=gfx_Crepair;
      break;
    case CSteal:
      if(cur==gfx_Csteal)return false;
      cur=gfx_Csteal;
      break;
    case CDisable:
      if(cur==gfx_Cdisable)return false;
      cur=gfx_Cdisable;
      break;
    case CActivate:
      if(cur==gfx_Cactivate)return false;
      cur=gfx_Cactivate;
      break;
  }
  if(back!=NULL){
    SDL_Surface *tmp;
    tmp=SDL_CreateRGBSurface(SDL_HWSURFACE|SDL_SRCCOLORKEY,cur->w,cur->h,32,0,0,0,0);
    SDL_BlitSurface(back,NULL,tmp,NULL);
    SDL_FreeSurface(back);
    back=tmp;
  }else{
    back=SDL_CreateRGBSurface(SDL_HWSURFACE|SDL_SRCCOLORKEY,cur->w,cur->h,32,0,0,0,0);
  }
  return true;
}

// Liest den Hintergrund in das Back-Surface ein:
void cMouse::GetBack(SDL_Surface *sf){
  SDL_Rect scr;
  GetPos();
  scr.x=DrawX;
  scr.y=DrawY;
  scr.w=back->w;
  scr.h=back->h;
  SDL_BlitSurface(sf,&scr,back,NULL);
}

// Liest die aktuelle Mausposition aus:
void cMouse::GetPos(void){
  SDL_GetMouseState(&x,&y);

  // Cursor Offset bestimmen:
  if(cur==gfx_Cselect||cur==gfx_Chelp||cur==gfx_Cmove||cur==gfx_Cno||cur==gfx_Ctransf||cur==gfx_Cband||cur==gfx_Cload||cur==gfx_Cmuni||cur==gfx_Crepair){
    DrawX=x-12;
    DrawY=y-12;
  }else if(cur==gfx_Cattack||cur==gfx_Csteal||cur==gfx_Cdisable){
    DrawX=x-19;
    DrawY=y-16;
  }else{
    if(x>ScreenW-cur->w){
      x=ScreenW-cur->w;
      SDL_WarpMouse(x,y);
    }
    if(y>ScreenH-cur->h){
      y=ScreenH-cur->h;
      SDL_WarpMouse(x,y);
    }
    DrawX=x;
    DrawY=y;
  }
  if(MoveCallback){
    MouseMoveCallback(false);
  }/*else if(MoveCallbackEditor){
    MouseMoveCallbackEditor();
  }*/
}

// Liefert die Koordinaten der Kachel unter der Maus:
void cMouse::GetKachel(int *X,int *Y){
  cHud *hud;
  if(x<180||y<18||x>180+(ScreenW-192)||y>18+(ScreenH-32)){*X=-1;*Y=-1;return;}
  hud=game->hud;
  *X=((x-180)+hud->OffX/(64.0/hud->Zoom))/hud->Zoom;
  *Y=((y-18)+hud->OffY/(64.0/hud->Zoom))/hud->Zoom;
  if(*X>=game->map->size)*X=game->map->size-1;
  if(*Y>=game->map->size)*Y=game->map->size-1;  
}

// Liefert den Offset der Kachel unter der Maus:
int cMouse::GetKachelOff(void){
  cHud *hud;
  int ret;
  if(x<180||y<18||x>180+(ScreenW-192)||y>18+(ScreenH-32))return -1;
  hud=game->hud;
  ret=((x-180)+hud->OffX/(64.0/hud->Zoom))/hud->Zoom;
  ret+=((int)(((y-18)+hud->OffY/(64.0/hud->Zoom))/hud->Zoom))*game->map->size;
  return ret;
}