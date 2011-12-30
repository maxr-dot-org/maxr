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
#include "player.h"
#include "attackJobs.h"

#include "vehicles.h"
#include "buildings.h"

using namespace std;

//-----------------------------------------------------------------------------
cUnit::cUnit (UnitType unitType, sUnitData* unitData, cPlayer* owner)
: PosX (0)
, PosY (0)
, dir (0)
, turnsDisabled (0)
, sentryActive (false)
, manualFireActive (false)
, attacking (false)
, isBeeingAttacked (false)
, owner (owner)
, unitType (unitType)
, isOriginalName (true)
{
	if (unitData != 0)
		data = *unitData;
	
	sentryActive = (isBuilding () && data.canAttack != TERRAIN_NONE);
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

//-----------------------------------------------------------------------------
bool cUnit::isNextTo (int x, int y) const
{
	if (x + 1 < PosX || y + 1 < PosY)
		return false;
	
	if (data.isBig)
	{
		if (x - 2 > PosX || y - 2 > PosY) 
			return false;
	}
	else 
	{
		if (x - 1 > PosX || y - 1 > PosY)
			return false;
	}
	
	return true;
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

//-----------------------------------------------------------------------------
/** Returns true, if the coordinates are in the menu's space */
//-----------------------------------------------------------------------------
bool cUnit::areCoordsOverMenu (int x, int y)
{
	SDL_Rect r = getMenuSize();
	
	if (x < r.x || x > r.x + r.w)
		return false;
	
	if (y < r.y || y > r.y + r.h)
		return false;
	
	return true;
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

//-----------------------------------------------------------------------------
/** Centers on this unit */
//-----------------------------------------------------------------------------
void cUnit::center () const
{
	int offX = PosX * 64 - ((int)( ((float)(Video.getResolutionX () - 192) / (2 * Client->gameGUI.getTileSize ())) * 64)) + 32;
	int offY = PosY * 64 - ((int)( ((float)(Video.getResolutionY () - 32)  / (2 * Client->gameGUI.getTileSize ())) * 64)) + 32;
	Client->gameGUI.setOffsetPosition (offX, offY);
}

//-----------------------------------------------------------------------------
/** Draws the ammunition bar over the unit */
//-----------------------------------------------------------------------------
void cUnit::drawMunBar () const
{
	if (owner != Client->ActivePlayer)
		return;
	
	SDL_Rect r1, r2;
	r1.x = getScreenPosX () + Client->gameGUI.getTileSize () / 10 + 1;
	r1.w = Client->gameGUI.getTileSize () * 8 / 10 ;
	r1.h = Client->gameGUI.getTileSize () / 8;
	r1.y = getScreenPosY () + Client->gameGUI.getTileSize () / 10 + Client->gameGUI.getTileSize () / 8;
	
	if (r1.h <= 2)
	{
		r1.y += 1;
		r1.h = 3;
	}
	
	r2.x = r1.x + 1;
	r2.y = r1.y + 1;
	r2.h = r1.h - 2;
	r2.w = (int)(((float)(r1.w - 2) / data.ammoMax) * data.ammoCur);
	
	SDL_FillRect (buffer, &r1, 0);
	
	if (data.ammoCur > data.ammoMax / 2)
		SDL_FillRect (buffer, &r2, 0x04AE04);
	else if (data.ammoCur > data.ammoMax / 4)
		SDL_FillRect (buffer, &r2, 0xDBDE00);
	else
		SDL_FillRect (buffer, &r2, 0xE60000);
}

//------------------------------------------------------------------------
/** draws the health bar over the unit */
//--------------------------------------------------------------------------
void cUnit::drawHealthBar () const
{
	SDL_Rect r1, r2;
	r1.x = getScreenPosX () + Client->gameGUI.getTileSize () / 10 + 1;
	r1.w = Client->gameGUI.getTileSize () * 8 / 10 ;
	r1.h = Client->gameGUI.getTileSize () / 8;
	r1.y = getScreenPosY () + Client->gameGUI.getTileSize () / 10;
	
	if (data.isBig)
	{
		r1.w += Client->gameGUI.getTileSize ();
		r1.h *= 2;
	}
	
	if (r1.h <= 2)
		r1.h = 3;
	
	r2.x = r1.x + 1;
	r2.y = r1.y + 1;
	r2.h = r1.h - 2;
	r2.w = (int)( ((float)(r1.w - 2) / data.hitpointsMax) * data.hitpointsCur);
	
	SDL_FillRect (buffer, &r1, 0);
	
	if (data.hitpointsCur > data.hitpointsMax / 2)
		SDL_FillRect (buffer, &r2, 0x04AE04);
	else if (data.hitpointsCur > data.hitpointsMax / 4)
		SDL_FillRect (buffer, &r2, 0xDBDE00);
	else
		SDL_FillRect (buffer, &r2, 0xE60000);
}

//-----------------------------------------------------------------------------
void cUnit::drawStatus () const
{
	SDL_Rect dest;
	SDL_Rect speedSymbol = {244, 97, 8, 10};
	SDL_Rect shotsSymbol = {254, 97, 5, 10};
	SDL_Rect disabledSymbol = {150, 109, 25, 25};
	
	if (turnsDisabled > 0)
	{
		if (Client->gameGUI.getTileSize () < 25) 
			return;
		dest.x = getScreenPosX () + Client->gameGUI.getTileSize () / 2 - 12;
		dest.y = getScreenPosY () + Client->gameGUI.getTileSize () / 2 - 12;
		SDL_BlitSurface (GraphicsData.gfx_hud_stuff, &disabledSymbol, buffer, &dest);
	}
	else
	{
		dest.y = getScreenPosY () + Client->gameGUI.getTileSize () - 11;
		dest.x = getScreenPosX () + Client->gameGUI.getTileSize () / 2 - 4;
		if (data.isBig)
		{
			dest.y += (Client->gameGUI.getTileSize () / 2);
			dest.x += (Client->gameGUI.getTileSize () / 2);
		}
		if (data.speedCur >= 4)
		{
			if (data.shotsCur) 
				dest.x -= Client->gameGUI.getTileSize () / 4;
			SDL_BlitSurface (GraphicsData.gfx_hud_stuff, &speedSymbol, buffer, &dest);
		}
		
		dest.x = getScreenPosX () + Client->gameGUI.getTileSize () / 2 - 4;
		if (data.shotsCur)
		{
			if (data.speedCur)
				dest.x += Client->gameGUI.getTileSize () / 4;
			SDL_BlitSurface (GraphicsData.gfx_hud_stuff, &shotsSymbol, buffer, &dest);
		}
	}
}

//-----------------------------------------------------------------------------
/** rotates the unit to the given direction */
//-----------------------------------------------------------------------------
void cUnit::rotateTo (int newDir)
{
	if (newDir < 0 || newDir >= 8 || newDir == dir) 
		return;
		
	int t = dir;
	int dest;
	
	for (int i = 0; i < 8; i++)
	{
		if (t == newDir)
		{
			dest = i;
			break;
		}
		t++;
		
		if (t > 7)
			t = 0;
	}
	
	if (dest < 4)
		dir++;
	else
		dir--;
	
	if (dir < 0)
		dir += 8;
	else
	{
		if (dir > 7)
			dir -= 8;
	}
}



//-----------------------------------------------------------------------------
/** Checks, if the unit can attack an object at the given coordinates*/
//-----------------------------------------------------------------------------
bool cUnit::canAttackObjectAt (int x, int y, cMap* map, bool forceAttack, bool checkRange)
{
	int off = x + y * map->size;
	
	if (isUnitLoaded ())
		return false;
	
	if (data.canAttack == false)
		return false;
	
	if (data.shotsCur <= 0)
		return false;
	
	if (data.ammoCur <= 0)
		return false;
	
	if (attacking)
		return false;
	
	if (isBeeingAttacked)
		return false;
	
	if (off < 0)
		return false;
	
	if (checkRange && isInRange (x, y) == false)
		return false;
	
	if (data.muzzleType == sUnitData::MUZZLE_TYPE_TORPEDO && map->isWater(x, y) == false)
		return false;
	
	if (owner->ScanMap[off] == false)
		return forceAttack ? true : false;
	
	if (forceAttack)
		return true;
	
	cVehicle* targetVehicle = 0;
	cBuilding* targetBuilding = 0;	
	selectTarget (targetVehicle, targetBuilding, x, y, data.canAttack, map);
	
	if (targetVehicle != 0)
	{
		if (Client && (targetVehicle == Client->gameGUI.getSelVehicle () || targetVehicle->owner == Client->ActivePlayer))
			return false;
	}
	else if (targetBuilding != 0)
	{
		if (isVehicle () && map->possiblePlace ((cVehicle*)this, x, y))  //do not fire on e. g. platforms, connectors etc.
			return false;												 //see ticket #436 on bug tracker
		
		if (Client && (targetBuilding == Client->gameGUI.getSelBuilding () || targetBuilding->owner == Client->ActivePlayer))
			return false;
	}
	else
		return false;
	
	return true;
}

//-----------------------------------------------------------------------------
void cUnit::upgradeToCurrentVersion ()
{
	sUnitData* upgradeVersion = getUpgradedUnitData ();
	if (upgradeVersion != 0)
	{
		data.version = upgradeVersion->version;
		
		if (data.hitpointsCur == data.hitpointsMax)
			data.hitpointsCur = upgradeVersion->hitpointsMax; // TODO: check behaviour in original
		data.hitpointsMax = upgradeVersion->hitpointsMax;
		
		data.ammoMax = upgradeVersion->ammoMax; // don't change the current ammo-amount!
		
		data.speedMax = upgradeVersion->speedMax;
		
		data.armor = upgradeVersion->armor;
		data.scan = upgradeVersion->scan;
		data.range = upgradeVersion->range;
		data.shotsMax = upgradeVersion->shotsMax; // TODO: check behaviour in original
		data.damage = upgradeVersion->damage;
		data.buildCosts = upgradeVersion->buildCosts;
	}
}

