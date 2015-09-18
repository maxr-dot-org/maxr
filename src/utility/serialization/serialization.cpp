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

namespace serialization
{

	cPointerLoader::cPointerLoader(cModel& model) :
		model(model)
	{}

	void cPointerLoader::get(unsigned int id, cJob*& value)
	{
		assert(false);
		//TODO
	}

	void cPointerLoader::get(unsigned int id, cPlayer*& value)
	{
		value = model.getPlayer(id);
		if (value == nullptr && id != -1)
			Log.write("Player with id " + iToStr(id) + " not found.", cLog::eLOG_TYPE_NET_ERROR);
	}

	void cPointerLoader::get(unsigned int id, cBuilding*& value)
	{
		value = model.getBuildingFromID(id);
		if (value == nullptr && id != -1)
			Log.write("Building with id " + iToStr(id) + " not found.", cLog::eLOG_TYPE_NET_ERROR);
	}

	void cPointerLoader::get(unsigned int id, cVehicle*& value)
	{
		value = model.getVehicleFromID(id);
		if (value == nullptr && id != -1)
			Log.write("Vehicle with id " + iToStr(id) + " not found.", cLog::eLOG_TYPE_NET_ERROR);
	}

	void cPointerLoader::get(unsigned int id, cUnit*& value)
	{
		value = model.getUnitFromID(id);
		if (value == nullptr && id != -1)
			Log.write("Unit with id " + iToStr(id) + " not found.", cLog::eLOG_TYPE_NET_ERROR);
	}

	template<typename T>
	void cPointerLoader::get(unsigned int id, T*& value)
	{
		//Pointer type not implemented
		assert(false);
	}

}
