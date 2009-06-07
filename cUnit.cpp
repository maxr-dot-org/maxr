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
/** Author: Anton Reis hackar@maxr.org **/

#include "cUnit.h"

cUnit::cGraphics::cGraphics(sBuilding building, sDamageEffects *damageEffects){
  img = building.img;
  img_org = building.img_org;
  shw = building.shw;
  shw_org = building.shw_org;
  eff = building.eff;
  eff_org = building.eff_org;
  video = building.video;
  info = building.info;
  text = building.text;
  showDamage = damageEffects;
}
cUnit::cGraphics::cGraphics(sVehicle vehicle, sDamageEffects *damageEffects){
  for (int i = 0; i < 8; i++){
    vimg[i] = vehicle.img[i];
    vimg_org[i] = vehicle.img_org[i];
    vshw[i] = vehicle.shw[i];
    vshw_org[i] = vehicle.shw_org[i];
  }
  build = vehicle.build;
  build_org = vehicle.build_org;
  build_shw = vehicle.build_shw;
  build_shw_org = vehicle.build_shw_org;
  clear_small = vehicle.clear_small;
  clear_small_org = vehicle.clear_small_org;
  clear_small_shw = vehicle.clear_small_shw;
  clear_small_shw_org = vehicle.clear_small_shw_org;
  overlay = vehicle.overlay;
  overlay_org = vehicle.overlay_org;
  storage = vehicle.storage;
  FLCFile = vehicle.FLCFile;
  info = vehicle.info;
  text = vehicle.text;
  showDamage = damageEffects;
}
cUnit::cGraphics::~cGraphics(void){
}

void cUnit::cGraphics::scaleSurfaces(float factor){
 	int width, height;
  if ( img_org ){
		width= (int) ( img_org->w*factor );
		height= (int) ( img_org->h*factor );
    scaleSurface ( img_org, img, width, height );
  }
  if ( shw_org ){
		width= (int) ( shw_org->w*factor );
		height= (int) ( shw_org->h*factor );
    scaleSurface ( shw_org, shw, width, height );
  }
  if ( eff_org ){
		width= (int) ( eff_org->w*factor );
		height= (int) ( eff_org->h*factor );
    scaleSurface ( eff_org, eff, width, height );
  }
	for ( int i = 0; i < 8; i++ )
	{
    if ( vimg_org[i] ){
		  width= (int) ( vimg_org[i]->w*factor );
		  height= (int) ( vimg_org[i]->h*factor );
		  scaleSurface ( vimg_org[i], vimg[i], width, height );
    }
    if ( vshw_org[i] ){
  		width= (int) ( vshw_org[i]->w*factor );
	  	height= (int) ( vshw_org[i]->h*factor );
		  scaleSurface ( vshw_org[i], vshw[i], width, height );
    }
	}
	if ( build_org )
	{
		height= (int) ( build_org->h*factor );
		width=height*4;
		scaleSurface ( build_org, build, width, height );
    if ( build_shw_org ){
		  width= (int) ( build_shw_org->w*factor );
		  height= (int) ( build_shw_org->h*factor );
		  scaleSurface ( build_shw_org, build_shw, width, height );
    }
	}
	if ( clear_small_org )
	{
		height= (int) ( clear_small_org->h*factor );
		width=height*4;
		scaleSurface ( clear_small_org, clear_small, width, height );
    if ( clear_small_shw_org ) {
		  width= (int) ( clear_small_shw_org->w*factor );
		  height= (int) ( clear_small_shw_org->h*factor );
		  scaleSurface ( clear_small_shw_org, clear_small_shw, width, height );
    }
	}
	if ( overlay_org )
	{
		height= (int) ( overlay_org->h*factor );
		width= (int) ( overlay_org->w*factor );
		scaleSurface ( overlay_org, overlay, width, height );
	}
}

cUnit::cSounds::cSounds(sBuilding building){
  Start = building.Start;
  Running = building.Running;
  Stop = building.Stop;
  Attack = building.Attack;
}

