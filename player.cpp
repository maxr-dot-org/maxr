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
#include <math.h>
#include "player.h"
#include "menu.h"
#include "client.h"
#include "server.h"
#include "serverevents.h"

//-----------------------------------------------------------------------
// Implementation cPlayer class
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------
cPlayer::cPlayer(string Name, SDL_Surface* Color, int nr, int iSocketNum) 
: base(this)
, name (Name)
, color (Color)
, Nr (nr)
{
	// copy the vehicle stats
	VehicleData = new sUnitData[UnitsData.vehicle.Size()];
	for (size_t i = 0; i < UnitsData.vehicle.Size(); i++)
		VehicleData[i]=UnitsData.vehicle[i].data;

	// copy the building stats
	BuildingData = new sUnitData[UnitsData.building.Size()];
	for (size_t i = 0; i < UnitsData.building.Size(); i++)
		BuildingData[i] = UnitsData.building[i].data;

	DetectLandMap = NULL;
	DetectSeaMap = NULL;
	DetectMinesMap = NULL;
	ScanMap = NULL;
	SentriesMapAir = NULL;
	SentriesMapGround = NULL;
	VehicleList = NULL;
	BuildingList = NULL;
	ResourceMap = NULL;
	
	ResearchCount = 0;
	for (int i = 0; i < cResearch::kNrResearchAreas; i++)
		researchCentersWorkingOnArea[i] = 0;	
	Credits=0;
	reportResearchFinished = false;
	
	this->iSocketNum = iSocketNum;
	isDefeated = false;
	bFinishedTurn = false;
}

//-----------------------------------------------------------------------
cPlayer::cPlayer(const cPlayer &Player) 
: base(this)
{
	name = Player.name;
	color = Player.color;
	Nr = Player.Nr;
	iSocketNum = Player.iSocketNum;

	// copy vehicle and building datas
	VehicleData = new sUnitData[UnitsData.vehicle.Size()];
	for ( unsigned int i = 0; i < UnitsData.vehicle.Size(); i++)
		VehicleData[i] = Player.VehicleData[i];
	BuildingData = new sUnitData[UnitsData.building.Size()];
	for ( unsigned int i = 0; i < UnitsData.building.Size(); i++)
		BuildingData[i] = Player.BuildingData[i];

	DetectLandMap = NULL;
	DetectSeaMap = NULL;
	DetectMinesMap = NULL;
	ScanMap = NULL;
	SentriesMapAir = NULL;
	SentriesMapGround = NULL;
	VehicleList = NULL;
	BuildingList = NULL;
	ResourceMap = NULL;
	
	Credits = Player.Credits;
	ResearchCount = Player.ResearchCount;
	for (int i = 0; i < cResearch::kNrResearchAreas; i++)
		researchCentersWorkingOnArea[i] = Player.researchCentersWorkingOnArea[i];
	for (int i = 0; i < cResearch::kNrResearchAreas; i++)
	{
		researchLevel.setCurResearchLevel(Player.researchLevel.getCurResearchLevel(i), i);
		researchLevel.setCurResearchPoints(Player.researchLevel.getCurResearchPoints(i), i);		
	}
	reportResearchFinished = Player.reportResearchFinished;

	this->iSocketNum = iSocketNum;
	isDefeated = false;
	bFinishedTurn = Player.bFinishedTurn;
}

//-----------------------------------------------------------------------
cPlayer::~cPlayer ()
{
	while ( SentriesAir.Size() )
	{
		delete SentriesAir[SentriesAir.Size() - 1];
		SentriesAir.Delete( SentriesAir.Size() - 1 );
	}

	while ( SentriesGround.Size() )
	{
		delete SentriesGround[SentriesGround.Size() - 1];
		SentriesGround.Delete( SentriesGround.Size() - 1 );
	}

	// Erst alle geladenen Vehicles lˆschen:
	cVehicle *ptr=VehicleList;
	while ( ptr )
	{
		if ( ptr->StoredVehicles.Size() )
		{
			ptr->DeleteStored();
		}
		ptr=ptr->next;
	}
	// Jetzt alle Vehicles lˆschen:
	while ( VehicleList )
	{
		cVehicle *ptr;
		ptr=VehicleList->next;
		VehicleList->bSentryStatus = false;
		delete VehicleList;
		VehicleList=ptr;
	}
	while ( BuildingList )
	{
		cBuilding *ptr;
		ptr=BuildingList->next;
		BuildingList->bSentryStatus = false;

		// Stored Vehicles are already deleted; just clear the list
		while( BuildingList->StoredVehicles.Size() > 0 )
		{
			BuildingList->StoredVehicles.Delete( BuildingList->StoredVehicles.Size() - 1 );
		}

		delete BuildingList;
		BuildingList=ptr;
	}
	delete [] VehicleData;
	delete [] BuildingData;
	if ( ScanMap ) delete [] ScanMap;
	if ( SentriesMapAir ) delete [] SentriesMapAir;
	if ( SentriesMapGround ) delete [] SentriesMapGround;
	if ( ResourceMap ) delete [] ResourceMap;

	if ( DetectLandMap ) delete [] DetectLandMap;
	if ( DetectSeaMap ) delete [] DetectSeaMap;
	if ( DetectMinesMap ) delete [] DetectMinesMap;

	while ( ReportVehicles.Size() )
	{
		delete ReportVehicles[0];
		ReportVehicles.Delete ( 0 );
	}
	while ( ReportBuildings.Size() )
	{
		delete ReportBuildings[0];
		ReportBuildings.Delete ( 0 );
	}
	while ( LockList.Size() )
	{
		delete LockList[0];
		LockList.Delete ( 0 );
	}
}

