#ifndef CLIST_H
#define CLIST_H

#include <algorithm>
#include <cstdlib>


#define MALLOCN(type, count)  static_cast<type*>(malloc(sizeof(type) * (count)))


template<typename T> class cList
{
	public:
		cList() : v_(), iCount(), capacity_() {}

		~cList() { Reserve(0); }

		size_t Size() const { return iCount; }

		T&       Back()       { return v_[iCount - 1]; }
		T const& Back() const { return v_[iCount - 1]; }

		T&       operator [](size_t const idx)       { return v_[idx]; }
		T const& operator [](size_t const idx) const { return v_[idx]; }

		void Add(T const& elem);

		void Delete(size_t idx);

		void PopBack();

		void Reserve(size_t n);

	private:
		T*     v_;
		size_t capacity_;
	public:
		size_t iCount;
};


template<typename T> void cList<T>::Add(T const& e)
{
	if (iCount >= capacity_) Reserve(std::max(1U, iCount * 2));
	new (&v_[iCount]) T(e);
	++iCount;
}


template<typename T> void cList<T>::Delete(size_t const idx)
{
	if (idx >= iCount) return; // XXX should throw exception

	for (size_t i = idx; i < iCount - 1; ++i) v_[i] = v_[i + 1];
	PopBack();
}


template<typename T> void cList<T>::PopBack()
{
	--iCount;
	v_[iCount].~T();
}


template<typename T> void cList<T>::Reserve(size_t const n)
{
	T* const new_v = n == 0 ? 0 : MALLOCN(T, n);

	size_t       i;
	T*     const old_v    = v_;
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

	v_        = new_v;
	capacity_ = n;
	iCount    = new_size;

	for (size_t k = old_size; k != 0;) old_v[--k].~T();
	free(old_v);
}

#endif
