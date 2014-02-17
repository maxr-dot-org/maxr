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

#include "keys.h"

#include "extendedtinyxml.h"
#include "files.h"
#include "log.h"
#include "settings.h"
#include "tinyxml2.h"

using namespace tinyxml2;

static void LoadSingleKey (XMLDocument& KeysXml, const char* keyString,
						   SDLKey& key, const char* defaultKeyString)
{
	const XMLElement* xmlElement = XmlGetFirstElement (KeysXml, "Controles", "Keys", keyString, NULL);
	const std::string value = xmlElement ? xmlElement->Attribute ("Text") : "";

	if (!value.empty())
		key = GetKeyFromString (value);
	else
	{
		std::string msg = "keys: Can't load ";
		msg += keyString;
		msg += " from keys.xml: using default value";

		Log.write (msg, LOG_TYPE_WARNING);
		key = GetKeyFromString (defaultKeyString);
	}
}

int LoadKeys()
{
	Log.write ("Loading Keys", LOG_TYPE_INFO);
	if (!FileExists (KEYS_XMLUsers) && !FileExists (KEYS_XMLGame))
	{
		Log.write ("generating new keys-file", LOG_TYPE_WARNING);
		GenerateKeysXml();
	}
	else if (!FileExists (KEYS_XMLUsers))
	{
		copyFile (KEYS_XMLGame, KEYS_XMLUsers);
		Log.write ("Key-file copied from gamedir to userdir", LOG_TYPE_INFO);
	}
	else // => (FileExists (KEYS_XMLUsers))
	{
		Log.write ("User key-file in use", LOG_TYPE_INFO);
	}

	XMLDocument KeysXml;
	if (KeysXml.LoadFile (KEYS_XMLUsers) != XML_NO_ERROR)
	{
		Log.write ("cannot load keys.xml\ngenerating new file", LOG_TYPE_WARNING);
		GenerateKeysXml();
	}

	LoadSingleKey (KeysXml, "KeyExit", KeysList.KeyExit, "ESCAPE");
	LoadSingleKey (KeysXml, "KeyJumpToAction", KeysList.KeyJumpToAction, "F1");
	LoadSingleKey (KeysXml, "KeyEndTurn", KeysList.KeyEndTurn, "RETURN");
	LoadSingleKey (KeysXml, "KeyChat", KeysList.KeyChat, "TAB");
	LoadSingleKey (KeysXml, "KeyScroll8a", KeysList.KeyScroll8a, "UP");
	LoadSingleKey (KeysXml, "KeyScroll8b", KeysList.KeyScroll8b, "KP8");
	LoadSingleKey (KeysXml, "KeyScroll2a", KeysList.KeyScroll2a, "DOWN");
	LoadSingleKey (KeysXml, "KeyScroll2b", KeysList.KeyScroll2b, "KP2");
	LoadSingleKey (KeysXml, "KeyScroll6a", KeysList.KeyScroll6a, "RIGHT");
	LoadSingleKey (KeysXml, "KeyScroll6b", KeysList.KeyScroll6b, "KP6");
	LoadSingleKey (KeysXml, "KeyScroll4a", KeysList.KeyScroll4a, "LEFT");
	LoadSingleKey (KeysXml, "KeyScroll4b", KeysList.KeyScroll4b, "KP4");
	LoadSingleKey (KeysXml, "KeyScroll7", KeysList.KeyScroll7, "KP7");
	LoadSingleKey (KeysXml, "KeyScroll9", KeysList.KeyScroll9, "KP9");
	LoadSingleKey (KeysXml, "KeyScroll1", KeysList.KeyScroll1, "KP1");
	LoadSingleKey (KeysXml, "KeyScroll3", KeysList.KeyScroll3, "KP3");
	LoadSingleKey (KeysXml, "KeyZoomIna", KeysList.KeyZoomIna, "RIGHTBRACKET");
	LoadSingleKey (KeysXml, "KeyZoomInb", KeysList.KeyZoomInb, "KP_PLUS");
	LoadSingleKey (KeysXml, "KeyZoomOuta", KeysList.KeyZoomOuta, "SLASH");
	LoadSingleKey (KeysXml, "KeyZoomOutb", KeysList.KeyZoomOutb, "KP_MINUS");
	LoadSingleKey (KeysXml, "KeyFog", KeysList.KeyFog, "N");
	LoadSingleKey (KeysXml, "KeyGrid", KeysList.KeyGrid, "G");
	LoadSingleKey (KeysXml, "KeyScan", KeysList.KeyScan, "S");
	LoadSingleKey (KeysXml, "KeyRange", KeysList.KeyRange, "R");
	LoadSingleKey (KeysXml, "KeyAmmo", KeysList.KeyAmmo, "M");
	LoadSingleKey (KeysXml, "KeyHitpoints", KeysList.KeyHitpoints, "T");
	LoadSingleKey (KeysXml, "KeyColors", KeysList.KeyColors, "F");
	LoadSingleKey (KeysXml, "KeyStatus", KeysList.KeyStatus, "P");
	LoadSingleKey (KeysXml, "KeySurvey", KeysList.KeySurvey, "H");
	LoadSingleKey (KeysXml, "KeyCalcPath", KeysList.KeyCalcPath, "LSHIFT");
	LoadSingleKey (KeysXml, "KeyCenterUnit", KeysList.KeyCenterUnit, "F");
	LoadSingleKey (KeysXml, "KeyUnitDone", KeysList.KeyUnitDone, "E");
	LoadSingleKey (KeysXml, "KeyUnitDoneAndNext", KeysList.KeyUnitDoneAndNext, "SPACE");
	LoadSingleKey (KeysXml, "KeyUnitNext", KeysList.KeyUnitNext, "W");
	LoadSingleKey (KeysXml, "KeyUnitPrev", KeysList.KeyUnitPrev, "Q");
	LoadSingleKey (KeysXml, "KeyUnitMenuAttack", KeysList.KeyUnitMenuAttack, "A");
	LoadSingleKey (KeysXml, "KeyUnitMenuBuild", KeysList.KeyUnitMenuBuild, "B");
	LoadSingleKey (KeysXml, "KeyUnitMenuTransfer", KeysList.KeyUnitMenuTransfer, "X");
	LoadSingleKey (KeysXml, "KeyUnitMenuAutomove", KeysList.KeyUnitMenuAutomove, "A");
	LoadSingleKey (KeysXml, "KeyUnitMenuStart", KeysList.KeyUnitMenuStart, "S");
	LoadSingleKey (KeysXml, "KeyUnitMenuStop", KeysList.KeyUnitMenuStop, "S");
	LoadSingleKey (KeysXml, "KeyUnitMenuClear", KeysList.KeyUnitMenuClear, "C");
	LoadSingleKey (KeysXml, "KeyUnitMenuSentry", KeysList.KeyUnitMenuSentry, "S");
	LoadSingleKey (KeysXml, "KeyUnitMenuManualFire", KeysList.KeyUnitMenuManualFire, "M");
	LoadSingleKey (KeysXml, "KeyUnitMenuActivate", KeysList.KeyUnitMenuActivate, "A");
	LoadSingleKey (KeysXml, "KeyUnitMenuLoad", KeysList.KeyUnitMenuLoad, "L");
	LoadSingleKey (KeysXml, "KeyUnitMenuReload", KeysList.KeyUnitMenuReload, "R");
	LoadSingleKey (KeysXml, "KeyUnitMenuRepair", KeysList.KeyUnitMenuRepair, "R");
	LoadSingleKey (KeysXml, "KeyUnitMenuLayMine", KeysList.KeyUnitMenuLayMine, "L");
	LoadSingleKey (KeysXml, "KeyUnitMenuClearMine", KeysList.KeyUnitMenuClearMine, "C");
	LoadSingleKey (KeysXml, "KeyUnitMenuDisable", KeysList.KeyUnitMenuDisable, "D");
	LoadSingleKey (KeysXml, "KeyUnitMenuSteal", KeysList.KeyUnitMenuSteal, "S");
	LoadSingleKey (KeysXml, "KeyUnitMenuInfo", KeysList.KeyUnitMenuInfo, "H");
	LoadSingleKey (KeysXml, "KeyUnitMenuDistribute", KeysList.KeyUnitMenuDistribute, "D");
	LoadSingleKey (KeysXml, "KeyUnitMenuResearch", KeysList.KeyUnitMenuResearch, "R");
	LoadSingleKey (KeysXml, "KeyUnitMenuUpgrade", KeysList.KeyUnitMenuUpgrade, "U");
	LoadSingleKey (KeysXml, "KeyUnitMenuDestroy", KeysList.KeyUnitMenuDestroy, "D");

	XMLElement* element = XmlGetFirstElement (KeysXml, "Controles", "Mouse", "MOUSE_STYLE", NULL);
	if (element->Attribute ("Text", "OLD_SCHOOL"))
		MouseStyle = OldSchool;
	else
		MouseStyle = Modern;

	Log.write ("Done", LOG_TYPE_DEBUG);
	return 1;
}

