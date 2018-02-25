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


#include "serialization.h"
#include "game/data/model.h"
#include "utility/log.h"
#include "main.h"

namespace serialization
{

	cPointerLoader::cPointerLoader(cModel& model) :
		model(model)
	{}

	void cPointerLoader::get(int id, cJob*& value) const
	{
		assert(false);
		//TODO
	}

	void cPointerLoader::get(int id, cPlayer*& value) const
	{
		value = model.getPlayer(id);
		if (value == nullptr && id != -1)
			Log.write("Player with id " + iToStr(id) + " not found.", cLog::eLOG_TYPE_NET_ERROR);
	}

	void cPointerLoader::get(int id, const cPlayer*& value) const
	{
		value = model.getPlayer(id);
		if (value == nullptr && id != -1)
			Log.write("Player with id " + iToStr(id) + " not found.", cLog::eLOG_TYPE_NET_ERROR);
	}

	void cPointerLoader::get(int id, cBuilding*& value) const
	{
		value = model.getBuildingFromID(id);
		if (value == nullptr && id != -1)
			Log.write("Building with id " + iToStr(id) + " not found.", cLog::eLOG_TYPE_NET_ERROR);
	}

	void cPointerLoader::get(int id, cVehicle*& value) const
	{
		value = model.getVehicleFromID(id);
		if (value == nullptr && id != -1)
			Log.write("Vehicle with id " + iToStr(id) + " not found.", cLog::eLOG_TYPE_NET_ERROR);
	}

	void cPointerLoader::get(int id, cUnit*& value) const
	{
		value = model.getUnitFromID(id);
		if (value == nullptr && id != -1)
			Log.write("Unit with id " + iToStr(id) + " not found.", cLog::eLOG_TYPE_NET_ERROR);
	}

	void cPointerLoader::get(sID id, const cStaticUnitData*& value) const
	{
		if (!model.getUnitsData()->isValidId(id))
		{
			Log.write("Static unit data for sID " + id.getText() + " not found.", cLog::eLOG_TYPE_NET_ERROR);
			throw std::runtime_error("Error restoring pointer to static unitdata");
		}
		value = &model.getUnitsData()->getStaticUnitData(id);
	}

	const cStaticUnitData* cPointerLoader::getBigRubbleData() const
	{
		return &model.getUnitsData()->getRubbleBigData();
	}

	const cStaticUnitData* cPointerLoader::getSmallRubbleData() const
	{
		return &model.getUnitsData()->getRubbleSmallData();
	}

}