cUnit::cSounds::cSounds(sVehicle vehicle){
  Wait = vehicle.Wait;
  WaitWater = vehicle.WaitWater;
  Start = vehicle.Start;
  StartWater = vehicle.StartWater;
  Stop = vehicle.Stop;
  StopWater = vehicle.StopWater;
  Drive = vehicle.Drive;
  DriveWater = vehicle.DriveWater;
  Attack = vehicle.Attack;
}

cUnit::cUnit(int buildingType, unsigned int posX, unsigned int posY, cPlayer *owner, cBase *base){
  iBuildingType = buildingType;
  iPosX = posX;
  iPosY = posY;
//iRubbleType = 0;
//iRubbleValue = 0;
  graphics = new cIGraphics(this);
  iDir = 0;
  unitData = (*allUnits)[iBuildingType];
  if (unitData->abilities.producing){
    producing = new cIProducing(this);
  }
}
cPlayer* cUnit::getOwner(void){
  return owner;
}
void cUnit::init_SetUnits(cList<sUnit*> *units){
  if (allUnits){
    while(allUnits->Size()){
 			sUnit *ptr;
			ptr = (*allUnits)[0];
			delete ptr;
			allUnits->Delete( 0 );
    }
    delete allUnits;
  }
  allUnits = new cList<sUnit*>;
  allUnits->Reserve(units->Size());
  for (unsigned int i = 0; i<units->Size(); i++){
    allUnits->Add((*units)[i]);
  }
}
void cUnit::init_AddUnit(cUnit::sUnit *unit){
  allUnits->Add(unit);
}

cUnit::~cUnit(void){
  if (producing) {delete producing;}
  if (graphics) {delete graphics;}
}
cUnit::cIProducing::cIProducing(cUnit *unit){
  lBuildList = new cList<sBuildList*>;
  this->unit = unit;
}

cUnit::cIActivable::cIActivable(void){
  setActive(false);
}
void cUnit::cIActivable::setActive(bool working){
  bWorking = working;
}

