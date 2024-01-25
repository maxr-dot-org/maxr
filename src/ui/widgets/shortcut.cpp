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

#include "shortcut.h"

//------------------------------------------------------------------------------
cShortcut::cShortcut (cKeySequence keySequence_) :
	keySequence (keySequence_)
{}

//------------------------------------------------------------------------------
void cShortcut::activate()
{
	active = true;
}

//------------------------------------------------------------------------------
void cShortcut::deactivate()
{
	active = false;
}

//------------------------------------------------------------------------------
bool cShortcut::hit (const cKeySequence& currentKeySequence)
{
	if (!isActive()) return false;

	if (keySequence.length() > currentKeySequence.length()) return false;

	for (size_t j = 1; j <= keySequence.length(); ++j)
	{
		if (!currentKeySequence[currentKeySequence.length() - j].matches (keySequence[keySequence.length() - j]))
		{
			return false;
		}
	}

	triggered();
	return true;
}