//--------------------------------------------------------------------------
/** Adds the vehicle to the list of the player */
//--------------------------------------------------------------------------
cVehicle *cPlayer::AddVehicle (int posx, int posy, sVehicle *v)
{
	cVehicle *n;

	n=new cVehicle ( v,this );
	n->PosX=posx;
	n->PosY=posy;
	n->prev=NULL;
	if ( VehicleList!=NULL )
	{
		VehicleList->prev=n;
	}
	n->next=VehicleList;
	VehicleList=n;
	n->GenerateName();
	drawSpecialCircle ( n->PosX,n->PosY,n->data.scan,ScanMap, (int)sqrt ( (double)MapSize ) );
	if ( n->data.can_detect_land ) drawSpecialCircle ( n->PosX, n->PosY, n->data.scan, DetectLandMap, (int)sqrt ( (double)MapSize ) );
	if ( n->data.can_detect_sea  ) drawSpecialCircle ( n->PosX, n->PosY, n->data.scan, DetectSeaMap, (int)sqrt ( (double)MapSize )  );
	if ( n->data.can_detect_mines)
	{
		for ( int x = n->PosX - 1; x <= n->PosX + 1; x++ )
		{
			if ( x < 0 || x >= (int)sqrt ( (double)MapSize ) ) continue;
			for ( int y = n->PosY - 1; y <= n->PosY + 1; y++ )
			{
				if ( y < 0 || y >= (int)sqrt ( (double)MapSize ) ) continue;
				DetectMinesMap[x + (int)sqrt ( (double)MapSize )*y] = 1;
			}
		}
	}
	return n;
}

//--------------------------------------------------------------------------
/** initialize the maps */
//--------------------------------------------------------------------------
void cPlayer::InitMaps ( int MapSizeX, cMap *map )
{
	MapSize=MapSizeX*MapSizeX;
	// Scanner-Map:
	ScanMap = new char[MapSize];
	memset ( ScanMap,0,MapSize );
	// Ressource-Map
	ResourceMap = new char[MapSize];
	memset ( ResourceMap,0,MapSize );

	base.map = map;
	// Sentry-Map:
	SentriesMapAir = new char[MapSize];
	memset ( SentriesMapAir,0,MapSize );
	SentriesMapGround = new char[MapSize];
	memset ( SentriesMapGround,0,MapSize );

	// Detect-Maps:
	DetectLandMap = new char[MapSize];
	memset ( DetectLandMap,0,MapSize );
	DetectSeaMap = new char[MapSize];
	memset ( DetectSeaMap,0,MapSize );
	DetectMinesMap = new char[MapSize];
	memset ( DetectMinesMap, 0, MapSize );
}

//--------------------------------------------------------------------------
/** Adds the building to the list of the player */
//--------------------------------------------------------------------------
cBuilding *cPlayer::addBuilding ( int posx, int posy, sBuilding *b )
{
	cBuilding *Building;

	Building = new cBuilding ( b, this, &base );
	Building->PosX = posx;
	Building->PosY = posy;
	Building->prev = NULL;
	if ( BuildingList != NULL )
	{
		BuildingList->prev = Building;
	}
	Building->next = BuildingList;
	BuildingList = Building;
	Building->GenerateName();
	if ( Building->data.scan )
	{
		if ( Building->data.is_big ) drawSpecialCircleBig ( Building->PosX, Building->PosY, Building->data.scan, ScanMap, (int)sqrt ( (double)MapSize ) );
		else drawSpecialCircle ( Building->PosX, Building->PosY, Building->data.scan, ScanMap, (int)sqrt ( (double)MapSize ) );
	}
	return Building;
}

//--------------------------------------------------------------------------
void cPlayer::addSentryVehicle ( cVehicle *v )
{
	sSentry *n;
	n=new sSentry;
	n->b=NULL;
	n->v=v;
	if ( v->data.can_attack==ATTACK_AIR )
	{
		SentriesAir.Add ( n );
		drawSpecialCircle ( v->PosX,v->PosY,v->data.range,SentriesMapAir, (int)sqrt ( (double)MapSize ) );
	}
	else if ( v->data.can_attack==ATTACK_AIRnLAND )
	{
		SentriesAir.Add ( n );
		drawSpecialCircle ( v->PosX,v->PosY,v->data.range,SentriesMapAir, (int)sqrt ( (double)MapSize ) );
		n=new sSentry;
		n->b=NULL;
		n->v=v;
		SentriesGround.Add ( n );
		drawSpecialCircle ( v->PosX,v->PosY,v->data.range,SentriesMapGround, (int)sqrt ( (double)MapSize ) );
	}
	else
	{
		SentriesGround.Add ( n );
		drawSpecialCircle ( v->PosX,v->PosY,v->data.range,SentriesMapGround, (int)sqrt ( (double)MapSize ) );
	}
}

