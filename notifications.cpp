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

/* Author: Paul Grathwohl */

#include "notifications.h"

using namespace std;

//--------------------------------------------------------------------------
bool cNotificationSender::addNotificationListener (INotificationListener* listener)
{
	if (listener == 0)
		return false;
	for (size_t i = 0; i < listeners.size (); i++)
	{
		if (listeners[i] == listener)
			return false;
	}
	listeners.push_back (listener);
	return true;
}

//--------------------------------------------------------------------------
bool cNotificationSender::removeNotificationListener (INotificationListener* listener)
{
	if (listener == 0)
		return false;
	for (size_t i = 0; i < listeners.size (); i++)
	{
		if (listeners[i] == listener)
		{
			listeners.erase (listeners.begin () + i);
			return true;
		}
	}
	return false;
}

//--------------------------------------------------------------------------
void cNotificationSender::notifyListeners (const std::string& message)
{
	for (size_t i = 0; i < listeners.size (); i++)
		listeners[i]->notify (message, (void*)this);
}
