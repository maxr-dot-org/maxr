//////////////////////////////////////////////////////////////////////////////
// M.A.X. - dialog.cpp
//////////////////////////////////////////////////////////////////////////////
#include <SDL.h>
//#include <dirent.h>
#include "dialog.h"
#include "game.h"
#include "mouse.h"
#include "keyinp.h"
#include "fonts.h"
#include "sound.h"
#include "menu.h"
#include "pcx.h"

// Zeigt einen Ja/Nein Dialog an:
bool ShowYesNo(string text){
  int b,x,y,lx=0,ly=0,lb=0;
  bool ret=false;
  SDL_Rect dest;

  mouse->SetCursor(CHand);
  game->DrawMap(false);
  SDL_BlitSurface(GraphicsData.gfx_hud,NULL,buffer,NULL);

  if(cSettingsData.bAlphaEffects){
    SDL_BlitSurface(GraphicsData.gfx_shadow,NULL,buffer,NULL);
  }
  LoadPCXtoSF((char *) GraphicsData.Dialog2Path.c_str(),GraphicsData.gfx_dialog);
  dest.x=640/2-300/2;
  dest.y=480/2-231/2;
  dest.w=300;
  dest.h=231;
  SDL_BlitSurface(GraphicsData.gfx_dialog,NULL,buffer,&dest);
  PlaceSmallButton("Ja",640/2-300/2+80,480/2-231/2+150,false);
  PlaceSmallButton("Nein",640/2-300/2+80,480/2-231/2+185,false);
  dest.x+=20;dest.w-=40;
  dest.y+=20;dest.h-=150;
  fonts->OutTextBlock((char *) text.c_str(),dest,buffer);
  SHOW_SCREEN
  mouse->draw(false,screen);

  while(1){
    if(game){
      game->engine->Run();
      game->HandleTimer();
    }

    // Eingaben holen:
    SDL_PumpEvents();
    // Die Maus:
    mouse->GetPos();
    b=mouse->GetMouseButton();
    x=mouse->x;
    y=mouse->y;
    if(lx!=x||ly!=y){
      mouse->draw(true,screen);
    }

    // Ja Button:
    if(x>=640/2-300/2+80&&x<640/2-300/2+80+150&&y>=480/2-231/2+150&&y<480/2-231/2+150+29){
      if(b&&!lb){
        PlayFX(SNDHudButton);
        PlaceSmallButton("Ja",640/2-300/2+80,480/2-231/2+150,true);
        SHOW_SCREEN
        mouse->draw(false,screen);
        ret=true;
        break;
      }
    }
    // Nein Button:
    if(x>=640/2-300/2+80&&x<640/2-300/2+80+150&&y>=480/2-231/2+185&&y<480/2-231/2+185+29){
      if(b&&!lb){
        PlayFX(SNDHudButton);
        PlaceSmallButton("Nein",640/2-300/2+80,480/2-231/2+185,true);
        SHOW_SCREEN
        mouse->draw(false,screen);
        ret=false;
        break;
      }
    }

    lx=x;
    ly=y;
    lb=b;
    SDL_Delay(1);
  }

  LoadPCXtoSF((char *) GraphicsData.DialogPath.c_str(),GraphicsData.gfx_dialog);
  game->fDrawMap=true;
  return ret;
}

