#ifndef AUTOOBJ_H
#define AUTOOBJ_H

template <typename T, void (&dealloc)(T*)> class AutoObj
{
public:
	explicit AutoObj(T* const p = 0) : p_(p) {}

	~AutoObj() { if (p_) dealloc(p_); }

	void operator =(T* const p)
	{
		if (p_) dealloc(p_);
		p_ = p;
	}

	T* operator ->() const { return p_; }

	operator T*() const { return p_; }

private:
	T* p_;

	AutoObj(AutoObj const&);   // No copy.
	void operator =(AutoObj&); // No assignment.
};

#endif
