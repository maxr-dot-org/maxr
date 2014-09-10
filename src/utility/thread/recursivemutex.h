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

#ifndef utility_thread_recursivemutexH
#define utility_thread_recursivemutexH

#include <SDL_mutex.h>
#include <stdexcept>

class cRecursiveMutex
{
public:
	cRecursiveMutex () :
		sdlMutex (SDL_CreateMutex ())
	{
		if (!sdlMutex) throw std::runtime_error ("Failed to create mutex");
	}

	~cRecursiveMutex ()
	{
		SDL_DestroyMutex (sdlMutex);
	}

	void lock ()
	{
		const auto result = SDL_LockMutex (sdlMutex);
		if (result != 0)
		{
			throw std::runtime_error ("Could not lock mutex");
		}
	}

	bool tryLock ()
	{
		const auto result = SDL_TryLockMutex (sdlMutex);
		if (result == -1)
		{
			throw std::runtime_error ("Error while trying to lock mutex");
		}
		return result == 0;
	}

	void unlock ()
	{
		SDL_UnlockMutex (sdlMutex);
	}

private:
	SDL_mutex* const sdlMutex;
};

#endif // utility_thread_recursivemutexH
