//////////////////////////////////////////////////////////////////////////////
// M.A.X. - prefer.cpp
//////////////////////////////////////////////////////////////////////////////
#include "prefer.h"
#include "mouse.h"
#include "sound.h"
#include "keyinp.h"
#include "fonts.h"

// DoPraeferenzen ////////////////////////////////////////////////////////////
// Zeigt das Präferenzenfenster an:
void DoPraeferenzen(void){
  bool OldMusicMute,OldSoundMute,OldVoiceMute,OldAutosave,OldAnimation,OldSchatten,OldAlpha,OldDamageEffects,OldDamageEffectsVehicles,OldMakeTracks;
  bool FertigPressed=false,AbbruchPressed=false,Input=false;
  int OldScrollSpeed,OldMusicVol,OldSoundVol,OldVoiceVol;
  int LastMouseX=0,LastMouseY=0,LastB=0,x,y,b;
  string OldName;
  string stmp;
  SDL_Rect scr,dest;
  Uint8 *keystate;
  bool cursor=true;

  OldMusicMute=MusicMute;OldSoundMute=SoundMute;OldVoiceMute=VoiceMute;OldAutosave=Autosave;OldAnimation=Animation;OldSchatten=Schatten;OldAlpha=Alpha;
  OldScrollSpeed=ScrollSpeed;OldMusicVol=MusicVol;OldSoundVol=SoundVol;OldVoiceVol=VoiceVol;
  OldName=game->ActivePlayer->name;
  OldDamageEffects=DamageEffects;OldDamageEffectsVehicles=DamageEffectsVehicles;OldMakeTracks=MakeTracks;

  dest.x=120;
  dest.y=29;
  dest.w=gfx_praefer->w;
  dest.h=gfx_praefer->h;
  SDL_BlitSurface(gfx_hud,NULL,buffer,NULL);
  if(Alpha)SDL_BlitSurface(gfx_shadow,NULL,buffer,NULL);
  SDL_BlitSurface(gfx_praefer,NULL,buffer,&dest);

  ShowBar(74+120,81+29,MusicVol*2);
  ShowBar(84+120,101+29,SoundVol*2);
  ShowBar(80+120,121+29,VoiceVol*2);
  ShowBar(121+120,261+29,ScrollSpeed*5);
  SetButton(210+120,73+29,MusicMute);
  SetButton(210+120,93+29,SoundMute);
  SetButton(210+120,113+29,VoiceMute);
  SetButton(25+120,133+29,Autosave);
  SetButton(25+120,193+29,Animation);
  SetButton(25+120,213+29,Schatten);
  SetButton(25+120,233+29,Alpha);
  SetButton(210+120,193+29,DamageEffects);
  SetButton(210+120,213+29,DamageEffectsVehicles);
  SetButton(210+120,233+29,MakeTracks);
  fonts->OutText((char *)game->ActivePlayer->name.c_str(),122+120,158+29,buffer);
  SHOW_SCREEN

  mouse->GetBack(buffer);
  while(1){
    // Die Engine laufen lassen:
    game->engine->Run();
    game->HandleTimer();

    // Events holen:
    SDL_PumpEvents();

    // Tasten prüfen:
    keystate=SDL_GetKeyState(NULL);
    if(Input){
      if(DoKeyInp(keystate)||timer2){
        scr.x=116;
        scr.y=154;
        dest.w=scr.w=184;
        dest.h=scr.h=17;
        dest.x=116+120;
        dest.y=154+29;
        SDL_BlitSurface(gfx_praefer,&scr,buffer,&dest);
        if(InputEnter){
          fonts->OutText((char *)InputStr.c_str(),122+120,158+29,buffer);
          Input=false;
          game->ActivePlayer->name=InputStr;
        }else{
		  stmp = InputStr; stmp += "_";
          if(fonts->GetTextLen((char *) stmp.c_str())>178){
		    InputStr.erase(InputStr.length()-1);
          }
          if(cursor){
		    stmp = InputStr; stmp += "_";
            fonts->OutText((char *)stmp.c_str(),122+120,158+29,buffer);
          }else{
            fonts->OutText((char *) InputStr.c_str(),122+120,158+29,buffer);
          }
          if(timer2)cursor=!cursor;
        }
        SHOW_SCREEN
        mouse->draw(false,screen);
      }
    }
    // Die Maus machen:
    mouse->GetPos();
    b=mouse->GetMouseButton();
    x=mouse->x;y=mouse->y;
    if(x!=LastMouseX||y!=LastMouseY){
      mouse->draw(true,screen);
    }
    if(b){
      if(x>=74+120&&x<74+120+57&&y>=81+29-7&&y<=81+29+10&&(x!=LastMouseX||y!=LastMouseY||!LastB)){
        MusicVol=(x-(74+120))*(128.0/57);
        if(MusicVol>=125)MusicVol=128;
        ShowBar(74+120,81+29,MusicVol*2);
        SHOW_SCREEN
        mouse->draw(false,screen);
        SetMusicVol(MusicVol);
      }else if(x>=84+120&&x<84+120+57&&y>=101+29-7&&y<=101+29+10&&(x!=LastMouseX||y!=LastMouseY||!LastB)){
        SoundVol=(x-(84+120))*(128.0/57);
        if(SoundVol>=125)SoundVol=128;
        ShowBar(84+120,101+29,SoundVol*2);
        SHOW_SCREEN
        mouse->draw(false,screen);
      }else if(x>=80+120&&x<80+120+57&&y>=121+29-7&&y<=121+29+10&&(x!=LastMouseX||y!=LastMouseY||!LastB)){
        VoiceVol=(x-(80+120))*(128.0/57);
        if(VoiceVol>=125)VoiceVol=128;
        ShowBar(80+120,121+29,VoiceVol*2);
        SHOW_SCREEN
        mouse->draw(false,screen);
      }else if(x>=121+120&&x<121+120+57&&y>=261+29-7&&y<=261+29+10&&(x!=LastMouseX||y!=LastMouseY||!LastB)){
        ScrollSpeed=(int)((x-(121+120))*(255.0/57))/5;
        ShowBar(121+120,261+29,ScrollSpeed*5);
        SHOW_SCREEN
        mouse->draw(false,screen);
      }
      if(!LastB){
        if(x>=210+120&&x<210+120+18&&y>=73+29&&y<73+29+17){
          MusicMute=!MusicMute;
          SetButton(210+120,73+29,MusicMute);
          SHOW_SCREEN
          mouse->draw(false,screen);
          if(MusicMute){
            StopMusic();
          }else{
            StartMusic();          
          }
        }else if(x>=210+120&&x<210+120+18&&y>=93+29&&y<93+29+17){
          SoundMute=!SoundMute;
          SetButton(210+120,93+29,SoundMute);
          SHOW_SCREEN
          mouse->draw(false,screen);
        }else if(x>=210+120&&x<210+120+18&&y>=113+29&&y<113+29+17){
          VoiceMute=!VoiceMute;
          SetButton(210+120,113+29,VoiceMute);
          SHOW_SCREEN
          mouse->draw(false,screen);
        }else if(x>=25+120&&x<25+120+18&&y>=133+29&&y<133+29+17){
          Autosave=!Autosave;
          SetButton(25+120,133+29,Autosave);
          SHOW_SCREEN
          mouse->draw(false,screen);
        }else if(x>=25+120&&x<25+120+18&&y>=193+29&&y<193+29+17){
          Animation=!Animation;
          SetButton(25+120,193+29,Animation);
          SHOW_SCREEN
          mouse->draw(false,screen);
        }else if(x>=25+120&&x<25+120+18&&y>=213+29&&y<213+29+17){
          Schatten=!Schatten;
          SetButton(25+120,213+29,Schatten);
          SHOW_SCREEN
          mouse->draw(false,screen);
        }else if(x>=25+120&&x<25+120+18&&y>=233+29&&y<233+29+17){
          Alpha=!Alpha;
          SetButton(25+120,233+29,Alpha);
          SHOW_SCREEN
          mouse->draw(false,screen);
        }else if(x>=116+120&&x<116+120+184&&y>=154+29&&y<154+29+17&&!Input){
          Input=true;
          InputStr=game->ActivePlayer->name;
		  stmp = InputStr; stmp += "_";
          fonts->OutText((char *) stmp.c_str(),122+120,158+29,buffer);
          SHOW_SCREEN
          mouse->draw(false,screen);
        }else if(x>=210+120&&x<210+120+18&&y>=193+29&&y<193+29+17){
          DamageEffects=!DamageEffects;
          SetButton(210+120,193+29,DamageEffects);
          SHOW_SCREEN
          mouse->draw(false,screen);
        }else if(x>=210+120&&x<210+120+18&&y>=213+29&&y<213+29+17){
          DamageEffectsVehicles=!DamageEffectsVehicles;
          SetButton(210+120,213+29,DamageEffectsVehicles);
          SHOW_SCREEN
          mouse->draw(false,screen);
        }else if(x>=210+120&&x<210+120+18&&y>=233+29&&y<233+29+17){
          MakeTracks=!MakeTracks;
          SetButton(210+120,233+29,MakeTracks);
          SHOW_SCREEN
          mouse->draw(false,screen);
        }
      }
    }
    // Fertig-Button:
    if(x>=215+120&&x<215+120+63&&y>=383+29&&y<382+29+24){
      if(b&&!FertigPressed){
        PlayFX(SNDMenuButton);      
        scr.x=68;
        scr.y=172;
        dest.w=scr.w=63;
        dest.h=scr.h=24;
        dest.x=215+120;
        dest.y=383+29;
        SDL_BlitSurface(gfx_hud_stuff,&scr,buffer,&dest);
        SHOW_SCREEN
        mouse->draw(false,screen);
        FertigPressed=true;
      }else if(!b&&LastB){
        if(Input){
          game->ActivePlayer->name=InputStr;
        }
		if(strcmp(game->ActivePlayer->name.c_str(),OldName.c_str())!=0){
          game->engine->ChangePlayerName(game->ActivePlayer->name);
        }
        return;
      }
    }else if(FertigPressed){
      scr.x=215;
      scr.y=383;
      dest.w=scr.w=63;
      dest.h=scr.h=24;
      dest.x=215+120;
      dest.y=383+29;
      SDL_BlitSurface(gfx_praefer,&scr,buffer,&dest);
      SHOW_SCREEN
      mouse->draw(false,screen);
      FertigPressed=false;
    }
    // Abbruch-Button:
    if(x>=125+120&&x<125+120+63&&y>=383+29&&y<382+29+24){
      if(b&&!AbbruchPressed){
        PlayFX(SNDMenuButton);
        scr.x=0;
        scr.y=190;
        dest.w=scr.w=63;
        dest.h=scr.h=24;
        dest.x=125+120;
        dest.y=383+29;
        SDL_BlitSurface(gfx_hud_stuff,&scr,buffer,&dest);
        SHOW_SCREEN
        mouse->draw(false,screen);
        AbbruchPressed=true;
      }else if(!b&&LastB){
        MusicMute=OldMusicMute;SoundMute=OldSoundMute;VoiceMute=OldVoiceMute;Autosave=OldAutosave;Animation=OldAnimation;Schatten=OldSchatten;Alpha=OldAlpha;
        ScrollSpeed=OldScrollSpeed;MusicVol=OldMusicVol;SoundVol=OldSoundVol;VoiceVol=OldVoiceVol;
        DamageEffects=OldDamageEffects;DamageEffectsVehicles=OldDamageEffectsVehicles;MakeTracks=OldMakeTracks;
        game->ActivePlayer->name=OldName;
        SetMusicVol(MusicVol);
        return;
      }
    }else if(AbbruchPressed){
      scr.x=125;
      scr.y=383;
      dest.w=scr.w=63;
      dest.h=scr.h=24;
      dest.x=125+120;
      dest.y=383+29;
      SDL_BlitSurface(gfx_praefer,&scr,buffer,&dest);
      SHOW_SCREEN
      mouse->draw(false,screen);
      AbbruchPressed=false;
    }


    LastMouseX=x;LastMouseY=y;
    LastB=b;
    SDL_Delay(1);
  }
}

void ShowBar(int offx,int offy,int value){
  SDL_Rect scr,dest;
  scr.x=offx-120-6;
  scr.y=offy-29-7;
  dest.w=scr.w=57+14;
  dest.h=scr.h=17;
  dest.x=offx-6;
  dest.y=offy-7;
  SDL_BlitSurface(gfx_praefer,&scr,buffer,&dest);

  scr.x=412;
  scr.y=46;
  dest.w=scr.w=14;
  dest.x=offx-6+(int)((57/255.0)*value);
  dest.y=offy-7;
  SDL_BlitSurface(gfx_hud_stuff,&scr,buffer,&dest);
}

void SetButton(int offx,int offy,bool set){
  SDL_Rect scr,dest;
  scr.x=393;
  if(!set){
    scr.y=46;
  }else{
    scr.y=64;
  }
  dest.w=scr.w=18;
  dest.h=scr.h=17;
  dest.x=offx;
  dest.y=offy;
  SDL_BlitSurface(gfx_hud_stuff,&scr,buffer,&dest);
}
