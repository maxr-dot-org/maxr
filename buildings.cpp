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
#include "math.h"
#include "buildings.h"
#include "main.h"
#include "unifonts.h"
#include "mouse.h"
#include "files.h"
#include "pcx.h"
#include "events.h"
#include "serverevents.h"
#include "client.h"
#include "server.h"
#include "upgradecalculator.h"
#include "menus.h"
#include "dialog.h"


//--------------------------------------------------------------------------
// cBuilding Implementation
//--------------------------------------------------------------------------

//--------------------------------------------------------------------------
cBuilding::cBuilding ( sBuilding *b, cPlayer *Owner, cBase *Base )
{
	PosX = 0;
	PosY = 0;
	RubbleTyp = 0;
	RubbleValue = 0;
	EffectAlpha = 0;
	EffectInc = true;
	dir = 0;
	StartUp = 0;
	IsWorking = false;
	researchArea = cResearch::kAttackResearch;
	IsLocked = false;
	typ = b;
	owner = Owner;
	base = Base;

	if ( Owner == NULL || b == NULL )
	{
		if ( b != NULL )
		{
			data = b->data;
		}
		else
		{
			memset ( &data, 0, sizeof ( sUnitData ) );
		}

		BuildList = NULL;

		bSentryStatus = false;
		return;
	}

	data = owner->BuildingData[typ->nr];

	selected = false;
	MenuActive = false;
	AttackMode = false;
	Transfer = false;
	BaseN = false;
	BaseBN = false;
	BaseE = false;
	BaseBE = false;
	BaseS = false;
	BaseBS = false;
	BaseW = false;
	BaseBW = false;
	Attacking = false;
	LoadActive = false;
	ActivatingVehicle = false;
	bIsBeeingAttacked = false;
	RepeatBuild = false;
	hasBeenAttacked = false;

	if ( data.can_attack != ATTACK_NONE ) bSentryStatus = true;
	else bSentryStatus = false;
	
	MaxMetalProd = 0;
	MaxGoldProd = 0;
	MaxOilProd = 0;
	Disabled = 0;
	data.hit_points = data.max_hit_points;
	data.ammo = data.max_ammo;
	SubBase = NULL;
	BuildSpeed = 0;
	BuildList = NULL;

	if ( data.can_build )
		BuildList = new cList<sBuildList*>;

	if ( data.is_big )
	{
		DamageFXPointX  = random(64) + 32;
		DamageFXPointY  = random(64) + 32;
		DamageFXPointX2 = random(64) + 32;
		DamageFXPointY2 = random(64) + 32;
	}
	else
	{
		DamageFXPointX = random(64 - 24);
		DamageFXPointY = random(64 - 24);
	}

	refreshData();
}

//--------------------------------------------------------------------------
cBuilding::~cBuilding ()
{
	if ( BuildList )
	{
		while (BuildList->Size())
		{
			sBuildList *ptr;
			ptr = (*BuildList)[0];
			delete ptr;
			BuildList->Delete( 0 );
		}
		delete BuildList;
	}

	while (StoredVehicles.Size())
	{
		cVehicle *v;
		v = StoredVehicles[0];

		if ( v->prev )
		{
			cVehicle *vp;
			vp = v->prev;
			vp->next = v->next;

			if ( v->next )
				v->next->prev = vp;
		}
		else
		{
			v->owner->VehicleList = v->next;

			if ( v->next )
				v->next->prev = NULL;
		}
		delete v;

		StoredVehicles.Delete ( 0 );
	}

	if ( bSentryStatus )
	{
		owner->deleteSentryBuilding ( this );
	}

	if ( IsLocked )
	{
		cPlayer *p;

		for (unsigned int i = 0; i < Client->PlayerList->Size(); i++)
		{
			p = (*Client->PlayerList)[i];
			p->DeleteLock ( this );
		}
	}
	if ( Server )
	{
		for ( unsigned int i = 0; i < Server->AJobs.Size(); i++ )
		{
			if ( Server->AJobs[i]->building == this ) Server->AJobs[i]->building = NULL;
		}
	}
	if ( Client )
	{
		for ( unsigned int i = 0; i < Client->attackJobs.Size(); i++ )
		{
			if ( Client->attackJobs[i]->building == this ) Client->attackJobs[i]->building = NULL;
		}
	}
}

//----------------------------------------------------
/** Returns a string with the current state */
//----------------------------------------------------
string cBuilding::getStatusStr ()
{
	if ( IsWorking )
	{
		// Factory:
		if (data.can_build && BuildList && BuildList->Size() && owner == Client->ActivePlayer)
		{
			sBuildList *ptr;
			ptr = (*BuildList)[0];

			if ( ptr->metall_remaining > 0 )
			{
				string sText;
				int iRound;

				iRound = ( int ) ceil ( ptr->metall_remaining / ( double ) MetalPerRound );
				sText = lngPack.i18n ( "Text~Comp~Producing" ) + ": ";
				sText += ( string ) owner->VehicleData[ptr->typ->nr].szName + " (";
				sText += iToStr ( iRound ) + ")";

				if ( font->getTextWide ( sText, FONT_LATIN_SMALL_WHITE ) > 126 )
				{
					sText = lngPack.i18n ( "Text~Comp~Producing" ) + ":\n";
					sText += ( string ) owner->VehicleData[ptr->typ->nr].szName + " (";
					sText += iToStr ( iRound ) + ")";
				}

				return sText;
			}
			else
			{
				return lngPack.i18n ( "Text~Comp~Producing_Fin" );
			}
		}

		// Research Center
		if (data.can_research && owner == Client->ActivePlayer)
		{
			string sText = lngPack.i18n ( "Text~Comp~Working" ) + "\n";
			for (int area = 0; area < cResearch::kNrResearchAreas; area++)
			{
				if (owner->researchCentersWorkingOnArea[area] > 0)
				{
					switch (area)
					{
						case cResearch::kAttackResearch: sText += lngPack.i18n ( "Text~Vehicles~Damage" ); break;
						case cResearch::kShotsResearch: sText += lngPack.i18n ( "Text~Hud~Shots" ); break;
						case cResearch::kRangeResearch: sText += lngPack.i18n ( "Text~Hud~Range" ); break;
						case cResearch::kArmorResearch: sText += lngPack.i18n ( "Text~Vehicles~Armor" ); break;
						case cResearch::kHitpointsResearch: sText += lngPack.i18n ( "Text~Hud~Hitpoints" ); break;
						case cResearch::kSpeedResearch: sText += lngPack.i18n ( "Text~Hud~Speed" ); break;
						case cResearch::kScanResearch: sText += lngPack.i18n ( "Text~Hud~Scan" ); break;
						case cResearch::kCostResearch: sText += lngPack.i18n ( "Text~Vehicles~Costs" ); break;
					}
					sText += ": " + iToStr (owner->researchLevel.getRemainingTurns (area, owner->researchCentersWorkingOnArea[area])) + "\n";
				}
			}
			return sText;
		}

		// Goldraffinerie:
		if ( data.gold_need && owner == Client->ActivePlayer )
		{
			string sText;
			sText = lngPack.i18n ( "Text~Comp~Working" ) + "\n";
			sText += lngPack.i18n ( "Text~Title~Credits" ) + ": ";
			sText += iToStr ( owner->Credits );
			return sText.c_str();
		}

		return lngPack.i18n ( "Text~Comp~Working" );
	}

	if ( Disabled )
	{
		string sText;
		sText = lngPack.i18n ( "Text~Comp~Disabled" ) + " (";
		sText += iToStr ( Disabled ) + ")";
		return sText.c_str();
	}
	if ( bSentryStatus )
	{
		return lngPack.i18n ( "Text~Comp~Sentry" );
	}

	return lngPack.i18n ( "Text~Comp~Waits" );
}

//--------------------------------------------------------------------------
/** Refreshs all data to the maximum values */
//--------------------------------------------------------------------------
int cBuilding::refreshData ()
{
	if ( data.shots < data.max_shots )
	{
		if ( data.ammo >= data.max_shots )
			data.shots = data.max_shots;
		else
			data.shots = data.ammo;
		return 1;
	}
	return 0;
}

//--------------------------------------------------------------------------
/** generates the name for the building depending on the versionnumber */
//--------------------------------------------------------------------------
void cBuilding::GenerateName ()
{
	string rome, tmp_name;
	int nr, tmp;
	string::size_type tmp_name_idx;
	rome = "";
	nr = data.version;

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

	while ( nr-- )
		rome += "I";

	// concatenate the name
	// name=(string)data.name + " MK "+rome;
/*
	name = ( string ) data.name;

	name += " MK ";

	name += rome;
*/
	if ( name.length() == 0 )
	{
		// prefix
		name = "MK ";
		name += rome;
		name += " ";
		// object name
		name += ( string ) data.szName;
	}
	else
	{
		// check for MK prefix
		tmp_name = name.substr(0,2);
		if ( 0 == (int)tmp_name.compare("MK") )
		{
			// current name, without prefix
			tmp_name_idx = name.find_first_of(" ", 4 );
			if( tmp_name_idx != string::npos )
			{
				tmp_name = ( string )name.substr(tmp_name_idx);
				// prefix
				name = "MK ";
				name += rome;
				// name
				name += tmp_name;
			}
			else
			{
				tmp_name = name;
				// prefix
				name = "MK ";
				name += rome;
				name += " ";
				// name
				name += tmp_name;
			}
		}
		else
		{
			tmp_name = name;
			name = "MK ";
			name += rome;
			name += " ";
			name += tmp_name;
		}
	}
}

