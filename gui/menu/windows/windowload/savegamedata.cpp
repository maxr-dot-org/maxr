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

#include "savegamedata.h"

//------------------------------------------------------------------------------
cSaveGameData::cSaveGameData () :
	number (-1)
{}

//------------------------------------------------------------------------------
cSaveGameData::cSaveGameData (std::string fileName_, std::string gameName_, std::string type_, std::string date_, int number_) :
	fileName (std::move (fileName_)),
	gameName (std::move (gameName_)),
	type (std::move (type_)),
	date (std::move (date_)),
	number (number_)
{}

//------------------------------------------------------------------------------
const std::string& cSaveGameData::getFileName () const
{
	return fileName;
}

//------------------------------------------------------------------------------
void cSaveGameData::setFileName (std::string name)
{
	fileName = name;
}

//------------------------------------------------------------------------------
const std::string& cSaveGameData::getGameName () const
{
	return gameName;
}

//------------------------------------------------------------------------------
void cSaveGameData::setGameName (std::string name)
{
	gameName = name;
}

//------------------------------------------------------------------------------
const std::string& cSaveGameData::getType () const
{
	return type;
}

//------------------------------------------------------------------------------
void cSaveGameData::setType (std::string type_)
{
	type = type_;
}

//------------------------------------------------------------------------------
const std::string& cSaveGameData::getDate () const
{
	return date;
}

//------------------------------------------------------------------------------
void cSaveGameData::setDate (std::string date_)
{
	date = date_;
}

//------------------------------------------------------------------------------
int cSaveGameData::getNumber () const
{
	return number;
}

//------------------------------------------------------------------------------
void cSaveGameData::setNumber (int number_)
{
	number = number_;
}