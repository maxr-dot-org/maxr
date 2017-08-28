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

#ifndef game_logic_actionStartBuildH
#define game_logic_actionStartBuildH

#include "action.h"

class cUnit;

class cActionStartBuild : public cAction
{
public:
	cActionStartBuild(const cVehicle& vehicle, sID buildingTypeID, int buildSpeed, const cPosition& buildPosition);
	cActionStartBuild(const cVehicle& vehicle, sID buildingTypeID, int buildSpeed, const cPosition& buildPosition, const cPosition& pathEndPosition);
	cActionStartBuild(cBinaryArchiveOut& archive);

	virtual void serialize(cBinaryArchiveIn& archive) { cAction::serialize(archive); serializeThis(archive); }
	virtual void serialize(cTextArchiveIn& archive)   { cAction::serialize(archive); serializeThis(archive); }

	virtual void execute(cModel& model) const override;
private:
	template<typename T>
	void serializeThis(T& archive)
	{
		archive & vehicleID;
		archive & buildingTypeID;
		archive & buildSpeed;
		archive & buildPosition;
		archive & buildPath;
		archive & pathEndPosition;
	}

	int vehicleID;
	sID buildingTypeID;
	int buildSpeed;
	cPosition buildPosition;
	bool buildPath; 
	cPosition pathEndPosition;
};

#endif // game_logic_actionTransferH