//--------------------------------------------------------------------------
void cBuilding::draw ( SDL_Rect *screenPos )
{
	SDL_Rect dest, tmp;
	float factor = (float)(Client->Hud.Zoom/64.0);
	
	// draw the damage effects
	if ( Client->iTimer1 && !data.is_base && !data.is_connector && data.hit_points < data.max_hit_points && SettingsData.bDamageEffects && ( owner == Client->ActivePlayer || Client->ActivePlayer->ScanMap[PosX+PosY*Client->Map->size] ) )
	{
		int intense = ( int ) ( 200 - 200 * ( ( float ) data.hit_points / data.max_hit_points ) );
		Client->addFX ( fxDarkSmoke, PosX*64 + DamageFXPointX, PosY*64 + DamageFXPointY, intense );

		if ( data.is_big && intense > 50 )
		{
			intense -= 50;
			Client->addFX ( fxDarkSmoke, PosX*64 + DamageFXPointX2, PosY*64 + DamageFXPointY2, intense );
		}
	}

	dest.x = dest.y = 0;
	bool bDraw = false;
	SDL_Surface* drawingSurface = Client->dCache.getCachedImage(this);
	if ( drawingSurface == NULL )
	{
		//no cached image found. building needs to be redrawn.
		bDraw = true;
		drawingSurface = Client->dCache.createNewEntry(this);
	}

	if ( drawingSurface == NULL )
	{
		//image will not be cached. So blitt directly to the screen buffer.
		dest = *screenPos;
		drawingSurface = buffer;
	}
		
	if ( bDraw )
	{
		render ( drawingSurface, dest );
	}

	//now check, whether the image has to be blitted to screen buffer
	if ( drawingSurface != buffer )
	{
		dest = *screenPos;
		SDL_BlitSurface( drawingSurface, NULL, buffer, &dest );

		//all folling graphic operations are drawn directly to buffer
		dest = *screenPos;
	}

	if (!owner ) return;

	if ( StartUp )
	{
		if ( Client->iTimer0 )
			StartUp += 25;

		if ( StartUp >= 255 )
			StartUp = 0;
	}

	// draw the effect if necessary
	if ( data.has_effect && SettingsData.bAnimations && ( IsWorking || !data.can_work ) )
	{
		tmp = dest;
		SDL_SetAlpha ( typ->eff, SDL_SRCALPHA, EffectAlpha );

		CHECK_SCALING( typ->eff, typ->eff_org, factor);
		SDL_BlitSurface ( typ->eff, NULL, buffer, &tmp );

		if ( Client->iTimer0 )
		{
			if ( EffectInc )
			{
				EffectAlpha += 30;

				if ( EffectAlpha > 220 )
				{
					EffectAlpha = 255;
					EffectInc = false;
				}
			}
			else
			{
				EffectAlpha -= 30;

				if ( EffectAlpha < 30 )
				{
					EffectAlpha = 0;
					EffectInc = true;
				}
			}
		}
	}

	// draw the mark, when a build order is finished 
	if (BuildList && BuildList->Size() && !IsWorking && (*BuildList)[0]->metall_remaining <= 0 && owner == Client->ActivePlayer)
	{
		SDL_Rect d, t;
		int max, nr;
		nr = 0xFF00 - ( ( Client->iFrame % 0x8 ) * 0x1000 );
		max = ( Client->Hud.Zoom - 2 ) * 2;
		d.x = dest.x + 2;
		d.y = dest.y + 2;
		d.w = max;
		d.h = 1;
		t = d;
		SDL_FillRect ( buffer, &d, nr );
		d = t;
		d.y += max - 1;
		t = d;
		SDL_FillRect ( buffer, &d, nr );
		d = t;
		d.y = dest.y + 2;
		d.w = 1;
		d.h = max;
		t = d;
		SDL_FillRect ( buffer, &d, nr );
		d = t;
		d.x += max - 1;
		SDL_FillRect ( buffer, &d, nr );
	}

	// draw a colored frame if necessary
	if ( Client->Hud.Farben )
	{
		SDL_Rect d, t;
		int nr = *((unsigned int*)(owner->color->pixels));
		int max = data.is_big ? (Client->Hud.Zoom - 1) * 2 : Client->Hud.Zoom - 1;

		d.x = dest.x + 1;
		d.y = dest.y + 1;
		d.w = max;
		d.h = 1;
		t = d;
		SDL_FillRect ( buffer, &d, nr );
		d = t;
		d.y += max - 1;
		t = d;
		SDL_FillRect ( buffer, &d, nr );
		d = t;
		d.y = dest.y + 1;
		d.w = 1;
		d.h = max;
		t = d;
		SDL_FillRect ( buffer, &d, nr );
		d = t;
		d.x += max - 1;
		SDL_FillRect ( buffer, &d, nr );
	}

	// draw a colored frame if necessary
	if ( selected )
	{
		SDL_Rect d, t;
		int max = data.is_big ? Client->Hud.Zoom * 2 : Client->Hud.Zoom;
		int len = max / 4;

		d.x = dest.x + 1;
		d.y = dest.y + 1;
		d.w = len;
		d.h = 1;
		t = d;
		SDL_FillRect ( buffer, &d, Client->iBlinkColor );
		d = t;
		d.x += max - len - 1;
		t = d;
		SDL_FillRect ( buffer, &d, Client->iBlinkColor );
		d = t;
		d.y += max - 2;
		t = d;
		SDL_FillRect ( buffer, &d, Client->iBlinkColor );
		d = t;
		d.x = dest.x + 1;
		t = d;
		SDL_FillRect ( buffer, &d, Client->iBlinkColor );
		d = t;
		d.y = dest.y + 1;
		d.w = 1;
		d.h = len;
		t = d;
		SDL_FillRect ( buffer, &d, Client->iBlinkColor );
		d = t;
		d.x += max - 2;
		t = d;
		SDL_FillRect ( buffer, &d, Client->iBlinkColor );
		d = t;
		d.y += max - len - 1;
		t = d;
		SDL_FillRect ( buffer, &d, Client->iBlinkColor );
		d = t;
		d.x = dest.x + 1;
		SDL_FillRect ( buffer, &d, Client->iBlinkColor );
	}

	//draw health bar
	if ( Client->Hud.Treffer )
		DrawHelthBar();

	//draw ammo bar
	if ( Client->Hud.Munition && data.can_attack && data.max_ammo > 0 )
		DrawMunBar();

	//draw status
	if ( Client->Hud.Status )
		drawStatus();

	//attack job debug output
	if ( Client->bDebugAjobs )
	{
		cBuilding* serverBuilding = NULL;
		if ( Server ) serverBuilding = Server->Map->fields[PosX + PosY*Server->Map->size].getBuildings();
		if ( bIsBeeingAttacked ) font->showText(dest.x + 1,dest.y + 1, "C: attacked", FONT_LATIN_SMALL_WHITE );
		if ( serverBuilding && serverBuilding->bIsBeeingAttacked ) font->showText(dest.x + 1,dest.y + 9, "S: attacked", FONT_LATIN_SMALL_YELLOW );
		if ( Attacking ) font->showText(dest.x + 1,dest.y + 17, "C: attacking", FONT_LATIN_SMALL_WHITE );
		if ( serverBuilding && serverBuilding->Attacking ) font->showText(dest.x + 1,dest.y + 25, "S: attacking", FONT_LATIN_SMALL_YELLOW );
	}
}

void cBuilding::render( SDL_Surface* surface, const SDL_Rect& dest)
{	
	//Note: when changing something in this function, make sure to update the caching rules!
	SDL_Rect src, tmp;
	src.x = 0;
	src.y = 0;

	float factor = (float)(Client->Hud.Zoom/64.0);

	// check, if it is dirt:
	if ( !owner )
	{
		if ( data.is_big )
		{
			if ( !UnitsData.dirt_big ) return;
			src.w = src.h = (int)(UnitsData.dirt_big_org->h*factor);
		}
		else
		{
			if ( !UnitsData.dirt_small ) return;
			src.w = src.h = (int)(UnitsData.dirt_small_org->h*factor);
		}

		src.x = src.w * RubbleTyp;
		tmp = dest;
		src.y = 0;

		// draw the shadows
		if ( SettingsData.bShadows )
		{
			if ( data.is_big )
			{
				CHECK_SCALING( UnitsData.dirt_big_shw, UnitsData.dirt_big_shw_org, factor );
				SDL_BlitSurface ( UnitsData.dirt_big_shw, &src, surface, &tmp );
			}
			else
			{
				CHECK_SCALING( UnitsData.dirt_small_shw, UnitsData.dirt_small_shw_org, factor );
				SDL_BlitSurface ( UnitsData.dirt_small_shw, &src, surface, &tmp );
			}
		}

		// draw the building
		tmp = dest;

		if ( data.is_big )
		{
			CHECK_SCALING( UnitsData.dirt_big, UnitsData.dirt_big_org, factor);
			SDL_BlitSurface ( UnitsData.dirt_big, &src, surface, &tmp );
		}
		else
		{
			CHECK_SCALING( UnitsData.dirt_small, UnitsData.dirt_small_org, factor);
			SDL_BlitSurface ( UnitsData.dirt_small, &src, surface, &tmp );
		}

		return;
	}

	// read the size:
	if ( data.has_frames )
	{
		src.w = Client->Hud.Zoom;
		src.h = Client->Hud.Zoom;
	}
	else
	{
		src.w = (int)(typ->img_org->w*factor);
		src.h = (int)(typ->img_org->h*factor);
	}
	
	// draw the concrete
	tmp = dest;
	if ( !data.build_on_water && !data.is_expl_mine )
	{
		if ( data.is_big )
		{
			CHECK_SCALING( GraphicsData.gfx_big_beton, GraphicsData.gfx_big_beton_org, factor);

			if ( StartUp && SettingsData.bAlphaEffects )
				SDL_SetAlpha ( GraphicsData.gfx_big_beton, SDL_SRCALPHA, StartUp );
			else
				SDL_SetAlpha ( GraphicsData.gfx_big_beton, SDL_SRCALPHA, 255 );
			
			SDL_BlitSurface ( GraphicsData.gfx_big_beton, NULL, surface, &tmp );
		}
		else
		{
			CHECK_SCALING( UnitsData.ptr_small_beton, UnitsData.ptr_small_beton_org, factor);
			if ( !data.is_road && !data.is_connector )
			{
				if ( StartUp && SettingsData.bAlphaEffects )
					SDL_SetAlpha ( UnitsData.ptr_small_beton, SDL_SRCALPHA, StartUp );
				else
					SDL_SetAlpha ( UnitsData.ptr_small_beton, SDL_SRCALPHA, 255 );

				SDL_BlitSurface ( UnitsData.ptr_small_beton, NULL, surface, &tmp );
				SDL_SetAlpha ( UnitsData.ptr_small_beton, SDL_SRCALPHA, 255 );
			}
		}
	}

	tmp = dest;
 
	// draw the connector slots:
	if ( (this->SubBase && !StartUp) || data.is_connector )
	{
		drawConnectors (  surface, dest );
	}

	// draw the shadows
	if ( SettingsData.bShadows && !data.is_connector )
	{
		if ( StartUp && SettingsData.bAlphaEffects ) 
			SDL_SetAlpha ( typ->shw, SDL_SRCALPHA, StartUp / 5 );
		else
			SDL_SetAlpha ( typ->shw, SDL_SRCALPHA, 50 );

		CHECK_SCALING( typ->shw, typ->shw_org, factor);
		blittAlphaSurface ( typ->shw, NULL, surface, &tmp );
	}

	// blit the players color
	if ( !data.is_road && !data.is_connector )
	{
		SDL_BlitSurface ( owner->color, NULL, GraphicsData.gfx_tmp, NULL );

		if ( data.has_frames )
		{
			if ( data.is_annimated && SettingsData.bAnimations && !Disabled )
			{
				src.x = ( Client->iFrame % data.has_frames ) * Client->Hud.Zoom;
			}
			else
			{
				src.x = dir * Client->Hud.Zoom;
			}

			CHECK_SCALING( typ->img, typ->img_org, factor);
			SDL_BlitSurface ( typ->img, &src, GraphicsData.gfx_tmp, NULL );

			src.x = 0;
		}
		else if ( data.is_mine )
		{
			CHECK_SCALING( typ->img, typ->img_org, factor);
			src.x = 0;
			src.y = 0;
			src.w = (int) (128 * factor);
			src.h = (int) (128 * factor);
			//select clan image
			if ( owner->getClan() != -1 )
				src.x = (int) ((owner->getClan() + 1) * 128 * factor);
			SDL_BlitSurface ( typ->img, &src, GraphicsData.gfx_tmp, NULL );
		
		}
		else
		{
			CHECK_SCALING( typ->img, typ->img_org, factor);
			SDL_BlitSurface ( typ->img, NULL, GraphicsData.gfx_tmp, NULL );
		}
	}
	else if ( !data.is_connector )
	{
		SDL_FillRect ( GraphicsData.gfx_tmp, NULL, 0xFF00FF );

		CHECK_SCALING( typ->img, typ->img_org, factor);
		SDL_BlitSurface ( typ->img, NULL, GraphicsData.gfx_tmp, NULL );
	}

	// draw the building 
	tmp = dest;

	src.x = 0;
	src.y = 0;
	
	if ( !data.is_connector )
	{
		if ( StartUp && SettingsData.bAlphaEffects )
			SDL_SetAlpha ( GraphicsData.gfx_tmp, SDL_SRCALPHA, StartUp );
		else
			SDL_SetAlpha ( GraphicsData.gfx_tmp, SDL_SRCALPHA, 255 );

		SDL_BlitSurface ( GraphicsData.gfx_tmp, &src, surface, &tmp );
	}
}