// Zeigt einen Dialog für eine Zahleneingabe an:
int ShowNumberInput(string text){
  int b,x,y,lx=0,ly=0,lb=0;
  int value=2;
  SDL_Rect dest,scr;
  Uint8 *keystate;
  bool Cursor = true;
  string stmp;

  LoadPCXtoSF((char *) GraphicsData.Dialog3Path.c_str(),GraphicsData.gfx_dialog);
  dest.x=640/2-300/2;
  dest.y=480/2-231/2;
  dest.w=300;
  dest.h=231;
  SDL_BlitSurface(GraphicsData.gfx_dialog,NULL,buffer,&dest);
  PlaceSmallButton("OK",640/2-300/2+80,480/2-231/2+185,false);
  dest.x+=20;dest.w-=40;
  dest.y+=20;dest.h-=150;
  fonts->OutTextBlock((char *) text.c_str(),dest,buffer);
  fonts->OutText("_",20+170+7,167+124+4,buffer);
  InputStr="";
  SHOW_SCREEN
  mouse->draw(false,screen);

  while(1){
    // Eingaben holen:
    SDL_PumpEvents();
    // Die Maus:
    mouse->GetPos();
    b=mouse->GetMouseButton();
    x=mouse->x;
    y=mouse->y;
    if(lx!=x||ly!=y){
      mouse->draw(true,screen);
    }
    // Die Tastatur:
    keystate=SDL_GetKeyState(NULL);
    if(DoKeyInp(keystate)||timer2){
      scr.x=20;
      scr.y=167;
      dest.w=scr.w=259;
      dest.h=scr.h=17;
      dest.x=20+170;
      dest.y=167+124;
      SDL_BlitSurface(GraphicsData.gfx_dialog,&scr,buffer,&dest);
	  stmp = InputStr; stmp += "_";
      if(fonts->GetTextLen((char *) stmp.c_str())>=246){
		  InputStr.erase(InputStr.length()-1, 0);
      }
      if(Cursor){
        Cursor=false;
		stmp = InputStr; stmp += "_";
        fonts->OutText((char *) stmp.c_str(),dest.x+7,dest.y+4,buffer);
      }else{
        Cursor=true;
        fonts->OutText((char *) InputStr.c_str(),dest.x+7,dest.y+4,buffer);
      }
      SHOW_SCREEN
      mouse->draw(false,screen);
    }
    // OK Button:
    if(x>=640/2-300/2+80&&x<640/2-300/2+80+150&&y>=480/2-231/2+185&&y<480/2-231/2+185+29){
      if(b&&!lb){
        PlayFX(SNDHudButton);
        PlaceSmallButton("OK",640/2-300/2+80,480/2-231/2+185,true);
        SHOW_SCREEN
        mouse->draw(false,screen);
        break;
      }
    }

    lx=x;
    ly=y;
    lb=b;
    SDL_Delay(1);
  }

  LoadPCXtoSF((char *)  GraphicsData.DialogPath.c_str(),GraphicsData.gfx_dialog);
  value=atoi(InputStr.c_str());
  return value;
}

void ShowOK(string text,bool pure){
  int b,x,y,lx=0,ly=0,lb=0;
  SDL_Rect dest;

  mouse->SetCursor(CHand);
  if(!pure){
    dest.x=180;dest.y=18;
    dest.w=dest.h=448;
    SDL_FillRect(buffer,&dest,0);
    SDL_BlitSurface(GraphicsData.gfx_hud,NULL,buffer,NULL);
    dest.x=15;dest.y=356;
    dest.w=dest.h=112;
    SDL_FillRect(buffer,&dest,0);
    if(cSettingsData.bAlphaEffects){
      SDL_BlitSurface(GraphicsData.gfx_shadow,NULL,buffer,NULL);
    }
  }
  LoadPCXtoSF((char *) GraphicsData.Dialog2Path.c_str(),GraphicsData.gfx_dialog);
  dest.x=640/2-300/2;
  dest.y=480/2-231/2;
  dest.w=300;
  dest.h=231;
  SDL_BlitSurface(GraphicsData.gfx_dialog,NULL,buffer,&dest);
  PlaceSmallButton("OK",640/2-300/2+80,480/2-231/2+185,false);
  dest.x+=20;dest.w-=40;
  dest.y+=20;dest.h-=150;
  fonts->OutTextBlock((char *) text.c_str(),dest,buffer);
  SHOW_SCREEN
  mouse->draw(false,screen);

  while(1){
    if(!pure&&game){
      game->engine->Run();
      game->HandleTimer();
    }

    // Eingaben holen:
    SDL_PumpEvents();
    // Die Maus:
    mouse->GetPos();
    b=mouse->GetMouseButton();
    x=mouse->x;
    y=mouse->y;
    if(lx!=x||ly!=y){
      mouse->draw(true,screen);
    }

    // OK Button:
    if(x>=640/2-300/2+80&&x<640/2-300/2+80+150&&y>=480/2-231/2+185&&y<480/2-231/2+185+29){
      if(b&&!lb){
        PlayFX(SNDHudButton);
        PlaceSmallButton("OK",640/2-300/2+80,480/2-231/2+185,true);
        SHOW_SCREEN
        mouse->draw(false,screen);
        break;
      }
    }

    lx=x;
    ly=y;
    lb=b;
    SDL_Delay(1);
  }

  LoadPCXtoSF((char *) GraphicsData.DialogPath.c_str(),GraphicsData.gfx_dialog);
  if(!pure)game->fDrawMap=true;
}
