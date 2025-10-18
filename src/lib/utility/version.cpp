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

#include "utility/version.h"

#include "utility/string/toNumber.h"

#ifdef major
# undef major
#endif

#ifdef minor
# undef minor
#endif

//------------------------------------------------------------------------------
cVersion::cVersion (std::string_view string)
{
	parseFromString (string);
}

//------------------------------------------------------------------------------
cVersion::cVersion (int major_, int minor_, int revision_) :
	major (major_),
	minor (minor_),
	revision (revision_)
{}

//------------------------------------------------------------------------------
int cVersion::getMajor() const
{
	return major;
}

//------------------------------------------------------------------------------
void cVersion::setMajor (int value)
{
	major = value;
}

//------------------------------------------------------------------------------
int cVersion::getMinor() const
{
	return minor;
}

//------------------------------------------------------------------------------
void cVersion::setMinor (int value)
{
	minor = value;
}

//------------------------------------------------------------------------------
int cVersion::getRevision() const
{
	return revision;
}

//------------------------------------------------------------------------------
void cVersion::setRevision (int value)
{
	revision = value;
}

//------------------------------------------------------------------------------
void cVersion::parseFromString (std::string_view string)
{
	// NOTE: do we need this to be more robust?

	auto firstDotPos = string.find_first_of (".");
	major = toInt (string.substr (0, firstDotPos)).value_or (0);

	if (firstDotPos == std::string::npos)
	{
		minor = 0;
		revision = 0;
	}
	else
	{
		auto secondDotPos = string.find_first_of (".", firstDotPos + 1);
		minor = toInt (string.substr (firstDotPos + 1, secondDotPos)).value_or (0);

		if (secondDotPos == std::string::npos)
		{
			revision = 0;
		}
		else
		{
			revision = toInt (string.substr (secondDotPos + 1)).value_or (0);
		}
	}
}

//------------------------------------------------------------------------------
std::string cVersion::toString() const
{
	return std::to_string (major) + "." + std::to_string (minor) + "." + std::to_string (revision);
}