//--------------------------------------------------------------------------
/** Get the numer of menu items */
//--------------------------------------------------------------------------
int cBuilding::GetMenuPointAnz ()
{
	int nr = 2;

	if ( !typ->data.is_road )
		nr++;

	if ( typ->data.can_build )
		nr++;

	if ( typ->data.can_load == TRANS_METAL || typ->data.can_load == TRANS_OIL || typ->data.can_load == TRANS_GOLD )
		nr++;

	if ( typ->data.can_attack && data.shots )
		nr++;

	if ( typ->data.can_work && ( IsWorking || typ->data.can_build == BUILD_NONE ) )
		nr++;

	if ( typ->data.is_mine )
		nr++;

	if ( bSentryStatus || data.can_attack )
		nr++;

	if ( typ->data.can_load == TRANS_VEHICLES || typ->data.can_load == TRANS_MEN || typ->data.can_load == TRANS_AIR )
		nr += 2;

	if ( typ->data.can_research && IsWorking )
		nr++;

	if ( data.version != owner->BuildingData[typ->nr].version && SubBase && SubBase->Metal >= 2 )
		nr += 2;

	if ( data.gold_need )
		nr++;

	return nr;
}

//--------------------------------------------------------------------------
/** Returns the size and position of the menu */
//--------------------------------------------------------------------------
SDL_Rect cBuilding::GetMenuSize ()
{
	SDL_Rect dest;
	int i, size;
	dest.x = GetScreenPosX();
	dest.y = GetScreenPosY();
	dest.h = i = GetMenuPointAnz() * 22;
	dest.w = 42;
	size = Client->Hud.Zoom;

	if ( data.is_big )
		size *= 2;

	if ( dest.x + size + 42 >= SettingsData.iScreenW - 12 )
		dest.x -= 42;
	else
		dest.x += size;

	if ( dest.y - ( i - size ) / 2 <= 24 )
	{
		dest.y -= ( i - size ) / 2;
		dest.y += - ( dest.y - 24 );
	}
	else if ( dest.y - ( i - size ) / 2 + i >= SettingsData.iScreenH - 24 )
	{
		dest.y -= ( i - size ) / 2;
		dest.y -= ( dest.y + i ) - ( SettingsData.iScreenH - 24 );
	}
	else
		dest.y -= ( i - size ) / 2;

	return dest;
}

//--------------------------------------------------------------------------
/** returns true, if the mouse coordinates are inside the menu area */
//--------------------------------------------------------------------------
bool cBuilding::MouseOverMenu (int mx, int my)
{
	SDL_Rect r;
	r = GetMenuSize();

	if ( mx < r.x || mx > r.x + r.w )
		return false;

	if ( my < r.y || my > r.y + r.h )
		return false;

	return true;
}

//--------------------------------------------------------------------------
/** displays the self destruction menu */
//--------------------------------------------------------------------------
void cBuilding::SelfDestructionMenu ()
{
	//TODO: self destruction dialog and destruction code
	Client->bFlagDrawHud = true;
}

//--------------------------------------------------------------------------
void cBuilding::updateNeighbours (cMap *Map)
{
	int iPosOff = PosX+PosY*Map->size;
	if ( !data.is_big )
	{
		owner->base.checkNeighbour ( iPosOff-Map->size, this );
		owner->base.checkNeighbour ( iPosOff+1, this );
		owner->base.checkNeighbour ( iPosOff+Map->size, this );
		owner->base.checkNeighbour ( iPosOff-1, this );
	}
	else
	{
		owner->base.checkNeighbour ( iPosOff-Map->size, this );
		owner->base.checkNeighbour ( iPosOff-Map->size+1, this );
		owner->base.checkNeighbour ( iPosOff+2, this );
		owner->base.checkNeighbour ( iPosOff+2+Map->size, this );
		owner->base.checkNeighbour ( iPosOff+Map->size*2, this );
		owner->base.checkNeighbour ( iPosOff+Map->size*2+1, this );
		owner->base.checkNeighbour ( iPosOff-1, this );
		owner->base.checkNeighbour ( iPosOff-1+Map->size, this );
	}
	CheckNeighbours( Map );
}

//--------------------------------------------------------------------------
/** Checks, if there are neighbours */
//--------------------------------------------------------------------------
void cBuilding::CheckNeighbours ( cMap *Map )
{
#define CHECK_NEIGHBOUR(x,y,m)								\
	if(x >= 0 && x < Map->size && y >= 0 && y < Map->size ) \
	{														\
		cBuilding* b = Map->fields[(x) + (y) * Map->size].getTopBuilding();		\
		if ( b && b->owner == owner && b->SubBase )			\
			{m=true;}else{m=false;}							\
	}														\

	if ( !data.is_big )
	{
		CHECK_NEIGHBOUR ( PosX    , PosY - 1, BaseN )
		CHECK_NEIGHBOUR ( PosX + 1, PosY    , BaseE )
		CHECK_NEIGHBOUR ( PosX    , PosY + 1, BaseS )
		CHECK_NEIGHBOUR ( PosX - 1, PosY    , BaseW )
	}
	else
	{
		
		CHECK_NEIGHBOUR ( PosX    , PosY - 1, BaseN  )
		CHECK_NEIGHBOUR ( PosX + 1, PosY - 1, BaseBN )
		CHECK_NEIGHBOUR ( PosX + 2, PosY    , BaseE  )
		CHECK_NEIGHBOUR ( PosX + 2, PosY + 1, BaseBE )
		CHECK_NEIGHBOUR ( PosX    , PosY + 2, BaseS  )
		CHECK_NEIGHBOUR ( PosX + 1, PosY + 2, BaseBS )
		CHECK_NEIGHBOUR ( PosX - 1, PosY    , BaseW  )
		CHECK_NEIGHBOUR ( PosX - 1, PosY + 1, BaseBW )
	}
}

//--------------------------------------------------------------------------
/** Draws the connectors at the building: */
//--------------------------------------------------------------------------
void cBuilding::drawConnectors ( SDL_Surface* surface, SDL_Rect dest )
{
	SDL_Rect src, temp;
	int zoom = Client->Hud.Zoom;
	float factor = (float)(zoom/64.0);

	CHECK_SCALING( UnitsData.ptr_connector, UnitsData.ptr_connector_org, factor);
	CHECK_SCALING( UnitsData.ptr_connector_shw, UnitsData.ptr_connector_shw_org, factor);

	if ( StartUp )
		SDL_SetAlpha( UnitsData.ptr_connector, SDL_SRCALPHA, StartUp );
	else
		SDL_SetAlpha( UnitsData.ptr_connector, SDL_SRCALPHA, 255 );

	src.y = 0;
	src.x = 0;
	src.h = src.w = UnitsData.ptr_connector->h;
	
	if ( !data.is_big )
	{
		if      (  BaseN &&  BaseE &&  BaseS &&  BaseW ) src.x = 15;
		else if (  BaseN &&  BaseE &&  BaseS && !BaseW ) src.x = 13; 
		else if (  BaseN &&  BaseE && !BaseS &&  BaseW ) src.x = 12;
		else if (  BaseN &&  BaseE && !BaseS && !BaseW ) src.x =  8;
		else if (  BaseN && !BaseE &&  BaseS &&  BaseW ) src.x = 11;
		else if (  BaseN && !BaseE &&  BaseS && !BaseW ) src.x =  5;
		else if (  BaseN && !BaseE && !BaseS &&  BaseW ) src.x =  7;
		else if (  BaseN && !BaseE && !BaseS && !BaseW ) src.x =  1;
		else if ( !BaseN &&  BaseE &&  BaseS &&  BaseW ) src.x = 14;
		else if ( !BaseN &&  BaseE &&  BaseS && !BaseW ) src.x =  9;
		else if ( !BaseN &&  BaseE && !BaseS &&  BaseW ) src.x =  6;
		else if ( !BaseN &&  BaseE && !BaseS && !BaseW ) src.x =  2;
		else if ( !BaseN && !BaseE &&  BaseS &&  BaseW ) src.x = 10;
		else if ( !BaseN && !BaseE &&  BaseS && !BaseW ) src.x =  3;
		else if ( !BaseN && !BaseE && !BaseS &&  BaseW ) src.x =  4;
		else if ( !BaseN && !BaseE && !BaseS && !BaseW ) src.x =  0;
		src.x *= src.h;

		if ( src.x != 0 || data.is_connector )
		{
			//blit shadow
			temp = dest;
			if ( SettingsData.bShadows ) blittAlphaSurface( UnitsData.ptr_connector_shw, &src, surface, &temp );
			//blit the image
			temp = dest;
			SDL_BlitSurface ( UnitsData.ptr_connector, &src, surface, &temp );
		}

	}
	else
	{
		//make connector stubs of big buildings.
		//upper left field
		src.x = 0;
		if      (  BaseN &&  BaseW ) src.x = 7;
		else if (  BaseN && !BaseW ) src.x = 1;
		else if ( !BaseN &&  BaseW ) src.x = 4;
		src.x *= src.h;

		if ( src.x != 0 )
		{
			temp = dest;
			if ( SettingsData.bShadows ) blittAlphaSurface( UnitsData.ptr_connector_shw, &src, surface, &temp );
			temp = dest;
			SDL_BlitSurface ( UnitsData.ptr_connector, &src, surface, &temp );
		}

		//upper right field
		src.x = 0;
		dest.x += zoom;
		if      (  BaseBN &&  BaseE ) src.x = 8;
		else if (  BaseBN && !BaseE ) src.x = 1;
		else if ( !BaseBN &&  BaseE ) src.x = 2;
		src.x *= src.h;

		if ( src.x != 0 )
		{
			temp = dest;
			if ( SettingsData.bShadows ) blittAlphaSurface( UnitsData.ptr_connector_shw, &src, surface, &temp );
			temp = dest;
			SDL_BlitSurface ( UnitsData.ptr_connector, &src, surface, &temp );
		}

		//lower right field
		src.x = 0;
		dest.y += zoom;
		if      (  BaseBE &&  BaseBS ) src.x = 9;
		else if (  BaseBE && !BaseBS ) src.x = 2;
		else if ( !BaseBE &&  BaseBS ) src.x = 3;
		src.x *= src.h;

		if ( src.x != 0 )
		{
			temp = dest;
			if ( SettingsData.bShadows ) blittAlphaSurface( UnitsData.ptr_connector_shw, &src, surface, &temp );
			temp = dest;
			SDL_BlitSurface ( UnitsData.ptr_connector, &src, surface, &temp );
		}

		//lower left field
		src.x = 0;
		dest.x -= zoom;
		if      (  BaseS &&  BaseBW ) src.x = 10;
		else if (  BaseS && !BaseBW ) src.x =  3;
		else if ( !BaseS &&  BaseBW ) src.x =  4;
		src.x *= src.h;

		if ( src.x != 0 )
		{
			temp = dest;
			if ( SettingsData.bShadows ) blittAlphaSurface( UnitsData.ptr_connector_shw, &src, surface, &temp );
			temp = dest;
			SDL_BlitSurface ( UnitsData.ptr_connector, &src, surface, &temp );
		}
	}
}

