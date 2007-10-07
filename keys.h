//////////////////////////////////////////////////////////////////////////////
// M.A.X. - keys.h
//////////////////////////////////////////////////////////////////////////////
#ifndef keysH
#define keysH
#include "defines.h"
#include "SDL.h"
#include "main.h"

// Globale Daten /////////////////////////////////////////////////////////////
class cKeysList
{
public:
	SDLKey KeyExit;
	SDLKey KeyJumpToAction;
	SDLKey KeyEndTurn;
	SDLKey KeyChat;
	SDLKey KeyScroll8a;
	SDLKey KeyScroll8b;
	SDLKey KeyScroll2a;
	SDLKey KeyScroll2b;
	SDLKey KeyScroll6a;
	SDLKey KeyScroll6b;
	SDLKey KeyScroll4a;
	SDLKey KeyScroll4b;
	SDLKey KeyScroll7;
	SDLKey KeyScroll9;
	SDLKey KeyScroll1;
	SDLKey KeyScroll3;
	SDLKey KeyZoomIna;
	SDLKey KeyZoomInb;
	SDLKey KeyZoomOuta;
	SDLKey KeyZoomOutb;
	SDLKey KeyFog;
	SDLKey KeyGrid;
	SDLKey KeyScan;
	SDLKey KeyRange;
	SDLKey KeyAmmo;
	SDLKey KeyHitpoints;
	SDLKey KeyColors;
	SDLKey KeyStatus;
	SDLKey KeySurvey;
	SDLKey KeyCalcPath;
} EX KeysList;

typedef enum {OldSchool,Modern}eMouseStyle;
EX eMouseStyle MouseStyle;

// Prototypen ////////////////////////////////////////////////////////////////
int LoadKeys();
char *GetKeyString(SDLKey key);
SDLKey GetKeyFromString(string key);
/**
	* Generats a new keys.xml file
	*/
void GenerateKeysXml();

#endif