//--------------------------------------------------------------------------
void cPlayer::addSentryBuilding ( cBuilding *b )
{
	sSentry *n;
	n=new sSentry;
	n->b=b;
	n->v=NULL;
	if ( b->data.can_attack==ATTACK_AIR )
	{
		SentriesAir.Add ( n );
		drawSpecialCircle ( b->PosX,b->PosY,b->data.range,SentriesMapAir, (int)sqrt ( (double)MapSize ) );
	}
	else if ( b->data.can_attack==ATTACK_AIRnLAND )
	{
		SentriesAir.Add ( n );
		drawSpecialCircle ( b->PosX,b->PosY,b->data.range,SentriesMapAir, (int)sqrt ( (double)MapSize ) );
		n=new sSentry;
		n->b=b;
		n->v=NULL;
		SentriesGround.Add ( n );
		drawSpecialCircle ( b->PosX,b->PosY,b->data.range,SentriesMapGround, (int)sqrt ( (double)MapSize ) );
	}
	else
	{
		SentriesGround.Add ( n );
		drawSpecialCircle ( b->PosX,b->PosY,b->data.range,SentriesMapGround, (int)sqrt ( (double)MapSize ) );
	}
}

//--------------------------------------------------------------------------
void cPlayer::deleteSentryVehicle ( cVehicle *v )
{
	sSentry *ptr;
	if ( v->data.can_attack == ATTACK_AIR )
	{
		for ( unsigned int i = 0; i < SentriesAir.Size(); i++ )
		{
			ptr = SentriesAir[i];
			if ( ptr->v == v )
			{
				SentriesAir.Delete ( i );
				delete ptr;
				break;
			}
		}
		refreshSentryAir();
	}
	else if ( v->data.can_attack==ATTACK_AIRnLAND )
	{
		for ( unsigned int i = 0; i < SentriesAir.Size(); i++ )
		{
			ptr = SentriesAir[i];
			if ( ptr->v == v )
			{
				SentriesAir.Delete ( i );
				delete ptr;
				break;
			}
		}
		for ( unsigned int i = 0; i < SentriesGround.Size(); i++ )
		{
			ptr = SentriesGround[i];
			if ( ptr->v == v )
			{
				SentriesGround.Delete ( i );
				delete ptr;
				break;
			}
		}
		refreshSentryAir();
		refreshSentryGround();
	}
	else
	{
		for ( unsigned int i = 0; i < SentriesGround.Size(); i++ )
		{
			ptr = SentriesGround[i];
			if ( ptr->v == v )
			{
				SentriesGround.Delete ( i );
				delete ptr;
				break;
			}
		}
		refreshSentryGround();
	}
}

//--------------------------------------------------------------------------
void cPlayer::deleteSentryBuilding ( cBuilding *b )
{
	sSentry *ptr;
	if ( b->data.can_attack == ATTACK_AIR )
	{
		for ( unsigned int i = 0; i < SentriesAir.Size(); i++ )
		{
			ptr = SentriesAir[i];
			if ( ptr->b == b )
			{
				SentriesAir.Delete ( i );
				delete ptr;
				break;
			}
		}
		refreshSentryAir();
	}
	else if ( b->data.can_attack==ATTACK_AIRnLAND )
	{
		for ( unsigned int i = 0; i < SentriesAir.Size(); i++ )
		{
			ptr = SentriesAir[i];
			if ( ptr->b == b )
			{
				SentriesAir.Delete ( i );
				delete ptr;
				break;
			}
		}
		for ( unsigned int i = 0; i < SentriesGround.Size(); i++ )
		{
			ptr = SentriesGround[i];
			if ( ptr->b == b )
			{
				SentriesGround.Delete ( i );
				delete ptr;
				break;
			}
		}
		refreshSentryAir();
		refreshSentryGround();
	}
	else
	{
		for ( unsigned int i = 0; i < SentriesGround.Size(); i++ )
		{
			ptr = SentriesGround[i];
			if ( ptr->b == b )
			{
				SentriesGround.Delete ( i );
				delete ptr;
				break;
			}
		}
		refreshSentryGround();

	}
}

//--------------------------------------------------------------------------
void cPlayer::refreshSentryAir ()
{
	sSentry *ptr;
	memset ( SentriesMapAir,0,MapSize );
	for ( unsigned int i = 0; i < SentriesAir.Size(); i++ )
	{
		ptr = SentriesAir[i];
		if ( ptr->v )
		{
			drawSpecialCircle ( ptr->v->PosX, ptr->v->PosY, ptr->v->data.range, SentriesMapAir, (int)sqrt ( (double)MapSize ) );
		}
		else
		{
			drawSpecialCircle ( ptr->b->PosX, ptr->b->PosY, ptr->b->data.range, SentriesMapAir, (int)sqrt ( (double)MapSize ) );
		}
	}
}