#if 0 // TODO: [SDL2]: obsolete keys... use scancode ?
SDLK_KP0; SDLK_KP1; SDLK_KP2; SDLK_KP3; SDLK_KP4;
SDLK_KP5; SDLK_KP6; SDLK_KP7; SDLK_KP8; SDLK_KP9;
SDLK_NUMLOCK; SDLK_SCROLLOCK;
SDLK_RMETA; SDLK_LMETA;
SDLK_LSUPER; SDLK_RSUPER;
SDLK_COMPOSE; SDLK_PRINT; SDLK_BREAK; SDLK_EURO;
#endif


// Liefert einen String mit dem Namen der Taste zurueck:
const char* GetKeyString (SDLKey key)
{
	switch (key)
	{
		case SDLK_UNKNOWN: return "UNKNOWN";
		case SDLK_BACKSPACE: return "BACKSPACE";
		case SDLK_TAB: return "TAB";
		case SDLK_CLEAR: return "CLEAR";
		case SDLK_RETURN: return "RETURN";
		case SDLK_PAUSE: return "PAUSE";
		case SDLK_ESCAPE: return "ESCAPE";
		case SDLK_SPACE: return "SPACE";
		case SDLK_EXCLAIM: return "EXCLAIM";
		case SDLK_QUOTEDBL: return "QUOTEDBL";
		case SDLK_HASH: return "HASH";
		case SDLK_DOLLAR: return "DOLLAR";
		case SDLK_AMPERSAND: return "AMPERSAND";
		case SDLK_QUOTE: return "QUOTE";
		case SDLK_LEFTPAREN: return "LEFTPAREN";
		case SDLK_RIGHTPAREN: return "RIGHTPAREN";
		case SDLK_ASTERISK: return "ASTERISK";
		case SDLK_PLUS: return "PLUS";
		case SDLK_COMMA: return "COMMA";
		case SDLK_MINUS: return "MINUS";
		case SDLK_PERIOD: return "PERIOD";
		case SDLK_SLASH: return "SLASH";
		case SDLK_0: return "0";
		case SDLK_1: return "1";
		case SDLK_2: return "2";
		case SDLK_3: return "3";
		case SDLK_4: return "4";
		case SDLK_5: return "5";
		case SDLK_6: return "6";
		case SDLK_7: return "7";
		case SDLK_8: return "8";
		case SDLK_9: return "9";
		case SDLK_COLON: return "COLON";
		case SDLK_SEMICOLON: return "SEMICOLON";
		case SDLK_LESS: return "LESS";
		case SDLK_EQUALS: return "EQUALS";
		case SDLK_GREATER: return "GREATER";
		case SDLK_QUESTION: return "QUESTION";
		case SDLK_AT: return "AT";
		case SDLK_LEFTBRACKET: return "LEFTBRACKET";
		case SDLK_BACKSLASH: return "BACKSLASH";
		case SDLK_RIGHTBRACKET: return "RIGHTBRACKET";
		case SDLK_CARET: return "CARET";
		case SDLK_UNDERSCORE: return "UNDERSCORE";
		case SDLK_BACKQUOTE: return "BACKQUOTE";
		case SDLK_a: return "A";
		case SDLK_b: return "B";
		case SDLK_c: return "C";
		case SDLK_d: return "D";
		case SDLK_e: return "E";
		case SDLK_f: return "F";
		case SDLK_g: return "G";
		case SDLK_h: return "H";
		case SDLK_i: return "I";
		case SDLK_j: return "J";
		case SDLK_k: return "K";
		case SDLK_l: return "L";
		case SDLK_m: return "M";
		case SDLK_n: return "N";
		case SDLK_o: return "O";
		case SDLK_p: return "P";
		case SDLK_q: return "Q";
		case SDLK_r: return "R";
		case SDLK_s: return "S";
		case SDLK_t: return "T";
		case SDLK_u: return "U";
		case SDLK_v: return "V";
		case SDLK_w: return "W";
		case SDLK_x: return "X";
		case SDLK_y: return "Y";
		case SDLK_z: return "Z";
		case SDLK_DELETE: return "DELETE";
		case SDLK_KP_PERIOD: return "KP_PERIOD";
		case SDLK_KP_DIVIDE: return "KP_DIVIDE";
		case SDLK_KP_MULTIPLY: return "KP_MULTIPLY";
		case SDLK_KP_MINUS: return "KP_MINUS";
		case SDLK_KP_PLUS: return "KP_PLUS";
		case SDLK_KP_ENTER: return "KP_ENTER";
		case SDLK_KP_EQUALS: return "KP_EQUALS";
		case SDLK_UP: return "UP";
		case SDLK_DOWN: return "DOWN";
		case SDLK_RIGHT: return "RIGHT";
		case SDLK_LEFT: return "LEFT";
		case SDLK_INSERT: return "INSERT";
		case SDLK_HOME: return "HOME";
		case SDLK_END: return "END";
		case SDLK_PAGEUP: return "PAGEUP";
		case SDLK_PAGEDOWN: return "PAGEDOWN";
		case SDLK_F1: return "F1";
		case SDLK_F2: return "F2";
		case SDLK_F3: return "F3";
		case SDLK_F4: return "F4";
		case SDLK_F5: return "F5";
		case SDLK_F6: return "F6";
		case SDLK_F7: return "F7";
		case SDLK_F8: return "F8";
		case SDLK_F9: return "F9";
		case SDLK_F10: return "F10";
		case SDLK_F11: return "F11";
		case SDLK_F12: return "F12";
		case SDLK_F13: return "F13";
		case SDLK_F14: return "F14";
		case SDLK_F15: return "F15";
		case SDLK_CAPSLOCK: return "CAPSLOCK";
		case SDLK_RSHIFT: return "RSHIFT";
		case SDLK_LSHIFT: return "LSHIFT";
		case SDLK_RCTRL: return "RCTRL";
		case SDLK_LCTRL: return "LCTRL";
		case SDLK_RALT: return "RALT";
		case SDLK_LALT: return "LALT";
		case SDLK_MODE: return "MODE";
		case SDLK_HELP: return "HELP";
		case SDLK_SYSREQ: return "SYSREQ";
		case SDLK_MENU: return "MENU";
		case SDLK_POWER: return "POWER";
		case SDLK_UNDO: return "UNDO";
		default: return "?";
	}
}

