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

#ifndef config_workaround_cpp17_filesystemH
#define config_workaround_cpp17_filesystemH

#if __cplusplus < 201700

# include <string>

namespace std
{
	namespace filesystem
	{

		inline namespace compatibility_cpp17
		{

			class path
			{
			public:
# ifdef WIN32
				using value_type = wchar_t;
				using string_type = std::wstring;
# else
				using value_type = char;
				using string_type = std::string;
# endif
			public:
				path() noexcept = default;
				path (const path& rhs) = default;
				path (path&& rhs) noexcept = default;
				~path() = default;

				path (std::string&& s) :
					pathname (std::move (s))
				{}
				path (const std::string& s) :
					pathname (s)
				{}
# ifdef WIN32
				path (const std::wstring& s);
# endif
				path (const char* s) :
					pathname (s)
				{}

				path& operator= (const path& rhs) = default;
				path& operator= (path&& rhs) noexcept = default;
				path& operator= (const std::string& s)
				{
					pathname = s;
					return *this;
				}
				path& operator= (std::string&& s) noexcept
				{
					pathname = std::move (s);
					return *this;
				}
				path& operator= (const char* s)
				{
					pathname = s;
					return *this;
				}

				path& operator/= (const path& rhs);
				friend path operator/ (const path& lhs, const path& rhs) { return path (lhs) /= rhs; }

				//const value_type* c_str() const noexcept { return pathname.c_str(); }
				//operator string_type() const { return pathname; }
				std::string string() const { return pathname; }

# ifdef WIN32
				std::wstring native() const;
# else
				std::string native() const { return pathname; }
# endif

				bool empty() const { return pathname.empty(); }

				bool operator== (const path& rhs) const
				{
					return pathname == rhs.pathname;
				}

				bool operator!= (const path& rhs) const { return !(*this == rhs); }

			private:
				std::string pathname;
			};

			bool copy_file (const path& from, const path& to);
			void create_directories (const path&);
			bool exists (const path&);

		} // namespace compatibility_cpp17
	} // namespace filesystem
} // namespace std
#else
# include <filesystem>
#endif

#endif
