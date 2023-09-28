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

#include "actionchangeresearch.h"

#include "game/data/model.h"

//------------------------------------------------------------------------------
cActionChangeResearch::cActionChangeResearch (const std::array<int, cResearch::kNrResearchAreas>& researchAreas) :
	researchAreas (researchAreas)
{}

//------------------------------------------------------------------------------
cActionChangeResearch::cActionChangeResearch (cBinaryArchiveIn& archive)
{
	serializeThis (archive);
}

//------------------------------------------------------------------------------
void cActionChangeResearch::execute (cModel& model) const
{
	//Note: this function handles incoming data from network. Make every possible sanity check!

	auto player = model.getPlayer (playerNr);
	if (player == nullptr) return;

	int newUsedResearch = 0;
	for (int nrCenters : researchAreas)
	{
		if (nrCenters < 0) return;
		newUsedResearch += nrCenters;
	}
	if (newUsedResearch > player->getResearchCentersWorkingTotal())
	{
		return; // can't turn on research centers automatically!
	}

	// needed, if newUsedResearch < player->ResearchCount
	std::vector<cBuilding*> researchCentersToStop;
	std::vector<cBuilding*> researchCentersToChangeArea;
	std::vector<cResearch::eResearchArea> newAreasForResearchCenters;

	bool error = false;
	const auto buildings = player->getBuildings();
	auto currentBuildingIter = buildings.begin();
	for (int newArea = 0; newArea != cResearch::kNrResearchAreas; ++newArea)
	{
		int centersToAssign = researchAreas[newArea];
		for (; currentBuildingIter != buildings.end() && centersToAssign > 0; ++currentBuildingIter)
		{
			auto& building = *currentBuildingIter;
			if (building->getStaticData().canResearch && building->isUnitWorking())
			{
				researchCentersToChangeArea.push_back (building.get());
				newAreasForResearchCenters.push_back ((cResearch::eResearchArea) newArea);
				--centersToAssign;
			}
		}
		if (centersToAssign > 0)
		{
			error = true; // not enough active research centers!
			break;
		}
	}
	// shut down unused research centers
	for (; currentBuildingIter != buildings.end(); ++currentBuildingIter)
	{
		auto& building = *currentBuildingIter;
		if (building->getStaticData().canResearch && building->isUnitWorking())
			researchCentersToStop.push_back (building.get());
	}
	if (error)
		return;

	for (auto* researchCenter : researchCentersToStop)
		researchCenter->stopWork (false);

	for (size_t i = 0; i != researchCentersToChangeArea.size(); ++i)
		researchCentersToChangeArea[i]->setResearchArea (newAreasForResearchCenters[i]);
	player->refreshResearchCentersWorkingOnArea();
}
