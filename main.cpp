//////////////////////////////////////////////////////////////////////////////
// M.A.X. - main.cpp
//////////////////////////////////////////////////////////////////////////////
#define TIXML_USE_STL

#include <windows.h>  
#include <io.h>
#include <math.h>

#include <SDL.h>
#include <SDL_thread.h>
#include <SDL_net.h>
#include <tinyxml.h>
#include "SDL_mixer.h"
#define __main__
#include "defines.h"
#include "main.h"
#include "files.h"
#include "mouse.h"
#include "menu.h"
#include "pcx.h"
#include "fonts.h"
#include "keyinp.h"
#include "keys.h"
#include "sound.h"
#include "map.h"
#include "prefer.h"
#include "game.h"
#include "buildings.h"
#include "vehicles.h"
#include "player.h"
#include "base.h"
#include "ajobs.h"
#include "mjobs.h"
#include "fstcpip.h"

struct _finddata_t c_file;

TList::TList(void) {
	Count = 0;
}

// WINMAIN Function
int WINAPI WinMain(HINSTANCE hInst,HINSTANCE hPrevInstance,LPSTR args,INT)
{
	srand( (unsigned)time( NULL ) );
	SDL_Thread *thread = NULL;
	putenv("SDL_VIDEO_CENTERED=center");

	// Pfad der .exe ermitteln
	char PathTmp[MAX_PATH+2];
	GetModuleFileNameA(NULL, PathTmp, MAX_PATH);
	AppPath=PathTmp;
	AppPath.erase(AppPath.find_last_of("\\"));

	// *Tests*

	// SDL starten
	if( SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_NOPARACHUTE | SDL_INIT_AUDIO) == -1)
		return 0;

	// SplashScreen anzeigen
	buffer=SDL_LoadBMP("InitPopup.bmp");
	SDL_BlitSurface(buffer,NULL,screen,NULL);
	screen=SDL_SetVideoMode(500,420,32,SDL_HWSURFACE|SDL_NOFRAME);
	SDL_UpdateRect(screen,0,0,0,0);
	SDL_WM_SetCaption("MMs M.A.X. by DoctorDeath",NULL);

	// SDL_Net starten
	if( SDLNet_Init() == -1)
		return 0;

	// Ini-Datei lesen
	if(!FileExists("max.ini")){
		MessageBox(NULL,L"max.ini not found",L"LoadIni",MB_ICONERROR);
		return 0;
	}
	// Einstellungen aus der Ini laden:
	TiXmlDocument doc;
	TiXmlNode* rootnode;
	if(!doc.LoadFile("max.xml")){
		MessageBoxA(NULL,"max.xml kann nicht geladen werden!","Error",MB_ICONERROR);
		return 0;
	}
	rootnode = doc.FirstChildElement("MAXOptions")->FirstChildElement("StartOptions");

	FastMode=GetXMLBool(rootnode,"fastmode");;
	WindowMode=GetXMLBool(rootnode,"windowmode");
	NoIntro=GetXMLBool(rootnode,"nointro");
	switch(atoi(rootnode->FirstChildElement("resolution")->FirstChild()->ValueStr().c_str())){
		default:
		case 0: ScreenW=640;ScreenH=480;break;
		case 1: ScreenW=800;ScreenH=600;break;
		case 2: ScreenW=1024;ScreenH=768;break;
		case 3: ScreenW=1280;ScreenH=960;break;
	}
	// Dateien laden
	thread = SDL_CreateThread(LoadData,NULL);

	SDL_WaitThread(thread, NULL);

	// Prüfen, ob noch genug Speicher frei ist:
	{
		_MEMORYSTATUS buff;
		GlobalMemoryStatus(&buff);
		if((buff.dwTotalPhys/1024)/1024<32){
			MessageBoxA(NULL,"not enough memory!","Fatal Error",0);
			return -1;
		}
	}

	// IniDatei auslesen:
	rootnode = doc.FirstChildElement("MAXOptions")->FirstChildElement("GameOptions");
	ScrollSpeed = atoi(rootnode->FirstChildElement("ScrollSpeed")->FirstChild()->ValueStr().c_str());
	MusicVol = atoi(rootnode->FirstChildElement("MusicVol")->FirstChild()->ValueStr().c_str());
	SoundVol = atoi(rootnode->FirstChildElement("SoundVol")->FirstChild()->ValueStr().c_str());
	VoiceVol = atoi(rootnode->FirstChildElement("VoiceVol")->FirstChild()->ValueStr().c_str());
	MusicMute = GetXMLBool(rootnode,"MusicMute");
	SoundMute = GetXMLBool(rootnode,"SoundMute");
	VoiceMute = GetXMLBool(rootnode,"VoiceMute");
	Autosave = GetXMLBool(rootnode,"Autosave");
	Animation = GetXMLBool(rootnode,"Animation");
	Schatten = GetXMLBool(rootnode,"Schatten");
	Alpha = GetXMLBool(rootnode,"Alpha");
	DamageEffects = GetXMLBool(rootnode,"DamageEffects");
	DamageEffectsVehicles = GetXMLBool(rootnode,"DamageEffectsVehicles");
	MakeTracks = GetXMLBool(rootnode,"MakeTracks");
	ShowBeschreibung = GetXMLBool(rootnode,"ShowBeschreibung");
	// MapPath=AppPath+ini->ReadString("max","maps","maps");
	LastIP = rootnode->FirstChildElement("LastIP")->FirstChild()->ValueStr();
	LastPort = atoi(rootnode->FirstChildElement("LastPort")->FirstChild()->ValueStr().c_str());
	LastPlayerName = rootnode->FirstChildElement("LastPlayerName")->FirstChild()->ValueStr();

	MapPath = "maps\\";
	SavePath = "save\\";

	// Musik starten
	PlayMusic("music\\main.ogg");

	// Intro

	// Den Grafikmodus starten:
	buffer=SDL_CreateRGBSurface(SDL_HWSURFACE|SDL_SRCCOLORKEY,ScreenW,ScreenH,32,0,0,0,0);
	if(!WindowMode) {
		screen=SDL_SetVideoMode(ScreenW,ScreenH,32,SDL_HWSURFACE|SDL_FULLSCREEN);
	} else {
		screen=SDL_SetVideoMode(ScreenW,ScreenH,32,SDL_HWSURFACE);
	}
	SDL_FillRect(buffer,NULL,0);
	SDL_ShowCursor(0); // Den Cursor verstecken.
	// Die Caption muss neu gesetzt werden:
	SDL_WM_SetCaption("MMs M.A.X. by DoctorDeath",NULL);

	// Die Maus erzeugen:
	mouse = new cMouse;

	// Das Menü starten:
	RunMainMenu();

	// Alles löschen und beenden:
	delete mouse;
	if(sound){
		DeleteVoices();
		DeleteSounds();
		DeleteMusic();
	}
	DeleteBuildings();
	DeleteVehicles();
	DeleteFonts();
	DeleteTerrain();
	DeleteFX();
	DeleteGFX();
	SDL_Quit();
	SDLNet_Quit();
	if(sound) CloseSound();
	/*
	// Die Einstellungen speichern:
	WriteIniInteger("max","ScrollSpeed",ScrollSpeed,"max.ini");
	WriteIniInteger("max","MusicVol",MusicVol,"max.ini");
	WriteIniInteger("max","SoundVol",SoundVol,"max.ini");
	WriteIniInteger("max","VoiceVol",VoiceVol,"max.ini");
	WriteIniBool("max","MusicMute",MusicMute,"max.ini");
	WriteIniBool("max","SoundMute",SoundMute,"max.ini");
	WriteIniBool("max","VoiceMute",VoiceMute,"max.ini");
	WriteIniBool("max","Autosave",Autosave,"max.ini");
	WriteIniBool("max","Animation",Animation,"max.ini");
	WriteIniBool("max","Schatten",Schatten,"max.ini");
	WriteIniBool("max","DamageEffects",DamageEffects,"max.ini");
	WriteIniBool("max","DamageEffectsVehicles",DamageEffectsVehicles,"max.ini");
	WriteIniBool("max","MakeTracks",MakeTracks,"max.ini");
	WriteIniBool("max","ShowBeschreibung",ShowBeschreibung,"max.ini");
	WriteIniBool("max","DamageEffects",DamageEffects,"max.ini");
	WriteIniString("max","LastIP",LastIP.c_str(),"max.ini");
	WriteIniInteger("max","LastPort",LastPort,"max.ini");
	WriteIniString("max","LastPlayerName",LastPlayerName.c_str(),"max.ini");*/
    return 0;
}
int LoadData(void *)
{
	// Fonts laden
	if(!LoadFonts())
	{
		DeleteTerrain();
		DeleteFX();
		DeleteGFX();
		SDL_Quit();
		if(sound)CloseSound();
		return -1;
	}
	string logstr;
	logstr="DDs M.A.X.  -  Version ";logstr+=MAX_VERSION;
	MakeLog((char *)logstr.c_str(),false,0);
	MakeLog("SDL/SDL_Net starten...",false,2);
	MakeLog("SDL/SDL_Net starten...",true,2);
	MakeLog("Ini Datei lesen...",false,3);
	MakeLog("Ini Datei lesen...",true,3);
	MakeLog("Fonts laden...",false,4);
	MakeLog("Fonts laden...",true,4);

	// Sound Initialisieren
	MakeLog("Sound initialisieren...",false,5);
	if(!InitSound()){
		SDL_Quit();
		return -1;
	}
	MakeLog("Sound initialisieren...",true,5);

	// Keys laden:
	MakeLog("Keys laden...",false,6);
	if(!LoadKeys("keys.ini"))
	{
		SDL_Quit();
		if(sound)CloseSound();
		return -1;
	}
	MakeLog("Keys laden...",true,6);

	// Diverse Bilder laden
	MakeLog("Bilder laden...",false,7);
	if(!LoadGFX())
	{
		SDL_Quit();
		if(sound)CloseSound();
		return -1;
	}
	MakeLog("Bilder laden...",true,7);

	// GFX On Demmand prüfen


	// FX laden
	MakeLog("Effekte laden...",false,8);
	if(!LoadFX())
	{
		DeleteGFX();
		SDL_Quit();
		if(sound)CloseSound();
		return -1;
	}
	MakeLog("Effekte laden...",true,8);

	// Terrain laden
	MakeLog("Terrain laden...",false,9);
	if(!LoadTerrain())
	{
		DeleteFX();
		DeleteGFX();
		SDL_Quit();
		if(sound)CloseSound();
		return -1;
	}
	MakeLog("Terrain laden...",true,9);

	// Die Vehicles laden
	MakeLog("Fahrzeuge laden...",false,10);
	if(!LoadVehicles())
	{
		DeleteFonts();
		DeleteTerrain();
		DeleteFX();
		DeleteGFX();
		SDL_Quit();
		if(sound)CloseSound();
		return -1;
	}
	MakeLog("Fahrzeuge laden...",true,10);

	// Die Buildings laden
	MakeLog("Gebäude laden...",false,11);
	if(!LoadBuildings())
	{
		DeleteVehicles();
		DeleteFonts();
		DeleteTerrain();
		DeleteFX();
		DeleteGFX();
		SDL_Quit();
		if(sound)CloseSound();
		return -1;
	}
	MakeLog("Gebäude laden...",true,11);

	// Die Musik laden
	MakeLog("Musik laden...",false,12);
	if(!LoadMusic())
	{
		DeleteBuildings();
		DeleteVehicles();
		DeleteFonts();
		DeleteTerrain();
		DeleteFX();
		DeleteGFX();
		SDL_Quit();
		if(sound)CloseSound();
		return -1;
	}
	MakeLog("Musik laden...",true,12);

	// Die Sounds laden
	MakeLog("Sounds laden...",false,13);
	if(!LoadSounds())
	{
		DeleteBuildings();
		DeleteVehicles();
		DeleteMusic();
		DeleteFonts();
		DeleteTerrain();
		DeleteFX();
		DeleteGFX();
		SDL_Quit();
		if(sound)CloseSound();
		return -1;
	}
	MakeLog("Sounds laden...",true,13);

	// Stimmen laden
	MakeLog("Stimmen laden...",false,14);
	if(!LoadVoices())
	{
		DeleteSounds();
		DeleteBuildings();
		DeleteVehicles();
		DeleteMusic();
		DeleteFonts();
		DeleteTerrain();
		DeleteFX();
		DeleteGFX();
		SDL_Quit();
		if(sound)CloseSound();
		return -1;
	}
	MakeLog("Stimmen laden...",true,14);
	return 1;
}

// MakeLog ///////////////////////////////////////////////////////////////////
// Schreibt eine Nachricht auf dem SplashScreen:
void MakeLog(char* sztxt,bool ok,int pos)
{
	if(!ok)
		fonts->OutTextBig(sztxt,22,152+16*pos,buffer);
	else
		fonts->OutTextBig("OK",250,152+16*pos,buffer);
	SDL_BlitSurface(buffer,NULL,screen,NULL);
	SDL_UpdateRect(screen,0,0,0,0);
}

