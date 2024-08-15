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

#include <atomic>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>

class cLog
{
public:
	void info (const std::string& msg);
	void warn (const std::string& msg);
	void debug (const std::string& msg);
	void error (const std::string& msg);

	/**
	* Writes a marker into logfile - please use only very few times!
	*/
	void mark();

	void setLogPath (const std::filesystem::path&);
	void showDebug (bool b) { isPrintingDebug = b; }

private:
	void writeToFile (const std::string& msg);

private:
	std::mutex mutex;
	std::atomic<bool> isPrintingDebug{true};
	std::ofstream logfile;
};

extern cLog Log;
extern cLog NetLog;

#endif // utility_logH
