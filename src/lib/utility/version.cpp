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

#ifdef major
# undef major
#endif

#ifdef minor
# undef minor
#endif

//------------------------------------------------------------------------------
cVersion::cVersion (const std::string& string)
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
void cVersion::parseFromString (const std::string& string)
{
	// NOTE: do we need this to be more robust?

	auto firstDotPos = string.find_first_of (".");
	major = atoi (string.substr (0, firstDotPos).c_str());

	if (firstDotPos == std::string::npos)
	{
		minor = 0;
		revision = 0;
	}
	else
	{
		auto secondDotPos = string.find_first_of (".", firstDotPos + 1);
		minor = atoi (string.substr (firstDotPos + 1, secondDotPos).c_str());

		if (secondDotPos == std::string::npos)
		{
			revision = 0;
		}
		else
		{
			revision = atoi (string.substr (secondDotPos + 1).c_str());
		}
	}
}

//------------------------------------------------------------------------------
std::string cVersion::toString() const
{
	return std::to_string (major) + "." + std::to_string (minor) + "." + std::to_string (revision);
}

//------------------------------------------------------------------------------
bool cVersion::operator== (const cVersion& other) const
{
	return major == other.major && minor == other.minor && revision == other.revision;
}

//------------------------------------------------------------------------------
bool cVersion::operator!= (const cVersion& other) const
{
	return !(*this == other);
}

//------------------------------------------------------------------------------
bool cVersion::operator< (const cVersion& other) const
{
	return major < other.major || (major == other.major && (minor < other.minor || (minor == other.minor && revision < other.revision)));
}

//------------------------------------------------------------------------------
bool cVersion::operator<= (const cVersion& other) const
{
	return major < other.major || (major == other.major && (minor < other.minor || (minor == other.minor && revision <= other.revision)));
}

//------------------------------------------------------------------------------
bool cVersion::operator> (const cVersion& other) const
{
	return major > other.major || (major == other.major && (minor > other.minor || (minor == other.minor && revision > other.revision)));
}

//------------------------------------------------------------------------------
bool cVersion::operator>= (const cVersion& other) const
{
	return major > other.major || (major == other.major && (minor > other.minor || (minor == other.minor && revision >= other.revision)));
}
