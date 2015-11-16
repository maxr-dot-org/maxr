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
 
#include <ctime>
#include <iostream>
 
#include "utility/log.h"
 
#include "utility/thread/mutex.h"
#include "utility/files.h"
#include "settings.h"

#define LOGFILE cSettings::getInstance().getLogPath().c_str()
#define NETLOGFILE cSettings::getInstance().getNetLogPath().c_str()
/** errors */
#define EE "(EE): "
/** warnings */
#define WW "(WW): "
/** informations */
#define II "(II): "
/** debuginformations */
#define DD "(DD): "
/**mem error*/
#define MM "(MM): "

//------------------------------------------------------------------------------
cLog::cLog() :
	logfile(nullptr),
	netLogfile(nullptr)
{}

//------------------------------------------------------------------------------
cLog::~cLog()
{
	if (netLogfile) fclose (netLogfile);
	if (logfile) fclose (logfile);
}

//------------------------------------------------------------------------------
void cLog::write(const char* msg)
{
	write(std::string(msg), eLOG_TYPE_INFO);
}

//------------------------------------------------------------------------------
void cLog::write(const std::string& msg)
{
	write(msg, eLOG_TYPE_INFO);
}

//------------------------------------------------------------------------------
void cLog::write (const char* msg, eLogType type)
{
	return write (std::string (msg), type);
}

//------------------------------------------------------------------------------
void cLog::write (const std::string& msg, eLogType type)
{
	cLockGuard<cMutex> l (mutex);

	if ((type == eLOG_TYPE_DEBUG || type == eLOG_TYPE_NET_DEBUG) && !cSettings::getInstance().isDebug())
 	{
		//in case debug is disabled we skip message
 		return;
 	}

	checkOpenFile(type);

	//attach log message type to string
	std::string tmp;
	switch (type)
	{
		case eLOG_TYPE_NET_WARNING :
		case eLOG_TYPE_WARNING : 
			tmp = WW + msg;
			break;
		case eLOG_TYPE_NET_ERROR :
		case eLOG_TYPE_ERROR :
			tmp = EE + msg;
			std::cout << tmp << "\n"; break;
		case eLOG_TYPE_NET_DEBUG :
		case eLOG_TYPE_DEBUG :   
			tmp = DD + msg;
			break;
		case eLOG_TYPE_INFO :
			tmp = II + msg;
			break;
		case eLOG_TYPE_MEM :     
			tmp = MM + msg; break;
		default :               
			tmp = II + msg;
	}
	tmp += TEXT_FILE_LF;

	if (type == eLOG_TYPE_NET_DEBUG || type == eLOG_TYPE_NET_WARNING || type == eLOG_TYPE_NET_ERROR)
	{
		writeToFile(tmp, netLogfile);
	}
	else
	{
		writeToFile(tmp, logfile);

	}
}

//------------------------------------------------------------------------------
void cLog::mark()
{
	cLockGuard<cMutex> l (mutex);

	checkOpenFile(eLOG_TYPE_INFO);

	std::string str = "==============================(MARK)==============================";
	str += TEXT_FILE_LF;

	writeToFile(str, logfile);
}

//------------------------------------------------------------------------------
void cLog::checkOpenFile(eLogType type)
{

	if (type == eLOG_TYPE_NET_DEBUG || type == eLOG_TYPE_NET_WARNING || type == eLOG_TYPE_NET_ERROR)
	{
		if (netLogfile)
		{
			//file is already open
			return;
		}

		//append time stamp to log file name
		time_t tTime = time(nullptr);
#if defined (_MSC_VER)
		tm tm_;
		tm* tmTime = &tm_;
		localtime_s(tmTime, &tTime);
#else
		// Not thread safe, but we have a mutex which protects also that.
		tm* tmTime = localtime(&tTime);
#endif
		char timestr[25];
		strftime(timestr, 21, "%Y-%m-%d-%H%M_", tmTime);
		std::string sTime = timestr;
		cSettings::getInstance().setNetLogPath((getUserLogDir() + sTime + MAX_NET_LOG).c_str());

		//create + open new log file
		netLogfile = fopen(NETLOGFILE, "wt");
		if (netLogfile == nullptr)
		{
			fprintf(stderr, "(EE): Couldn't open net.log!\n Please check file/directory permissions\n");
		}
	}
	else
	{
		if (logfile)
		{
			//file is already open
			return;
		}

		//create + open new log file
		logfile = fopen(LOGFILE, "wt");
		if (logfile == nullptr)
		{
			fprintf(stderr, "(EE): Couldn't open maxr.log!\n Please check file/directory permissions\n");
		}
	}
}

//------------------------------------------------------------------------------
void cLog::writeToFile(std::string &msg, FILE* file)
{
	int result = 0;
	if (file)
	{
		result = fputs(msg.c_str(), file);
		fflush(file);
	}
	if (file == nullptr || result == EOF)
	{
		fprintf(stderr, msg.c_str());
	}
}
