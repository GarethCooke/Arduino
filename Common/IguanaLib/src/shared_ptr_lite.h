#pragma once

template <typename T> class shared_ptr_lite
{
public:
	shared_ptr_lite(T* p) : m_p(p) {}
	shared_ptr_lite(const shared_ptr_lite& right) { *this = right; }

	~shared_ptr_lite() { delete m_p; }

	shared_ptr_lite& operator=(const shared_ptr_lite& right) { m_p = const_cast<shared_ptr_lite&>(right).release(); return *this; }
	T* release() { T* p = m_p; m_p = NULL; return p; }

private:
	mutable T* m_p;
};

template<typename T> shared_ptr_lite<T> make_shared_ptr_lite(T* p)
{
	return shared_ptr_lite<T>(p);
}