bool cUnit::cIActivable::active(void){
  return bWorking;
}
const cList<cUnit::cIProducing::sBuildList*> cUnit::cIProducing::getBuildList(void){
  return (*this->lBuildList);
}
unsigned int cUnit::cIProducing::getMult(void){
  return this->iMult;
}
bool cUnit::cIProducing::productionFinished(void){
  return (lBuildList->Size() && !unit->activable->active() && (*lBuildList)[0]->metall_remaining <= 0);
}
cUnit::cIProducing::~cIProducing(void){
  if (lBuildList){
    while(lBuildList->Size()){
 			sBuildList *ptr;
			ptr = (*lBuildList)[0];
			delete ptr;
			lBuildList->Delete( 0 );
    }
    delete lBuildList;
  }
}
cUnit::cIGraphics::cIGraphics(cUnit *unit){
  setEffectAlpha(0);
  setEffectInc(true);
  this->unit = unit;
  iStartUp = 0;
}
cUnit::cIGraphics::~cIGraphics(void){
}
void cUnit::cIGraphics::setEffectAlpha(int effectAlpha){
  iEffectAlpha = effectAlpha;
}
void cUnit::cIGraphics::setEffectInc(bool effectInc){
  bEffectInc = effectInc;
}
int cUnit::cIGraphics::getEffectAlpha(void){
  return iEffectAlpha;
}
bool cUnit::cIGraphics::getEffectInc(void){
  return bEffectInc;
}
//----------------------------------------------------
/** Returns a string with the current state */
//----------------------------------------------------
string cUnit::getStatusStr (){
  string sReturn = "";
  bool nl = false;
  if (owner == Client->ActivePlayer){
    if (producing && producing->getBuildList().Size() ){
      if (nl) sReturn += "\n"; else nl = true;
      cIProducing::sBuildList *ptr;
      ptr = producing->getBuildList()[0];
      if ( ptr->metall_remaining > 0 ){
        sReturn += lngPack.i18n ( "Text~Comp~Producing" ) + ":";
        string sTemp = (*allUnits)[ptr->unit]->name + " (";
        sTemp += iToStr (( int ) ceil ( ptr->metall_remaining / ( double ) (((*allUnits)[iBuildingType]->abilities.producing->iProducingMPR)*this->producing->getMult()) ))+ ")";
        string sTemp2 = sReturn + " " + sTemp;
        if ( font->getTextWide ( sTemp2, FONT_LATIN_SMALL_WHITE ) > 126 ){
          sReturn += "\n" + sTemp;
        }else{
          sReturn = sTemp2;
        }
      }else{
        return lngPack.i18n ( "Text~Comp~Producing_Fin" );
      }
    }
  }else{
    return lngPack.i18n ( "Text~Comp~Working" );
  }
	//if ( IsWorking )
	//{
	//	// Producing:
	//	if (data.can_build && BuildList && BuildList->Size() && owner == Client->ActivePlayer)
	//	{
	//		sBuildList *ptr;
	//		ptr = (*BuildList)[0];

	//		if ( ptr->metall_remaining > 0 )
	//		{
	//			string sText;
	//			int iRound;

	//			iRound = ( int ) ceil ( ptr->metall_remaining / ( double ) MetalPerRound );
	//			sText = lngPack.i18n ( "Text~Comp~Producing" ) + ": ";
	//			sText += ( string ) owner->VehicleData[ptr->typ->nr].name + " (";
	//			sText += iToStr ( iRound ) + ")";

	//			if ( font->getTextWide ( sText, FONT_LATIN_SMALL_WHITE ) > 126 )
	//			{
	//				sText = lngPack.i18n ( "Text~Comp~Producing" ) + ":\n";
	//				sText += ( string ) owner->VehicleData[ptr->typ->nr].name + " (";
	//				sText += iToStr ( iRound ) + ")";
	//			}

	//			return sText;
	//		}
	//		else
	//		{
	//			return lngPack.i18n ( "Text~Comp~Producing_Fin" );
	//		}
	//	}

	//	// Research Center
	//	if (data.can_research && owner == Client->ActivePlayer)
	//	{
	//		string sText = lngPack.i18n ( "Text~Comp~Working" ) + "\n";
	//		for (int area = 0; area < cResearch::kNrResearchAreas; area++)
	//		{
	//			if (owner->researchCentersWorkingOnArea[area] > 0)
	//			{
	//				switch (area)
	//				{
	//					case cResearch::kAttackResearch: sText += lngPack.i18n ( "Text~Vehicles~Damage" ); break;
	//					case cResearch::kShotsResearch: sText += lngPack.i18n ( "Text~Hud~Shots" ); break;
	//					case cResearch::kRangeResearch: sText += lngPack.i18n ( "Text~Hud~Range" ); break;
	//					case cResearch::kArmorResearch: sText += lngPack.i18n ( "Text~Vehicles~Armor" ); break;
	//					case cResearch::kHitpointsResearch: sText += lngPack.i18n ( "Text~Hud~Hitpoints" ); break;
	//					case cResearch::kSpeedResearch: sText += lngPack.i18n ( "Text~Hud~Speed" ); break;
	//					case cResearch::kScanResearch: sText += lngPack.i18n ( "Text~Hud~Scan" ); break;
	//					case cResearch::kCostResearch: sText += lngPack.i18n ( "Text~Vehicles~Costs" ); break;
	//				}
	//				sText += ": " + iToStr (owner->researchLevel.getRemainingTurns (area, owner->researchCentersWorkingOnArea[area])) + "\n";
	//			}
	//		}
	//		return sText;
	//	}

	//	// Goldraffinerie:
	//	if ( data.gold_need && owner == Client->ActivePlayer )
	//	{
	//		string sText;
	//		sText = lngPack.i18n ( "Text~Comp~Working" ) + "\n";
	//		sText += lngPack.i18n ( "Text~Title~Credits" ) + ": ";
	//		sText += iToStr ( owner->Credits );
	//		return sText.c_str();
	//	}

	//	return lngPack.i18n ( "Text~Comp~Working" );
	//}

	//if ( Disabled )
	//{
	//	string sText;
	//	sText = lngPack.i18n ( "Text~Comp~Disabled" ) + " (";
	//	sText += iToStr ( Disabled ) + ")";
	//	return sText.c_str();
	//}
	//if ( bSentryStatus )
	//{
	//	return lngPack.i18n ( "Text~Comp~Sentry" );
	//}

	//return lngPack.i18n ( "Text~Comp~Waits" );
}
//--------------------------------------------------------------------------
//   0 = Metal
//   1 = Fuel
//   2 = Gold
//   3 = Energy
//   4 = Credits
unsigned int cUnit::cIGenerating::GetProd(ResourceKind const resource)
{
		if ((resource < 0)||(resource > 4)) throw std::logic_error("Invalid resource kind");
    return this->generation[resource];
}
unsigned int cUnit::cIGenerating::GetMaxProd(ResourceKind const resource) const
{
		if ((resource < 0)||(resource > 4)) throw std::logic_error("Invalid resource kind");
    return maxGeneration[resource];
}
int cUnit::cIAttacking::refreshData (void)
{
	if ( iShots < iMaxShots )
	{
		if ( iAmmo >= iMaxShots )
			iShots = iMaxShots;
		else
			iShots = iAmmo;
		return 1;
	}
	return 0;
}
unsigned int cUnit::cIAttacking::getAmmo(void){
  return iAmmo;
}
bool cUnit::cIAttacking::isAttacking(void){
  return bAttacking;
}
bool cUnit::isAttacked(void){
  return bIsAttacked;
}
int cUnit::refreshData(void){
  if(attacking) return attacking->refreshData();
  Log.write("Call of refreshData(void) on not attacking unit",LOG_TYPE_WARNING);
  return 0;
}
//--------------------------------------------------------------------------
/** generates the name for the building depending on the versionnumber */
//--------------------------------------------------------------------------
void cUnit::generateName (void)
{
	string rome, tmp_name;
	int nr, tmp;
	string::size_type tmp_name_idx;
	rome = "";
	nr = iVersion;

	// generate the roman versionnumber (correct until 899)

	if ( nr > 100 ){
		tmp = nr / 100;
		nr %= 100;
		while ( tmp-- ) rome += "C";
	}
	if ( nr >= 90 ){
		rome += "XC";
		nr -= 90;
	}
	if ( nr >= 50 ){
		nr -= 50;
		rome += "L";
	}
	if ( nr >= 40 ){
		nr -= 40;
		rome += "XL";
	}
	if ( nr >= 10 ){
		tmp = nr / 10;
		nr %= 10;
		while ( tmp-- ) rome += "X";
	}
	if ( nr == 9 ){
		nr -= 9;
		rome += "IX";
	}
	if ( nr >= 5 ){
		nr -= 5;
		rome += "V";
	}
	if ( nr == 4 ){
		nr -= 4;
		rome += "IV";
	}
	while ( nr-- ) rome += "I";

  if ( name.length() == 0 ){
		// prefix
		name = "MK ";
		name += rome;
		name += " ";
		// object name
		name += ( string ) (*allUnits)[iBuildingType]->name;
	}else{
		// check for MK prefix
		tmp_name = name.substr(0,2);
		if ( 0 == (int)tmp_name.compare("MK") ){
			// current name, without prefix
			tmp_name_idx = name.find_first_of(" ", 4 );
      if( tmp_name_idx != string::npos ){
				tmp_name = ( string )name.substr(tmp_name_idx);
				// prefix
				name = "MK ";
				name += rome;
				// name
				name += tmp_name;
			}else{
				tmp_name = name;
				// prefix
				name = "MK ";
				name += rome;
				name += " ";
				// name
				name += tmp_name;
			}
		}else{
			tmp_name = name;
			name = "MK ";
			name += rome;
			name += " ";
			name += tmp_name;
		}
	}
}