//--------------------------------------------------------------------------
void cPlayer::refreshSentryGround ()
{
	sSentry *ptr;
	memset ( SentriesMapGround,0,MapSize );
	for ( unsigned int i = 0 ; i < SentriesGround.Size(); i++ )
	{
		ptr = SentriesGround[i];
		if ( ptr->v )
		{
			drawSpecialCircle ( ptr->v->PosX, ptr->v->PosY, ptr->v->data.range, SentriesMapGround, (int)sqrt ( (double)MapSize ) );
		}
		else
		{
			drawSpecialCircle ( ptr->b->PosX, ptr->b->PosY, ptr->b->data.range, SentriesMapGround, (int)sqrt ( (double)MapSize ) );
		}
	}
}

//--------------------------------------------------------------------------
/** Does a scan for all units of the player */
//--------------------------------------------------------------------------
void cPlayer::DoScan ()
{
	cVehicle *vp;
	cBuilding *bp;

	if ( isDefeated ) return;
	memset ( ScanMap,       0, MapSize );
	memset ( DetectLandMap, 0, MapSize );
	memset ( DetectSeaMap,  0, MapSize );
	memset ( DetectMinesMap,0, MapSize );
	
	// iterate the vehicle list
	vp = VehicleList;
	while ( vp )
	{
		if ( vp->Loaded )
		{
			vp = vp->next;
			continue;
		}

		if ( vp->Disabled )
			ScanMap[vp->PosX+vp->PosY*(int)sqrt ( (double)MapSize )] = 1;
		else
		{
			if ( vp->data.is_big )
				drawSpecialCircleBig ( vp->PosX, vp->PosY, vp->data.scan, ScanMap, (int)sqrt ( (double)MapSize ) );
			else
				drawSpecialCircle ( vp->PosX,vp->PosY,vp->data.scan,ScanMap, (int)sqrt ( (double)MapSize ) );

			//detection maps
			if ( vp->data.can_detect_land )
				drawSpecialCircle ( vp->PosX, vp->PosY, vp->data.scan, DetectLandMap, (int)sqrt ( (double)MapSize ) );
			else if ( vp->data.can_detect_sea )
				drawSpecialCircle ( vp->PosX, vp->PosY, vp->data.scan, DetectSeaMap, (int)sqrt ( (double)MapSize ) );
			if ( vp->data.can_detect_mines )
			{
				for ( int x = vp->PosX - 1; x <= vp->PosX + 1; x++ )
				{
					if ( x < 0 || x >= (int)sqrt ( (double)MapSize ) )
						continue;
					for ( int y = vp->PosY - 1; y <= vp->PosY + 1; y++ )
					{
						if ( y < 0 || y >= (int)sqrt ( (double)MapSize ) ) 
							continue;
						DetectMinesMap[x + (int)sqrt ( (double)MapSize )*y] = 1;
					}
				}
			}
		}
		vp = vp->next;
	}

	// iterate the building list
	bp = BuildingList;
	while ( bp )
	{

		if ( bp->Disabled )
			ScanMap[bp->PosX+bp->PosY*(int)sqrt ( (double)MapSize )]=1;
		else
		{
			if ( bp->data.scan )
			{
				if ( bp->data.is_big ) 
					drawSpecialCircleBig ( bp->PosX,bp->PosY,bp->data.scan,ScanMap, (int)sqrt ( (double)MapSize ) );
				else 
					drawSpecialCircle ( bp->PosX,bp->PosY,bp->data.scan,ScanMap, (int)sqrt ( (double)MapSize ) );
			}
		}
		bp = bp->next;
	}
}


//--------------------------------------------------------------------------
/** Returns the next vehicle that can still fire/shoot */
//--------------------------------------------------------------------------
cVehicle *cPlayer::GetNextVehicle ()
{
	cVehicle *v, *start;
	bool next = false;
	if ( Client->SelectedVehicle && Client->SelectedVehicle->owner == this )
	{
		start = Client->SelectedVehicle;
		next = true;
	}
	else
		start = VehicleList;

	if ( !start ) 
		return NULL;
	v = start;
	do
	{
		if ( !next && ( v->data.speed||v->data.shots ) && !v->IsBuilding && !v->IsClearing && !v->bSentryStatus && !v->Loaded ) 
			return v;
		next = false;
		if ( v->next )
			v = v->next;
		else
			v = VehicleList;
	}
	while ( v != start );
	return NULL;
}

//--------------------------------------------------------------------------
/** Returns the previous vehicle, that can still move / shoot */
//--------------------------------------------------------------------------
cVehicle *cPlayer::GetPrevVehicle ()
{
	cVehicle *v, *start;
	bool next = false;
	if ( Client->SelectedVehicle && Client->SelectedVehicle->owner == this )
	{
		start = Client->SelectedVehicle;
		next = true;
	}
	else
		start = VehicleList;
	if ( !start ) 
		return NULL;
	v = start;
	do
	{
		if ( !next && (v->data.speed || v->data.shots) && !v->IsBuilding && !v->IsClearing && !v->bSentryStatus && !v->Loaded ) 
			return v;
		next = false;
		if ( v->prev )
			v = v->prev;
		else
		{
			while ( v->next )
				v = v->next;
		}
	}
	while ( v != start );
	return NULL;
}