// InitSound /////////////////////////////////////////////////////////////////
// Initialisiert den Sound:
int InitSound(){
	int frequency, chunksize;
	TiXmlDocument doc;
	TiXmlNode* rootnode;
	doc.LoadFile("max.xml");
	rootnode = doc.FirstChildElement("MAXOptions")->FirstChildElement("StartOptions");
	sound=GetXMLBool(rootnode,"sound");
	if(!sound)return 1;
	frequency = atoi(rootnode->FirstChildElement("frequency")->FirstChild()->ValueStr().c_str());
	chunksize = atoi(rootnode->FirstChildElement("chunksize")->FirstChild()->ValueStr().c_str());
	if(!InitSound(frequency,chunksize)){
		MessageBoxA(NULL,"Sound konnte nicht Initialisiert werden.","InitSound",MB_ICONERROR);
		return 0;
	}
	sound=true;
	return 1;
}

// LoadGFX ///////////////////////////////////////////////////////////////////
// Läd alle benötigten Bilder:
int LoadGFX()
{
	string stmp;
	#define LOADGFX(a,b) if(!FileExists(b)){stmp = "File not found: "; stmp += b; MessageBoxA(NULL,stmp.c_str(),"LoadGFX",MB_ICONERROR);return 0;}a=LoadPCX(b);
	#define CHECKGFX(a,b) if(!FileExists(b)){stmp = "File not found: "; stmp += b; MessageBoxA(NULL,stmp.c_str(),"LoadGFX",MB_ICONERROR);return 0;}a=b;

	LOADGFX(gfx_Chand,"gfx//hand.pcx");
	LOADGFX(gfx_Cno,"gfx//no.pcx");
	LOADGFX(gfx_Cselect,"gfx//select.pcx");
	LOADGFX(gfx_Cmove,"gfx//move.pcx");
	LOADGFX(gfx_Chelp,"gfx//help.pcx");
	LOADGFX(gfx_Cattack,"gfx//attack.pcx");
	LOADGFX(gfx_Cpfeil1,"gfx//pf_1.pcx");
	LOADGFX(gfx_Cpfeil2,"gfx//pf_2.pcx");
	LOADGFX(gfx_Cpfeil3,"gfx//pf_3.pcx");
	LOADGFX(gfx_Cpfeil4,"gfx//pf_4.pcx");
	LOADGFX(gfx_Cpfeil6,"gfx//pf_6.pcx");
	LOADGFX(gfx_Cpfeil7,"gfx//pf_7.pcx");
	LOADGFX(gfx_Cpfeil8,"gfx//pf_8.pcx");
	LOADGFX(gfx_Cpfeil9,"gfx//pf_9.pcx");
	LOADGFX(gfx_hud_stuff,"gfx//hud_stuff.pcx");
	LOADGFX(gfx_praefer,"gfx//praefer.pcx");
	LOADGFX(gfx_help_screen,"gfx//help_screen.pcx");
	LOADGFX(gfx_object_menu,"gfx//object_menu.pcx");
	LOADGFX(gfx_destruction,"gfx//destruction.pcx");
	LOADGFX(gfx_build_screen,"gfx//build_screen.pcx");
	LOADGFX(gfx_fac_build_screen,"gfx//fac_build_screen.pcx");
	LOADGFX(gfx_Cband,"gfx//band_cur.pcx");
	LOADGFX(gfx_band_small,"gfx//band_small.pcx");
	LOADGFX(gfx_band_big,"gfx//band_big.pcx");
	LOADGFX(gfx_band_small_org,"gfx//band_small.pcx");
	LOADGFX(gfx_band_big_org,"gfx//band_big.pcx");
	LOADGFX(gfx_big_beton_org,"gfx//big_beton.pcx");
	LOADGFX(gfx_big_beton,"gfx//big_beton.pcx");
	LOADGFX(gfx_Ctransf,"gfx//transf.pcx");
	LOADGFX(gfx_transfer,"gfx//transfer.pcx");
	LOADGFX(gfx_mine_manager,"gfx//mine_manager.pcx");
	LOADGFX(gfx_Cload,"gfx//load.pcx");
	LOADGFX(gfx_Cactivate,"new_gfx//activate.pcx");
	LOADGFX(gfx_storage,"gfx//storage.pcx");
	LOADGFX(gfx_storage_ground,"gfx//storage_ground.pcx");
	LOADGFX(gfx_dialog,"gfx//dialog.pcx");
	LOADGFX(gfx_edock,"gfx//edock.pcx");
	LOADGFX(gfx_Cmuni,"gfx//muni.pcx");
	LOADGFX(gfx_Crepair,"gfx//repair.pcx");
	LOADGFX(gfx_research,"gfx//research.pcx");
	LOADGFX(gfx_upgrade,"gfx//upgrade.pcx");
	LOADGFX(gfx_panel_top,"gfx//panel_top.pcx");
	LOADGFX(gfx_panel_bottom,"gfx//panel_bottom.pcx");
	LOADGFX(gfx_Csteal,"gfx//steal.pcx");
	LOADGFX(gfx_Cdisable,"gfx//disable.pcx");
	LOADGFX(gfx_menu_stuff,"gfx//menu_stuff.pcx");
	CHECKGFX(DialogPath,"gfx//dialog.pcx");
	CHECKGFX(Dialog2Path,"gfx//dialog2.pcx");
	CHECKGFX(Dialog3Path,"gfx//dialog3.pcx");

	LOADGFX(gfx_player_pc,"new_gfx//player_pc.pcx");
	LOADGFX(gfx_player_human,"new_gfx//player_human.pcx");
	LOADGFX(gfx_player_none,"new_gfx//player_none.pcx");
	LOADGFX(gfx_load_save_menu,"new_gfx//load_save_menu.pcx");
	/*LOADGFX(gfx_build_finished_org,"new_gfx//build_finished.pcx");
	LOADGFX(gfx_build_finished,"new_gfx//build_finished.pcx");*/
	LOADGFX(gfx_exitpoints_org,"new_gfx//activate_field.pcx");
	LOADGFX(gfx_exitpoints,"new_gfx//activate_field.pcx");
	LOADGFX(gfx_player_select,"new_gfx//customgame_menu.pcx");
	LOADGFX(gfx_menu_buttons,"new_gfx//menu_buttons.pcx");

	// Hud:
	SDL_Rect scr,dest;
	gfx_hud=SDL_CreateRGBSurface(SDL_HWSURFACE,ScreenW,ScreenH,32,0,0,0,0);
	SDL_FillRect(gfx_hud,NULL,0xFF00FF);
	SDL_SetColorKey(gfx_hud,SDL_SRCCOLORKEY,0xFF00FF);

	LOADGFX(gfx_tmp,"gfx//hud_left.pcx");
	SDL_BlitSurface(gfx_tmp,NULL,gfx_hud,NULL);
	SDL_FreeSurface(gfx_tmp);

	LOADGFX(gfx_tmp,"gfx//hud_top.pcx");
	scr.x=0;scr.y=0;scr.w=gfx_tmp->w;scr.h=gfx_tmp->h;
	dest.x=180;dest.y=0;dest.w=gfx_hud->w-180;dest.h=18;
	SDL_BlitSurface(gfx_tmp,&scr,gfx_hud,&dest);
	scr.x=1275;scr.w=18;scr.h=18;
	dest.x=gfx_hud->w-18;
	SDL_BlitSurface(gfx_tmp,&scr,gfx_hud,&dest);
	SDL_FreeSurface(gfx_tmp);

	LOADGFX(gfx_tmp,"gfx//hud_right.pcx");
	scr.x=0;scr.y=0;scr.w=gfx_tmp->w;scr.h=gfx_tmp->h;
	dest.x=gfx_hud->w-12;dest.y=18;dest.w=12;dest.h=gfx_hud->h;
	SDL_BlitSurface(gfx_tmp,&scr,gfx_hud,&dest);
	SDL_FreeSurface(gfx_tmp);

	LOADGFX(gfx_tmp,"gfx//hud_bottom.pcx");
	scr.x=0;scr.y=0;scr.w=gfx_tmp->w;scr.h=gfx_tmp->h;
	dest.x=180;dest.y=gfx_hud->h-24;dest.w=gfx_hud->w-180;dest.h=24;
	SDL_BlitSurface(gfx_tmp,&scr,gfx_hud,&dest);
	scr.x=1275;scr.w=23;scr.h=24;
	dest.x=gfx_hud->w-23;
	SDL_BlitSurface(gfx_tmp,&scr,gfx_hud,&dest);
	scr.x=1299;scr.w=16;scr.h=22;
	dest.x=180-16;dest.y=gfx_hud->h-22;
	SDL_BlitSurface(gfx_tmp,&scr,gfx_hud,&dest);
	SDL_FreeSurface(gfx_tmp);

	if(ScreenH>480)
	{
		LOADGFX(gfx_tmp,"gfx//logo.pcx");
		dest.x=9;dest.y=ScreenH-32-15;dest.w=152;dest.h=32;
		SDL_BlitSurface(gfx_tmp,NULL,gfx_hud,&dest);
		SDL_FreeSurface(gfx_tmp);
	}

	// Farben:
	colors=(SDL_Surface**)malloc(sizeof(SDL_Surface*)*8);
	LOADGFX(colors[cl_red],"gfx//cl_red.pcx");
	LOADGFX(colors[cl_blue],"gfx//cl_blue.pcx");
	LOADGFX(colors[cl_green],"gfx//cl_green.pcx");
	LOADGFX(colors[cl_grey],"gfx//cl_grey.pcx");
	LOADGFX(colors[cl_orange],"gfx//cl_orange.pcx");
	LOADGFX(colors[cl_yellow],"gfx//cl_yellow.pcx");
	LOADGFX(colors[cl_purple],"gfx//cl_purple.pcx");
	LOADGFX(colors[cl_aqua],"gfx//cl_aqua.pcx");

	ShieldColors=(SDL_Surface**)malloc(sizeof(SDL_Surface*)*8);
	MakeShieldColor(&(ShieldColors[0]),colors[0]);
	MakeShieldColor(&(ShieldColors[1]),colors[1]);
	MakeShieldColor(&(ShieldColors[2]),colors[2]);
	MakeShieldColor(&(ShieldColors[3]),colors[3]);
	MakeShieldColor(&(ShieldColors[4]),colors[4]);
	MakeShieldColor(&(ShieldColors[5]),colors[5]);
	MakeShieldColor(&(ShieldColors[6]),colors[6]);
	MakeShieldColor(&(ShieldColors[7]),colors[7]);

	// Shadow:
	gfx_shadow=SDL_CreateRGBSurface(SDL_HWSURFACE,ScreenW,ScreenH,32,0,0,0,0);
	SDL_FillRect(gfx_shadow,NULL,0x0);
	SDL_SetAlpha(gfx_shadow,SDL_SRCALPHA,50);
	gfx_tmp=SDL_CreateRGBSurface(SDL_HWSURFACE,128,128,32,0,0,0,0);
	SDL_SetColorKey(gfx_tmp,SDL_SRCCOLORKEY,0xFF00FF);

	// Glas:
	LOADGFX(gfx_destruction_glas,"gfx//destruction_glas.pcx");
	SDL_SetAlpha(gfx_destruction_glas,SDL_SRCALPHA,150);

	// Waypoints:
	for(int i = 0;i<60;i++){
		WayPointPfeile[0][i] = CreatePfeil(26,11,51,36,14,48,PFEIL_COLOR,64-i);
		WayPointPfeile[1][i] = CreatePfeil(14,14,49,14,31,49,PFEIL_COLOR,64-i);
		WayPointPfeile[2][i] = CreatePfeil(37,11,12,36,49,48,PFEIL_COLOR,64-i);
		WayPointPfeile[3][i] = CreatePfeil(49,14,49,49,14,31,PFEIL_COLOR,64-i);
		WayPointPfeile[4][i] = CreatePfeil(14,14,14,49,49,31,PFEIL_COLOR,64-i);
		WayPointPfeile[5][i] = CreatePfeil(15,14,52,26,27,51,PFEIL_COLOR,64-i);
		WayPointPfeile[6][i] = CreatePfeil(31,14,14,49,49,49,PFEIL_COLOR,64-i);
		WayPointPfeile[7][i] = CreatePfeil(48,14,36,51,11,26,PFEIL_COLOR,64-i);

		WayPointPfeileSpecial[0][i] = CreatePfeil(26,11,51,36,14,48,PFEILS_COLOR,64-i);
		WayPointPfeileSpecial[1][i] = CreatePfeil(14,14,49,14,31,49,PFEILS_COLOR,64-i);
		WayPointPfeileSpecial[2][i] = CreatePfeil(37,11,12,36,49,48,PFEILS_COLOR,64-i);
		WayPointPfeileSpecial[3][i] = CreatePfeil(49,14,49,49,14,31,PFEILS_COLOR,64-i);
		WayPointPfeileSpecial[4][i] = CreatePfeil(14,14,14,49,49,31,PFEILS_COLOR,64-i);
		WayPointPfeileSpecial[5][i] = CreatePfeil(15,14,52,26,27,51,PFEILS_COLOR,64-i);
		WayPointPfeileSpecial[6][i] = CreatePfeil(31,14,14,49,49,49,PFEILS_COLOR,64-i);
		WayPointPfeileSpecial[7][i] = CreatePfeil(48,14,36,51,11,26,PFEILS_COLOR,64-i);
	}

	// Resources:
	LOADGFX(res_metal_org,"gfx\\res.pcx");
	SDL_SetColorKey(res_metal_org,SDL_SRCCOLORKEY,-1);
	res_metal=SDL_CreateRGBSurface(SDL_HWSURFACE,res_metal_org->w,res_metal_org->h,32,0,0,0,0);
	SDL_BlitSurface(res_metal_org,NULL,res_metal,NULL);
	SDL_SetColorKey(res_metal,SDL_SRCCOLORKEY,0xFFFFFF);

	res_oil_org=SDL_CreateRGBSurface(SDL_HWSURFACE,res_metal_org->w,res_metal_org->h,32,0,0,0,0);
	SDL_FillRect(res_oil_org,NULL,0x00FF00);
	SDL_SetColorKey(res_oil_org,SDL_SRCCOLORKEY,0xFF00FF);
	SDL_BlitSurface(res_metal,NULL,res_oil_org,NULL);
	res_oil=SDL_CreateRGBSurface(SDL_HWSURFACE,res_metal_org->w,res_metal_org->h,32,0,0,0,0);
	SDL_FillRect(res_oil,NULL,0x00FF00);
	SDL_SetColorKey(res_oil,SDL_SRCCOLORKEY,0xFF00FF);  
	SDL_BlitSurface(res_metal,NULL,res_oil,NULL);

	res_gold_org=SDL_CreateRGBSurface(SDL_HWSURFACE,res_metal_org->w,res_metal_org->h,32,0,0,0,0);
	SDL_FillRect(res_gold_org,NULL,0xFFFF00);
	SDL_SetColorKey(res_gold_org,SDL_SRCCOLORKEY,0xFF00FF);
	SDL_BlitSurface(res_metal,NULL,res_gold_org,NULL);
	res_gold=SDL_CreateRGBSurface(SDL_HWSURFACE,res_metal_org->w,res_metal_org->h,32,0,0,0,0);
	SDL_FillRect(res_gold,NULL,0xFFFF00);
	SDL_SetColorKey(res_gold,SDL_SRCCOLORKEY,0xFF00FF);
	SDL_BlitSurface(res_metal,NULL,res_gold,NULL);

	SDL_SetColorKey(res_metal,SDL_SRCCOLORKEY,0xFF00FF);
	return 1;
}

