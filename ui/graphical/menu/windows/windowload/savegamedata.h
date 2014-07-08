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
#ifndef ui_graphical_menu_windows_windowload_savegamedataH
#define ui_graphical_menu_windows_windowload_savegamedataH

#include <string>

class cSaveGameData
{
public:
	cSaveGameData ();
	cSaveGameData (std::string fileName, std::string gameName, std::string type, std::string date, int number);

	const std::string& getFileName () const;
	void setFileName (std::string name);

	const std::string& getGameName () const;
	void setGameName (std::string name);

	const std::string& getType () const;
	void setType (std::string type);

	const std::string& getDate () const;
	void setDate (std::string date);

	int getNumber () const;
	void setNumber (int number);

private:
	/** the file name of the save game */
	std::string fileName;
	/** the displayed name of the save game */
	std::string gameName;
	/** the type of the save game (SIN, HOT, NET) */
	std::string type;
	/** the date and time when this save game was saved */
	std::string date;

	/** the number of the save game */
	int number;
};

#endif // ui_graphical_menu_windows_windowload_savegamedataH