//--------------------------------------------------------------------------
/** starts the building for the server thread */
//--------------------------------------------------------------------------
void cBuilding::ServerStartWork ()
{
	if ( IsWorking )
	{
		sendDoStartWork(this);
		return;
	}

	//-- first check all requirements

	if ( Disabled )
	{
		sendChatMessageToClient("Text~Comp~Building_Disabled", SERVER_ERROR_MESSAGE, owner->Nr );
		return;
	}

	// needs human workers:
	if ( data.human_need )
		if ( SubBase->HumanNeed + data.human_need > SubBase->HumanProd )
		{
			sendChatMessageToClient ( "Text~Comp~Team_Low", SERVER_ERROR_MESSAGE, owner->Nr );
			return;
		}

	// needs gold:
	if ( data.gold_need )
	{
		if ( data.gold_need + SubBase->GoldNeed > SubBase->GoldProd + SubBase->Gold )
		{
			sendChatMessageToClient( "Text~Comp~Gold_Insufficient", SERVER_ERROR_MESSAGE, owner->Nr );
			return;
		}
	}

	// needs raw material:
	if ( data.metal_need )
	{
		if ( SubBase->MetalNeed + min(MetalPerRound, (*BuildList)[0]->metall_remaining) > SubBase->MetalProd + SubBase->Metal )
		{
			sendChatMessageToClient( "Text~Comp~Metal_Insufficient", SERVER_ERROR_MESSAGE, owner->Nr );
			return;
		}
	}

	// needs oil:
	if ( data.oil_need )
	{
		// check if there is enough Oil for the generators (current prodiction + reserves)
		if ( data.oil_need + SubBase->OilNeed > SubBase->Oil + SubBase->getMaxOilProd() )
		{
			sendChatMessageToClient ( "Text~Comp~Fuel_Insufficient", SERVER_ERROR_MESSAGE, owner->Nr );
			return;
		}
		else if ( data.oil_need + SubBase->OilNeed > SubBase->Oil + SubBase->getOilProd() )
		{
			//increase oil production
			int missingOil = data.oil_need + SubBase->OilNeed - (SubBase->Oil + SubBase->getOilProd());

			int metal = SubBase->getMetalProd();
			int gold = SubBase->getGoldProd();

			SubBase->setMetalProd(0);	//temporay decrease metal and gold production
			SubBase->setGoldProd(0);

			SubBase->changeOilProd( missingOil );

			SubBase->setGoldProd( gold );		
			SubBase->setMetalProd( metal );

			sendChatMessageToClient ( "Text~Comp~Adjustments_Fuel_Increased", SERVER_INFO_MESSAGE, owner->Nr, iToStr(missingOil) );
			if ( SubBase->getMetalProd() < metal )
				sendChatMessageToClient ( "Text~Comp~Adjustments_Metal_Decreased", SERVER_INFO_MESSAGE, owner->Nr, iToStr(metal - SubBase->getMetalProd()) );
			if ( SubBase->getGoldProd() < gold )
				sendChatMessageToClient ( "Text~Comp~Adjustments_Gold_Decreased", SERVER_INFO_MESSAGE, owner->Nr, iToStr(gold - SubBase->getGoldProd()) );
		}
	}

	// IsWorking is set to true before checking the energy production. So if an energy generator has to be started,
	// it can use the fuel production of this building (when this building is a mine).
	IsWorking = true;

	//set mine values. This has to be undone, if the energy is insufficient
	if ( data.is_mine )
	{
		int mineFree = MAX_MINE_PROD;

		SubBase->changeMetalProd( MaxMetalProd );
		mineFree -= MaxMetalProd;

		SubBase->changeGoldProd( min(MaxGoldProd, mineFree) );
		mineFree-= min ( MaxGoldProd, mineFree );

		SubBase->changeOilProd( min( MaxOilProd, mineFree) );
	}

	// Energy consumers:
	if ( data.energy_need )
	{
		if ( data.energy_need + SubBase->EnergyNeed > SubBase->EnergyProd )
		{
			//try to increase energy production
			if ( !SubBase->increaseEnergyProd( data.energy_need + SubBase->EnergyNeed - SubBase->EnergyProd ) )
			{
				IsWorking = false;

				//reset mine values
				if ( data.is_mine )
				{
					int metal = SubBase->getMetalProd();
					int oil =  SubBase->getOilProd();
					int gold = SubBase->getGoldProd();

					SubBase->setMetalProd(0);
					SubBase->setOilProd(0);
					SubBase->setGoldProd(0);

					SubBase->setMetalProd( min(metal, SubBase->getMaxAllowedMetalProd()) );
					SubBase->setGoldProd( min(gold, SubBase->getMaxAllowedGoldProd()) );
					SubBase->setOilProd( min(oil, SubBase->getMaxAllowedOilProd()) );
				}

				sendChatMessageToClient ( "Text~Comp~Energy_Insufficient", SERVER_ERROR_MESSAGE, owner->Nr );
				return;
			}
			sendChatMessageToClient ( "Text~Comp~Energy_ToLow", SERVER_INFO_MESSAGE, owner->Nr );
		}
	}

	//-- everything is ready to start the building

	SubBase->EnergyProd += data.energy_prod;
	SubBase->EnergyNeed += data.energy_need;

	SubBase->HumanNeed += data.human_need;
	SubBase->HumanProd += data.human_prod;

	SubBase->OilNeed += data.oil_need;

	// raw material consumer:
	if ( data.metal_need )
		SubBase->MetalNeed += min(MetalPerRound, (*BuildList)[0]->metall_remaining);

	// gold consumer:
	SubBase->GoldNeed += data.gold_need;

	// research building
	if ( data.can_research )
	{
		owner->ResearchCount++;
		owner->researchCentersWorkingOnArea[researchArea]++;
	}
	
	
	sendSubbaseValues(SubBase, owner->Nr);
	sendDoStartWork(this);
}

//------------------------------------------------------------
/** starts the building in the client thread */
//------------------------------------------------------------
void cBuilding::ClientStartWork()
{
	if (IsWorking) 
		return;
	IsWorking = true;
	EffectAlpha = 0;
	if (selected)
	{
		StopFXLoop (Client->iObjectStream);
		PlayFX (typ->Start);
		Client->iObjectStream = playStream ();
		ShowDetails();
	}
	if (data.can_research) 
		owner->startAResearch (researchArea);
}

//--------------------------------------------------------------------------
/** Stops the building's working in the server thread */
//--------------------------------------------------------------------------
void cBuilding::ServerStopWork ( bool override )
{
	if ( !IsWorking )
	{
		sendDoStopWork(this);
		return;
	}

	// energy generators
	if ( data.energy_prod )
	{
		if ( SubBase->EnergyNeed > SubBase->EnergyProd - data.energy_prod && !override )
		{
			sendChatMessageToClient ( "Text~Comp~Energy_IsNeeded", SERVER_ERROR_MESSAGE, owner->Nr );
			return;
		}

		SubBase->EnergyProd -= data.energy_prod;
		SubBase->OilNeed -= data.oil_need;
	}

	IsWorking = false;

	// Energy consumers:
	if ( data.energy_need )
		SubBase->EnergyNeed -= data.energy_need;

	// raw material consumer:
	if ( data.metal_need )
		SubBase->MetalNeed -= min(MetalPerRound, (*BuildList)[0]->metall_remaining);

	// gold consumer
	if ( data.gold_need )
		SubBase->GoldNeed -= data.gold_need;

	// human consumer
	if ( data.human_need )
		SubBase->HumanNeed -= data.human_need;

	// Minen:
	if ( data.is_mine )
	{
		int metal = SubBase->getMetalProd();
		int oil =  SubBase->getOilProd();
		int gold = SubBase->getGoldProd();

		SubBase->setMetalProd(0);
		SubBase->setOilProd(0);
		SubBase->setGoldProd(0);

		SubBase->setMetalProd( min(metal, SubBase->getMaxAllowedMetalProd()) );
		SubBase->setGoldProd( min(gold, SubBase->getMaxAllowedGoldProd()) );
		SubBase->setOilProd( min(oil, SubBase->getMaxAllowedOilProd()) );

	}
	
	if ( data.can_research )
	{
		owner->ResearchCount--;
		owner->researchCentersWorkingOnArea[researchArea]--;
	}
	
	sendSubbaseValues(SubBase, owner->Nr);
	sendDoStopWork(this);
}

//------------------------------------------------------------
/** stops the building in the client thread */
//------------------------------------------------------------
void cBuilding::ClientStopWork()
{
	if (!IsWorking) 
		return;
	IsWorking = false;
	if (selected)
	{
		StopFXLoop (Client->iObjectStream);
		PlayFX (typ->Stop);
		Client->iObjectStream = playStream ();
		ShowDetails ();
	}
	if (data.can_research) 
		owner->stopAResearch (researchArea);
}

//------------------------------------------------------------
bool cBuilding::CanTransferTo ( cMapField *OverUnitField )
{
	cBuilding *b;
	cVehicle *v;
	int x, y;
	mouse->GetKachel ( &x, &y );

	if ( OverUnitField->getVehicles() )
	{
		v = OverUnitField->getVehicles();

		if ( v->owner != Client->ActivePlayer )
			return false;

		if ( v->data.can_transport != data.can_load )
			return false;

		if ( v->IsBuilding || v->IsClearing )
			return false;

		for (unsigned int i = 0; i < SubBase->buildings.Size(); i++)
		{
			b = SubBase->buildings[i];

			if ( b->data.is_big )
			{
				if ( x < b->PosX - 1 || x > b->PosX + 2 || y < b->PosY - 1 || y > b->PosY + 2 )
					continue;
			}
			else
			{
				if ( x < b->PosX - 1 || x > b->PosX + 1 || y < b->PosY - 1 || y > b->PosY + 1 )
					continue;
			}

			return true;
		}

		return false;
	}
	else
		if ( OverUnitField->getTopBuilding() )
		{
			b = OverUnitField->getTopBuilding();

			if ( b == this )
				return false;

			if ( b->SubBase != SubBase )
				return false;

			if ( b->owner != Client->ActivePlayer )
				return false;

			if ( data.can_load != b->data.can_load )
				return false;

			return true;
		}

	return false;
}

//--------------------------------------------------------------------------
bool cBuilding::isNextTo( int x, int y) const
{
	if ( x + 1 < PosX || y + 1 < PosY ) return false;

	if ( data.is_big )
	{
		if ( x - 2 > PosX || y - 2 > PosY ) return false;
	}
	else
	{
		if ( x - 1 > PosX || y - 1 > PosY ) return false;
	}

	return true;
}


//--------------------------------------------------------------------------
/** draws the exit points for a vehicle of the given type: */
//--------------------------------------------------------------------------
void cBuilding::DrawExitPoints ( sVehicle *typ )
{
	int spx, spy, size;
	spx = GetScreenPosX();
	spy = GetScreenPosY();
	size = Client->Map->size;

	if ( canExitTo ( PosX - 1, PosY - 1, Client->Map, typ ) )
		Client->drawExitPoint ( spx - Client->Hud.Zoom, spy - Client->Hud.Zoom );

	if ( canExitTo ( PosX    , PosY - 1, Client->Map, typ ) )
		Client->drawExitPoint ( spx, spy - Client->Hud.Zoom );

	if ( canExitTo ( PosX + 1, PosY - 1, Client->Map, typ ) )
		Client->drawExitPoint ( spx + Client->Hud.Zoom, spy - Client->Hud.Zoom );

	if ( canExitTo ( PosX + 2, PosY - 1, Client->Map, typ ) )
		Client->drawExitPoint ( spx + Client->Hud.Zoom*2, spy - Client->Hud.Zoom );

	if ( canExitTo ( PosX - 1, PosY    , Client->Map, typ ) )
		Client->drawExitPoint ( spx - Client->Hud.Zoom, spy );

	if ( canExitTo ( PosX + 2, PosY    , Client->Map, typ ) )
		Client->drawExitPoint ( spx + Client->Hud.Zoom*2, spy );

	if ( canExitTo ( PosX - 1, PosY + 1, Client->Map, typ ) )
		Client->drawExitPoint ( spx - Client->Hud.Zoom, spy + Client->Hud.Zoom );

	if ( canExitTo ( PosX + 2, PosY + 1, Client->Map, typ ) )
		Client->drawExitPoint ( spx + Client->Hud.Zoom*2, spy + Client->Hud.Zoom );

	if ( canExitTo ( PosX - 1, PosY + 2, Client->Map, typ ) )
		Client->drawExitPoint ( spx - Client->Hud.Zoom, spy + Client->Hud.Zoom*2 );

	if ( canExitTo ( PosX    , PosY + 2, Client->Map, typ ) )
		Client->drawExitPoint ( spx, spy + Client->Hud.Zoom*2 );

	if ( canExitTo ( PosX + 1, PosY + 2, Client->Map, typ ) )
		Client->drawExitPoint ( spx + Client->Hud.Zoom, spy + Client->Hud.Zoom*2 );

	if ( canExitTo ( PosX + 2, PosY + 2, Client->Map, typ ) )
		Client->drawExitPoint ( spx + Client->Hud.Zoom*2, spy + Client->Hud.Zoom*2 );
}