// DeleteGFC /////////////////////////////////////////////////////////////////
// Löscht alle geladenen Bilder:
void DeleteGFX(void){
	int i,k;
	SDL_FreeSurface(gfx_hud);
	SDL_FreeSurface(gfx_Chand);
	SDL_FreeSurface(gfx_Cno);
	SDL_FreeSurface(gfx_Cselect);
	SDL_FreeSurface(gfx_Cmove);
	SDL_FreeSurface(gfx_Chelp);
	SDL_FreeSurface(gfx_Cattack);
	SDL_FreeSurface(gfx_Cpfeil1);
	SDL_FreeSurface(gfx_Cpfeil2);
	SDL_FreeSurface(gfx_Cpfeil3);
	SDL_FreeSurface(gfx_Cpfeil4);
	SDL_FreeSurface(gfx_Cpfeil6);
	SDL_FreeSurface(gfx_Cpfeil7);
	SDL_FreeSurface(gfx_Cpfeil8);
	SDL_FreeSurface(gfx_Cpfeil9);
	SDL_FreeSurface(gfx_praefer);
	SDL_FreeSurface(gfx_shadow);
	SDL_FreeSurface(gfx_tmp);
	SDL_FreeSurface(gfx_help_screen);
	SDL_FreeSurface(gfx_object_menu);
	SDL_FreeSurface(gfx_destruction);
	SDL_FreeSurface(gfx_destruction_glas);
	SDL_FreeSurface(gfx_build_screen);
	SDL_FreeSurface(gfx_fac_build_screen);
	SDL_FreeSurface(gfx_Cband);
	SDL_FreeSurface(gfx_band_small);
	SDL_FreeSurface(gfx_band_big);
	SDL_FreeSurface(gfx_band_small_org);
	SDL_FreeSurface(gfx_band_big_org);
	SDL_FreeSurface(res_metal_org);
	SDL_FreeSurface(res_metal);
	SDL_FreeSurface(res_oil_org);
	SDL_FreeSurface(res_oil);
	SDL_FreeSurface(res_gold_org);
	SDL_FreeSurface(res_gold);
	SDL_FreeSurface(gfx_big_beton_org);
	SDL_FreeSurface(gfx_big_beton);
	SDL_FreeSurface(gfx_Ctransf);
	SDL_FreeSurface(gfx_transfer);
	SDL_FreeSurface(gfx_mine_manager);
	SDL_FreeSurface(gfx_Cload);
	SDL_FreeSurface(gfx_storage);
	SDL_FreeSurface(gfx_storage_ground);
	SDL_FreeSurface(gfx_editor);
	SDL_FreeSurface(gfx_dialog);
	SDL_FreeSurface(gfx_edock);
	SDL_FreeSurface(gfx_Cmuni);
	SDL_FreeSurface(gfx_Crepair);
	SDL_FreeSurface(gfx_research);
	SDL_FreeSurface(gfx_upgrade);
	SDL_FreeSurface(gfx_panel_top);
	SDL_FreeSurface(gfx_panel_bottom);
	SDL_FreeSurface(gfx_Csteal);
	SDL_FreeSurface(gfx_Cdisable);
	SDL_FreeSurface(gfx_menu_stuff);

	SDL_FreeSurface(gfx_player_pc);
	SDL_FreeSurface(gfx_player_human);
	SDL_FreeSurface(gfx_player_none);
	SDL_FreeSurface(gfx_load_save_menu);
	SDL_FreeSurface(gfx_exitpoints_org);
	SDL_FreeSurface(gfx_exitpoints);
	SDL_FreeSurface(gfx_player_select);
	SDL_FreeSurface(gfx_menu_buttons);

	for(i=0;i<8;i++){
		SDL_FreeSurface(colors[i]);
		SDL_FreeSurface(ShieldColors[i]);

		for(k=0;k<60;k++){
			SDL_FreeSurface(WayPointPfeile[i][k]);
		}
	}
	free(colors);
	free(ShieldColors);
}

// LoadFX ////////////////////////////////////////////////////////////////////
// Läd alle benötigten Bilder:
int LoadFX(){
	// Die Surfaces anlegen:
	#define LOADFX(a,b) if(!FileExists(b)){MessageBox(NULL,(L"File not found: "),L"LoadFX",MB_ICONERROR); return 0;} a = (SDL_Surface**)malloc(sizeof(SDL_Surface*)*2); a[0] = LoadPCX(b); a[1] = LoadPCX(b);
	#define LOADFXALPHA(a,b,c) if(!FileExists(b)){MessageBox(NULL,(L"File not found: "),L"LoadFX",MB_ICONERROR); return 0;}a=(SDL_Surface**)malloc(sizeof(SDL_Surface*)*2); a[0] = LoadPCX(b); SDL_SetAlpha(a[0],SDL_SRCALPHA,c); a[1] = LoadPCX(b); SDL_SetAlpha(a[1],SDL_SRCALPHA,c);
	LOADFX(fx_explo_small0,"fx\\explo_small0.pcx");
	LOADFX(fx_explo_small1,"fx\\explo_small1.pcx");
	LOADFX(fx_explo_small2,"fx\\explo_small2.pcx");
	LOADFX(fx_explo_big0,"fx\\explo_big0.pcx");
	LOADFX(fx_explo_big1,"fx\\explo_big1.pcx");
	LOADFX(fx_explo_big2,"fx\\explo_big2.pcx");
	LOADFX(fx_explo_big3,"fx\\explo_big3.pcx");
	LOADFX(fx_explo_big4,"fx\\explo_big4.pcx");
	LOADFX(fx_muzzle_big,"fx\\muzzle_big.pcx");
	LOADFX(fx_muzzle_small,"fx\\muzzle_small.pcx");
	LOADFX(fx_muzzle_med,"fx\\muzzle_med.pcx");
	LOADFX(fx_hit,"fx\\hit.pcx");
	LOADFXALPHA(fx_smoke,"fx\\smoke.pcx",100);
	LOADFX(fx_rocket,"fx\\rocket.pcx");
	LOADFXALPHA(fx_dark_smoke,"fx\\dark_smoke.pcx",100);
	LOADFXALPHA(fx_tracks,"fx\\tracks.pcx",100);
	LOADFXALPHA(fx_corpse,"fx\\corpse.pcx",255);
	LOADFXALPHA(fx_absorb,"fx\\absorb.pcx",150);
	return 1;
}

// DeleteFX //////////////////////////////////////////////////////////////////
// Löscht alle geladenen Bilder:
void DeleteFX(void){
  #define DELFX(a) SDL_FreeSurface(a[0]);SDL_FreeSurface(a[1]);free(a);
  DELFX(fx_explo_small0);
  DELFX(fx_explo_small1);
  DELFX(fx_explo_small2);
  DELFX(fx_explo_big0);
  DELFX(fx_explo_big1);
  DELFX(fx_explo_big2);
  DELFX(fx_explo_big3);
  DELFX(fx_explo_big4);
  DELFX(fx_muzzle_big);
  DELFX(fx_muzzle_small);
  DELFX(fx_muzzle_med);
  DELFX(fx_hit);
  DELFX(fx_smoke);
  DELFX(fx_rocket);
  DELFX(fx_dark_smoke);
  DELFX(fx_tracks);
  DELFX(fx_corpse);
  DELFX(fx_absorb);
}

// LoadTerrain ///////////////////////////////////////////////////////////////
// Läd alle Terrain Grafiken:
int LoadTerrain(){
	TiXmlDocument doc;
	TiXmlNode* rootnode;
	TiXmlNode* node;
	string file;
	int i;

	if(!doc.LoadFile("terrain\\terrain.xml")){
		MessageBoxA(NULL,"terrain.xml not found","LoadTerrain",MB_ICONERROR);
		return 0;
	}
	rootnode = doc.FirstChildElement("Terrains");

	TList *sections;
	sections = new TList();
	node=rootnode->FirstChildElement();
	if(node)
		sections->Add(node->ToElement()->ValueStr());
	while(node){
		node=node->NextSibling();
		if(node && node->Type()==1){
			sections->Add(node->ToElement()->ValueStr());
		}
	}

	for(i=0;i<sections->Count;i++){
		node = rootnode->FirstChildElement(sections->Items[i].c_str());
		terrain_anz++;
		terrain=(sTerrain*)realloc(terrain,sizeof(sTerrain)*terrain_anz);
		file="terrain\\";
		if(node->ToElement()->Attribute("file")==NULL)
			MessageBoxA(NULL,"Fehlerhaftes Format der XML-Datei","LoadTerrain",MB_ICONERROR);
		file+=node->ToElement()->Attribute("file");
		if(!FileExists(file.c_str())) {
			MessageBoxA(NULL, sections->Items[i].c_str(), "LoadTerrain", MB_ICONERROR);
			return 0;
		}

		#define DUP_SF(a,b)b= SDL_CreateRGBSurface(SDL_HWSURFACE|SDL_SRCCOLORKEY,a->w,a->h,32,0,0,0,0);SDL_FillRect(b,NULL,0xFF00FF);SDL_BlitSurface(a,NULL,b,NULL);
		terrain[i].sf_org = LoadPCX((char *)file.c_str());
		DUP_SF(terrain[i].sf_org,terrain[i].sf);

		DUP_SF(terrain[i].sf_org,terrain[i].shw_org);
		SDL_BlitSurface(gfx_shadow,NULL,terrain[i].shw_org,NULL);
		DUP_SF(terrain[i].shw_org,terrain[i].shw);

		terrain[i].water=false;
		if(node->ToElement()->Attribute("water")!=NULL)
			if(strcmp(node->ToElement()->Attribute("water"),"true")==0)
				terrain[i].water=true;
		terrain[i].coast=false;
		if(node->ToElement()->Attribute("coast")!=NULL)
			if(strcmp(node->ToElement()->Attribute("coast"),"true")==0)
				terrain[i].coast=true;
		terrain[i].overlay=false;
		if(node->ToElement()->Attribute("overlay")!=NULL)
			if(strcmp(node->ToElement()->Attribute("overlay"),"true")==0)
				terrain[i].overlay=true;
		terrain[i].blocked=false;
		if(node->ToElement()->Attribute("blocked")!=NULL)
			if(strcmp(node->ToElement()->Attribute("blocked"),"true")==0)
				terrain[i].blocked=true;

		terrain[i].frames=terrain[i].sf_org->w/64;
		terrain[i].id=(char*)malloc((sections->Items[i]).length()+1);
		strcpy(terrain[i].id,(sections->Items[i]).c_str());

		if(terrain[i].overlay){
			int t=0xFFCD00CD;
			SDL_SetColorKey(terrain[i].sf_org,SDL_SRCCOLORKEY,0xFF00FF);
			SDL_SetColorKey(terrain[i].shw_org,SDL_SRCCOLORKEY,t);
			SDL_SetColorKey(terrain[i].sf,SDL_SRCCOLORKEY,0xFF00FF);
			SDL_SetColorKey(terrain[i].shw,SDL_SRCCOLORKEY,t);
		}
	}
	delete sections;
	return 1;
}

// DeleteTerrain /////////////////////////////////////////////////////////////
// Löscht alle Terrain Grafiken:
void DeleteTerrain(void){
  int i;
  if(terrain==NULL)return;
  // Alle Terrains löschen:
  for(i=0;i<terrain_anz;i++){
    SDL_FreeSurface(terrain[i].sf_org);
    SDL_FreeSurface(terrain[i].shw_org);
    SDL_FreeSurface(terrain[i].sf);
    SDL_FreeSurface(terrain[i].shw);
    free(terrain[i].id);
  }
  free(terrain);
}

