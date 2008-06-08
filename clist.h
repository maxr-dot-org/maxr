#ifndef CLIST_H
#define CLIST_H

#include <algorithm>
#include <cstdlib>


#define MALLOCN(type, count)  static_cast<type*>(malloc(sizeof(type) * (count)))


template<typename T> class cList
{
	public:
		cList() : Items(), iCount(), capacity_() {}

		~cList() { Reserve(0); }

		void Add(T const& elem);

		void Delete(size_t idx);

		void Reserve(size_t n);

	public:
		T*     Items;
		size_t iCount;
	private:
		size_t capacity_;
};


template<typename T> void cList<T>::Add(T const& e)
{
	if (iCount >= capacity_) Reserve(std::max(1U, iCount * 2));
	new (&Items[iCount]) T(e);
	++iCount;
}


template<typename T> void cList<T>::Delete(size_t const idx)
{
	if (idx >= iCount) return; // XXX should throw exception

	for (size_t i = idx; i < iCount - 1; ++i) Items[i] = Items[i + 1];
	--iCount;
	Items[iCount].~T();
}


template<typename T> void cList<T>::Reserve(size_t const n)
{
	T* const new_v = n == 0 ? 0 : MALLOCN(T, n);

	size_t       i;
	T*     const old_v    = Items;
	size_t const old_size = iCount;
	size_t const new_size = std::min(old_size, n);
	try
	{
		for (i = 0; i < new_size; ++i) new (&new_v[i]) T(old_v[i]);
	}
	catch (...)
	{
		for (size_t k = i; k != 0;) new_v[--i].~T();
		free(new_v);
		throw;
	}

	Items     = new_v;
	capacity_ = n;
	iCount    = new_size;

	for (size_t k = old_size; k != 0;) old_v[--k].~T();
	free(old_v);
}

#endif