void cUnit::setSelected(bool selected){
  bSelected = selected;
}
bool cUnit::isSelected(void){
  return bSelected;
}
void cUnit::cIGraphics::draw (SDL_Rect *screenPos)
{
	SDL_Rect dest, tmp;
	float factor = (float)(Client->Hud.Zoom/64.0);
	
	// draw the damage effects
  if (Client->iTimer1 && unit->unitData->graphics.showDamage
   && unit->iHP < unit->iMaxHP && SettingsData.bDamageEffects
   && (unit->getOwner() == Client->ActivePlayer || Client->ActivePlayer->ScanMap[unit->iPosX+unit->iPosY*Client->Map->size])){
    int intense = ( int ) ( 200 - 200 * ( ( float ) unit->iHP / unit->iMaxHP ) );
		Client->addFX ( fxDarkSmoke, unit->iPosX*64 + unit->unitData->graphics.showDamage->DamageFXPointX, unit->iPosY*64 + unit->unitData->graphics.showDamage->DamageFXPointY, intense );

		if ( unit->unitData->bBig && intense > 50 ){
			intense -= 50;
			Client->addFX ( fxDarkSmoke, unit->iPosX*64 + unit->unitData->graphics.showDamage->DamageFXPointX2, unit->iPosY*64 + unit->unitData->graphics.showDamage->DamageFXPointY2, intense );
		}
	}

	dest.x = dest.y = 0;
	bool bDraw = false;
	SDL_Surface* drawingSurface = NULL;//Client->dCache.getCachedImage(this); //TODO: 'this'?
	if ( drawingSurface == NULL )
	{
		//no cached image found. building needs to be redrawn.
		bDraw = true;
		drawingSurface = NULL; //Client->dCache.createNewEntry(this);
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

	if (!(unit->getOwner())) return;

	if ( iStartUp )
	{
		if ( Client->iTimer0 )
			iStartUp += 25;

		if ( iStartUp >= 255 )
			iStartUp = 0;
	}

	// draw the effect if necessary
	if ( unit->unitData->graphics.eff && SettingsData.bAnimations && ( !unit->activable || unit->activable->active() ) )
	{
		tmp = dest;
		SDL_SetAlpha ( unit->unitData->graphics.eff, SDL_SRCALPHA, iEffectAlpha );

		CHECK_SCALING( unit->unitData->graphics.eff, unit->unitData->graphics.eff_org, factor);
		SDL_BlitSurface ( unit->unitData->graphics.eff, NULL, buffer, &tmp );

		if ( Client->iTimer0 )
		{
			if ( bEffectInc )
			{
				iEffectAlpha += 30;

				if ( iEffectAlpha > 220 )
				{
					iEffectAlpha = 255;
					bEffectInc = false;
				}
			}
			else
			{
				iEffectAlpha -= 30;

				if ( iEffectAlpha < 30 )
				{
					iEffectAlpha = 0;
					bEffectInc = true;
				}
			}
		}
	}

	// draw the mark, when a build order is finished 
	if (unit->producing->productionFinished() && unit->getOwner() == Client->ActivePlayer)
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
		int nr = *((unsigned int*)(unit->getOwner()->color->pixels));
		int max = unit->unitData->bBig ? (Client->Hud.Zoom - 1) * 2 : Client->Hud.Zoom - 1;

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
	if ( unit->isSelected() )
	{
		SDL_Rect d, t;
		int max = unit->unitData->bBig ? Client->Hud.Zoom * 2 : Client->Hud.Zoom;
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
		drawHealthBar();

	//draw ammo bar
	if ( Client->Hud.Munition && unit->attacking && unit->attacking->getAmmo() > 0 )
		drawMunBar();

	//draw status
	if ( Client->Hud.Status )
		drawStatus();

	//attack job debug output
	if ( Client->bDebugAjobs )
	{
    //TODO: change needed?
		cBuilding* serverBuilding = NULL;
		if ( Server ) serverBuilding = Server->Map->fields[unit->iPosX + unit->iPosY*Server->Map->size].getBuildings();
		if ( unit->isAttacked() ) font->showText(dest.x + 1,dest.y + 1, "C: attacked", FONT_LATIN_SMALL_WHITE );
		if ( serverBuilding && serverBuilding->bIsBeeingAttacked ) font->showText(dest.x + 1,dest.y + 9, "S: attacked", FONT_LATIN_SMALL_YELLOW );
		if ( unit->attacking && unit->attacking->isAttacking() ) font->showText(dest.x + 1,dest.y + 17, "C: attacking", FONT_LATIN_SMALL_WHITE );
		if ( serverBuilding && serverBuilding->Attacking ) font->showText(dest.x + 1,dest.y + 25, "S: attacking", FONT_LATIN_SMALL_YELLOW );
	}
}