//--------------------------------------------------------------
/** Starts a research center. */
//--------------------------------------------------------------
void cPlayer::startAResearch (int researchArea)
{
	if (0 <= researchArea && researchArea <= cResearch::kNrResearchAreas)
	{
		ResearchCount++;
		researchCentersWorkingOnArea[researchArea] += 1;
	}
}

//--------------------------------------------------------------
/** Stops a research center. */
//--------------------------------------------------------------
void cPlayer::stopAResearch (int researchArea)
{
	if (0 <= researchArea && researchArea <= cResearch::kNrResearchAreas)
	{
		ResearchCount--;
		if (researchCentersWorkingOnArea[researchArea] > 0)
			researchCentersWorkingOnArea[researchArea] -= 1;
	}
}

//--------------------------------------------------------------
/** At turnend update the research level */
//--------------------------------------------------------------
void cPlayer::doResearch()
{
	bool researchFinished = false;
	cList<sUnitData*> upgradedUnitDatas; 
	cList<int> areasReachingNextLevel;
	for (int area = 0; area < cResearch::kNrResearchAreas; area++)
	{
		if (researchCentersWorkingOnArea[area] > 0)
		{
			if (researchLevel.doResearch(researchCentersWorkingOnArea[area], area))
			{ // next level reached
				areasReachingNextLevel.Add(area);
				researchFinished = true;
			}
		}
	}
	if (researchFinished)
	{
		upgradeUnitTypes (areasReachingNextLevel, upgradedUnitDatas);
		
		for (unsigned int i = 0; i < upgradedUnitDatas.Size(); i++)
			sendUnitUpgrades (upgradedUnitDatas[i], Nr);
	}
	sendResearchLevel (&researchLevel, Nr);
	
	reportResearchFinished = researchFinished;
}