// LoadFonts /////////////////////////////////////////////////////////////////
// Läd alle Fonts und bereitet diese vor:
int LoadFonts(){
  // Die Fonts laden:
  #define LOADFONT(a,b) if(!FileExists(b)){MessageBox(NULL,(L"File not found: "),L"LoadFonts",MB_ICONERROR);return 0;}a=LoadPCX(b);
  LOADFONT(font,"fonts//font.pcx");
  LOADFONT(font_small_white,"fonts//font_small_white.pcx");
  LOADFONT(font_small_red,"fonts//font_small_red.pcx");
  LOADFONT(font_small_green,"fonts//font_small_green.pcx");
  LOADFONT(font_small_yellow,"fonts//font_small_yellow.pcx");
  LOADFONT(font_big,"fonts//font_big.pcx");
  LOADFONT(font_big_gold,"fonts//font_big_gold.pcx");
  // Die Fonts anlegen:
  fonts=new cFonts;
  return 1;
}

// DeleteFonts ///////////////////////////////////////////////////////////////
// Löscht alle Fonts:
void DeleteFonts(void){
  SDL_FreeSurface(font);
  SDL_FreeSurface(font_small_white);
  SDL_FreeSurface(font_small_red);
  SDL_FreeSurface(font_small_green);
  SDL_FreeSurface(font_small_yellow);
  SDL_FreeSurface(font_big);
  SDL_FreeSurface(font_big_gold);
  delete fonts;
}

// ScaleSurface //////////////////////////////////////////////////////////////
// Skaliert ein Surface in ein Anderes:
void ScaleSurface(SDL_Surface *scr,SDL_Surface **dest,int size){
  int x,y,rx,ry,dx,dy,sizex=size;
  unsigned int *s,*d;

  if(scr->w>scr->h){
    sizex=scr->w/64*size;
  }
  *dest=SDL_CreateRGBSurface(SDL_HWSURFACE|SDL_SRCCOLORKEY,sizex,size,32,0,0,0,0);
  dx=rx=0;
  dy=0;
  SDL_LockSurface(*dest);
  SDL_LockSurface(scr);
  s=(unsigned int*)scr->pixels;
  d=(unsigned int*)(*dest)->pixels;
  ry=scr->h;
  for(y=0;y<scr->h;y++){
    if(ry>=scr->h){
      ry-=scr->h;
      dx=0;
      rx=scr->w;
      for(x=0;x<scr->w;x++){
        if(rx>=scr->w){
          rx-=scr->w;
          d[dx+dy*sizex]=s[x+y*scr->w];
          dx++;
        }
        if(dx>=sizex)break;
        rx+=sizex;
      }
      dy++;
      if(dy>=size)break;
    }
    ry+=size;
  }
  SDL_UnlockSurface(*dest);
  SDL_UnlockSurface(scr);
}

void ScaleSurface2(SDL_Surface *scr,SDL_Surface *dest,int size){
  int x,y,rx,ry,dx,dy,sizex=size;
  unsigned int *s,*d;
  if(scr->w>scr->h){
    sizex=scr->w/64*size;
  }
  dest->w=scr->w;
  dest->h=scr->h;
  dx=rx=0;
  dy=0;
  SDL_LockSurface(dest);
  SDL_LockSurface(scr);
  s=(unsigned int*)scr->pixels;
  d=(unsigned int*)dest->pixels;
  ry=scr->h;
  for(y=0;y<scr->h;y++){
    if(ry>=scr->h){
      ry-=scr->h;
      dx=0;
      rx=scr->w;
      for(x=0;x<scr->w;x++){
        if(rx>=scr->w){
          rx-=scr->w;
          d[dx+dy*dest->w]=s[x+y*scr->w];
          dx++;
        }
        if(dx>=sizex)break;
        rx+=sizex;
      }
      dy++;
      if(dy>=size)break;
    }
    ry+=size;
  }
  SDL_UnlockSurface(dest);
  SDL_UnlockSurface(scr);
  dest->w=sizex;
  dest->h=size;
}

// LoadMusic /////////////////////////////////////////////////////////////////
// Organisiert alle Namen der Musikstücke:
int LoadMusic(){
  char sztmp[256];
  MainMusicFile;
  string stmp;

  MusicAnz=0;
  if(!FileExists("music\\music.ini")){
    MessageBox(NULL,L"music.ini not found or empty",L"LoadMusic",MB_ICONERROR);
    return 0;
  }

  MainMusicFile = ReadIniString("music","main","None","music\\music.ini");
  if(MainMusicFile.c_str() == "" || !FileExists("music\\main.ogg")) {
    MessageBoxA(NULL, "MainMusicFile ", "LoadMusic", MB_ICONERROR);
    return 0;
  }

  CreditsMusicFile = ReadIniString("music","credits","None","music\\music.ini");;
  if(CreditsMusicFile.c_str() == "" || !FileExists("music\\credits.ogg")) {
    MessageBoxA(NULL, "CreditsMusicFile ", "LoadMusic", MB_ICONERROR);
    return 0;
  }

  MusicAnz = ReadIniInteger("music","bkgcount",0,"music\\music.ini");
  MusicFiles = new TList;
  for(int i=1;i<=MusicAnz;i++){
	itoa(i,sztmp,10);
    stmp = "bkg"; stmp += sztmp;
	GetPrivateProfileStringA("music", stmp.c_str(), "None", sztmp, 255, "music//music.ini" );
	stmp = "music\\"; stmp += sztmp;
    MusicFiles->Add(stmp);
	if( MusicFiles->Items[i - 1].c_str() == "" ||!FileExists(MusicFiles->Items[i - 1].c_str())) {
      MessageBoxA(NULL,"MusicFile ","LoadMusic",MB_ICONERROR);
      return 0;
    }
  }
  return 1;
}

// DeleteMusic ///////////////////////////////////////////////////////////////
// Gibt alle Namen der Musikstücke wieder frei:
void DeleteMusic(void){
  if(MusicFiles!=NULL){
    delete MusicFiles;
  }
}

// LoadSounds //////////////////////////////////////////////////////////////
// Läd alle Sounds:
int LoadSounds()
{
	#define LOADSND(a,b) if(!FileExists(b)){MessageBox(NULL,(L"File not found: "),L"LoadSND",MB_ICONERROR);return 0;}a=Mix_LoadWAV(b);

	LOADSND(SNDHudSwitch, "sounds/HudSwitch.wav");
	LOADSND(SNDHudButton, "sounds/HudButton.wav");
	LOADSND(SNDMenuButton, "sounds/MenuButton.wav");
	LOADSND(SNDChat, "sounds/Chat.wav");
	LOADSND(SNDObjectMenu, "sounds/ObjectMenu.wav");
	LOADSND(EXPBigWet0, "sounds/exp_big_wet0.wav");
	LOADSND(EXPBigWet1, "sounds/exp_big_wet1.wav");
	LOADSND(EXPBig0, "sounds/exp_big0.wav");
	LOADSND(EXPBig1, "sounds/exp_big1.wav");
	LOADSND(EXPBig2, "sounds/exp_big2.wav");
	LOADSND(EXPBig3, "sounds/exp_big3.wav");
	LOADSND(EXPSmallWet0, "sounds/exp_small_wet0.wav");
	LOADSND(EXPSmallWet1, "sounds/exp_small_wet1.wav");
	LOADSND(EXPSmallWet2, "sounds/exp_small_wet2.wav");
	LOADSND(EXPSmall0, "sounds/exp_small0.wav");
	LOADSND(EXPSmall1, "sounds/exp_small1.wav");
	LOADSND(EXPSmall2, "sounds/exp_small2.wav");
	LOADSND(SNDArm, "sounds/arm.wav");
	LOADSND(SNDBuilding, "sounds/building.wav");
	LOADSND(SNDClearing, "sounds/clearing.wav");
	LOADSND(SNDQuitsch, "sounds/quitsch.wav");
	LOADSND(SNDActivate, "sounds/activate.wav");
	LOADSND(SNDLoad, "sounds/load.wav");
	LOADSND(SNDReload, "sounds/reload.wav");
	LOADSND(SNDRepair, "sounds/repair.wav");
	LOADSND(SNDLandMinePlace, "sounds/land_mine_place.wav");
	LOADSND(SNDLandMineClear, "sounds/land_mine_clear.wav");
	LOADSND(SNDSeaMinePlace, "sounds/sea_mine_place.wav");
	LOADSND(SNDSeaMineClear, "sounds/sea_mine_clear.wav");
	LOADSND(SNDPanelOpen, "sounds/panel_open.wav");
	LOADSND(SNDPanelClose, "sounds/panel_close.wav");
	LOADSND(SNDAbsorb, "sounds/absorb.wav");
	return 1;
}

// DeleteSounds //////////////////////////////////////////////////////////////
// Löscht alle Sounds:
void DeleteSounds(void){
  #define DELSND(a) Mix_FreeChunk(a);
  DELSND(SNDHudSwitch);
  DELSND(SNDHudButton);
  DELSND(SNDMenuButton);
  DELSND(SNDChat);
  DELSND(SNDObjectMenu);
  DELSND(EXPBigWet0);
  DELSND(EXPBigWet1);
  DELSND(EXPBig0);
  DELSND(EXPBig1);
  DELSND(EXPBig2);
  DELSND(EXPBig3);
  DELSND(EXPSmallWet0);
  DELSND(EXPSmallWet1);
  DELSND(EXPSmallWet2);
  DELSND(EXPSmall0);
  DELSND(EXPSmall1);
  DELSND(EXPSmall2);
  DELSND(SNDArm);
  DELSND(SNDBuilding);
  DELSND(SNDClearing);
  DELSND(SNDQuitsch);
  DELSND(SNDActivate);
  DELSND(SNDLoad);
  DELSND(SNDReload);
  DELSND(SNDRepair);
  DELSND(SNDLandMinePlace);
  DELSND(SNDLandMineClear);
  DELSND(SNDSeaMinePlace);
  DELSND(SNDSeaMineClear);
  DELSND(SNDPanelOpen);
  DELSND(SNDPanelClose);
  DELSND(SNDAbsorb);
}

// LoadVoices //////////////////////////////////////////////////////////////
// Läd alle Voices:
int LoadVoices()
{
	#define LOADVOI(a,b) if(!FileExists(b)){MessageBox(NULL,(L"File not found: "),L"LoadVoices",MB_ICONERROR);return 0;}a=Mix_LoadWAV(b);
	LOADVOI(VOINoPath1,"voices\\no_path1.wav");
	LOADVOI(VOINoPath2,"voices\\no_path2.wav");
	LOADSND(VOIBuildDone1,"voices\\build_done1.wav");
	LOADSND(VOIBuildDone2,"voices\\build_done2.wav");
	LOADSND(VOINoSpeed,"voices\\no_speed.wav");
	LOADSND(VOIStatusRed,"voices\\status_red.wav");
	LOADSND(VOIStatusYellow,"voices\\status_yellow.wav");
	LOADSND(VOIClearing,"voices\\clearing.wav");
	LOADSND(VOILowAmmo1,"voices\\low_ammo1.wav");
	LOADSND(VOILowAmmo2,"voices\\low_ammo2.wav");
	LOADSND(VOIOK1,"voices\\ok1.wav");
	LOADSND(VOIOK2,"voices\\ok2.wav");
	LOADSND(VOIOK3,"voices\\ok3.wav");  
	LOADSND(VOIWachposten,"voices\\wachposten.wav");
	LOADSND(VOITransferDone,"voices\\transfer_done.wav");
	LOADSND(VOILoaded,"voices\\loaded.wav");
	LOADSND(VOIRepaired,"voices\\repaired.wav");
	LOADSND(VOILayingMines,"voices\\laying_mines.wav");
	LOADSND(VOIClearingMines,"voices\\clearing_mines.wav");
	LOADSND(VOIResearchComplete,"voices\\research_complete.wav");
	LOADSND(VOIUnitStolen,"voices\\unit_stolen.wav");
	LOADSND(VOIUnitDisabled,"voices\\unit_disabled.wav");
	LOADSND(VOICommandoDetected,"voices\\commando_detected.wav");
	LOADSND(VOIDisabled,"voices\\disabled.wav");
	LOADSND(VOISaved,"voices\\saved.wav");
	LOADSND(VOIStartNone,"voices\\start_none.wav");
	LOADSND(VOIStartOne,"voices\\start_one.wav");
	LOADSND(VOIStartMore,"voices\\start_more.wav");
	LOADSND(VOIDetected1,"voices\\detected1.wav");
	LOADSND(VOIDetected2,"voices\\detected2.wav");
	LOADSND(VOIAttackingUs,"voices\\attacking_us.wav");
	LOADSND(VOIDestroyedUs,"voices\\destroyed_us.wav");
	return 1;
}

