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
#include "settings.h"
#include "hud.h"

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
	points = 0;

	if ( Owner == NULL || b == NULL )
	{
		if ( b != NULL )
		{
			data = b->data;
		}

		BuildList = NULL;

		bSentryStatus = false;
		return;
	}

	data = owner->BuildingData[typ->nr];

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

	if ( data.canAttack != TERRAIN_NONE ) bSentryStatus = true;
	else bSentryStatus = false;

	MaxMetalProd = 0;
	MaxGoldProd = 0;
	MaxOilProd = 0;
	Disabled = 0;
	data.hitpointsCur = data.hitpointsMax;
	data.ammoCur = data.ammoMax;
	SubBase = NULL;
	BuildSpeed = 0;
	BuildList = NULL;

	if ( !data.canBuild.empty() )
		BuildList = new cList<sBuildList*>;

	if ( data.isBig )
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

	while( passiveEndMoveActions.Size() )
	{
		cEndMoveAction *endMoveAction = passiveEndMoveActions[0];
		passiveEndMoveActions.Delete ( 0 );
		delete endMoveAction;
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
		if (!data.canBuild.empty() && BuildList && BuildList->Size() && owner == Client->ActivePlayer)
		{
			sBuildList *ptr;
			ptr = (*BuildList)[0];

			if ( ptr->metall_remaining > 0 )
			{
				string sText;
				int iRound;

				iRound = ( int ) ceil ( ptr->metall_remaining / ( double ) MetalPerRound );
				sText = lngPack.i18n ( "Text~Comp~Producing" ) + ": ";
				sText += ( string ) owner->VehicleData[ptr->typ->nr].name + " (";
				sText += iToStr ( iRound ) + ")";

				if ( font->getTextWide ( sText, FONT_LATIN_SMALL_WHITE ) > 126 )
				{
					sText = lngPack.i18n ( "Text~Comp~Producing" ) + ":\n";
					sText += ( string ) owner->VehicleData[ptr->typ->nr].name + " (";
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
		if (data.canResearch && owner == Client->ActivePlayer)
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
		if ( data.convertsGold && owner == Client->ActivePlayer )
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
	if ( data.shotsCur < data.shotsMax )
	{
		if ( data.ammoCur >= data.shotsMax )
			data.shotsCur = data.shotsMax;
		else
			data.shotsCur = data.ammoCur;
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
	nr = data.version + 1;	// +1, because the numbers in the name start at 1, not at 0

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
		name += ( string ) data.name;
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
	float factor = (float)Client->gameGUI.getTileSize()/(float)64.0;

	// draw the damage effects
	if ( Client->timer100ms && data.hasDamageEffect && data.hitpointsCur < data.hitpointsMax && SettingsData.bDamageEffects && ( owner == Client->ActivePlayer || Client->ActivePlayer->ScanMap[PosX+PosY*Client->Map->size] ) )
	{
		int intense = ( int ) ( 200 - 200 * ( ( float ) data.hitpointsCur / data.hitpointsMax ) );
		Client->addFX ( fxDarkSmoke, PosX*64 + DamageFXPointX, PosY*64 + DamageFXPointY, intense );

		if ( data.isBig && intense > 50 )
		{
			intense -= 50;
			Client->addFX ( fxDarkSmoke, PosX*64 + DamageFXPointX2, PosY*64 + DamageFXPointY2, intense );
		}
	}

	dest.x = dest.y = 0;
	bool bDraw = false;
	SDL_Surface* drawingSurface = Client->gameGUI.getDCache()->getCachedImage(this);
	if ( drawingSurface == NULL )
	{
		//no cached image found. building needs to be redrawn.
		bDraw = true;
		drawingSurface = Client->gameGUI.getDCache()->createNewEntry(this);
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

		//all following graphic operations are drawn directly to buffer
		dest = *screenPos;
	}

	if (!owner ) return;

	if ( StartUp )
	{
		if ( Client->timer100ms )
			StartUp += 25;

		if ( StartUp >= 255 )
			StartUp = 0;
	}

	// draw the effect if necessary
	if ( data.powerOnGraphic && SettingsData.bAnimations && ( IsWorking || !data.canWork ) )
	{
		tmp = dest;
		SDL_SetAlpha ( typ->eff, SDL_SRCALPHA, EffectAlpha );

		CHECK_SCALING( typ->eff, typ->eff_org, factor);
		SDL_BlitSurface ( typ->eff, NULL, buffer, &tmp );

		if ( Client->timer100ms )
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
		nr = 0xFF00 - ( ( ANIMATION_SPEED % 0x8 ) * 0x1000 );
		max = (int)( Client->gameGUI.getTileSize() - 2 ) * 2;
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
	if ( Client->gameGUI.colorChecked() )
	{
		SDL_Rect d, t;
		int nr = *((unsigned int*)(owner->color->pixels));
		int max = data.isBig ? ((int)(Client->gameGUI.getTileSize()) - 1) * 2 : (int)(Client->gameGUI.getTileSize()) - 1;

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
	if ( Client->gameGUI.getSelBuilding() == this )
	{
		SDL_Rect d, t;
		int max = data.isBig ? (int)(Client->gameGUI.getTileSize()) * 2 : (int)(Client->gameGUI.getTileSize());
		int len = max / 4;

		d.x = dest.x + 1;
		d.y = dest.y + 1;
		d.w = len;
		d.h = 1;
		t = d;
		SDL_FillRect ( buffer, &d, Client->gameGUI.getBlinkColor() );
		d = t;
		d.x += max - len - 1;
		t = d;
		SDL_FillRect ( buffer, &d, Client->gameGUI.getBlinkColor() );
		d = t;
		d.y += max - 2;
		t = d;
		SDL_FillRect ( buffer, &d, Client->gameGUI.getBlinkColor() );
		d = t;
		d.x = dest.x + 1;
		t = d;
		SDL_FillRect ( buffer, &d, Client->gameGUI.getBlinkColor() );
		d = t;
		d.y = dest.y + 1;
		d.w = 1;
		d.h = len;
		t = d;
		SDL_FillRect ( buffer, &d, Client->gameGUI.getBlinkColor() );
		d = t;
		d.x += max - 2;
		t = d;
		SDL_FillRect ( buffer, &d, Client->gameGUI.getBlinkColor() );
		d = t;
		d.y += max - len - 1;
		t = d;
		SDL_FillRect ( buffer, &d, Client->gameGUI.getBlinkColor() );
		d = t;
		d.x = dest.x + 1;
		SDL_FillRect ( buffer, &d, Client->gameGUI.getBlinkColor() );
	}

	//draw health bar
	if ( Client->gameGUI.hitsChecked() )
		DrawHelthBar();

	//draw ammo bar
	if ( Client->gameGUI.ammoChecked() && data.canAttack && data.ammoMax > 0 )
		DrawMunBar();

	//draw status
	if ( Client->gameGUI.statusChecked() )
		drawStatus();

	//attack job debug output
	if ( Client->gameGUI.getAJobDebugStatus() )
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

	float factor = (float)Client->gameGUI.getTileSize()/(float)64.0;

	// check, if it is dirt:
	if ( !owner )
	{
		if ( data.isBig )
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
			if ( data.isBig )
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

		if ( data.isBig )
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
	if ( data.hasFrames )
	{
		src.w = (int)(Client->gameGUI.getTileSize());
		src.h = (int)(Client->gameGUI.getTileSize());
	}
	else
	{
		src.w = (int)(typ->img_org->w*factor);
		src.h = (int)(typ->img_org->h*factor);
	}

	// draw the concrete
	tmp = dest;
	if ( data.hasBetonUnderground )
	{
		if ( data.isBig )
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
			if ( StartUp && SettingsData.bAlphaEffects )
				SDL_SetAlpha ( UnitsData.ptr_small_beton, SDL_SRCALPHA, StartUp );
			else
				SDL_SetAlpha ( UnitsData.ptr_small_beton, SDL_SRCALPHA, 255 );

			SDL_BlitSurface ( UnitsData.ptr_small_beton, NULL, surface, &tmp );
			SDL_SetAlpha ( UnitsData.ptr_small_beton, SDL_SRCALPHA, 255 );
		}
	}

	tmp = dest;

	// draw the connector slots:
	if ( (this->SubBase && !StartUp) || data.isConnectorGraphic )
	{
		drawConnectors (  surface, dest );
		if ( data.isConnectorGraphic ) return;
	}

	// draw the shadows
	if ( SettingsData.bShadows )
	{
		if ( StartUp && SettingsData.bAlphaEffects )
			SDL_SetAlpha ( typ->shw, SDL_SRCALPHA, StartUp / 5 );
		else
			SDL_SetAlpha ( typ->shw, SDL_SRCALPHA, 50 );

		CHECK_SCALING( typ->shw, typ->shw_org, factor);
		blittAlphaSurface ( typ->shw, NULL, surface, &tmp );
	}

	// blit the players color and building graphic
	if ( data.hasPlayerColor ) SDL_BlitSurface ( owner->color, NULL, GraphicsData.gfx_tmp, NULL );
	else SDL_FillRect ( GraphicsData.gfx_tmp, NULL, 0xFF00FF );

	if ( data.hasFrames )
	{
		if ( data.isAnimated && SettingsData.bAnimations && !Disabled )
		{
			src.x = ( ANIMATION_SPEED % data.hasFrames ) * (int)(Client->gameGUI.getTileSize());
		}
		else
		{
			src.x = dir * (int)(Client->gameGUI.getTileSize());
		}

		CHECK_SCALING( typ->img, typ->img_org, factor);
		SDL_BlitSurface ( typ->img, &src, GraphicsData.gfx_tmp, NULL );

		src.x = 0;
	}
	else if ( data.hasClanLogos )
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

	// draw the building
	tmp = dest;

	src.x = 0;
	src.y = 0;

	if ( StartUp && SettingsData.bAlphaEffects ) SDL_SetAlpha ( GraphicsData.gfx_tmp, SDL_SRCALPHA, StartUp );
	else SDL_SetAlpha ( GraphicsData.gfx_tmp, SDL_SRCALPHA, 255 );

	SDL_BlitSurface ( GraphicsData.gfx_tmp, &src, surface, &tmp );
}

//--------------------------------------------------------------------------
/** Get the numer of menu items */
//--------------------------------------------------------------------------
int cBuilding::GetMenuPointAnz ()
{
	int nr = 2;

	if ( typ == NULL ) return 0;

	if ( typ->data.canSelfDestroy )
		nr++;

	if ( !typ->data.canBuild.empty() )
		nr++;

	if ( typ->data.storeResType != sUnitData::STORE_RES_NONE )
		nr++;

	if ( typ->data.canAttack && data.shotsCur )
		nr++;

	if ( typ->data.canWork && ( IsWorking || typ->data.canBuild.empty() ) )
		nr++;

	if ( typ->data.canMineMaxRes > 0 )
		nr++;

	if ( bSentryStatus || data.canAttack )
		nr++;

	if ( typ->data.storageUnitsMax > 0 )
		nr += 2;

	if ( typ->data.canResearch && IsWorking )
		nr++;

	if ( data.version != owner->BuildingData[typ->nr].version && SubBase && SubBase->Metal >= 2 )
		nr += 2;

	if ( data.convertsGold )
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
	size = Client->gameGUI.getTileSize();

	if ( data.isBig )
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
}

//--------------------------------------------------------------------------
void cBuilding::updateNeighbours (cMap *Map)
{
	int iPosOff = PosX+PosY*Map->size;
	if ( !data.isBig )
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
		if ( b && b->owner == owner && b->data.connectsToBase )			\
			{m=true;}else{m=false;}							\
	}														\

	if ( !data.isBig )
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
	float factor = (float)Client->gameGUI.getTileSize()/(float)64.0;

	CHECK_SCALING( UnitsData.ptr_connector, UnitsData.ptr_connector_org, factor);
	CHECK_SCALING( UnitsData.ptr_connector_shw, UnitsData.ptr_connector_shw_org, factor);

	if ( StartUp ) SDL_SetAlpha( UnitsData.ptr_connector, SDL_SRCALPHA, StartUp );
	else SDL_SetAlpha( UnitsData.ptr_connector, SDL_SRCALPHA, 255 );

	src.y = 0;
	src.x = 0;
	src.h = src.w = UnitsData.ptr_connector->h;

	if ( !data.isBig )
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

		if ( src.x != 0 || data.isConnectorGraphic )
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
		dest.x += Client->gameGUI.getTileSize();
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
		dest.y += Client->gameGUI.getTileSize();
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
		dest.x -= Client->gameGUI.getTileSize();
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
	if ( data.needsHumans )
		if ( SubBase->HumanNeed + data.needsHumans > SubBase->HumanProd )
		{
			sendChatMessageToClient ( "Text~Comp~Team_Insufficient", SERVER_ERROR_MESSAGE, owner->Nr );
			return;
		}

	// needs gold:
	if ( data.convertsGold )
	{
		if ( data.convertsGold + SubBase->GoldNeed > SubBase->getGoldProd() + SubBase->Gold )
		{
			sendChatMessageToClient( "Text~Comp~Gold_Insufficient", SERVER_ERROR_MESSAGE, owner->Nr );
			return;
		}
	}

	// needs raw material:
	if ( data.needsMetal )
	{
		if ( SubBase->MetalNeed + min(MetalPerRound, (*BuildList)[0]->metall_remaining) > SubBase->getMetalProd() + SubBase->Metal )
		{
			sendChatMessageToClient( "Text~Comp~Metal_Insufficient", SERVER_ERROR_MESSAGE, owner->Nr );
			return;
		}
	}

	// needs oil:
	if ( data.needsOil )
	{
		// check if there is enough Oil for the generators (current prodiction + reserves)
		if ( data.needsOil + SubBase->OilNeed > SubBase->Oil + SubBase->getMaxOilProd() )
		{
			sendChatMessageToClient ( "Text~Comp~Fuel_Insufficient", SERVER_ERROR_MESSAGE, owner->Nr );
			return;
		}
		else if ( data.needsOil + SubBase->OilNeed > SubBase->Oil + SubBase->getOilProd() )
		{
			//increase oil production
			int missingOil = data.needsOil + SubBase->OilNeed - (SubBase->Oil + SubBase->getOilProd());

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
	if ( data.canMineMaxRes > 0 )
	{
		int mineFree = data.canMineMaxRes;

		SubBase->changeMetalProd( MaxMetalProd );
		mineFree -= MaxMetalProd;

		SubBase->changeGoldProd( min(MaxGoldProd, mineFree) );
		mineFree-= min ( MaxGoldProd, mineFree );

		SubBase->changeOilProd( min( MaxOilProd, mineFree) );
	}

	// Energy consumers:
	if ( data.needsEnergy )
	{
		if ( data.needsEnergy + SubBase->EnergyNeed > SubBase->EnergyProd )
		{
			//try to increase energy production
			if ( !SubBase->increaseEnergyProd( data.needsEnergy + SubBase->EnergyNeed - SubBase->EnergyProd ) )
			{
				IsWorking = false;

				//reset mine values
				if ( data.canMineMaxRes > 0 )
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

	SubBase->EnergyProd += data.produceEnergy;
	SubBase->EnergyNeed += data.needsEnergy;

	SubBase->HumanNeed += data.needsHumans;
	SubBase->HumanProd += data.produceHumans;

	SubBase->OilNeed += data.needsOil;

	// raw material consumer:
	if ( data.needsMetal )
		SubBase->MetalNeed += min(MetalPerRound, (*BuildList)[0]->metall_remaining);

	// gold consumer:
	SubBase->GoldNeed += data.convertsGold;

	// research building
	if ( data.canResearch )
	{
		owner->ResearchCount++;
		owner->researchCentersWorkingOnArea[researchArea]++;
	}
	
	if( data.canScore )
	{
		sendNumEcos(owner);
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
	if ( Client->gameGUI.getSelBuilding() == this )
	{
		StopFXLoop (Client->iObjectStream);
		PlayFX (typ->Start);
		Client->iObjectStream = playStream ();
	}
	if (data.canResearch)
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
	if ( data.produceEnergy )
	{
		if ( SubBase->EnergyNeed > SubBase->EnergyProd - data.produceEnergy && !override )
		{
			sendChatMessageToClient ( "Text~Comp~Energy_IsNeeded", SERVER_ERROR_MESSAGE, owner->Nr );
			return;
		}

		SubBase->EnergyProd -= data.produceEnergy;
		SubBase->OilNeed -= data.needsOil;
	}

	IsWorking = false;

	// Energy consumers:
	if ( data.needsEnergy )
		SubBase->EnergyNeed -= data.needsEnergy;

	// raw material consumer:
	if ( data.needsMetal )
		SubBase->MetalNeed -= min(MetalPerRound, (*BuildList)[0]->metall_remaining);

	// gold consumer
	if ( data.convertsGold )
		SubBase->GoldNeed -= data.convertsGold;

	// human consumer
	if ( data.needsHumans )
		SubBase->HumanNeed -= data.needsHumans;

	// Minen:
	if ( data.canMineMaxRes > 0 )
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

	if ( data.canResearch )
	{
		owner->ResearchCount--;
		owner->researchCentersWorkingOnArea[researchArea]--;
	}
	
	if( data.canScore )
	{
		sendNumEcos(owner);
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
	if ( Client->gameGUI.getSelBuilding() == this )
	{
		StopFXLoop (Client->iObjectStream);
		PlayFX (typ->Stop);
		Client->iObjectStream = playStream ();
	}
	if (data.canResearch)
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

		if ( v->data.storeResType != data.storeResType )
			return false;

		if ( v->IsBuilding || v->IsClearing )
			return false;

		for (unsigned int i = 0; i < SubBase->buildings.Size(); i++)
		{
			b = SubBase->buildings[i];

			if ( b->data.isBig )
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

			if ( data.storeResType != b->data.storeResType )
				return false;

			return true;
		}

	return false;
}

//--------------------------------------------------------------------------
bool cBuilding::isNextTo( int x, int y) const
{
	if ( x + 1 < PosX || y + 1 < PosY ) return false;

	if ( data.isBig )
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

	if ( canExitTo ( PosX - 1, PosY - 1, Client->Map, typ ) ) Client->gameGUI.drawExitPoint ( spx - Client->gameGUI.getTileSize(), spy - Client->gameGUI.getTileSize() );
	if ( canExitTo ( PosX    , PosY - 1, Client->Map, typ ) ) Client->gameGUI.drawExitPoint ( spx, spy - Client->gameGUI.getTileSize() );
	if ( canExitTo ( PosX + 1, PosY - 1, Client->Map, typ ) ) Client->gameGUI.drawExitPoint ( spx + Client->gameGUI.getTileSize(), spy - Client->gameGUI.getTileSize() );
	if ( canExitTo ( PosX + 2, PosY - 1, Client->Map, typ ) ) Client->gameGUI.drawExitPoint ( spx + Client->gameGUI.getTileSize()*2, spy - Client->gameGUI.getTileSize() );
	if ( canExitTo ( PosX - 1, PosY    , Client->Map, typ ) ) Client->gameGUI.drawExitPoint ( spx - Client->gameGUI.getTileSize(), spy );
	if ( canExitTo ( PosX + 2, PosY    , Client->Map, typ ) ) Client->gameGUI.drawExitPoint ( spx + Client->gameGUI.getTileSize()*2, spy );
	if ( canExitTo ( PosX - 1, PosY + 1, Client->Map, typ ) ) Client->gameGUI.drawExitPoint ( spx - Client->gameGUI.getTileSize(), spy + Client->gameGUI.getTileSize() );
	if ( canExitTo ( PosX + 2, PosY + 1, Client->Map, typ ) ) Client->gameGUI.drawExitPoint ( spx + Client->gameGUI.getTileSize()*2, spy + Client->gameGUI.getTileSize() );
	if ( canExitTo ( PosX - 1, PosY + 2, Client->Map, typ ) ) Client->gameGUI.drawExitPoint ( spx - Client->gameGUI.getTileSize(), spy + Client->gameGUI.getTileSize()*2 );
	if ( canExitTo ( PosX    , PosY + 2, Client->Map, typ ) ) Client->gameGUI.drawExitPoint ( spx, spy + Client->gameGUI.getTileSize()*2 );
	if ( canExitTo ( PosX + 1, PosY + 2, Client->Map, typ ) ) Client->gameGUI.drawExitPoint ( spx + Client->gameGUI.getTileSize(), spy + Client->gameGUI.getTileSize()*2 );
	if ( canExitTo ( PosX + 2, PosY + 2, Client->Map, typ ) ) Client->gameGUI.drawExitPoint ( spx + Client->gameGUI.getTileSize()*2, spy + Client->gameGUI.getTileSize()*2 );
}

//--------------------------------------------------------------------------
bool cBuilding::canExitTo ( const int x, const int y, const cMap* map, const sVehicle *typ ) const
{
	if ( !map->possiblePlaceVehicle( typ->data, x, y ) ) return false;
	if ( !isNextTo(x, y) ) return false;

	return true;
}

//--------------------------------------------------------------------------
bool cBuilding::canLoad ( int offset, cMap *Map, bool checkPosition )
{
	if ( offset < 0 || offset > Map->size*Map->size ) return false;

	if ( canLoad ( Map->fields[offset].getPlanes(), checkPosition ) ) return true;
	else return canLoad ( Map->fields[offset].getVehicles(), checkPosition );

	return false;
}

//--------------------------------------------------------------------------
/** returns, if the vehicle can be loaded from its position: */
//--------------------------------------------------------------------------
bool cBuilding::canLoad ( cVehicle *Vehicle, bool checkPosition )
{
	if ( !Vehicle ) return false;

	if ( Vehicle->Loaded ) return false;

	if ( data.storageUnitsCur == data.storageUnitsMax ) return false;

	if ( checkPosition && !isNextTo ( Vehicle->PosX, Vehicle->PosY ) ) return false;

	int i;
	for ( i = 0; i < (int)data.storeUnitsTypes.size(); i++ )
	{
		if ( data.storeUnitsTypes[i].compare ( Vehicle->data.isStorageType ) == 0 ) break;
	}
	if ( i == data.storeUnitsTypes.size() ) return false;

	if ( Vehicle->ClientMoveJob && ( Vehicle->moving || Vehicle->Attacking || Vehicle->MoveJobActive ) ) return false;

	if ( Vehicle->owner == owner && !Vehicle->IsBuilding && !Vehicle->IsClearing ) return true;

	if ( Vehicle->bIsBeeingAttacked ) return false;

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
	data.storageUnitsCur++;

	if ( data.storageUnitsCur == data.storageUnitsMax ) LoadActive = false;

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

	data.storageUnitsCur--;

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

	MaxMetalProd = min(MaxMetalProd, data.canMineMaxRes);
	MaxGoldProd  = min(MaxGoldProd,  data.canMineMaxRes);
	MaxOilProd   = min(MaxOilProd,   data.canMineMaxRes);
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

	if ( !data.canAttack )
		return false;

	if ( !data.shotsCur )
		return false;

	if ( !data.ammoCur )
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

	selectTarget(v, b, off, data.canAttack, Map );

	if ( v )
	{
		if ( Client && ( v == Client->gameGUI.getSelVehicle() || v->owner == Client->ActivePlayer ) )
			return false;
	}
	else if ( b )
	{
		if ( Client && ( b == Client->gameGUI.getSelBuilding() || b->owner == Client->ActivePlayer ) )
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

	selectTarget(v, b, offset, data.canAttack, Client->Map );

	if ( !(v || b) || ( v && v == Client->gameGUI.getSelVehicle() ) || ( b && b == Client->gameGUI.getSelBuilding() ) )
	{
		r.x = 1;
		r.y = 29;
		r.h = 3;
		r.w = 35;
		SDL_FillRect ( GraphicsData.gfx_Cattack, &r, 0 );
		return;
	}

	if ( v )
		t = v->data.hitpointsCur;
	else if ( b )
		t = b->data.hitpointsCur;

	if ( t )
	{
		if ( v )
			wc = ( int ) ( ( float ) t / v->data.hitpointsMax * 35 );
		else  if ( b )
			wc = ( int ) ( ( float ) t / b->data.hitpointsMax * 35 );
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
			wp = ( int ) ( ( float ) t / v->data.hitpointsMax * 35 );
		else  if ( b )
			wp = ( int ) ( ( float ) t / b->data.hitpointsMax * 35 );
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
/** calculates the costs and the duration of the 3 buildspeeds for the vehicle with the given base costs
	iRemainingMetal is only needed for recalculating costs of vehicles in the Buildqueue and is set per default to -1 */
//--------------------------------------------------------------------------
void cBuilding::CalcTurboBuild ( int *iTurboBuildRounds, int *iTurboBuildCosts, int iVehicleCosts, int iRemainingMetal )
{
	//first calc costs for a new Vehical
	iTurboBuildCosts[0] = iVehicleCosts;

	iTurboBuildCosts[1] = iTurboBuildCosts[0];

	while ( iTurboBuildCosts[1] + ( 2 * data.needsMetal ) <= 2*iTurboBuildCosts[0] )
	{
		iTurboBuildCosts[1] += 2 * data.needsMetal;
	}

	iTurboBuildCosts[2] = iTurboBuildCosts[1];

	while ( iTurboBuildCosts[2] + ( 4 * data.needsMetal ) <= 3*iTurboBuildCosts[0] )
	{
		iTurboBuildCosts[2] += 4 * data.needsMetal;
	}

	//now this is a litle bit tricky ...
	//trying to calculate a plausible value, if we are changing the speed of an already started build-job
	if ( iRemainingMetal >= 0 )
	{
		float WorkedRounds;

		switch ( BuildSpeed )  //BuildSpeed here is the previous build speed
		{

			case 0:
				WorkedRounds = ( iTurboBuildCosts[0] - iRemainingMetal ) / ( float ) ( 1 * data.needsMetal );
				iTurboBuildCosts[0] -= ( int ) ( 1    *  1 * data.needsMetal * WorkedRounds );
				iTurboBuildCosts[1] -= ( int ) ( 0.5  *  4 * data.needsMetal * WorkedRounds );
				iTurboBuildCosts[2] -= ( int ) ( 0.25 * 12 * data.needsMetal * WorkedRounds );
				break;

			case 1:
				WorkedRounds = ( iTurboBuildCosts[1] - iRemainingMetal ) / ( float ) ( 4 * data.needsMetal );
				iTurboBuildCosts[0] -= ( int ) ( 2   *  1 * data.needsMetal * WorkedRounds );
				iTurboBuildCosts[1] -= ( int ) ( 1   *  4 * data.needsMetal * WorkedRounds );
				iTurboBuildCosts[2] -= ( int ) ( 0.5 * 12 * data.needsMetal * WorkedRounds );
				break;

			case 2:
				WorkedRounds = ( iTurboBuildCosts[2] - iRemainingMetal ) / ( float ) ( 12 * data.needsMetal );
				iTurboBuildCosts[0] -= ( int ) ( 4 *  1 * data.needsMetal * WorkedRounds );
				iTurboBuildCosts[1] -= ( int ) ( 2 *  4 * data.needsMetal * WorkedRounds );
				iTurboBuildCosts[2] -= ( int ) ( 1 * 12 * data.needsMetal * WorkedRounds );
				break;
		}
	}


	//calc needed Rounds
	iTurboBuildRounds[0] = ( int ) ceil ( iTurboBuildCosts[0] / ( double ) ( 1 * data.needsMetal ) );

	if ( data.maxBuildFactor > 1 )
	{
		iTurboBuildRounds[1] = ( int ) ceil ( iTurboBuildCosts[1] / ( double ) ( 4 * data.needsMetal ) );
		iTurboBuildRounds[2] = ( int ) ceil ( iTurboBuildCosts[2] / ( double ) ( 12 * data.needsMetal ) );
	}
	else
	{
		iTurboBuildRounds[1] = 0;
		iTurboBuildRounds[2] = 0;
	}
}

//--------------------------------------------------------------------------
/** Returns the screen x position of the building */
//--------------------------------------------------------------------------
int cBuilding::GetScreenPosX () const
{
	return 180 - ( ( int ) ( ( Client->gameGUI.getOffsetX() ) * Client->gameGUI.getZoom() ) ) + (int)(Client->gameGUI.getTileSize())*PosX;
}

//--------------------------------------------------------------------------
/** Returns the screen x position of the building */
//--------------------------------------------------------------------------
int cBuilding::GetScreenPosY () const
{
	return 18 - ( ( int ) ( ( Client->gameGUI.getOffsetY() ) * Client->gameGUI.getZoom() ) ) + (int)(Client->gameGUI.getTileSize())*PosY;
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
	hp = data.hitpointsCur - damage;

	if ( hp < 0 )
	{
		return 0;
	}

	return hp;
}

void cBuilding::menuReleased()
{
	int nr = 0, exeNr;
	SDL_Rect dest = GetMenuSize();
	if ( MouseOverMenu ( mouse->x, mouse->y ) ) exeNr = ( mouse->y - dest.y ) / 22;
	if ( exeNr != selMenuNr ) return;

	if ( bIsBeeingAttacked ) return;

	if (BuildList && BuildList->Size() && !IsWorking && (*BuildList)[0]->metall_remaining <= 0) return;

	// Angriff:
	if ( typ->data.canAttack && data.shotsCur )
	{
		if ( exeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			AttackMode = !AttackMode;
			Client->gameGUI.updateMouseCursor ();
			return;
		}
		nr++;
	}

	// Bauen:
	if ( !typ->data.canBuild.empty() )
	{
		if ( exeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			cVehiclesBuildMenu buildMenu ( owner, this );
			buildMenu.show();
			return;
		}
		nr++;
	}

	// Verteilen:
	if ( typ->data.canMineMaxRes > 0 && IsWorking )
	{
		if ( exeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			cMineManagerMenu mineManager ( this );
			mineManager.show();
			return;
		}
		dest.y += 22;
		nr++;
	}

	// Transfer:
	if ( typ->data.storeResType != sUnitData::STORE_RES_NONE )
	{
		if ( exeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			Transfer = !Transfer;
			return;
		}
		nr++;
	}

	// Start:
	if (typ->data.canWork &&
			!IsWorking         &&
			(
				(BuildList && BuildList->Size()) ||
				typ->data.canBuild.empty()
			))
	{
		if ( exeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			sendWantStartWork(this);
			return;
		}
		nr++;
	}

	// Stop:
	if ( IsWorking )
	{
		if ( exeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			sendWantStopWork(this);
			return;
		}
		nr++;
	}

	// Sentry status:
	if ( bSentryStatus || data.canAttack )
	{
		if ( exeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			sendChangeSentry ( iID, false );
			return;
		}
		nr++;
	}

	// Aktivieren/Laden:
	if ( typ->data.storageUnitsMax > 0 )
	{
		// Aktivieren:
		if ( exeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			cStorageMenu storageMenu ( StoredVehicles, NULL, this );
			storageMenu.show();
			return;
		}
		nr++;

		// Laden:
		if ( exeNr == nr )
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			LoadActive = !LoadActive;
			return;
		}
		nr++;
	}

	// research
	if (typ->data.canResearch && IsWorking)
	{
		if (exeNr == nr)
		{
			MenuActive = false;
			PlayFX (SoundData.SNDObjectMenu);
			cDialogResearch researchDialog ( owner );
			researchDialog.show();
			return;
		}
		nr++;
	}

	// upgradescreen
	if (data.convertsGold)
	{
		// update this
		if (exeNr == nr)
		{
			MenuActive = false;
			PlayFX (SoundData.SNDObjectMenu);
			cUpgradeMenu upgradeMenu ( owner );
			upgradeMenu.show();
			return;
		}
		nr++;
	}

	// Updates:
	if ( data.version != owner->BuildingData[typ->nr].version && SubBase && SubBase->Metal >= 2 )
	{
		// Update all buildings of this type in this subbase
		if (exeNr == nr)
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			sendUpgradeBuilding (this, true);
			return;
		}
		nr++;

		// update this building
		if (exeNr == nr)
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			sendUpgradeBuilding (this, false);
			return;
		}
		nr++;
	}

	// Self destruct
	if ( data.canSelfDestroy )
	{
		if (exeNr == nr)
		{
			MenuActive = false;
			PlayFX ( SoundData.SNDObjectMenu );
			cDestructMenu destructMenu;
			if ( destructMenu.show() == 0 )
				sendWantSelfDestroy(this);
			return;
		}
		nr++;
	}

	// Info:
	if (exeNr == nr)
	{
		MenuActive = false;
		PlayFX (SoundData.SNDObjectMenu);
		cUnitHelpMenu helpMenu ( &data, owner );
		helpMenu.show();
		return;
	}
	nr++;

	// Done:
	if (exeNr == nr)
	{
		MenuActive = false;
		PlayFX (SoundData.SNDObjectMenu);
		return;
	}
}

//--------------------------------------------------------------------------
/** draws the building menu */
//--------------------------------------------------------------------------
void cBuilding::DrawMenu ( sMouseState *mouseState )
{
	int nr = 0;
	static int LastSelMenu = -1;
	bool bSelection = false;
	SDL_Rect dest;
	dest = GetMenuSize();

	if ( bIsBeeingAttacked ) return;

	if ( ActivatingVehicle )
	{
		MenuActive = false;
		return;
	}

	if (BuildList && BuildList->Size() && !IsWorking && (*BuildList)[0]->metall_remaining <= 0) return;

	if ( mouseState && mouseState->leftButtonPressed && MouseOverMenu ( mouse->x, mouse->y ) && ( ( selMenuNr == -1 && LastSelMenu == -1 ) || LastSelMenu == ( mouse->y - dest.y ) / 22 ) )
	{
		selMenuNr = ( mouse->y - dest.y ) / 22;
	}
	else if ( mouseState && mouseState->leftButtonPressed && MouseOverMenu ( mouse->x, mouse->y ) && selMenuNr != -1 && selMenuNr != ( mouse->y - dest.y ) / 22 )
	{
		LastSelMenu = selMenuNr;
		selMenuNr = -1;
	}

	// Angriff:
	if ( typ->data.canAttack && data.shotsCur )
	{
		bSelection = selMenuNr == nr || AttackMode;

		drawContextItem( lngPack.i18n ( "Text~Context~Attack" ), bSelection, dest.x, dest.y, buffer );

		dest.y += 22;
		nr++;
	}

	// Bauen:
	if ( !typ->data.canBuild.empty() )
	{
		bSelection = selMenuNr == nr;

		drawContextItem( lngPack.i18n ( "Text~Context~Build" ), bSelection, dest.x, dest.y, buffer );

		dest.y += 22;
		nr++;
	}

	// Verteilen:
	if ( typ->data.canMineMaxRes > 0 && IsWorking )
	{
		bSelection = selMenuNr == nr;

		drawContextItem( lngPack.i18n ( "Text~Context~Dist" ), bSelection, dest.x, dest.y, buffer );

		dest.y += 22;
		nr++;
	}

	// Transfer:
	if ( typ->data.storeResType != sUnitData::STORE_RES_NONE )
	{
		bSelection = selMenuNr == nr || Transfer;

		drawContextItem( lngPack.i18n ( "Text~Context~Transfer" ), bSelection, dest.x, dest.y, buffer );

		dest.y += 22;
		nr++;
	}

	// Start:
	if (typ->data.canWork &&
			!IsWorking         &&
			(
				(BuildList && BuildList->Size()) ||
				typ->data.canBuild.empty()
			))
	{
		bSelection = selMenuNr == nr;

		drawContextItem( lngPack.i18n ( "Text~Context~Start" ), bSelection, dest.x, dest.y, buffer );

		dest.y += 22;
		nr++;
	}

	// Stop:
	if ( IsWorking )
	{
		bSelection = selMenuNr == nr;

		drawContextItem( lngPack.i18n ( "Text~Context~Stop" ), bSelection, dest.x, dest.y, buffer );

		dest.y += 22;
		nr++;
	}

	// Sentry status:
	if ( bSentryStatus || data.canAttack )
	{
		bSelection = selMenuNr == nr || bSentryStatus;

		drawContextItem( lngPack.i18n ( "Text~Context~Sentry" ), bSelection, dest.x, dest.y, buffer );

		dest.y += 22;
		nr++;
	}

	// Aktivieren/Laden:
	if ( typ->data.storageUnitsMax > 0 )
	{
		// Aktivieren:
		bSelection = selMenuNr == nr;

		drawContextItem( lngPack.i18n ( "Text~Context~Active" ), bSelection, dest.x, dest.y, buffer );

		dest.y += 22;
		nr++;

		// Laden:
		bSelection = selMenuNr == nr || LoadActive;

		drawContextItem( lngPack.i18n ( "Text~Context~Load" ), bSelection, dest.x, dest.y, buffer );

		dest.y += 22;
		nr++;
	}

	// research
	if (typ->data.canResearch && IsWorking)
	{
		bSelection = (selMenuNr == nr);
		drawContextItem (lngPack.i18n ("Text~Context~Research"), bSelection, dest.x, dest.y, buffer);
		dest.y += 22;
		nr++;
	}

	// upgradescreen
	if (data.convertsGold)
	{
		// update this
		bSelection = (selMenuNr == nr);
		drawContextItem (lngPack.i18n ("Text~Context~Upgrades"), bSelection, dest.x, dest.y, buffer);
		dest.y += 22;
		nr++;
	}

	// Updates:
	if ( data.version != owner->BuildingData[typ->nr].version && SubBase && SubBase->Metal >= 2 )
	{
		// Update all buildings of this type in this subbase
		bSelection = (selMenuNr == nr);
		drawContextItem (lngPack.i18n ("Text~Context~UpAll"), bSelection, dest.x, dest.y, buffer);
		dest.y += 22;
		nr++;

		// update this building
		bSelection = (selMenuNr == nr);
		drawContextItem (lngPack.i18n ("Text~Context~Upgrade"), bSelection, dest.x, dest.y, buffer);
		dest.y += 22;
		nr++;
	}

	// Self destruct
	if ( data.canSelfDestroy )
	{
		bSelection = (selMenuNr == nr);
		drawContextItem (lngPack.i18n ("Text~Context~Destroy"), bSelection, dest.x, dest.y, buffer);
		dest.y += 22;
		nr++;
	}

	// Info:
	bSelection = (selMenuNr == nr);
	drawContextItem (lngPack.i18n ("Text~Context~Info"), bSelection, dest.x, dest.y, buffer);
	dest.y += 22;
	nr++;

	// Done:
	bSelection = (selMenuNr == nr);
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
	data.version = upgradeVersion.version;

	if (data.hitpointsCur == data.hitpointsMax)
		data.hitpointsCur = upgradeVersion.hitpointsMax; // TODO: check behaviour in original
	data.hitpointsMax = upgradeVersion.hitpointsMax;

	data.ammoMax = upgradeVersion.ammoMax; // don't change the current ammo-amount!

	data.armor = upgradeVersion.armor;
	data.scan = upgradeVersion.scan;
	data.range = upgradeVersion.range;
	data.shotsMax = upgradeVersion.shotsMax; // TODO: check behaviour in original
	data.damage = upgradeVersion.damage;
	data.buildCosts = upgradeVersion.buildCosts;

	GenerateName();
}


//------------------------------------------------------------------------
/** centers on this building */
//--------------------------------------------------------------------------
void cBuilding::Center ()
{
	int offX = PosX * 64 - ( ( int ) ( ( ( float ) (SettingsData.iScreenW - 192) / (2 * Client->gameGUI.getTileSize() ) ) * 64 ) ) + 32;
	int offY = PosY * 64 - ( ( int ) ( ( ( float ) (SettingsData.iScreenH - 32 ) / (2 * Client->gameGUI.getTileSize() ) ) * 64 ) ) + 32;
	Client->gameGUI.setOffsetPosition ( offX, offY );
}

//------------------------------------------------------------------------
/** draws the available ammunition over the building: */
//--------------------------------------------------------------------------
void cBuilding::DrawMunBar ( void ) const
{
	SDL_Rect r1, r2;
	r1.x = GetScreenPosX() + Client->gameGUI.getTileSize()/10 + 1;
	r1.w = Client->gameGUI.getTileSize() * 8 / 10 ;
	r1.h = Client->gameGUI.getTileSize() / 8;
	r1.y = GetScreenPosY() + Client->gameGUI.getTileSize()/10 + Client->gameGUI.getTileSize() / 8;

	if ( r1.h <= 2 )
	{
		r1.h = 3;
		r1.y += 1;
	}

	r2.x = r1.x + 1;
	r2.y = r1.y + 1;
	r2.h = r1.h - 2;
	r2.w = ( int ) ( ( ( float ) ( r1.w - 2 ) ) / data.ammoMax  * data.ammoCur );

	SDL_FillRect ( buffer, &r1, 0 );

	if ( data.ammoCur > data.ammoMax / 2 )
	{
		SDL_FillRect ( buffer, &r2, 0x04AE04 );
	}
	else if ( data.ammoCur > data.ammoMax / 4 )
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
	r1.x = GetScreenPosX() + Client->gameGUI.getTileSize()/10 + 1;
	r1.w = Client->gameGUI.getTileSize() * 8 / 10 ;
	r1.h = Client->gameGUI.getTileSize() / 8;
	r1.y = GetScreenPosY() + Client->gameGUI.getTileSize()/10;

	if ( data.isBig )
	{
		r1.w += Client->gameGUI.getTileSize();
		r1.h *= 2;
	}

	if ( r1.h <= 2  )
		r1.h = 3;

	r2.x = r1.x + 1;
	r2.y = r1.y + 1;
	r2.h = r1.h - 2;
	r2.w = ( int ) ( ( ( float ) ( r1.w - 2 ) / data.hitpointsMax ) * data.hitpointsCur );

	SDL_FillRect ( buffer, &r1, 0 );

	if ( data.hitpointsCur > data.hitpointsMax / 2 )
	{
		SDL_FillRect ( buffer, &r2, 0x04AE04 );
	}
	else if ( data.hitpointsCur > data.hitpointsMax / 4 )
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
		if ( Client->gameGUI.getTileSize() < 25 ) return;
		dest.x = GetScreenPosX() + Client->gameGUI.getTileSize()/2 - 12;
		dest.y = GetScreenPosY() + Client->gameGUI.getTileSize()/2 - 12;
		SDL_BlitSurface( GraphicsData.gfx_hud_stuff, &disabledSymbol, buffer, &dest );
	}
	else
	{
		dest.y = GetScreenPosY() + Client->gameGUI.getTileSize() - 11;
		dest.x = GetScreenPosX() + Client->gameGUI.getTileSize()/2 - 4;
		if ( data.shotsCur )
		{
			SDL_BlitSurface( GraphicsData.gfx_hud_stuff, &shotsSymbol, buffer, &dest );
		}
	}
}

//--------------------------------------------------------------------------
void cBuilding::Select ()
{
	if ( !owner ) return;

	//load video
	if ( Client->gameGUI.getFLC() != NULL )
	{
		FLI_Close ( Client->gameGUI.getFLC() );
		Client->gameGUI.setFLC( NULL );
	}
	Client->gameGUI.setVideoSurface( typ->video );

	// Sound abspielen:
	if ( !IsWorking )
		PlayFX ( SoundData.SNDHudButton );

	// Die Eigenschaften anzeigen:
	Client->gameGUI.setUnitDetailsData ( NULL, this );
}

//--------------------------------------------------------------------------
void cBuilding::Deselct ()
{
	SDL_Rect src, dest;
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
	Client->gameGUI.setVideoSurface ( NULL );
	Client->gameGUI.setUnitDetailsData ( NULL, NULL );
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
	if ( data.isStealthOn == TERRAIN_NONE ) return;

	if ( data.isStealthOn&AREA_EXP_MINE )
	{
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
}

//--------------------------------------------------------------------------
void sBuilding::scaleSurfaces( float factor )
{
	scaleSurface ( img_org, img, (int)(img_org->w*factor), (int)(img_org->h*factor) );
	scaleSurface ( shw_org, shw, (int)(shw_org->w*factor), (int)(shw_org->h*factor) );
	if ( eff_org ) scaleSurface ( eff_org, eff, (int)(eff_org->w*factor), (int)(eff_org->h*factor) );
}
