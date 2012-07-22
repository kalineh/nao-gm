#ifndef _INCLUDE_WEAKPTR_H_
#define _INCLUDE_WEAKPTR_H_

#include <assert.h>
#include "PtrIncludes.h"

template< typename T >
class StongPtr;

template< typename T >
class WeakPtr
{
public:
	WeakPtr( T* pVal = 0 );
	WeakPtr( const WeakPtr& ptr );
	WeakPtr( const StrongHandle<T>& ptr );
	~WeakPtr();

	inline T*		Get() const { return m_pVal; }
	inline bool		IsNull() const;

	// Pointer ops
	inline T*		operator->();
	inline const T*	operator->() const;
	inline T&		operator*();
	inline const T&	operator*() const;

	// Casting
	inline operator T*();
	inline operator const T*() const;

	// Copy ops
	WeakPtr& operator=( T* pVal );
	WeakPtr& operator=( const WeakPtr& ptr );
	WeakPtr& operator=( const StrongHandle<T>& ptr );

	// Comparison ops
	bool operator==( const WeakPtr& other ) const;
	bool operator==( const T* pVal ) const;
	bool operator!=( const WeakPtr& other ) const;
	bool operator!=( const T* pVal ) const;

private:	

	// Holds the data
	T * m_pVal;

	friend class StrongHandle<T>;
	CounterData * m_counter;

	inline void AddRef();
	inline void Release();
};

///////////////////////////////////////////////////////////////////////////////


template< typename T >
WeakPtr<T>::WeakPtr( const StrongHandle<T>& ptr ) : m_pVal( 0 ), m_counter( 0 )
{
	m_pVal = ptr.m_pVal;
	m_counter = ptr.m_counter;
	AddRef();
}

template< typename T >
WeakPtr<T>::WeakPtr( T* pVal ) : m_pVal( pVal ), m_counter( 0 )
{
	if ( pVal ) 
	{
		m_counter = new CounterData;
		m_counter->refCount = 0;
		m_counter->weakCount = 1;
	}
}

template< typename T >
WeakPtr<T>::WeakPtr( const WeakPtr& ptr )
{
	m_pVal = ptr.m_pVal;
	m_counter = ptr.m_counter;
	AddRef();
}

template< typename T >
WeakPtr<T>::~WeakPtr()
{
	Release();
}

template< typename T >
bool WeakPtr<T>::IsNull() const
{
	return ( (m_counter->refCount + m_counter->weakCount) == 0)
		|| m_pVal == 0;
}

template< typename T >
void WeakPtr<T>::AddRef()
{
	if ( m_pVal ) 
	{
		m_counter->weakCount++;
	}
}

template< typename T >
void WeakPtr<T>::Release()
{
	if ( m_pVal ) 
	{
		m_counter->weakCount--;

		if ( m_counter->refCount + m_counter->weakCount == 0 )
		{
			delete m_counter;
			m_pVal = 0;
			m_counter = 0;
		}
	}
}

template< typename T >
T* WeakPtr<T>::operator->()
{
	assert( m_counter && m_counter->refCount );
	return m_pVal;
}

template< typename T >
const T* WeakPtr<T>::operator->() const
{
	assert( m_counter && m_counter->refCount );
	return m_pVal;
}

template< typename T >
T& WeakPtr<T>::operator*()
{
	assert( m_counter && m_counter->refCount );
	return *m_pVal;
}
template< typename T >
const T& WeakPtr<T>::operator*() const
{
	assert( m_counter && m_counter->refCount );
	return *m_pVal;
}

template< typename T >
WeakPtr<T>& WeakPtr<T>::operator=( T* pVal )
{
	// Ignore if same
	if ( pVal == m_pVal ) return *this;

	Release();
	m_pVal = pVal;

	if ( pVal ) 
	{
		m_counter = new CounterData;
		m_counter->refCount = 0;
		m_counter->weakCount = 1;
	}

	return *this;
}

template< typename T >
WeakPtr<T>& WeakPtr<T>::operator=( const WeakPtr& ptr )
{
	if ( ptr.m_pVal == m_pVal) return *this;

	Release();
	m_pVal = ptr.m_pVal;
	m_counter = ptr.m_counter;
	AddRef();

	return *this;
}

template< typename T >
WeakPtr<T>& WeakPtr<T>::operator=( const StrongHandle<T>& ptr )
{
	if ( ptr.m_pVal == m_pVal) return *this;

	Release();
	m_pVal = ptr.m_pVal;
	m_counter = ptr.m_counter;
	AddRef();

	return *this;
}

template< typename T >
WeakPtr<T>::operator T*()
{
	assert( m_counter && m_counter->refCount );
	return m_pVal;
}

template< typename T >
WeakPtr<T>::operator const T*() const
{
	assert( m_counter && m_counter->refCount );
	return m_pVal;
}

template< typename T >
bool WeakPtr<T>::operator==( const WeakPtr& other ) const
{
	T* ptr = m_pVal;
	if ( m_counter && m_counter->refCount == 0 )
	{
		ptr = 0;
	}

	return ptr == other.m_pVal;
}

template< typename T >
bool WeakPtr<T>::operator==( const T* pVal ) const
{
	T* ptr = m_pVal;
	if ( m_counter && m_counter->refCount == 0 )
	{
		ptr = 0;
	}

	return ptr == pVal;
}

template< typename T >
bool WeakPtr<T>::operator!=( const WeakPtr& other ) const
{
	return !( other == *this );
}

template< typename T >
bool WeakPtr<T>::operator!=( const T* pVal ) const
{
	return !( pVal == *this );
}

#endif