//--------------------------------------------------------------
void cPlayer::upgradeUnitTypes (cList<int>& areasReachingNextLevel, cList<sUnitData*>& resultUpgradedUnitDatas)
{
	for (unsigned int i = 0; i < UnitsData.vehicle.Size(); i++)
	{
		bool incrementVersion = false;
		for (unsigned int areaCounter = 0; areaCounter < areasReachingNextLevel.Size(); areaCounter++)
		{
			int researchArea = areasReachingNextLevel[areaCounter];
			int newResearchLevel = researchLevel.getCurResearchLevel (researchArea);
		
			int startValue = 0;
			switch (researchArea)
			{
				case cResearch::kAttackResearch: startValue = UnitsData.vehicle[i].data.damage; break;
				case cResearch::kShotsResearch: startValue = UnitsData.vehicle[i].data.max_shots; break;
				case cResearch::kRangeResearch: startValue = UnitsData.vehicle[i].data.range; break;
				case cResearch::kArmorResearch: startValue = UnitsData.vehicle[i].data.armor; break;
				case cResearch::kHitpointsResearch: startValue = UnitsData.vehicle[i].data.max_hit_points; break;
				case cResearch::kScanResearch: startValue = UnitsData.vehicle[i].data.scan; break;
				case cResearch::kSpeedResearch: startValue = UnitsData.vehicle[i].data.max_speed; break;
				case cResearch::kCostResearch: startValue = UnitsData.vehicle[i].data.iBuilt_Costs; break;
			}
			int oldResearchBonus = cUpgradeCalculator::instance().calcChangeByResearch(startValue, newResearchLevel - 10, 
																					   researchArea == cResearch::kCostResearch ? cUpgradeCalculator::kCost : -1,
																					   UnitsData.vehicle[i].data.is_human ? cUpgradeCalculator::kInfantry : cUpgradeCalculator::kStandardUnit);
			int newResearchBonus = cUpgradeCalculator::instance().calcChangeByResearch(startValue, newResearchLevel,
																					   researchArea == cResearch::kCostResearch ? cUpgradeCalculator::kCost : -1,
																					   UnitsData.vehicle[i].data.is_human ? cUpgradeCalculator::kInfantry : cUpgradeCalculator::kStandardUnit);
			if (oldResearchBonus != newResearchBonus)
			{
				switch (researchArea)
				{
					case cResearch::kAttackResearch: VehicleData[i].damage += newResearchBonus - oldResearchBonus; break;
					case cResearch::kShotsResearch: VehicleData[i].max_shots += newResearchBonus - oldResearchBonus; break;
					case cResearch::kRangeResearch: VehicleData[i].range += newResearchBonus - oldResearchBonus; break;
					case cResearch::kArmorResearch: VehicleData[i].armor += newResearchBonus - oldResearchBonus; break;
					case cResearch::kHitpointsResearch: VehicleData[i].max_hit_points += newResearchBonus - oldResearchBonus; break;
					case cResearch::kScanResearch: VehicleData[i].scan += newResearchBonus - oldResearchBonus; break;
					case cResearch::kSpeedResearch: VehicleData[i].max_speed += newResearchBonus - oldResearchBonus; break;
					case cResearch::kCostResearch: VehicleData[i].iBuilt_Costs += newResearchBonus - oldResearchBonus; break;
				}
				if (researchArea != cResearch::kCostResearch) // don't increment the version, if the only change are the costs
					incrementVersion = true;
				if (!resultUpgradedUnitDatas.Contains(&(VehicleData[i])))
					resultUpgradedUnitDatas.Add(&(VehicleData[i]));
			}
		}
		if (incrementVersion)
			VehicleData[i].version += 1;
	}
	
	for (unsigned int i = 0; i < UnitsData.building.Size(); i++)
	{
		bool incrementVersion = false;
		for (unsigned int areaCounter = 0; areaCounter < areasReachingNextLevel.Size(); areaCounter++)
		{
			int researchArea = areasReachingNextLevel[areaCounter];
			int newResearchLevel = researchLevel.getCurResearchLevel (researchArea);
			
			int startValue = 0;
			switch (researchArea)
			{
				case cResearch::kAttackResearch: startValue = UnitsData.building[i].data.damage; break;
				case cResearch::kShotsResearch: startValue = UnitsData.building[i].data.max_shots; break;
				case cResearch::kRangeResearch: startValue = UnitsData.building[i].data.range; break;
				case cResearch::kArmorResearch: startValue = UnitsData.building[i].data.armor; break;
				case cResearch::kHitpointsResearch: startValue = UnitsData.building[i].data.max_hit_points; break;
				case cResearch::kScanResearch: startValue = UnitsData.building[i].data.scan; break;
				case cResearch::kCostResearch: startValue = UnitsData.building[i].data.iBuilt_Costs; break;
			}
			int oldResearchBonus = cUpgradeCalculator::instance().calcChangeByResearch(startValue, newResearchLevel - 10, 
																					   researchArea == cResearch::kCostResearch ? cUpgradeCalculator::kCost : -1,
																					   cUpgradeCalculator::kBuilding);
			int newResearchBonus = cUpgradeCalculator::instance().calcChangeByResearch(startValue, newResearchLevel,
																					   researchArea == cResearch::kCostResearch ? cUpgradeCalculator::kCost : -1,
																					   cUpgradeCalculator::kBuilding);
			if (oldResearchBonus != newResearchBonus)
			{
				switch (researchArea)
				{
					case cResearch::kAttackResearch: BuildingData[i].damage += newResearchBonus - oldResearchBonus; break;
					case cResearch::kShotsResearch: BuildingData[i].max_shots += newResearchBonus - oldResearchBonus; break;
					case cResearch::kRangeResearch: BuildingData[i].range += newResearchBonus - oldResearchBonus; break;
					case cResearch::kArmorResearch: BuildingData[i].armor += newResearchBonus - oldResearchBonus; break;
					case cResearch::kHitpointsResearch: BuildingData[i].max_hit_points += newResearchBonus - oldResearchBonus; break;
					case cResearch::kScanResearch: BuildingData[i].scan += newResearchBonus - oldResearchBonus; break;
					case cResearch::kCostResearch: BuildingData[i].iBuilt_Costs += newResearchBonus - oldResearchBonus; break;
				}
				if (researchArea != cResearch::kCostResearch) // don't increment the version, if the only change are the costs
					incrementVersion = true;
				if (!resultUpgradedUnitDatas.Contains(&(BuildingData[i])))
					resultUpgradedUnitDatas.Add(&(BuildingData[i]));
			}
		}
		if (incrementVersion)
			BuildingData[i].version += 1;
	}
}

//------------------------------------------------------------
void cPlayer::refreshResearchCentersWorkingOnArea()
{
	int newResearchCount = 0;
	for (int i = 0; i < cResearch::kNrResearchAreas; i++)
		researchCentersWorkingOnArea[i] = 0;
	cBuilding* curBuilding = BuildingList;
	while (curBuilding)
	{
		if (curBuilding->data.can_research && curBuilding->IsWorking)
		{
			researchCentersWorkingOnArea[curBuilding->researchArea] += 1;
			newResearchCount++;
		}
		curBuilding = curBuilding->next;
	}
	ResearchCount = newResearchCount;
}

//------------------------------------------------------------
/** Adds a building to the lock list (the range/scan of the building will remain displayed, even if it is unselected). */
//------------------------------------------------------------
void cPlayer::AddLock (cBuilding *b)
{
	sLockElem *elem;
	elem = new sLockElem;
	elem->b = b;
	elem->v = NULL;
	b->IsLocked = true;
	LockList.Add (elem);
}

//------------------------------------------------------------
/** Adds a vehicle to the lock list (the range/scan of the vehicle will remain displayed, even if it is unselected). */
//------------------------------------------------------------
void cPlayer::AddLock (cVehicle *v)
{
	sLockElem *elem;
	elem = new sLockElem;
	elem->v = v;
	elem->b = NULL;
	v->IsLocked = true;
	LockList.Add (elem);
}

//------------------------------------------------------------
/** Removes a vehicle from the lock list. */
//------------------------------------------------------------
void cPlayer::DeleteLock (cVehicle *v)
{
	sLockElem *elem;
	for (unsigned int i = 0; i < LockList.Size(); i++)
	{
		elem = LockList[i];
		if (elem->v == v)
		{
			v->IsLocked=false;
			delete elem;
			LockList.Delete (i);
			return;
		}
	}
}

