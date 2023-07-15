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

#include "utility/log.h"

#include <iostream>
#include <sstream>
#include <thread>

cLog Log;
cLog NetLog;

namespace
{
	std::string toString (const std::thread::id& id)
	{
		std::stringstream ss;
		ss.imbue (std::locale ("C"));
		ss << id;
		return ss.str();
	}
} // namespace

//------------------------------------------------------------------------------
void cLog::info (const std::string& msg)
{
	writeToFile ("Thread " + toString (std::this_thread::get_id()) + ": (II): " + msg + "\n");
}

//------------------------------------------------------------------------------
void cLog::warn (const std::string& msg)
{
	writeToFile ("Thread " + toString (std::this_thread::get_id()) + ": (WW): " + msg + "\n");
}

//------------------------------------------------------------------------------
void cLog::debug (const std::string& msg)
{
	if (!isPrintingDebug)
	{
		//in case debug is disabled we skip message
		return;
	}
	writeToFile ("Thread " + toString (std::this_thread::get_id()) + ": (DD): " + msg + "\n");
}

//------------------------------------------------------------------------------
void cLog::error (const std::string& msg)
{
	auto fullmsg = "Thread " + toString (std::this_thread::get_id()) + ": (EE): " + msg + "\n";
	std::cout << fullmsg << "\n";
	writeToFile (fullmsg);
}

//------------------------------------------------------------------------------
void cLog::mark()
{
	writeToFile ("==============================(MARK)==============================\n");
}

//------------------------------------------------------------------------------
void cLog::setLogPath (const std::filesystem::path& path)
{
	std::unique_lock<std::mutex> l (mutex);
	if (logfile.is_open())
	{
		//file is already open
		return;
	}

	//create + open new log file
	logfile.open (path, std::fstream::out | std::fstream::trunc);
	if (!logfile.is_open())
	{
		std::cerr << "(EE): Couldn't open " << path << "!\n Please check file / directory permissions\n";
	}
}

//------------------------------------------------------------------------------
void cLog::writeToFile (const std::string& msg)
{
	std::unique_lock<std::mutex> l (mutex);
	logfile.write (msg.c_str(), msg.length());
	logfile.flush();

	if (logfile.bad())
	{
		std::cerr << msg;
	}
}
