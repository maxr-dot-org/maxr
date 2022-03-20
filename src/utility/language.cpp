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

#include "utility/language.h"

#include "game/data/units/id.h"
#include "utility/files.h"
#include "utility/listhelpers.h"
#include "utility/log.h"

#include <libintl.h>
#include <cstdlib>
#include <iomanip>
#include <mutex>

cLanguage lngPack;

namespace
{
	constexpr const char* maxrDomain = "maxr";
	constexpr const char* clansDomain = "clans";
	constexpr const char* unitsDomain = "units";

	// gettext use environment variable to select language.
	// We fall-back to English by changing environment variable
	// (whereas traditional way is to use English key).
	std::mutex putenv_mutex;

	//------------------------------------------------------------------------------
	void setLanguageEnv(const char* lang)
	{
		// putenv(char*) is some system, even if string is unchanged
		// it even seems that pointer should still be alive
		static char buffer[] = "LANGUAGE=....";
		snprintf (buffer, sizeof (buffer), "LANGUAGE=%s", lang);
		putenv (buffer);
	}

}

//------------------------------------------------------------------------------
void cLanguage::setLanguagesFolder (const std::string& path)
{
	bindtextdomain ("maxr", path.c_str());
	bind_textdomain_codeset ("maxr", "utf-8");
	bindtextdomain ("clans", path.c_str());
	bind_textdomain_codeset ("clans", "utf-8");
	bindtextdomain ("units", path.c_str());
	bind_textdomain_codeset ("units", "utf-8");
	textdomain ("maxr");
	std::unique_lock<std::mutex> lock (putenv_mutex);
	setLanguageEnv ("en");
}

//------------------------------------------------------------------------------
void cLanguage::setCurrentLanguage (const std::string& code)
{
	if (!Contains (getAvailableLanguages(), code))
	{
		Log.write ("Not a supported language: " + code, cLog::eLogType::Error);
		throw std::runtime_error ("Unsupported language " + code);
	}
	m_languageCode = code;
	std::unique_lock<std::mutex> lock (putenv_mutex);
	setLanguageEnv (code.c_str());
}

//------------------------------------------------------------------------------
std::string cLanguage::dGetText (const char* textDomain, const char* s) const
{
	std::unique_lock<std::mutex> lock (putenv_mutex);
	auto translated = dgettext (textDomain, s);

	if (translated == s)
	{
		Log.write ("Missing translation: " + std::string (s), cLog::eLogType::Warning);

		setLanguageEnv ("en");
		translated = dgettext (textDomain, s);
		if (translated == s)
		{
			Log.write ("Missing English translation: " + std::string (s), cLog::eLogType::Warning);
		}
		setLanguageEnv (m_languageCode.c_str());
	}
	return translated;
}

//------------------------------------------------------------------------------
std::string cLanguage::i18n (const std::string& s) const
{
	return dGetText (maxrDomain, s.c_str());
}

//------------------------------------------------------------------------------
// Translation with replace %s
std::string cLanguage::i18n (const std::string& format, const std::string& insertText) const
{
	std::string translated = dGetText (maxrDomain, format.c_str());
	auto pos = translated.find ("%s");

	if (pos == std::string::npos)
	{
		Log.write ("Found no place holder in language string. Update language file!", cLog::eLogType::Warning);
		Log.write ("*-> String in question is: \"" + format + "\"", cLog::eLogType::Warning);
		return format + insertText;
	}
	else
	{
		translated.replace (pos, 2, insertText);
		return translated;
	}
}

//------------------------------------------------------------------------------
std::string cLanguage::plural (const std::string& text, std::size_t n) const
{
	std::unique_lock<std::mutex> lock (putenv_mutex);
	std::string translated = dngettext (maxrDomain, text.c_str(), text.c_str(), n);

	if (translated == text)
	{
		Log.write ("Missing translation (plural entry): " + std::string (text), cLog::eLogType::Warning);

		setLanguageEnv ("en");
		translated = dngettext (maxrDomain, text.c_str(), text.c_str(), n);
		if (translated == text)
		{
			Log.write ("Missing English translation (plural entry): " + std::string (text), cLog::eLogType::Warning);
		}
		setLanguageEnv (m_languageCode.c_str());
	}
	const auto pos = translated.find ("%d");

	if (pos != std::string::npos)
	{
		translated.replace (pos, 2, std::to_string (n));
	}
	return translated;
}

//------------------------------------------------------------------------------
std::vector<std::string> cLanguage::getAvailableLanguages() const
{
#if 0 // TODO: detect automatically languages from directories
	return getDirectories (cSettings::getInstance().getLangPath());
#else
	return {"ca", "de", "en", "es", "fr", "hu", "nl", "ru", "sl"};
#endif
}

//------------------------------------------------------------------------------
std::string cLanguage::getUnitName (const sID& id) const
{
	std::stringstream ss;

	ss << (id.isABuilding() ? "Building" : "Vehicle")
		<< std::setfill ('0') << std::setw (2) << id.secondPart
		<< "_Name";

	return dGetText (unitsDomain, ss.str().c_str());
}

//------------------------------------------------------------------------------
std::string cLanguage::getUnitDescription (const sID& id) const
{
	std::stringstream ss;

	ss << (id.isABuilding() ? "Building" : "Vehicle")
		<< std::setfill ('0') << std::setw (2) << id.secondPart
		<< "_Desc";

	return dGetText (unitsDomain, ss.str().c_str());
}

//------------------------------------------------------------------------------
std::string cLanguage::getClanName (int num) const
{
	std::stringstream ss;

	ss << "Clan" << num << "_Name";

	return dGetText (clansDomain, ss.str().c_str());
}

//------------------------------------------------------------------------------
std::string cLanguage::getClanDescription (int num) const
{
	std::stringstream ss;

	ss << "Clan" << num << "_Desc";

	return dGetText (clansDomain, ss.str().c_str());
}
