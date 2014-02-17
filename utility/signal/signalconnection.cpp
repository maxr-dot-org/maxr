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

#include "signalconnection.h"
#include "signal.h"

//------------------------------------------------------------------------------
bool cSignalConnection::operator==(const cSignalConnection& other) const
{
	return signal == other.signal && identifier == other.identifier;
}

//------------------------------------------------------------------------------
void cSignalConnection::disconnect ()
{
	signal->disconnect (*this);
}

//------------------------------------------------------------------------------
cSignalConnection::cSignalConnection (int identifier_, cSignalBase& signal_) :
	identifier (identifier_),
	signal (&signal_)
{}
