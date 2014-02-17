#ifndef AUTOPTR_H
#define AUTOPTR_H

#include "autoobj.h"

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

	AutoPtr (const AutoPtr&)   DELETE_CPP11;  // No copy.
	void operator = (AutoPtr&) DELETE_CPP11;  // No assignment.
};

#endif
