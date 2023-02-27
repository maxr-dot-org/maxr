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

#if __cplusplus < 201700

# include "filesystem.h"

# ifdef _WIN32
#  include <direct.h>
#  include <io.h>
# else
#  include <dirent.h>
# endif

# ifdef WIN32
#  include <direct.h>
#  include <shlobj.h>
#  include <sys/stat.h>
#  include <sys/types.h>
# else
#  include <sys/stat.h>
#  include <unistd.h>
# endif

# include <fstream>
# include <iterator>
# include <vector>

namespace
{
	//--------------------------------------------------------------------------
	bool makeDir (const std::string& path)
	{
# ifdef WIN32
		return _mkdir (path.c_str()) == 0;
# else
		return mkdir (path.c_str(), 0755) == 0;
# endif
	}

	//--------------------------------------------------------------------------
	bool dirExists (const std::string& path)
	{
# ifdef WIN32
		if (_access (path.c_str(), 0) == 0)
		{
			struct stat status;
			stat (path.c_str(), &status);

			return (status.st_mode & S_IFDIR);
		}
		else
			return false;
# else
		return std::filesystem::exists (path); // on linux everything is a file
# endif
	}

} // namespace

namespace std
{
	namespace filesystem
	{

		inline namespace compatibility_cpp17
		{

# ifdef WIN32
			//----------------------------------------------------------------------
			path::path (const std::wstring& s) :
				pathname(s.begin(), s.end()) // only works for non UTF-8 char
			{

			}

			//----------------------------------------------------------------------
			std::wstring path::native() const
			{
				return {pathname.begin(), pathname.end()};
			}
# endif

			//----------------------------------------------------------------------
			path& path::operator/= (const path& rhs)
			{
				if (pathname.empty())
				{
					pathname = rhs.pathname;
					return *this;
				}
# ifdef WIN32
				pathname += "\\" + rhs.pathname;
# else
				pathname += "/" + rhs.pathname;
# endif
				return *this;
			}

			//----------------------------------------------------------------------
			bool copy_file (const path& from, const path& to)
			{
				std::ifstream from_file (from.string(), std::istream::binary);
				if (!from_file) return false;
				std::ofstream to_file (to.string(), std::ostream::binary | std::ostream::trunc);
				const std::vector<char> buffer{std::istream_iterator<char> (from_file), std::istream_iterator<char>()};

				to_file.write (buffer.data(), buffer.size());
				return true;
			}

			//----------------------------------------------------------------------
			void create_directories (const path& p)
			{
				std::size_t prev_pos = 0;
				auto fullname = p.string();

				while (prev_pos != std::string::npos)
				{
					const auto pos = fullname.find_first_of ("/\\", prev_pos);
					const auto subPath = fullname.substr (0, pos);
					if (!dirExists (subPath))
					{
						makeDir (subPath);
					}
					prev_pos = pos == std::string::npos ? pos : pos + 1;
				}
			}

			//----------------------------------------------------------------------
			bool exists (const path& p)
			{
				return std::ifstream (p.string(), std::ifstream::binary).is_open();
			}

		} // namespace compatibility_cpp17
	} // namespace filesystem
} // namespace std
#endif
