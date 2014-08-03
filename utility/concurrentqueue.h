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

#ifndef utility_concurrentqueueH
#define utility_concurrentqueueH

#include <deque>

#include "cmutex.h"

template<typename T>
class cConcurrentQueue
{
public:
    typedef T value_type;
    typedef T& reference;
    typedef const T& const_reference;
    typedef std::ptrdiff_t size_type;
    typedef std::ptrdiff_t difference_type;

    typedef typename std::deque<T>::iterator iterator;
    typedef typename std::deque<T>::const_iterator const_iterator;

    void push(const T& value);
    void push(T&& value);
    bool try_pop(T& destination);
    void clear();

	size_type safe_size () const;
	bool safe_empty () const;

    size_type unsafe_size() const;
    bool unsafe_empty() const;

    iterator unsafe_begin();
    iterator unsafe_end();
    const_iterator unsafe_begin() const;
    const_iterator unsafe_end() const;

private:
    std::deque<T> internalQueue;

    mutable cMutex mutex;
};

//------------------------------------------------------------------------------
template<typename T>
void cConcurrentQueue<T>::push(const T& value)
{
    cMutex::Lock lock(mutex);

    internalQueue.push_back(value);
}

//------------------------------------------------------------------------------
template<typename T>
void cConcurrentQueue<T>::push(T&& value)
{
    cMutex::Lock lock(mutex);

    internalQueue.push_back(std::forward<T>(value));
}

//------------------------------------------------------------------------------
template<typename T>
bool cConcurrentQueue<T>::try_pop(T& destination)
{
    cMutex::Lock lock(mutex);

    if(unsafe_empty()) return false;

    destination = std::move(internalQueue.front());
    internalQueue.pop_front();

    return true;
}

//------------------------------------------------------------------------------
template<typename T>
void cConcurrentQueue<T>::clear()
{
    cMutex::Lock lock(mutex);

    internalQueue.clear();
}

//------------------------------------------------------------------------------
template<typename T>
typename cConcurrentQueue<T>::size_type cConcurrentQueue<T>::safe_size() const
{
	cMutex::Lock lock (mutex);
	return unsafe_size ();
}

//------------------------------------------------------------------------------
template<typename T>
bool cConcurrentQueue<T>::safe_empty() const
{
	cMutex::Lock lock (mutex);
	return unsafe_empty ();
}

//------------------------------------------------------------------------------
template<typename T>
typename cConcurrentQueue<T>::size_type cConcurrentQueue<T>::unsafe_size() const
{
    return internalQueue.size();
}

//------------------------------------------------------------------------------
template<typename T>
bool cConcurrentQueue<T>::unsafe_empty() const
{
    return internalQueue.empty();
}

//------------------------------------------------------------------------------
template<typename T>
typename cConcurrentQueue<T>::iterator cConcurrentQueue<T>::unsafe_begin()
{
    return internalQueue.begin();
}

//------------------------------------------------------------------------------
template<typename T>
typename cConcurrentQueue<T>::iterator cConcurrentQueue<T>::unsafe_end()
{
    return internalQueue.end();
}

//------------------------------------------------------------------------------
template<typename T>
typename cConcurrentQueue<T>::const_iterator cConcurrentQueue<T>::unsafe_begin() const
{
    return internalQueue.begin();
}

//------------------------------------------------------------------------------
template<typename T>
typename cConcurrentQueue<T>::const_iterator cConcurrentQueue<T>::unsafe_end() const
{
    return internalQueue.end();
}

#endif // utility_concurrentqueueH