// DeleteVoices //////////////////////////////////////////////////////////////
// Löscht alle Voices:
void DeleteVoices(void){
  DELSND(VOINoPath1);
  DELSND(VOINoPath2);
  DELSND(VOIBuildDone1);
  DELSND(VOIBuildDone2);
  DELSND(VOINoSpeed);
  DELSND(VOIStatusRed);
  DELSND(VOIStatusYellow);
  DELSND(VOIClearing);
  DELSND(VOILowAmmo1);
  DELSND(VOILowAmmo2);
  DELSND(VOIOK1);
  DELSND(VOIOK2);
  DELSND(VOIOK3);  
  DELSND(VOIWachposten);
  DELSND(VOITransferDone);
  DELSND(VOILoaded);
  DELSND(VOIRepaired);
  DELSND(VOILayingMines);
  DELSND(VOIClearingMines);
  DELSND(VOIResearchComplete);
  DELSND(VOIUnitStolen);
  DELSND(VOIUnitDisabled);
  DELSND(VOICommandoDetected);
  DELSND(VOIDisabled);
  DELSND(VOISaved);
  DELSND(VOIStartNone);
  DELSND(VOIStartOne);
  DELSND(VOIStartMore);
  DELSND(VOIDetected1);
  DELSND(VOIDetected2);
  DELSND(VOIAttackingUs);
  DELSND(VOIDestroyedUs);
}

// LoadBuildings /////////////////////////////////////////////////////////////
// Läd alle Buildings:
int LoadBuildings(){
  string p, file, TmpStr;
  bool changed;
  int i,max;
  building_anz=0;
  string suchdir="buildings\\*.*";
  long hFile;
  char tmp[256];
  char IniTmp[256];

  if( (hFile = _findfirst( suchdir.c_str(), &c_file )) == -1L ) {
    _findclose(hFile); return 0;
  }

  // Alle Unterverzeichnisse durchsuchen:
  do{
	if (c_file.size == 0 && strcmp(c_file.name,".") && strcmp(c_file.name,"..")) ;
	else continue;
	p = c_file.name;
	p += "\\";
    building_anz++;
    building=(sBuilding*)realloc(building,sizeof(sBuilding)*building_anz);
    memset(&(building[building_anz-1].data),0,sizeof(sBuildingData));
    // img laden:
	file = p; file.insert(0,"buildings\\"); file += "img.pcx";
	if(!FileExists(file.c_str())){
      MessageBoxA(NULL,"File not found: img.pcx","LoadBuildings",MB_ICONERROR);
      return 0;
    }
	strcpy(tmp, file.c_str());
    building[building_anz-1].img_org=LoadPCX(tmp);
    SDL_SetColorKey(building[building_anz-1].img_org,SDL_SRCCOLORKEY,0xFFFFFF);
    building[building_anz-1].img=LoadPCX(tmp);
    SDL_SetColorKey(building[building_anz-1].img,SDL_SRCCOLORKEY,0xFFFFFF);
    // shw laden:
	file = p; file.insert(0,"buildings\\"); file += "shw.pcx";
    if(!FileExists(file.c_str())){
      MessageBoxA(NULL,"File not found: shw.pcx","LoadBuildings",MB_ICONERROR);
      return 0;
    }
	strcpy(tmp, file.c_str());
    building[building_anz-1].shw_org=LoadPCX(tmp);
    SDL_SetColorKey(building[building_anz-1].shw_org,SDL_SRCCOLORKEY,0xFF00FF);
    building[building_anz-1].shw=LoadPCX(tmp);
    SDL_SetAlpha(building[building_anz-1].shw,SDL_SRCALPHA,50);
    SDL_SetColorKey(building[building_anz-1].shw,SDL_SRCCOLORKEY,0xFF00FF);
    // Das Video laden:
	file = p; file.insert(0,"buildings\\"); file += "video.pcx";
    if(!FileExists(file.c_str())){
      MessageBoxA(NULL,"File not found: video.pcx","LoadBuildings",MB_ICONERROR);
      return 0;
    }
	strcpy(tmp, file.c_str());
    building[building_anz-1].video=LoadPCX(tmp);
    // Das Infobild laden:
	file = p; file.insert(0,"buildings\\"); file += "info.pcx";
    if(!FileExists(file.c_str())){
      MessageBoxA(NULL,"File not found: info.pcx","LoadBuildings",MB_ICONERROR);
      return 0;
    }
	strcpy(tmp, file.c_str());
    building[building_anz-1].info=LoadPCX(tmp,true);
    // Die Daten laden:
	file = p; file.insert(0,"buildings\\"); file += "data.ini";
    if(!FileExists(file.c_str())){
      MessageBoxA(NULL,"File not found: data.ini","LoadBuildings",MB_ICONERROR);
      return 0;
    }
    building[building_anz-1].data.version=1;
	GetPrivateProfileStringA("data", "name", "None", IniTmp, 255, file.c_str() );
    strncpy(building[building_anz-1].data.name,IniTmp,24);
	GetPrivateProfileIntA("data", "max_hit_points", 1, file.c_str());
    building[building_anz-1].data.max_hit_points =GetPrivateProfileIntA("data", "max_hit_points", 1, file.c_str());
    building[building_anz-1].data.hit_points = 0;
    building[building_anz-1].data.armor = GetPrivateProfileIntA("data", "armor", 1, file.c_str());
    building[building_anz-1].data.scan = GetPrivateProfileIntA("data", "scan", 1, file.c_str());
    building[building_anz-1].data.range = GetPrivateProfileIntA("data", "range", 1, file.c_str());
    building[building_anz-1].data.max_shots = GetPrivateProfileIntA("data", "max_shots", 1, file.c_str());
    building[building_anz-1].data.shots = 0;
    building[building_anz-1].data.damage = GetPrivateProfileIntA("data", "damage", 1, file.c_str());
    building[building_anz-1].data.max_cargo = GetPrivateProfileIntA("data", "max_cargo", 0, file.c_str());
    building[building_anz-1].data.cargo = 0;
    building[building_anz-1].data.max_ammo = GetPrivateProfileIntA("data", "max_ammo", 0, file.c_str());
    building[building_anz-1].data.ammo = 0;
    building[building_anz-1].data.costs = GetPrivateProfileIntA("data", "costs", 1, file.c_str());
    building[building_anz-1].data.energy_prod = GetPrivateProfileIntA("data", "energy_prod", 0, file.c_str());
    building[building_anz-1].data.oil_need = GetPrivateProfileIntA("data", "oil_need", 0, file.c_str());
    building[building_anz-1].data.energy_need = GetPrivateProfileIntA("data", "energy_need", 0, file.c_str());
    building[building_anz-1].data.metal_need = GetPrivateProfileIntA("data", "metal_need", 0, file.c_str());
    building[building_anz-1].data.gold_need = GetPrivateProfileIntA("data", "gold_need", 0, file.c_str());
    building[building_anz-1].data.max_shield = GetPrivateProfileIntA("data", "max_shield", 0, file.c_str());
    building[building_anz-1].data.shield = 0;

    building[building_anz-1].data.can_build = GetPrivateProfileIntA("data", "can_build", BUILD_NONE, file.c_str());
    building[building_anz-1].data.can_load = GetPrivateProfileIntA("data", "can_load", TRANS_NONE, file.c_str());
    building[building_anz-1].data.can_attack = GetPrivateProfileIntA("data", "can_attack", ATTACK_NONE, file.c_str());
    building[building_anz-1].data.muzzle_typ = GetPrivateProfileIntA("data", "muzzle_typ", MUZZLE_BIG, file.c_str());
    building[building_anz-1].data.human_prod = GetPrivateProfileIntA("data", "human_prod", 0, file.c_str());
    building[building_anz-1].data.human_need = GetPrivateProfileIntA("data", "human_need", 0, file.c_str());

    building[building_anz-1].data.is_base = GetPrivateProfileIntA("data", "is_base", 0, file.c_str());
    building[building_anz-1].data.is_big = building[building_anz-1].img_org->h>64;
    building[building_anz-1].data.is_road = GetPrivateProfileIntA("data", "is_road", 0, file.c_str());
    building[building_anz-1].data.is_connector = GetPrivateProfileIntA("data", "is_connector", 0, file.c_str());
    building[building_anz-1].data.has_effect = GetPrivateProfileIntA("data", "has_effect", 0, file.c_str());
    building[building_anz-1].data.is_mine = GetPrivateProfileIntA("data", "is_mine", 0, file.c_str());
    building[building_anz-1].data.is_annimated = GetPrivateProfileIntA("data", "is_annimated", 0, file.c_str());
    building[building_anz-1].data.is_pad = GetPrivateProfileIntA("data", "is_pad", 0, file.c_str());
    building[building_anz-1].data.is_expl_mine = GetPrivateProfileIntA("data", "is_expl_mine", 0, file.c_str());
    building[building_anz-1].data.can_research = GetPrivateProfileIntA("data", "can_research", 0, file.c_str());
    building[building_anz-1].data.build_alien = GetPrivateProfileIntA("data", "build_alien", 0, file.c_str());
    building[building_anz-1].data.is_alien = GetPrivateProfileIntA("data", "is_alien", 0, file.c_str());

    building[building_anz-1].data.is_bridge = GetPrivateProfileIntA("data", "is_bridge", 0, file.c_str());
    building[building_anz-1].data.is_platform = GetPrivateProfileIntA("data", "is_platform", 0, file.c_str());
    building[building_anz-1].data.build_on_water = GetPrivateProfileIntA("data", "build_on_water", 0, file.c_str());
    if(building[building_anz-1].data.is_bridge || building[building_anz-1].data.is_platform){
      building[building_anz-1].data.build_on_water = true;
    }

	GetPrivateProfileStringA("data", "id", "", IniTmp, 255, file.c_str() );
    strncpy(building[building_anz-1].id,IniTmp,3);
    building[building_anz-1].id[3] = 0;

    // Den Infotext auslesen:
	GetPrivateProfileStringA("data", "text", "", IniTmp, 255, file.c_str() );
	TmpStr = IniTmp;
	TmpStr.replace(TmpStr.find("\\",0),4,"\n\n");
    building[building_anz-1].text=(char*)malloc(TmpStr.length()+1);
    strcpy(building[building_anz-1].text,TmpStr.c_str());

    // Prüfen, ob es mehrere Frames gibt:
    if(building[building_anz-1].img_org->w>128&&!building[building_anz-1].data.is_connector){
      building[building_anz-1].data.has_frames=building[building_anz-1].img_org->w/building[building_anz-1].img_org->h;
    }else{
      building[building_anz-1].data.has_frames=0;
    }

    // Ggf den Effect laden:
    if(building[building_anz-1].data.has_effect){
	  file = p; file.insert(0,"buildings\\"); file += "effect.pcx";
      if(!FileExists(file.c_str())){
        MessageBoxA(NULL,"File not found: data.ini","LoadBuildings",MB_ICONERROR);
        return 0;
      }
	  strcpy(tmp, file.c_str());
      building[building_anz-1].eff_org=LoadPCX(tmp);
      SDL_SetColorKey(building[building_anz-1].eff_org,SDL_SRCCOLORKEY,0xFF00FF);
      building[building_anz-1].eff=LoadPCX(tmp);
      SDL_SetColorKey(building[building_anz-1].eff,SDL_SRCCOLORKEY,0xFF00FF);
      SDL_SetAlpha(building[building_anz-1].eff,SDL_SRCALPHA,10);
    }else{
      building[building_anz-1].eff=NULL;
      building[building_anz-1].eff_org=NULL;
    }

    // Ggf Ptr auf Surface anlegen:
    if(building[building_anz-1].data.is_connector){
      ptr_connector=building[building_anz-1].img;
      SDL_SetColorKey(ptr_connector,SDL_SRCCOLORKEY,0xFF00FF);
      ptr_connector_shw=building[building_anz-1].shw;
      SDL_SetColorKey(ptr_connector_shw,SDL_SRCCOLORKEY,0xFF00FF);
    }else if(building[building_anz-1].data.is_road){
      ptr_small_beton=building[building_anz-1].img;
      SDL_SetColorKey(ptr_small_beton,SDL_SRCCOLORKEY,0xFF00FF);
    }

    // Festlegen, on das Gebäude arbeiten kann:
    if(building[building_anz-1].data.energy_prod||building[building_anz-1].data.energy_need){
      building[building_anz-1].data.can_work=true;
    }else{
      building[building_anz-1].data.can_work=false;
    }

	#define LOADBUISND(a,b) if(!FileExists(b)){MessageBoxA(NULL,"File not found: ","LoadBuildingSounds",MB_ICONERROR);return 0;} a = Mix_LoadWAV(b);
    // Ggf noch Sounds laden:
    if(building[building_anz-1].data.can_work){
	  file = p; file.insert(0,"buildings\\"); file += "start.wav";
	  LOADBUISND(building[building_anz-1].Start, file.c_str())
	  file = p; file.insert(0,"buildings\\"); file += "running.wav";
      LOADBUISND(building[building_anz-1].Running, file.c_str())
	  file = p; file.insert(0,"buildings\\"); file += "stop.wav";
      LOADBUISND(building[building_anz-1].Stop, file.c_str())
    }else{
      building[building_anz-1].Start=NULL;
      building[building_anz-1].Running=NULL;
      building[building_anz-1].Stop=NULL;
    }
    if(building[building_anz-1].data.can_attack){
	  file = p; file.insert(0,"buildings\\"); file += "attack.wav";
      LOADBUISND(building[building_anz-1].Attack, file.c_str())
    }else{
      building[building_anz-1].Attack=NULL;
    }
  }
  while( _findnext( hFile, &c_file ) == 0);
  _findclose(hFile);	
  // Die Buildings sortieren:
  changed=true;
  max=building_anz-1;
  while(changed&&max>0){
    changed=false;
    for(i=0;i<max;i++){
      if(stricmp(building[i].id,building[i+1].id)==0){
		MessageBoxA(NULL,"double defined ID","LoadBuildings",MB_ICONERROR);
        return 0;
      }
      if(teststr(building[i].id,building[i+1].id)){
        sBuilding tmpBuil;
        tmpBuil=building[i];
        building[i]=building[i+1];
        building[i+1]=tmpBuil;
        changed=true;
      }
    }
    max--;
  }
  for(i=0;i<building_anz;i++)building[i].nr=i;
  // Die Dirt-Surfaces laden:
  file = "buildings\\"; file += "dirt_small.pcx";
  if(!FileExists(file.c_str())){
    MessageBoxA(NULL,"File not found: dirt_small.pcx","LoadBuildings",MB_ICONERROR);
    return 0;
  }
  strcpy(tmp, file.c_str());
  dirt_small_org=LoadPCX(tmp,true);
  dirt_small=LoadPCX(tmp,true);
  SDL_SetColorKey(dirt_small,SDL_SRCCOLORKEY,0xFF00FF);
  file = "buildings\\"; file += "dirt_small_shw.pcx";
  if(!FileExists(file.c_str())){
    MessageBoxA(NULL,"File not found: dirt_small_shw.pcx","LoadBuildings",MB_ICONERROR);
    return 0;
  }
  strcpy(tmp, file.c_str());
  dirt_small_shw_org=LoadPCX(tmp,true);
  dirt_small_shw=LoadPCX(tmp,true);
  SDL_SetColorKey(dirt_small_shw,SDL_SRCCOLORKEY,0xFF00FF);
  SDL_SetAlpha(dirt_small_shw,SDL_SRCALPHA,50);

  file = "buildings\\"; file += "dirt_big.pcx";
  if(!FileExists(file.c_str())){
    MessageBoxA(NULL,"File not found: dirt_big.pcx","LoadBuildings",MB_ICONERROR);
    return 0;
  }
  strcpy(tmp, file.c_str());
  dirt_big_org=LoadPCX(tmp,true);
  dirt_big=LoadPCX(tmp,true);
  SDL_SetColorKey(dirt_big,SDL_SRCCOLORKEY,0xFF00FF);
  file = "buildings\\"; file += "dirt_big_shw.pcx";
  if(!FileExists(file.c_str())){
    MessageBoxA(NULL,"File not found: dirt_big_shw.pcx","LoadBuildings",MB_ICONERROR);
    return 0;
  }
  strcpy(tmp, file.c_str());
  dirt_big_shw_org=LoadPCX(tmp,true);
  dirt_big_shw=LoadPCX(tmp,true);
  SDL_SetColorKey(dirt_big_shw,SDL_SRCCOLORKEY,0xFF00FF);
  SDL_SetAlpha(dirt_big_shw,SDL_SRCALPHA,50);

  // Prüfen, ob alle Ptr eingerichtet wurden:
  if(!ptr_connector||!ptr_connector_shw||!ptr_small_beton){
    MessageBoxA(NULL,"Missing road or connector building","LoadBuildings",MB_ICONERROR);
    return 0;
  }

  // Prüfen, ob es die Land/See-Minen gibt:
  for(i=0;i<building_anz;i++){
    if(!building[i].data.is_expl_mine)continue;
    if(building[i].data.build_on_water)BNrSeaMine=i;
    else BNrLandMine=i;
    if(BNrLandMine&&BNrSeaMine)break;
  }
  if(!BNrLandMine||!BNrSeaMine){
    MessageBoxA(NULL,"Missing land or sea-mine","LoadBuildings",MB_ICONERROR);
    return 0;
  }
  // Gebäude für die Landung suchen:
  for(i=0;i<building_anz;i++){
    if(building[i].data.is_mine&&!building[i].data.is_alien){
      BNrMine=i;
    }else if(building[i].data.can_load==TRANS_OIL){
      BNrOilStore=i;
    }else if(building[i].data.energy_prod==1){
      BNrSmallGen=i;
    }
    if(BNrSmallGen&&BNrOilStore&&BNrMine)break;
  }
  return 1;
}

