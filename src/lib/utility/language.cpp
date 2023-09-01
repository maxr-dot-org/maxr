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
#include "settings.h"
#include "utility/listhelpers.h"
#include "utility/log.h"
#include "utility/os.h"

#include <iomanip>
#include <spiritless_po.h>

cLanguage lngPack;

struct cLanguage::cPimpl
{
	spiritless_po::Catalog maxrCatalogEng;
	spiritless_po::Catalog clansCatalogEng;
	spiritless_po::Catalog unitsCatalogEng;
	spiritless_po::Catalog maxrCatalog;
	spiritless_po::Catalog clansCatalog;
	spiritless_po::Catalog unitsCatalog;
};

namespace
{
	//--------------------------------------------------------------------------
	void openCatalog (spiritless_po::Catalog& catalog, const std::filesystem::path& path)
	{
		catalog.Clear();
		std::ifstream file (path);

		if (!catalog.Add (file))
		{
			Log.error ("Cannot open translation file: " + path.u8string());
			for (const auto& s : catalog.GetError())
			{
				Log.error (s);
			}
		}
	}

	//--------------------------------------------------------------------------
	std::string getText (const spiritless_po::Catalog& catalogEng, const spiritless_po::Catalog& catalog, const std::string& s)
	{
		const auto& translated = catalog.gettext (s);
		if (&translated != &s)
		{
			return translated;
		}
		Log.warn ("Missing translation: " + s);
		const auto& translatedEng = catalogEng.gettext (s);
		if (&translatedEng == &s)
		{
			Log.warn ("Missing English translation: " + std::string (s));
			return s;
		}
		return translatedEng;
	}

	//--------------------------------------------------------------------------
	std::string nGetText (const spiritless_po::Catalog& catalogEng, const spiritless_po::Catalog& catalog, const std::string& s, int n)
	{
		const auto& translated = catalog.ngettext (s, s, n);
		if (&translated != &s)
		{
			return translated;
		}
		Log.warn ("Missing translation: " + s);
		const auto& translatedEng = catalogEng.ngettext (s, s, n);
		if (&translatedEng == &s)
		{
			Log.warn ("Missing English translation: " + std::string (s));
			return s;
		}
		return translatedEng;
	}

	//--------------------------------------------------------------------------
	void checkMissingEntries (const spiritless_po::Catalog& ref, const spiritless_po::Catalog& cat)
	{
		for (const auto& [key, refIndex] : ref.GetIndex())
		{
			auto it = cat.GetIndex().find (key);
			if (it == cat.GetIndex().end())
			{
				Log.warn ("Missing or fuzzy translation for: " + key);
				continue;
			}
			const auto& catIndex = it->second;
			for (std::size_t i = 0; i != catIndex.totalPlurals; ++i)
			{
				if (cat.GetStringTable()[catIndex.stringTableIndex + i].empty())
				{
					Log.warn ("Missing or fuzzy translation for: " + key);
				}
			}
		}
	}

} // namespace

//------------------------------------------------------------------------------
cLanguage::cLanguage() :
	pimpl (std::make_shared<cLanguage::cPimpl>())
{
}

//------------------------------------------------------------------------------
void cLanguage::setLanguagesFolder (const std::filesystem::path& path)
{
	rootDir = path;
	openCatalog (pimpl->maxrCatalogEng, path / "en/maxr.po");
	openCatalog (pimpl->clansCatalogEng, path / "en/clans.po");
	openCatalog (pimpl->unitsCatalogEng, path / "en/units.po");
}

//------------------------------------------------------------------------------
void cLanguage::setCurrentLanguage (const std::string& code)
{
	if (!ranges::contains (getAvailableLanguages(), code))
	{
		Log.error ("Not a supported language: " + code);
		throw std::runtime_error ("Unsupported language " + code);
	}
	Log.info ("Set current language to " + code);

	m_languageCode = code;
	openCatalog (pimpl->maxrCatalog, rootDir / code / "maxr.po");
	openCatalog (pimpl->clansCatalog, rootDir / code / "clans.po");
	openCatalog (pimpl->unitsCatalog, rootDir / code / "units.po");
	if (cSettings::getInstance().isDebug())
	{
		checkMissingEntries (pimpl->maxrCatalogEng, pimpl->maxrCatalog);
		checkMissingEntries (pimpl->clansCatalogEng, pimpl->clansCatalog);
		checkMissingEntries (pimpl->unitsCatalogEng, pimpl->unitsCatalog);
	}
}

//------------------------------------------------------------------------------
std::string cLanguage::i18n (const std::string& s) const
{
	return getText (pimpl->maxrCatalogEng, pimpl->maxrCatalog, s);
}

//------------------------------------------------------------------------------
// Translation with replace %s
std::string cLanguage::i18n (const std::string& format, const std::string& insertText) const
{
	std::string translated = getText (pimpl->maxrCatalogEng, pimpl->maxrCatalog, format);
	auto pos = translated.find ("%s");

	if (pos == std::string::npos)
	{
		Log.warn ("Found no place holder in language string. Update language file!");
		Log.warn ("*-> String in question is: \"" + format + "\"");
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
	std::string translated = nGetText (pimpl->maxrCatalogEng, pimpl->maxrCatalog, text, n);
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
	return os::getDirectories (cSettings::getInstance().getLangPath());
}

//------------------------------------------------------------------------------
std::string cLanguage::getUnitName (const sID& id) const
{
	std::stringstream ss;

	ss << (id.isABuilding() ? "Building" : "Vehicle")
	   << std::setfill ('0') << std::setw (2) << id.secondPart
	   << "_Name";

	return getText (pimpl->unitsCatalogEng, pimpl->unitsCatalog, ss.str());
}

//------------------------------------------------------------------------------
std::string cLanguage::getUnitDescription (const sID& id) const
{
	std::stringstream ss;

	ss << (id.isABuilding() ? "Building" : "Vehicle")
	   << std::setfill ('0') << std::setw (2) << id.secondPart
	   << "_Desc";

	return getText (pimpl->unitsCatalogEng, pimpl->unitsCatalog, ss.str());
}

//------------------------------------------------------------------------------
std::string cLanguage::getClanName (int num) const
{
	return getText (pimpl->clansCatalogEng, pimpl->clansCatalog, "Clan" + std::to_string (num) + "_Name");
}

//------------------------------------------------------------------------------
std::string cLanguage::getClanDescription (int num) const
{
	return getText (pimpl->clansCatalogEng, pimpl->clansCatalog, "Clan" + std::to_string (num) + "_Desc");
}

//------------------------------------------------------------------------------
std::vector<std::pair<std::string, std::string>> cLanguage::getAllTranslations() const
{
	std::vector<std::pair<std::string, std::string>> res;
	const auto& catalog = pimpl->maxrCatalog;
	for (const auto& [key, index] : catalog.GetIndex())
	{
		for (std::size_t i = 0; i != index.totalPlurals; ++i)
		{
			res.emplace_back (key, catalog.GetStringTable()[index.stringTableIndex + i]);
		}
	}
	return res;
}