//--------------------------------------------------------------------------
bool cBuilding::canExitTo ( const int x, const int y, const cMap* map, const sVehicle *typ ) const
{
	if ( !map->possiblePlaceVehicle( typ->data, x, y ) ) return false;
	if ( !isNextTo(x, y) ) return false;

	return true;
}

//--------------------------------------------------------------------------
bool cBuilding::canLoad ( int offset, cMap *Map )
{
	if ( offset < 0 || offset > Map->size*Map->size ) return false;

	if ( data.can_load == TRANS_AIR ) return canLoad ( Map->fields[offset].getPlanes() );
	if ( data.can_load != TRANS_AIR ) return canLoad ( Map->fields[offset].getVehicles() );

	return false;
}

//--------------------------------------------------------------------------
/** returns, if the vehicle can be loaded from its position: */
//--------------------------------------------------------------------------
bool cBuilding::canLoad ( cVehicle *Vehicle )
{
	if ( !Vehicle ) return false;

	if ( Vehicle->Loaded ) return false;

	if ( data.cargo == data.max_cargo ) return false;

	if ( !isNextTo ( Vehicle->PosX, Vehicle->PosY ) ) return false;

	if ( data.can_load == TRANS_MEN && !Vehicle->data.is_human ) return false;

	if ( data.can_load != TRANS_MEN && Vehicle->data.is_human ) return false;

	if ( !( data.build_on_water ? ( Vehicle->data.can_drive == DRIVE_SEA ) : ( Vehicle->data.can_drive != DRIVE_SEA ) ) ) return false;

	if ( Vehicle->ClientMoveJob && ( Vehicle->moving || Vehicle->Attacking || Vehicle->MoveJobActive ) ) return false;

	if ( Vehicle->owner == owner && !Vehicle->IsBuilding && !Vehicle->IsClearing ) return true;

	return false;
}

//--------------------------------------------------------------------------
/** loads a vehicle: */
//--------------------------------------------------------------------------
void cBuilding::storeVehicle( cVehicle *Vehicle, cMap *Map  )
{
	Map->deleteVehicle ( Vehicle );
	if ( Vehicle->bSentryStatus )
	{
		Vehicle->owner->deleteSentryVehicle( Vehicle );
		Vehicle->bSentryStatus = false;
	}

	Vehicle->Loaded = true;

	StoredVehicles.Add ( Vehicle );
	data.cargo++;

	if ( data.cargo == data.max_cargo ) LoadActive = false;

	owner->DoScan();
}

//-----------------------------------------------------------------------
// Unloads a vehicle
void cBuilding::exitVehicleTo( cVehicle *Vehicle, int offset, cMap *Map )
{
	for ( unsigned int i = 0; i < StoredVehicles.Size(); i++ )
	{
		if ( StoredVehicles[i] == Vehicle )
		{
			StoredVehicles.Delete ( i );
			break;
		}
		if ( i == StoredVehicles.Size()-1 ) return;
	}

	data.cargo--;

	ActivatingVehicle = false;

	ActivatingVehicle = false;

	Map->addVehicle ( Vehicle, offset );

	Vehicle->PosX = offset % Map->size;
	Vehicle->PosY = offset / Map->size;
	Vehicle->Loaded = false;

	owner->DoScan();
}

//-------------------------------------------------------------------------------
// Draws big symbols for the info menu:
//-------------------------------------------------------------------------------
void cBuilding::DrawSymbolBig ( eSymbolsBig sym, int x, int y, int maxx, int value, int orgvalue, SDL_Surface *sf)
{
	SDL_Rect src, dest;
	int i, offx;

	switch ( sym )
	{

		case SBSpeed:
			src.x = 0;
			src.y = 109;
			src.w = 11;
			src.h = 12;
			break;

		case SBHits:
			src.x = 11;
			src.y = 109;
			src.w = 7;
			src.h = 11;
			break;

		case SBAmmo:
			src.x = 18;
			src.y = 109;
			src.w = 9;
			src.h = 14;
			break;

		case SBAttack:
			src.x = 27;
			src.y = 109;
			src.w = 10;
			src.h = 14;
			break;

		case SBShots:
			src.x = 37;
			src.y = 109;
			src.w = 15;
			src.h = 7;
			break;

		case SBRange:
			src.x = 52;
			src.y = 109;
			src.w = 13;
			src.h = 13;
			break;

		case SBArmor:
			src.x = 65;
			src.y = 109;
			src.w = 11;
			src.h = 14;
			break;

		case SBScan:
			src.x = 76;
			src.y = 109;
			src.w = 13;
			src.h = 13;
			break;

		case SBMetal:
			src.x = 89;
			src.y = 109;
			src.w = 12;
			src.h = 15;
			break;

		case SBOil:
			src.x = 101;
			src.y = 109;
			src.w = 11;
			src.h = 12;
			break;

		case SBGold:
			src.x = 112;
			src.y = 109;
			src.w = 13;
			src.h = 10;
			break;

		case SBEnergy:
			src.x = 125;
			src.y = 109;
			src.w = 13;
			src.h = 17;
			break;

		case SBHuman:
			src.x = 138;
			src.y = 109;
			src.w = 12;
			src.h = 16;
			break;
	}

	maxx -= src.w;

	if ( orgvalue < value )
	{
		maxx -= src.w + 3;
	}

	offx = src.w;

	while ( offx*value >= maxx )
	{
		offx--;

		if ( offx < 4 )
		{
			value /= 2;
			orgvalue /= 2;
			offx = src.w;
		}
	}

	dest.x = x;

	dest.y = y;
	
	for ( i = 0;i < value;i++ )
	{
		if ( i == orgvalue )
		{
			SDL_Rect mark;
			dest.x += src.w + 3;
			mark.x = dest.x - src.w / 2;
			mark.y = dest.y;
			mark.w = 1;
			mark.h = src.h;
			SDL_FillRect ( sf, &mark, 0xFC0000 );
		}

		SDL_BlitSurface ( GraphicsData.gfx_hud_stuff, &src, sf, &dest );

		dest.x += offx;
	}
}

//-------------------------------------------------------------------------------
/** checks the resources that are available under the mining station */
//--------------------------------------------------------------------------
void cBuilding::CheckRessourceProd ( void )
{
	int pos = PosX + PosY * Server->Map->size;

	MaxMetalProd = 0;
	MaxGoldProd = 0;
	MaxOilProd = 0;

	if ( Server->Map->Resources[pos].typ == RES_METAL )
	{
		MaxMetalProd += Server->Map->Resources[pos].value;
	}
	else if ( Server->Map->Resources[pos].typ == RES_OIL )
	{
		MaxOilProd += Server->Map->Resources[pos].value;
	}
	else if ( Server->Map->Resources[pos].typ == RES_GOLD )
	{
		MaxGoldProd += Server->Map->Resources[pos].value;
	}

	pos++;

	if ( Server->Map->Resources[pos].typ == RES_METAL )
	{
		MaxMetalProd += Server->Map->Resources[pos].value;
	}
	else if ( Server->Map->Resources[pos].typ == RES_OIL )
	{
		MaxOilProd += Server->Map->Resources[pos].value;
	}
	else if ( Server->Map->Resources[pos].typ == RES_GOLD )
	{
		MaxGoldProd += Server->Map->Resources[pos].value;
	}

	pos += Server->Map->size;

	if ( Server->Map->Resources[pos].typ == RES_METAL )
	{
		MaxMetalProd += Server->Map->Resources[pos].value;
	}
	else if ( Server->Map->Resources[pos].typ == RES_OIL )
	{
		MaxOilProd += Server->Map->Resources[pos].value;
	}
	else if ( Server->Map->Resources[pos].typ == RES_GOLD )
	{
		MaxGoldProd += Server->Map->Resources[pos].value;
	}

	pos--;

	if ( Server->Map->Resources[pos].typ == RES_METAL )
	{
		MaxMetalProd += Server->Map->Resources[pos].value;
	}
	else if ( Server->Map->Resources[pos].typ == RES_OIL )
	{
		MaxOilProd += Server->Map->Resources[pos].value;
	}
	else if ( Server->Map->Resources[pos].typ == RES_GOLD )
	{
		MaxGoldProd += Server->Map->Resources[pos].value;
	}

	MaxMetalProd = min(MaxMetalProd, MAX_MINE_PROD);
	MaxGoldProd  = min(MaxGoldProd,  MAX_MINE_PROD);
	MaxOilProd   = min(MaxOilProd,   MAX_MINE_PROD);
}

//--------------------------------------------------------------------------
/** Checks if the target is in range */
//--------------------------------------------------------------------------
bool cBuilding::IsInRange ( int off, cMap *Map )
{
	int x, y;
	x = off % Map->size;
	y = off / Map->size;
	x -= PosX;
	y -= PosY;

	if ( sqrt ( ( double ) ( x*x + y*y ) ) <= data.range )
	{
		return true;
	}

	return false;
}

//--------------------------------------------------------------------------
/** Checks if the building is able to attack the object */
//--------------------------------------------------------------------------
bool cBuilding::CanAttackObject ( int off, cMap *Map, bool override )
{
	cVehicle *v = NULL;
	cBuilding *b = NULL;

	if ( !data.can_attack )
		return false;

	if ( !data.shots )
		return false;

	if ( !data.ammo )
		return false;

	if ( Attacking )
		return false;

	if ( bIsBeeingAttacked )
		return false;

	if ( off < 0 )
		return false;

	if ( !IsInRange ( off, Map ) )
		return false;

	if ( !owner->ScanMap[off] )
		return override?true:false;

	if ( override )
		return true;

	selectTarget(v, b, off, data.can_attack, Map );

	if ( v )
	{
		if ( Client && ( v == Client->SelectedVehicle || v->owner == Client->ActivePlayer ) )
			return false;
	}
	else if ( b )
	{
		if ( Client && ( b == Client->SelectedBuilding || b->owner == Client->ActivePlayer ) )
			return false;
	}
	else
		return false;

	return true;
}

//--------------------------------------------------------------------------
/** Draw the attack cursor */
//--------------------------------------------------------------------------
void cBuilding::DrawAttackCursor ( int offset )
{
	SDL_Rect r;
	int wp, wc, t = 0;
	cVehicle *v;
	cBuilding *b;

	selectTarget(v, b, offset, data.can_attack, Client->Map );

	if ( !(v || b) || ( v && v == Client->SelectedVehicle ) || ( b && b == Client->SelectedBuilding ) )
	{
		r.x = 1;
		r.y = 29;
		r.h = 3;
		r.w = 35;
		SDL_FillRect ( GraphicsData.gfx_Cattack, &r, 0 );
		return;
	}

	if ( v )
		t = v->data.hit_points;
	else if ( b )
		t = b->data.hit_points;

	if ( t )
	{
		if ( v )
			wc = ( int ) ( ( float ) t / v->data.max_hit_points * 35 );
		else  if ( b )
			wc = ( int ) ( ( float ) t / b->data.max_hit_points * 35 );
	}
	else
	{
		wc = 0;
	}

	if ( v )
		t = v->CalcHelth ( data.damage );
	else  if ( b )
		t = b->CalcHelth ( data.damage );

	if ( t )
	{
		if ( v )
			wp = ( int ) ( ( float ) t / v->data.max_hit_points * 35 );
		else  if ( b )
			wp = ( int ) ( ( float ) t / b->data.max_hit_points * 35 );
	}
	else
	{
		wp = 0;
	}

	r.x = 1;

	r.y = 29;
	r.h = 3;
	r.w = wp;

	if ( r.w )
		SDL_FillRect ( GraphicsData.gfx_Cattack, &r, 0x00FF00 );

	r.x += r.w;

	r.w = wc - wp;

	if ( r.w )
		SDL_FillRect ( GraphicsData.gfx_Cattack, &r, 0xFF0000 );

	r.x += r.w;

	r.w = 35 - wc;

	if ( r.w )
		SDL_FillRect ( GraphicsData.gfx_Cattack, &r, 0 );
}

