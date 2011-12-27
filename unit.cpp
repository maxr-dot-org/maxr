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

#include "unit.h"
#include "math.h"
#include "client.h"
#include "video.h"

using namespace std;

//-----------------------------------------------------------------------------
cUnit::cUnit (UnitType unitType, sUnitData* unitData)
: unitType (unitType)
, isOriginalName (true)
, PosX (0)
, PosY (0)
{
	if (unitData != 0)
		data = *unitData;
}

//-----------------------------------------------------------------------------
cUnit::~cUnit ()
{
}

//--------------------------------------------------------------------------
/** returns the remaining hitpoints after an attack */
//--------------------------------------------------------------------------
int cUnit::calcHealth (int damage) const
{
	damage -= data.armor;
	
	if (damage <= 0)
	{
		//minimum damage is 1
		damage = 1;
	}
	
	int hp;
	hp = data.hitpointsCur - damage;
	
	if (hp < 0)
		return 0;
	
	return hp;
}

//--------------------------------------------------------------------------
/** Checks if the target is in range */
//--------------------------------------------------------------------------
bool cUnit::isInRange (int x, int y) const
{
	x -= PosX;
	y -= PosY;
	
	return (sqrt ((double) (x*x + y*y)) <= data.range);
}

//--------------------------------------------------------------------------
/** generates the name for the unit depending on the versionnumber */
//--------------------------------------------------------------------------
string cUnit::getNamePrefix () const
{
	int tmp;
	string rome = "";
	int nr = data.version + 1;	// +1, because the numbers in the name start at 1, not at 0
	
	// generate the roman versionnumber (correct until 899)
	
	if ( nr > 100 )
	{
		tmp = nr / 100;
		nr %= 100;
		
		while ( tmp-- )
			rome += "C";
	}
	
	if ( nr >= 90 )
	{
		rome += "XC";
		nr -= 90;
	}
	
	if ( nr >= 50 )
	{
		nr -= 50;
		rome += "L";
	}
	
	if ( nr >= 40 )
	{
		nr -= 40;
		rome += "XL";
	}
	
	if ( nr >= 10 )
	{
		tmp = nr / 10;
		nr %= 10;
		
		while ( tmp-- )
			rome += "X";
	}
	
	if ( nr == 9 )
	{
		nr -= 9;
		rome += "IX";
	}
	
	if ( nr >= 5 )
	{
		nr -= 5;
		rome += "V";
	}
	
	if ( nr == 4 )
	{
		nr -= 4;
		rome += "IV";
	}
	
	// alzi:
	// We had a bug, when 'nr' was negative and the following loop never terminated.
	// I modified the loop to terminate on a negative 'nr', but since this should never be the case,
	// the error has to be occured somewhere before and I added this warning.
	if ( nr < 0 )
	{
		Log.write( "cUnit: Negative 'nr' in cUnit::getNamePrefix()", cLog::eLOG_TYPE_WARNING );
	}
	while ( nr-- > 0 )
	{
		rome += "I";
	}
	
	return "MK " + rome;
}

//-----------------------------------------------------------------------------
/** Returns the name of the vehicle how it should be displayed */
//-----------------------------------------------------------------------------
string cUnit::getDisplayName () const
{
	return getNamePrefix () + " " + (isNameOriginal () ? data.name : name);
}

//-----------------------------------------------------------------------------
/** changes the name of the unit and indicates it as "not default" */
//-----------------------------------------------------------------------------
void cUnit::changeName (string newName)
{
	name = newName;
	isOriginalName = false;
}

//-----------------------------------------------------------------------------
/** Returns the size of the menu and the position */
//-----------------------------------------------------------------------------
SDL_Rect cUnit::getMenuSize () const
{
	SDL_Rect dest;
	int i, size;
	dest.x = getScreenPosX ();
	dest.y = getScreenPosY ();
	dest.h = i = getNumberOfMenuEntries () * 22;
	dest.w = 42;
	size = Client->gameGUI.getTileSize ();
	
	if (data.isBig || treatAsBigForMenuDisplay ())
		size *= 2;
	
	if (dest.x + size + 42 >= Video.getResolutionX () - 12)
		dest.x -= 42;
	else
		dest.x += size;
	
	if (dest.y - (i - size) / 2 <= 24)
	{
		dest.y -= (i - size) / 2;
		dest.y += - (dest.y - 24);
	}
	else if (dest.y - (i - size) / 2 + i >= Video.getResolutionY () - 24)
	{
		dest.y -= (i - size) / 2;
		dest.y -= (dest.y + i) - (Video.getResolutionY () - 24);
	}
	else
		dest.y -= (i - size) / 2;
	
	return dest;
}

//--------------------------------------------------------------------------
/** Returns the screen x position of the unit */
//--------------------------------------------------------------------------
int cUnit::getScreenPosX () const
{
	return 180 - ((int)((Client->gameGUI.getOffsetX () - getMovementOffsetX ()) * Client->gameGUI.getZoom ())) + (int)(Client->gameGUI.getTileSize ()) * PosX;
}

//--------------------------------------------------------------------------
/** Returns the screen y position of the unit */
//--------------------------------------------------------------------------
int cUnit::getScreenPosY () const
{
	return 18 - ((int)((Client->gameGUI.getOffsetY () - getMovementOffsetY ()) * Client->gameGUI.getZoom ())) + (int)(Client->gameGUI.getTileSize ()) * PosY;
}

