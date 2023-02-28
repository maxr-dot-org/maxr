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

#include "settings.h"
#include "utility/os.h"

#include <ctime>
#include <iostream>
#include <sstream>
#include <thread>

cLog Log;

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
void cLog::write (const std::string& msg, eLogType type)
{
	if ((type == eLogType::Debug || type == eLogType::NetDebug) && !cSettings::getInstance().isDebug())
	{
		//in case debug is disabled we skip message
		return;
	}
	std::unique_lock<std::mutex> l (mutex);

	checkOpenFile (type);

	//attach log message type to string
	std::string tmp;
	auto threadId = "Thread " + toString (std::this_thread::get_id());
	switch (type)
	{
		case eLogType::NetWarning:
		case eLogType::Warning:
			tmp = threadId + ": (WW): " + msg;
			break;
		case eLogType::NetError:
		case eLogType::Error:
			tmp = threadId + ": (EE): " + msg;
			std::cout << tmp << "\n";
			break;
		case eLogType::NetDebug:
		case eLogType::Debug:
			tmp = threadId + ": (DD): " + msg;
			break;
		case eLogType::Info:
		default:
			tmp = threadId + ": (II): " + msg;
			break;
	}
	tmp += '\n';

	if (type == eLogType::NetDebug || type == eLogType::NetWarning || type == eLogType::NetError)
	{
		writeToFile (tmp, netLogfile);
	}
	else
	{
		writeToFile (tmp, logfile);
	}
}

//------------------------------------------------------------------------------
void cLog::warn (const std::string& msg)
{
	write (msg, eLogType::Warning);
}

//------------------------------------------------------------------------------
void cLog::mark()
{
	std::unique_lock<std::mutex> l (mutex);

	checkOpenFile (eLogType::Info);

	std::string str = "==============================(MARK)==============================";
	str += '\n';

	writeToFile (str, logfile);
}

//------------------------------------------------------------------------------
void cLog::checkOpenFile (eLogType type)
{
	if (type == eLogType::NetDebug || type == eLogType::NetWarning || type == eLogType::NetError)
	{
		if (netLogfile.is_open())
		{
			//file is already open
			return;
		}
		cSettings::getInstance().setNetLogPath (cSettings::getInstance().getUserLogDir() / os::formattedNow ( "%Y-%m-%d-%H%M%S_net.log"));

		//create + open new log file
		netLogfile.open (cSettings::getInstance().getNetLogPath().string(), std::fstream::out | std::fstream::trunc);
		if (!netLogfile.is_open())
		{
			std::cerr << "(EE): Couldn't open net.log!\n Please check file/directory permissions\n";
		}
	}
	else
	{
		if (logfile.is_open())
		{
			//file is already open
			return;
		}

		//create + open new log file
		logfile.open (cSettings::getInstance().getLogPath().string(), std::fstream::out | std::fstream::trunc);
		if (!logfile.is_open())
		{
			std::cerr << "(EE): Couldn't open maxr.log!\n Please check file/directory permissions\n";
		}
	}
}

//------------------------------------------------------------------------------
void cLog::writeToFile (const std::string& msg, std::ofstream& file)
{
	file.write (msg.c_str(), msg.length());
	file.flush();

	if (file.bad())
	{
		std::cerr << msg;
	}
}