//--------------------------------------------------------------------------
/** Rotates the building in the given direction */
//--------------------------------------------------------------------------
void cBuilding::RotateTo ( int Dir )
{
	int i, t, dest;

	if ( dir == Dir )
		return;

	t = dir;

	for ( i = 0;i < 8;i++ )
	{
		if ( t == Dir )
		{
			dest = i;
			break;
		}

		t++;

		if ( t > 7 )
			t = 0;
	}

	if ( dest < 4 )
		dir++;
	else
		dir--;

	if ( dir < 0 )
		dir += 8;
	else
		if ( dir > 7 )
			dir -= 8;
}

//--------------------------------------------------------------------------
/** calculates the costs and the duration of the 3 buildspeeds for the vehicle with the given id
	iRemainingMetal is only needed for recalculating costs of vehicles in the Buildqueue and is set per default to -1 */
//--------------------------------------------------------------------------
void cBuilding::CalcTurboBuild ( int *iTurboBuildRounds, int *iTurboBuildCosts, int iVehicleCosts, int iRemainingMetal )
{
	// calculate building time and costs

	//prevent division by zero
	if ( data.iNeeds_Metal == 0 ) data.iNeeds_Metal = 1;

	//step 1x
	iTurboBuildCosts[0] = iRemainingMetal;
	if ( iTurboBuildCosts[0] <= 0 ) iTurboBuildCosts[0] = iVehicleCosts;
	iTurboBuildRounds[0] = ( int ) ceil ( iTurboBuildCosts[0] / ( double ) ( data.iNeeds_Metal ) );

	iTurboBuildRounds[1] = 0;
	iTurboBuildRounds[2] = 0;

	if ( data.can_build != BUILD_MAN )
	{
		//step 2x
		if ( iTurboBuildRounds[0] > 1 )
		{
			iTurboBuildRounds[1] = iTurboBuildRounds[0];
			iTurboBuildCosts[1] = iTurboBuildCosts[0];

			while ( ( iTurboBuildRounds[1] > 1 ) && ( (iTurboBuildRounds[1]-1)*2 >= iTurboBuildRounds[0] ) )
			{
				iTurboBuildRounds[1]--;
				iTurboBuildCosts[1] += 2 * data.iNeeds_Metal;
			}
		}

		//step 4x
		if ( ( iTurboBuildRounds[1] > 1 ) )
		{
			iTurboBuildRounds[2] = iTurboBuildRounds[1];
			iTurboBuildCosts[2] = iTurboBuildCosts[1];

			while ( ( iTurboBuildRounds[2] > 1 ) && ( (iTurboBuildRounds[2]-1)*2 >= iTurboBuildRounds[1] ) )
			{
				iTurboBuildRounds[2]--;
				iTurboBuildCosts[2] += 4 * data.iNeeds_Metal;

			}
			if ( (iTurboBuildRounds[1] - iTurboBuildRounds[2]) * ( data.iNeeds_Metal ) * 12 > iTurboBuildCosts[2] )
				iTurboBuildCosts[2] = (iTurboBuildRounds[1] - iTurboBuildRounds[2]) * ( data.iNeeds_Metal ) * 12;
		}
	}
}

//--------------------------------------------------------------------------
/** Returns the screen x position of the building */
//--------------------------------------------------------------------------
int cBuilding::GetScreenPosX () const
{
	return 180 - ( ( int ) ( ( Client->Hud.OffX ) / ( 64.0 / Client->Hud.Zoom ) ) ) + Client->Hud.Zoom*PosX;
}

//--------------------------------------------------------------------------
/** Returns the screen x position of the building */
//--------------------------------------------------------------------------
int cBuilding::GetScreenPosY () const
{
	return 18 - ( ( int ) ( ( Client->Hud.OffY ) / ( 64.0 / Client->Hud.Zoom ) ) ) + Client->Hud.Zoom*PosY;
}

//--------------------------------------------------------------------------
/** returns the remaining hitpoints after an attack */
//--------------------------------------------------------------------------
int cBuilding::CalcHelth ( int damage )
{
	damage -= data.armor;

	if ( damage <= 0 )
	{
		//minimum damage is 1
		damage = 1;
	}

	int hp;
	hp = data.hit_points - damage;

	if ( hp < 0 )
	{
		return 0;
	}

	return hp;
}

//--------------------------------------------------------------------------
/** draws the building menu */
//--------------------------------------------------------------------------
void cBuilding::DrawMenu ( sMouseState *mouseState )
{
	int nr = 0, SelMenu = -1, ExeNr = -1;
	static int LastNr = -1;
	bool bSelection = false;
	SDL_Rect dest;
	dest = GetMenuSize();

	if ( bIsBeeingAttacked ) return;

	if ( ActivatingVehicle )
	{
		MenuActive = false;
		return;
	}

	if (BuildList && BuildList->Size() && !IsWorking && (*BuildList)[0]->metall_remaining <= 0)
		return;

	if ( mouseState && mouseState->leftButtonReleased && MouseOverMenu ( mouse->x, mouse->y ) )
	{
		SelMenu = ( mouse->y - dest.y ) / 22;
		LastNr = SelMenu;
	}
	else
		if ( MouseOverMenu ( mouse->x, mouse->y ) )
		{
			ExeNr = LastNr;
			LastNr = -1;
		}
		else
		{
			SelMenu = -1;
			LastNr = -1;
		}

	// Angriff:
	if ( typ->data.can_attack && data.shots )
	{
		if ( SelMenu == nr ) { bSelection = true; }
		else { bSelection = false; }


		if ( ExeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			AttackMode = true;
			Client->Hud.CheckScroll();
			Client->mouseMoveCallback ( true );
			return;
		}

		drawContextItem( lngPack.i18n ( "Text~Context~Attack" ), bSelection, dest.x, dest.y, buffer );

		dest.y += 22;
		nr++;
	}

	// Bauen:
	if ( typ->data.can_build )
	{
		if ( SelMenu == nr ) { bSelection = true; }
		else { bSelection = false; }

		if ( ExeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			cVehiclesBuildMenu buildMenu ( owner, this );
			buildMenu.show();
			return;
		}

		drawContextItem( lngPack.i18n ( "Text~Context~Build" ), bSelection, dest.x, dest.y, buffer );

		dest.y += 22;
		nr++;
	}

	// Verteilen:
	if ( typ->data.is_mine && IsWorking )
	{
		if ( SelMenu == nr ) { bSelection = true; }
		else { bSelection = false; }

		if ( ExeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			cMineManagerMenu mineManager ( this );
			mineManager.show();
			return;
		}

		drawContextItem( lngPack.i18n ( "Text~Context~Dist" ), bSelection, dest.x, dest.y, buffer );

		dest.y += 22;
		nr++;
	}

	// Transfer:
	if ( typ->data.can_load == TRANS_METAL || typ->data.can_load == TRANS_OIL || typ->data.can_load == TRANS_GOLD )
	{
		if ( SelMenu == nr ) { bSelection = true; }
		else { bSelection = false; }

		if ( ExeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			Transfer = true;
			return;
		}

		drawContextItem( lngPack.i18n ( "Text~Context~Transfer" ), bSelection, dest.x, dest.y, buffer );

		dest.y += 22;
		nr++;
	}

	// Start:
	if (typ->data.can_work &&
			!IsWorking         &&
			(
				(BuildList && BuildList->Size()) ||
				typ->data.can_build == BUILD_NONE
			))
	{
		if ( SelMenu == nr ) { bSelection = true; }
		else { bSelection = false; }

		if ( ExeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			sendWantStartWork(this);
			return;
		}

		drawContextItem( lngPack.i18n ( "Text~Context~Start" ), bSelection, dest.x, dest.y, buffer );

		dest.y += 22;
		nr++;
	}

	// Stop:
	if ( IsWorking )
	{
		if ( SelMenu == nr ) { bSelection = true; }
		else { bSelection = false; }

		if ( ExeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			sendWantStopWork(this);
			return;
		}

		drawContextItem( lngPack.i18n ( "Text~Context~Stop" ), bSelection, dest.x, dest.y, buffer );

		dest.y += 22;
		nr++;
	}

	// Sentry status:
	if ( bSentryStatus || data.can_attack )
	{
		bSelection = SelMenu == nr || bSentryStatus;

		if ( ExeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			sendChangeSentry ( iID, false );
			return;
		}

		drawContextItem( lngPack.i18n ( "Text~Context~Sentry" ), bSelection, dest.x, dest.y, buffer );

		dest.y += 22;
		nr++;
	}

	// Aktivieren/Laden:
	if ( typ->data.can_load == TRANS_VEHICLES || typ->data.can_load == TRANS_MEN || typ->data.can_load == TRANS_AIR )
	{
		// Aktivieren:
		if ( SelMenu == nr ) bSelection = true;
		else bSelection = false;

		if ( ExeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			cStorageMenu storageMenu ( StoredVehicles, NULL, this );
			storageMenu.show();
			return;
		}

		drawContextItem( lngPack.i18n ( "Text~Context~Active" ), bSelection, dest.x, dest.y, buffer );

		dest.y += 22;
		nr++;
		// Laden:

		if ( SelMenu == nr || LoadActive ) bSelection = true;
		else bSelection = false;

		if ( ExeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			LoadActive = !LoadActive;
			return;
		}

		drawContextItem( lngPack.i18n ( "Text~Context~Load" ), bSelection, dest.x, dest.y, buffer );

		dest.y += 22;
		nr++;
	}

	// research
	if (typ->data.can_research && IsWorking)
	{
		bSelection = (SelMenu == nr);
		if (ExeNr == nr)
		{
			MenuActive = false;
			PlayFX (SoundData.SNDObjectMenu);
			cDialogResearch researchDialog ( owner );
			researchDialog.show();
			return;
		}
		drawContextItem (lngPack.i18n ("Text~Context~Research"), bSelection, dest.x, dest.y, buffer);
		dest.y += 22;
		nr++;
	}

	// upgradescreen
	if (data.gold_need)
	{
		// update this
		bSelection = (SelMenu == nr);
		if (ExeNr == nr)
		{
			MenuActive = false;
			PlayFX (SoundData.SNDObjectMenu);
			cUpgradeMenu upgradeMenu ( owner );
			upgradeMenu.show();
			return;
		}
		drawContextItem (lngPack.i18n ("Text~Context~Upgrades"), bSelection, dest.x, dest.y, buffer);
		dest.y += 22;
		nr++;
	}

	// Updates:
	if ( data.version != owner->BuildingData[typ->nr].version && SubBase && SubBase->Metal >= 2 )
	{
		// Update all buildings of this type in this subbase
		bSelection = (SelMenu == nr);
		if (ExeNr == nr)
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			sendUpgradeBuilding (this, true);
			return;
		}
		drawContextItem (lngPack.i18n ("Text~Context~UpAll"), bSelection, dest.x, dest.y, buffer);
		dest.y += 22;
		nr++;

		// update this building
		bSelection = (SelMenu == nr);
		if (ExeNr == nr)
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			sendUpgradeBuilding (this, false);
			return;
		}
		drawContextItem (lngPack.i18n ("Text~Context~Upgrade"), bSelection, dest.x, dest.y, buffer);
		dest.y += 22;
		nr++;
	}

	// Self destruct
	if (!data.is_road)
	{
		bSelection = (SelMenu == nr);
		if (ExeNr == nr)
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			//TODO: implement self destruction
			Client->addMessage ( lngPack.i18n ( "Text~Error_Messages~INFO_Not_Implemented" ) );
			//SelfDestructionMenu();
			return;
		}
		drawContextItem (lngPack.i18n ("Text~Context~Destroy"), bSelection, dest.x, dest.y, buffer);
		dest.y += 22;
		nr++;
	}

	// Info:
	bSelection = (SelMenu == nr);
	if (ExeNr == nr)
	{
		MenuActive = false;
		PlayFX (SoundData.SNDObjectMenu);
		cUnitHelpMenu helpMenu ( &data, owner );
		helpMenu.show();
		return;
	}
	drawContextItem (lngPack.i18n ("Text~Context~Info"), bSelection, dest.x, dest.y, buffer);
	dest.y += 22;
	nr++;

	// Done:
	bSelection = (SelMenu == nr);
	if (ExeNr == nr)
	{
		MenuActive = false;
		PlayFX (SoundData.SNDObjectMenu);
		return;
	}
	drawContextItem (lngPack.i18n ("Text~Context~Done"), bSelection, dest.x, dest.y, buffer);
}

