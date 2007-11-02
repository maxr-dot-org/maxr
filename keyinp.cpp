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
#include "keyinp.h"
#include "fonts.h"
#include "main.h"

// DoKeyInp //////////////////////////////////////////////////////////////////
// Liest die gedrückten Tasten ein:
bool DoKeyInp ( Uint8 *keystate )
{
	static int lc = -1, DelayTime = 500;
	int c = -1;

	InputEnter=false;
#define SHIFT &&(keystate[SDLK_RSHIFT]||keystate[SDLK_LSHIFT])
#define ALT &&(keystate[SDLK_RALT]||keystate[SDLK_LALT])
	if ( keystate[SDLK_BACKSPACE] ) c=-2;
	else if ( keystate[SDLK_RETURN] ) c=-3;
	else if ( keystate[SDLK_KP_ENTER] ) c=-3;
	else if ( keystate[SDLK_SPACE] ) c=' ';

	else if ( keystate[SDLK_z]SHIFT ) c='Y';
	else if ( keystate[SDLK_z] ) c='y';
	else if ( keystate[SDLK_y]SHIFT ) c='Z';
	else if ( keystate[SDLK_y] ) c='z';
	else if ( keystate[SDLK_a]SHIFT ) c='A';
	else if ( keystate[SDLK_a] ) c='a';

	else if ( keystate[SDLK_b]SHIFT ) c='B';
	else if ( keystate[SDLK_b] ) c='b';
	else if ( keystate[SDLK_c]SHIFT ) c='C';
	else if ( keystate[SDLK_c] ) c='c';
	else if ( keystate[SDLK_d]SHIFT ) c='D';
	else if ( keystate[SDLK_d] ) c='d';
	else if ( keystate[SDLK_e]SHIFT ) c='E';
	else if ( keystate[SDLK_e] ) c='e';
	else if ( keystate[SDLK_f]SHIFT ) c='F';
	else if ( keystate[SDLK_f] ) c='f';
	else if ( keystate[SDLK_g]SHIFT ) c='G';
	else if ( keystate[SDLK_g] ) c='g';
	else if ( keystate[SDLK_h]SHIFT ) c='H';
	else if ( keystate[SDLK_h] ) c='h';
	else if ( keystate[SDLK_i]SHIFT ) c='I';
	else if ( keystate[SDLK_i] ) c='i';
	else if ( keystate[SDLK_j]SHIFT ) c='J';
	else if ( keystate[SDLK_j] ) c='j';
	else if ( keystate[SDLK_k]SHIFT ) c='K';
	else if ( keystate[SDLK_k] ) c='k';
	else if ( keystate[SDLK_l]SHIFT ) c='L';
	else if ( keystate[SDLK_l] ) c='l';
	else if ( keystate[SDLK_m]SHIFT ) c='M';
	else if ( keystate[SDLK_m] ) c='m';
	else if ( keystate[SDLK_n]SHIFT ) c='N';
	else if ( keystate[SDLK_n] ) c='n';
	else if ( keystate[SDLK_o]SHIFT ) c='O';
	else if ( keystate[SDLK_o] ) c='o';
	else if ( keystate[SDLK_p]SHIFT ) c='P';
	else if ( keystate[SDLK_p] ) c='p';
	else if ( keystate[SDLK_q]SHIFT ) c='Q';
	else if ( keystate[SDLK_q] ) c='q';
	else if ( keystate[SDLK_r]SHIFT ) c='R';
	else if ( keystate[SDLK_r] ) c='r';
	else if ( keystate[SDLK_s]SHIFT ) c='S';
	else if ( keystate[SDLK_s] ) c='s';
	else if ( keystate[SDLK_t]SHIFT ) c='T';
	else if ( keystate[SDLK_t] ) c='t';
	else if ( keystate[SDLK_u]SHIFT ) c='U';
	else if ( keystate[SDLK_u] ) c='u';
	else if ( keystate[SDLK_v]SHIFT ) c='V';
	else if ( keystate[SDLK_v] ) c='v';
	else if ( keystate[SDLK_w]SHIFT ) c='W';
	else if ( keystate[SDLK_w] ) c='w';
	else if ( keystate[SDLK_x]SHIFT ) c='X';
	else if ( keystate[SDLK_x] ) c='x';

	else if ( keystate[39]SHIFT ALT ) c='\'';
	else if ( keystate[39]ALT ) c='#';
	else if ( keystate[96]SHIFT ) c='°';
	else if ( keystate[96] ) c='^';
	else if ( keystate[59]SHIFT ) c='Ö';
	else if ( keystate[59] ) c='ö';
	else if ( keystate[39]SHIFT ) c='Ä';
	else if ( keystate[39] ) c='ä';
	else if ( keystate[91]SHIFT ) c='Ü';
	else if ( keystate[91] ) c='ü';
	else if ( ( keystate[93]||keystate[SDLK_KP_PLUS] ) SHIFT ) c='*';
	else if ( ( keystate[93]||keystate[SDLK_KP_PLUS] ) ALT ) c='~';
	else if ( keystate[93]||keystate[SDLK_KP_PLUS] ) c='+';
	else if ( ( keystate[47]||keystate[SDLK_KP_MINUS] ) SHIFT ) c='_';
	else if ( keystate[47]||keystate[SDLK_KP_MINUS] ) c='-';
	else if ( keystate[SDLK_COMMA]SHIFT ) c=';';
	else if ( keystate[SDLK_COMMA] ) c=',';
	else if ( keystate[SDLK_PERIOD]SHIFT ) c=':';
	else if ( keystate[SDLK_PERIOD] ) c='.';
	else if ( keystate[SDLK_0]SHIFT ) c='=';
	else if ( keystate[SDLK_0] ) c='0';
	else if ( keystate[SDLK_1]SHIFT ) c='!';
	else if ( keystate[SDLK_1] ) c='1';
	else if ( keystate[SDLK_2]ALT ) c='²';
	else if ( keystate[SDLK_2]SHIFT ) c='"';
	else if ( keystate[SDLK_2] ) c='2';
	else if ( keystate[SDLK_3]ALT ) c='³';
	else if ( keystate[SDLK_3]SHIFT ) c='§';
	else if ( keystate[SDLK_3] ) c='3';
	else if ( keystate[SDLK_4]SHIFT ) c='$';
	else if ( keystate[SDLK_4] ) c='4';
	else if ( keystate[SDLK_5]SHIFT ) c='%';
	else if ( keystate[SDLK_5] ) c='5';
	else if ( keystate[SDLK_6]SHIFT ) c='&';
	else if ( keystate[SDLK_6] ) c='6';
	else if ( keystate[SDLK_7]SHIFT ) c='/';
	else if ( keystate[SDLK_7] ) c='7';
	else if ( keystate[SDLK_8]SHIFT ) c='(';
	else if ( keystate[SDLK_8]ALT ) c='[';
	else if ( keystate[SDLK_8] ) c='8';
	else if ( keystate[SDLK_9]SHIFT ) c=')';
	else if ( keystate[SDLK_9]ALT ) c=']';
	else if ( keystate[SDLK_9] ) c='9';
	else if ( keystate[92]SHIFT ) c='>';
	else if ( keystate[92]ALT ) c='|';
	else if ( keystate[92] ) c='<';
	else if ( keystate[61]SHIFT ) c='`';
	else if ( keystate[61] ) c='?';
	else if ( keystate[45]SHIFT ) c='?';
	else if ( keystate[45]ALT ) c='\\';
	else if ( keystate[45] ) c='ß';

	// else if(keystate[SDLK_KP0])c='0';
	else if ( keystate[SDLK_KP1] ) c='1';
	else if ( keystate[SDLK_KP2] ) c='2';
	else if ( keystate[SDLK_KP3] ) c='3';
	else if ( keystate[SDLK_KP4] ) c='4';
	else if ( keystate[SDLK_KP5] ) c='5';
	else if ( keystate[SDLK_KP6] ) c='6';
	else if ( keystate[SDLK_KP7] ) c='7';
	else if ( keystate[SDLK_KP8] ) c='8';
	else if ( keystate[SDLK_KP9] ) c='9';
	else if ( keystate[SDLK_KP_PERIOD] ) c='.';
	else if ( keystate[SDLK_KP_DIVIDE] ) c='/';
	else if ( keystate[SDLK_KP_MULTIPLY] ) c='*';
	else if ( keystate[SDLK_KP_MINUS] ) c='-';
	else if ( keystate[SDLK_KP_PLUS] ) c='+';
	else if ( keystate[SDLK_KP_EQUALS] ) c='=';
	else
	{
		lc = -1;
		return false;
	}

	static unsigned int lasttime;
	if ( c == lc || ( c >= 'a'&& c <= 'z' && lc- ( 'A'-'a' ) == c ) )
	{
		int time;
		time=SDL_GetTicks()-lasttime;
		if ( time<DelayTime )
			return false;
		DelayTime=50;
	}
	else
	{
		DelayTime=500;
	}
	if ( c == -3 )
	{
		InputEnter = true;
		keystate[SDLK_RETURN] = 0;
	}
	else if ( c == -2 && InputStr.length() >= 1 )
		InputStr.erase ( InputStr.length()-1 );
	else
		InputStr += ( char ) c;
	lc = c;
	lasttime=SDL_GetTicks();
	return true;
}