//------------------------------------------------------------
/** Removes a building from the lock list. */
//------------------------------------------------------------
void cPlayer::DeleteLock (cBuilding *b)
{
	sLockElem *elem;
	for (unsigned int i = 0; i < LockList.Size(); i++)
	{
		elem = LockList[i];
		if (elem->b == b)
		{
			b->IsLocked = false;
			delete elem;
			LockList.Delete (i);
			return;
		}
	}
}

//------------------------------------------------------------
/** Checks if the building is contained in the lock list. */
//------------------------------------------------------------
bool cPlayer::InLockList (cBuilding *b)
{
	sLockElem *elem;
	for (unsigned int i = 0; i < LockList.Size(); i++)
	{
		elem = LockList[i];
		if (elem->b == b) 
			return true;
	}
	return false;
}

//------------------------------------------------------------
/** Checks if the vehicle is contained in the lock list. */
//------------------------------------------------------------
bool cPlayer::InLockList (cVehicle *v)
{
	sLockElem *elem;
	for (unsigned int i = 0; i < LockList.Size(); i++)
	{
		elem = LockList[i];
		if (elem->v == v)
			return true;
	}
	return false;
}

//--------------------------------------------------------------------------
/** Toggles the lock state of a unit under the mouse (when locked it's range and scan is displayed, although the unit is not selected). */
//--------------------------------------------------------------------------
void cPlayer::ToggelLock ( cMapField *OverUnitField )
{
	if ( OverUnitField->getBaseBuilding() && OverUnitField->getBaseBuilding()->owner!=this )
	{
		if ( InLockList ( OverUnitField->getBaseBuilding() ) ) DeleteLock ( OverUnitField->getBaseBuilding() );
		else AddLock ( OverUnitField->getBaseBuilding() );
	}
	if ( OverUnitField->getTopBuilding() && OverUnitField->getTopBuilding()->owner!=this )
	{
		if ( InLockList ( OverUnitField->getTopBuilding() ) ) DeleteLock ( OverUnitField->getTopBuilding() );
		else AddLock ( OverUnitField->getTopBuilding() );
	}
	if ( OverUnitField->getVehicles() && OverUnitField->getVehicles()->owner!=this )
	{
		if ( InLockList ( OverUnitField->getVehicles() ) ) DeleteLock ( OverUnitField->getVehicles() );
		else AddLock ( OverUnitField->getVehicles() );
	}
	if ( OverUnitField->getPlanes() && OverUnitField->getPlanes()->owner!=this )
	{
		if ( InLockList ( OverUnitField->getPlanes() ) ) DeleteLock ( OverUnitField->getPlanes() );
		else AddLock ( OverUnitField->getPlanes() );
	}
}

//--------------------------------------------------------------------------
/** Draws all entries, that are in the lock list. */
//--------------------------------------------------------------------------
void cPlayer::DrawLockList (cHud const& hud)
{
	if ( !hud.Lock ) return;
	sLockElem *elem;
	int spx, spy, off;

	for ( unsigned int i = 0; i < LockList.Size(); i++ )
	{
		elem = LockList[i];
		if ( elem->v )
		{
			off=elem->v->PosX+elem->v->PosY*Client->Map->size;
			if ( !ScanMap[off] )
			{
				DeleteLock ( elem->v );
				i--;
				continue;
			}
			spx=elem->v->GetScreenPosX();
			spy=elem->v->GetScreenPosY();

			if ( hud.Scan )
			{
				if ( elem->v->data.is_big )
					drawCircle ( spx+ hud.Zoom, spy + hud.Zoom, elem->v->data.scan * hud.Zoom, SCAN_COLOR, buffer );
				else
					drawCircle ( spx + hud.Zoom/2, spy + hud.Zoom/2, elem->v->data.scan * hud.Zoom, SCAN_COLOR, buffer );
			}
			if ( hud.Reichweite&& ( elem->v->data.can_attack==ATTACK_LAND||elem->v->data.can_attack==ATTACK_SUB_LAND ) )
				drawCircle ( spx+hud.Zoom/2,
				             spy+hud.Zoom/2,
				             elem->v->data.range*hud.Zoom+1,RANGE_GROUND_COLOR,buffer );
			if ( hud.Reichweite&&elem->v->data.can_attack==ATTACK_AIR )
				drawCircle ( spx+hud.Zoom/2,
				             spy+hud.Zoom/2,
				             elem->v->data.range*hud.Zoom+2,RANGE_AIR_COLOR,buffer );
			if ( hud.Munition&&elem->v->data.can_attack )
				elem->v->DrawMunBar();
			if ( hud.Treffer )
				elem->v->drawHealthBar();
		}
		else if ( elem->b )
		{
			off=elem->b->PosX+elem->b->PosY*Client->Map->size;
			if ( !ScanMap[off] )
			{
				DeleteLock ( elem->b );
				i--;
				continue;
			}
			spx=elem->b->GetScreenPosX();
			spy=elem->b->GetScreenPosY();

			if ( hud.Scan )
			{
				if ( elem->b->data.is_big )
					drawCircle ( spx+hud.Zoom,
					             spy+hud.Zoom,
					             elem->b->data.scan*hud.Zoom,SCAN_COLOR,buffer );
				else
					drawCircle ( spx+hud.Zoom/2,
					             spy+hud.Zoom/2,
					             elem->b->data.scan*hud.Zoom,SCAN_COLOR,buffer );
			}
			if ( hud.Reichweite&& ( elem->b->data.can_attack==ATTACK_LAND||elem->b->data.can_attack==ATTACK_SUB_LAND ) &&!elem->b->data.is_expl_mine )
				drawCircle ( spx+hud.Zoom/2,
				             spy+hud.Zoom/2,
				             elem->b->data.range*hud.Zoom+2,RANGE_GROUND_COLOR,buffer );
			if ( hud.Reichweite&&elem->b->data.can_attack==ATTACK_AIR )
				drawCircle ( spx+hud.Zoom/2,
				             spy+hud.Zoom/2,
				             elem->b->data.range*hud.Zoom+2,RANGE_AIR_COLOR,buffer );

			if ( hud.Munition&&elem->b->data.can_attack&&!elem->b->data.is_expl_mine )
				elem->b->DrawMunBar();
			if ( hud.Treffer )
				elem->b->DrawHelthBar();
		}
	}
}