// DeleteBuildings ///////////////////////////////////////////////////////////
// Löscht alle Buildings:
void DeleteBuildings(void){
  int i;
  for(i=0;i<building_anz;i++){
    SDL_FreeSurface(building[i].shw_org);
    SDL_FreeSurface(building[i].shw);
    SDL_FreeSurface(building[i].img_org);
    SDL_FreeSurface(building[i].img);
    SDL_FreeSurface(building[i].video);
    if(building[i].eff_org)SDL_FreeSurface(building[i].eff_org);
    if(building[i].eff)SDL_FreeSurface(building[i].eff);
    if(building[i].Start)DELSND(building[i].Start)
    if(building[i].Running)DELSND(building[i].Running)
    if(building[i].Stop)DELSND(building[i].Stop)
    if(building[i].Attack)DELSND(building[i].Attack)    
    free(building[i].text);
  }
  free(building);
  SDL_FreeSurface(dirt_small_org);
  SDL_FreeSurface(dirt_small);
  SDL_FreeSurface(dirt_small_shw_org);
  SDL_FreeSurface(dirt_small_shw);
  SDL_FreeSurface(dirt_big_org);
  SDL_FreeSurface(dirt_big);
  SDL_FreeSurface(dirt_big_shw_org);
  SDL_FreeSurface(dirt_big_shw);
}

