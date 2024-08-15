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

////////////////////////////////////////////////////////////////////////////////
//  Description:
//  This class handles the support for different language packs in gettext format.
////////////////////////////////////////////////////////////////////////////////

#ifndef utility_languageH
#define utility_languageH

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

struct sID;

class cLanguage
{
public:
	cLanguage();
	cLanguage (const cLanguage&) = delete;
	cLanguage& operator= (const cLanguage&) = delete;

	void setLanguagesFolder (const std::filesystem::path&);

	const std::string& getCurrentLanguage() const { return m_languageCode; }
	void setCurrentLanguage (const std::string& code);

	std::vector<std::string> getAvailableLanguages() const;

	std::string i18n (const std::string&) const;
	// Translation with replace %s
	std::string i18n (const std::string& text, std::string_view insertText) const;

	std::string plural (const std::string& text, std::size_t) const;

	std::string getUnitName (const sID&) const;
	std::string getUnitDescription (const sID&) const;

	std::string getClanName (int num) const;
	std::string getClanDescription (int num) const;

	std::vector<std::pair<std::string, std::string>> getAllTranslations() const;

private:
	struct cPimpl;
	std::shared_ptr<cPimpl> pimpl;

	std::filesystem::path rootDir;
	std::string m_languageCode;
};

extern cLanguage lngPack;

#endif // utility_languageH
