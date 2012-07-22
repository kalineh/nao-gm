#ifndef _INCLUDE_SINGLETON_H_
#define _INCLUDE_SINGLETON_H_

#include <cassert>

/*
Template class for all Singleton classes
*/
template <class T>
class Singleton
{
public:
	static void CreateInst();
	static void DestroyInst();

	inline static bool InstExists();
	inline static T *Get( void );
	inline static T &Ref( void ); 

protected:
	static T *s_pInst;
};

template <class T>
void Singleton<T>::CreateInst()
{
	if (!s_pInst) s_pInst = new T(); 
}

template <class T>
T & Singleton<T>::Ref( void )
{
	// Not instantiated yet!
	assert( s_pInst != 0 );

	return *s_pInst;
}

template <class T>
void Singleton<T>::DestroyInst()
{
	if (s_pInst != 0) 
	{
		delete s_pInst;
		s_pInst = 0;
	}
}

template <class T>
T *Singleton<T>::Get(void)
{
	return s_pInst;
}

template <class T>
bool Singleton<T>::InstExists()
{
	return s_pInst != NULL;
}

template <class T>
T *Singleton<T>::s_pInst = 0;

#endif