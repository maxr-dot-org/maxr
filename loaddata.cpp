///////////////////////////////////////////////////////////////////////////////
//
// M.A.X. Reloaded - loaddata.cpp
//
///////////////////////////////////////////////////////////////////////////////
//
// Loads all relevant files and datas at the start of the game.
// 
//
///////////////////////////////////////////////////////////////////////////////

#include "ExtendedTinyXml.h"
#include "loaddata.h"
#include "files.h"
#include "log.h"
#include "pcx.h"
#include "fonts.h"

// LoadData ///////////////////////////////////////////////////////////////////
// Loads all relevant files and datas:
int LoadData ( void * )
{
	LoadingData=LOAD_GOING;

	string sTmpString;

	// Prepare max.xml for reading
	TiXmlDocument MaxXml;
	ExTiXmlNode * pXmlNode = NULL;
	while(!FileExists("max.xml"))
	{
		cLog::write ( "file not found: max.xml./ngenerating new file", LOG_TYPE_WARNING );
		GenerateMaxXml();
	}
	while(!MaxXml.LoadFile("max.xml"))
	{
		cLog::write ( "cannot load max.xml/ngenerating new file", LOG_TYPE_WARNING );
		GenerateMaxXml();
	}
	// Load fonts for SplashMessages
	cLog::write ( "Loading font for Splash Messages\n", LOG_TYPE_INFO );
	pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Paths","Fonts", "");
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		SettingsData.sFontPath = sTmpString;
	else
	{
		cLog::write ( "Cannot load FontsPath out of max.xml: using default value", LOG_TYPE_INFO );
		SettingsData.sFontPath = "fonts";
	}

	if(!LoadGraphicToSurface(FontsData.font, SettingsData.sFontPath.c_str(), "font.pcx"))
	{
		cLog::write ( "File for game needed! Application Stoped", LOG_TYPE_ERROR );
		LoadingData=LOAD_ERROR;
		return 0;
	}
	if(!LoadGraphicToSurface(FontsData.font_big, SettingsData.sFontPath.c_str(), "font_big.pcx"))
	{
		cLog::write ( "File for game needed! Application Stoped", LOG_TYPE_ERROR );
		LoadingData=LOAD_ERROR;
		return 0;
	}
	if(!LoadGraphicToSurface(FontsData.font_small_white, SettingsData.sFontPath.c_str(), "font_small_white.pcx"))
	{
		cLog::write ( "File for game needed! Application Stoped", LOG_TYPE_ERROR );
		LoadingData=LOAD_ERROR;
		return 0;
	}
	fonts = new cFonts;
	cLog::write ( "Success\n", LOG_TYPE_DEBUG );

	MakeLog(MAXVERSION,false,0);

	// Read the MaxXml-file
	MakeLog("Reading max.xml...",false,2);
	ReadMaxXml(MaxXml);
	MakeLog("Reading max.xml...",true,2);

	// Load Graphics
	MakeLog("Loading Gfx...",false,3);
	if(!LoadGraphics(SettingsData.sGfxPath.c_str()))
	{
		cLog::write ( "Error while loading graphics", LOG_TYPE_ERROR );
		LoadingData=LOAD_ERROR;
		return 0;
	}
	MakeLog("Loading Gfx...",true,3);

	while(1)
	{
		SDL_Delay(10);
	}
	LoadingData=LOAD_FINISHED;
	return 1;
}

// MakeLog ///////////////////////////////////////////////////////////////////
// Writes a Logmessage on the SplashScreen:
void MakeLog ( const char* sztxt,bool ok,int pos )
{
	if ( !ok )
		fonts->OutTextBig ( (char *)sztxt,22,152+16*pos,buffer );
	else
		fonts->OutTextBig ( "OK",250,152+16*pos,buffer );
	SDL_BlitSurface ( buffer,NULL,screen,NULL );
	SDL_UpdateRect ( screen,0,0,0,0 );
	return;
}

