#ifndef AUTOPTR_H
#define AUTOPTR_H

#include "autoobj.h"

template<typename T>
void deleteFunc( T* ptr ) { delete ptr; }

template <typename T>
struct AutoPtr
{
	typedef AutoObj<T, deleteFunc<T> > type;
};

#endif
