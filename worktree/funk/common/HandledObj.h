#ifndef _INCLUDE_HANDLED_OBJ_H_
#define _INCLUDE_HANDLED_OBJ_H_

#include <assert.h>
#include <common/StrongHandle.h>

namespace funk
{
template< typename T >
class HandledObj
{
public:
	HandledObj();

	StrongHandle<T> GetHandle();

	inline int	RefCount() const { return m_refCount; }
	inline void AddRef();
	inline void ReleaseRef();

private:

	int m_refCount;	
};

///////////////////////////////////////////////////////////////////////////////

template< typename T >
HandledObj<T>::HandledObj() : m_refCount(0)
{;}

template< typename T >
void HandledObj<T>::AddRef()
{
	++m_refCount;
	assert(m_refCount >= 0);
}

template< typename T >
void HandledObj<T>::ReleaseRef()
{
	--m_refCount;
	assert(m_refCount >= 0);
}

template< typename T >
StrongHandle<T> HandledObj<T>::GetHandle()
{
	assert(m_refCount >= 0);
	return StrongHandle<T>((T*)this);
}
}

#endif