// LoadData ///////////////////////////////////////////////////////////////////
// Loads all relevant files and datas:
int LoadGraphicToSurface(SDL_Surface* &dest, const char* directory, const char* filename)
{
	string filepath;
	if(strcmp(directory,""))
	{
		filepath = directory;
		filepath += PATH_DELIMITER;
	}
	filepath += filename;
	if(!FileExists(filepath.c_str()))
	{
		filepath.insert(0,"file not found: ");
		cLog::write ( filepath.c_str(), LOG_TYPE_WARNING );
		return 0;
	}

	dest = LoadPCX((char *)filepath.c_str());

	filepath.insert(0,"file successful loaded: ");
	cLog::write ( filepath.c_str(), LOG_TYPE_DEBUG );

	return 1;
}

// CheckFile ///////////////////////////////////////////////////////////////////
// Checks whether the file exists
int CheckFile(const char* directory, const char* filename)
{
	string filepath;
	if(strcmp(directory,""))
	{
		filepath = directory;
		filepath += PATH_DELIMITER;
	}
	filepath += filename;
	if(!FileExists(filepath.c_str()))
	{
		filepath.insert(0,"file not found: ");
		cLog::write ( filepath.c_str(), LOG_TYPE_WARNING );
		return 0;
	}
	return 1;
}

// ReadMaxXml /////////////////////////////////////////////////////////////////
// Reads the Information out of the max.xml:
void ReadMaxXml(TiXmlDocument MaxXml)
{
	cLog::write ( "Reading max.xml\n", LOG_TYPE_INFO );

	string sTmpString;
	ExTiXmlNode * pXmlNode = NULL;

	pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Start","Resolution", "");
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Num"))
	{
		SettingsData.iScreenW = atoi(sTmpString.substr(0,sTmpString.find(".",0)).c_str());
		SettingsData.iScreenH = atoi(sTmpString.substr(sTmpString.find(".",0)+1,sTmpString.length()).c_str());
	}
	else
	{
		cLog::write ( "Cannot load Resolution out of max.xml: using default value", LOG_TYPE_INFO );
		SettingsData.iScreenW = 640;
		SettingsData.iScreenH = 480;
	}

	pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Start","ColourDepth", "");
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Num"))
		SettingsData.iColourDepth = atoi(sTmpString.c_str());
	else
	{
		cLog::write ( "Cannot load ColourDepth out of max.xml: using default value", LOG_TYPE_INFO );
		SettingsData.iColourDepth = 32;
	}

/*	pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Start","Intro", "");
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"YN"))
		SettingsData.iColourDepth = pXmlNode->XmlDataToBool(sTmpString);
	else
	{
		cLog::write ( "Cannot load Intro out of max.xml: using default value", LOG_TYPE_INFO );
		SettingsData.iColourDepth = false;
	}*/

	pXmlNode = pXmlNode->XmlGetFirstNode(MaxXml,"Options","Game","Paths","GFX", "");
	if(pXmlNode->XmlReadNodeData(sTmpString,ExTiXmlNode::eXML_ATTRIBUTE,"Text"))
		SettingsData.sGfxPath = sTmpString;
	else
	{
		cLog::write ( "Cannot load GfxPath out of max.xml: using default value", LOG_TYPE_INFO );
		SettingsData.sGfxPath = "gfx";
	}

	cLog::write ( "Success\n", LOG_TYPE_DEBUG );
	return;
}

// GenerateMaxXml /////////////////////////////////////////////////////////////
// Generats a new max.xml file
void GenerateMaxXml()
{
	static int iGenerateTrys = 0;
	iGenerateTrys++;
	return;
}

