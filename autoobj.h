#ifndef AUTOOBJ_H
#define AUTOOBJ_H

#if __cplusplus >= 201103L || defined __GXX_EXPERIMENTAL_CXX0X__
# define DELETE_CPP11 = delete
#else
# define DELETE_CPP11
#endif

template <typename T, void (&dealloc) (T*) > class AutoObj
{
public:
	explicit AutoObj (T* const p = 0) : p_ (p) {}

	~AutoObj() { if (p_) dealloc (p_); }

	T* Release()
	{
		T* const p = p_;
		p_ = 0;
		return p;
	}

	void operator = (T* const p)
	{
		if (p_) dealloc (p_);
		p_ = p;
	}

	T* operator ->() const { return p_; }

	operator T* () const { return p_; }

private:
	T* p_;

	AutoObj (AutoObj const&)   DELETE_CPP11;  // No copy.
	void operator = (AutoObj&) DELETE_CPP11;  // No assignment.
};

#endif
