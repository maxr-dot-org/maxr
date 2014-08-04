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

#ifndef utility_autoptrH
#define utility_autoptrH

#include "utility/autoobj.h"

template <typename T>
class AutoPtr
{
public:
	explicit AutoPtr (T* const p_ = NULL) : p (p_) {}
	~AutoPtr() { delete p; }

	T* Release()
	{
		T* const p_ = p;
		p = NULL;
		return p_;
	}

	void operator = (T* const p_)
	{
		delete p;
		p = p_;
	}

	T* operator ->() const { return p; }
	T& operator *() const { return *p; }

	bool operator == (const T* rhs) const { return p == rhs; }
	bool operator != (const T* rhs) const { return p != rhs; }

	bool operator !() const { return p == NULL; }

	T* get() const { return p; }

private:
	T* p;

	AutoPtr (const AutoPtr&)   MAXR_DELETE_FUNCTION;  // No copy.
	void operator = (AutoPtr&) MAXR_DELETE_FUNCTION;  // No assignment.
};

#endif // utility_autoptrH
