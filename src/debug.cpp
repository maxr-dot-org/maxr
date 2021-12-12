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


/* This file is used to enable the crash reporting form the CrashRpt library:
*  http://crashrpt.sourceforge.net/
*/


#ifdef USE_CRASH_RPT

#include <SDL.h>

#include "utility/log.h"
#include "maxrversion.h"
#include "settings.h"
#include "output/video/video.h"

#include "debug.h"

int CALLBACK CrashCallback (CR_CRASH_CALLBACK_INFO* pInfo)
{
	// The application has crashed!
	if (cVideo::buffer)
	{
		std::string home = cSettings::getInstance().getHomeDir();
		if (!home.empty())
		{
			std::string path = home + "\\Crashshot.bmp";
			SDL_SaveBMP (cVideo::buffer, path.c_str());
			crAddFile2(path.c_str(), nullptr, "Screenshot at the moment of the crash", CR_AF_MAKE_FILE_COPY | CR_AF_MISSING_FILE_OK);
		}

	}


	// Return CR_CB_DODEFAULT to generate error report
	return CR_CB_DODEFAULT;
}


void initCrashreporting()
{
	//crUninstall();

	CR_INSTALL_INFO info;
	memset (&info, 0, sizeof (CR_INSTALL_INFO));
	info.cb = sizeof (CR_INSTALL_INFO);
	info.pszAppName = "maxr";
	info.pszAppVersion = PACKAGE_VERSION " " PACKAGE_REV;
	info.uMiniDumpType = MiniDumpWithIndirectlyReferencedMemory;
	//info.pszUrl
	//info.uPriorities[CR_SFTP] = 1;
	info.uPriorities[CR_SMAPI] = -1;
	info.uPriorities[CR_SMTP] = -1;
	info.uPriorities[CR_HTTP] = -1;
	//info.dwFlags = CR_INST_DONT_SEND_REPORT;
	//info.dwFlags = CR_INST_STORE_ZIP_ARCHIVES;
	info.dwFlags = CR_INST_SHOW_ADDITIONAL_INFO_FIELDS;
	info.dwFlags |= CR_INST_ALLOW_ATTACH_MORE_FILES;
	info.dwFlags |= CR_INST_ALL_POSSIBLE_HANDLERS;
	info.dwFlags |= CR_INST_SEND_QUEUED_REPORTS;
	info.dwFlags |= CR_INST_APP_RESTART;
	info.pszCustomSenderIcon = "maxr.ico";
	info.pszPrivacyPolicyURL = "http://eiko.maxr.org/crashreports/Privacy%20Policy.html";
	std::string path = cSettings::getInstance().getHomeDir() + "\\Crashreports\\";
	info.pszErrorReportSaveDir = path.c_str();
	std::string lang = cSettings::getInstance().getLanguage();
	const auto currentExeDir = getCurrentExeDir();
	std::string langPath = currentExeDir + "\\crashrpt_lang_EN.ini";
	if (lang == "GER")
		std::string langPath = currentExeDir + "\\crashrpt_lang_DE.ini";

	info.pszLangFilePath = langPath.c_str();

	int result = crInstall (&info);
	if (result != 0)
	{
		char msg[512];
		crGetLastErrorMsg (msg, sizeof (msg));
		Log.write (std::string ("Couldn't install crash reporting: ") + msg, cLog::eLOG_TYPE_WARNING);
	}

	crSetCrashCallback (CrashCallback, nullptr);

	std::string log = cSettings::getInstance().getLogPath();
	if (!log.empty())
	{
		crAddFile2 (log.c_str(), nullptr, "Maxr Logfile", CR_AF_MAKE_FILE_COPY | CR_AF_MISSING_FILE_OK);
	}

	std::string settings = cSettings::getInstance().getHomeDir() + "maxr.json";
	if (!settings.empty())
	{
		crAddFile2 (settings.c_str(), nullptr, "Maxr Configuration File", CR_AF_MAKE_FILE_COPY | CR_AF_MISSING_FILE_OK);
	}

	std::string netlog = cSettings::getInstance().getNetLogPath();
	if (!netlog.empty())
	{
		crAddFile2 (netlog.c_str(), nullptr, "Maxr Network Logfile", CR_AF_MAKE_FILE_COPY | CR_AF_MISSING_FILE_OK);
	}

	std::string home = cSettings::getInstance().getHomeDir();
	if (!home.empty())
	{
		crAddFile2 ((home + "resinstaller.log").c_str(), nullptr, "Maxr Resinstaller Logfile", CR_AF_MAKE_FILE_COPY | CR_AF_MISSING_FILE_OK);
	}

	//internal screenshot function is useless...
	//crAddScreenshot2(CR_AS_PROCESS_WINDOWS, 0);
}

#endif //USE_CRASH_RPT