// LoadGFX ///////////////////////////////////////////////////////////////////
// Loads all graphics
int LoadGraphics(const char* path)
{
	cLog::write ( "Loading Graphics\n", LOG_TYPE_INFO );
	string stmp;

	cLog::write ( "loading normal graphics", LOG_TYPE_DEBUG );
	LoadGraphicToSurface ( GraphicsData.gfx_Chand,path,"hand.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_Cno,path,"no.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_Cselect,path,"select.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_Cmove,path,"move.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_Chelp,path,"help.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_Cattack,path,"attack.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_Cpfeil1,path,"pf_1.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_Cpfeil2,path,"pf_2.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_Cpfeil3,path,"pf_3.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_Cpfeil4,path,"pf_4.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_Cpfeil6,path,"pf_6.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_Cpfeil7,path,"pf_7.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_Cpfeil8,path,"pf_8.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_Cpfeil9,path,"pf_9.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_hud_stuff,path,"hud_stuff.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_praefer,path,"praefer.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_help_screen,path,"help_screen.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_object_menu,path,"object_menu.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_destruction,path,"destruction.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_build_screen,path,"build_screen.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_fac_build_screen,path,"fac_build_screen.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_Cband,path,"band_cur.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_band_small,path,"band_small.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_band_big,path,"band_big.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_band_small_org,path,"band_small.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_band_big_org,path,"band_big.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_big_beton_org,path,"big_beton.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_big_beton,path,"big_beton.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_Ctransf,path,"transf.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_transfer,path,"transfer.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_mine_manager,path,"mine_manager.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_Cload,path,"load.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_Cactivate,path,"activate.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_storage,path,"storage.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_storage_ground,path,"storage_ground.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_dialog,path,"dialog.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_edock,path,"edock.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_Cmuni,path,"muni.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_Crepair,path,"repair.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_research,path,"research.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_upgrade,path,"upgrade.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_panel_top,path,"panel_top.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_panel_bottom,path,"panel_bottom.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_Csteal,path,"steal.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_Cdisable,path,"disable.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_menu_stuff,path,"menu_stuff.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_player_pc,path,"player_pc.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_player_human,path,"player_human.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_player_none,path,"player_none.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_load_save_menu,path,"load_save_menu.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_exitpoints_org,path,"activate_field.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_exitpoints,path,"activate_field.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_player_select,path,"customgame_menu.pcx" );
	LoadGraphicToSurface ( GraphicsData.gfx_menu_buttons,path,"menu_buttons.pcx" );

	CheckFile(path,"dialog.pcx");
	CheckFile(path,"dialog2.pcx");
	CheckFile(path,"dialog3.pcx");

	// Hud:
	cLog::write ( "loading hud-graphics", LOG_TYPE_DEBUG );
	SDL_Rect scr,dest;
	GraphicsData.gfx_hud = SDL_CreateRGBSurface ( SDL_HWSURFACE, SettingsData.iScreenW,
		SettingsData.iScreenH, SettingsData.iColourDepth, 0, 0, 0, 0 );
	SDL_FillRect ( GraphicsData.gfx_hud, NULL, 0xFF00FF );
	SDL_SetColorKey ( GraphicsData.gfx_hud, SDL_SRCCOLORKEY, 0xFF00FF );

	LoadGraphicToSurface ( GraphicsData.gfx_tmp, path, "hud_left.pcx" );
	SDL_BlitSurface ( GraphicsData.gfx_tmp, NULL, GraphicsData.gfx_hud, NULL );
	SDL_FreeSurface ( GraphicsData.gfx_tmp );

	LoadGraphicToSurface ( GraphicsData.gfx_tmp,path,"hud_top.pcx" );
	scr.x = 0;
	scr.y=0;
	scr.w = GraphicsData.gfx_tmp->w;
	scr.h = GraphicsData.gfx_tmp->h;
	dest.x = 180;
	dest.y = 0;
	dest.w = GraphicsData.gfx_hud->w-180;
	dest.h = 18;
	SDL_BlitSurface ( GraphicsData.gfx_tmp, &scr, GraphicsData.gfx_hud, &dest );
	scr.x = 1275;
	scr.w = 18;
	scr.h = 18;
	dest.x = GraphicsData.gfx_hud->w-18;
	SDL_BlitSurface ( GraphicsData.gfx_tmp, &scr, GraphicsData.gfx_hud, &dest );
	SDL_FreeSurface ( GraphicsData.gfx_tmp );

	LoadGraphicToSurface ( GraphicsData.gfx_tmp, path, "hud_right.pcx" );
	scr.x = 0;
	scr.y = 0;
	scr.w = GraphicsData.gfx_tmp->w;
	scr.h = GraphicsData.gfx_tmp->h;
	dest.x = GraphicsData.gfx_hud->w-12;
	dest.y = 18;
	dest.w = 12;
	dest.h = GraphicsData.gfx_hud->h;
	SDL_BlitSurface ( GraphicsData.gfx_tmp, &scr, GraphicsData.gfx_hud,&dest );
	SDL_FreeSurface ( GraphicsData.gfx_tmp );

	LoadGraphicToSurface ( GraphicsData.gfx_tmp,path,"hud_bottom.pcx" );
	scr.x = 0;
	scr.y = 0;
	scr.w = GraphicsData.gfx_tmp->w;
	scr.h = GraphicsData.gfx_tmp->h;
	dest.x = 180;
	dest.y = GraphicsData.gfx_hud->h-24;
	dest.w = GraphicsData.gfx_hud->w-180;
	dest.h = 24;
	SDL_BlitSurface ( GraphicsData.gfx_tmp,&scr,GraphicsData.gfx_hud,&dest );
	scr.x = 1275;
	scr.w = 23;
	scr.h = 24;
	dest.x = GraphicsData.gfx_hud->w-23;
	SDL_BlitSurface ( GraphicsData.gfx_tmp,&scr,GraphicsData.gfx_hud,&dest );
	scr.x = 1299;
	scr.w = 16;
	scr.h = 22;
	dest.x = 180-16;
	dest.y = GraphicsData.gfx_hud->h-22;
	SDL_BlitSurface ( GraphicsData.gfx_tmp,&scr,GraphicsData.gfx_hud,&dest );
	SDL_FreeSurface ( GraphicsData.gfx_tmp );

	if ( SettingsData.iScreenH > 480 )
	{
		LoadGraphicToSurface ( GraphicsData.gfx_tmp, path, "logo.pcx" );
		dest.x = 9;
		dest.y = SettingsData.iScreenH-32-15;
		dest.w = 152;
		dest.h = 32;
		SDL_BlitSurface ( GraphicsData.gfx_tmp,NULL,GraphicsData.gfx_hud,&dest );
		SDL_FreeSurface ( GraphicsData.gfx_tmp );
	}

	cLog::write ( "loading color-graphics", LOG_TYPE_DEBUG );
	// Farben:
	OtherData.colors= ( SDL_Surface** ) malloc ( sizeof ( SDL_Surface* ) *8 );
	LoadGraphicToSurface ( OtherData.colors[cl_red],path,"cl_red.pcx" );
	LoadGraphicToSurface ( OtherData.colors[cl_blue],path,"cl_blue.pcx" );
	LoadGraphicToSurface ( OtherData.colors[cl_green],path,"cl_green.pcx" );
	LoadGraphicToSurface ( OtherData.colors[cl_grey],path,"cl_grey.pcx" );
	LoadGraphicToSurface ( OtherData.colors[cl_orange],path,"cl_orange.pcx" );
	LoadGraphicToSurface ( OtherData.colors[cl_yellow],path,"cl_yellow.pcx" );
	LoadGraphicToSurface ( OtherData.colors[cl_purple],path,"cl_purple.pcx" );
	LoadGraphicToSurface ( OtherData.colors[cl_aqua],path,"cl_aqua.pcx" );

	OtherData.ShieldColors= ( SDL_Surface** ) malloc ( sizeof ( SDL_Surface* ) *8 );
	MakeShieldColor ( & ( OtherData.ShieldColors[0] ),OtherData.colors[0] );
	MakeShieldColor ( & ( OtherData.ShieldColors[1] ),OtherData.colors[1] );
	MakeShieldColor ( & ( OtherData.ShieldColors[2] ),OtherData.colors[2] );
	MakeShieldColor ( & ( OtherData.ShieldColors[3] ),OtherData.colors[3] );
	MakeShieldColor ( & ( OtherData.ShieldColors[4] ),OtherData.colors[4] );
	MakeShieldColor ( & ( OtherData.ShieldColors[5] ),OtherData.colors[5] );
	MakeShieldColor ( & ( OtherData.ShieldColors[6] ),OtherData.colors[6] );
	MakeShieldColor ( & ( OtherData.ShieldColors[7] ),OtherData.colors[7] );

	cLog::write ( "generating shadow-graphic", LOG_TYPE_DEBUG );
	// Shadow:
	GraphicsData.gfx_shadow = SDL_CreateRGBSurface ( SDL_HWSURFACE, SettingsData.iScreenW,
		SettingsData.iScreenH, SettingsData.iColourDepth, 0, 0, 0, 0 );
	SDL_FillRect ( GraphicsData.gfx_shadow, NULL, 0x0 );
	SDL_SetAlpha ( GraphicsData.gfx_shadow, SDL_SRCALPHA, 50 );
	GraphicsData.gfx_tmp = SDL_CreateRGBSurface ( SDL_HWSURFACE, 128, 128, SettingsData.iColourDepth, 0, 0, 0, 0 );
	SDL_SetColorKey ( GraphicsData.gfx_tmp, SDL_SRCCOLORKEY, 0xFF00FF );

	// Glas:
	cLog::write ( "loading glass-graphic", LOG_TYPE_DEBUG );
	LoadGraphicToSurface ( GraphicsData.gfx_destruction_glas, path, "destruction_glas.pcx" );
	SDL_SetAlpha ( GraphicsData.gfx_destruction_glas, SDL_SRCALPHA, 150 );

	// Waypoints:
	cLog::write ( "generating waypoint-graphics", LOG_TYPE_DEBUG );
	for ( int i = 0; i < 60; i++ )
	{
		OtherData.WayPointPfeile[0][i] = CreatePfeil ( 26,11,51,36,14,48,PFEIL_COLOR,64-i );
		OtherData.WayPointPfeile[1][i] = CreatePfeil ( 14,14,49,14,31,49,PFEIL_COLOR,64-i );
		OtherData.WayPointPfeile[2][i] = CreatePfeil ( 37,11,12,36,49,48,PFEIL_COLOR,64-i );
		OtherData.WayPointPfeile[3][i] = CreatePfeil ( 49,14,49,49,14,31,PFEIL_COLOR,64-i );
		OtherData.WayPointPfeile[4][i] = CreatePfeil ( 14,14,14,49,49,31,PFEIL_COLOR,64-i );
		OtherData.WayPointPfeile[5][i] = CreatePfeil ( 15,14,52,26,27,51,PFEIL_COLOR,64-i );
		OtherData.WayPointPfeile[6][i] = CreatePfeil ( 31,14,14,49,49,49,PFEIL_COLOR,64-i );
		OtherData.WayPointPfeile[7][i] = CreatePfeil ( 48,14,36,51,11,26,PFEIL_COLOR,64-i );

		OtherData.WayPointPfeileSpecial[0][i] = CreatePfeil ( 26,11,51,36,14,48,PFEILS_COLOR,64-i );
		OtherData.WayPointPfeileSpecial[1][i] = CreatePfeil ( 14,14,49,14,31,49,PFEILS_COLOR,64-i );
		OtherData.WayPointPfeileSpecial[2][i] = CreatePfeil ( 37,11,12,36,49,48,PFEILS_COLOR,64-i );
		OtherData.WayPointPfeileSpecial[3][i] = CreatePfeil ( 49,14,49,49,14,31,PFEILS_COLOR,64-i );
		OtherData.WayPointPfeileSpecial[4][i] = CreatePfeil ( 14,14,14,49,49,31,PFEILS_COLOR,64-i );
		OtherData.WayPointPfeileSpecial[5][i] = CreatePfeil ( 15,14,52,26,27,51,PFEILS_COLOR,64-i );
		OtherData.WayPointPfeileSpecial[6][i] = CreatePfeil ( 31,14,14,49,49,49,PFEILS_COLOR,64-i );
		OtherData.WayPointPfeileSpecial[7][i] = CreatePfeil ( 48,14,36,51,11,26,PFEILS_COLOR,64-i );
	}

	// Resources:
	cLog::write ( "loading resource-graphics", LOG_TYPE_DEBUG );
	LoadGraphicToSurface ( ResourceData.res_metal_org,path,"res.pcx" );
	SDL_SetColorKey ( ResourceData.res_metal_org,SDL_SRCCOLORKEY,-1 );
	ResourceData.res_metal=SDL_CreateRGBSurface ( SDL_HWSURFACE,ResourceData.res_metal_org->w,ResourceData.res_metal_org->h,SettingsData.iColourDepth,0,0,0,0 );
	SDL_BlitSurface ( ResourceData.res_metal_org,NULL,ResourceData.res_metal,NULL );
	SDL_SetColorKey ( ResourceData.res_metal,SDL_SRCCOLORKEY,0xFFFFFF );

	ResourceData.res_oil_org=SDL_CreateRGBSurface ( SDL_HWSURFACE,ResourceData.res_metal_org->w,ResourceData.res_metal_org->h,SettingsData.iColourDepth,0,0,0,0 );
	SDL_FillRect ( ResourceData.res_oil_org,NULL,0x00FF00 );
	SDL_SetColorKey ( ResourceData.res_oil_org,SDL_SRCCOLORKEY,0xFF00FF );
	SDL_BlitSurface ( ResourceData.res_metal,NULL,ResourceData.res_oil_org,NULL );
	ResourceData.res_oil=SDL_CreateRGBSurface ( SDL_HWSURFACE,ResourceData.res_metal_org->w,ResourceData.res_metal_org->h,SettingsData.iColourDepth,0,0,0,0 );
	SDL_FillRect ( ResourceData.res_oil,NULL,0x00FF00 );
	SDL_SetColorKey ( ResourceData.res_oil,SDL_SRCCOLORKEY,0xFF00FF );
	SDL_BlitSurface ( ResourceData.res_metal,NULL,ResourceData.res_oil,NULL );

	ResourceData.res_gold_org=SDL_CreateRGBSurface ( SDL_HWSURFACE,ResourceData.res_metal_org->w,ResourceData.res_metal_org->h,SettingsData.iColourDepth,0,0,0,0 );
	SDL_FillRect ( ResourceData.res_gold_org,NULL,0xFFFF00 );
	SDL_SetColorKey ( ResourceData.res_gold_org,SDL_SRCCOLORKEY,0xFF00FF );
	SDL_BlitSurface ( ResourceData.res_metal,NULL,ResourceData.res_gold_org,NULL );
	ResourceData.res_gold=SDL_CreateRGBSurface ( SDL_HWSURFACE,ResourceData.res_metal_org->w,ResourceData.res_metal_org->h,SettingsData.iColourDepth,0,0,0,0 );
	SDL_FillRect ( ResourceData.res_gold,NULL,0xFFFF00 );
	SDL_SetColorKey ( ResourceData.res_gold,SDL_SRCCOLORKEY,0xFF00FF );
	SDL_BlitSurface ( ResourceData.res_metal,NULL,ResourceData.res_gold,NULL );

	SDL_SetColorKey ( ResourceData.res_metal,SDL_SRCCOLORKEY,0xFF00FF );

	cLog::write ( "Success\n", LOG_TYPE_DEBUG );
	return 1;
}