//////////////////////////////////////////////////////////////////////////////
// M.A.X. - keys.h
//////////////////////////////////////////////////////////////////////////////
#ifndef keysH
#define keysH
#include "defines.h"
#include "SDL.h"
#include "main.h"

// Globale Daten /////////////////////////////////////////////////////////////
EX SDLKey KeyExit;
EX SDLKey KeyJumpToAction;
EX SDLKey KeyEndTurn;
EX SDLKey KeyChat;
EX SDLKey KeyScroll8a;
EX SDLKey KeyScroll8b;
EX SDLKey KeyScroll2a;
EX SDLKey KeyScroll2b;
EX SDLKey KeyScroll6a;
EX SDLKey KeyScroll6b;
EX SDLKey KeyScroll4a;
EX SDLKey KeyScroll4b;
EX SDLKey KeyScroll7;
EX SDLKey KeyScroll9;
EX SDLKey KeyScroll1;
EX SDLKey KeyScroll3;
EX SDLKey KeyZoomIna;
EX SDLKey KeyZoomInb;
EX SDLKey KeyZoomOuta;
EX SDLKey KeyZoomOutb;
EX SDLKey KeyFog;
EX SDLKey KeyGrid;
EX SDLKey KeyScan;
EX SDLKey KeyRange;
EX SDLKey KeyAmmo;
EX SDLKey KeyHitpoints;
EX SDLKey KeyColors;
EX SDLKey KeyStatus;
EX SDLKey KeySurvey;
EX SDLKey KeyCalcPath;

typedef enum {OldSchool,Modern}eMouseStyle;
EX eMouseStyle MouseStyle;

// Prototypen ////////////////////////////////////////////////////////////////
int LoadKeys(string file);
char *GetKeyString(SDLKey key);
SDLKey GetKeyFromString(string key);

#endif