// LoadVehicles //////////////////////////////////////////////////////////////
// Läd alle Vehicles:
int LoadVehicles(){
	string p, file, TmpStr;
	char TmpChr[4];
	bool changed;
	int i,max;
	vehicle_anz=0;
	string suchdir = "vehicles\\*.*";
	long hFile;
	char tmp[256];
	char IniTmp[256];

	if( (hFile = _findfirst( suchdir.c_str(), &c_file )) == -1L ) {
		_findclose(hFile); return 0;
	}

	// Alle Unterverzeichnisse durchsuchen:
	do{
		if (c_file.size == 0 && strcmp(c_file.name,".") && strcmp(c_file.name,"..")) ;
		else continue;
		p = c_file.name;
		p += "\\";
		vehicle_anz++;
		vehicle=(sVehicle*)realloc(vehicle,sizeof(sVehicle)*vehicle_anz);
		memset(&(vehicle[vehicle_anz-1].data),0,sizeof(sVehicleData));

		if(!stricmp(c_file.name,"infantery") || !stricmp(c_file.name,"commando")){
			if(!LoadInfantery(vehicle+(vehicle_anz-1),p)){
				return 0;
			}
		}else {
			for(i=0;i<8;i++){
				// img laden:
				itoa(i, TmpChr,10); file = p; file.insert(0,"vehicles\\"); file += "img"; file.insert(file.length(),TmpChr); file += ".pcx";
				if(!FileExists(file.c_str())){
					MessageBoxA(NULL,"File not found: img.pcx","LoadVehicles",MB_ICONERROR);
					return 0;
				}
				strcpy(tmp, file.c_str());
				if(vehicle_anz == 34)
					int h = 1;
				vehicle[vehicle_anz-1].img_org[i] = LoadPCX(tmp);
				SDL_SetColorKey(vehicle[vehicle_anz-1].img_org[i],SDL_SRCCOLORKEY,0xFFFFFF);
				vehicle[vehicle_anz-1].img[i] = LoadPCX(tmp);
				SDL_SetColorKey(vehicle[vehicle_anz-1].img[i],SDL_SRCCOLORKEY,0xFFFFFF);
				// shw laden:
				itoa(i, TmpChr,10); file = p; file.insert(0,"vehicles\\"); file += "shw"; file.insert(file.length(),TmpChr); file += ".pcx";
				if(!FileExists(file.c_str())){
					MessageBoxA(NULL,"File not found: shw.pcx","LoadVehicles",MB_ICONERROR);
					return 0;
				}
				strcpy(tmp, file.c_str());
				vehicle[vehicle_anz-1].shw_org[i] = LoadPCX(tmp);
				SDL_SetColorKey(vehicle[vehicle_anz-1].shw_org[i],SDL_SRCCOLORKEY,0xFF00FF);
				vehicle[vehicle_anz-1].shw[i] = LoadPCX(tmp);
				SDL_SetAlpha(vehicle[vehicle_anz-1].shw[i],SDL_SRCALPHA,50);
				SDL_SetColorKey(vehicle[vehicle_anz-1].shw[i],SDL_SRCCOLORKEY,0xFF00FF);
			}
		}
		// Das Video laden:
		file = p; file.insert(0,"vehicles\\"); file += "video.flc";
		vehicle[vehicle_anz-1].FLCFile=(char*)malloc(strlen(file.c_str())+1);
		strcpy(vehicle[vehicle_anz-1].FLCFile,file.c_str());

		if(!FileExists(file.c_str())){
			MessageBoxA(NULL,"File not found: video.flc","LoadVehicles",MB_ICONERROR);
			return 0;
		}
		// Das Infobild laden:
		file = p; file.insert(0,"vehicles\\"); file += "info.pcx";
		if(!FileExists(file.c_str())){
			MessageBoxA(NULL,"File not found: info.pcx","LoadVehicles",MB_ICONERROR);
			return 0;
		}
		strcpy(tmp, file.c_str());
		vehicle[vehicle_anz-1].info=LoadPCX(tmp);

		// Das Storagebild laden:
		file = p; file.insert(0,"vehicles\\"); file += "store.pcx";
		if(!FileExists(file.c_str())){
			MessageBoxA(NULL,"File not found: strore.pcx","LoadVehicles",MB_ICONERROR);
			return 0;
		}
		strcpy(tmp, file.c_str());
		vehicle[vehicle_anz-1].storage=LoadPCX(tmp);

		// Die Daten laden:
		file = p; file.insert(0,"vehicles\\"); file += "data.ini";
		if(!FileExists(file.c_str())){
			MessageBoxA(NULL,"File not found: data.ini","LoadVehicles",MB_ICONERROR);
			return 0;
		}

		vehicle[vehicle_anz-1].data.version=1;
		GetPrivateProfileStringA("data", "name", "unknown", IniTmp, 255, file.c_str() );
		strncpy(vehicle[vehicle_anz-1].data.name,IniTmp,24);

		vehicle[vehicle_anz-1].data.max_speed = GetPrivateProfileIntA("data", "max_speed", 1, file.c_str())*2;
		vehicle[vehicle_anz-1].data.speed = 0;
		vehicle[vehicle_anz-1].data.max_hit_points = GetPrivateProfileIntA("data", "max_hit_points", 1, file.c_str());
		vehicle[vehicle_anz-1].data.hit_points = 0;
		vehicle[vehicle_anz-1].data.armor = GetPrivateProfileIntA("data", "armor", 1, file.c_str());
		vehicle[vehicle_anz-1].data.scan = GetPrivateProfileIntA("data", "scan", 1, file.c_str());
		vehicle[vehicle_anz-1].data.range = GetPrivateProfileIntA("data", "range", 1, file.c_str());
		vehicle[vehicle_anz-1].data.max_shots = GetPrivateProfileIntA("data", "max_shots", 1, file.c_str());
		vehicle[vehicle_anz-1].data.shots = 0;
		vehicle[vehicle_anz-1].data.damage = GetPrivateProfileIntA("data", "damage", 1, file.c_str());
		vehicle[vehicle_anz-1].data.max_cargo = GetPrivateProfileIntA("data", "max_cargo", 0, file.c_str());
		vehicle[vehicle_anz-1].data.cargo = 0;
		vehicle[vehicle_anz-1].data.max_ammo = GetPrivateProfileIntA("data", "max_ammo", 0, file.c_str());
		vehicle[vehicle_anz-1].data.ammo = 0;
		vehicle[vehicle_anz-1].data.costs = GetPrivateProfileIntA("data", "costs", 1, file.c_str());

		vehicle[vehicle_anz-1].data.can_build = GetPrivateProfileIntA("data", "can_build", BUILD_NONE, file.c_str());
		vehicle[vehicle_anz-1].data.can_drive = GetPrivateProfileIntA("data", "can_drive", DRIVE_LAND, file.c_str());
		vehicle[vehicle_anz-1].data.can_transport = GetPrivateProfileIntA("data", "can_transport", TRANS_NONE, file.c_str());
		vehicle[vehicle_anz-1].data.can_attack = GetPrivateProfileIntA("data", "can_attack", ATTACK_NONE, file.c_str());
		vehicle[vehicle_anz-1].data.muzzle_typ = GetPrivateProfileIntA("data", "muzzle_typ", MUZZLE_BIG, file.c_str());

		vehicle[vehicle_anz-1].data.can_reload = GetPrivateProfileIntA("data", "can_reload", 0, file.c_str());
		vehicle[vehicle_anz-1].data.can_repair = GetPrivateProfileIntA("data", "can_repair", 0, file.c_str());
		vehicle[vehicle_anz-1].data.can_drive_and_fire = GetPrivateProfileIntA("data", "can_drive_and_fire", 0, file.c_str());
		vehicle[vehicle_anz-1].data.is_stealth_land = GetPrivateProfileIntA("data", "is_stealth_land", 0, file.c_str());
		vehicle[vehicle_anz-1].data.is_stealth_sea = GetPrivateProfileIntA("data", "is_stealth_sea", 0, file.c_str());
		vehicle[vehicle_anz-1].data.is_human = GetPrivateProfileIntA("data", "is_human", 0, file.c_str());
		vehicle[vehicle_anz-1].data.can_survey = GetPrivateProfileIntA("data", "can_survey", 0, file.c_str());
		vehicle[vehicle_anz-1].data.can_clear = GetPrivateProfileIntA("data", "can_clear", 0, file.c_str());
		vehicle[vehicle_anz-1].data.has_overlay = GetPrivateProfileIntA("data", "has_overlay", 0, file.c_str());
		vehicle[vehicle_anz-1].data.build_by_big = GetPrivateProfileIntA("data", "build_by_big", 0, file.c_str());
		vehicle[vehicle_anz-1].data.can_lay_mines = GetPrivateProfileIntA("data", "can_lay_mines", 0, file.c_str());
		vehicle[vehicle_anz-1].data.can_detect_mines = GetPrivateProfileIntA("data", "can_detect_mines", 0, file.c_str());
		vehicle[vehicle_anz-1].data.can_detect_sea = GetPrivateProfileIntA("data", "can_detect_sea", 0, file.c_str());
		vehicle[vehicle_anz-1].data.can_detect_land = GetPrivateProfileIntA("data", "can_detect_land", 0, file.c_str());
		vehicle[vehicle_anz-1].data.make_tracks = GetPrivateProfileIntA("data", "make_tracks", 0, file.c_str());
		vehicle[vehicle_anz-1].data.is_commando = GetPrivateProfileIntA("data", "is_commando", 0, file.c_str());
		vehicle[vehicle_anz-1].data.is_alien = GetPrivateProfileIntA("data", "is_alien", 0, file.c_str());

		GetPrivateProfileStringA("data", "id", "", IniTmp, 255, file.c_str() );
		strncpy(vehicle[vehicle_anz-1].id,IniTmp,3);
		vehicle[vehicle_anz-1].id[3] = 0;

		 // Den Infotext auslesen:
		GetPrivateProfileStringA("data", "text", "", IniTmp, 255, file.c_str() );
		TmpStr = IniTmp;
		TmpStr.replace(TmpStr.find("\\",0),4,"\n\n");
		vehicle[vehicle_anz-1].text=(char*)malloc(TmpStr.length()+1);
		strcpy(vehicle[vehicle_anz-1].text,TmpStr.c_str());

		// Ggf Overlay laden:
		if(vehicle[vehicle_anz-1].data.has_overlay){
			file = p; file.insert(0,"vehicles\\"); file += "overlay.pcx";
			if(!FileExists(file.c_str())){
				MessageBoxA(NULL,"File not found: overlay.pcx","LoadVehicles",MB_ICONERROR);
				return 0;
			}
			strcpy(tmp, file.c_str());
			vehicle[vehicle_anz-1].overlay_org=LoadPCX(tmp);
			vehicle[vehicle_anz-1].overlay=LoadPCX(tmp);
		}else{
			vehicle[vehicle_anz-1].overlay_org=NULL;
			vehicle[vehicle_anz-1].overlay=NULL;
		}

		// Build-Surfaces laden:
		if(vehicle[vehicle_anz-1].data.can_build){
			// img laden:
			file = p; file.insert(0,"vehicles\\"); file += "build.pcx";
			if(!FileExists(file.c_str())){
				MessageBoxA(NULL,"File not found: build.pcx","LoadVehicles",MB_ICONERROR);
				return 0;
			}
			strcpy(tmp, file.c_str());
			vehicle[vehicle_anz-1].build_org=LoadPCX(tmp);
			SDL_SetColorKey(vehicle[vehicle_anz-1].build_org,SDL_SRCCOLORKEY,0xFFFFFF);
			vehicle[vehicle_anz-1].build=LoadPCX(tmp);
			SDL_SetColorKey(vehicle[vehicle_anz-1].build,SDL_SRCCOLORKEY,0xFFFFFF);
			// shw laden:
			file = p; file.insert(0,"vehicles\\"); file += "build_shw.pcx";
			if(!FileExists(file.c_str())){
				MessageBoxA(NULL,"File not found: build_shw.pcx","LoadVehicles",MB_ICONERROR);
				return 0;
			}
			strcpy(tmp, file.c_str());
			vehicle[vehicle_anz-1].build_shw_org=LoadPCX(tmp);
			SDL_SetColorKey(vehicle[vehicle_anz-1].build_shw_org,SDL_SRCCOLORKEY,0xFF00FF);
			vehicle[vehicle_anz-1].build_shw=LoadPCX(tmp);
			SDL_SetAlpha(vehicle[vehicle_anz-1].build_shw,SDL_SRCALPHA,50);
			SDL_SetColorKey(vehicle[vehicle_anz-1].build_shw,SDL_SRCCOLORKEY,0xFF00FF);
		}else{
			vehicle[vehicle_anz-1].build=NULL;
			vehicle[vehicle_anz-1].build_org=NULL;
			vehicle[vehicle_anz-1].build_shw=NULL;
			vehicle[vehicle_anz-1].build_shw_org=NULL;
		}

		// Clear-Surfaces laden:
		if(vehicle[vehicle_anz-1].data.can_clear){
			// img laden:
			file = p; file.insert(0,"vehicles\\"); file += "clear_small.pcx";
			if(!FileExists(file.c_str())){
				MessageBoxA(NULL,"File not found: clear_small.pcx","LoadVehicles",MB_ICONERROR);
				return 0;
			}
			strcpy(tmp, file.c_str());
			vehicle[vehicle_anz-1].clear_small_org=LoadPCX(tmp);
			SDL_SetColorKey(vehicle[vehicle_anz-1].clear_small_org,SDL_SRCCOLORKEY,0xFFFFFF);
			vehicle[vehicle_anz-1].clear_small=LoadPCX(tmp);
			SDL_SetColorKey(vehicle[vehicle_anz-1].clear_small,SDL_SRCCOLORKEY,0xFFFFFF);
			// shw laden:
			file = p; file.insert(0,"vehicles\\"); file += "clear_small_shw.pcx";
			if(!FileExists(file.c_str())){
				MessageBoxA(NULL,"File not found: clear_small_shw.pcx","LoadVehicles",MB_ICONERROR);
				return 0;
			}
			strcpy(tmp, file.c_str());
			vehicle[vehicle_anz-1].clear_small_shw_org=LoadPCX(tmp);
			SDL_SetColorKey(vehicle[vehicle_anz-1].clear_small_shw_org,SDL_SRCCOLORKEY,0xFF00FF);
			vehicle[vehicle_anz-1].clear_small_shw=LoadPCX(tmp);
			SDL_SetAlpha(vehicle[vehicle_anz-1].clear_small_shw,SDL_SRCALPHA,50);
			SDL_SetColorKey(vehicle[vehicle_anz-1].clear_small_shw,SDL_SRCCOLORKEY,0xFF00FF);
			// img laden:
			file = p; file.insert(0,"vehicles\\"); file += "clear_big.pcx";
			if(!FileExists(file.c_str())){
				MessageBoxA(NULL,"File not found: clear_big.pcx","LoadVehicles",MB_ICONERROR);
				return 0;
			}
			strcpy(tmp, file.c_str());
			vehicle[vehicle_anz-1].build_org=LoadPCX(tmp);
			SDL_SetColorKey(vehicle[vehicle_anz-1].build_org,SDL_SRCCOLORKEY,0xFFFFFF);
			vehicle[vehicle_anz-1].build=LoadPCX(tmp);
			SDL_SetColorKey(vehicle[vehicle_anz-1].build,SDL_SRCCOLORKEY,0xFFFFFF);
			// shw laden:
			file = p; file.insert(0,"vehicles\\"); file += "clear_big_shw.pcx";
			if(!FileExists(file.c_str())){
				MessageBoxA(NULL,"File not found: clear_big_shw.pcx","LoadVehicles",MB_ICONERROR);
				return 0;
			}
			strcpy(tmp, file.c_str());
			vehicle[vehicle_anz-1].build_shw_org=LoadPCX(tmp);
			SDL_SetColorKey(vehicle[vehicle_anz-1].build_shw_org,SDL_SRCCOLORKEY,0xFF00FF);
			vehicle[vehicle_anz-1].build_shw=LoadPCX(tmp);
			SDL_SetAlpha(vehicle[vehicle_anz-1].build_shw,SDL_SRCALPHA,50);
			SDL_SetColorKey(vehicle[vehicle_anz-1].build_shw,SDL_SRCCOLORKEY,0xFF00FF);
		}else{
			vehicle[vehicle_anz-1].clear_small=NULL;
			vehicle[vehicle_anz-1].clear_small_org=NULL;
			vehicle[vehicle_anz-1].clear_small_shw=NULL;
			vehicle[vehicle_anz-1].clear_small_shw_org=NULL;
		}

		#define LOADVEHSND(a,b) if(!FileExists(b)){MessageBoxA(NULL,"SoundFile not found: ","LoadVehicleSounds",MB_ICONERROR);return 0;} a = Mix_LoadWAV(b);

		// Die Sounds laden:
		switch(vehicle[vehicle_anz-1].data.can_drive){
			case DRIVE_LAND:
			case DRIVE_AIR:
				file = p; file.insert(0,"vehicles\\"); file += "wait.wav";
				LOADVEHSND(vehicle[vehicle_anz-1].Wait,file.c_str())
				vehicle[vehicle_anz-1].WaitWater=NULL;
				if(!vehicle[vehicle_anz-1].data.is_human){file = p; file.insert(0,"vehicles\\"); file += "wait.wav"; LOADVEHSND(vehicle[vehicle_anz-1].Start,file.c_str())}else{vehicle[vehicle_anz-1].Start=NULL;}
				vehicle[vehicle_anz-1].StartWater=NULL;
				if(!vehicle[vehicle_anz-1].data.is_human){file = p; file.insert(0,"vehicles\\"); file += "wait.wav"; LOADVEHSND(vehicle[vehicle_anz-1].Stop,file.c_str())}else{vehicle[vehicle_anz-1].Stop=NULL;}
				vehicle[vehicle_anz-1].StopWater=NULL;
				file = p; file.insert(0,"vehicles\\"); file += "drive.wav";
				LOADVEHSND(vehicle[vehicle_anz-1].Drive,file.c_str())
				vehicle[vehicle_anz-1].DriveWater=NULL;
				break;
			case DRIVE_SEA:
				file = p; file.insert(0,"vehicles\\"); file += "wait_water.wav";
				LOADVEHSND(vehicle[vehicle_anz-1].WaitWater,file.c_str())
				vehicle[vehicle_anz-1].Wait=NULL;
				file = p; file.insert(0,"vehicles\\"); file += "start_water.wav";
				LOADVEHSND(vehicle[vehicle_anz-1].StartWater,file.c_str())
				vehicle[vehicle_anz-1].Start=NULL;
				file = p; file.insert(0,"vehicles\\"); file += "stop_water.wav";
				LOADVEHSND(vehicle[vehicle_anz-1].StopWater,file.c_str())
				vehicle[vehicle_anz-1].Stop=NULL;
				file = p; file.insert(0,"vehicles\\"); file += "drive_water.wav";
				LOADVEHSND(vehicle[vehicle_anz-1].DriveWater,file.c_str())
				vehicle[vehicle_anz-1].Drive=NULL;
				break;
			case DRIVE_LANDnSEA:
				file = p; file.insert(0,"vehicles\\"); file += "wait.wav";
				LOADVEHSND(vehicle[vehicle_anz-1].Wait,file.c_str())
				file = p; file.insert(0,"vehicles\\"); file += "wait_water.wav";
				LOADVEHSND(vehicle[vehicle_anz-1].WaitWater,file.c_str())
				file = p; file.insert(0,"vehicles\\"); file += "start.wav";
				LOADVEHSND(vehicle[vehicle_anz-1].Start,file.c_str())
				file = p; file.insert(0,"vehicles\\"); file += "start_water.wav";
				LOADVEHSND(vehicle[vehicle_anz-1].StartWater,file.c_str())
				file = p; file.insert(0,"vehicles\\"); file += "stop.wav";
				LOADVEHSND(vehicle[vehicle_anz-1].Stop,file.c_str())
				file = p; file.insert(0,"vehicles\\"); file += "stop_water.wav";
				LOADVEHSND(vehicle[vehicle_anz-1].StopWater,file.c_str())
				file = p; file.insert(0,"vehicles\\"); file += "drive.wav";
				LOADVEHSND(vehicle[vehicle_anz-1].Drive,file.c_str())
				file = p; file.insert(0,"vehicles\\"); file += "drive_water.wav";
				LOADVEHSND(vehicle[vehicle_anz-1].DriveWater,file.c_str())
				break;
		}
		if(vehicle[vehicle_anz-1].data.can_attack){
			file = p; file.insert(0,"vehicles\\"); file += "attack.wav";
			LOADVEHSND(vehicle[vehicle_anz-1].Attack,file.c_str())
		}else{
			vehicle[vehicle_anz-1].Attack=NULL;
		}
	}
	while( _findnext( hFile, &c_file ) == 0);
	_findclose(hFile);
	// Die Vehicles sortieren:
	changed=true;
	max=vehicle_anz-1;
	while(changed&&max>0){
		changed=false;
		for(i=0;i<max;i++){
			if(stricmp(vehicle[i].id,vehicle[i+1].id)==0){
				MessageBoxA(NULL,"double defined ID","LoadVehicles",MB_ICONERROR);
				return 0;
			}
			if(teststr(vehicle[i].id,vehicle[i+1].id)){
				sVehicle tmp;
				tmp=vehicle[i];
				vehicle[i]=vehicle[i+1];
				vehicle[i+1]=tmp;
				changed=true;
			}
		}
		max--;
	}
	for(i=0;i<vehicle_anz;i++)vehicle[i].nr=i;
	return 1;
}