// Liefert den Code der Taste zurueck:
SDLKey GetKeyFromString (const std::string& key)
{
	if (!key.compare ("UNKNOWN")) return SDLK_UNKNOWN;
	if (!key.compare ("BACKSPACE")) return SDLK_BACKSPACE;
	if (!key.compare ("TAB")) return SDLK_TAB;
	if (!key.compare ("CLEAR")) return SDLK_CLEAR;
	if (!key.compare ("RETURN")) return SDLK_RETURN;
	if (!key.compare ("PAUSE")) return SDLK_PAUSE;
	if (!key.compare ("ESCAPE")) return SDLK_ESCAPE;
	if (!key.compare ("SPACE")) return SDLK_SPACE;
	if (!key.compare ("EXCLAIM")) return SDLK_EXCLAIM;
	if (!key.compare ("QUOTEDBL")) return SDLK_QUOTEDBL;
	if (!key.compare ("HASH")) return SDLK_HASH;
	if (!key.compare ("DOLLAR")) return SDLK_DOLLAR;
	if (!key.compare ("AMPERSAND")) return SDLK_AMPERSAND;
	if (!key.compare ("QUOTE")) return SDLK_QUOTE;
	if (!key.compare ("LEFTPAREN")) return SDLK_LEFTPAREN;
	if (!key.compare ("RIGHTPAREN")) return SDLK_RIGHTPAREN;
	if (!key.compare ("ASTERISK")) return SDLK_ASTERISK;
	if (!key.compare ("PLUS")) return SDLK_PLUS;
	if (!key.compare ("COMMA")) return SDLK_COMMA;
	if (!key.compare ("MINUS")) return SDLK_MINUS;
	if (!key.compare ("PERIOD")) return SDLK_PERIOD;
	if (!key.compare ("SLASH")) return SDLK_SLASH;
	if (!key.compare ("0")) return SDLK_0;
	if (!key.compare ("1")) return SDLK_1;
	if (!key.compare ("2")) return SDLK_2;
	if (!key.compare ("3")) return SDLK_3;
	if (!key.compare ("4")) return SDLK_4;
	if (!key.compare ("5")) return SDLK_5;
	if (!key.compare ("6")) return SDLK_6;
	if (!key.compare ("7")) return SDLK_7;
	if (!key.compare ("8")) return SDLK_8;
	if (!key.compare ("9")) return SDLK_9;
	if (!key.compare ("COLON")) return SDLK_COLON;
	if (!key.compare ("SEMICOLON")) return SDLK_SEMICOLON;
	if (!key.compare ("LESS")) return SDLK_LESS;
	if (!key.compare ("EQUALS")) return SDLK_EQUALS;
	if (!key.compare ("GREATER")) return SDLK_GREATER;
	if (!key.compare ("QUESTION")) return SDLK_QUESTION;
	if (!key.compare ("AT")) return SDLK_AT;
	if (!key.compare ("LEFTBRACKET")) return SDLK_LEFTBRACKET;
	if (!key.compare ("BACKSLASH")) return SDLK_BACKSLASH;
	if (!key.compare ("RIGHTBRACKET")) return SDLK_RIGHTBRACKET;
	if (!key.compare ("CARET")) return SDLK_CARET;
	if (!key.compare ("UNDERSCORE")) return SDLK_UNDERSCORE;
	if (!key.compare ("BACKQUOTE")) return SDLK_BACKQUOTE;
	if (!key.compare ("A")) return SDLK_a;
	if (!key.compare ("B")) return SDLK_b;
	if (!key.compare ("C")) return SDLK_c;
	if (!key.compare ("D")) return SDLK_d;
	if (!key.compare ("E")) return SDLK_e;
	if (!key.compare ("F")) return SDLK_f;
	if (!key.compare ("G")) return SDLK_g;
	if (!key.compare ("H")) return SDLK_h;
	if (!key.compare ("I")) return SDLK_i;
	if (!key.compare ("J")) return SDLK_j;
	if (!key.compare ("K")) return SDLK_k;
	if (!key.compare ("L")) return SDLK_l;
	if (!key.compare ("M")) return SDLK_m;
	if (!key.compare ("N")) return SDLK_n;
	if (!key.compare ("O")) return SDLK_o;
	if (!key.compare ("P")) return SDLK_p;
	if (!key.compare ("Q")) return SDLK_q;
	if (!key.compare ("R")) return SDLK_r;
	if (!key.compare ("S")) return SDLK_s;
	if (!key.compare ("T")) return SDLK_t;
	if (!key.compare ("U")) return SDLK_u;
	if (!key.compare ("V")) return SDLK_v;
	if (!key.compare ("W")) return SDLK_w;
	if (!key.compare ("X")) return SDLK_x;
	if (!key.compare ("Y")) return SDLK_y;
	if (!key.compare ("Z")) return SDLK_z;
	if (!key.compare ("DELETE")) return SDLK_DELETE;
	if (!key.compare ("KP_PERIOD")) return SDLK_KP_PERIOD;
	if (!key.compare ("KP_DIVIDE")) return SDLK_KP_DIVIDE;
	if (!key.compare ("KP_MULTIPLY")) return SDLK_KP_MULTIPLY;
	if (!key.compare ("KP_MINUS")) return SDLK_KP_MINUS;
	if (!key.compare ("KP_PLUS")) return SDLK_KP_PLUS;
	if (!key.compare ("KP_ENTER")) return SDLK_KP_ENTER;
	if (!key.compare ("KP_EQUALS")) return SDLK_KP_EQUALS;
	if (!key.compare ("UP")) return SDLK_UP;
	if (!key.compare ("DOWN")) return SDLK_DOWN;
	if (!key.compare ("RIGHT")) return SDLK_RIGHT;
	if (!key.compare ("LEFT")) return SDLK_LEFT;
	if (!key.compare ("INSERT")) return SDLK_INSERT;
	if (!key.compare ("HOME")) return SDLK_HOME;
	if (!key.compare ("END")) return SDLK_END;
	if (!key.compare ("PAGEUP")) return SDLK_PAGEUP;
	if (!key.compare ("PAGEDOWN")) return SDLK_PAGEDOWN;
	if (!key.compare ("F1")) return SDLK_F1;
	if (!key.compare ("F2")) return SDLK_F2;
	if (!key.compare ("F3")) return SDLK_F3;
	if (!key.compare ("F4")) return SDLK_F4;
	if (!key.compare ("F5")) return SDLK_F5;
	if (!key.compare ("F6")) return SDLK_F6;
	if (!key.compare ("F7")) return SDLK_F7;
	if (!key.compare ("F8")) return SDLK_F8;
	if (!key.compare ("F9")) return SDLK_F9;
	if (!key.compare ("F10")) return SDLK_F10;
	if (!key.compare ("F11")) return SDLK_F11;
	if (!key.compare ("F12")) return SDLK_F12;
	if (!key.compare ("F13")) return SDLK_F13;
	if (!key.compare ("F14")) return SDLK_F14;
	if (!key.compare ("F15")) return SDLK_F15;
	if (!key.compare ("CAPSLOCK")) return SDLK_CAPSLOCK;
	if (!key.compare ("RSHIFT")) return SDLK_RSHIFT;
	if (!key.compare ("LSHIFT")) return SDLK_LSHIFT;
	if (!key.compare ("RCTRL")) return SDLK_RCTRL;
	if (!key.compare ("LCTRL")) return SDLK_LCTRL;
	if (!key.compare ("RALT")) return SDLK_RALT;
	if (!key.compare ("LALT")) return SDLK_LALT;
	if (!key.compare ("MODE")) return SDLK_MODE;
	if (!key.compare ("HELP")) return SDLK_HELP;
	if (!key.compare ("SYSREQ")) return SDLK_SYSREQ;
	if (!key.compare ("MENU")) return SDLK_MENU;
	if (!key.compare ("POWER")) return SDLK_POWER;
	if (!key.compare ("UNDO")) return SDLK_UNDO;
	return SDLK_UNKNOWN;
}

void GenerateKeysXml()
{
	//TODO: add generation of key xml
	Log.write ("GenerateKeysXML not yet implemented", cLog::eLOG_TYPE_WARNING);
}
