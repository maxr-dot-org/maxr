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

#ifndef utility_logH
#define utility_logH

#include <fstream>
#include <mutex>
#include <string>

class cLog
{
public:
	enum class eLogType
	{
		Debug,
		Info,
		Warning,
		Error,
		NetDebug,
		NetWarning,
		NetError
	};

	/**
	* Writes message with given type to logfile
	*
	* @param str Message for the log
	* @param type Type for the log
	*/
	void write (const std::string& msg, eLogType type = eLogType::Info);

	void warn (const std::string& msg);

	/**
	* Writes a marker into logfile - please use only very few times!
	*/
	void mark();

private:
	void checkOpenFile (eLogType);
	void writeToFile (const std::string& msg, std::ofstream& file);

private:
	std::mutex mutex;
	std::ofstream logfile;
	std::ofstream netLogfile;
};

extern cLog Log;

#endif // utility_logH