// DeleteVehicles ////////////////////////////////////////////////////////////
// Löscht alle Vehicles:
void DeleteVehicles(void){
  int i,k;
  for(i=0;i<vehicle_anz;i++){
    for(k=0;k<8;k++){
      SDL_FreeSurface(vehicle[i].shw_org[k]);
      SDL_FreeSurface(vehicle[i].shw[k]);
      SDL_FreeSurface(vehicle[i].img_org[k]);
      SDL_FreeSurface(vehicle[i].img[k]);
    }
    if(vehicle[i].build){
      SDL_FreeSurface(vehicle[i].build);
      SDL_FreeSurface(vehicle[i].build_org);
      SDL_FreeSurface(vehicle[i].build_shw);
      SDL_FreeSurface(vehicle[i].build_shw_org);
    }
    if(vehicle[i].clear_small){
      SDL_FreeSurface(vehicle[i].clear_small);
      SDL_FreeSurface(vehicle[i].clear_small_org);
      SDL_FreeSurface(vehicle[i].clear_small_shw);
      SDL_FreeSurface(vehicle[i].clear_small_shw_org);
    }
    if(vehicle[i].overlay){
      SDL_FreeSurface(vehicle[i].overlay);
      SDL_FreeSurface(vehicle[i].overlay_org);
    }
    free(vehicle[i].FLCFile);
    free(vehicle[i].text);
    SDL_FreeSurface(vehicle[i].info);
    if(vehicle[i].Wait)DELSND(vehicle[i].Wait)
    if(vehicle[i].WaitWater)DELSND(vehicle[i].WaitWater)
    if(vehicle[i].Start)DELSND(vehicle[i].Start)
    if(vehicle[i].StartWater)DELSND(vehicle[i].StartWater)
    if(vehicle[i].Stop)DELSND(vehicle[i].Stop)
    if(vehicle[i].StopWater)DELSND(vehicle[i].StopWater)
    if(vehicle[i].Drive)DELSND(vehicle[i].Drive)
    if(vehicle[i].DriveWater)DELSND(vehicle[i].DriveWater)
    if(vehicle[i].Attack)DELSND(vehicle[i].Attack)
  }
  free(vehicle);
}

// Läd alle Infanteriebilder und verarbeitet sie:
bool LoadInfantery(sVehicle *v,string path){
	SDL_Surface *sf;
	SDL_Rect dest;
	int i,k,*ptr;
	string file, TmpStr;
	char TmpChr[4];
	char tmp[256];


	for(i=0;i<8;i++){
		v->img[i]=SDL_CreateRGBSurface(SDL_HWSURFACE|SDL_SRCCOLORKEY,64*13,64,32,0,0,0,0);
		SDL_SetColorKey(v->img[i],SDL_SRCCOLORKEY,0xFFFFFF);
		SDL_FillRect(v->img[i],NULL,0xFF00FF);

		for(k=0;k<13;k++){
			file = path; file.insert(0,"vehicles\\"); file += "img"; itoa(i, TmpChr,10); file.insert(file.length(),TmpChr); file += "_"; if(k<10) file += "0"; itoa(k, TmpChr,10); file.insert(file.length(),TmpChr); file += ".pcx";
			if(!FileExists(file.c_str())){
				MessageBoxA(NULL,"File not found: imgx_y.pcx","LoadInfantery",MB_ICONERROR);
				return 0;
			}
			strcpy(tmp, file.c_str());
			sf = LoadPCX(tmp);
			dest.x=64*k+32-sf->w/2;
			dest.y=32-sf->h/2;
			dest.w=sf->w;
			dest.h=sf->h;
			SDL_BlitSurface(sf,NULL,v->img[i],&dest);
			SDL_FreeSurface(sf);
		}

		v->img_org[i]=SDL_CreateRGBSurface(SDL_HWSURFACE|SDL_SRCCOLORKEY,64*13,64,32,0,0,0,0);
		SDL_SetColorKey(v->img[i],SDL_SRCCOLORKEY,0xFFFFFF);
		SDL_FillRect(v->img_org[i],NULL,0xFFFFFF);
		SDL_BlitSurface(v->img[i],NULL,v->img_org[i],NULL);

		v->shw[i]=SDL_CreateRGBSurface(SDL_HWSURFACE|SDL_SRCCOLORKEY,64*13,64,32,0,0,0,0);
		SDL_SetColorKey(v->shw[i],SDL_SRCCOLORKEY,0xFF00FF);
		SDL_FillRect(v->shw[i],NULL,0xFF00FF);
		v->shw_org[i]=SDL_CreateRGBSurface(SDL_HWSURFACE|SDL_SRCCOLORKEY,64*13,64,32,0,0,0,0);
		SDL_SetColorKey(v->shw_org[i],SDL_SRCCOLORKEY,0xFF00FF);
		SDL_FillRect(v->shw_org[i],NULL,0xFF00FF);

		dest.x=3;dest.y=3;dest.w=64*13;dest.h=64;
		SDL_BlitSurface(v->img_org[i],NULL,v->shw_org[i],&dest);
		SDL_LockSurface(v->shw_org[i]);
		ptr=(int*)(v->shw_org[i]->pixels);
		for(k=0;k<64*13*64;k++){
			if(*ptr!=0xFF00FF)*ptr=0;
			ptr++;
		}
		SDL_UnlockSurface(v->shw_org[i]);
		SDL_BlitSurface(v->shw_org[i],NULL,v->shw[i],NULL);
		SDL_SetAlpha(v->shw_org[i],SDL_SRCALPHA,50);
		SDL_SetAlpha(v->shw[i],SDL_SRCALPHA,50);    
	}
	return true;
}

// Vergleicht die beiden Strings (true wenn s1 größer ist):
bool teststr(char *s1,char *s2){
  unsigned char c1,c2;

  while(1){
    c1=*s1++;
    c2=*s2++;
    if(c2==0)return true;
    if(c1==0)return false;
    if(c1==c2)continue;
    if(c1>c2)return true;
    return false;
  }
}

// ScaleSurfaceAdv ///////////////////////////////////////////////////////////
// Skaliert ein Surface in ein Anderes:
void ScaleSurfaceAdv(SDL_Surface *scr,SDL_Surface **dest,int sizex,int sizey){
  int x,y,rx,ry,dx,dy;
  unsigned int *s,*d;
  *dest=SDL_CreateRGBSurface(SDL_HWSURFACE|SDL_SRCCOLORKEY,sizex,sizey,32,0,0,0,0);
  dx=rx=0;
  dy=0;
  SDL_LockSurface(*dest);
  SDL_LockSurface(scr);
  s=(unsigned int*)scr->pixels;
  d=(unsigned int*)(*dest)->pixels;
  ry=scr->h;
  for(y=0;y<scr->h;y++){
    if(ry>=scr->h){
      ry-=scr->h;
      dx=0;
      rx=scr->w;
      for(x=0;x<scr->w;x++){
        if(rx>=scr->w){
          rx-=scr->w;
          d[dx+dy*sizex]=s[x+y*scr->w];
          dx++;
        }
        if(dx>=sizex)break;
        rx+=sizex;
      }
      dy++;
      if(dy>=sizey)break;
    }
    ry+=sizey;
  }
  SDL_UnlockSurface(*dest);
  SDL_UnlockSurface(scr);
}

void ScaleSurfaceAdv2(SDL_Surface *scr,SDL_Surface *dest,int sizex,int sizey){
  int x,y,rx,ry,dx,dy;
  unsigned int *s,*d;
  dx=rx=0;
  dy=0;
  dest->w=scr->w;
  dest->h=scr->h;
  SDL_LockSurface(dest);
  SDL_LockSurface(scr);
  s=(unsigned int*)scr->pixels;
  d=(unsigned int*)dest->pixels;
  ry=scr->h;
  for(y=0;y<scr->h;y++){
    if(ry>=scr->h){
      ry-=scr->h;
      dx=0;
      rx=scr->w;
      for(x=0;x<scr->w;x++){
        if(rx>=scr->w){
          rx-=scr->w;
          d[dx+dy*dest->w]=s[x+y*scr->w];
          dx++;
        }
        if(dx>=sizex)break;
        rx+=sizex;
      }
      dy++;
      if(dy>=sizey)break;
    }
    ry+=sizey;
  }
  SDL_UnlockSurface(dest);
  SDL_UnlockSurface(scr);
  dest->w=sizex;
  dest->h=sizey;
}

void ScaleSurfaceAdv2Spec(SDL_Surface *scr,SDL_Surface *dest,int sizex,int sizey){
  int x,y,rx,ry,dx,dy;
  unsigned int *s,*d;
  dx=rx=0;
  dy=0;
  dest->w=scr->w;
  dest->h=scr->h;
  SDL_LockSurface(dest);
  SDL_LockSurface(scr);
  s=(unsigned int*)scr->pixels;
  d=(unsigned int*)dest->pixels;
  ry=scr->h;
  for(y=0;y<scr->h;y++){
    if(ry>=scr->h){
      ry-=scr->h;
      dx=0;
      rx=scr->w;
      for(x=0;x<scr->w;x++){
        if(rx>=scr->w){
          unsigned int t,sc,de;
          rx-=scr->w;

          sc=x+y*scr->w;
          de=dx+dy*dest->w;
          t=d[de]=s[sc];

          if(t==0xFF00FF){
            if(x>0&&s[sc-1]!=0xFF00FF)d[de]=s[sc-1];
            else if(x<scr->w-1&&s[sc+1]!=0xFF00FF)d[de]=s[sc+1];
          }

          dx++;
        }
        if(dx>=sizex)break;
        rx+=sizex;
      }
      dy++;
      if(dy>=sizey)break;
    }
    ry+=sizey;
  }
  SDL_UnlockSurface(dest);
  SDL_UnlockSurface(scr);
  dest->w=sizex;
  dest->h=sizey;
}

// Erzeigt ein Pfeil-Surface:
SDL_Surface *CreatePfeil(int p1x,int p1y,int p2x,int p2y,int p3x,int p3y,unsigned int color,int size){
  SDL_Surface *sf;
  float fak;
  sf=SDL_CreateRGBSurface(SDL_HWSURFACE|SDL_SRCCOLORKEY,size,size,32,0,0,0,0);
  SDL_SetColorKey(sf,SDL_SRCCOLORKEY,0xFF00FF);
  SDL_FillRect(sf,NULL,0xFF00FF);
  SDL_LockSurface(sf);

  fak=size/64.0;
  line(p1x*fak,p1y*fak,p2x*fak,p2y*fak,color,sf);
  line(p2x*fak,p2y*fak,p3x*fak,p3y*fak,color,sf);
  line(p3x*fak,p3y*fak,p1x*fak,p1y*fak,color,sf);

  SDL_UnlockSurface(sf);
  return sf;
}

// Malt eine Linie auf dem Surface (muss vorher gelocked sein):
void line(int x1,int y1,int x2,int y2,unsigned int color,SDL_Surface *sf){
  int dx,dy,dir=1,error=0,*ptr;
  ptr=(int*)(sf->pixels);
  if(x2<x1){
    dx=x1;dy=y1;
    x1=x2;y1=y2;
    x2=dx;y2=dy;
  }
  dx=x2-x1;
  dy=y2-y1;
  if(dy<0){dy=-dy;dir=-1;}
  if(dx>dy){
    for(;x1!=x2;x1++,error+=dy){
      if(error>dx){error-=dx;y1+=dir;}
      if(x1<sf->w&&x1>=0&&y1>=0&&y1<sf->h)
        ptr[x1+y1*sf->w]=color;
    }
    return;
  }
  for(;y1!=y2;y1+=dir,error+=dx){
    if(error>dy){error-=dy;x1++;}
    if(x1<sf->w&&x1>=0&&y1>=0&&y1<sf->h)
      ptr[x1+y1*sf->w]=color;
  }
}

// Erzeugt eine Schield-Farbe:
void MakeShieldColor(SDL_Surface **dest,SDL_Surface *scr){
  SDL_Rect r;
  int i;
  *dest=SDL_CreateRGBSurface(SDL_HWSURFACE|SDL_SRCCOLORKEY,64,64,32,0,0,0,0);
  SDL_BlitSurface(scr,NULL,*dest,NULL);
  SDL_SetColorKey(*dest,SDL_SRCCOLORKEY,0xFF00FF);
  r.w=64;
  r.h=1;
  r.x=0;
  for(i=1;i<64;i+=2){
    r.y=i;
    SDL_FillRect(*dest,&r,0xFF00FF);
  }
  r.w=1;
  r.h=64;
  r.y=0;
  for(i=1;i<64;i+=2){
    r.x=i;
    SDL_FillRect(*dest,&r,0xFF00FF);
  }
}

int random(int x, int y)
{
	return ((int)(((double)rand()/RAND_MAX)*((x-y)+y)));
}

double Round(double num, unsigned int n) 
{ 
    num *= pow((double) 10, (int) n); 
    if (num >= 0) 
        num = floor(num + 0.5);
    else 
        num = ceil(num - 0.5); 
    num /= pow((double) 10, (int) n); 
    return num; 
}

bool GetXMLBool(TiXmlNode* rootnode,const char *nodename){
	if(strcmp(rootnode->FirstChildElement(nodename)->FirstChild()->ValueStr().c_str(),"true")==0)
		return true;
	else
		return false;
}