//------------------------------------------------------------------------
void cBuilding::sendUpgradeBuilding (cBuilding* building, bool upgradeAll)
{	
	if (building == 0 || building->owner == 0)
		return;
	
	sUnitData& currentVersion = building->data;
	sUnitData& upgradedVersion = building->owner->BuildingData[building->typ->nr];
	if (currentVersion.version >= upgradedVersion.version)
		return; // already uptodate

	cNetMessage* msg = new cNetMessage (GAME_EV_WANT_BUILDING_UPGRADE);
	msg->pushBool (upgradeAll);
	msg->pushInt32 (building->iID);
	
	Client->sendNetMessage (msg);	
}

//------------------------------------------------------------------------
void cBuilding::upgradeToCurrentVersion ()
{
	sUnitData& upgradeVersion = owner->BuildingData[typ->nr];
	data.version = upgradeVersion.version; // TODO: iVersion?
	
	if (data.hit_points == data.max_hit_points)
		data.hit_points = upgradeVersion.max_hit_points; // TODO: check behaviour in original
	data.max_hit_points = upgradeVersion.max_hit_points;

	data.max_ammo = upgradeVersion.max_ammo; // don't change the current ammo-amount!
	
	data.armor = upgradeVersion.armor;
	data.scan = upgradeVersion.scan;
	data.range = upgradeVersion.range;
	data.max_shots = upgradeVersion.max_shots; // TODO: check behaviour in original
	data.damage = upgradeVersion.damage;
	data.iBuilt_Costs = upgradeVersion.iBuilt_Costs;

	GenerateName();

	if (this == Client->SelectedBuilding)
		ShowDetails();
}


//------------------------------------------------------------------------
/** centers on this building */
//--------------------------------------------------------------------------
void cBuilding::Center ()
{
	Client->Hud.OffX = PosX * 64 - ( ( int ) ( ( ( float ) (SettingsData.iScreenW - 192) / (2 * Client->Hud.Zoom) ) * 64 ) ) + 32;
	Client->Hud.OffY = PosY * 64 - ( ( int ) ( ( ( float ) (SettingsData.iScreenH - 32 ) / (2 * Client->Hud.Zoom) ) * 64 ) ) + 32;
	Client->bFlagDrawMap = true;
	Client->Hud.DoScroll ( 0 );
}

//------------------------------------------------------------------------
/** draws the available ammunition over the building: */
//--------------------------------------------------------------------------
void cBuilding::DrawMunBar ( void ) const
{
	SDL_Rect r1, r2;
	r1.x = GetScreenPosX() + Client->Hud.Zoom/10 + 1;
	r1.w = Client->Hud.Zoom * 8 / 10 ;
	r1.h = Client->Hud.Zoom / 8;
	r1.y = GetScreenPosY() + Client->Hud.Zoom/10 + Client->Hud.Zoom / 8;

	if ( r1.h <= 2 )
	{
		r1.h = 3;
		r1.y += 1;
	}

	r2.x = r1.x + 1;
	r2.y = r1.y + 1;
	r2.h = r1.h - 2;
	r2.w = ( int ) ( ( ( float ) ( r1.w - 2 ) ) / data.max_ammo  * data.ammo );

	SDL_FillRect ( buffer, &r1, 0 );

	if ( data.ammo > data.max_ammo / 2 )
	{
		SDL_FillRect ( buffer, &r2, 0x04AE04 );
	}
	else if ( data.ammo > data.max_ammo / 4 )
	{
		SDL_FillRect ( buffer, &r2, 0xDBDE00 );
	}
	else
	{
		SDL_FillRect ( buffer, &r2, 0xE60000 );
	}
}

//------------------------------------------------------------------------
/** draws the health bar over the building */
//--------------------------------------------------------------------------
void cBuilding::DrawHelthBar ( void ) const
{
	SDL_Rect r1, r2;
	r1.x = GetScreenPosX() + Client->Hud.Zoom/10 + 1;
	r1.w = Client->Hud.Zoom * 8 / 10 ;
	r1.h = Client->Hud.Zoom / 8;
	r1.y = GetScreenPosY() + Client->Hud.Zoom/10;

	if ( data.is_big )
	{
		r1.w += Client->Hud.Zoom;
		r1.h *= 2;
	}

	if ( r1.h <= 2  )
		r1.h = 3;

	r2.x = r1.x + 1;
	r2.y = r1.y + 1;
	r2.h = r1.h - 2;
	r2.w = ( int ) ( ( ( float ) ( r1.w - 2 ) / data.max_hit_points ) * data.hit_points );

	SDL_FillRect ( buffer, &r1, 0 );

	if ( data.hit_points > data.max_hit_points / 2 )
	{
		SDL_FillRect ( buffer, &r2, 0x04AE04 );
	}
	else if ( data.hit_points > data.max_hit_points / 4 )
	{
		SDL_FillRect ( buffer, &r2, 0xDBDE00 );
	}
	else
	{
		SDL_FillRect ( buffer, &r2, 0xE60000 );
	}
}

//--------------------------------------------------------------------------
void cBuilding::drawStatus() const
{
	SDL_Rect dest;
	SDL_Rect shotsSymbol = {254, 97, 5, 10 };
	SDL_Rect disabledSymbol = {150, 109, 25, 25};

	if ( Disabled )
	{
		if ( Client->Hud.Zoom < 25 ) return;
		dest.x = GetScreenPosX() + Client->Hud.Zoom/2 - 12;
		dest.y = GetScreenPosY() + Client->Hud.Zoom/2 - 12;
		SDL_BlitSurface( GraphicsData.gfx_hud_stuff, &disabledSymbol, buffer, &dest );
	}
	else
	{
		dest.y = GetScreenPosY() + Client->Hud.Zoom - 11;
		dest.x = GetScreenPosX() + Client->Hud.Zoom/2 - 4;
		if ( data.shots )
		{
			SDL_BlitSurface( GraphicsData.gfx_hud_stuff, &shotsSymbol, buffer, &dest );
		}
	}
}

//--------------------------------------------------------------------------
void cBuilding::Select ()
{
	selected = true;
	// Das Video laden:
	if ( Client->FLC != NULL )
	{
		FLI_Close ( Client->FLC );
		Client->FLC = NULL;
		Client->sFLCname = "";
	}
	Client->video = typ->video;

	// Sound abspielen:
	if ( !IsWorking )
		PlayFX ( SoundData.SNDHudButton );

	// Die Eigenschaften anzeigen:
	ShowDetails();
}

//--------------------------------------------------------------------------
void cBuilding::Deselct ()
{
	SDL_Rect src, dest;
	selected = false;
	MenuActive = false;
	AttackMode = false;
	Transfer = false;
	LoadActive = false;
	ActivatingVehicle = false;
	// Den Hintergrund wiederherstellen:
	src.x = 0;
	src.y = 215;
	src.w = 155;
	src.h = 48;
	dest.x = 8;
	dest.y = 171;
	SDL_BlitSurface ( GraphicsData.gfx_hud_stuff, &src, GraphicsData.gfx_hud, &dest );
	StopFXLoop ( Client->iObjectStream );
	Client->iObjectStream = -1;
	Client->bFlagDrawHud = true;
}

