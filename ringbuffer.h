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

#ifndef ringbuffer_h
#define ringbuffer_h

#include "cmutex.h"
#include "assert.h"

/**
* a simple thread safe ring buffer
*@author eiko
*/
template<typename T> class cRingbuffer
{
private:
	T* elements;
	int read_;
	int write_;
	int capacity;
	cMutex mutex;
	void checkCapacity();


public:
	cRingbuffer() : elements (NULL), read_ (0), write_ (0), capacity (0) {}
	~cRingbuffer() { delete [] elements; }

	int size();
	void write (const T element);
	T read();
	const T peep();
};

template<typename T> int cRingbuffer<T>::size()
{
	cMutex::Lock lock (mutex);

	int size = write_ - read_;
	if (size < 0) size += capacity;

	return size;
}

template<typename T> void cRingbuffer<T>::write (const T element)
{
	cMutex::Lock lock (mutex);

	checkCapacity();

	elements[write_] = element;

	write_++;
	if (write_ >= capacity)
	{
		write_ = 0;
	}

}

template<typename T> void cRingbuffer<T>::checkCapacity ()
{
	int s = size();
	if (s >= capacity - 1)
	{
		//alloc new memory
		int newCapacity = capacity + 128;

		T* newElements = new T[newCapacity];

		int newWrite_;
		for (newWrite_ = 0; newWrite_ < capacity - 1; newWrite_++)
		{
			newElements[newWrite_] = read();
		}

		delete [] elements;
		elements = newElements;
		write_ = newWrite_;
		read_ = 0;
		capacity = newCapacity;
	}
}

template<typename T> T cRingbuffer<T>::read()
{
	cMutex::Lock lock (mutex);

	assert (size() != 0);

	T element = elements[read_];
	read_++;
	if (read_ >= capacity)
	{
		read_ = 0;
		//TODO: free memory here
	}
	return element;
}

template<typename T> const T cRingbuffer<T>::peep()
{
	cMutex::Lock lock (mutex);

	return elements[read_];
}


#endif //ringbuffer_h