//--------------------------------------------------------------------------
void cPlayer::drawSpecialCircle( int iX, int iY, int iRadius, char *map, int mapsize )
{
	float w = (float)(0.017453*45), step;
	int rx, ry, x1, x2;
	if ( iRadius <= 0 ) return;
	iRadius *= 10;
	step = (float)(0.017453*90-acos ( 1.0/iRadius ));
	step /= 2;
	for ( float i = 0; i <= w; i += step )
	{
		rx= ( int ) ( cos ( i ) *iRadius );
		ry= ( int ) ( sin ( i ) *iRadius);
		rx /= 10;
		ry /= 10;

		x1 = rx+iX;
		x2 = -rx+iX;
		for ( int k = x2; k <= x1; k++ )
		{
			if ( k < 0 ) continue;
			if ( k >= mapsize ) break;
			if ( iY+ry >= 0 && iY+ry < mapsize )
				map[k+ ( iY+ry ) *mapsize] |= 1;
			if ( iY-ry >= 0 && iY-ry < mapsize )
				map[k+ ( iY-ry ) *mapsize] |= 1;
		}

		x1 = ry+iX;
		x2 = -ry+iX;
		for ( int k = x2; k <= x1; k++ )
		{
			if ( k < 0 ) continue;
			if ( k >= mapsize ) break;
			if ( iY+rx >= 0 && iY+rx < mapsize )
				map[k+ ( iY+rx ) *mapsize] |= 1;
			if ( iY-rx >= 0 && iY-rx < mapsize )
				map[k+ ( iY-rx ) *mapsize] |= 1;
		}
	}
}

//--------------------------------------------------------------------------
void cPlayer::drawSpecialCircleBig( int iX, int iY, int iRadius, char *map, int mapsize )
{
	float w=(float)(0.017453*45), step;
	int rx, ry, x1, x2;
	if ( iRadius > 0 ) iRadius--;
	else return;
	iRadius *= 10;
	step = (float)(0.017453*90-acos ( 1.0/iRadius ));
	step /= 2;
	for ( float i = 0; i <= w; i += step )
	{
		rx= ( int ) ( cos ( i ) *iRadius );
		ry= ( int ) ( sin ( i ) *iRadius);
		rx /= 10;
		ry /= 10;

		x1 = rx+iX;
		x2 = -rx+iX;
		for ( int k = x2; k <= x1+1; k++ )
		{
			if ( k < 0 ) continue;
			if ( k >= mapsize ) break;
			if ( iY+ry >= 0 && iY+ry < mapsize )
				map[k+ ( iY+ry ) *mapsize] |= 1;
			if ( iY-ry >= 0 && iY-ry < mapsize )
				map[k+ ( iY-ry ) *mapsize] |= 1;

			if( iY+ry+1 >= 0&& iY+ry+1 < mapsize)
				map[k+( iY+ry+1 ) *mapsize] |= 1;
			if( iY-ry+1 >= 0 && iY-ry+1 < mapsize)
				map[k+( iY-ry+1 ) *mapsize] |= 1;
		}

		x1 = ry+iX;
		x2 = -ry+iX;
		for ( int k = x2; k <= x1+1; k++ )
		{
			if ( k < 0 ) continue;
			if ( k >= mapsize ) break;
			if ( iY+rx >= 0 && iY+rx < mapsize )
				map[k+ ( iY+rx ) *mapsize] |= 1;
			if ( iY-rx >= 0 && iY-rx < mapsize )
				map[k+ ( iY-rx ) *mapsize] |= 1;

			if( iY+rx+1 >= 0 && iY+rx+1 < mapsize)
				map[k+( iY+rx+1 ) *mapsize] |= 1;
			if( iY-rx+1 >= 0 && iY-rx+1 < mapsize)
				map[k+( iY-rx+1 ) *mapsize] |= 1;
		}

	}
}