//--------------------------------------------------------------------------
void cBuilding::ShowDetails ()
{
	SDL_Rect src, dest;
	// Den Hintergrund wiederherstellen:
	src.x = 0;
	src.y = 215;
	src.w = 155;
	src.h = 48;
	dest.x = 8;
	dest.y = 171;
	SDL_BlitSurface ( GraphicsData.gfx_hud_stuff, &src, GraphicsData.gfx_hud, &dest );
	// Die Hitpoints anzeigen:
	DrawNumber ( 31, 177, data.hit_points, data.max_hit_points, GraphicsData.gfx_hud );
	font->showText ( 55, 177, lngPack.i18n ( "Text~Hud~Hitpoints" ), FONT_LATIN_SMALL_WHITE, GraphicsData.gfx_hud );

	DrawSymbol ( SHits, 88, 174, 70, data.hit_points, data.max_hit_points, GraphicsData.gfx_hud );
	// additional values:

	if ( data.can_load && owner == Client->ActivePlayer )
	{
		// Load:
		DrawNumber ( 31, 189, data.cargo, data.max_cargo, GraphicsData.gfx_hud );
		font->showText ( 55, 189, lngPack.i18n ( "Text~Hud~Cargo" ), FONT_LATIN_SMALL_WHITE, GraphicsData.gfx_hud );

		switch ( data.can_load )
		{

			case TRANS_METAL:
				DrawSymbol ( SMetal, 88, 186, 70, data.cargo, data.max_cargo, GraphicsData.gfx_hud );
				break;

			case TRANS_OIL:
				DrawSymbol ( SOil, 88, 186, 70, data.cargo, data.max_cargo, GraphicsData.gfx_hud );
				break;

			case TRANS_GOLD:
				DrawSymbol ( SGold, 88, 187, 70, data.cargo, data.max_cargo, GraphicsData.gfx_hud );
				break;

			case TRANS_VEHICLES:
				DrawSymbol ( STrans, 88, 186, 70, data.cargo, data.max_cargo, GraphicsData.gfx_hud );
				break;

			case TRANS_MEN:
				DrawSymbol ( SHuman, 88, 186, 70, data.cargo, data.max_cargo, GraphicsData.gfx_hud );
				break;

			case TRANS_AIR:
				DrawSymbol ( SAir, 88, 186, 50, data.cargo, data.max_cargo, GraphicsData.gfx_hud );
				break;
		}

		// Gesamt:
		if ( data.can_load == TRANS_METAL || data.can_load == TRANS_OIL || data.can_load == TRANS_GOLD )
		{
			font->showText ( 55, 201, lngPack.i18n ( "Text~Hud~Total" ), FONT_LATIN_SMALL_WHITE, GraphicsData.gfx_hud );

			switch ( data.can_load )
			{

				case TRANS_METAL:
					DrawNumber ( 31, 201, SubBase->Metal, SubBase->MaxMetal, GraphicsData.gfx_hud );
					DrawSymbol ( SMetal, 88, 198, 70, SubBase->Metal, SubBase->MaxMetal, GraphicsData.gfx_hud );
					break;

				case TRANS_OIL:
					DrawNumber ( 31, 201, SubBase->Oil, SubBase->MaxOil, GraphicsData.gfx_hud );
					DrawSymbol ( SOil, 88, 198, 70, SubBase->Oil, SubBase->MaxOil, GraphicsData.gfx_hud );
					break;

				case TRANS_GOLD:
					DrawNumber ( 31, 201, SubBase->Gold, SubBase->MaxGold, GraphicsData.gfx_hud );
					DrawSymbol ( SGold, 88, 199, 70, SubBase->Gold, SubBase->MaxGold, GraphicsData.gfx_hud );
					break;
			}
		}
	}
	else if ( data.can_attack && !data.is_expl_mine )
	{
		if ( owner == Client->ActivePlayer )
		{
			// Munition:
			DrawNumber ( 31, 189, data.ammo, data.max_ammo, GraphicsData.gfx_hud );
			font->showText ( 55, 189, lngPack.i18n ( "Text~Hud~AmmoShort" ), FONT_LATIN_SMALL_WHITE, GraphicsData.gfx_hud );

			DrawSymbol ( SAmmo, 88, 187, 70, data.ammo, data.max_ammo, GraphicsData.gfx_hud );
		}

		// shots:
		DrawNumber ( 31, 212, data.shots, data.max_shots, GraphicsData.gfx_hud );

		font->showText ( 55, 212, lngPack.i18n ( "Text~Hud~Shots" ), FONT_LATIN_SMALL_WHITE, GraphicsData.gfx_hud );

		DrawSymbol ( SShots, 88, 212, 70, data.shots, data.max_shots, GraphicsData.gfx_hud );
	}
	else if ( data.energy_prod )
	{
		// EnergieProduktion:
		DrawNumber ( 31, 189, ( IsWorking ? data.energy_prod : 0 ), data.energy_prod, GraphicsData.gfx_hud );
		font->showText ( 55, 189, lngPack.i18n ( "Text~Hud~Energy" ), FONT_LATIN_SMALL_WHITE, GraphicsData.gfx_hud );

		DrawSymbol ( SEnergy, 88, 187, 70, ( IsWorking ? data.energy_prod : 0 ), data.energy_prod, GraphicsData.gfx_hud );

		if ( owner == Client->ActivePlayer )
		{
			// Gesammt:
			font->showText ( 55, 201, lngPack.i18n ( "Text~Hud~Total" ), FONT_LATIN_SMALL_WHITE, GraphicsData.gfx_hud );

			DrawNumber ( 31, 201, SubBase->EnergyProd, SubBase->MaxEnergyProd, GraphicsData.gfx_hud );
			DrawSymbol ( SEnergy, 88, 199, 70, SubBase->EnergyProd, SubBase->MaxEnergyProd, GraphicsData.gfx_hud );
			// Verbrauch:
			font->showText ( 55, 212, lngPack.i18n ( "Text~Hud~Usage" ), FONT_LATIN_SMALL_WHITE, GraphicsData.gfx_hud );

			DrawNumber ( 31, 212, SubBase->EnergyNeed, SubBase->MaxEnergyNeed, GraphicsData.gfx_hud );
			DrawSymbol ( SEnergy, 88, 212, 70, SubBase->EnergyNeed, SubBase->MaxEnergyNeed, GraphicsData.gfx_hud );
		}
	}
	else if ( data.human_prod )
	{
		// HumanProduktion:
		DrawNumber ( 31, 189, data.human_prod, data.human_prod, GraphicsData.gfx_hud );
		font->showText ( 55, 189, lngPack.i18n ( "Text~Hud~Teams" ), FONT_LATIN_SMALL_WHITE, GraphicsData.gfx_hud );

		DrawSymbol ( SHuman, 88, 187, 70, data.human_prod, data.human_prod, GraphicsData.gfx_hud );

		if ( owner == Client->ActivePlayer )
		{
			// Gesammt:
			font->showText ( 55, 201, lngPack.i18n ( "Text~Hud~Total" ), FONT_LATIN_SMALL_WHITE, GraphicsData.gfx_hud );

			DrawNumber ( 31, 201, SubBase->HumanProd, SubBase->HumanProd, GraphicsData.gfx_hud );
			DrawSymbol ( SHuman, 88, 199, 70, SubBase->HumanProd, SubBase->HumanProd, GraphicsData.gfx_hud );
			// Verbrauch:
			font->showText ( 55, 212, lngPack.i18n ( "Text~Hud~Usage" ), FONT_LATIN_SMALL_WHITE, GraphicsData.gfx_hud );

			DrawNumber ( 31, 212, SubBase->HumanNeed, SubBase->MaxHumanNeed, GraphicsData.gfx_hud );
			DrawSymbol ( SHuman, 88, 210, 70, SubBase->HumanNeed, SubBase->MaxHumanNeed, GraphicsData.gfx_hud );
		}
	}
	else if ( data.human_need )
	{
		// HumanNeed:
		if ( IsWorking )
		{
			DrawNumber ( 31, 189, data.human_need, data.human_need, GraphicsData.gfx_hud );
			font->showText ( 55, 189, lngPack.i18n ( "Text~Hud~Usage" ), FONT_LATIN_SMALL_WHITE, GraphicsData.gfx_hud );

			DrawSymbol ( SHuman, 88, 187, 70, data.human_need, data.human_need, GraphicsData.gfx_hud );
		}
		else
		{
			DrawNumber ( 31, 189, 0, data.human_need, GraphicsData.gfx_hud );
			font->showText ( 55, 189, lngPack.i18n ( "Text~Hud~Usage" ), FONT_LATIN_SMALL_WHITE, GraphicsData.gfx_hud );

			DrawSymbol ( SHuman, 88, 187, 70, 0, data.human_need, GraphicsData.gfx_hud );
		}

		if ( owner == Client->ActivePlayer )
		{
			// Gesammt:
			font->showText ( 55, 201, lngPack.i18n ( "Text~Hud~Total" ), FONT_LATIN_SMALL_WHITE, GraphicsData.gfx_hud );

			DrawNumber ( 31, 201, SubBase->HumanNeed, SubBase->MaxHumanNeed, GraphicsData.gfx_hud );
			DrawSymbol ( SHuman, 88, 199, 70, SubBase->HumanNeed, SubBase->MaxHumanNeed, GraphicsData.gfx_hud );
		}
	}
	Client->bFlagDrawHud = true;
}

//--------------------------------------------------------------------------
/** draws a row of symbols */
//--------------------------------------------------------------------------
void cBuilding::DrawSymbol ( eSymbols sym, int x, int y, int maxx, int value, int maxvalue, SDL_Surface *sf )
{
	SDL_Rect full, empty, dest;
	int i, to, step, offx;

	switch ( sym )
	{
		case SSpeed:
			full.x = 0;
			empty.y = full.y = 98;
			empty.w = full.w = 7;
			empty.h = full.h = 7;
			empty.x = 7;
			break;

		case SHits:
			empty.y = full.y = 98;
			empty.w = full.w = 6;
			empty.h = full.h = 9;

			if ( value > maxvalue / 2 )
			{
				full.x = 14;
				empty.x = 20;
			}
			else
				if ( value > maxvalue / 4 )
				{
					full.x = 26;
					empty.x = 32;
				}
				else
				{
					full.x = 38;
					empty.x = 44;
				}

			break;

		case SAmmo:
			full.x = 50;
			empty.y = full.y = 98;
			empty.w = full.w = 5;
			empty.h = full.h = 7;
			empty.x = 55;
			break;

		case SMetal:
			full.x = 60;
			empty.y = full.y = 98;
			empty.w = full.w = 7;
			empty.h = full.h = 10;
			empty.x = 67;
			break;

		case SEnergy:
			full.x = 74;
			empty.y = full.y = 98;
			empty.w = full.w = 7;
			empty.h = full.h = 7;
			empty.x = 81;
			break;

		case SShots:
			full.x = 88;
			empty.y = full.y = 98;
			empty.w = full.w = 8;
			empty.h = full.h = 4;
			empty.x = 96;
			break;

		case SOil:
			full.x = 104;
			empty.y = full.y = 98;
			empty.w = full.w = 8;
			empty.h = full.h = 9;
			empty.x = 112;
			break;

		case SGold:
			full.x = 120;
			empty.y = full.y = 98;
			empty.w = full.w = 9;
			empty.h = full.h = 8;
			empty.x = 129;
			break;

		case STrans:
			full.x = 138;
			empty.y = full.y = 98;
			empty.w = full.w = 16;
			empty.h = full.h = 8;
			empty.x = 154;
			break;

		case SHuman:
			full.x = 170;
			empty.y = full.y = 98;
			empty.w = full.w = 8;
			empty.h = full.h = 9;
			empty.x = 178;
			break;

		case SAir:
			full.x = 186;
			empty.y = full.y = 98;
			empty.w = full.w = 21;
			empty.h = full.h = 8;
			empty.x = 207;
			break;
	}

	to = maxvalue;

	step = 1;
	offx = full.w;

	while ( offx*to > maxx )
	{
		offx--;

		if ( offx < 4 )
		{
			to /= 2;
			step *= 2;
			offx = full.w;
		}
	}

	dest.x = x;
	dest.y = y;
	
	for ( i = 0;i < to;i++ )
	{
		if ( value > 0 )
			SDL_BlitSurface ( GraphicsData.gfx_hud_stuff, &full, sf, &dest );
		else
			SDL_BlitSurface ( GraphicsData.gfx_hud_stuff, &empty, sf, &dest );

		dest.x += offx;
		value -= step;
	}
}

//--------------------------------------------------------------------------
/** Draws a number/number on the surface */
//--------------------------------------------------------------------------
void cBuilding::DrawNumber ( int x, int y, int value, int maxvalue, SDL_Surface *sf )
{
	if ( value > maxvalue / 2 )
		font->showTextCentered ( x, y, iToStr ( value ) + "/" + iToStr ( maxvalue ), FONT_LATIN_SMALL_GREEN, sf );
	else if ( value > maxvalue / 4 )
		font->showTextCentered ( x, y, iToStr ( value ) + "/" + iToStr ( maxvalue ), FONT_LATIN_SMALL_YELLOW, sf );
	else
		font->showTextCentered ( x, y, iToStr ( value ) + "/" + iToStr ( maxvalue ), FONT_LATIN_SMALL_RED, sf );
}

//----------------------------------------------------------------
/** Playback of the soundstream that belongs to this building */
//----------------------------------------------------------------
int cBuilding::playStream ()
{
	if ( IsWorking )
		return PlayFXLoop ( typ->Running );

	return 0;
}

//--------------------------------------------------------------------------
bool cBuilding::isDetectedByPlayer( cPlayer* player )
{
	for (unsigned int i = 0; i < DetectedByPlayerList.Size(); i++)
	{
		if ( DetectedByPlayerList[i] == player ) return true;
	}
	return false;
}

//--------------------------------------------------------------------------
void cBuilding::setDetectedByPlayer( cPlayer* player )
{
	if (!isDetectedByPlayer( player ))
		DetectedByPlayerList.Add( player );
}

//--------------------------------------------------------------------------
void cBuilding::resetDetectedByPlayer( cPlayer* player )
{
	for ( unsigned int i = 0; i < DetectedByPlayerList.Size(); i++ )
	{
		if ( DetectedByPlayerList[i] == player ) DetectedByPlayerList.Delete(i);
	}
}

//--------------------------------------------------------------------------
void cBuilding::makeDetection()
{
	//check whether the building has been detected by others
	if ( !data.is_expl_mine ) return;

	int offset = PosX + PosY * Server->Map->size;
	for ( unsigned int i = 0; i < Server->PlayerList->Size(); i++ )
	{
		cPlayer* player = (*Server->PlayerList)[i];
		if ( player == owner ) continue;
		if ( player->DetectMinesMap[offset] )
		{
			setDetectedByPlayer( player );
		}
	}
}

//--------------------------------------------------------------------------
void sBuilding::scaleSurfaces( float factor )
{
	scaleSurface ( img_org, img, (int)(img_org->w*factor), (int)(img_org->h*factor) );
	scaleSurface ( shw_org, shw, (int)(shw_org->w*factor), (int)(shw_org->h*factor) );
	if ( eff_org ) scaleSurface ( eff_org, eff, (int)(eff_org->w*factor), (int)(eff_org->h*factor) );
}
