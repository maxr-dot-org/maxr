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

# include "debug.h"

# include "maxrversion.h"
# include "output/video/video.h"
# include "settings.h"
# include "utility/log.h"
# include "utility/os.h"
# include "utility/string/toupper.h"

# ifndef NOMINMAX
#  define NOMINMAX // CrashRpt includes windows.h
# endif
# include <CrashRpt.h>
# include <SDL.h>

# ifdef WIN32
#  define L(LITERAL) L"" LITERAL
# else
#  define L(LITERAL) "" LITERAL
# endif

//------------------------------------------------------------------------------
int CALLBACK CrashCallback (CR_CRASH_CALLBACK_INFO* pInfo)
{
	// The application has crashed!
	if (cVideo::buffer)
	{
		auto home = cSettings::getInstance().getMaxrHomeDir();
		if (!home.empty())
		{
			auto path = home / "Crashshot.bmp";
			SDL_SaveBMP (cVideo::buffer, path.u8string().c_str());
			crAddFile2 (path.native().c_str(), nullptr, L ("Screenshot at the moment of the crash"), CR_AF_MAKE_FILE_COPY | CR_AF_MISSING_FILE_OK);
		}
	}

	// Return CR_CB_DODEFAULT to generate error report
	return CR_CB_DODEFAULT;
}

//------------------------------------------------------------------------------
void initCrashreporting()
{
	//crUninstall();

	CR_INSTALL_INFO info;
	memset (&info, 0, sizeof (CR_INSTALL_INFO));
	info.cb = sizeof (CR_INSTALL_INFO);
	info.pszAppName = L ("maxr");
	info.pszAppVersion = L (PACKAGE_VERSION " " PACKAGE_REV);
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
	info.pszCustomSenderIcon = L ("maxr.ico");
	auto privacyPolicyPath = (cSettings::getInstance().getDataDir() / "CrashRpt/Privacy Policy.html").native();
	info.pszPrivacyPolicyURL = privacyPolicyPath.c_str();
	const auto path = (cSettings::getInstance().getMaxrHomeDir() / "Crashreports").native();
	info.pszErrorReportSaveDir = path.c_str();
	std::string lang = to_upper_copy (cSettings::getInstance().getLanguage());
	const auto currentExeDir = os::getCurrentExeDir();
	auto langPath = (currentExeDir / ("CrashRpt/crashrpt_lang_" + lang + ".ini")).native();
	if (!std::filesystem::exists (langPath))
	{
		langPath = (currentExeDir / ("CrashRpt/crashrpt_lang_EN.ini")).native();
	}
	info.pszLangFilePath = langPath.c_str();

	int result = crInstall (&info);
	if (result != 0)
	{
		char msg[512];
		crGetLastErrorMsgA (msg, sizeof (msg));
		Log.warn (std::string ("Couldn't install crash reporting: ") + msg);
	}

	crSetCrashCallback (CrashCallback, nullptr);

	auto log = cSettings::getInstance().getLogPath().native();
	if (!log.empty())
	{
		crAddFile2 (log.c_str(), nullptr, L ("Maxr Logfile"), CR_AF_MAKE_FILE_COPY | CR_AF_MISSING_FILE_OK);
	}

	auto home = cSettings::getInstance().getMaxrHomeDir();
	if (!home.empty())
	{
		auto settings = home / "maxr.json";
		crAddFile2 (settings.native().c_str(), nullptr, L ("Maxr Configuration File"), CR_AF_MAKE_FILE_COPY | CR_AF_MISSING_FILE_OK);
	}

	auto netlog = cSettings::getInstance().getNetLogPath().native();
	if (!netlog.empty())
	{
		crAddFile2 (netlog.c_str(), nullptr, L ("Maxr Network Logfile"), CR_AF_MAKE_FILE_COPY | CR_AF_MISSING_FILE_OK);
	}

	if (!home.empty())
	{
		crAddFile2 ((home / "resinstaller.log").native().c_str(), nullptr, L ("Maxr Resinstaller Logfile"), CR_AF_MAKE_FILE_COPY | CR_AF_MISSING_FILE_OK);
	}

	//internal screenshot function is useless...
	//crAddScreenshot2(CR_AS_PROCESS_WINDOWS, 0);
}

//------------------------------------------------------------------------------
CR_RPT_RAII::CR_RPT_RAII() :
	pimpl (std::make_shared<CrThreadAutoInstallHelper> (0))
{
}

//------------------------------------------------------------------------------
void CR_EMULATE_CRASH()
{
	crEmulateCrash (CR_SEH_EXCEPTION);
}

//------------------------------------------------------------------------------
void CR_INIT_CRASHREPORTING()
{
	initCrashreporting();
}

#endif //USE_CRASH_RPT
