#ifndef CLIST_H
#define CLIST_H

#include <algorithm>
#include <cstdlib>


#define MALLOCN(type, count)  static_cast<type*>(malloc(sizeof(type) * (count)))


template<typename T> class cList
{
	public:
		cList() : v_(), size_(), capacity_() {}

		~cList() { Reserve(0); }

		size_t Size() const { return size_; }

		T&       Back()       { return v_[size_ - 1]; }
		T const& Back() const { return v_[size_ - 1]; }

		T&       operator [](size_t const idx)       { return v_[idx]; }
		T const& operator [](size_t const idx) const { return v_[idx]; }

		void Add(T const& elem);

		void Insert( size_t const i, T const& e );

		void Delete(size_t idx);

		void PopBack();

		void Reserve(size_t n);
	
		bool Contains(const T& e);

	private:
		T*     v_;
		size_t capacity_;
		size_t size_;
};


template<typename T> void cList<T>::Add(T const& e)
{
	if (size_ >= capacity_) Reserve(std::max((size_t)1U, size_ * 2));
	new (&v_[size_]) T(e);
	++size_;
}

template<typename T> void cList<T>::Insert( size_t const i, T const& e )
{
	if ( i > size_ ) throw;
	if (size_ >= capacity_) Reserve(std::max((size_t)1U, size_ * 2));

	for (size_t n = size_; n > i; --n ) v_[n] = v_[n - 1];
	new (&v_[i]) T(e);
	++size_;
}

template<typename T> void cList<T>::Delete(size_t const idx)
{
	if (idx >= size_) 
		return; // XXX should throw exception

	for (size_t i = idx; i < size_ - 1; ++i) v_[i] = v_[i + 1];
	PopBack();
}


template<typename T> void cList<T>::PopBack()
{
	--size_;
	v_[size_].~T();
}


template<typename T> void cList<T>::Reserve(size_t const n)
{
	T* const new_v = n == 0 ? 0 : MALLOCN(T, n);

	size_t       i;
	T*     const old_v    = v_;
	size_t const old_size = size_;
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
	size_     = new_size;

	for (size_t k = old_size; k != 0;) old_v[--k].~T();
	free(old_v);
}

template<typename T> bool cList<T>::Contains(const T& e)
{
	for (size_t idx = 0; idx < size_; idx++)
	{
		if (v_[idx] == e)
			return true;
	}
	return false;
}

#endif
