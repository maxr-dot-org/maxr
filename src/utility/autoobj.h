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

#ifndef utility_autoobjH
#define utility_autoobjH

#include "maxrconfig.h"

template <typename T, void (&dealloc) (T*) > class AutoObj
{
public:
	explicit AutoObj (T* const p = 0) : p_ (p) {}

	~AutoObj() { if (p_) dealloc (p_); }

	T* Release()
	{
		T* const p = p_;
		p_ = nullptr;
		return p;
	}

	void operator = (T* const p)
	{
		if (p_) dealloc (p_);
		p_ = p;
	}

	T* operator ->() const { return p_; }
	T& operator *() const { return *p_; }

	bool operator == (const T* rhs) const { return p_ == rhs; }
	bool operator != (const T* rhs) const { return p_ != rhs; }

	bool operator !() const { return p_ == nullptr; }

	T* get() const { return p_; }

private:
	T* p_;

	AutoObj (AutoObj const&)   MAXR_DELETE_FUNCTION;  // No copy.
	void operator = (AutoObj&) MAXR_DELETE_FUNCTION;  // No assignment.
};

#endif // utility_autoobjH
